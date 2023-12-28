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
#include "srtm_pmic_service.h"
#include "srtm_message.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Protocol definition */
#define SRTM_PMIC_CATEGORY (0x2U)

#define SRTM_PMIC_VERSION (0x0100U)

#define SRTM_PMIC_RETURN_CODE_SUCEESS     (0x0U)
#define SRTM_PMIC_RETURN_CODE_FAIL        (0x1U)
#define SRTM_PMIC_RETURN_CODE_UNSUPPORTED (0x2U)

#define SRTM_PMIC_CMD_ENABLE              (0x0U)
#define SRTM_PMIC_CMD_DISABLE             (0x1U)
#define SRTM_PMIC_CMD_IS_ENABLED          (0x2U)
#define SRTM_PMIC_CMD_SET_VOLTAGE         (0x3U)
#define SRTM_PMIC_CMD_GET_VOLTAGE         (0x4U)
#define SRTM_PMIC_CMD_GET_REGISTER        (0x5U)
#define SRTM_PMIC_CMD_SET_REGISTER        (0x6U)
#define SRTM_PMIC_CMD_SET_STANDBY_VOLTAGE (0x7U)

#define SRTM_PMIC_REGULATOR_STATUS_DISABLED (0x0U)
#define SRTM_PMIC_REGULATOR_STATUS_ENABLED  (0x1U)

/* Service handle */
typedef struct _srtm_pmic_service
{
    struct _srtm_service service;
    srtm_pmic_adapter_t adapter;
} *srtm_pmic_service_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Both request and notify are called from SRTM dispatcher context */
static srtm_status_t SRTM_PmicService_Request(srtm_service_t service, srtm_request_t request)
{
    srtm_status_t status;
    srtm_pmic_service_t handle  = (srtm_pmic_service_t)(void *)service;
    srtm_pmic_adapter_t adapter = handle->adapter;
    srtm_channel_t channel;
    uint8_t command;
    uint32_t payloadLen;
    srtm_response_t response;
    uint32_t tmpOutValue;
    struct _srtm_pmic_payload *pmicReq;
    struct _srtm_pmic_payload *pmicResp;

    assert(adapter);
    assert(service->dispatcher);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    channel    = SRTM_CommMessage_GetChannel(request);
    command    = SRTM_CommMessage_GetCommand(request);
    pmicReq    = (struct _srtm_pmic_payload *)(void *)SRTM_CommMessage_GetPayload(request);
    payloadLen = SRTM_CommMessage_GetPayloadLen(request);

    response = SRTM_Response_Create(channel, SRTM_PMIC_CATEGORY, SRTM_PMIC_VERSION, command,
                                    (uint16_t)sizeof(struct _srtm_pmic_payload));
    if (response == NULL)
    {
        return SRTM_Status_OutOfMemory;
    }

    pmicResp = (struct _srtm_pmic_payload *)(void *)SRTM_CommMessage_GetPayload(response);

    status = SRTM_Service_CheckVersion(service, request, SRTM_PMIC_VERSION);
    if ((status != SRTM_Status_Success) || (pmicReq == NULL) || (payloadLen != sizeof(struct _srtm_pmic_payload)))
    {
        SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s format error!\r\n", __func__);
        pmicResp->reg     = (pmicReq != NULL) ? pmicReq->reg : 0U;
        pmicResp->retCode = SRTM_PMIC_RETURN_CODE_UNSUPPORTED;
    }
    else
    {
        pmicResp->reg = pmicReq->reg; /* Regulator or register number */
        switch (command)
        {
            case SRTM_PMIC_CMD_ENABLE:
                assert(adapter->enable);
                status = adapter->enable(adapter, pmicReq->reg);
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_DISABLE:
                assert(adapter->disable);
                status = adapter->disable(adapter, pmicReq->reg);
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_IS_ENABLED:
                assert(adapter->isEnabled);
                pmicResp->status  = adapter->isEnabled(adapter, pmicReq->reg) ? SRTM_PMIC_REGULATOR_STATUS_ENABLED :
                                                                                SRTM_PMIC_REGULATOR_STATUS_DISABLED;
                pmicResp->retCode = SRTM_PMIC_RETURN_CODE_SUCEESS;
                break;
            case SRTM_PMIC_CMD_SET_VOLTAGE:
                assert(adapter->setVoltage);
                status = adapter->setVoltage(adapter, pmicReq->reg, pmicReq->value);
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_GET_VOLTAGE:
                assert(adapter->getVoltage);
                status          = adapter->getVoltage(adapter, pmicReq->reg, &tmpOutValue);
                pmicResp->value = tmpOutValue;
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_GET_REGISTER:
                assert(adapter->getRegister);
                status          = adapter->getRegister(adapter, pmicReq->reg, &tmpOutValue);
                pmicResp->value = tmpOutValue;
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_SET_REGISTER:
                assert(adapter->setRegister);
                status = adapter->setRegister(adapter, pmicReq->reg, pmicReq->value);
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            case SRTM_PMIC_CMD_SET_STANDBY_VOLTAGE:
                assert(adapter->setStandbyVoltage);
                status = adapter->setStandbyVoltage(adapter, pmicReq->reg, pmicReq->value);
                pmicResp->retCode =
                    status == SRTM_Status_Success ? SRTM_PMIC_RETURN_CODE_SUCEESS : SRTM_PMIC_RETURN_CODE_FAIL;
                break;
            default:
                SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__, command);
                pmicResp->retCode = SRTM_PMIC_RETURN_CODE_UNSUPPORTED;
                break;
        }
    }

    return SRTM_Dispatcher_DeliverResponse(service->dispatcher, response);
}

static srtm_status_t SRTM_PmicService_Notify(srtm_service_t service, srtm_notification_t notif)
{
    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_WARN, "%s: command %d unsupported\r\n", __func__,
                       SRTM_CommMessage_GetCommand(notif));

    return SRTM_Status_ServiceNotFound;
}

srtm_service_t SRTM_PmicService_Create(srtm_pmic_adapter_t adapter)
{
    srtm_pmic_service_t handle;

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    handle = (srtm_pmic_service_t)SRTM_Heap_Malloc(sizeof(struct _srtm_pmic_service));
    assert(handle);

    handle->adapter = adapter;

    SRTM_List_Init(&handle->service.node);
    handle->service.dispatcher = NULL;
    handle->service.category   = SRTM_PMIC_CATEGORY;
    handle->service.destroy    = SRTM_PmicService_Destroy;
    handle->service.request    = SRTM_PmicService_Request;
    handle->service.notify     = SRTM_PmicService_Notify;

    return &handle->service;
}

void SRTM_PmicService_Destroy(srtm_service_t service)
{
    srtm_pmic_service_t handle = (srtm_pmic_service_t)(void *)service;

    assert(service);

    SRTM_DEBUG_MESSAGE(SRTM_DEBUG_VERBOSE_INFO, "%s\r\n", __func__);

    /* Service must be unregistered from dispatcher before destroy */
    assert(SRTM_List_IsEmpty(&service->node));

    SRTM_Heap_Free(handle);
}
