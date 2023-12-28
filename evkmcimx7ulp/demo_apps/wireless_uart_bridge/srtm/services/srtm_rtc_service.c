/*
 * Copyright 2017-2018, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "srtm_heap.h"
#include "srtm_dispatcher.h"
#include "srtm_service.h"
#include "srtm_service_struct.h"
#include "srtm_rtc_service.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_RTC_CATEGORY (0x6U)

#define SRTM_RTC_VERSION (0x0100U)

#define SRTM_RTC_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_RTC_RETURN_CODE_FAIL        (0x1U)
#define SRTM_RTC_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_RTC_CMD_SET_TIME     (0x0U)
#define SRTM_RTC_CMD_GET_TIME     (0x1U)
#define SRTM_RTC_CMD_SET_ALARM    (0x2U)
#define SRTM_RTC_CMD_GET_ALARM    (0x3U)
#define SRTM_RTC_CMD_ENABLE_ALARM (0x4U)

#define SRTM_RTC_NTF_ALARM (0x0U)

/* Service handle */
typedef struct _srtm_rtc_service
{
    struct _srtm_service service;
    srtm_rtc_adapter_t adapter;
    /* Currently assume just one peer core, to support multiple peer cores, channel need to be a list */
    srtm_channel_t channel;
    srtm_notification_t notif;
} *srtm_rtc_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void SRTM_RtcService_RecycleMessage(srtm_message_t msg, void *param)
{
    srtm_rtc_service_t handle = (srtm_rtc_service_t)param;

    handle->notif = msg;
}

/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_RtcService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_rtc_service_t handle  = (srtm_rtc_service_t)service;
    srtm_rtc_adapter_t adapter = handle->adapter;
    srtm_channel_t channel;
    uint8_t command;
    uint32_t payloadLen;
    uint32_t seconds;
    srtm_response_t response;
    struct _srtm_rtc_payload *rtcReq;
    struct _srtm_rtc_payload *rtcResp;

    assert(adapter);
    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel    = SRTM_CommMessage_GetChannel(request);
    command    = SRTM_CommMessage_GetCommand(request);
    rtcReq     = (struct _srtm_rtc_payload *)SRTM_CommMessage_GetPayload(request);
    payloadLen = SRTM_CommMessage_GetPayloadLen(request);

    response = SRTM_Response_Create(channel, SRTM_RTC_CATEGORY, SRTM_RTC_VERSION, command,
                                    (uint16_t)sizeof(struct _srtm_rtc_payload));
    if (!response)
    {
        return SRTM_Status_OutOfMemory;
    }
    else
    {
        /* Remember the channel for future notification */
        handle->channel = channel;
    }

    rtcResp = (struct _srtm_rtc_payload *)(void *)SRTM_CommMessage_GetPayload(response);

    status = SRTM_Service_CheckVersion(service, request, SRTM_RTC_VERSION);
    if ((status != SRTM_Status_Success) || (rtcReq == NULL) || (payloadLen != sizeof(struct _srtm_rtc_payload)))
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s format error %d!\r\n", __func__, payloadLen);
        rtcResp->retCode = SRTM_RTC_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        switch (command)
        {
            case SRTM_RTC_CMD_SET_TIME:
                assert(adapter->setTime);
                status = adapter->setTime(adapter, rtcReq->alarm.seconds);
                rtcResp->retCode =
                    status == SRTM_Status_Success ? SRTM_RTC_RETURN_CODE_SUCEESS : SRTM_RTC_RETURN_CODE_FAIL;
                break;
            case SRTM_RTC_CMD_GET_TIME:
                assert(adapter->getTime);
                status                 = adapter->getTime(adapter, &seconds);
                rtcResp->alarm.seconds = seconds;
                rtcResp->retCode =
                    status == SRTM_Status_Success ? SRTM_RTC_RETURN_CODE_SUCEESS : SRTM_RTC_RETURN_CODE_FAIL;
                break;
            case SRTM_RTC_CMD_SET_ALARM:
                assert(adapter->setAlarm);
                status = adapter->setAlarm(adapter, &rtcReq->alarm);
                rtcResp->retCode =
                    status == SRTM_Status_Success ? SRTM_RTC_RETURN_CODE_SUCEESS : SRTM_RTC_RETURN_CODE_FAIL;
                break;
            case SRTM_RTC_CMD_GET_ALARM:
                assert(adapter->getAlarm);
                status = adapter->getAlarm(adapter, &rtcResp->alarm);
                rtcResp->retCode =
                    status == SRTM_Status_Success ? SRTM_RTC_RETURN_CODE_SUCEESS : SRTM_RTC_RETURN_CODE_FAIL;
                break;
            case SRTM_RTC_CMD_ENABLE_ALARM:
                assert(adapter->enableAlarm);
                status = adapter->enableAlarm(adapter, rtcReq->alarm.enabled);
                rtcResp->retCode =
                    status == SRTM_Status_Success ? SRTM_RTC_RETURN_CODE_SUCEESS : SRTM_RTC_RETURN_CODE_FAIL;
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__, command);
                rtcResp->retCode = SRTM_RTC_RETURN_CODE_UNSUPPORTED;
                break;
        }
    }

    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_RtcService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

/* CALLED IN RTC DRIVER ISR */
static srtm_status_t SRTM_RtcService_NotifyAlarm(srtm_service_t service)
{
    srtm_status_t status      = SRTM_Status_Success;
    srtm_rtc_service_t handle = (srtm_rtc_service_t)(void *)service;

    /* If service is still serving and no pending alarm */
    if ((service->dispatcher != NULL) && (handle->notif != NULL) && (handle->channel != NULL))
    {
        handle->notif->channel = handle->channel;
        status                 = SRTM_Dispatcher_DeliverNotification(service->dispatcher, handle->notif);
        handle->notif          = NULL;
    }

    return status;
}

srtm_service_t SRTM_RtcService_Create(srtm_rtc_adapter_t adapter)
{
    srtm_rtc_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_rtc_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_rtc_service));
    assert(handle);

    adapter->service     = &handle->service;
    adapter->notifyAlarm = SRTM_RtcService_NotifyAlarm;
    handle->adapter      = adapter;

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_RTC_CATEGORY;
    handle->service.destroy    = SRTM_RtcService_Destroy;
    handle->service.request    = SRTM_RtcService_Request;
    handle->service.notify     = SRTM_RtcService_Notify;

    handle->channel = NULL;
    handle->notif   = SRTM_Notification_Create(NULL, SRTM_RTC_CATEGORY, SRTM_RTC_VERSION, SRTM_RTC_NTF_ALARM, 0);
    assert(handle->notif);
    SRTM_Message_SetFreeFunc(handle->notif, SRTM_RtcService_RecycleMessage, handle);

    return &handle->service;
}

void SRTM_RtcService_Destroy(srtm_service_t service)
{
    srtm_rtc_service_t handle = (srtm_rtc_service_t)(void *)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    if (handle->notif != NULL)
    {
        SRTM_Message_SetFreeFunc(handle->notif, NULL, NULL);
        SRTM_Message_Destroy(handle->notif);
    }

    SRTM_Heap_Free(handle);
}

void SRTM_RtcService_Reset(srtm_service_t service, srtm_peercore_t core)
{
    srtm_rtc_service_t handle = (srtm_rtc_service_t)(void *)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Currently assume just one peer core, need to reset the service. */
    (void)handle->adapter->enableAlarm(handle->adapter, false);
    handle->channel = NULL;
}
