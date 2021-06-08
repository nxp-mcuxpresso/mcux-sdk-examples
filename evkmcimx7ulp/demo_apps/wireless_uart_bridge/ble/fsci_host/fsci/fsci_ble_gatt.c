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

#include "fsci_ble_gatt.h"
#include "host_ble.h"

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

/* Macro used for saving the out parameters pointers of the GATT functions */
#define fsciBleGattSaveOutParams(pOutParam) fsciBleGattOutParams.pParam = (uint8_t *)pOutParam

/* Macro used for restoring the out parameters pointers of the GATT functions */
#define fsciBleGattRestoreOutParams() &fsciBleGattOutParams

/* Macro used for setting the out parameters pointers of the GATT
functions to NULL */
#define fsciBleGattCleanOutParams() fsciBleGattOutParams.pParam = NULL

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

/*! Structure that keeps callbacks registered by application or FSCI in GATT. */
typedef struct fsciBleGattCallbacks_tag
{
    gattClientProcedureCallback_t clientProcedureCallback;       /* GATT Client procedure callback. */
    gattClientNotificationCallback_t clientNotificationCallback; /* GATT Client notification callback. */
    gattClientIndicationCallback_t clientIndicationCallback;     /* GATT Client indication callback. */
    gattServerCallback_t serverCallback;                         /* GATT Server callback. */
} fsciBleGattCallbacks_t;

/* Structure used for keeping the out parameters pointers of the GATT
 functions */
typedef struct fsciBleGattOutParams_tag
{
    uint8_t *pParam;
} fsciBleGattOutParams_t;

/* Structure used for keeping the out parameters pointers of the
Gatt_GetMtu function */
typedef struct fsciBleGattGetMtuOutParams_tag
{
    uint16_t *pMtu;
} fsciBleGattGetMtuOutParams_t;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

static void fsciBleGattClientProcedureCallback(deviceId_t deviceId,
                                               gattProcedureType_t procedureType,
                                               gattProcedureResult_t procedureResult,
                                               bleResult_t error);
static void fsciBleGattClientNotificationCallback(deviceId_t deviceId,
                                                  uint16_t characteristicValueHandle,
                                                  uint8_t *aValue,
                                                  uint16_t valueLength);
static void fsciBleGattClientIndicationCallback(deviceId_t deviceId,
                                                uint16_t characteristicValueHandle,
                                                uint8_t *aValue,
                                                uint16_t valueLength);

static void fsciBleGattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent);

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* GATT callbacks structure initialized with FSCI empty static functions */
static fsciBleGattCallbacks_t fsciBleGattCallbacks = {fsciBleGattClientProcedureCallback,
                                                      fsciBleGattClientNotificationCallback,
                                                      fsciBleGattClientIndicationCallback, fsciBleGattServerCallback};

/* Keeps out parameters pointers for Host - BBox functionality */
static fsciBleGattOutParams_t fsciBleGattOutParams = {NULL};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

void fsciBleSetGattClientProcedureCallback(gattClientProcedureCallback_t clientProcedureCallback)
{
    /* Set GATT Client procedure callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGattCallbacks.clientProcedureCallback =
        (NULL != clientProcedureCallback) ? clientProcedureCallback : fsciBleGattClientProcedureCallback;
}

void fsciBleSetGattClientNotificationCallback(gattClientNotificationCallback_t clientNotificationCallback)
{
    /* Set GATT Client notification callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGattCallbacks.clientNotificationCallback =
        (NULL != clientNotificationCallback) ? clientNotificationCallback : fsciBleGattClientNotificationCallback;
}

void fsciBleSetGattClientIndicationCallback(gattClientIndicationCallback_t clientIndicationCallback)
{
    /* Set GATT Client indication callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGattCallbacks.clientIndicationCallback =
        (NULL != clientIndicationCallback) ? clientIndicationCallback : fsciBleGattClientIndicationCallback;
}

void fsciBleSetGattServerCallback(gattServerCallback_t serverCallback)
{
    /* Set GATT Server callback to an application desired function, only
    if not NULL. Otherwise set it to the FSCI empty static function */
    fsciBleGattCallbacks.serverCallback = (NULL != serverCallback) ? serverCallback : fsciBleGattServerCallback;
}

void fsciBleGattHandler(void *pData, void *pParam, uint32_t fsciBleInterfaceId)
{
    clientPacket_t *pClientPacket = (clientPacket_t *)pData;
    uint8_t *pBuffer              = &pClientPacket->structured.payload[0];

    /* Select the GATT, GATT Client or GATT Server function to be called (using the FSCI opcode) */
    switch (pClientPacket->structured.header.opCode)
    {
        case gBleGattStatusOpCode_c:
        {
            bleResult_t status;

            /* Get status from buffer */
            fsciBleGetEnumValueFromBuffer(status, pBuffer, bleResult_t);

            if (gBleSuccess_c != status)
            {
                /* Clean out parameters pointers kept in FSCI (only for synchronous function) */
                fsciBleGattCleanOutParams();

                /* Erase the information kept (for asynchronous functions) */
                fsciBleGattClientEraseInfo(FALSE);
            }
        }
        break;

        case gBleGattEvtGetMtuOpCode_c:
        {
            fsciBleGattGetMtuOutParams_t *pOutParams = (fsciBleGattGetMtuOutParams_t *)fsciBleGattRestoreOutParams();

            if (NULL != pOutParams->pMtu)
            {
                /* Get out parameter of the Gatt_GetMtu function from buffer */
                fsciBleGetUint16ValueFromBuffer(*pOutParams->pMtu, pBuffer);

                /* Clean out parameters pointers kept in FSCI */
                fsciBleGattCleanOutParams();

                /* Signal out parameters ready */
                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattEvtClientProcedureExchangeMtuOpCode_c:
        case gBleGattEvtClientProcedureDiscoverAllPrimaryServicesOpCode_c:
        case gBleGattEvtClientProcedureDiscoverPrimaryServicesByUuidOpCode_c:
        case gBleGattEvtClientProcedureFindIncludedServicesOpCode_c:
        case gBleGattEvtClientProcedureDiscoverAllCharacteristicsOpCode_c:
        case gBleGattEvtClientProcedureDiscoverCharacteristicByUuidOpCode_c:
        case gBleGattEvtClientProcedureDiscoverAllCharacteristicDescriptorsOpCode_c:
        case gBleGattEvtClientProcedureReadCharacteristicValueOpCode_c:
        case gBleGattEvtClientProcedureReadUsingCharacteristicUuidOpCode_c:
        case gBleGattEvtClientProcedureReadMultipleCharacteristicValuesOpCode_c:
        case gBleGattEvtClientProcedureWriteCharacteristicValueOpCode_c:
        case gBleGattEvtClientProcedureReadCharacteristicDescriptorOpCode_c:
        case gBleGattEvtClientProcedureWriteCharacteristicDescriptorOpCode_c:
        {
            deviceId_t deviceId;
            gattProcedureType_t procedureType = gGattProcExchangeMtu_c;
            gattProcedureResult_t procedureResult;
            bleResult_t error;
            /* Procedure completed - get the kept information pointers */
            gattService_t *pServices               = NULL;
            gattCharacteristic_t *pCharacteristics = NULL;
            gattAttribute_t *pDescriptors          = NULL;
            uint8_t *pValue                        = NULL;
            uint16_t *pArraySize                   = NULL;
            uint32_t iCount;

            /* Get client procedure callback parameters (from buffer) */
            fsciBleGetDeviceIdFromBuffer(&deviceId, &pBuffer);
            fsciBleGetEnumValueFromBuffer(procedureResult, pBuffer, gattProcedureResult_t);
            fsciBleGetEnumValueFromBuffer(error, pBuffer, bleResult_t);

            pServices        = fsciBleGattClientGetServicesInfo(deviceId, FALSE);
            pCharacteristics = fsciBleGattClientGetCharacteristicsInfo(deviceId, FALSE);
            pDescriptors     = fsciBleGattClientGetDescriptorsInfo(deviceId, FALSE);
            pValue           = fsciBleGattClientGetValueInfo(deviceId, FALSE);
            pArraySize       = fsciBleGattClientGetArraySizeInfo(deviceId, FALSE);

            /* Get procedure type (from opCode) and variable length data) */
            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGattEvtClientProcedureExchangeMtuOpCode_c:
                {
                    procedureType = gGattProcExchangeMtu_c;
                }
                break;

                case gBleGattEvtClientProcedureDiscoverAllPrimaryServicesOpCode_c:
                {
                    procedureType = gGattProcDiscoverAllPrimaryServices_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get services array size parameter from buffer */
                        fsciBleGetUint8ValueFromBuffer(*(uint8_t *)pArraySize, pBuffer);
                        /* Get services array parameter from buffer */
                        for (iCount = 0; iCount < *(uint8_t *)pArraySize; iCount++)
                        {
                            fsciBleGattClientGetServiceFromBuffer(&pServices[iCount], &pBuffer);
                        }
                    }
                }
                break;

                case gBleGattEvtClientProcedureDiscoverPrimaryServicesByUuidOpCode_c:
                {
                    procedureType = gGattProcDiscoverPrimaryServicesByUuid_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get services array size parameter from buffer */
                        fsciBleGetUint8ValueFromBuffer(*(uint8_t *)pArraySize, pBuffer);
                        /* Get services array parameter from buffer */
                        for (iCount = 0; iCount < *(uint8_t *)pArraySize; iCount++)
                        {
                            fsciBleGattClientGetServiceFromBuffer(&pServices[iCount], &pBuffer);
                        }
                    }
                }
                break;

                case gBleGattEvtClientProcedureFindIncludedServicesOpCode_c:
                {
                    procedureType = gGattProcFindIncludedServices_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get service parameter from buffer */
                        fsciBleGattClientGetServiceFromBuffer(pServices, &pBuffer);
                    }
                }
                break;

                case gBleGattEvtClientProcedureDiscoverAllCharacteristicsOpCode_c:
                {
                    procedureType = gGattProcDiscoverAllCharacteristics_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get service parameter from buffer */
                        fsciBleGattClientGetServiceFromBuffer(pServices, &pBuffer);
                    }
                }
                break;

                case gBleGattEvtClientProcedureDiscoverCharacteristicByUuidOpCode_c:
                {
                    procedureType = gGattProcDiscoverCharacteristicByUuid_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get characteristics array size parameter from buffer */
                        fsciBleGetUint8ValueFromBuffer(*(uint8_t *)pArraySize, pBuffer);
                        /* Get characteristics array parameter from buffer */
                        for (iCount = 0; iCount < *(uint8_t *)pArraySize; iCount++)
                        {
                            fsciBleGattClientGetCharacteristicFromBuffer(&pCharacteristics[iCount], &pBuffer);
                        }
                    }
                }
                break;

                case gBleGattEvtClientProcedureDiscoverAllCharacteristicDescriptorsOpCode_c:
                {
                    procedureType = gGattProcDiscoverAllCharacteristicDescriptors_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get characteristic parameter from buffer */
                        fsciBleGattClientGetCharacteristicFromBuffer(pCharacteristics, &pBuffer);
                    }
                }
                break;

                case gBleGattEvtClientProcedureReadCharacteristicValueOpCode_c:
                {
                    procedureType = gGattProcReadCharacteristicValue_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get characteristic parameter from buffer */
                        fsciBleGattClientGetCharacteristicFromBuffer(pCharacteristics, &pBuffer);
                    }
                }
                break;

                case gBleGattEvtClientProcedureReadUsingCharacteristicUuidOpCode_c:
                {
                    procedureType = gGattProcReadUsingCharacteristicUuid_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get value size parameter from buffer */
                        fsciBleGetUint16ValueFromBuffer(*pArraySize, pBuffer);
                        /* Get value parameter from buffer */
                        fsciBleGetArrayFromBuffer(pValue, pBuffer, *pArraySize);
                    }
                }
                break;

                case gBleGattEvtClientProcedureReadMultipleCharacteristicValuesOpCode_c:
                {
                    uint8_t nbOfCharacteristics;
                    procedureType = gGattProcReadMultipleCharacteristicValues_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get characteristics array size parameter from buffer */
                        fsciBleGetUint8ValueFromBuffer(nbOfCharacteristics, pBuffer);
                        /* Get characteristics array parameter from buffer */
                        for (iCount = 0; iCount < nbOfCharacteristics; iCount++)
                        {
                            fsciBleGattClientGetCharacteristicFromBuffer(&pCharacteristics[iCount], &pBuffer);
                        }
                    }
                }
                break;

                case gBleGattEvtClientProcedureWriteCharacteristicValueOpCode_c:
                {
                    procedureType = gGattProcWriteCharacteristicValue_c;
                }
                break;

                case gBleGattEvtClientProcedureReadCharacteristicDescriptorOpCode_c:
                {
                    procedureType = gGattProcReadCharacteristicDescriptor_c;

                    if (gGattProcSuccess_c == procedureResult)
                    {
                        /* Get descriptor parameter from buffer */
                        fsciBleGattClientGetAttributeFromBuffer(pDescriptors, &pBuffer);
                    }
                }
                break;

                case gBleGattEvtClientProcedureWriteCharacteristicDescriptorOpCode_c:
                {
                    procedureType = gGattProcWriteCharacteristicDescriptor_c;
                }
                break;

                default:
                {
                }
                break;
            }

            /* Call client procedure callback */
            fsciBleGattCallbacks.clientProcedureCallback(deviceId, procedureType, procedureResult, error);

            /* Erase the information kept (allocated or just saved) */
            fsciBleGattClientEraseInfo(FALSE);
        }
        break;

        case gBleGattEvtClientNotificationOpCode_c:
        case gBleGattEvtClientIndicationOpCode_c:
        {
            deviceId_t deviceId;
            uint16_t characteristicValueHandle;
            uint8_t *pValue;
            uint16_t valueLength;

            /* Get event parameters from buffer */
            fsciBleGetDeviceIdFromBuffer(&deviceId, &pBuffer);
            fsciBleGetUint16ValueFromBuffer(characteristicValueHandle, pBuffer);
            fsciBleGetUint16ValueFromBuffer(valueLength, pBuffer);

            pValue = MEM_BufferAlloc(valueLength);

            if (NULL == pValue)
            {
                /* No memory */
                FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
            }
            else
            {
                fsciBleGetArrayFromBuffer(pValue, pBuffer, valueLength);

                if (gBleGattEvtClientNotificationOpCode_c == pClientPacket->structured.header.opCode)
                {
                    /* Call client notification callback */
                    fsciBleGattCallbacks.clientNotificationCallback(deviceId, characteristicValueHandle, pValue,
                                                                    valueLength);
                }
                else
                {
                    /* Call client indication callback */
                    fsciBleGattCallbacks.clientIndicationCallback(deviceId, characteristicValueHandle, pValue,
                                                                  valueLength);
                }

                MEM_BufferFree(pValue);

                Ble_OutParamsReady();
            }
        }
        break;

        case gBleGattEvtServerMtuChangedOpCode_c:
        case gBleGattEvtServerHandleValueConfirmationOpCode_c:
        case gBleGattEvtServerAttributeWrittenOpCode_c:
        case gBleGattEvtServerCharacteristicCccdWrittenOpCode_c:
        case gBleGattEvtServerAttributeWrittenWithoutResponseOpCode_c:
        case gBleGattEvtServerErrorOpCode_c:
        case gBleGattEvtServerLongCharacteristicWrittenOpCode_c:
        case gBleGattEvtServerAttributeReadOpCode_c:
        {
            deviceId_t deviceId;
            gattServerEvent_t *pServerEvent;
            gattServerEventType_t eventType = gEvtMtuChanged_c;

            switch (pClientPacket->structured.header.opCode)
            {
                case gBleGattEvtServerMtuChangedOpCode_c:
                {
                    eventType = gEvtMtuChanged_c;
                }
                break;

                case gBleGattEvtServerHandleValueConfirmationOpCode_c:
                {
                    eventType = gEvtHandleValueConfirmation_c;
                }
                break;

                case gBleGattEvtServerAttributeWrittenOpCode_c:
                {
                    eventType = gEvtAttributeWritten_c;
                }
                break;

                case gBleGattEvtServerCharacteristicCccdWrittenOpCode_c:
                {
                    eventType = gEvtCharacteristicCccdWritten_c;
                }
                break;

                case gBleGattEvtServerAttributeWrittenWithoutResponseOpCode_c:
                {
                    eventType = gEvtAttributeWrittenWithoutResponse_c;
                }
                break;

                case gBleGattEvtServerErrorOpCode_c:
                {
                    eventType = gEvtError_c;
                }
                break;

                case gBleGattEvtServerLongCharacteristicWrittenOpCode_c:
                {
                    eventType = gEvtLongCharacteristicWritten_c;
                }
                break;

                case gBleGattEvtServerAttributeReadOpCode_c:
                {
                    eventType = gEvtAttributeRead_c;
                }
                break;

                default:
                    break;
            }

            pServerEvent = fsciBleGattServerAllocServerEventForBuffer(eventType, pBuffer);

            if (NULL == pServerEvent)
            {
                /* No memory */
                FSCI_Error(gFsciOutOfMessages_c, fsciBleInterfaceId);
            }
            else
            {
                /* Get event parameters from buffer */
                fsciBleGetDeviceIdFromBuffer(&deviceId, &pBuffer);
                fsciBleGattServerGetServerEventFromBuffer(pServerEvent, &pBuffer);

                /* Call server callback */
                fsciBleGattCallbacks.serverCallback(deviceId, pServerEvent);

                /* Free allocated server event */
                MEM_BufferFree(pServerEvent);
            }
        }
        break;

        default:
        {
            /* Unknown FSCI opcode */
            FSCI_Error(gFsciUnknownOpcode_c, fsciBleInterfaceId);
        }
        break;
    }

    MEM_BufferFree(pData);
}

void fsciBleGattNoParamCmdMonitor(fsciBleGattOpCode_t opCode)
{
    /* Call the generic FSCI BLE monitor for commands or events that have no parameters */
    fsciBleNoParamCmdOrEvtMonitor(gFsciBleGattOpcodeGroup_c, opCode);
}

void fsciBleGattMtuCmdMonitor(fsciBleGattOpCode_t opCode, deviceId_t deviceId)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(opCode, fsciBleGetDeviceIdBufferSize(&deviceId));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattGetMtuCmdMonitor(deviceId_t deviceId, uint16_t *pOutMtu)
{
    /* Save out parameters pointers */
    fsciBleGattSaveOutParams(pOutMtu);

    fsciBleGattMtuCmdMonitor(gBleGattCmdGetMtuOpCode_c, deviceId);
}

void fsciBleGattClientRegisterProcedureCallbackCmdMonitor(gattClientProcedureCallback_t callback)
{
    fsciBleGattCallbacks.clientProcedureCallback = callback;

    fsciBleGattNoParamCmdMonitor(gBleGattCmdClientRegisterProcedureCallbackOpCode_c);
}

void fsciBleGattClientRegisterNotificationCallbackCmdMonitor(gattClientNotificationCallback_t callback)
{
    fsciBleGattCallbacks.clientNotificationCallback = callback;

    fsciBleGattNoParamCmdMonitor(gBleGattCmdClientRegisterNotificationCallbackOpCode_c);
}

void fsciBleGattClientRegisterIndicationCallbackCmdMonitor(gattClientIndicationCallback_t callback)
{
    fsciBleGattCallbacks.clientIndicationCallback = callback;

    fsciBleGattNoParamCmdMonitor(gBleGattCmdClientRegisterIndicationCallbackOpCode_c);
}

void fsciBleGattClientDiscoverAllPrimaryServicesCmdMonitor(deviceId_t deviceId,
                                                           gattService_t *pOutPrimaryServices,
                                                           uint8_t maxServiceCount,
                                                           uint8_t *pOutDiscoveredCount)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientDiscoverAllPrimaryServicesOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint8Value(maxServiceCount, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientDiscoverPrimaryServicesByUuidCmdMonitor(deviceId_t deviceId,
                                                              bleUuidType_t uuidType,
                                                              bleUuid_t *pUuid,
                                                              gattService_t *aOutPrimaryServices,
                                                              uint8_t maxServiceCount,
                                                              uint8_t *pOutDiscoveredCount)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientDiscoverPrimaryServicesByUuidOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(bleUuidType_t) +
                                                   fsciBleGetUuidBufferSize(uuidType) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(uuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pUuid, &pBuffer, uuidType);
    fsciBleGetBufferFromUint8Value(maxServiceCount, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientDiscoverCharacteristicOfServiceByUuidCmdMonitor(deviceId_t deviceId,
                                                                      bleUuidType_t uuidType,
                                                                      bleUuid_t *pUuid,
                                                                      gattService_t *pIoService,
                                                                      gattCharacteristic_t *aOutCharacteristics,
                                                                      uint8_t maxCharacteristicCount,
                                                                      uint8_t *pOutDiscoveredCount)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientDiscoverCharacteristicOfServiceByUuidOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(bleUuidType_t) +
                                                   fsciBleGetUuidBufferSize(uuidType) +
                                                   fsciBleGattClientGetServiceBufferSize(pIoService) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(uuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pUuid, &pBuffer, uuidType);
    fsciBleGattClientGetBufferFromService(pIoService, &pBuffer);
    fsciBleGetBufferFromUint8Value(maxCharacteristicCount, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientDiscoverAllCharacteristicDescriptorsCmdMonitor(deviceId_t deviceId,
                                                                     gattCharacteristic_t *pIoCharacteristic,
                                                                     uint16_t endingHandle,
                                                                     uint8_t maxDescriptorCount)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientDiscoverAllCharacteristicDescriptorsOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) +
                                                   fsciBleGattClientGetCharacteristicBufferSize(pIoCharacteristic) +
                                                   sizeof(uint16_t) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromCharacteristic(pIoCharacteristic, &pBuffer);
    fsciBleGetBufferFromUint16Value(endingHandle, pBuffer);
    fsciBleGetBufferFromUint8Value(maxDescriptorCount, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientReadCharacteristicValueCmdMonitor(deviceId_t deviceId,
                                                        gattCharacteristic_t *pIoCharacteristic,
                                                        uint16_t maxReadBytes)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientReadCharacteristicValueOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) +
                                                   fsciBleGattClientGetCharacteristicBufferSize(pIoCharacteristic) +
                                                   sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromCharacteristic(pIoCharacteristic, &pBuffer);
    fsciBleGetBufferFromUint16Value(maxReadBytes, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientReadUsingCharacteristicUuidCmdMonitor(deviceId_t deviceId,
                                                            bleUuidType_t uuidType,
                                                            bleUuid_t *pUuid,
                                                            gattHandleRange_t *pHandleRange,
                                                            uint8_t *aOutBuffer,
                                                            uint16_t maxReadBytes,
                                                            uint16_t *pOutActualReadBytes)
{
    bool_t bHandleRangeIncluded = (NULL == pHandleRange) ? FALSE : TRUE;
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(
        gBleGattCmdClientReadUsingCharacteristicUuidOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(bleUuidType_t) + fsciBleGetUuidBufferSize(uuidType) +
            sizeof(bool_t) + ((TRUE == bHandleRangeIncluded) ? fsciBleGattGetHandleRangeBufferSize(pHandleRange) : 0) +
            sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromEnumValue(uuidType, pBuffer, bleUuidType_t);
    fsciBleGetBufferFromUuid(pUuid, &pBuffer, uuidType);
    fsciBleGetBufferFromBoolValue(bHandleRangeIncluded, pBuffer);

    if (TRUE == bHandleRangeIncluded)
    {
        fsciBleGattGetBufferFromHandleRange(pHandleRange, &pBuffer);
    }

    fsciBleGetBufferFromUint16Value(maxReadBytes, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientReadMultipleCharacteristicValuesCmdMonitor(deviceId_t deviceId,
                                                                 uint8_t cNumCharacteristics,
                                                                 gattCharacteristic_t *aIoCharacteristics)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;
    uint16_t dataSize = fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint8_t);
    uint32_t iCount;

    for (iCount = 0; iCount < cNumCharacteristics; iCount++)
    {
        dataSize += fsciBleGattClientGetCharacteristicBufferSize(&aIoCharacteristics[iCount]);
    }

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientReadMultipleCharacteristicValuesOpCode_c, dataSize);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint8Value(cNumCharacteristics, pBuffer);

    for (iCount = 0; iCount < cNumCharacteristics; iCount++)
    {
        fsciBleGattClientGetBufferFromCharacteristic(&aIoCharacteristics[iCount], &pBuffer);
    }

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientWriteCharacteristicValueCmdMonitor(deviceId_t deviceId,
                                                         gattCharacteristic_t *pCharacteristic,
                                                         uint16_t valueLength,
                                                         uint8_t *aValue,
                                                         bool_t withoutResponse,
                                                         bool_t signedWrite,
                                                         bool_t doReliableLongCharWrites,
                                                         uint8_t *aCsrk)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(
        gBleGattCmdClientWriteCharacteristicValueOpCode_c,
        fsciBleGetDeviceIdBufferSize(&deviceId) + fsciBleGattClientGetCharacteristicBufferSize(pCharacteristic) +
            sizeof(uint16_t) + valueLength + sizeof(bool_t) + sizeof(bool_t) + sizeof(bool_t) + gcSmpCsrkSize_c);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromCharacteristic(pCharacteristic, &pBuffer);
    fsciBleGetBufferFromUint16Value(valueLength, pBuffer);
    fsciBleGetBufferFromArray(aValue, pBuffer, valueLength);
    fsciBleGetBufferFromBoolValue(withoutResponse, pBuffer);
    fsciBleGetBufferFromBoolValue(signedWrite, pBuffer);
    fsciBleGetBufferFromBoolValue(doReliableLongCharWrites, pBuffer);
    fsciBleGetBufferFromCsrk(aCsrk, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientReadCharacteristicDescriptorCmdMonitor(deviceId_t deviceId,
                                                             gattAttribute_t *pIoDescriptor,
                                                             uint16_t maxReadBytes)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGattAllocFsciPacket(gBleGattCmdClientReadCharacteristicDescriptorsOpCode_c,
                                   fsciBleGetDeviceIdBufferSize(&deviceId) +
                                       fsciBleGattClientGetAttributeBufferSize(pIoDescriptor) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromAttribute(pIoDescriptor, &pBuffer);
    fsciBleGetBufferFromUint16Value(maxReadBytes, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientWriteCharacteristicDescriptorCmdMonitor(deviceId_t deviceId,
                                                              gattAttribute_t *pDescriptor,
                                                              uint16_t valueLength,
                                                              uint8_t *aValue)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(gBleGattCmdClientWriteCharacteristicDescriptorsOpCode_c,
                                               fsciBleGetDeviceIdBufferSize(&deviceId) +
                                                   fsciBleGattClientGetAttributeBufferSize(pDescriptor) +
                                                   sizeof(uint16_t) + valueLength);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromAttribute(pDescriptor, &pBuffer);
    fsciBleGetBufferFromUint16Value(valueLength, pBuffer);
    fsciBleGetBufferFromArray(aValue, pBuffer, valueLength);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattClientFindIncludedServicesOrCharacteristicsCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                      deviceId_t deviceId,
                                                                      gattService_t *pIoService,
                                                                      uint8_t maxCount)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket =
        fsciBleGattAllocFsciPacket(opCode, fsciBleGetDeviceIdBufferSize(&deviceId) +
                                               fsciBleGattClientGetServiceBufferSize(pIoService) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGattClientGetBufferFromService(pIoService, &pBuffer);
    fsciBleGetBufferFromUint8Value(maxCount, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattServerRegisterCallbackCmdMonitor(gattServerCallback_t callback)
{
    fsciBleGattCallbacks.serverCallback = callback;

    fsciBleGattNoParamCmdMonitor(gBleGattCmdServerRegisterCallbackOpCode_c);
}

void fsciBleGattServerRegisterHandlesForWriteOrReadNotificationsCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                           uint8_t handleCount,
                                                                           uint16_t *aAttributeHandles)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(opCode, sizeof(uint8_t) + handleCount * sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromUint8Value(handleCount, pBuffer);
    fsciBleGetBufferFromArray(aAttributeHandles, pBuffer, handleCount * sizeof(uint16_t));

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattServerSendAttributeWrittenOrReadStatusCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                 deviceId_t deviceId,
                                                                 uint16_t attributeHandle,
                                                                 uint8_t status)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(
        opCode, fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint8_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(attributeHandle, pBuffer);
    fsciBleGetBufferFromUint8Value(status, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattServerSendNotificationOrIndicationCmdMonitor(fsciBleGattOpCode_t opCode,
                                                             deviceId_t deviceId,
                                                             uint16_t handle)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(opCode, fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t));

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(handle, pBuffer);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

void fsciBleGattServerSendInstantValueNotificationOrIndicationCmdMonitor(
    fsciBleGattOpCode_t opCode, deviceId_t deviceId, uint16_t handle, uint16_t valueLength, uint8_t *pValue)
{
    clientPacketStructured_t *pClientPacket;
    uint8_t *pBuffer;

    /* Allocate the packet to be sent over UART */
    pClientPacket = fsciBleGattAllocFsciPacket(
        opCode, fsciBleGetDeviceIdBufferSize(&deviceId) + sizeof(uint16_t) + sizeof(uint16_t) + valueLength);

    if (NULL == pClientPacket)
    {
        return;
    }

    pBuffer = &pClientPacket->payload[0];

    /* Set command parameters in the buffer */
    fsciBleGetBufferFromDeviceId(&deviceId, &pBuffer);
    fsciBleGetBufferFromUint16Value(handle, pBuffer);
    fsciBleGetBufferFromUint16Value(valueLength, pBuffer);
    fsciBleGetBufferFromArray(pValue, pBuffer, valueLength);

    /* Transmit the packet over UART */
    FSCI_transmitFormatedPacket(pClientPacket, fsciBleInterfaceId);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void fsciBleGattClientProcedureCallback(deviceId_t deviceId,
                                               gattProcedureType_t procedureType,
                                               gattProcedureResult_t procedureResult,
                                               bleResult_t error)
{
}

static void fsciBleGattClientNotificationCallback(deviceId_t deviceId,
                                                  uint16_t characteristicValueHandle,
                                                  uint8_t *aValue,
                                                  uint16_t valueLength)
{
}

static void fsciBleGattClientIndicationCallback(deviceId_t deviceId,
                                                uint16_t characteristicValueHandle,
                                                uint8_t *aValue,
                                                uint16_t valueLength)
{
}

static void fsciBleGattServerCallback(deviceId_t deviceId, gattServerEvent_t *pServerEvent)
{
}

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
