/*
 * Copyright 2017, 2024, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "srtm_heap.h"
#include "srtm_dispatcher.h"
#include "srtm_dispatcher_struct.h"
#include "srtm_peercore.h"
#include "srtm_peercore_struct.h"
#include "srtm_service.h"
#include "srtm_service_struct.h"
#include "srtm_channel.h"
#include "srtm_channel_struct.h"
#include "srtm_lfcl_service.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_LFCL_CATEGORY (0x1U)

#define SRTM_LFCL_VERSION (0x0100U)

#define SRTM_LFCL_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_LFCL_RETURN_CODE_FAIL        (0x1U)
#define SRTM_LFCL_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_LFCL_CMD_CHANGE_POWER_MODE (0x00U)
#define SRTM_LFCL_CMD_HEART_BEAT_ENABLE (0x02U)

#define SRTM_LFCL_NTF_HEART_BEAT (0x01U)

#define SRTM_LFCL_POWER_MODE_RUN      (0x01U)
#define SRTM_LFCL_POWER_MODE_VLLS     (0x05U)
#define SRTM_LFCL_POWER_MODE_REBOOT   (0x06U)
#define SRTM_LFCL_POWER_MODE_SHUTDOWN (0x07U)
#define SRTM_LFCL_HEART_BEAT_DISABLE  (0x00U)
#define SRTM_LFCL_HEART_BEAT_ENABLE   (0x01U)

/* Callback list node */
typedef struct _srtm_lfcl_callback
{
    srtm_list_t node;
    srtm_lfcl_service_cb_t callback;
    void *param;
} *srtm_lfcl_callback_t;

/* Service handle */
typedef struct _srtm_lfcl_service
{
    struct _srtm_service service;
    srtm_list_t subscribers; /*!< SRTM life cycle event subscribers */
    srtm_mutex_t mutex;
#if defined(SRTM_STATIC_API) && SRTM_STATIC_API
    srtm_mutex_buf_t mutexStatic;
#endif
} *srtm_lfcl_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static srtm_status_t SRTM_LfclService_NotifySubscribers(srtm_lfcl_service_t handle,
                                                        srtm_peercore_t core,
                                                        srtm_lfcl_event_t event,
                                                        void *eventParam)
{
    srtm_status_t status = SRTM_Status_Success;
    srtm_lfcl_callback_t cb;
    srtm_list_t *list;

    (void)SRTM_Mutex_Lock(handle->mutex);
    for (list = handle->subscribers.next; list != &handle->subscribers; list = list->next)
    {
        cb     = SRTM_LIST_OBJ(srtm_lfcl_callback_t, node, list);
        status = cb->callback(&handle->service, core, event, eventParam, cb->param);
        if (status != SRTM_Status_Success)
        {
            break;
        }
    }
    (void)SRTM_Mutex_Unlock(handle->mutex);

    return status;
}

static srtm_status_t SRTM_LfclService_WakeupPeerCore(srtm_peercore_t core, void *param)
{
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)param;

    return SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_WakeupReq, NULL);
}

static void SRTM_LfclService_DeactivatePeerCore(srtm_dispatcher_t disp, void *param1, void *param2)
{
    srtm_peercore_t core = (srtm_peercore_t)param2;

    (void)SRTM_PeerCore_Deactivate(core, SRTM_LfclService_WakeupPeerCore, param1);
}

static srtm_status_t SRTM_LfclService_ChangePowerMode(
    srtm_lfcl_service_t handle, srtm_peercore_t core, uint8_t mode, srtm_procedure_t *pPre, srtm_procedure_t *pPost)
{
    srtm_status_t status;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, mode);

    switch (mode)
    {
        case SRTM_LFCL_POWER_MODE_RUN:
            status = SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_Running, NULL);
            if (status == SRTM_Status_Success)
            {
                (void)SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Activated);
            }
            break;

        case SRTM_LFCL_POWER_MODE_VLLS:
            status = SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_SuspendReq, NULL);
            if (status == SRTM_Status_Success)
            {
                *pPost = SRTM_Procedure_Create(SRTM_LfclService_DeactivatePeerCore, handle, core);
                if ((*pPost) == NULL)
                {
                    status = SRTM_Status_OutOfMemory;
                }
            }
            break;

        case SRTM_LFCL_POWER_MODE_REBOOT:
            status = SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_RebootReq, NULL);
            (void)SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Inactive);
            break;

        case SRTM_LFCL_POWER_MODE_SHUTDOWN:
            status = SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_ShutdownReq, NULL);
            (void)SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Inactive);
            break;
        default:
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: error mode\r\n", __func__);
            status = SRTM_Status_ServiceNotFound;
            break;
    }

    return status;
}

static srtm_status_t SRTM_LfclService_EnableHeartBeat(srtm_lfcl_service_t handle, srtm_peercore_t core, uint8_t enable)
{
    srtm_lfcl_event_t event = (enable != 0U) ? SRTM_Lfcl_Event_HeartBeatEnable : SRTM_Lfcl_Event_HeartBeatDisable;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s: %d\r\n", __func__, enable);

    return SRTM_LfclService_NotifySubscribers(handle, core, event, NULL);
}

static srtm_status_t SRTM_LfclService_HeartBeat(srtm_lfcl_service_t handle, srtm_peercore_t core)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    return SRTM_LfclService_NotifySubscribers(handle, core, SRTM_Lfcl_Event_HeartBeat, NULL);
}

/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_LfclService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)(void *)service;
    srtm_channel_t channel;
    uint8_t command, retCode;
    uint8_t *payload;
    srtm_response_t response;
    srtm_procedure_t pre  = NULL;
    srtm_procedure_t post = NULL;
    srtm_list_t listHead;

    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel = SRTM_CommMessage_GetChannel(request);
    command = SRTM_CommMessage_GetCommand(request);
    payload = SRTM_CommMessage_GetPayload(request);

    status = SRTM_Service_CheckVersion(service, request, SRTM_LFCL_VERSION);
    if ((status != SRTM_Status_Success) || (payload == NULL))
    {
        /* Either version mismatch or empty payload is not supported */
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: format error!\r\n", __func__);
        retCode = SRTM_LFCL_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        switch (command)
        {
            case SRTM_LFCL_CMD_CHANGE_POWER_MODE:
                status  = SRTM_LfclService_ChangePowerMode(handle, channel->core, *payload, &pre, &post);
                retCode = status == SRTM_Status_Success ? SRTM_LFCL_RETURN_CODE_SUCEESS : SRTM_LFCL_RETURN_CODE_FAIL;
                break;
            case SRTM_LFCL_CMD_HEART_BEAT_ENABLE:
                status  = SRTM_LfclService_EnableHeartBeat(handle, channel->core, *payload);
                retCode = status == SRTM_Status_Success ? SRTM_LFCL_RETURN_CODE_SUCEESS : SRTM_LFCL_RETURN_CODE_FAIL;
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__, command);
                retCode = SRTM_LFCL_RETURN_CODE_UNSUPPORTED;
                break;
        }
    }

    if (SRTM_PeerCore_GetState(channel->core) == SRTM_PeerCore_State_Inactive)
    {
        /* If peer core request to reboot or shutdown, response is not needed */
        return SRTM_Status_TransferNotAvail;
    }

    response = SRTM_Response_Create(channel, SRTM_LFCL_CATEGORY, SRTM_LFCL_VERSION, command, 1U);
    if (response == NULL)
    {
        if (pre != NULL)
        {
            SRTM_Procedure_Destroy(pre);
        }
        if (post != NULL)
        {
            SRTM_Procedure_Destroy(post);
        }
        return SRTM_Status_OutOfMemory;
    }

    payload  = SRTM_CommMessage_GetPayload(response);
    *payload = retCode;

    /* Now the response is ready */
    /* If there's procedure combination, use message list to deliver */
    if ((pre != NULL) || (post != NULL))
    {
        SRTM_List_Init(&listHead);
        if (pre != NULL)
        {
            SRTM_List_AddTail(&listHead, &pre->node);
        }
        SRTM_List_AddTail(&listHead, &response->node);
        if (post != NULL)
        {
            SRTM_List_AddTail(&listHead, &post->node);
        }

        return SRTM_Dispatcher_DeliverMessages(service->dispatcher, &listHead);
    }

    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_LfclService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    srtm_status_t status;
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)(void *)service;
    srtm_channel_t channel;
    uint8_t command;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel = SRTM_CommMessage_GetChannel(notif);
    command = SRTM_CommMessage_GetCommand(notif);

    status = SRTM_Service_CheckVersion(service, notif, SRTM_LFCL_VERSION);
    if (status == SRTM_Status_Success)
    {
        switch (command)
        {
            case SRTM_LFCL_NTF_HEART_BEAT:
                status = SRTM_LfclService_HeartBeat(handle, channel->core);
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__, command);
                status = SRTM_Status_ServiceNotFound;
                break;
        }
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: version mismatch!\r\n", __func__);
    }

    return status;
}

srtm_service_t SRTM_LfclService_Create(void)
{
    srtm_lfcl_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_lfcl_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_lfcl_service));
    assert(handle);

    SRTM_List_Init(&handle->subscribers);
#if defined(SRTM_STATIC_API) && SRTM_STATIC_API
    handle->mutex = SRTM_Mutex_Create(&handle->mutexStatic);
#else
    handle->mutex = SRTM_Mutex_Create();
#endif
    assert(handle->mutex);

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_LFCL_CATEGORY;
    handle->service.destroy    = SRTM_LfclService_Destroy;
    handle->service.request    = SRTM_LfclService_Request;
    handle->service.notify     = SRTM_LfclService_Notify;

    return &handle->service;
}

void SRTM_LfclService_Destroy(srtm_service_t service)
{
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)(void *)service;
    srtm_list_t *list;
    srtm_lfcl_callback_t cb;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(service);
    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    while (!SRTM_List_IsEmpty(&handle->subscribers))
    {
        list = handle->subscribers.next;
        SRTM_List_Remove(list);
        cb = SRTM_LIST_OBJ(srtm_lfcl_callback_t, node, list);
        SRTM_Heap_Free(cb);
    }

    SRTM_Mutex_Destroy(handle->mutex);
    SRTM_Heap_Free(handle);
}

srtm_status_t SRTM_LfclService_Subscribe(srtm_service_t service, srtm_lfcl_service_cb_t callback, void *param)
{
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)(void *)service;
    srtm_lfcl_callback_t cb;

    assert(service);
    assert(callback);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    cb = (srtm_lfcl_callback_t)SRTM_Heap_Malloc(sizeof(struct _srtm_lfcl_callback));
    if (cb == NULL)
    {
        return SRTM_Status_OutOfMemory;
    }

    cb->callback = callback;
    cb->param    = param;

    (void)SRTM_Mutex_Lock(handle->mutex);
    SRTM_List_AddTail(&handle->subscribers, &cb->node);
    (void)SRTM_Mutex_Unlock(handle->mutex);

    return SRTM_Status_Success;
}

srtm_status_t SRTM_LfclService_Unsubscribe(srtm_service_t service, srtm_lfcl_service_cb_t callback, void *param)
{
    srtm_lfcl_service_t handle = (srtm_lfcl_service_t)(void *)service;
    srtm_lfcl_callback_t cb    = NULL;
    srtm_list_t *list;

    assert(service);
    assert(callback);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    (void)SRTM_Mutex_Lock(handle->mutex);
    for (list = handle->subscribers.next; list != &handle->subscribers; list = list->next)
    {
        cb = SRTM_LIST_OBJ(srtm_lfcl_callback_t, node, list);
        if (cb->callback == callback && cb->param == param)
        {
            SRTM_List_Remove(list);
            break;
        }
    }
    (void)SRTM_Mutex_Unlock(handle->mutex);

    if (list == &handle->subscribers)
    {
        return SRTM_Status_ListRemoveFailed;
    }

    SRTM_Heap_Free(cb);

    return SRTM_Status_Success;
}
