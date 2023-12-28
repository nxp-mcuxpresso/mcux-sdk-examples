/*
 * Copyright 2017, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "srtm_heap.h"
#include "srtm_dispatcher.h"
#include "srtm_peercore.h"
#include "srtm_service.h"
#include "srtm_service_struct.h"
#include "srtm_channel.h"
#include "srtm_channel_struct.h"
#include "srtm_keypad_service.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_KEYPAD_CATEGORY (0x4U)

#define SRTM_KEYPAD_VERSION (0x0100U)

#define SRTM_KEYPAD_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_KEYPAD_RETURN_CODE_FAIL        (0x1U)
#define SRTM_KEYPAD_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_KEYPAD_CMD_CONF_EVENT (0x00U)

#define SRTM_KEYPAD_NTF_KEYPAD_EVENT (0x00U)

/* Consider press and release in short time, we need 2 events per key */
#define SRTM_KEYPAD_NTF_NUM_PER_KEY (0x2U)

/* keypad key list node */
typedef struct _srtm_keypad_key
{
    srtm_list_t node;
    uint8_t keyIdx;
    srtm_channel_t channel;
    srtm_notification_t notif[SRTM_KEYPAD_NTF_NUM_PER_KEY]; /* SRTM notification message for keypad event */
    srtm_keypad_service_conf_t confKEvent;
    void *param;
} *srtm_keypad_key_t;

/* Service handle */
typedef struct _srtm_keypad_service
{
    struct _srtm_service service;
    srtm_list_t keys; /*!< SRTM keypad keys list */
} *srtm_keypad_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SRTM_KeypadService_FreeKey(srtm_keypad_key_t key)
{
    uint32_t i;

    for (i = 0U; i < SRTM_KEYPAD_NTF_NUM_PER_KEY; i++)
    {
        SRTM_Notification_Destroy(key->notif[i]);
    }
    SRTM_Heap_Free(key);
}

static uint32_t SRTM_KeypadService_GetKeyNotifIndex(srtm_keypad_key_t key)
{
    uint32_t i;

    for (i = 0U; i < SRTM_KEYPAD_NTF_NUM_PER_KEY; i++)
    {
        if (key->notif[i] != NULL)
        {
            break;
        }
    }

    return i;
}

static void SRTM_KeypadService_RecycleMessage(srtm_message_t msg, void *param)
{
    uint32_t primask;
    srtm_notification_t *pNotif = (srtm_notification_t *)param;

    assert(pNotif != NULL);
    assert(*pNotif == NULL);

    primask = DisableGlobalIRQ();
    /* Return msg to key */
    *pNotif = msg;
    EnableGlobalIRQ(primask);
}

static srtm_keypad_key_t SRTM_KeypadService_FindKey(
    srtm_keypad_service_t handle, uint8_t keyIdx, bool rm, bool notify, srtm_keypad_value_t value)
{
    srtm_keypad_key_t key = NULL;
    srtm_list_t *list;
    srtm_notification_t notif = NULL;
    uint32_t primask;
    uint8_t *payload;
    uint32_t i;

    primask = DisableGlobalIRQ();
    for (list = handle->keys.next; list != &handle->keys; list = list->next)
    {
        key = SRTM_LIST_OBJ(srtm_keypad_key_t, node, list);
        if (key->keyIdx == keyIdx)
        {
            if (rm)
            {
                SRTM_List_Remove(list);
            }
            if (notify)
            {
                i = SRTM_KeypadService_GetKeyNotifIndex(key);
                if (i != SRTM_KEYPAD_NTF_NUM_PER_KEY)
                {
                    notif         = key->notif[i];
                    key->notif[i] = NULL;
                }
            }
            break;
        }
    }
    EnableGlobalIRQ(primask);

    if (notify && notif && key->channel)
    {
        /* If notification message exists, just deliver it. Otherwise it's on the way, no need
           to deliver again. */
        notif->channel = key->channel;
        payload        = SRTM_CommMessage_GetPayload(notif);
        *(payload + 1) = (uint8_t)value;
        SRTM_Dispatcher_DeliverNotification(handle->service.dispatcher, notif);
    }
    else if (notify && key->channel)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "Keypad event lost\r\n");
    }
    else
    {
        /* Do nothing */
    }

    return list == &handle->keys ? NULL : key;
}

/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_KeypadService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_keypad_key_t key;
    srtm_channel_t channel;
    uint8_t command, retCode;
    uint8_t *payload;
    srtm_response_t response;
    uint8_t keyIdx = 0U;
    uint32_t len;

    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel = SRTM_CommMessage_GetChannel(request);
    assert(channel);
    command = SRTM_CommMessage_GetCommand(request);
    payload = SRTM_CommMessage_GetPayload(request);
    len     = SRTM_CommMessage_GetPayloadLen(request);

    status = SRTM_Service_CheckVersion(service, request, SRTM_KEYPAD_VERSION);
    if (status != SRTM_Status_Success || !payload || len < 3)
    {
        /* Either version mismatch or empty payload is not supported */
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: format error, len %d!\r\n", __func__, len);
        retCode = SRTM_KEYPAD_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        keyIdx = *payload;
        key    = SRTM_KeypadService_FindKey(handle, keyIdx, false, false, SRTM_KeypadValueReleased);
        if (!key)
        {
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: Key 0x%x not registered!\r\n", __func__, keyIdx);
            retCode = SRTM_KEYPAD_RETURN_CODE_FAIL;
        }
        else
        {
            /* Record key's channel for further event notification */
            key->channel = channel;
            switch (command)
            {
                case SRTM_KEYPAD_CMD_CONF_EVENT:
                    assert(key->confKEvent);
                    status = key->confKEvent(service, channel->core, keyIdx, (srtm_keypad_event_t)(*(payload + 1)),
                                             (bool)(*(payload + 2)));
                    retCode =
                        status == SRTM_Status_Success ? SRTM_KEYPAD_RETURN_CODE_SUCEESS : SRTM_KEYPAD_RETURN_CODE_FAIL;
                    break;
                default:
                    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__, command);
                    retCode = SRTM_KEYPAD_RETURN_CODE_UNSUPPORTED;
                    break;
            }
        }
    }

    response = SRTM_Response_Create(channel, SRTM_KEYPAD_CATEGORY, SRTM_KEYPAD_VERSION, command, 2U);
    if (!response)
    {
        return SRTM_Status_OutOfMemory;
    }

    payload        = SRTM_CommMessage_GetPayload(response);
    *payload       = keyIdx;
    *(payload + 1) = retCode;

    /* Now the response is ready */
    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_KeypadService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

srtm_service_t SRTM_KeypadService_Create(void)
{
    srtm_keypad_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_keypad_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_keypad_service));
    assert(handle);

    SRTM_List_Init(&handle->keys);

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_KEYPAD_CATEGORY;
    handle->service.destroy    = SRTM_KeypadService_Destroy;
    handle->service.request    = SRTM_KeypadService_Request;
    handle->service.notify     = SRTM_KeypadService_Notify;

    return &handle->service;
}

void SRTM_KeypadService_Destroy(srtm_service_t service)
{
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_list_t *list;
    srtm_keypad_key_t key;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(service);
    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    while (!SRTM_List_IsEmpty(&handle->keys))
    {
        list = handle->keys.next;
        SRTM_List_Remove(list);
        key = SRTM_LIST_OBJ(srtm_keypad_key_t, node, list);
        SRTM_KeypadService_FreeKey(key);
    }

    SRTM_Heap_Free(handle);
}

void SRTM_KeypadService_Reset(srtm_service_t service, srtm_peercore_t core)
{
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_list_t *list;
    srtm_keypad_key_t key;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(service);

    /* Currently assume just one peer core, need to update all keys. */
    for (list = handle->keys.next; list != &handle->keys; list = list->next)
    {
        key          = SRTM_LIST_OBJ(srtm_keypad_key_t, node, list);
        key->channel = NULL;
    }
}

srtm_status_t SRTM_KeypadService_RegisterKey(srtm_service_t service,
                                             uint8_t keyIdx,
                                             srtm_keypad_service_conf_t confKEvent,
                                             void *param)
{
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_keypad_key_t key;
    uint8_t *payload;
    uint32_t primask;
    uint32_t i;

    assert(service);
    /* Key must be registered before service registration. */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    key = SRTM_KeypadService_FindKey(handle, keyIdx, false, false, SRTM_KeypadValueReleased);
    if (key)
    {
        /* keyIdx already registered */
        return SRTM_Status_InvalidParameter;
    }

    key = (srtm_keypad_key_t)SRTM_Heap_Malloc(sizeof(struct _srtm_keypad_key));
    if (key == NULL)
    {
        return SRTM_Status_OutOfMemory;
    }

    key->keyIdx     = keyIdx;
    key->confKEvent = confKEvent;
    key->param      = param;
    for (i = 0U; i < SRTM_KEYPAD_NTF_NUM_PER_KEY; i++)
    {
        key->notif[i] =
            SRTM_Notification_Create(NULL, SRTM_KEYPAD_CATEGORY, SRTM_KEYPAD_VERSION, SRTM_KEYPAD_NTF_KEYPAD_EVENT, 2U);
        assert(key->notif[i]);
        SRTM_Message_SetFreeFunc(key->notif[i], SRTM_KeypadService_RecycleMessage, &key->notif[i]);
        payload = (uint8_t *)SRTM_CommMessage_GetPayload(key->notif[i]);
        /* Little endian keypad key ID */
        *payload = keyIdx;
    }

    primask = DisableGlobalIRQ();
    SRTM_List_AddTail(&handle->keys, &key->node);
    EnableGlobalIRQ(primask);

    return SRTM_Status_Success;
}

srtm_status_t SRTM_KeypadService_UnregisterKey(srtm_service_t service, uint8_t keyIdx)
{
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_keypad_key_t key;

    assert(service);
    /* Key must be unregistered when service is not running. */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    key = SRTM_KeypadService_FindKey(handle, keyIdx, true, false, SRTM_KeypadValueReleased);
    if (key)
    {
        SRTM_KeypadService_FreeKey(key);
    }
    else
    {
        /* Not found */
        return SRTM_Status_ListRemoveFailed;
    }

    return SRTM_Status_Success;
}

/* Called in ISR */
srtm_status_t SRTM_KeypadService_NotifyKeypadEvent(srtm_service_t service, uint8_t keyIdx, srtm_keypad_value_t value)
{
    srtm_keypad_service_t handle = (srtm_keypad_service_t)service;
    srtm_keypad_key_t key;

    assert(service);
    /* Service must be running in dispatcher when notifying keypad event */
    assert(!SRTM_List_IsEmpty(&service->node));

    key = SRTM_KeypadService_FindKey(handle, keyIdx, false, true, value);
    if (!key)
    {
        /* Key not registered */
        return SRTM_Status_InvalidParameter;
    }

    return SRTM_Status_Success;
}
