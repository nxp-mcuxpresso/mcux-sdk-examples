/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_BLE_GATT_TYPES_H
#define _FSCI_BLE_GATT_TYPES_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble.h"
#include "Panic.h"
#include "gatt_types.h"
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

#define fsciBleGattClientGetAttributeBufferSize(pAttribute)                                                           \
    (sizeof(uint16_t) + sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize((pAttribute)->uuidType) + sizeof(uint16_t) + \
     sizeof(uint16_t) + (pAttribute)->valueLength)

#define fsciBleGattClientEraseTmpInfo(bAllocated) fsciBleGattClientErasePermanentOrTmpInfo((bAllocated), TRUE)

#define fsciBleGattClientEraseInfo(bAllocated) fsciBleGattClientErasePermanentOrTmpInfo((bAllocated), FALSE)

#define fsciBleGattServerGetCharacteristicBufferSize(pCharacteristic) \
    fsciBleGattClientGetCharacteristicBufferSize(pCharacteristic)

#define fsciBleGattServerGetCharacteristicFromBuffer(pCharacteristic, ppBuffer) \
    fsciBleGattClientGetCharacteristicFromBuffer((pCharacteristic), (ppBuffer))

#define fsciBleGattServerGetBufferFromCharacteristic(pCharacteristic, ppBuffer) \
    fsciBleGattClientGetBufferFromCharacteristic((pCharacteristic), (ppBuffer))

#define fsciBleGattServerGetServerMtuChangedEventBufferSize(pServerMtuChangedEvent) sizeof(uint16_t)

#define fsciBleGattServerGetServerMtuChangedEventFromBuffer(pServerMtuChangedEvent, ppBuffer) \
    fsciBleGetUint16ValueFromBuffer((pServerMtuChangedEvent)->newMtu, *(ppBuffer))

#define fsciBleGattServerGetBufferFromServerMtuChangedEvent(pServerMtuChangedEvent, ppBuffer) \
    fsciBleGetBufferFromUint16Value((pServerMtuChangedEvent)->newMtu, *(ppBuffer))

#define fsciBleGattServerGetServerAttributeWrittenEventBufferSize(pServerAttributeWrittenEvent) \
    (sizeof(uint16_t) + sizeof(uint16_t) + (pServerAttributeWrittenEvent)->cValueLength)

#define fsciBleGattServerGetServerLongCharacteristicWrittenEventBufferSize(pServerLongCharacteristicWrittenEvent) \
    (sizeof(uint16_t) + sizeof(uint16_t) + (pServerLongCharacteristicWrittenEvent)->cValueLength)

#define fsciBleGattServerGetServerLongCharacteristicWrittenEventBufferSize(pServerLongCharacteristicWrittenEvent) \
    (sizeof(uint16_t) + sizeof(uint16_t) + (pServerLongCharacteristicWrittenEvent)->cValueLength)

#define fsciBleGattServerGetServerCccdWrittenEventBufferSize(pServerCccdWrittenEvent) \
    (sizeof(uint16_t) + sizeof(gattCccdFlags_t))

#define fsciBleGattServerGetServerAttributeReadEventBufferSize(pServerAttributeReadEvent) sizeof(uint16_t)

#define fsciBleGattServerGetServerAttributeReadEventFromBuffer(pServerAttributeReadEvent, ppBuffer) \
    fsciBleGetUint16ValueFromBuffer((pServerAttributeReadEvent)->handle, *(ppBuffer))

#define fsciBleGattServerGetBufferFromServerAttributeReadEvent(pServerAttributeReadEvent, ppBuffer) \
    fsciBleGetBufferFromUint16Value((pServerAttributeReadEvent)->handle, *(ppBuffer))

#define fsciBleGattServerGetServerProcedureErrorBufferSize(pServerProcedureError) \
    (sizeof(gattServerProcedureType_t) + sizeof(bleResult_t))

#define fsciBleGattServerFreeServerEvent(pServerEvent) MEM_BufferFree(pServerEvent)

#define fsciBleGattGetHandleRangeBufferSize(pHandleRange) (sizeof(uint16_t) + sizeof(uint16_t))

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

uint16_t *fsciBleGattClientAllocOutOrIoArraySize(void);

uint8_t *fsciBleGattClientAllocOutOrIoValue(uint16_t maxValueLength);

gattAttribute_t *fsciBleGattClientAllocOutOrIoAttributes(uint8_t maxNbOfAtributes);

void fsciBleGattClientGetAttributeFromBuffer(gattAttribute_t *pAttribute, uint8_t **ppBuffer);

void fsciBleGattClientGetBufferFromAttribute(gattAttribute_t *pAttribute, uint8_t **ppBuffer);

gattCharacteristic_t *fsciBleGattClientAllocOutOrIoCharacteristics(uint8_t maxNbOfCharacteristics);

uint16_t fsciBleGattClientGetCharacteristicBufferSize(gattCharacteristic_t *pCharacteristic);

void fsciBleGattClientGetCharacteristicFromBuffer(gattCharacteristic_t *pCharacteristic, uint8_t **ppBuffer);

void fsciBleGattClientGetBufferFromCharacteristic(gattCharacteristic_t *pCharacteristic, uint8_t **ppBuffer);

gattService_t *fsciBleGattClientAllocOutOrIoIncludedServices(uint8_t maxNbOfIncludedServices);

gattService_t *fsciBleGattClientAllocOutOrIoServices(uint8_t maxNbOfServices);

uint16_t fsciBleGattClientGetServiceBufferSize(gattService_t *pService);

void fsciBleGattClientGetServiceFromBuffer(gattService_t *pService, uint8_t **ppBuffer);

void fsciBleGattClientGetBufferFromService(gattService_t *pService, uint8_t **ppBuffer);

void fsciBleGattClientSaveServicesInfo(deviceId_t deviceId, gattService_t *pServices);

void fsciBleGattClientSaveIncludedServicesInfo(deviceId_t deviceId, gattService_t *pIncludedServices);

void fsciBleGattClientSaveCharacteristicsInfo(deviceId_t deviceId, gattCharacteristic_t *pCharacteristics);

void fsciBleGattClientSaveDescriptorsInfo(deviceId_t deviceId, gattAttribute_t *pDescriptors);

void fsciBleGattClientSaveValueInfo(deviceId_t deviceId, uint8_t *pValue);

void fsciBleGattClientSaveArraySizeInfo(deviceId_t deviceId, uint16_t *pArraySize);

gattService_t *fsciBleGattClientGetServicesInfo(deviceId_t deviceId, bool_t bAllocated);

gattService_t *fsciBleGattClientGetIncludedServicesInfo(deviceId_t deviceId, bool_t bAllocated);

gattCharacteristic_t *fsciBleGattClientGetCharacteristicsInfo(deviceId_t deviceId, bool_t bAllocated);

gattAttribute_t *fsciBleGattClientGetDescriptorsInfo(deviceId_t deviceId, bool_t bAllocated);

uint8_t *fsciBleGattClientGetValueInfo(deviceId_t deviceId, bool_t bAllocated);

uint16_t *fsciBleGattClientGetArraySizeInfo(deviceId_t deviceId, bool_t bAllocated);

void fsciBleGattClientKeepInfo(bool_t bAllocatedInfo);

void fsciBleGattClientErasePermanentOrTmpInfo(bool_t bAllocatedInfo, bool_t bTmp);

void fsciBleGattServerGetServerAttributeWrittenEventFromBuffer(
    gattServerAttributeWrittenEvent_t *pServerAttributeWrittenEvent, uint8_t **ppBuffer);

void fsciBleGattServerGetBufferFromServerAttributeWrittenEvent(
    gattServerAttributeWrittenEvent_t *pServerAttributeWrittenEvent, uint8_t **ppBuffer);

void fsciBleGattServerGetServerLongCharacteristicWrittenEventFromBuffer(
    gattServerLongCharacteristicWrittenEvent_t *pServerLongCharacteristicWrittenEvent, uint8_t **ppBuffer);

void fsciBleGattServerGetBufferFromServerLongCharacteristicWrittenEvent(
    gattServerLongCharacteristicWrittenEvent_t *pServerLongCharacteristicWrittenEvent, uint8_t **ppBuffer);

void fsciBleGattServerGetServerCccdWrittenEventFromBuffer(gattServerCccdWrittenEvent_t *pServerCccdWrittenEvent,
                                                          uint8_t **ppBuffer);

void fsciBleGattServerGetBufferFromServerCccdWrittenEvent(gattServerCccdWrittenEvent_t *pServerCccdWrittenEvent,
                                                          uint8_t **ppBuffer);

void fsciBleGattServerGetServerProcedureErrorFromBuffer(gattServerProcedureError_t *pServerProcedureError,
                                                        uint8_t **ppBuffer);

void fsciBleGattServerGetBufferFromServerProcedureError(gattServerProcedureError_t *pServerProcedureError,
                                                        uint8_t **ppBuffer);

gattServerEvent_t *fsciBleGattServerAllocServerEventForBuffer(gattServerEventType_t eventType, uint8_t *pBuffer);

uint16_t fsciBleGattServerGetServerEventBufferSize(gattServerEvent_t *pServerEvent);

void fsciBleGattServerGetServerEventFromBuffer(gattServerEvent_t *pServerEvent, uint8_t **ppBuffer);

void fsciBleGattServerGetBufferFromServerEvent(gattServerEvent_t *pServerEvent, uint8_t **ppBuffer);

void fsciBleGattGetHandleRangeFromBuffer(gattHandleRange_t *pHandleRange, uint8_t **ppBuffer);

void fsciBleGattGetBufferFromHandleRange(gattHandleRange_t *pHandleRange, uint8_t **ppBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_GATT_TYPES_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
