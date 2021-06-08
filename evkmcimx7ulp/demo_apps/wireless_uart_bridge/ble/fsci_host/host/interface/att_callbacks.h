/*! *********************************************************************************
 * \addtogroup ATT
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

#ifndef _ATT_CALLBACKS_H_
#define _ATT_CALLBACKS_H_

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef void (*attIncomingServerErrorResponseCallback_t)(deviceId_t deviceId, attErrorResponseParams_t *pParams);

typedef void (*attIncomingClientExchangeMtuRequestCallback_t)(deviceId_t deviceId,
                                                              attExchangeMtuRequestParams_t *pParams);

typedef void (*attIncomingServerExchangeMtuResponseCallback_t)(deviceId_t deviceId,
                                                               attExchangeMtuResponseParams_t *pParams);

typedef void (*attIncomingClientFindInformationRequestCallback_t)(deviceId_t deviceId,
                                                                  attFindInformationRequestParams_t *pParams);

typedef void (*attIncomingServerFindInformationResponseCallback_t)(deviceId_t deviceId,
                                                                   attFindInformationResponseParams_t *pParams);

typedef void (*attIncomingClientFindByTypeValueRequestCallback_t)(deviceId_t deviceId,
                                                                  attFindByTypeValueRequestParams_t *pParams);

typedef void (*attIncomingServerFindByTypeValueResponseCallback_t)(deviceId_t deviceId,
                                                                   attFindByTypeValueResponseParams_t *pParams);

typedef void (*attIncomingClientReadByTypeRequestCallback_t)(deviceId_t deviceId,
                                                             attReadByTypeRequestParams_t *pParams);

typedef void (*attIncomingServerReadByTypeResponseCallback_t)(deviceId_t deviceId,
                                                              attReadByTypeResponseParams_t *pParams);

typedef void (*attIncomingClientReadRequestCallback_t)(deviceId_t deviceId, attReadRequestParams_t *pParams);

typedef void (*attIncomingServerReadResponseCallback_t)(deviceId_t deviceId, attReadResponseParams_t *pParams);

typedef void (*attIncomingClientReadBlobRequestCallback_t)(deviceId_t deviceId, attReadBlobRequestParams_t *pParams);

typedef void (*attIncomingServerReadBlobResponseCallback_t)(deviceId_t deviceId, attReadBlobResponseParams_t *pParams);

typedef void (*attIncomingClientReadMultipleRequestCallback_t)(deviceId_t deviceId,
                                                               attReadMultipleRequestParams_t *pParams);

typedef void (*attIncomingServerReadMultipleResponseCallback_t)(deviceId_t deviceId,
                                                                attReadMultipleResponseParams_t *pParams);

typedef void (*attIncomingClientReadByGroupTypeRequestCallback_t)(deviceId_t deviceId,
                                                                  attReadByGroupTypeRequestParams_t *pParams);

typedef void (*attIncomingServerReadByGroupTypeResponseCallback_t)(deviceId_t deviceId,
                                                                   attReadByGroupTypeResponseParams_t *pParams);

typedef void (*attIncomingClientWriteRequestCallback_t)(deviceId_t deviceId,
                                                        attWriteRequestAndCommandParams_t *pParams);

typedef void (*attIncomingServerWriteResponseCallback_t)(deviceId_t deviceId);

typedef void (*attIncomingClientWriteCommandCallback_t)(deviceId_t deviceId,
                                                        attWriteRequestAndCommandParams_t *pParams);

typedef void (*attIncomingClientSignedWriteCommandCallback_t)(deviceId_t deviceId,
                                                              attSignedWriteCommandParams_t *pParams);

typedef void (*attIncomingClientPrepareWriteRequestCallback_t)(deviceId_t deviceId,
                                                               attPrepareWriteRequestResponseParams_t *pParams);

typedef void (*attIncomingServerPrepareWriteResponseCallback_t)(deviceId_t deviceId,
                                                                attPrepareWriteRequestResponseParams_t *pParams);

typedef void (*attIncomingClientExecuteWriteRequestCallback_t)(deviceId_t deviceId,
                                                               attExecuteWriteRequestParams_t *pParams);

typedef void (*attIncomingServerExecuteWriteResponseCallback_t)(deviceId_t deviceId);

typedef void (*attIncomingServerHandleValueNotificationCallback_t)(
    deviceId_t deviceId, attHandleValueNotificationIndicationParams_t *pParams);

typedef void (*attIncomingServerHandleValueIndicationCallback_t)(deviceId_t deviceId,
                                                                 attHandleValueNotificationIndicationParams_t *pParams);

typedef void (*attIncomingClientHandleValueConfirmationCallback_t)(deviceId_t deviceId);

typedef void (*attUnsupportedOpcodeCallback_t)(deviceId_t deviceId, uint8_t opcode);

typedef void (*attTimeoutCallback_t)(deviceId_t deviceId);

#endif /* _ATT_CALLBACKS_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
