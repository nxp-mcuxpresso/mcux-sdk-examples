/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause.
 */

#ifndef _FSCI_BLE_L2CAP_TYPES_H
#define _FSCI_BLE_L2CAP_TYPES_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble.h"
#include "l2ca_interface.h"
#include "l2ca_cb_interface.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

#define fsciBleL2capGetLeCbConnectionRequestBufferSize(pLeCbConnectionRequest)                                 \
    (fsciBleGetDeviceIdBufferSize(&(pLeCbConnectionRequest)->deviceId) + sizeof(uint16_t) + sizeof(uint16_t) + \
     sizeof(uint16_t) + sizeof(uint16_t))

#define fsciBleL2capGetLeCbConnectionCompleteBufferSize(pLeCbConnectionComplete)                                \
    (fsciBleGetDeviceIdBufferSize(&(pLeCbConnectionComplete)->deviceId) + sizeof(uint16_t) + sizeof(uint16_t) + \
     sizeof(uint16_t) + sizeof(uint16_t) + sizeof(l2caLeCbConnectionRequestResult_t))

#define fsciBleL2capGetLeCbDisconnectionBufferSize(pLeCbDisconnection) \
    (fsciBleGetDeviceIdBufferSize(&(pLeCbDisconnection)->deviceId) + sizeof(uint16_t))

#define fsciBleL2capGetLeCbNoPeerCreditsBufferSize(pLeCbNoPeerCredits) \
    (fsciBleGetDeviceIdBufferSize(&(pLeCbNoPeerCredits)->deviceId) + sizeof(uint16_t))

#define fsciBleL2capGetLeCbLocalCreditsNotificationBufferSize(pLeCbLocalCreditsNotification) \
    (fsciBleGetDeviceIdBufferSize(&(pLeCbLocalCreditsNotification)->deviceId) + sizeof(uint16_t) + sizeof(uint16_t))

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

void fsciBleL2capGetLeCbConnectionRequestFromBuffer(l2caLeCbConnectionRequest_t *pLeCbConnectionRequest,
                                                    uint8_t **ppBuffer);

void fsciBleL2capGetBufferFromLeCbConnectionRequest(l2caLeCbConnectionRequest_t *pLeCbConnectionRequest,
                                                    uint8_t **ppBuffer);

void fsciBleL2capGetLeCbConnectionCompleteFromBuffer(l2caLeCbConnectionComplete_t *pLeCbConnectionComplete,
                                                     uint8_t **ppBuffer);

void fsciBleL2capGetBufferFromLeCbConnectionComplete(l2caLeCbConnectionComplete_t *pLeCbConnectionComplete,
                                                     uint8_t **ppBuffer);

void fsciBleL2capGetLeCbDisconnectionFromBuffer(l2caLeCbDisconnection_t *pLeCbDisconnection, uint8_t **ppBuffer);

void fsciBleL2capGetBufferFromLeCbDisconnection(l2caLeCbDisconnection_t *pLeCbDisconnection, uint8_t **ppBuffer);

void fsciBleL2capGetLeCbNoPeerCreditsFromBuffer(l2caLeCbNoPeerCredits_t *pLeCbNoPeerCredits, uint8_t **ppBuffer);

void fsciBleL2capGetBufferFromLeCbNoPeerCredits(l2caLeCbNoPeerCredits_t *pLeCbNoPeerCredits, uint8_t **ppBuffer);

void fsciBleL2capGetLeCbLocalCreditsNotificationFromBuffer(
    l2caLeCbLocalCreditsNotification_t *pLeCbLocalCreditsNotification, uint8_t **ppBuffer);

void fsciBleL2capGetBufferFromLeCbLocalCreditsNotification(
    l2caLeCbLocalCreditsNotification_t *pLeCbLocalCreditsNotification, uint8_t **ppBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_L2CAP_TYPES_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
