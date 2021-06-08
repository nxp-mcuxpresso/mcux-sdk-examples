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

#include "fsci_ble_gatt_types.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define fsciBleGattClientHandleNoMemoryStatus()      fsciBleGattClientHandleErrorStatus(gFsciOutOfMessages_c)
#define fsciBleGattClientHandleInternalErrorStatus() fsciBleGattClientHandleErrorStatus(gFsciError_c)

#ifndef mFsciGapMaximumActiveConnections_c
#define mFsciGapMaximumActiveConnections_c 2
#endif

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/* Structure needed by GattClient for keeping information used
by asynchronous procedures */
typedef struct fsciBleGattClientManagementInfo_tag
{
    gattService_t *pServices;
    gattService_t *pIncludedServices;
    gattCharacteristic_t *pCharacteristics;
    gattAttribute_t *pDescriptors;
    uint8_t *pValue;
    uint16_t *pArraySize;
} fsciBleGattClientManagementInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static fsciBleGattClientManagementInfo_t fsciBleGattClientAllocatedManagementInfo = {0}; /* Keeps information about the
                                                                                            allocated management info */
static fsciBleGattClientManagementInfo_t fsciBleGattClientTmpAllocatedManagementInfo = {
    0}; /* Keeps information about the temporary
           allocated management info */

static fsciBleGattClientManagementInfo_t fsciBleGattClientSavedManagementInfo[mFsciGapMaximumActiveConnections_c];

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

static void fsciBleGattClientHandleErrorStatus(gFsciStatus_t status);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

uint16_t *fsciBleGattClientAllocOutOrIoArraySize(void)
{
    /* Verify if an array size is already allocated (only one kept array size can
    be allocated at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pArraySize)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the array size */
    fsciBleGattClientTmpAllocatedManagementInfo.pArraySize = MEM_BufferAlloc(sizeof(uint16_t));

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pArraySize)
    {
        fsciBleGattClientHandleNoMemoryStatus();
    }

    /* Return the allocated kept array size */
    return fsciBleGattClientTmpAllocatedManagementInfo.pArraySize;
}

uint8_t *fsciBleGattClientAllocOutOrIoValue(uint16_t maxValueLength)
{
    /* Verify if a value is already allocated (only one kept value can be allocated
    at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pValue)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the value */
    fsciBleGattClientTmpAllocatedManagementInfo.pValue = MEM_BufferAlloc(maxValueLength);

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pValue)
    {
        fsciBleGattClientHandleNoMemoryStatus();
    }

    /* Return the allocated kept value */
    return fsciBleGattClientTmpAllocatedManagementInfo.pValue;
}

gattAttribute_t *fsciBleGattClientAllocOutOrIoAttributes(uint8_t maxNbOfAtributes)
{
    uint32_t iCount;

    /* Verify if a descriptors array is already allocated (only one kept descriptors array
    can be allocated at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the descriptors array */
    fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors =
        MEM_BufferAlloc(maxNbOfAtributes * sizeof(gattAttribute_t));

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors)
    {
        fsciBleGattClientHandleNoMemoryStatus();
        return NULL;
    }

    /* For every descriptor in the array, set the paValue field to NULL (if paValue is not NULL,
    a different buffer must be separately allocated for it using fsciBleGattClientAllocOutOrIoValue function) */
    for (iCount = 0; iCount < maxNbOfAtributes; iCount++)
    {
        fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors[iCount].valueLength = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors[iCount].paValue     = NULL;
    }

    /* Return the allocated kept descriptors array */
    return fsciBleGattClientTmpAllocatedManagementInfo.pDescriptors;
}

void fsciBleGattClientGetAttributeFromBuffer(gattAttribute_t *pAttribute, uint8_t **ppBuffer)
{
    /* Read gattAttribute_t fields from buffer */
    fsciBleGetUint16ValueFromBuffer(pAttribute->handle, *ppBuffer);
    fsciBleGetEnumValueFromBuffer(pAttribute->uuidType, *ppBuffer, bleUuidType_t);
    fsciBleGetUuidFromBuffer(&pAttribute->uuid, ppBuffer, pAttribute->uuidType);
    fsciBleGetUint16ValueFromBuffer(pAttribute->valueLength, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pAttribute->maxValueLength, *ppBuffer);

    /* If valueLength is bigger than 0, also the paValue field must be read from buffer */
    if (0 != pAttribute->valueLength)
    {
        /* Verify if paValue is NULL (this situation is not allowed) */
        if (NULL == pAttribute->paValue)
        {
            panic(0, (uint32_t)fsciBleGattClientGetAttributeFromBuffer, 0, 0);
            return;
        }

        fsciBleGetArrayFromBuffer(pAttribute->paValue, *ppBuffer, pAttribute->valueLength);
    }
}

void fsciBleGattClientGetBufferFromAttribute(gattAttribute_t *pAttribute, uint8_t **ppBuffer)
{
    /* Write in buffer all the gattAttribute_t fields */
    fsciBleGetBufferFromUint16Value(pAttribute->handle, *ppBuffer);
    fsciBleGetBufferFromEnumValue(pAttribute->uuidType, *ppBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(&pAttribute->uuid, ppBuffer, pAttribute->uuidType);
    fsciBleGetBufferFromUint16Value(pAttribute->valueLength, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pAttribute->maxValueLength, *ppBuffer);

    /* If valueLength is bigger than 0, also the paValue field must be written in buffer */
    if (0 != pAttribute->valueLength)
    {
        fsciBleGetBufferFromArray(pAttribute->paValue, *ppBuffer, pAttribute->valueLength);
    }
}

gattCharacteristic_t *fsciBleGattClientAllocOutOrIoCharacteristics(uint8_t maxNbOfCharacteristics)
{
    uint32_t iCount;

    /* Verify if a characteristics array is already allocated (only one kept characteristics array
    can be allocated at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the characteristics array */
    fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics =
        MEM_BufferAlloc(maxNbOfCharacteristics * sizeof(gattCharacteristic_t));

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics)
    {
        fsciBleGattClientHandleNoMemoryStatus();
        return NULL;
    }

    /* For every characteristic in the array, set the paValue field and aDescriptors array to NULL
    (if paValue is not NULL, a different buffer must be separately allocated for it using
    fsciBleGattClientAllocOutOrIoValue function; if aDescriptors is not NULL, a different buffer must be separately
    allocated for it using fsciBleGattClientAllocOutOrIoAttributes function) */
    for (iCount = 0; iCount < maxNbOfCharacteristics; iCount++)
    {
        fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics[iCount].value.valueLength = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics[iCount].value.paValue     = NULL;
        fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics[iCount].cNumDescriptors   = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics[iCount].aDescriptors      = NULL;
    }

    /* Return the allocated kept characteristics array */
    return fsciBleGattClientTmpAllocatedManagementInfo.pCharacteristics;
}

uint16_t fsciBleGattClientGetCharacteristicBufferSize(gattCharacteristic_t *pCharacteristic)
{
    /* Get the constant size for the buffer needed by a characteristic */
    uint16_t bufferSize = sizeof(gattCharacteristicPropertiesBitFields_t) +
                          fsciBleGattClientGetAttributeBufferSize(&pCharacteristic->value) + sizeof(uint8_t);
    uint32_t iCount;

    /* Get the variable size for the buffer needed by a characteristic */
    for (iCount = 0; iCount < pCharacteristic->cNumDescriptors; iCount++)
    {
        bufferSize += fsciBleGattClientGetAttributeBufferSize(&pCharacteristic->aDescriptors[iCount]);
    }

    /* Return the buffer size needed for this characteristic */
    return bufferSize;
}

void fsciBleGattClientGetCharacteristicFromBuffer(gattCharacteristic_t *pCharacteristic, uint8_t **ppBuffer)
{
    /* Read gattCharacteristic_t fields from buffer */
    fsciBleGetEnumValueFromBuffer(pCharacteristic->properties, *ppBuffer, gattCharacteristicPropertiesBitFields_t);
    fsciBleGattClientGetAttributeFromBuffer(&pCharacteristic->value, ppBuffer);
    fsciBleGetUint8ValueFromBuffer(pCharacteristic->cNumDescriptors, *ppBuffer);

    /* If cNumDescriptors is bigger than 0, also the descriptors array field must be read from buffer */
    if (0 != pCharacteristic->cNumDescriptors)
    {
        uint32_t iCount;

        /* Verify if aDescriptors is NULL (this situation is not allowed) */
        if (NULL == pCharacteristic->aDescriptors)
        {
            panic(0, (uint32_t)fsciBleGattClientGetCharacteristicFromBuffer, 0, 0);
            return;
        }

        for (iCount = 0; iCount < pCharacteristic->cNumDescriptors; iCount++)
        {
            fsciBleGattClientGetAttributeFromBuffer(&pCharacteristic->aDescriptors[iCount], ppBuffer);
        }
    }
}

void fsciBleGattClientGetBufferFromCharacteristic(gattCharacteristic_t *pCharacteristic, uint8_t **ppBuffer)
{
    uint32_t iCount;

    /* Write in buffer all the gattCharacteristic_t fields */
    fsciBleGetBufferFromEnumValue(pCharacteristic->properties, *ppBuffer, gattCharacteristicPropertiesBitFields_t);
    fsciBleGattClientGetBufferFromAttribute(&pCharacteristic->value, ppBuffer);
    fsciBleGetBufferFromUint8Value(pCharacteristic->cNumDescriptors, *ppBuffer);

    /* If cNumDescriptors is bigger than 0, also the aDescriptors array field must be written in buffer */
    for (iCount = 0; iCount < pCharacteristic->cNumDescriptors; iCount++)
    {
        fsciBleGattClientGetBufferFromAttribute(&pCharacteristic->aDescriptors[iCount], ppBuffer);
    }
}

gattService_t *fsciBleGattClientAllocOutOrIoIncludedServices(uint8_t maxNbOfIncludedServices)
{
    uint32_t iCount;

    /* Verify if an includedServices array is already allocated (only one kept includedServices
    array can be allocated at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the includedServices array */
    fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices =
        MEM_BufferAlloc(maxNbOfIncludedServices * sizeof(gattService_t));

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices)
    {
        fsciBleGattClientHandleNoMemoryStatus();
        return NULL;
    }

    /* For every includedService in the array, set the aCharacteristics and aIncludedServices arrays to NULL */
    for (iCount = 0; iCount < maxNbOfIncludedServices; iCount++)
    {
        fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices[iCount].cNumCharacteristics  = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices[iCount].aCharacteristics     = NULL;
        fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices[iCount].cNumIncludedServices = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices[iCount].aIncludedServices    = NULL;
    }

    /* Return the allocated kept includedServices array */
    return fsciBleGattClientTmpAllocatedManagementInfo.pIncludedServices;
}

gattService_t *fsciBleGattClientAllocOutOrIoServices(uint8_t maxNbOfServices)
{
    uint32_t iCount;

    /* Verify if a services array is already allocated (only one kept services
    array can be allocated at a time) */
    if (NULL != fsciBleGattClientTmpAllocatedManagementInfo.pServices)
    {
        fsciBleGattClientHandleInternalErrorStatus();
        return NULL;
    }

    /* Allocate buffer for the services array */
    fsciBleGattClientTmpAllocatedManagementInfo.pServices = MEM_BufferAlloc(maxNbOfServices * sizeof(gattService_t));

    if (NULL == fsciBleGattClientTmpAllocatedManagementInfo.pServices)
    {
        fsciBleGattClientHandleNoMemoryStatus();
        return NULL;
    }

    /* For every includedService in the array, set the aCharacteristics and aIncludedServices arrays to NULL
    (if aCharacteristics is not NULL, a different buffer must be separately allocated for it using
    fsciBleGattClientAllocOutOrIoCharacteristics function; if aIncludedServices is not NULL, a different buffer must
    be separately allocated for it using fsciBleGattClientAllocOutOrIoIncludedServicess function */
    for (iCount = 0; iCount < maxNbOfServices; iCount++)
    {
        fsciBleGattClientTmpAllocatedManagementInfo.pServices[iCount].cNumCharacteristics  = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pServices[iCount].aCharacteristics     = NULL;
        fsciBleGattClientTmpAllocatedManagementInfo.pServices[iCount].cNumIncludedServices = 0;
        fsciBleGattClientTmpAllocatedManagementInfo.pServices[iCount].aIncludedServices    = NULL;
    }

    /* Return the allocated kept services array */
    return fsciBleGattClientTmpAllocatedManagementInfo.pServices;
}

uint16_t fsciBleGattClientGetServiceBufferSize(gattService_t *pService)
{
    /* Get the constant size for the buffer needed by a service */
    uint16_t bufferSize = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(bleUuidType_t) +
                          fsciBleGetUuidBufferSize(pService->uuidType) + sizeof(uint8_t);
    uint32_t iCount;

    /* Get the variable size for the buffer needed by a service */

    for (iCount = 0; iCount < pService->cNumCharacteristics; iCount++)
    {
        bufferSize += fsciBleGattClientGetCharacteristicBufferSize(&pService->aCharacteristics[iCount]);
    }

    bufferSize += sizeof(uint8_t);

    for (iCount = 0; iCount < pService->cNumIncludedServices; iCount++)
    {
        bufferSize += fsciBleGattClientGetServiceBufferSize(&pService->aIncludedServices[iCount]);
    }

    /* Return the buffer size needed for this service */
    return bufferSize;
}

void fsciBleGattClientGetServiceFromBuffer(gattService_t *pService, uint8_t **ppBuffer)
{
    uint32_t iCount;

    /* Read gattService_t fields from buffer */
    fsciBleGetUint16ValueFromBuffer(pService->startHandle, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pService->endHandle, *ppBuffer);
    fsciBleGetEnumValueFromBuffer(pService->uuidType, *ppBuffer, bleUuidType_t);
    fsciBleGetUuidFromBuffer(&pService->uuid, ppBuffer, pService->uuidType);
    fsciBleGetUint8ValueFromBuffer(pService->cNumCharacteristics, *ppBuffer);

    /* If cNumCharacteristics is bigger than 0, also the characteristics array field must be read from buffer */
    if (0 != pService->cNumCharacteristics)
    {
        /* Verify if aCharacteristics is NULL (this situation is not allowed) */
        if (NULL == pService->aCharacteristics)
        {
            panic(0, (uint32_t)fsciBleGattClientGetServiceFromBuffer, 0, 0);
            return;
        }

        for (iCount = 0; iCount < pService->cNumCharacteristics; iCount++)
        {
            fsciBleGattClientGetCharacteristicFromBuffer(&pService->aCharacteristics[iCount], ppBuffer);
        }
    }

    /* If cNumIncludedServices is bigger than 0, also the includedServices array field must be read from buffer */
    fsciBleGetUint8ValueFromBuffer(pService->cNumIncludedServices, *ppBuffer);

    if (0 != pService->cNumIncludedServices)
    {
        /* Verify if aIncludedServices is NULL (this situation is not allowed) */
        if (NULL == pService->aIncludedServices)
        {
            panic(0, (uint32_t)fsciBleGattClientGetServiceFromBuffer, 0, 0);
            return;
        }

        for (iCount = 0; iCount < pService->cNumIncludedServices; iCount++)
        {
            fsciBleGattClientGetServiceFromBuffer(&pService->aIncludedServices[iCount], ppBuffer);
        }
    }
}

void fsciBleGattClientGetBufferFromService(gattService_t *pService, uint8_t **ppBuffer)
{
    uint32_t iCount;

    /* Write in buffer all the gattService_t fields */
    fsciBleGetBufferFromUint16Value(pService->startHandle, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pService->endHandle, *ppBuffer);
    fsciBleGetBufferFromEnumValue(pService->uuidType, *ppBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(&pService->uuid, ppBuffer, pService->uuidType);
    fsciBleGetBufferFromUint8Value(pService->cNumCharacteristics, *ppBuffer);

    /* If cNumCharacteristics is bigger than 0, also the aCharacteristics array field must be written in buffer */
    for (iCount = 0; iCount < pService->cNumCharacteristics; iCount++)
    {
        fsciBleGattClientGetBufferFromCharacteristic(&pService->aCharacteristics[iCount], ppBuffer);
    }

    fsciBleGetBufferFromUint8Value(pService->cNumIncludedServices, *ppBuffer);

    /* If cNumIncludedServices is bigger than 0, also the aIncludedServices array field must be written in buffer */
    for (iCount = 0; iCount < pService->cNumIncludedServices; iCount++)
    {
        fsciBleGattClientGetBufferFromService(&pService->aIncludedServices[iCount], ppBuffer);
    }
}

void fsciBleGattClientSaveServicesInfo(deviceId_t deviceId, gattService_t *pServices)
{
    /* Keep (only save) the services array */
    fsciBleGattClientSavedManagementInfo[deviceId].pServices = pServices;
}

void fsciBleGattClientSaveIncludedServicesInfo(deviceId_t deviceId, gattService_t *pIncludedServices)
{
    /* Keep (only save) the includedServices array */
    fsciBleGattClientSavedManagementInfo[deviceId].pIncludedServices = pIncludedServices;
}

void fsciBleGattClientSaveCharacteristicsInfo(deviceId_t deviceId, gattCharacteristic_t *pCharacteristics)
{
    /* Keep (only save) the characteristics array */
    fsciBleGattClientSavedManagementInfo[deviceId].pCharacteristics = pCharacteristics;
}

void fsciBleGattClientSaveDescriptorsInfo(deviceId_t deviceId, gattAttribute_t *pDescriptors)
{
    /* Keep (only save) the descriptors array */
    fsciBleGattClientSavedManagementInfo[deviceId].pDescriptors = pDescriptors;
}

void fsciBleGattClientSaveValueInfo(deviceId_t deviceId, uint8_t *pValue)
{
    /* Keep (only save) the value */
    fsciBleGattClientSavedManagementInfo[deviceId].pValue = pValue;
}

void fsciBleGattClientSaveArraySizeInfo(deviceId_t deviceId, uint16_t *pArraySize)
{
    /* Keep (only save) the array size pointer */
    fsciBleGattClientSavedManagementInfo[deviceId].pArraySize = pArraySize;
}

gattService_t *fsciBleGattClientGetServicesInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) services array */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pServices :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pServices;
}

gattService_t *fsciBleGattClientGetIncludedServicesInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) includedServices array */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pIncludedServices :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pIncludedServices;
}

gattCharacteristic_t *fsciBleGattClientGetCharacteristicsInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) characteristics array */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pCharacteristics :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pCharacteristics;
}

gattAttribute_t *fsciBleGattClientGetDescriptorsInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) descriptors array */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pDescriptors :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pDescriptors;
}

uint8_t *fsciBleGattClientGetValueInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) value */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pValue :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pValue;
}

uint16_t *fsciBleGattClientGetArraySizeInfo(deviceId_t deviceId, bool_t bAllocated)
{
    /* Return the kept (allocated or saved) array size pointer */
    return (TRUE == bAllocated) ? fsciBleGattClientAllocatedManagementInfo.pArraySize :
                                  fsciBleGattClientSavedManagementInfo[deviceId].pArraySize;
}

void fsciBleGattClientKeepInfo(bool_t bAllocatedInfo)
{
    /* Keep temporary information */
    if (TRUE == bAllocatedInfo)
    {
        /* Keep temporary allocated information */
        FLib_MemCpy(&fsciBleGattClientAllocatedManagementInfo, &fsciBleGattClientTmpAllocatedManagementInfo,
                    sizeof(fsciBleGattClientManagementInfo_t));
        /* Erase the temporary allocated information */
        FLib_MemSet(&fsciBleGattClientTmpAllocatedManagementInfo, 0x00, sizeof(fsciBleGattClientManagementInfo_t));
    }
}

void fsciBleGattClientErasePermanentOrTmpInfo(bool_t bAllocatedInfo, bool_t bTmp)
{
    /* Erase allocated or saved information */
    if (TRUE == bAllocatedInfo)
    {
        /* Erase allocated (temporary or kept) information */
        fsciBleGattClientManagementInfo_t *pFsciBleGattClientAllocatedManagementInfo =
            (TRUE == bTmp) ? &fsciBleGattClientTmpAllocatedManagementInfo : &fsciBleGattClientAllocatedManagementInfo;

        /* Free the allocated buffers */
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pServices);
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pIncludedServices);
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pCharacteristics);
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pDescriptors);
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pValue);
        MEM_BufferFree(pFsciBleGattClientAllocatedManagementInfo->pArraySize);

        FLib_MemSet(pFsciBleGattClientAllocatedManagementInfo, 0x00, sizeof(fsciBleGattClientManagementInfo_t));
    }
    else
    {
        /* Erase saved information */
        FLib_MemSet(&fsciBleGattClientSavedManagementInfo, 0x00, sizeof(fsciBleGattClientManagementInfo_t));
    }
}

#if 0

void fsciBleGattServerAllocCharacteristicForBuffer(uint8_t* pBuffer)
{
    gattCharacteristic_t*           pCharacteristic;
    uint8_t*                        pCharacteristicValue;

    uint8_t                         nbOfDescriptors;
    uint16_t                        maxDescriptorsValueLength = 0
    uint16_t*                       pMaxDescriptorsValueLengthList;
    gattAttribute_t*                pDescriptors;
    uint8_t*                        pDescriptorsValue;

    bleUuidType_t                   uuidType;
    uint16_t                        maxValueLength;
    uint16_t                        valueLength;
    uint32_t                        iCount;

    /* Go to uuidType field, in value field */
    pBuffer += sizeof(gattCharacteristicPropertiesBitFields_t) + sizeof(uint16_t);
    /* Get the uuidType field, in value field */
    fsciBleGetEnumValueFromBuffer(uuidType, pBuffer, bleUuidType_t);
    /* Go to valueLength field, in value field (uuid size in buffer depends on uuidType) */
    pBuffer += fsciBleGetUuidSizeForBuffer(uuidType);
    /* Get the valueLength and maxValueLength fields, in value field */
    fsciBleGetUint16ValueFromBuffer(valueLength, pBuffer);
    fsciBleGetUint16ValueFromBuffer(maxValueLength, pBuffer);
    /* Go to cNumDescriptors field */
    pBuffer += valueLength;

    /* Allocate buffer for the characteristic and its value */
    pCharacteristics = (gattCharacteristic_t*)MEM_BufferAlloc(sizeof(gattCharacteristic_t) + maxValueLength);

    if(NULL == pCharacteristics)
    {
        FSCI_Error(gFsciOutOfMessages_c, fsciInterfaceId);

        return NULL;
    }

    /* Set paValue field in value field */
    pCharacteristics->value.paValue = (uint8_t*)pCharacteristics + sizeof(gattCharacteristic_t);

    /* Get the cNumDescriptors field */
    fsciBleGetUint8ValueFromBuffer(nbOfDescriptors, pBuffer);

    if(0 != nbOfDescriptors)
    {
        /* Allocate a buffer to keep descriptors maximum value lengths */
        pMaxDescriptorsValueLengthList = (uint16_t*)MEM_BufferAlloc(nbOfDescriptors * sizeof(uint16_t));

        if(NULL == pMaxDescriptorsValueLengthList)
        {
            FSCI_Error(gFsciOutOfMessages_c, fsciInterfaceId);

            MEM_BufferFree(pCharacteristics);

            return NULL;
        }

        /* Get the descriptors maximum value lengths */
        for(iCount = 0; iCount < nbOfDescriptors; iCount ++)
        {
            /* Go to uuidType field, in aDescriptors[iCount] field */
            pBuffer += sizeof(uint16_t);
            /* Get the uuidType field, in aDescriptors[iCount] field */
            fsciBleGetEnumValueFromBuffer(uuidType, pBuffer, bleUuidType_t);
            /* Go to valueLength field, in aDescriptors[iCount] field */
            pBuffer += fsciBleGetUuidSizeForBuffer(uuidType);
            /* Get the valueLength and maxValueLength fields, in aDescriptors[iCount] field */
            fsciBleGetUint16ValueFromBuffer(valueLength, pBuffer);
            fsciBleGetUint16ValueFromBuffer(maxValueLength, pBuffer);
            /* Go to next descriptor field */
            pBufer += valueLength;

            /* Compute all descriptors maximum value length */
            maxDescriptorsValueLength              += maxValueLength;
            /* Save this descriptor maximum value length */
            pMaxDescriptorsValueLengthList[iCount]  = maxValueLength;
        }

        /* Allocate buffer for the descriptors and their values */
        pDescriptors = (gattAttribute_t*)MEM_BufferAlloc(nbOfDescriptors * sizeof(gattAttribute_t) + maxDescriptorsValueLength)

        if(NULL == pDescriptors)
        {
            FSCI_Error(gFsciOutOfMessages_c, fsciInterfaceId);

            MEM_BufferFree(pCharacteristics);
            MEM_BufferFree(pMaxDescriptorsValueLengthList);

            return NULL;
        }

        /* Get the pointer of the first descriptor value */
        pDescriptorsValue               = (uint8_t*)pDescriptors + nbOfDescriptors * sizeof(gattAttribute_t);
        /* Set descriptors in the characteristic */
        pCharacteristics->aDescriptors  = pDescriptors;

        /* Set the value pointers in all the descriptors */
        for(iCount = 0; iCount < nbOfDescriptors; iCount++)
        {
            pDescriptors->paValue   = pDescriptorsValue;
            pDescriptors ++;
            pDescriptorsValue      += pMaxDescriptorsValueLengthList[iCount];
        }

        /* Free the buffer used to keep descriptors maximum value lengths */
        MEM_BufferFree(pMaxDescriptorsValueLengthList);
    }
    else
    {
        /* No descriptors for this characteristic */
        pCharacteristics->cNumDescriptors   = 0;
         pCharacteristics->aDescriptors     = NULL;
    }

    /* Return the characteristic buffer */
    return pCharacteristic;
}


void fsciBleGattServerFreeCharacteristic(gattCharacteristic_t* pCharacteristic)
{
    /* Free the characteristic descriptors buffer */
    MEM_BufferFree(pCharacteristics->aDescriptors);
    /* Free the characteristic buffer */
    MEM_BufferFree(pCharacteristics);
}

#endif

void fsciBleGattServerGetServerAttributeWrittenEventFromBuffer(
    gattServerAttributeWrittenEvent_t *pServerAttributeWrittenEvent, uint8_t **ppBuffer)
{
    /* Read gattServerAttributeWrittenEvent_t field from buffer */
    fsciBleGetUint16ValueFromBuffer(pServerAttributeWrittenEvent->handle, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pServerAttributeWrittenEvent->cValueLength, *ppBuffer);
    fsciBleGetArrayFromBuffer(pServerAttributeWrittenEvent->aValue, *ppBuffer,
                              pServerAttributeWrittenEvent->cValueLength);
}

void fsciBleGattServerGetBufferFromServerAttributeWrittenEvent(
    gattServerAttributeWrittenEvent_t *pServerAttributeWrittenEvent, uint8_t **ppBuffer)
{
    /* Write gattServerAttributeWrittenEvent_t field in buffer */
    fsciBleGetBufferFromUint16Value(pServerAttributeWrittenEvent->handle, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pServerAttributeWrittenEvent->cValueLength, *ppBuffer);
    fsciBleGetBufferFromArray(pServerAttributeWrittenEvent->aValue, *ppBuffer,
                              pServerAttributeWrittenEvent->cValueLength);
}

void fsciBleGattServerGetServerLongCharacteristicWrittenEventFromBuffer(
    gattServerLongCharacteristicWrittenEvent_t *pServerLongCharacteristicWrittenEvent, uint8_t **ppBuffer)
{
    /* Read gattServerLongCharacteristicWrittenEvent_t field from buffer */
    fsciBleGetUint16ValueFromBuffer(pServerLongCharacteristicWrittenEvent->handle, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pServerLongCharacteristicWrittenEvent->cValueLength, *ppBuffer);
    fsciBleGetArrayFromBuffer(pServerLongCharacteristicWrittenEvent->aValue, *ppBuffer,
                              pServerLongCharacteristicWrittenEvent->cValueLength);
}

void fsciBleGattServerGetBufferFromServerLongCharacteristicWrittenEvent(
    gattServerLongCharacteristicWrittenEvent_t *pServerLongCharacteristicWrittenEvent, uint8_t **ppBuffer)
{
    /* Write gattServerLongCharacteristicWrittenEvent_t field in buffer */
    fsciBleGetBufferFromUint16Value(pServerLongCharacteristicWrittenEvent->handle, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pServerLongCharacteristicWrittenEvent->cValueLength, *ppBuffer);
    fsciBleGetBufferFromArray(pServerLongCharacteristicWrittenEvent->aValue, *ppBuffer,
                              pServerLongCharacteristicWrittenEvent->cValueLength);
}

void fsciBleGattServerGetServerCccdWrittenEventFromBuffer(gattServerCccdWrittenEvent_t *pServerCccdWrittenEvent,
                                                          uint8_t **ppBuffer)
{
    /* Read gattServerCccdWrittenEvent_t field from buffer */
    fsciBleGetUint16ValueFromBuffer(pServerCccdWrittenEvent->handle, *ppBuffer);
    fsciBleGetEnumValueFromBuffer(pServerCccdWrittenEvent->newCccd, *ppBuffer, gattCccdFlags_t);
}

void fsciBleGattServerGetBufferFromServerCccdWrittenEvent(gattServerCccdWrittenEvent_t *pServerCccdWrittenEvent,
                                                          uint8_t **ppBuffer)
{
    /* Write gattServerCccdWrittenEvent_t field in buffer */
    fsciBleGetBufferFromUint16Value(pServerCccdWrittenEvent->handle, *ppBuffer);
    fsciBleGetBufferFromEnumValue(pServerCccdWrittenEvent->newCccd, *ppBuffer, gattCccdFlags_t);
}

void fsciBleGattServerGetServerProcedureErrorFromBuffer(gattServerProcedureError_t *pServerProcedureError,
                                                        uint8_t **ppBuffer)
{
    /* Read gattServerProcedureError_t field from buffer */
    fsciBleGetEnumValueFromBuffer(pServerProcedureError->procedureType, *ppBuffer, gattServerProcedureType_t);
    fsciBleGetEnumValueFromBuffer(pServerProcedureError->error, *ppBuffer, bleResult_t);
}

void fsciBleGattServerGetBufferFromServerProcedureError(gattServerProcedureError_t *pServerProcedureError,
                                                        uint8_t **ppBuffer)
{
    /* Write gattServerProcedureError_t field in buffer */
    fsciBleGetBufferFromEnumValue(pServerProcedureError->procedureType, *ppBuffer, gattServerProcedureType_t);
    fsciBleGetBufferFromEnumValue(pServerProcedureError->error, *ppBuffer, bleResult_t);
}

gattServerEvent_t *fsciBleGattServerAllocServerEventForBuffer(gattServerEventType_t eventType, uint8_t *pBuffer)
{
    uint16_t variableLength = 0;
    gattServerEvent_t *pServerEvent;

    if ((gEvtAttributeWritten_c == eventType) || (gEvtAttributeWrittenWithoutResponse_c == eventType) ||
        (gEvtLongCharacteristicWritten_c == eventType))
    {
        /* Go to cValueLength field in structure, skip device_id and handle */
        pBuffer += sizeof(uint8_t) + sizeof(uint16_t);

        /* Get cValueLength field from buffer */
        fsciBleGetUint16ValueFromBuffer(variableLength, pBuffer);
    }

    /* Allocate memory for the server event */
    pServerEvent = (gattServerEvent_t *)MEM_BufferAlloc(sizeof(gattServerEvent_t) + variableLength);

    if (NULL == pServerEvent)
    {
        /* No memory */
        FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
    }
    else
    {
        /* Set event type in server event */
        pServerEvent->eventType = eventType;

        if ((gEvtAttributeWritten_c == eventType) || (gEvtAttributeWrittenWithoutResponse_c == eventType))
        {
            /* Set pointer to the variable length value */
            pServerEvent->eventData.attributeWrittenEvent.aValue = (uint8_t *)pServerEvent + sizeof(gattServerEvent_t);
        }
        else if (gEvtLongCharacteristicWritten_c == eventType)
        {
            /* Set pointer to the variable length value */
            pServerEvent->eventData.longCharWrittenEvent.aValue = (uint8_t *)pServerEvent + sizeof(gattServerEvent_t);
        }
    }

    /* Return memory allocated for the server event */
    return pServerEvent;
}

uint16_t fsciBleGattServerGetServerEventBufferSize(gattServerEvent_t *pServerEvent)
{
    /* Get the constant size for the buffer needed by a server event */
    uint16_t bufferSize = 0;

    /* Get the variable size for the buffer needed by a server event (depending on eventType) */
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            bufferSize += fsciBleGattServerGetServerMtuChangedEventBufferSize(&pServerEvent->eventData.mtuChangedEvent);
        }
        break;

        case gEvtAttributeWritten_c:
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            bufferSize += fsciBleGattServerGetServerAttributeWrittenEventBufferSize(
                &pServerEvent->eventData.attributeWrittenEvent);
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            bufferSize +=
                fsciBleGattServerGetServerCccdWrittenEventBufferSize(&pServerEvent->eventData.charCccdWrittenEvent);
        }
        break;

        case gEvtError_c:
        {
            bufferSize += fsciBleGattServerGetServerProcedureErrorBufferSize(&pServerEvent->eventData.procedureError);
        }
        break;

        case gEvtLongCharacteristicWritten_c:
        {
            bufferSize += fsciBleGattServerGetServerLongCharacteristicWrittenEventBufferSize(
                &pServerEvent->eventData.longCharWrittenEvent);
        }
        break;

        case gEvtAttributeRead_c:
        {
            bufferSize +=
                fsciBleGattServerGetServerAttributeReadEventBufferSize(&pServerEvent->eventData.attributeReadEvent);
        }
        break;

        default:
            break;
    }

    /* Return the buffer size needed for this server event */
    return bufferSize;
}

void fsciBleGattServerGetServerEventFromBuffer(gattServerEvent_t *pServerEvent, uint8_t **ppBuffer)
{
    /* Read the event fields from buffer (depending on eventType) */
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            fsciBleGattServerGetServerMtuChangedEventFromBuffer(&pServerEvent->eventData.mtuChangedEvent, ppBuffer);
        }
        break;

        case gEvtAttributeWritten_c:
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            fsciBleGattServerGetServerAttributeWrittenEventFromBuffer(&pServerEvent->eventData.attributeWrittenEvent,
                                                                      ppBuffer);
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            fsciBleGattServerGetServerCccdWrittenEventFromBuffer(&pServerEvent->eventData.charCccdWrittenEvent,
                                                                 ppBuffer);
        }
        break;

        case gEvtError_c:
        {
            fsciBleGattServerGetServerProcedureErrorFromBuffer(&pServerEvent->eventData.procedureError, ppBuffer);
        }
        break;

        case gEvtLongCharacteristicWritten_c:
        {
            fsciBleGattServerGetServerLongCharacteristicWrittenEventFromBuffer(
                &pServerEvent->eventData.longCharWrittenEvent, ppBuffer);
        }
        break;

        case gEvtAttributeRead_c:
        {
            fsciBleGattServerGetServerAttributeReadEventFromBuffer(&pServerEvent->eventData.attributeReadEvent,
                                                                   ppBuffer);
        }
        break;

        default:
            break;
    }
}

void fsciBleGattServerGetBufferFromServerEvent(gattServerEvent_t *pServerEvent, uint8_t **ppBuffer)
{
    /* Write the event field in buffer (depending on eventType) */
    switch (pServerEvent->eventType)
    {
        case gEvtMtuChanged_c:
        {
            fsciBleGattServerGetBufferFromServerMtuChangedEvent(&pServerEvent->eventData.mtuChangedEvent, ppBuffer);
        }
        break;

        case gEvtAttributeWritten_c:
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            fsciBleGattServerGetBufferFromServerAttributeWrittenEvent(&pServerEvent->eventData.attributeWrittenEvent,
                                                                      ppBuffer);
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            fsciBleGattServerGetBufferFromServerCccdWrittenEvent(&pServerEvent->eventData.charCccdWrittenEvent,
                                                                 ppBuffer);
        }
        break;

        case gEvtError_c:
        {
            fsciBleGattServerGetBufferFromServerProcedureError(&pServerEvent->eventData.procedureError, ppBuffer);
        }
        break;

        case gEvtLongCharacteristicWritten_c:
        {
            fsciBleGattServerGetBufferFromServerLongCharacteristicWrittenEvent(
                &pServerEvent->eventData.longCharWrittenEvent, ppBuffer);
        }
        break;

        case gEvtAttributeRead_c:
        {
            fsciBleGattServerGetBufferFromServerAttributeReadEvent(&pServerEvent->eventData.attributeReadEvent,
                                                                   ppBuffer);
        }
        break;

        default:
            break;
    }
}

void fsciBleGattGetHandleRangeFromBuffer(gattHandleRange_t *pHandleRange, uint8_t **ppBuffer)
{
    fsciBleGetUint16ValueFromBuffer(pHandleRange->startHandle, *ppBuffer);
    fsciBleGetUint16ValueFromBuffer(pHandleRange->endHandle, *ppBuffer);
}

void fsciBleGattGetBufferFromHandleRange(gattHandleRange_t *pHandleRange, uint8_t **ppBuffer)
{
    fsciBleGetBufferFromUint16Value(pHandleRange->startHandle, *ppBuffer);
    fsciBleGetBufferFromUint16Value(pHandleRange->endHandle, *ppBuffer);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void fsciBleGattClientHandleErrorStatus(gFsciStatus_t status)
{
    /* Send status over UART */
    FSCI_Error(status, fsciBleInterfaceId);
    /* Erase temporary allocated information */
    fsciBleGattClientEraseTmpInfo(TRUE);
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
