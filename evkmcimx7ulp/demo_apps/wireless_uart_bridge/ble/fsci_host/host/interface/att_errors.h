/*! *********************************************************************************
 * \addtogroup GATT
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

#ifndef _ATT_ERRORS_H_
#define _ATT_ERRORS_H_

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! ATT error codes */
typedef enum
{
    gAttErrCodeNoError_c                       = 0x00,
    gAttErrCodeInvalidHandle_c                 = 0x01,
    gAttErrCodeReadNotPermitted_c              = 0x02,
    gAttErrCodeWriteNotPermitted_c             = 0x03,
    gAttErrCodeInvalidPdu_c                    = 0x04,
    gAttErrCodeInsufficientAuthentication_c    = 0x05,
    gAttErrCodeRequestNotSupported_c           = 0x06,
    gAttErrCodeInvalidOffset_c                 = 0x07,
    gAttErrCodeInsufficientAuthorization_c     = 0x08,
    gAttErrCodePrepareQueueFull_c              = 0x09,
    gAttErrCodeAttributeNotFound_c             = 0x0A,
    gAttErrCodeAttributeNotLong_c              = 0x0B,
    gAttErrCodeInsufficientEncryptionKeySize_c = 0x0C,
    gAttErrCodeInvalidAttributeValueLength_c   = 0x0D,
    gAttErrCodeUnlikelyError_c                 = 0x0E,
    gAttErrCodeInsufficientEncryption_c        = 0x0F,
    gAttErrCodeUnsupportedGroupType_c          = 0x10,
    gAttErrCodeInsufficientResources_c         = 0x11,
    /* Reserved Error Opcodes                          = 0x12 - 0x7F */
    /* Application Error Opcodes                       = 0x80 - 0x9F */
    /* Reserved Error Opcodes                          = 0xA0 - 0xDF */
    /* Common Profile And Service Error Opcodes        = 0xE0 - 0xFF */
} attErrorCode_t;

/*! Prepare Write Request Parameters Structure used by external reference */
typedef struct attPrepareWriteRequestParams_tag
{
    uint16_t attributeHandle;
    uint16_t valueOffset;
    uint8_t attributeValue[gAttMaxMtu_c - 5];
    uint16_t attributeLength;
} attPrepareWriteRequestParams_t;

#endif /* _ATT_ERRORS_H_ */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
