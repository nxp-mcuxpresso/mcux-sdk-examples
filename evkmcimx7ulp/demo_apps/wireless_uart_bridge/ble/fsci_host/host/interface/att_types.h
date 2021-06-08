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

#ifndef _ATT_TYPES_H_
#define _ATT_TYPES_H_

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "att_errors.h"

/************************************************************************************
*************************************************************************************
* Public constants and macros
*************************************************************************************
************************************************************************************/

#define gAttOpcodeSize_d 1
#define gAttHandleSize_d 2
#define gAttOffsetSize_d 2

/* Macros used to compute maximum data size for various ATT payloads */
#define gAttMaxDataSize_d(mtu)          (mtu - gAttOpcodeSize_d)                         /* 22 */
#define gAttMaxReadDataSize_d(mtu)      (gAttMaxDataSize_d(mtu))                         /* 22 */
#define gAttMaxWriteDataSize_d(mtu)     (gAttMaxDataSize_d(mtu) - gAttHandleSize_d)      /* 20 */
#define gAttMaxNotifIndDataSize_d(mtu)  (gAttMaxWriteDataSize_d(mtu))                    /* 20 */
#define gAttMaxPrepWriteDataSize_d(mtu) (gAttMaxWriteDataSize_d(mtu) - gAttOffsetSize_d) /* 18 */

#define gAttAuthSignatureSize 12

#define gAttTimeoutSeconds_c 30

#define isTimedRequestOpcode(opcode)                                                              \
    (opcode == gAttOpcodeExchangeMtuRequest_c || opcode == gAttOpcodeFindInformationRequest_c ||  \
     opcode == gAttOpcodeFindByTypeValueRequest_c || opcode == gAttOpcodeReadByTypeRequest_c ||   \
     opcode == gAttOpcodeReadRequest_c || opcode == gAttOpcodeReadBlobRequest_c ||                \
     opcode == gAttOpcodeReadMultipleRequest_c || opcode == gAttOpcodeReadByGroupTypeRequest_c || \
     opcode == gAttOpcodeWriteRequest_c || opcode == gAttOpcodePrepareWriteRequest_c ||           \
     opcode == gAttOpcodeExecuteWriteRequest_c || opcode == gAttOpcodeHandleValueIndication_c)

/* Based on the fact that corresponding request and response opcodes are consecutive */
#define responseOpcodeOfRequestOpcode(opcode) ((attOpcode_t)((uint8_t)opcode + 1))
#define requestOpcodeOfResponseOpcode(opcode) ((attOpcode_t)((uint8_t)opcode - 1))

#define isTimedResponseOpcode(opcode) isTimedRequestOpcode(requestOpcodeOfResponseOpcode(opcode))

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
    /* Error Handling */
    gAttOpcodeErrorResponse_c = 0x01,
    /* MTU Exchange */
    gAttOpcodeExchangeMtuRequest_c  = 0x02,
    gAttOpcodeExchangeMtuResponse_c = 0x03,
    /* Find Information */
    gAttOpcodeFindInformationRequest_c  = 0x04,
    gAttOpcodeFindInformationResponse_c = 0x05,
    gAttOpcodeFindByTypeValueRequest_c  = 0x06,
    gAttOpcodeFindByTypeValueResponse_c = 0x07,
    /* Reading Attributes */
    gAttOpcodeReadByTypeRequest_c       = 0x08,
    gAttOpcodeReadByTypeResponse_c      = 0x09,
    gAttOpcodeReadRequest_c             = 0x0A,
    gAttOpcodeReadResponse_c            = 0x0B,
    gAttOpcodeReadBlobRequest_c         = 0x0C,
    gAttOpcodeReadBlobResponse_c        = 0x0D,
    gAttOpcodeReadMultipleRequest_c     = 0x0E,
    gAttOpcodeReadMultipleResponse_c    = 0x0F,
    gAttOpcodeReadByGroupTypeRequest_c  = 0x10,
    gAttOpcodeReadByGroupTypeResponse_c = 0x11,
    /* Writing Attributes */
    gAttOpcodeWriteRequest_c       = 0x12,
    gAttOpcodeWriteResponse_c      = 0x13,
    gAttOpcodeWriteCommand_c       = 0x52,
    gAttOpcodeSignedWriteCommand_c = 0xD2,
    /* Queued Writes */
    gAttOpcodePrepareWriteRequest_c  = 0x16,
    gAttOpcodePrepareWriteResponse_c = 0x17,
    gAttOpcodeExecuteWriteRequest_c  = 0x18,
    gAttOpcodeExecuteWriteResponse_c = 0x19,
    /* Server Initiated */
    gAttOpcodeHandleValueNotification_c = 0x1B,
    gAttOpcodeHandleValueIndication_c   = 0x1D,
    gAttOpcodeHandleValueConfirmation_c = 0x1E,

    gAttLastOpcode_c = 0xE0 /* testing purposes */
} attOpcode_t;

typedef enum
{
    gAttUuid16BitFormat_c  = 0x01,
    gAttUuid128BitFormat_c = 0x02,
} attUuidFormat_t;

typedef enum
{
    gAttExecuteWriteRequestCancel      = 0x00,
    gAttExecuteWriteRequestImmediately = 0x01,
} attExecuteWriteRequestFlags_t;

#endif /* _ATT_TYPES_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
