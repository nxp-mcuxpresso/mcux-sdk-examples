/*! *********************************************************************************
 * \addtogroup HOST_BBOX_UTILITY
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

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_gatt.h"
#include "host_ble.h"

#if gFsciIncluded_c
#include "FsciCommunication.h"
#endif

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

bleResult_t Gatt_Init(void)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(Init);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t Gatt_GetMtu(deviceId_t deviceId, uint16_t *pOutMtu)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, TRUE);
    FsciGattCmdMonitor(GetMtu, deviceId, pOutMtu);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_Init(void)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientInit);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ResetProcedure(void)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientResetProcedure);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_RegisterProcedureCallback(gattClientProcedureCallback_t callback)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientRegisterProcedureCallback, callback);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_RegisterNotificationCallback(gattClientNotificationCallback_t callback)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientRegisterNotificationCallback, callback);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_RegisterIndicationCallback(gattClientIndicationCallback_t callback)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientRegisterIndicationCallback, callback);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ExchangeMtu(deviceId_t deviceId)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientExchangeMtu, deviceId);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_DiscoverAllPrimaryServices(deviceId_t deviceId,
                                                  gattService_t *aOutPrimaryServices,
                                                  uint8_t maxServiceCount,
                                                  uint8_t *pOutDiscoveredCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveServicesInfo(deviceId, aOutPrimaryServices);
    fsciBleGattClientSaveArraySizeInfo(deviceId, (uint16_t *)pOutDiscoveredCount);
    FsciGattCmdMonitor(ClientDiscoverAllPrimaryServices, deviceId, aOutPrimaryServices, maxServiceCount,
                       pOutDiscoveredCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveServicesInfo(deviceId, NULL);
        fsciBleGattClientSaveArraySizeInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_DiscoverPrimaryServicesByUuid(deviceId_t deviceId,
                                                     bleUuidType_t uuidType,
                                                     bleUuid_t *pUuid,
                                                     gattService_t *aOutPrimaryServices,
                                                     uint8_t maxServiceCount,
                                                     uint8_t *pOutDiscoveredCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveServicesInfo(deviceId, aOutPrimaryServices);
    fsciBleGattClientSaveArraySizeInfo(deviceId, (uint16_t *)pOutDiscoveredCount);
    FsciGattCmdMonitor(ClientDiscoverPrimaryServicesByUuid, deviceId, uuidType, pUuid, aOutPrimaryServices,
                       maxServiceCount, pOutDiscoveredCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveServicesInfo(deviceId, NULL);
        fsciBleGattClientSaveArraySizeInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_FindIncludedServices(deviceId_t deviceId, gattService_t *pIoService, uint8_t maxServiceCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveServicesInfo(deviceId, pIoService);
    FsciGattCmdMonitor(ClientFindIncludedServices, deviceId, pIoService, maxServiceCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveServicesInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_DiscoverAllCharacteristicsOfService(deviceId_t deviceId,
                                                           gattService_t *pIoService,
                                                           uint8_t maxCharacteristicCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveServicesInfo(deviceId, pIoService);
    FsciGattCmdMonitor(ClientDiscoverAllCharacteristicsOfService, deviceId, pIoService, maxCharacteristicCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveServicesInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_DiscoverCharacteristicOfServiceByUuid(deviceId_t deviceId,
                                                             bleUuidType_t uuidType,
                                                             bleUuid_t *pUuid,
                                                             gattService_t *pService,
                                                             gattCharacteristic_t *aOutCharacteristics,
                                                             uint8_t maxCharacteristicCount,
                                                             uint8_t *pOutDiscoveredCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveCharacteristicsInfo(deviceId, aOutCharacteristics);
    fsciBleGattClientSaveArraySizeInfo(deviceId, (uint16_t *)pOutDiscoveredCount);
    FsciGattCmdMonitor(ClientDiscoverCharacteristicOfServiceByUuid, deviceId, uuidType, pUuid, pService,
                       aOutCharacteristics, maxCharacteristicCount, pOutDiscoveredCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveCharacteristicsInfo(deviceId, NULL);
        fsciBleGattClientSaveArraySizeInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_DiscoverAllCharacteristicDescriptors(deviceId_t deviceId,
                                                            gattCharacteristic_t *pIoCharacteristic,
                                                            uint16_t endingHandle,
                                                            uint8_t maxDescriptorCount)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveCharacteristicsInfo(deviceId, pIoCharacteristic);
    FsciGattCmdMonitor(ClientDiscoverAllCharacteristicDescriptors, deviceId, pIoCharacteristic, endingHandle,
                       maxDescriptorCount);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveCharacteristicsInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ReadCharacteristicValue(deviceId_t deviceId,
                                               gattCharacteristic_t *pIoCharacteristic,
                                               uint16_t maxReadBytes)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveCharacteristicsInfo(deviceId, pIoCharacteristic);
    FsciGattCmdMonitor(ClientReadCharacteristicValue, deviceId, pIoCharacteristic, maxReadBytes);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveCharacteristicsInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ReadUsingCharacteristicUuid(deviceId_t deviceId,
                                                   bleUuidType_t uuidType,
                                                   bleUuid_t *pUuid,
                                                   gattHandleRange_t *pHandleRange,
                                                   uint8_t *aOutBuffer,
                                                   uint16_t maxReadBytes,
                                                   uint16_t *pOutActualReadBytes)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveValueInfo(deviceId, aOutBuffer);
    fsciBleGattClientSaveArraySizeInfo(deviceId, pOutActualReadBytes);
    FsciGattCmdMonitor(ClientReadUsingCharacteristicUuid, deviceId, uuidType, pUuid, pHandleRange, aOutBuffer,
                       maxReadBytes, pOutActualReadBytes);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveValueInfo(deviceId, NULL);
        fsciBleGattClientSaveArraySizeInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ReadMultipleCharacteristicValues(deviceId_t deviceId,
                                                        uint8_t cNumCharacteristics,
                                                        gattCharacteristic_t *aIoCharacteristics)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveCharacteristicsInfo(deviceId, aIoCharacteristics);
    FsciGattCmdMonitor(ClientReadMultipleCharacteristicValues, deviceId, cNumCharacteristics, aIoCharacteristics);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveCharacteristicsInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_WriteCharacteristicValue(deviceId_t deviceId,
                                                gattCharacteristic_t *pCharacteristic,
                                                uint16_t valueLength,
                                                uint8_t *aValue,
                                                bool_t withoutResponse,
                                                bool_t signedWrite,
                                                bool_t doReliableLongCharWrites,
                                                uint8_t *aCsrk)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientWriteCharacteristicValue, deviceId, pCharacteristic, valueLength, aValue, withoutResponse,
                       signedWrite, doReliableLongCharWrites, aCsrk);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_ReadCharacteristicDescriptor(deviceId_t deviceId,
                                                    gattAttribute_t *pIoDescriptor,
                                                    uint16_t maxReadBytes)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    /* Save the information that must be monitored when the procedure ends */
    fsciBleGattClientSaveDescriptorsInfo(deviceId, pIoDescriptor);
    FsciGattCmdMonitor(ClientReadCharacteristicDescriptor, deviceId, pIoDescriptor, maxReadBytes);
    result = Ble_GetCmdStatus();

    if (gBleSuccess_c != result)
    {
        fsciBleGattClientSaveDescriptorsInfo(deviceId, NULL);
    }

    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattClient_WriteCharacteristicDescriptor(deviceId_t deviceId,
                                                     gattAttribute_t *pDescriptor,
                                                     uint16_t valueLength,
                                                     uint8_t *aValue)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ClientWriteCharacteristicDescriptor, deviceId, pDescriptor, valueLength, aValue);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_Init(void)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerInit);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_RegisterCallback(gattServerCallback_t callback)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerRegisterCallback, callback);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_RegisterHandlesForWriteNotifications(uint8_t handleCount, uint16_t *aHandles)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerRegisterHandlesForWriteNotifications, handleCount, aHandles);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendAttributeWrittenStatus(deviceId_t deviceId, uint16_t attributeHandle, uint8_t status)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendAttributeWrittenStatus, deviceId, attributeHandle, status);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_RegisterHandlesForReadNotifications(uint8_t handleCount, uint16_t *aHandles)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerRegisterHandlesForReadNotifications, handleCount, aHandles);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendAttributeReadStatus(deviceId_t deviceId, uint16_t attributeHandle, uint8_t status)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendAttributeReadStatus, deviceId, attributeHandle, status);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendNotification(deviceId_t deviceId, uint16_t handle)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendNotification, deviceId, handle);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendIndication(deviceId_t deviceId, uint16_t handle)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendIndication, deviceId, handle);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendInstantValueNotification(deviceId_t deviceId,
                                                    uint16_t handle,
                                                    uint16_t valueLength,
                                                    uint8_t *aValue)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendInstantValueNotification, deviceId, handle, valueLength, aValue);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

bleResult_t GattServer_SendInstantValueIndication(deviceId_t deviceId,
                                                  uint16_t handle,
                                                  uint16_t valueLength,
                                                  uint8_t *aValue)
{
    bleResult_t result = gBleSuccess_c;

    FSCI_HostSyncLock(fsciBleInterfaceId, gFsciBleGattOpcodeGroup_c, gBleGattStatusOpCode_c, FALSE);
    FsciGattCmdMonitor(ServerSendInstantValueIndication, deviceId, handle, valueLength, aValue);
    result = Ble_GetCmdStatus();
    FSCI_HostSyncUnlock(fsciBleInterfaceId);

    return result;
}

/*************************************************************************************
 *************************************************************************************
 * Private functions
 *************************************************************************************
 ************************************************************************************/

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
