/*
 * Copyright 2021 NXP
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
#include "srtm_pwm_service.h"
#include "srtm_message.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_PWM_CATEGORY (0xAU)

#define SRTM_PWM_VERSION (0x0100U)

#define SRTM_PWM_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_PWM_RETURN_CODE_FAIL        (0x1U)
#define SRTM_PWM_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_PWM_CMD_GET (0x0U)
#define SRTM_PWM_CMD_SET (0x1U)

/* Service handle */
typedef struct _srtm_pwm_service
{
    struct _srtm_service service;
    srtm_pwm_adapter_t adapter;
} *srtm_pwm_service_t;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_PwmService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_pwm_service_t handle  = (srtm_pwm_service_t)(void *)service;
    srtm_pwm_adapter_t adapter = handle->adapter;
    srtm_channel_t channel;
    uint8_t command;
    uint32_t payloadLen;
    srtm_response_t response;
    struct _srtm_pwm_payload *pwmReq;
    struct _srtm_pwm_payload *pwmResp;
    uint64_t period, duty;

    assert(adapter);
    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel    = SRTM_CommMessage_GetChannel(request);
    command    = SRTM_CommMessage_GetCommand(request);
    pwmReq     = (struct _srtm_pwm_payload *)(void *)SRTM_CommMessage_GetPayload(request);
    payloadLen = SRTM_CommMessage_GetPayloadLen(request);

    response = SRTM_Response_Create(channel, SRTM_PWM_CATEGORY, SRTM_PWM_VERSION, command,
                                    (uint16_t)sizeof(struct _srtm_pwm_payload));
    if (response == NULL)
    {
        return SRTM_Status_OutOfMemory;
    }

    pwmResp = (struct _srtm_pwm_payload *)(void *)SRTM_CommMessage_GetPayload(response);

    status = SRTM_Service_CheckVersion(service, request, SRTM_PWM_VERSION);
    if ((status != SRTM_Status_Success) || (pwmReq == NULL) || (payloadLen != sizeof(struct _srtm_pwm_payload)))
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s format error!\r\n", __func__);
        pwmResp->retCode = SRTM_PWM_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO,
                           "SRTM receive PWM request: cmd = 0x%x, chipId = 0x%x, channelId = 0x%x,\
                           period    = 0x % x,\
                           dutyCycle = 0x % x, polarity = 0x % x,\
                           enable = 0x % x\n",
                           command, pwmReq->chipId, pwmReq->channelId, pwmReq->period, pwmReq->dutyCycle,
                           pwmReq->polarity, pwmReq->enable);
        memcpy(pwmResp, pwmReq, sizeof(struct _srtm_pwm_payload));
        switch (command)
        {
            case SRTM_PWM_CMD_GET:
                assert(adapter->getPwm);
                status             = adapter->getPwm(adapter, pwmResp->chipId, pwmResp->channelId, &period, &duty,
                                                     &pwmResp->polarity, &pwmResp->enable);
                pwmResp->period    = period;
                pwmResp->dutyCycle = duty;
                pwmResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PWM_RETURN_CODE_SUCEESS : SRTM_PWM_RETURN_CODE_FAIL;
                break;
            case SRTM_PWM_CMD_SET:
                assert(adapter->setPwm);
                status = adapter->setPwm(adapter, pwmReq->chipId, pwmResp->channelId, pwmResp->period,
                                         pwmResp->dutyCycle, pwmResp->polarity, pwmResp->enable);
                pwmResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PWM_RETURN_CODE_SUCEESS : SRTM_PWM_RETURN_CODE_FAIL;
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__, command);
                pwmResp->retCode = SRTM_PWM_RETURN_CODE_UNSUPPORTED;
                break;
        }
    }

    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_PwmService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

srtm_service_t SRTM_PwmService_Create(srtm_pwm_adapter_t adapter)
{
    srtm_pwm_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_pwm_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_pwm_service));
    assert(handle);

    handle->adapter = adapter;

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_PWM_CATEGORY;
    handle->service.destroy    = SRTM_PwmService_Destroy;
    handle->service.request    = SRTM_PwmService_Request;
    handle->service.notify     = SRTM_PwmService_Notify;

    return &handle->service;
}

void SRTM_PwmService_Destroy(srtm_service_t service)
{
    srtm_pwm_service_t handle = (srtm_pwm_service_t)(void *)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_Heap_Free(handle);
}
