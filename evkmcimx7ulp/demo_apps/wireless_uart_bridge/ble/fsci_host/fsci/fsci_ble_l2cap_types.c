/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_l2cap_types.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleL2capGetLeCbConnectionRequestFromBuffer(l2caLeCbConnectionRequest_t *pLeCbConnectionRequest,
                                                    uint8_t **ppBuffer)
{
    /* Read l2caLeCbConnectionRequest_t structure fields from buffer */
    fsciBleGetDeviceIdFromBuffer(&pLeCbConnectionRequest->deviceId, ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionRequest->lePsm, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionRequest->peerMtu, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionRequest->peerMps, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionRequest->initialCredits, *ppBuffer);
}

void fsciBleL2capGetBufferFromLeCbConnectionRequest(l2caLeCbConnectionRequest_t *pLeCbConnectionRequest,
                                                    uint8_t **ppBuffer)
{
    /* Write l2caLeCbConnectionRequest_t structure fields in buffer */
    fsciBleGetBufferFromDeviceId(&pLeCbConnectionRequest->deviceId, ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionRequest->lePsm, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionRequest->peerMtu, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionRequest->peerMps, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionRequest->initialCredits, *ppBuffer);
}

void fsciBleL2capGetLeCbConnectionCompleteFromBuffer(l2caLeCbConnectionComplete_t *pLeCbConnectionComplete,
                                                     uint8_t **ppBuffer)
{
    /* Read l2caLeCbConnectionComplete_t structure fields from buffer */
    fsciBleGetDeviceIdFromBuffer(&pLeCbConnectionComplete->deviceId, ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionComplete->cId, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionComplete->peerMtu, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionComplete->peerMps, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbConnectionComplete->initialCredits, *ppBuffer);
    fsciBleGetEnumValueFromBuffer(pLeCbConnectionComplete->result, *ppBuffer, l2caLeCbConnectionRequestResult_t);
}

void fsciBleL2capGetBufferFromLeCbConnectionComplete(l2caLeCbConnectionComplete_t *pLeCbConnectionComplete,
                                                     uint8_t **ppBuffer)
{
    /* Write l2caLeCbConnectionComplete_t structure fields in buffer */
    fsciBleGetBufferFromDeviceId(&pLeCbConnectionComplete->deviceId, ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionComplete->cId, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionComplete->peerMtu, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionComplete->peerMps, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbConnectionComplete->initialCredits, *ppBuffer);
    fsciBleGetBufferFromEnumValue(pLeCbConnectionComplete->result, *ppBuffer, l2caLeCbConnectionRequestResult_t);
}

void fsciBleL2capGetLeCbDisconnectionFromBuffer(l2caLeCbDisconnection_t *pLeCbDisconnection, uint8_t **ppBuffer)
{
    /* Read l2caLeCbDisconnection_t structure fields from buffer */
    fsciBleGetDeviceIdFromBuffer(&pLeCbDisconnection->deviceId, ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbDisconnection->cId, *ppBuffer);
}

void fsciBleL2capGetBufferFromLeCbDisconnection(l2caLeCbDisconnection_t *pLeCbDisconnection, uint8_t **ppBuffer)
{
    /* Write l2caLeCbDisconnection_t structure fields in buffer */
    fsciBleGetBufferFromDeviceId(&pLeCbDisconnection->deviceId, ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbDisconnection->cId, *ppBuffer);
}

void fsciBleL2capGetLeCbNoPeerCreditsFromBuffer(l2caLeCbNoPeerCredits_t *pLeCbNoPeerCredits, uint8_t **ppBuffer)
{
    /* Read l2caLeCbNoPeerCredits_t structure fields from buffer */
    fsciBleGetDeviceIdFromBuffer(&pLeCbNoPeerCredits->deviceId, ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbNoPeerCredits->cId, *ppBuffer);
}

void fsciBleL2capGetBufferFromLeCbNoPeerCredits(l2caLeCbNoPeerCredits_t *pLeCbNoPeerCredits, uint8_t **ppBuffer)
{
    fsciBleGetBufferFromDeviceId(&pLeCbNoPeerCredits->deviceId, ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbNoPeerCredits->cId, *ppBuffer);
}

void fsciBleL2capGetLeCbLocalCreditsNotificationFromBuffer(
    l2caLeCbLocalCreditsNotification_t *pLeCbLocalCreditsNotification, uint8_t **ppBuffer)
{
    /* Read l2caLeCbLocalCreditsNotification_t structure fields from buffer */
    fsciBleGetDeviceIdFromBuffer(&pLeCbLocalCreditsNotification->deviceId, ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbLocalCreditsNotification->cId, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pLeCbLocalCreditsNotification->localCredits, *ppBuffer);
}

void fsciBleL2capGetBufferFromLeCbLocalCreditsNotification(
    l2caLeCbLocalCreditsNotification_t *pLeCbLocalCreditsNotification, uint8_t **ppBuffer)
{
    /* Write l2caLeCbLocalCreditsNotification_t structure fields in buffer */
    fsciBleGetBufferFromDeviceId(&pLeCbLocalCreditsNotification->deviceId, ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbLocalCreditsNotification->cId, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pLeCbLocalCreditsNotification->localCredits, *ppBuffer);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
