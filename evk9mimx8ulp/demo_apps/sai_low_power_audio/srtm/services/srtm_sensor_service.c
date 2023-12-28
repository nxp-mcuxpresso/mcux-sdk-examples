/*
 * Copyright 2018, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include "fsl_common.h"

#include "srtm_heap.h"
#include "srtm_list.h"
#include "srtm_dispatcher.h"
#include "srtm_service.h"
#include "srtm_service_struct.h"
#include "srtm_sensor_service.h"
#include "srtm_message.h"
#include "srtm_message_struct.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_SENSOR_CATEGORY (0x7U)

#define SRTM_SENSOR_VERSION (0x0100U)

#define SRTM_SENSOR_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_SENSOR_RETURN_CODE_FAIL        (0x1U)
#define SRTM_SENSOR_RETURN_CODE_UNSUPPORTED (0x2U)

/* Sensor Service Request Command definition */
#define SRTM_SENSOR_CMD_ENABLE_STATE_DETECTOR (0x0U)
#define SRTM_SENSOR_CMD_ENABLE_DATA_REPORT    (0x1U)
#define SRTM_SENSOR_CMD_SET_POLL_DELAY        (0x2U)

/* Sensor Service Notification Command definition */
#define SRTM_SENSOR_NTF_STATE_CHANGED (0x0U)
#define SRTM_SENSOR_NTF_DATA          (0x1U)

/* Service handle */
typedef struct _srtm_sensor_service
{
    struct _srtm_service service;
    srtm_sensor_adapter_t sensor;
    /* Currently assume just one peer core, to support multiple peer cores, channel need to be a list */
    srtm_channel_t channel;
} *srtm_sensor_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static srtm_status_t SRTM_SensorService_UpdateState(srtm_service_t service, srtm_sensor_type_t type, uint8_t index)
{
    srtm_notification_t notif;
    struct _srtm_sensor_payload *payload;
    srtm_sensor_service_t handle = (srtm_sensor_service_t)(void *)service;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* First allocate a notification and send to peer core */
    notif = SRTM_Notification_Create(handle->channel, SRTM_SENSOR_CATEGORY, SRTM_SENSOR_VERSION,
                                     SRTM_SENSOR_NTF_STATE_CHANGED, (uint16_t)sizeof(struct _srtm_sensor_payload));
    if (notif == NULL)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: alloc notification failed.\r\n", __func__);
        return SRTM_Status_OutOfMemory;
    }

    payload        = (struct _srtm_sensor_payload *)(void *)SRTM_CommMessage_GetPayload(notif);
    payload->type  = (uint8_t)type;
    payload->index = index;

    (void)SRTM_Dispatcher_DeliverNotification(handle->service.dispatcher, notif);

    return SRTM_Status_Success;
}

static srtm_status_t SRTM_SensorService_ReportData(
    srtm_service_t service, srtm_sensor_type_t type, uint8_t index, uint8_t *data, uint32_t dataLen)
{
    srtm_notification_t notif;
    struct _srtm_sensor_payload *payload;
    srtm_sensor_service_t handle = (srtm_sensor_service_t)(void *)service;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* First allocate a notification and send to peer core */
    notif = SRTM_Notification_Create(handle->channel, SRTM_SENSOR_CATEGORY, SRTM_SENSOR_VERSION, SRTM_SENSOR_NTF_DATA,
                                     (uint16_t)sizeof(struct _srtm_sensor_payload));
    if (!notif)
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_ERROR, "%s: alloc notification failed.\r\n", __func__);
        return SRTM_Status_OutOfMemory;
    }

    payload        = (struct _srtm_sensor_payload *)SRTM_CommMessage_GetPayload(notif);
    payload->type  = (uint8_t)type;
    payload->index = index;
    assert(dataLen == 4);
    payload->data = *((uint32_t *)data);

    SRTM_Dispatcher_DeliverNotification(handle->service.dispatcher, notif);

    return SRTM_Status_Success;
}

/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_SensorService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_sensor_service_t handle = (srtm_sensor_service_t)service;
    srtm_sensor_adapter_t sensor = handle->sensor;
    srtm_channel_t channel;
    uint8_t command;
    uint32_t payloadLen;
    srtm_response_t response;
    struct _srtm_sensor_payload *sensorReq;
    struct _srtm_sensor_payload *sensorResp;

    assert(sensor);
    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel    = SRTM_CommMessage_GetChannel(request);
    command    = SRTM_CommMessage_GetCommand(request);
    sensorReq  = (struct _srtm_sensor_payload *)SRTM_CommMessage_GetPayload(request);
    payloadLen = SRTM_CommMessage_GetPayloadLen(request);

    response = SRTM_Response_Create(channel, SRTM_SENSOR_CATEGORY, SRTM_SENSOR_VERSION, command,
                                    (uint16_t)sizeof(struct _srtm_sensor_payload));
    if (!response)
    {
        return SRTM_Status_OutOfMemory;
    }
    else
    {
        /* Remember the channel for future notification */
        handle->channel = channel;
    }

    sensorResp = (struct _srtm_sensor_payload *)SRTM_CommMessage_GetPayload(response);

    status = SRTM_Service_CheckVersion(service, request, SRTM_SENSOR_VERSION);
    if (status != SRTM_Status_Success || !sensorReq || payloadLen != sizeof(struct _srtm_sensor_payload))
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: format error!\r\n", __func__);
        sensorResp->type    = sensorReq ? sensorReq->type : 0;
        sensorResp->index   = sensorReq ? sensorReq->index : 0;
        sensorResp->retCode = SRTM_SENSOR_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        sensorResp->type  = sensorReq->type;
        sensorResp->index = sensorReq->index;
        switch (command)
        {
            case SRTM_SENSOR_CMD_ENABLE_STATE_DETECTOR:
                assert(sensor->enableStateDetector);
                status = sensor->enableStateDetector(sensor, (srtm_sensor_type_t)sensorReq->type, sensorReq->index,
                                                     sensorReq->enable ? true : false);
                sensorResp->retCode =
                    status == SRTM_Status_Success ? SRTM_SENSOR_RETURN_CODE_SUCEESS : SRTM_SENSOR_RETURN_CODE_FAIL;
                break;
            case SRTM_SENSOR_CMD_ENABLE_DATA_REPORT:
                assert(sensor->enableDataReport);
                status = sensor->enableDataReport(sensor, (srtm_sensor_type_t)sensorReq->type, sensorReq->index,
                                                  sensorReq->enable ? true : false);
                sensorResp->retCode =
                    status == SRTM_Status_Success ? SRTM_SENSOR_RETURN_CODE_SUCEESS : SRTM_SENSOR_RETURN_CODE_FAIL;
                break;
            case SRTM_SENSOR_CMD_SET_POLL_DELAY:
                assert(sensor->setPollDelay);
                status = sensor->setPollDelay(sensor, (srtm_sensor_type_t)sensorReq->type, sensorReq->index,
                                              sensorReq->pollDelay);
                sensorResp->retCode =
                    status == SRTM_Status_Success ? SRTM_SENSOR_RETURN_CODE_SUCEESS : SRTM_SENSOR_RETURN_CODE_FAIL;
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__, command);
                sensorResp->retCode = SRTM_SENSOR_RETURN_CODE_UNSUPPORTED;
                break;
        }
    }

    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_SensorService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported!\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

srtm_service_t SRTM_SensorService_Create(srtm_sensor_adapter_t sensor)
{
    srtm_sensor_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_sensor_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_sensor_service));
    assert(handle);

    sensor->service     = &handle->service;
    sensor->updateState = SRTM_SensorService_UpdateState;
    sensor->reportData  = SRTM_SensorService_ReportData;
    handle->sensor      = sensor;

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_SENSOR_CATEGORY;
    handle->service.destroy    = SRTM_SensorService_Destroy;
    handle->service.request    = SRTM_SensorService_Request;
    handle->service.notify     = SRTM_SensorService_Notify;

    handle->channel = NULL;

    return &handle->service;
}

void SRTM_SensorService_Destroy(srtm_service_t service)
{
    srtm_sensor_service_t handle = (srtm_sensor_service_t)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_Heap_Free(handle);
}

void SRTM_SensorService_Reset(srtm_service_t service, srtm_peercore_t core)
{
    srtm_sensor_service_t handle = (srtm_sensor_service_t)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Currently assume just one peer core, need to disable all sensor activity. */
    handle->sensor->enableStateDetector(handle->sensor, SRTM_SensorTypePedometer, 0, false);
    handle->sensor->enableStateDetector(handle->sensor, SRTM_SensorTypeTilt, 0, false);
    handle->sensor->enableDataReport(handle->sensor, SRTM_SensorTypePedometer, 0, false);

    handle->channel = NULL;
}
