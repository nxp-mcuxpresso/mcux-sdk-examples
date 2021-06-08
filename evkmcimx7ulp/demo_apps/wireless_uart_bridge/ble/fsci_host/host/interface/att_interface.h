/*! *********************************************************************************
 * \defgroup ATT ATT
 * @{
 ********************************************************************************** */
/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _ATT_INTERFACE_H_
#define _ATT_INTERFACE_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "ble_general.h"
#include "att_types.h"
#include "att_params.h"
#include "att_callbacks.h"

/************************************************************************************
 *************************************************************************************
 * Public prototypes
 *************************************************************************************
 ************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_Init(void);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_NotifyConnection(deviceId_t deviceId);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_NotifyDisconnection(deviceId_t deviceId);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_SetMtu(deviceId_t deviceId, uint16_t mtu);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_GetMtu(deviceId_t deviceId, uint16_t *pOutMtu);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_RegisterOpcodeCallback(attOpcode_t opcode, void *callback);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_RegisterUnsupportedOpcodeCallback(attUnsupportedOpcodeCallback_t callback);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t Att_RegisterTimeoutCallback(attTimeoutCallback_t timeoutCallback);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendErrorResponse(deviceId_t deviceId, attErrorResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendExchangeMtuRequest(deviceId_t deviceId, attExchangeMtuRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendExchangeMtuResponse(deviceId_t deviceId, attExchangeMtuResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendFindInformationRequest(deviceId_t deviceId, attFindInformationRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendFindInformationResponse(deviceId_t deviceId, attFindInformationResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendFindByTypeValueRequest(deviceId_t deviceId, attFindByTypeValueRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendFindByTypeValueResponse(deviceId_t deviceId, attFindByTypeValueResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendReadByTypeRequest(deviceId_t deviceId, attReadByTypeRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendReadByTypeResponse(deviceId_t deviceId, attReadByTypeResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendReadRequest(deviceId_t deviceId, attReadRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendReadResponse(deviceId_t deviceId, attReadResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendReadBlobRequest(deviceId_t deviceId, attReadBlobRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendReadBlobResponse(deviceId_t deviceId, attReadBlobResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendReadMultipleRequest(deviceId_t deviceId, attReadMultipleRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendReadMultipleResponse(deviceId_t deviceId, attReadMultipleResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendReadByGroupTypeRequest(deviceId_t deviceId, attReadByGroupTypeRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendReadByGroupTypeResponse(deviceId_t deviceId, attReadByGroupTypeResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendWriteRequest(deviceId_t deviceId, attWriteRequestAndCommandParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendWriteResponse(deviceId_t deviceId);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendWriteCommand(deviceId_t deviceId, attWriteRequestAndCommandParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendSignedWriteCommand(deviceId_t deviceId, attSignedWriteCommandParams_t *pParams);

/*! *********************************************************************************
 * \brief]
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendPrepareWriteRequest(deviceId_t deviceId, attPrepareWriteRequestResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendPrepareWriteResponse(deviceId_t deviceId, attPrepareWriteRequestResponseParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendExecuteWriteRequest(deviceId_t deviceId, attExecuteWriteRequestParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendExecuteWriteResponse(deviceId_t deviceId);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendHandleValueNotification(deviceId_t deviceId,
                                                  attHandleValueNotificationIndicationParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttServer_SendHandleValueIndication(deviceId_t deviceId,
                                                attHandleValueNotificationIndicationParams_t *pParams);

/*! *********************************************************************************
 * \brief
 *
 * \return
 *
 * \pre
 *
 * \remarks
 *
 ********************************************************************************** */
bleResult_t AttClient_SendHandleValueConfirmation(deviceId_t deviceId);

#ifdef __cplusplus
}
#endif

#endif /* _ATT_INTERFACE_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
