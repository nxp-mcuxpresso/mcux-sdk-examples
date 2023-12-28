/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_BLE_GATT_H
#define _FSCI_BLE_GATT_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_gatt_types.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/*! FSCI operation group for GATT */
#define gFsciBleGattOpcodeGroup_c 0x44

/*! Macros used for monitoring commands, statuses and events */
#define FsciGattCmdMonitor(function, ...) fsciBleGatt##function##CmdMonitor(__VA_ARGS__)

/*! *********************************************************************************
 * \brief  Allocates a FSCI packet for GATT.
 *
 * \param[in]    opCode      FSCI GATT operation code
 * \param[in]    dataSize    Size of the payload
 *
 * \return The allocated FSCI packet
 *
 ********************************************************************************** */
#define fsciBleGattAllocFsciPacket(opCode, dataSize) \
    fsciBleAllocFsciPacket(gFsciBleGattOpcodeGroup_c, (opCode), (dataSize))

/*! *********************************************************************************
 * \brief  Gatt_Init command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGattInitCmdMonitor() fsciBleGattNoParamCmdMonitor(gBleGattCmdInitOpCode_c)

/*! *********************************************************************************
 * \brief  GattClient_Init command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGattClientInitCmdMonitor() fsciBleGattNoParamCmdMonitor(gBleGattCmdClientInitOpCode_c)

/*! *********************************************************************************
 * \brief  GattClient_ResetProcedure command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGattClientResetProcedureCmdMonitor() \
    fsciBleGattNoParamCmdMonitor(gBleGattCmdClientResetProceduresOpCode_c)

/*! *********************************************************************************
 * \brief  GattClient_ExchangeMtu command monitoring macro.
 *
 * \param[in]    deviceId    Device ID of the connected peer.
 *
 ********************************************************************************** */
#define fsciBleGattClientExchangeMtuCmdMonitor(deviceId) \
    fsciBleGattMtuCmdMonitor(gBleGattCmdClientExchangeMtuOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  GattClient_FindIncludedServices command monitoring macro.
 *
 * \param[in]    deviceId            Device ID of the connected peer.
 * \param[in]    pIoService          The service within which inclusions should be
 *                                       searched.
 * \param[in]    maxServiceCount     Maximum number of included services to be filled.
 *
 ********************************************************************************** */
#define fsciBleGattClientFindIncludedServicesCmdMonitor(deviceId, pIoService, maxServiceCount)                      \
    fsciBleGattClientFindIncludedServicesOrCharacteristicsCmdMonitor(gBleGattCmdClientFindIncludedServicesOpCode_c, \
                                                                     (deviceId), (pIoService), (maxServiceCount))

/*! *********************************************************************************
 * \brief  GattClient_DiscoverAllCharacteristicsOfService command monitoring macro.
 *
 * \param[in]    deviceId                  Device ID of the connected peer.
 * \param[in]    pIoService                The service within which characteristics
 *                                         should be searched.
 * \param[in]    maxCharacteristicCount    Maximum number of characteristics to be filled.
 *
 ********************************************************************************** */
#define fsciBleGattClientDiscoverAllCharacteristicsOfServiceCmdMonitor(deviceId, pIoService, maxCharacteristicCount) \
    fsciBleGattClientFindIncludedServicesOrCharacteristicsCmdMonitor(                                                \
        gBleGattCmdClientDiscoverAllCharacteristicsOfServiceOpCode_c, (deviceId), (pIoService),                      \
        (maxCharacteristicCount))

/*! *********************************************************************************
 * \brief  GattServer_Init command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGattServerInitCmdMonitor() fsciBleGattNoParamCmdMonitor(gBleGattCmdServerInitOpCode_c)

/*! *********************************************************************************
 * \brief  GattServer_RegisterHandlesForWriteNotifications command monitoring macro.
 *
 * \param[in]    handleCount         Number of handles in array.
 * \param[in]    aAttributeHandles   Statically allocated array of handles.
 *
 ********************************************************************************** */
#define fsciBleGattServerRegisterHandlesForWriteNotificationsCmdMonitor(handleCount, aAttributeHandles) \
    fsciBleGattServerRegisterHandlesForWriteOrReadNotificationsCmdMonitor(                              \
        gBleGattCmdServerRegisterHandlesForWriteNotificationsOpCode_c, (handleCount), (aAttributeHandles))

/*! *********************************************************************************
 * \brief  GattServer_RegisterHandlesForReadNotifications command monitoring macro.
 *
 * \param[in]    handleCount         Number of handles in array.
 * \param[in]    aAttributeHandles   Statically allocated array of handles.
 *
 ********************************************************************************** */
#define fsciBleGattServerRegisterHandlesForReadNotificationsCmdMonitor(handleCount, aAttributeHandles) \
    fsciBleGattServerRegisterHandlesForWriteOrReadNotificationsCmdMonitor(                             \
        gBleGattCmdServerRegisterHandlesForReadNotificationsOpCode_c, (handleCount), (aAttributeHandles))

/*! *********************************************************************************
 * \brief  GattServer_SendAttributeWrittenStatus command monitoring macro.
 *
 * \param[in]    deviceId            The device ID of the connected peer.
 * \param[in]    attributeHandle     The attribute handle that was written.
 * \param[in]    status              The status of the write operation.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendAttributeWrittenStatusCmdMonitor(deviceId, attributeHandle, status)                     \
    fsciBleGattServerSendAttributeWrittenOrReadStatusCmdMonitor(gBleGattCmdServerSendAttributeWrittenStatusOpCode_c, \
                                                                (deviceId), (attributeHandle), (status))

/*! *********************************************************************************
 * \brief  GattServer_SendAttributeReadStatus command monitoring macro.
 *
 * \param[in]    deviceId            The device ID of the connected peer.
 * \param[in]    attributeHandle     The attribute handle that was written.
 * \param[in]    status              The status of the write operation.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendAttributeReadStatusCmdMonitor(deviceId, attributeHandle, status)                     \
    fsciBleGattServerSendAttributeWrittenOrReadStatusCmdMonitor(gBleGattCmdServerSendAttributeReadStatusOpCode_c, \
                                                                (deviceId), (attributeHandle), (status))

/*! *********************************************************************************
 * \brief  GattServer_SendNotification command monitoring macro.
 *
 * \param[in]    deviceId    The device ID of the connected peer.
 * \param[in]    handle      Handle of the Value of the Characteristic to be notified.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendNotificationCmdMonitor(deviceId, handle)                                              \
    fsciBleGattServerSendNotificationOrIndicationCmdMonitor(gBleGattCmdServerSendNotificationOpCode_c, (deviceId), \
                                                            (handle))

/*! *********************************************************************************
 * \brief  GattServer_SendIndication command monitoring macro.
 *
 * \param[in]    deviceId    The device ID of the connected peer.
 * \param[in]    handle      Handle of the Value of the Characteristic to be notified.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendIndicationCmdMonitor(deviceId, handle)                                              \
    fsciBleGattServerSendNotificationOrIndicationCmdMonitor(gBleGattCmdServerSendIndicationOpCode_c, (deviceId), \
                                                            (handle))

/*! *********************************************************************************
 * \brief  GattServer_SendInstantValueNotification command monitoring macro.
 *
 * \param[in]    opCode          GATT event operation code.
 * \param[in]    deviceId        The device ID of the connected peer.
 * \param[in]    handle          Handle of the Value of the Characteristic to be notified.
 * \param[in]    valueLength     Length of data to be notified.
 * \param[in]    pValue          Data to be notified.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendInstantValueNotificationCmdMonitor(deviceId, handle, valueLength, pValue) \
    fsciBleGattServerSendInstantValueNotificationOrIndicationCmdMonitor(                               \
        gBleGattCmdServerSendInstantValueNotificationOpCode_c, (deviceId), (handle), (valueLength), (pValue))

/*! *********************************************************************************
 * \brief  GattServer_SendInstantValueIndication command monitoring macro.
 *
 * \param[in]    opCode          GATT event operation code.
 * \param[in]    deviceId        The device ID of the connected peer.
 * \param[in]    handle          Handle of the Value of the Characteristic to be indicated.
 * \param[in]    valueLength     Length of data to be indicated.
 * \param[in]    pValue          Data to be indicated.
 *
 ********************************************************************************** */
#define fsciBleGattServerSendInstantValueIndicationCmdMonitor(deviceId, handle, valueLength, pValue) \
    fsciBleGattServerSendInstantValueNotificationOrIndicationCmdMonitor(                             \
        gBleGattCmdServerSendInstantValueIndicationOpCode_c, (deviceId), (handle), (valueLength), (pValue))

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! FSCI operation codes for GATT */
typedef enum
{
    gBleGattModeSelectOpCode_c = 0x00, /*! GAP Mode Select operation code */

    gBleGattCmdFirstOpCode_c = 0x01,
    gBleGattCmdInitOpCode_c  = gBleGattCmdFirstOpCode_c, /*! Gatt_Init command operation code */
    gBleGattCmdGetMtuOpCode_c,                           /*! Gatt_GetMtu command operation code */

    gBleGattCmdClientInitOpCode_c,                       /*! GattClient_Init command operation code */
    gBleGattCmdClientResetProceduresOpCode_c,            /*! GattClient_ResetProcedures command operation code */
    gBleGattCmdClientRegisterProcedureCallbackOpCode_c,  /*! GattClient_RegisterProcedureCallback command operation code
                                                          */
    gBleGattCmdClientRegisterNotificationCallbackOpCode_c, /*! GattClient_RegisterNotificationCallback command operation
                                                              code */
    gBleGattCmdClientRegisterIndicationCallbackOpCode_c,   /*! GattClient_RegisterIndicationCallback command operation
                                                              code */
    gBleGattCmdClientExchangeMtuOpCode_c,                  /*! GattClient_ExchangeMtu command operation code */
    gBleGattCmdClientDiscoverAllPrimaryServicesOpCode_c,   /*! GattClient_DiscoverAllPrimaryServices command operation
                                                              code */
    gBleGattCmdClientDiscoverPrimaryServicesByUuidOpCode_c, /*! GattClient_DiscoverPrimaryServicesByUuid command
                                                               operation code */
    gBleGattCmdClientFindIncludedServicesOpCode_c, /*! GattClient_FindIncludedServices command operation code */
    gBleGattCmdClientDiscoverAllCharacteristicsOfServiceOpCode_c,   /*! GattClient_DiscoverAllCharacteristicsOfService
                                                                       command operation code */
    gBleGattCmdClientDiscoverCharacteristicOfServiceByUuidOpCode_c, /*! GattClient_DiscoverCharacteristicOfServiceByUuid
                                                                       command operation code */
    gBleGattCmdClientDiscoverAllCharacteristicDescriptorsOpCode_c,  /*! GattClient_DiscoverAllCharacteristicDescriptors
                                                                       command operation code */
    gBleGattCmdClientReadCharacteristicValueOpCode_c, /*! GattClient_ReadCharacteristicValue command operation code */
    gBleGattCmdClientReadUsingCharacteristicUuidOpCode_c, /*! GattClient_ReadUsingCharacteristicUuid command operation
                                                             code */
    gBleGattCmdClientReadMultipleCharacteristicValuesOpCode_c, /*! GattClient_ReadMultipleCharacteristicValues command
                                                                  operation code */
    gBleGattCmdClientWriteCharacteristicValueOpCode_c, /*! GattClient_WriteCharacteristicValue command operation code */
    gBleGattCmdClientReadCharacteristicDescriptorsOpCode_c,  /*! GattClient_ReadCharacteristicDescriptors command
                                                                operation code */
    gBleGattCmdClientWriteCharacteristicDescriptorsOpCode_c, /*! GattClient_WriteCharacteristicDescriptors command
                                                                operation code */

    gBleGattCmdServerInitOpCode_c,                           /*! GattServer_Init command operation code */
    gBleGattCmdServerRegisterCallbackOpCode_c,               /*! GattServer_RegisterCallback command operation code */
    gBleGattCmdServerRegisterHandlesForWriteNotificationsOpCode_c, /*! GattServer_RegisterHandlesForWriteNotifications
                                                                      command operation code */
    gBleGattCmdServerSendAttributeWrittenStatusOpCode_c,   /*! GattServer_SendAttributeWrittenStatus command operation
                                                              code */
    gBleGattCmdServerSendNotificationOpCode_c,             /*! GattServer_SendNotification command operation code */
    gBleGattCmdServerSendIndicationOpCode_c,               /*! GattServer_SendIndication command operation code */
    gBleGattCmdServerSendInstantValueNotificationOpCode_c, /*! GattServer_SendInstantValueNotification command operation
                                                              code */
    gBleGattCmdServerSendInstantValueIndicationOpCode_c,   /*! GattServer_SendInstantValueIndication command operation
                                                              code */
    gBleGattCmdServerRegisterHandlesForReadNotificationsOpCode_c, /*! GattServer_RegisterHandlesForReadNotifications
                                                                     command operation code */
    gBleGattCmdServerSendAttributeReadStatusOpCode_c, /*! GattServer_SendAttributeReadStatus command operation code */

    gBleGattStatusOpCode_c = 0x80,                    /*! GAP status operation code */

    gBleGattEvtFirstOpCode_c  = 0x81,
    gBleGattEvtGetMtuOpCode_c = gBleGattEvtFirstOpCode_c, /*! Gatt_GetMtu command out parameter event operation code */

    gBleGattEvtClientProcedureExchangeMtuOpCode_c,        /*! gattClientProcedureCallback (procedureType ==
                                                             gGattProcExchangeMtu_c) event operation code */
    gBleGattEvtClientProcedureDiscoverAllPrimaryServicesOpCode_c,    /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcDiscoverAllPrimaryServices_c) event
                                                                        operation code */
    gBleGattEvtClientProcedureDiscoverPrimaryServicesByUuidOpCode_c, /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcDiscoverPrimaryServicesByUuid_c) event
                                                                        operation code */
    gBleGattEvtClientProcedureFindIncludedServicesOpCode_c,          /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcFindIncludedServices_c) event operation code */
    gBleGattEvtClientProcedureDiscoverAllCharacteristicsOpCode_c,    /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcDiscoverAllCharacteristics_c) event
                                                                        operation code */
    gBleGattEvtClientProcedureDiscoverCharacteristicByUuidOpCode_c,  /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcDiscoverCharacteristicByUuid_c) event
                                                                        operation code */
    gBleGattEvtClientProcedureDiscoverAllCharacteristicDescriptorsOpCode_c, /*! gattClientProcedureCallback
                                                                               (procedureType ==
                                                                               gGattProcDiscoverAllCharacteristicDescriptors_c)
                                                                               event operation code */
    gBleGattEvtClientProcedureReadCharacteristicValueOpCode_c,     /*! gattClientProcedureCallback (procedureType ==
                                                                      gGattProcReadCharacteristicValue_c) event operation
                                                                      code */
    gBleGattEvtClientProcedureReadUsingCharacteristicUuidOpCode_c, /*! gattClientProcedureCallback (procedureType ==
                                                                      gGattProcReadUsingCharacteristicUuid_c) event
                                                                      operation code */
    gBleGattEvtClientProcedureReadMultipleCharacteristicValuesOpCode_c, /*! gattClientProcedureCallback (procedureType
                                                                           ==
                                                                           gGattProcReadMultipleCharacteristicValues_c)
                                                                           event operation code */
    gBleGattEvtClientProcedureWriteCharacteristicValueOpCode_c,      /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcWriteCharacteristicValue_c) event operation
                                                                        code */
    gBleGattEvtClientProcedureReadCharacteristicDescriptorOpCode_c,  /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcReadCharacteristicDescriptor_c) event
                                                                        operation code */
    gBleGattEvtClientProcedureWriteCharacteristicDescriptorOpCode_c, /*! gattClientProcedureCallback (procedureType ==
                                                                        gGattProcWriteCharacteristicDescriptor_c) event
                                                                        operation code */
    gBleGattEvtClientNotificationOpCode_c, /*! gattClientNotificationCallback event operation code */
    gBleGattEvtClientIndicationOpCode_c,   /*! gattClientIndicationCallback event operation code */

    gBleGattEvtServerMtuChangedOpCode_c, /*! gattServerCallback (eventType == gEvtMtuChanged_c) event operation code */
    gBleGattEvtServerHandleValueConfirmationOpCode_c, /*! gattServerCallback (eventType ==
                                                         gEvtHandleValueConfirmation_c) event operation code */
    gBleGattEvtServerAttributeWrittenOpCode_c,        /*! gattServerCallback (eventType == gEvtAttributeWritten_c) event
                                                         operation code */
    gBleGattEvtServerCharacteristicCccdWrittenOpCode_c,       /*! gattServerCallback (eventType ==
                                                                 gEvtCharacteristicCccdWritten_c) event operation code */
    gBleGattEvtServerAttributeWrittenWithoutResponseOpCode_c, /*! gattServerCallback (eventType ==
                                                                 gEvtAttributeWrittenWithoutResponse_c) event operation
                                                                 code */
    gBleGattEvtServerErrorOpCode_c, /*! gattServerCallback (eventType == gEvtErrorOpCode_c) event operation code */
    gBleGattEvtServerLongCharacteristicWrittenOpCode_c, /*! gattServerCallback (eventType ==
                                                           gEvtLongCharacteristicWritten_c) event operation code */
    gBleGattEvtServerAttributeReadOpCode_c /*! gattServerCallback (eventType == gEvtAttributeRead_c) event operation
                                              code */
} fsciBleGattOpCode_t;

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

/*! *********************************************************************************
* \brief  Calls the GATT function associated with the operation code received over UART.
*         The GATT function parameters are extracted from the received FSCI payload.
*
* \param[in]    pData               Packet (containing FSCI header and FSCI
                                    payload) received over UART
* \param[in]    param               Pointer given when this function is registered in
                                    FSCI
* \param[in]    fsciInterfaceId     FSCI interface on which the packet was received
*
********************************************************************************** */
void fsciBleGattHandler(void *pData, void *pParam, uint32_t fsciInterfaceId);

/*! *********************************************************************************
 * \brief  Creates a GATT FSCI packet without payload and sends it over UART.
 *
 * \param[in]    opCode      GATT command operation code.
 *
 ********************************************************************************** */
void fsciBleGattNoParamCmdMonitor(fsciBleGattOpCode_t opCode);

/*! *********************************************************************************
 * \brief  Gatt_GetMtu and GattClient_ExchangeMtu commands monitoring function.
 *
 * \param[in]    opCode          GATT command operation code.
 * \param[in]    deviceId        The device ID of the connected peer.
 *
 ********************************************************************************** */
void fsciBleGattMtuCmdMonitor(fsciBleGattOpCode_t opCode, deviceId_t deviceId);

/*! *********************************************************************************
 * \brief  Gatt_GetMtu command monitoring function.
 *
 * \param[in]    deviceId    The device ID of the connected peer.
 * \param[in]    pOutMtu     Pointer to integer to be written.
 *
 ********************************************************************************** */
void fsciBleGattGetMtuCmdMonitor(deviceId_t deviceId, uint16_t *pOutMtu);

/*! *********************************************************************************
*
{brief  GattClient_RegisterProcedureCallback command monitoring function.
*
*
{param[in]    callback    Application defined callback to be triggered by GATT module.
*
********************************************************************************** */
void fsciBleGattClientRegisterProcedureCallbackCmdMonitor(gattClientProcedureCallback_t callback);

/*! *********************************************************************************
*
{brief  GattClient_RegisterNotificationCallback command monitoring function.
*
*
{param[in]    callback    Application defined callback to be triggered by GATT module.
*
********************************************************************************** */
void fsciBleGattClientRegisterNotificationCallbackCmdMonitor(gattClientNotificationCallback_t callback);

/*! *********************************************************************************
*
{brief  GattClient_RegisterIndicationCallback command monitoring function.
*
*
{param[in]    callback    Application defined callback to be triggered by GATT module.
*
********************************************************************************** */
void fsciBleGattClientRegisterIndicationCallbackCmdMonitor(gattClientIndicationCallback_t callback);

/*! *********************************************************************************
 * \brief  GattClient_DiscoverAllPrimaryServices command monitoring function.
 *
 * \param[in]    deviceId                Device ID of the connected peer.
 * \param[in]    aOutPrimaryServices     Statically allocated array of gattService_t.
 * \param[in]    maxServiceCount         Maximum number of services to be filled.
 * \param[in]    pOutDiscoveredCount     The actual number of services discovered.
 *
 ********************************************************************************** */
void fsciBleGattClientDiscoverAllPrimaryServicesCmdMonitor(deviceId_t deviceId,
                                                           gattService_t *pOutPrimaryServices,
                                                           uint8_t maxServiceCount,
                                                           uint8_t *pOutDiscoveredCount);

/*! *********************************************************************************
 * \brief  GattClient_DiscoverPrimaryServicesByUuid command monitoring function.
 *
 * \param[in]    deviceId              Device ID of the connected peer.
 * \param[in]    uuidType              Service UUID type.
 * \param[in]    pUuid                 Service UUID.
 * \param[in]    aOutPrimaryServices   Statically allocated array of gattService_t.
 * \param[in]    maxServiceCount       Maximum number of services to be filled.
 * \param[in]    pOutDiscoveredCount   The actual number of services discovered.
 *
 ********************************************************************************** */
void fsciBleGattClientDiscoverPrimaryServicesByUuidCmdMonitor(deviceId_t deviceId,
                                                              bleUuidType_t uuidType,
                                                              bleUuid_t *pUuid,
                                                              gattService_t *aOutPrimaryServices,
                                                              uint8_t maxServiceCount,
                                                              uint8_t *pOutDiscoveredCount);

/*! *********************************************************************************
 * \brief  GattClient_DiscoverCharacteristicOfServiceByUuid command monitoring function.
 *
 * \param[in]    deviceId                  Device ID of the connected peer.
 * \param[in]    uuidType                  Characteristic UUID type.
 * \param[in]    pUuid                     Characteristic UUID.
 * \param[in]    pService                  The service within which characteristics should be searched.
 * \param[in]    aOutCharacteristics       The allocated array of Characteristics to be filled.
 * \param[in]    maxCharacteristicCount    Maximum number of characteristics to be filled.
 * \param[in]    pOutDiscoveredCount       The actual number of characteristics discovered.
 *
 ********************************************************************************** */
void fsciBleGattClientDiscoverCharacteristicOfServiceByUuidCmdMonitor(deviceId_t deviceId,
                                                                      bleUuidType_t uuidType,
                                                                      bleUuid_t *pUuid,
                                                                      gattService_t *pIoService,
                                                                      gattCharacteristic_t *aOutCharacteristics,
                                                                      uint8_t maxCharacteristicCount,
                                                                      uint8_t *pOutDiscoveredCount);

/*! *********************************************************************************
 * \brief  GattClient_DiscoverAllCharacteristicDescriptors command monitoring function.
 *
 * \param[in]    deviceId            Device ID of the connected peer.
 * \param[in]    pIoCharacteristic   The characteristic within which descriptors should be searched.
 * \param[in]    endingHandle        The last handle of the Characteristic.
 * \param[in]    maxDescriptorCount  Maximum number of descriptors to be filled.
 *
 ********************************************************************************** */
void fsciBleGattClientDiscoverAllCharacteristicDescriptorsCmdMonitor(deviceId_t deviceId,
                                                                     gattCharacteristic_t *pIoCharacteristic,
                                                                     uint16_t endingHandle,
                                                                     uint8_t maxDescriptorCount);

/*! *********************************************************************************
 * \brief  GattClient_ReadCharacteristicValue command monitoring function.
 *
 * \param[in]    deviceId            Device ID of the connected peer.
 * \param[in]    pIoCharacteristic   The characteristic whose value must be read.
 * \param[in]    maxReadBytes        Maximum number of bytes to be read.
 *
 ********************************************************************************** */
void fsciBleGattClientReadCharacteristicValueCmdMonitor(deviceId_t deviceId,
                                                        gattCharacteristic_t *pIoCharacteristic,
                                                        uint16_t maxReadBytes);

/*! *********************************************************************************
 * \brief  GattClient_ReadUsingCharacteristicUuid command monitoring function.
 *
 * \param[in]    deviceId                Device ID of the connected peer.
 * \param[in]    uuidType                Characteristic UUID type.
 * \param[in]    pUuid                   Characteristic UUID.
 * \param[in]    aOutBuffer              The allocated buffer to read into.
 * \param[in]    maxReadBytes            Maximum number of bytes to be read.
 * \param[in]    pOutActualReadBytes     The actual number of bytes read.
 *
 ********************************************************************************** */
void fsciBleGattClientReadUsingCharacteristicUuidCmdMonitor(deviceId_t deviceId,
                                                            bleUuidType_t uuidType,
                                                            bleUuid_t *pUuid,
                                                            gattHandleRange_t *pHandleRange,
                                                            uint8_t *aOutBuffer,
                                                            uint16_t maxReadBytes,
                                                            uint16_t *pOutActualReadBytes);

/*! *********************************************************************************
 * \brief  GattClient_ReadMultipleCharacteristicValues command monitoring function.
 *
 * \param[in]    deviceId                Device ID of the connected peer.
 * \param[in]    aIoCharacteristics      Array of the characteristics whose values
 *                                       are to be read.
 * \param[in]    cNumCharacteristics     Number of characteristics in the array.
 *
 ********************************************************************************** */
void fsciBleGattClientReadMultipleCharacteristicValuesCmdMonitor(deviceId_t deviceId,
                                                                 uint8_t cNumCharacteristics,
                                                                 gattCharacteristic_t *aIoCharacteristics);

/*! *********************************************************************************
 * \brief  GattClient_WriteCharacteristicValue command monitoring function.
 *
 * \param[in]  deviceId                  Device ID of the connected peer.
 * \param[in]  pCharacteristic           The characteristic whose value must be written.
 * \param[in]  valueLength               Number of bytes to be written.
 * \param[in]  aValue                    Array of bytes to be written.
 * \param[in]  withoutResponse           Indicates if a Write Command will be used.
 * \param[in]  signedWrite               Indicates if a Signed Write will be performed.
 * \param[in]  doReliableLongCharWrites  Indicates Reliable Long Writes.
 * \param[in]  aCsrk                     The CSRK (gcCsrkSize_d bytes).
 *
 ********************************************************************************** */
void fsciBleGattClientWriteCharacteristicValueCmdMonitor(deviceId_t deviceId,
                                                         gattCharacteristic_t *pCharacteristic,
                                                         uint16_t valueLength,
                                                         uint8_t *aValue,
                                                         bool_t withoutResponse,
                                                         bool_t signedWrite,
                                                         bool_t doReliableLongCharWrites,
                                                         uint8_t *aCsrk);

/*! *********************************************************************************
 * \brief  GattClient_ReadCharacteristicDescriptor command monitoring function.
 *
 * \param[in]  deviceId          Device ID of the connected peer.
 * \param[in]  pIoDescriptor     The characteristic descriptor whose value must be read.
 * \param[in]  maxReadBytes      Maximum number of bytes to be read.
 *
 ********************************************************************************** */
void fsciBleGattClientReadCharacteristicDescriptorCmdMonitor(deviceId_t deviceId,
                                                             gattAttribute_t *pIoDescriptor,
                                                             uint16_t maxReadBytes);

/*! *********************************************************************************
 * \brief  GattClient_ReadCharacteristicDescriptor command monitoring function.
 *
 * \param[in]  deviceId      Device ID of the connected peer.
 * \param[in]  pDescriptor   The characteristic descriptor whose value must be written.
 * \param[in]  valueLength   Number of bytes to be written.
 * \param[in]  aValue        Array of bytes to be written.
 *
 ********************************************************************************** */
void fsciBleGattClientWriteCharacteristicDescriptorCmdMonitor(deviceId_t deviceId,
                                                              gattAttribute_t *pDescriptor,
                                                              uint16_t valueLength,
                                                              uint8_t *aValue);

/*! *********************************************************************************
 * \brief  GattClient_FindIncludedServices and GattClient_DiscoverAllCharacteristicsOfService
 *         commands monitoring function.
 *
 * \param[in]    opCode      GATT event operation code.
 * \param[in]    deviceId    Device ID of the connected peer.
 * \param[in]    pIoService  The service within which inclusions or characteristics
 *                           should be searched.
 * \param[in]    maxCount    Maximum number of included services or characteristics.
 *
 ********************************************************************************** */
void fsciBleGattClientFindIncludedServicesOrCharacteristicsCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                      deviceId_t deviceId,
                                                                      gattService_t *pIoService,
                                                                      uint8_t maxCount);

/*! *********************************************************************************
 * \brief  GattServer_RegisterCallback command monitoring function.
 *
 * \param[in]    callback    Application-defined callback to be triggered by GATT module.
 *
 ********************************************************************************** */
void fsciBleGattServerRegisterCallbackCmdMonitor(gattServerCallback_t callback);

/*! *********************************************************************************
 * \brief  GattServer_RegisterHandlesForWriteNotifications and
 *         GattServer_RegisterHandlesForReadNotifications commands monitoring function.
 *
 * \param[in]    opCode              GATT event operation code.
 * \param[in]    handleCount         Number of handles in array.
 * \param[in]    aAttributeHandles   Statically allocated array of handles.
 *
 ********************************************************************************** */
void fsciBleGattServerRegisterHandlesForWriteOrReadNotificationsCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                           uint8_t handleCount,
                                                                           uint16_t *aAttributeHandles);

/*! *********************************************************************************
 * \brief  GattServer_SendAttributeWrittenStatus and GattServer_SendAttributeReadStatus
 *         commands monitoring function.
 *
 * \param[in]    opCode              GATT event operation code.
 * \param[in]    deviceId            The device ID of the connected peer.
 * \param[in]    attributeHandle     The attribute handle that was written.
 * \param[in]    status              The status of the write operation.
 *
 ********************************************************************************** */
void fsciBleGattServerSendAttributeWrittenOrReadStatusCmdMonitor(fsciBleGattOpCode_t opCode,
                                                                 deviceId_t deviceId,
                                                                 uint16_t attributeHandle,
                                                                 uint8_t status);

/*! *********************************************************************************
 * \brief  GattServer_SendNotification and GattServer_SendIndication commands
 *         monitoring function.
 *
 * \param[in]    opCode      GATT event operation code.
 * \param[in]    deviceId    The device ID of the connected peer.
 * \param[in]    handle      Handle of the Value of the Characteristic to be notified
 *                           or indicated.
 *
 ********************************************************************************** */
void fsciBleGattServerSendNotificationOrIndicationCmdMonitor(fsciBleGattOpCode_t opCode,
                                                             deviceId_t deviceId,
                                                             uint16_t handle);

/*! *********************************************************************************
 * \brief  GattServer_SendInstantValueNotification and GattServer_SendInstantValueIndication
 *         commands monitoring function.
 *
 * \param[in]    opCode          GATT event operation code.
 * \param[in]    deviceId        The device ID of the connected peer.
 * \param[in]    handle          Handle of the Value of the Characteristic to be notified
 *                               or indicated.
 * \param[in]    valueLength     Length of data to be notified or indicated.
 * \param[in]    pValue          Data to be notified or indicated.
 *
 ********************************************************************************** */
void fsciBleGattServerSendInstantValueNotificationOrIndicationCmdMonitor(
    fsciBleGattOpCode_t opCode, deviceId_t deviceId, uint16_t handle, uint16_t valueLength, uint8_t *pValue);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_ATT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
