/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_BLE_TYPES_H
#define _FSCI_BLE_TYPES_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "ble_general.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

#define fsciBleGetBoolValueFromBuffer(value, pBuff) \
    (value) = (*(pBuff) == 0) ? FALSE : TRUE;       \
    (pBuff)++

#define fsciBleGetBufferFromBoolValue(value, pBuff) \
    *(pBuff) = ((value) == FALSE) ? 0 : 1;          \
    (pBuff)++

#define fsciBleGetUint8ValueFromBuffer(value, pBuff) \
    (value) = *(pBuff);                              \
    (pBuff)++

#define fsciBleGetBufferFromUint8Value(value, pBuff) \
    *(pBuff) = (value);                              \
    (pBuff)++

#define fsciBleGetUint16ValueFromBuffer(value, pBuff) \
    (value) = Utils_ExtractTwoByteValue((pBuff));     \
    (pBuff) += 2

#define fsciBleGetBufferFromUint16Value(value, pBuff) \
    Utils_PackTwoByteValue((value), (pBuff));         \
    (pBuff) += 2

#define fsciBleGetUint32ValueFromBuffer(value, pBuff) \
    (value) = Utils_ExtractFourByteValue((pBuff));    \
    (pBuff) += 4

#define fsciBleGetBufferFromUint32Value(value, pBuff) \
    Utils_PackFourByteValue((value), (pBuff));        \
    (pBuff) += 4

#define fsciBleGetUint128ValueFromBuffer(pValue, pBuff) fsciBleGetArrayFromBuffer((pValue), (pBuff), 16)

#define fsciBleGetBufferFromUint128Value(pValue, pBuff) fsciBleGetBufferFromArray((pValue), (pBuff), 16)

#define fsciBleGetArrayFromBuffer(pArray, pBuff, nbOfBytes) \
    FLib_MemCpy((pArray), (pBuff), (nbOfBytes));            \
    (pBuff) += (nbOfBytes)

#define fsciBleGetBufferFromArray(pArray, pBuff, nbOfBytes) \
    FLib_MemCpy((pBuff), (pArray), (nbOfBytes));            \
    (pBuff) += (nbOfBytes)

#define fsciBleGetEnumValueFromBuffer(value, pBuff, enumType) \
    fsciBleGetArrayFromBuffer((uint8_t *)&(value), (pBuff), sizeof(enumType))

#define fsciBleGetBufferFromEnumValue(value, pBuff, enumType) \
    fsciBleGetBufferFromArray((uint8_t *)&(value), (pBuff), sizeof(enumType))

#define fsciBleGetAddressFromBuffer(pAddr, pBuff) fsciBleGetArrayFromBuffer((pAddr), (pBuff), gcBleDeviceAddressSize_c)

#define fsciBleGetBufferFromAddress(pAddr, pBuff) fsciBleGetBufferFromArray((pAddr), (pBuff), gcBleDeviceAddressSize_c)

#define fsciBleGetIrkFromBuffer(pAddr, pBuff) fsciBleGetArrayFromBuffer((pAddr), (pBuff), gcSmpIrkSize_c)

#define fsciBleGetBufferFromIrk(pAddr, pBuff) fsciBleGetBufferFromArray((pAddr), (pBuff), gcSmpIrkSize_c)

#define fsciBleGetCsrkFromBuffer(pAddr, pBuff) fsciBleGetArrayFromBuffer((pAddr), (pBuff), gcSmpCsrkSize_c)

#define fsciBleGetBufferFromCsrk(pAddr, pBuff) fsciBleGetBufferFromArray((pAddr), (pBuff), gcSmpCsrkSize_c)

#define fsciBleGetOobFromBuffer(pAddr, pBuff) fsciBleGetArrayFromBuffer((pAddr), (pBuff), gcSmpOobSize_c)

#define fsciBleGetBufferFromOob(pAddr, pBuff) fsciBleGetBufferFromArray((pAddr), (pBuff), gcSmpOobSize_c)

#define fsciBleGetDeviceIdBufferSize(pDeviceId) sizeof(uint8_t)

#define fsciBleGetDeviceIdFromBuffer(pDeviceId, ppBuff) fsciBleGetUint8ValueFromBuffer(*(pDeviceId), *(ppBuff))

#define fsciBleGetBufferFromDeviceId(pDeviceId, ppBuff) fsciBleGetBufferFromUint8Value(*(pDeviceId), *(ppBuff))

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

uint16_t fsciBleGetUuidBufferSize(bleUuidType_t uuidType);

void fsciBleGetUuidFromBuffer(bleUuid_t *pUuid, uint8_t **ppBuffer, bleUuidType_t uuidType);

void fsciBleGetBufferFromUuid(bleUuid_t *pUuid, uint8_t **ppBuffer, bleUuidType_t uuidType);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_TYPES_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
