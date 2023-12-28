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
#include "srtm_io_service.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"

#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_IO_CATEGORY (0x5U)

#define SRTM_IO_VERSION (0x0100U)

#define SRTM_IO_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_IO_RETURN_CODE_FAIL        (0x1U)
#define SRTM_IO_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_IO_CMD_CONF_INPUT_EVENT (0x00U)
#define SRTM_IO_CMD_SET_OUTPUT       (0x01U)
#define SRTM_IO_CMD_GET_INPUT        (0x02U)

#define SRTM_IO_NTF_INPUT_EVENT (0x00U)

/* IO pin list node */
typedef struct _srtm_io_pin
{
    srtm_list_t node;
    uint16_t ioId;
    srtm_channel_t channel;
    srtm_notification_t notif; /* SRTM notification message for input event */
    srtm_io_service_set_output_t setOutput;
    srtm_io_service_get_input_t getInput;
    srtm_io_service_conf_input_t confIEvent;
    void *param;
} *srtm_io_pin_t;

/* Service handle */
typedef struct _srtm_io_service
{
    struct _srtm_service service;
    srtm_list_t pins; /*!< SRTM IO pins list */
} *srtm_io_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SRTM_IoService_FreePin(srtm_io_pin_t pin)
{
    SRTM_Notification_Destroy(pin->notif);
    SRTM_Heap_Free(pin);
}

static void SRTM_IoService_RecycleMessage(srtm_message_t msg, void *param)
{
    uint32_t primask;
    srtm_io_pin_t pin = (srtm_io_pin_t)param;

    assert(pin);
    assert(pin->notif == NULL);

    primask = DisableGlobalIRQ();
    /* Return msg to pin */
    pin->notif = msg;
    EnableGlobalIRQ(primask);
}

static srtm_io_pin_t SRTM_IoService_FindPin(srtm_io_service_t handle, uint16_t ioId, bool rm, bool notify)
{
    srtm_io_pin_t pin = NULL;
    srtm_list_t *list;
    srtm_notification_t notif = NULL;
    uint32_t primask;

    primask = DisableGlobalIRQ();
    for (list = handle->pins.next; list != &handle->pins; list = list->next)
    {
        pin = SRTM_LIST_OBJ(srtm_io_pin_t, node, list);
        if (pin->ioId == ioId)
        {
            if (rm)
            {
                SRTM_List_Remove(list);
            }
            if (notify)
            {
                notif = pin->notif;
                /* If channel is destoryed, the notif of pin should be recycled */
                if (pin->channel)
                {
                    pin->notif = NULL;
                }
            }
            break;
        }
    }
    EnableGlobalIRQ(primask);

    if (notify && notif && pin->channel)
    {
        /* If notification message exists, just deliver it. Otherwise it's on the way, no need
           to deliver again. */
        notif->channel = pin->channel;
        SRTM_Dispatcher_DeliverNotification(handle->service.dispatcher, notif);
    }

    return list == &handle->pins ? NULL : pin;
}

/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_IoService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_io_pin_t pin;
    srtm_channel_t channel;
    uint8_t command, retCode;
    uint8_t *payload;
    srtm_response_t response;
    uint16_t ioId = 0U;
    uint32_t len;
    srtm_io_value_t value = SRTM_IoValueLow;

    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel = SRTM_CommMessage_GetChannel(request);
    assert(channel);
    command = SRTM_CommMessage_GetCommand(request);
    payload = SRTM_CommMessage_GetPayload(request);
    len     = SRTM_CommMessage_GetPayloadLen(request);

    status = SRTM_Service_CheckVersion(service, request, SRTM_IO_VERSION);
    if ((status != SRTM_Status_Success) || (payload == NULL) || (len < 2U))
    {
        /* Either version mismatch or empty payload is not supported */
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: format error, len %d!\r\n", __func__, len);
        retCode = SRTM_IO_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        ioId = (((uint16_t)(*(payload + 1))) << 8) | *payload;
        pin  = SRTM_IoService_FindPin(handle, ioId, false, false);
        if (!pin)
        {
            SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: Pin 0x%x not registered!\r\n", __func__, ioId);
            retCode = SRTM_IO_RETURN_CODE_FAIL;
        }
        else
        {
            /* Record pin's channel for further input event */
            pin->channel = channel;
            switch (command)
            {
                case SRTM_IO_CMD_CONF_INPUT_EVENT:
                    if ((len >= 4U) && (pin->confIEvent != NULL))
                    {
                        status = pin->confIEvent(service, channel->core, ioId, (srtm_io_event_t)(*(payload + 2)),
                                                 (bool)(*(payload + 3)));
                        retCode =
                            status == SRTM_Status_Success ? SRTM_IO_RETURN_CODE_SUCEESS : SRTM_IO_RETURN_CODE_FAIL;
                    }
                    else
                    {
                        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN,
                                           "%s: Command ConfInputEvent not allowed or len %d error!\r\n", __func__,
                                           len);
                        retCode = SRTM_IO_RETURN_CODE_FAIL;
                    }
                    break;
                case SRTM_IO_CMD_SET_OUTPUT:
                    if ((len >= 3U) && (pin->setOutput != NULL))
                    {
                        status = pin->setOutput(service, channel->core, ioId, (srtm_io_value_t)(*(payload + 2)));
                        retCode =
                            status == SRTM_Status_Success ? SRTM_IO_RETURN_CODE_SUCEESS : SRTM_IO_RETURN_CODE_FAIL;
                    }
                    else
                    {
                        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN,
                                           "%s: Command SetOutput not allowed or len %d error!\r\n", __func__, len);
                        retCode = SRTM_IO_RETURN_CODE_FAIL;
                    }
                    break;
                case SRTM_IO_CMD_GET_INPUT:
                    if (pin->getInput)
                    {
                        status = pin->getInput(service, channel->core, ioId, &value);
                        retCode =
                            status == SRTM_Status_Success ? SRTM_IO_RETURN_CODE_SUCEESS : SRTM_IO_RETURN_CODE_FAIL;
                    }
                    else
                    {
                        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: Command %d function not registered!\r\n",
                                           __func__, SRTM_IO_CMD_GET_INPUT);
                        retCode = SRTM_IO_RETURN_CODE_FAIL;
                    }
                    break;
                default:
                    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__, command);
                    retCode = SRTM_IO_RETURN_CODE_UNSUPPORTED;
                    break;
            }
        }
    }

    response = SRTM_Response_Create(channel, SRTM_IO_CATEGORY, SRTM_IO_VERSION, command, 4U);
    if (!response)
    {
        return SRTM_Status_OutOfMemory;
    }

    payload        = SRTM_CommMessage_GetPayload(response);
    *payload       = (uint8_t)ioId;
    *(payload + 1) = (uint8_t)(ioId >> 8U);
    *(payload + 2) = retCode;
    *(payload + 3) = (uint8_t)value; /* Only used in SRTM_IO_CMD_GET_INPUT */

    /* Now the response is ready */
    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_IoService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

srtm_service_t SRTM_IoService_Create(void)
{
    srtm_io_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_io_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_io_service));
    assert(handle);

    SRTM_List_Init(&handle->pins);

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_IO_CATEGORY;
    handle->service.destroy    = SRTM_IoService_Destroy;
    handle->service.request    = SRTM_IoService_Request;
    handle->service.notify     = SRTM_IoService_Notify;

    return &handle->service;
}

void SRTM_IoService_Destroy(srtm_service_t service)
{
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_list_t *list;
    srtm_io_pin_t pin;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(service);
    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    while (!SRTM_List_IsEmpty(&handle->pins))
    {
        list = handle->pins.next;
        SRTM_List_Remove(list);
        pin = SRTM_LIST_OBJ(srtm_io_pin_t, node, list);
        SRTM_IoService_FreePin(pin);
    }

    SRTM_Heap_Free(handle);
}

void SRTM_IoService_Reset(srtm_service_t service, srtm_peercore_t core)
{
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_list_t *list;
    srtm_io_pin_t pin;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    assert(service);

    /* Currently assume just one peer core, need to update all pins. */
    for (list = handle->pins.next; list != &handle->pins; list = list->next)
    {
        pin          = SRTM_LIST_OBJ(srtm_io_pin_t, node, list);
        pin->channel = NULL;
    }
}

srtm_status_t SRTM_IoService_RegisterPin(srtm_service_t service,
                                         uint16_t ioId,
                                         srtm_io_service_set_output_t setOutput,
                                         srtm_io_service_get_input_t getInput,
                                         srtm_io_service_conf_input_t confIEvent,
                                         void *param)
{
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_io_pin_t pin;
    uint8_t *payload;
    uint32_t primask;

    assert(service);
    /* Pin must be registered before service registration. */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    pin = SRTM_IoService_FindPin(handle, ioId, false, false);
    if (pin)
    {
        /* ioId already registered */
        return SRTM_Status_InvalidParameter;
    }

    pin = (srtm_io_pin_t)SRTM_Heap_Malloc(sizeof(struct _srtm_io_pin));
    if (pin == NULL)
    {
        return SRTM_Status_OutOfMemory;
    }

    pin->ioId       = ioId;
    pin->setOutput  = setOutput;
    pin->getInput   = getInput;
    pin->confIEvent = confIEvent;
    pin->param      = param;
    pin->notif      = SRTM_Notification_Create(NULL, SRTM_IO_CATEGORY, SRTM_IO_VERSION, SRTM_IO_NTF_INPUT_EVENT, 2U);
    assert(pin->notif);
    SRTM_Message_SetFreeFunc(pin->notif, SRTM_IoService_RecycleMessage, pin);
    payload = (uint8_t *)SRTM_CommMessage_GetPayload(pin->notif);
    /* Little endian IO pin ID */
    *payload       = (uint8_t)ioId;
    *(payload + 1) = (uint8_t)(ioId >> 8U);

    primask = DisableGlobalIRQ();
    SRTM_List_AddTail(&handle->pins, &pin->node);
    EnableGlobalIRQ(primask);

    return SRTM_Status_Success;
}

srtm_status_t SRTM_IoService_UnregisterPin(srtm_service_t service, uint16_t ioId)
{
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_io_pin_t pin;

    assert(service);
    /* Pin must be unregistered when service is not running. */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    pin = SRTM_IoService_FindPin(handle, ioId, true, false);
    if (pin)
    {
        SRTM_IoService_FreePin(pin);
    }
    else
    {
        /* Not found */
        return SRTM_Status_ListRemoveFailed;
    }

    return SRTM_Status_Success;
}

/* Called in ISR */
srtm_status_t SRTM_IoService_NotifyInputEvent(srtm_service_t service, uint16_t ioId)
{
    srtm_io_service_t handle = (srtm_io_service_t)service;
    srtm_io_pin_t pin;

    assert(service);
    /* Service must be running in dispatcher when notifying input event */
    assert(!SRTM_List_IsEmpty(&service->node));

    pin = SRTM_IoService_FindPin(handle, ioId, false, true);
    if (!pin)
    {
        /* Pin not registered */
        return SRTM_Status_InvalidParameter;
    }

    return SRTM_Status_Success;
}
