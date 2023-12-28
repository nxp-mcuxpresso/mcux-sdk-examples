/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 , NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSCI_BLE_GAP_H
#define _FSCI_BLE_GAP_H

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

#include "fsci_ble_gap_types.h"

/************************************************************************************
*************************************************************************************
* Public constants & macros
*************************************************************************************
************************************************************************************/

/*! FSCI operation group for GAP */
#define gFsciBleGapOpcodeGroup_c 0x47

/*! Macros used for monitoring commands, statuses and events */
#define FsciGapCmdMonitor(function, ...) fsciBleGap##function##CmdMonitor(__VA_ARGS__)

/*! *********************************************************************************
 * \brief  Allocates a FSCI packet for GAP.
 *
 * \param[in]    opCode      FSCI GAP operation code
 * \param[in]    dataSize    Size of the payload
 *
 * \return The allocated FSCI packet
 *
 ********************************************************************************** */
#define fsciBleGapAllocFsciPacket(opCode, dataSize) \
    fsciBleAllocFsciPacket(gFsciBleGapOpcodeGroup_c, (opCode), (dataSize))

/*! *********************************************************************************
 * \brief  Gap_StopAdvertising command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapStopAdvertisingCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdStopAdvertisingOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_StopScanning command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapStopScanningCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdStopScanningOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_ReadWhiteListSize command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapReadWhiteListSizeCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdReadWhiteListSizeOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_ClearWhiteList command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapClearWhiteListCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdClearWhiteListOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_RemoveAllBonds command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapRemoveAllBondsCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdRemoveAllBondsOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_EncryptLink command monitoring macro.
 *
 * \param[in]    deviceId    Device ID of the peer.
 *
 ********************************************************************************** */
#define fsciBleGapEncryptLinkCmdMonitor(deviceId) \
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdEncryptLinkOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  Gap_RemoveBond command monitoring macro.
 *
 * \param[in]    nvmIndex    Index of the device in NVM bonding area.
 *
 ********************************************************************************** */
#define fsciBleGapRemoveBondCmdMonitor(nvmIndex) \
    fsciBleGapUint8ParamCmdMonitor(gBleGapCmdRemoveBondOpCode_c, (nvmIndex))

/*! *********************************************************************************
 * \brief  Gap_RejectPasskeyRequest command monitoring macro.
 *
 * \param[in]    deviceId    The GAP peer that requested a passkey entry.
 *
 ********************************************************************************** */
#define fsciBleGapRejectPasskeyRequestCmdMonitor(deviceId) \
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdRejectPasskeyRequestOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  Gap_RejectKeyExchangeRequest command monitoring macro.
 *
 * \param[in]    deviceId    The GAP peer who requested the Key Exchange procedure.
 *
 ********************************************************************************** */
#define fsciBleGapRejectKeyExchangeRequestCmdMonitor(deviceId) \
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdRejectKeyExchangeRequestOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  Gap_DenyLongTermKey command monitoring macro.
 *
 * \param[in]    deviceId    The GAP peer who requested encryption.
 *
 ********************************************************************************** */
#define fsciBleGapDenyLongTermKeyCmdMonitor(deviceId) \
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdDenyLongTermKeyOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  Gap_Disconnect command monitoring macro.
 *
 * \param[in]    deviceId    The connected peer to disconnect from.
 *
 ********************************************************************************** */
#define fsciBleGapDisconnectCmdMonitor(deviceId) \
    fsciBleGapDeviceIdParamCmdMonitor(gBleGapCmdDisconnectOpCode_c, (deviceId))

/*! *********************************************************************************
 * \brief  Gap_CheckNotificationStatus command monitoring macro.
 *
 * \param[in]    deviceId        The peer GATT Client.
 * \param[in]    handle          The handle of the CCCD.
 * \param[in]    pOutIsActive    The address to store the status into.
 *
 ********************************************************************************** */
#define fsciBleGapCheckNotificationStatusCmdMonitor(deviceId, handle, pOutIsActive)                             \
    fsciBleGapCheckNotificationsAndIndicationsCmdMonitor(gBleGapCmdCheckNotificationStatusOpCode_c, (deviceId), \
                                                         (handle), (pOutIsActive))

/*! *********************************************************************************
 * \brief  Gap_CheckIndicationStatus command monitoring macro.
 *
 * \param[in]    deviceId        The peer GATT Client.
 * \param[in]    handle          The handle of the CCCD.
 * \param[in]    pOutIsActive    The address to store the status into.
 *
 ********************************************************************************** */
#define fsciBleGapCheckIndicationStatusCmdMonitor(deviceId, handle, pOutIsActive)                             \
    fsciBleGapCheckNotificationsAndIndicationsCmdMonitor(gBleGapCmdCheckIndicationStatusOpCode_c, (deviceId), \
                                                         (handle), (pOutIsActive))

/*! *********************************************************************************
 * \brief  Gap_AddDeviceToWhiteList command monitoring macro.
 *
 * \param[in]    address         The address of the white-listed device.
 * \param[in]    addressType     The device address type (public or random).
 *
 ********************************************************************************** */
#define fsciBleGapAddDeviceToWhiteListCmdMonitor(addressType, address) \
    fsciBleGapAddressParamsCmdMonitor(gBleGapCmdAddDeviceToWhiteListOpCode_c, (addressType), (address))

/*! *********************************************************************************
 * \brief  Gap_RemoveDeviceFromWhiteList command monitoring macro.
 *
 * \param[in]    address         The address of the white-listed device.
 * \param[in]    addressType     The device address type (public or random).
 *
 ********************************************************************************** */
#define fsciBleGapRemoveDeviceFromWhiteListCmdMonitor(addressType, address) \
    fsciBleGapAddressParamsCmdMonitor(gBleGapCmdRemoveDeviceFromWhiteListOpCode_c, (addressType), (address))

/*! *********************************************************************************
 * \brief  Gap_ReadPublicDeviceAddress command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapReadPublicDeviceAddressCmdMonitor() \
    fsciBleGapNoParamCmdMonitor(gBleGapCmdReadPublicDeviceAddressOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_ControllerReset command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapControllerResetCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdControllerResetOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_LeScRegeneratePublicKey command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapLeScRegeneratePublicKeyCmdMonitor() \
    fsciBleGapNoParamCmdMonitor(gBleGapCmdLeScRegeneratePublicKeyOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_LeScGetLocalOobData command monitoring macro.
 *
 ********************************************************************************** */
#define fsciBleGapLeScGetLocalOobDataCmdMonitor() fsciBleGapNoParamCmdMonitor(gBleGapCmdLeScGetLocalOobDataOpCode_c)

/*! *********************************************************************************
 * \brief  Gap_CheckNotificationStatus out parameter monitoring macro.
 *
 * \param[in]    pOutIsActive    Pointer to the notification status.
 *
 ********************************************************************************** */
#define fsciBleGapCheckNotificationStatusEvtMonitor(pOutIsActive) \
    fsciBleGapBoolParamEvtMonitor(gBleGapEvtCheckNotificationStatusOpCode_c, *(pOutIsActive))

/*! *********************************************************************************
 * \brief  Gap_CheckIndicationStatus out parameter monitoring macro.
 *
 * \param[in]    pOutIsActive    Pointer to the indication status.
 *
 ********************************************************************************** */
#define fsciBleGapCheckIndicationStatusEvtMonitor(pOutIsActive) \
    fsciBleGapBoolParamEvtMonitor(gBleGapEvtCheckIndicationStatusOpCode_c, *(pOutIsActive))

/*! *********************************************************************************
 * \brief  Gap_CheckIfBonded out parameter monitoring macro.
 *
 * \param[in]    pOutIsActive    Pointer to the bonded flag.
 *
 ********************************************************************************** */
#define fsciBleGapCheckIfBondedEvtMonitor(pOutIsBonded) \
    fsciBleGapBoolParamEvtMonitor(gBleGapEvtCheckIfBondedOpCode_c, *(pOutIsBonded))

/*! *********************************************************************************
 * \brief  Gap_LoadEncryptionInformation out parameters monitoring macro.
 *
 * \param[in]    aOutLtk         Array filled with the LTK.
 * \param[in]    pOutLtkSize     The LTK size.
 *
 ********************************************************************************** */
#define fsciBleGapLoadEncryptionInformationEvtMonitor(aOutLtk, pOutLtkSize) \
    fsciBleGapArrayAndSizeParamEvtMonitor(gBleGapEvtLoadEncryptionInformationOpCode_c, (aOutLtk), *(pOutLtkSize))

/*! *********************************************************************************
 * \brief  Gap_GetBondedDeviceName out parameter monitoring macro.
 *
 * \param[in]    aOutName    Destination array that keeps the device name.
 *
 ********************************************************************************** */
#define fsciBleGapGetBondedDeviceNameEvtMonitor(aOutName)                                               \
    fsciBleGapArrayAndSizeParamEvtMonitor(gBleGapEvtGetBondedDeviceNameOpCode_c, (uint8_t *)(aOutName), \
                                          strlen((char const *)(aOutName)))

/*! *********************************************************************************
 * \brief  Gap_GetBondedDevicesCount out parameter monitoring macro.
 *
 * \param[in]    pOutBondedDevicesCount  Pointer to the number of bonded devices.
 *
 ********************************************************************************** */
#define fsciBleGapGetBondedDevicesCountEvtMonitor(pOutBondedDevicesCount) \
    fsciBleGapArrayAndSizeParamEvtMonitor(gBleGapEvtGetBondedDevicesCountOpCode_c, NULL, *(pOutBondedDevicesCount))

/************************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
************************************************************************************/

/*! FSCI operation codes for GAP */
typedef enum
{
    gBleGapModeSelectOpCode_c = 0x00, /*! GAP Mode Select operation code */

    gBleGapCmdFirstOpCode_c             = 0x01,
    gBleGapCmdBleHostInitializeOpCode_c = gBleGapCmdFirstOpCode_c, /*! Ble_HostInitialize command operation code */
    gBleGapCmdRegisterDeviceSecurityRequirementsOpCode_c, /*! Gap_RegisterDeviceSecurityRequirements command operation
                                                             code */
    gBleGapCmdSetAdvertisingParametersOpCode_c,           /*! Gap_SetAdvertisingParameters command operation code */
    gBleGapCmdSetAdvertisingDataOpCode_c,                 /*! Gap_SetAdvertisingData command operation code */
    gBleGapCmdStartAdvertisingOpCode_c,                   /*! Gap_StartAdvertising command operation code */
    gBleGapCmdStopAdvertisingOpCode_c,                    /*! Gap_StopAdvertising command operation code */
    gBleGapCmdAuthorizeOpCode_c,                          /*! Gap_Authorize command operation code */
    gBleGapCmdSaveCccdOpCode_c,                           /*! Gap_SaveCccd command operation code */
    gBleGapCmdCheckNotificationStatusOpCode_c,            /*! Gap_CheckNotificationStatus command operation code */
    gBleGapCmdCheckIndicationStatusOpCode_c,              /*! Gap_CheckIndicationStatus command operation code */
    gBleGapCmdGetBondedStaticAddressesOpCode_c,           /*! Gap_GetBondedStaticAddresses command operation code */
    gBleGapCmdPairOpCode_c,                               /*! Gap_Pair command operation code */
    gBleGapCmdSendSlaveSecurityRequestOpCode_c,           /*! Gap_SendSlaveSecurityRequest command operation code */
    gBleGapCmdEncryptLinkOpCode_c,                        /*! Gap_EncryptLink command operation code */
    gBleGapCmdAcceptPairingRequestOpCode_c,               /*! Gap_AcceptPairingRequest command operation code */
    gBleGapCmdRejectPairingOpCode_c,                      /*! Gap_RejectAuthentication command operation code */
    gBleGapCmdEnterPasskeyOpCode_c,                       /*! Gap_EnterPasskey command operation code */
    gBleGapCmdProvideOobOpCode_c,                         /*! Gap_ProvideOob command operation code */
    gBleGapCmdRejectPasskeyRequestOpCode_c,               /*! Gap_RejectPasskeyRequest command operation code */
    gBleGapCmdSendSmpKeysOpCode_c,                        /*! Gap_SendSmpKeys command operation code */
    gBleGapCmdRejectKeyExchangeRequestOpCode_c,           /*! Gap_RejectKeyExchangeRequest command operation code */
    gBleGapCmdProvideLongTermKeyOpCode_c,                 /*! Gap_ProvideLongTermKey command operation code */
    gBleGapCmdDenyLongTermKeyOpCode_c,                    /*! Gap_DenyLongTermKey command operation code */
    gBleGapCmdLoadEncryptionInformationOpCode_c,          /*! Gap_LoadEncryptionInformation command operation code */
    gBleGapCmdSetLocalPasskeyOpCode_c,                    /*! Gap_SetLocalPasskey command operation code */
    gBleGapCmdStartScanningOpCode_c,                      /*! Gap_StartScanning command operation code */
    gBleGapCmdStopScanningOpCode_c,                       /*! Gap_StopScanning command operation code */
    gBleGapCmdConnectOpCode_c,                            /*! Gap_Connect command operation code */
    gBleGapCmdDisconnectOpCode_c,                         /*! Gap_Disconnect command operation code */
    gBleGapCmdSaveCustomPeerInformationOpCode_c,          /*! Gap_SaveCustomPeerInformation command operation code */
    gBleGapCmdLoadCustomPeerInformationOpCode_c,          /*! Gap_LoadCustomPeerInformation command operation code */
    gBleGapCmdCheckIfBondedOpCode_c,                      /*! Gap_CheckIfBonded command operation code */
    gBleGapCmdReadWhiteListSizeOpCode_c,                  /*! Gap_ReadWhiteListSize command operation code */
    gBleGapCmdClearWhiteListOpCode_c,                     /*! Gap_ClearWhiteList command operation code */
    gBleGapCmdAddDeviceToWhiteListOpCode_c,               /*! Gap_AddDeviceToWhiteList command operation code */
    gBleGapCmdRemoveDeviceFromWhiteListOpCode_c,          /*! Gap_RemoveDeviceFromWhiteList command operation code */
    gBleGapCmdReadPublicDeviceAddressOpCode_c,            /*! Gap_ReadPublicDeviceAddress command operation code */
    gBleGapCmdCreateRandomDeviceAddressOpCode_c,          /*! Gap_CreateRandomDeviceAddress command operation code */
    gBleGapCmdSaveDeviceNameOpCode_c,                     /*! Gap_SaveDeviceName command operation code */
    gBleGapCmdGetBondedDevicesCountOpCode_c,              /*! Gap_GetBondedDevicesCount command operation code */
    gBleGapCmdGetBondedDeviceNameOpCode_c,                /*! Gap_GetBondedDeviceName command operation code */
    gBleGapCmdRemoveBondOpCode_c,                         /*! Gap_RemoveBond command operation code */
    gBleGapCmdRemoveAllBondsOpCode_c,                     /*! Gap_RemoveAllBonds command operation code */
    gBleGapCmdReadRadioPowerLevelOpCode_c,                /*! Gap_ReadRadioPowerLevel command operation code */
    gBleGapCmdVerifyPrivateResolvableAddressOpCode_c,   /*! Gap_VerifyPrivateResolvableAddress command operation code */
    gBleGapCmdSetRandomAddressOpCode_c,                 /*! Gap_SetRandomAddress command operation code */
    gBleGapCmdSetScanModeOpCode_c,                      /*! Gap_SetScanMode command operation code */
    gBleGapCmdSetDefaultPairingParameters_c,            /*! Gap_SetDefaultPairingParameters command operation code */
    gBleGapCmdUpdateConnectionParametersOpCode_c,       /*! Gap_UpdateConnectionParameters command operation code */
    gBleGapCmdEnableUpdateConnectionParametersOpCode_c, /*! Gap_EnableUpdateConnectionParameters command operation code
                                                         */
    gBleGapCmdUpdateLeDataLengthOpCode_c,               /*! Gap_UpdateLeDataLength command operation code */
    gBleGapCmdControllerResetOpCode_c,                  /*! Gap_ControllerReset command operation code */
    gBleGapCmdEnableHostPrivacyOpCode_c,                /*! Gap_EnableHostPrivacy command operation code */
    gBleGapCmdEnableControllerPrivacyOpCode_c,          /*! Gap_EnableControllerPrivacy command operation code */
    gBleGapCmdLeScRegeneratePublicKeyOpCode_c,          /*! Gap_LeScRegeneratePublicKey command operation code */
    gBleGapCmdLeScValidateNumericValueOpCode_c,         /*! Gap_LeScValidateNumericValue command operation code */
    gBleGapCmdLeScGetLocalOobDataOpCode_c,              /*! Gap_LeScGetLocalOobData command operation code */
    gBleGapCmdLeScSetPeerOobDataOpCode_c,               /*! Gap_LeScSetPeerOobData command operation code */
    gBleGapCmdLeScSendKeypressNotificationPrivacyOpCode_c, /*! Gap_LeScSendKeypressNotification command operation code
                                                            */
    gBleGapCmdGetBondedDevicesIdentityInformationOpCode_c, /*! Gap_GetBondedDevicesIdentityInformation command operation
                                                              code */

    gBleGapStatusOpCode_c = 0x80,                          /*! GAP status operation code */

    gBleGapEvtFirstOpCode_c = 0x81,
    gBleGapEvtCheckNotificationStatusOpCode_c =
        gBleGapEvtFirstOpCode_c, /*! Gap_CheckNotificationStatus command out parameters event operation code */
    gBleGapEvtCheckIndicationStatusOpCode_c, /*! Gap_CheckIndicationStatus command out parameters event operation code
                                              */
    gBleGapEvtGetBondedStaticAddressesOpCode_c,  /*! Gap_GetBondedStaticAddresses command out parameters event operation
                                                    code */
    gBleGapEvtLoadEncryptionInformationOpCode_c, /*! Gap_LoadEncryptionInformation command out parameters event
                                                    operation code */
    gBleGapEvtLoadCustomPeerInformationOpCode_c, /*! Gap_LoadCustomPeerInformation command out parameters event
                                                    operation code */
    gBleGapEvtCheckIfBondedOpCode_c,             /*! Gap_CheckIfBonded command out parameters event operation code */
    gBleGapEvtGetBondedDevicesCountOpCode_c, /*! Gap_GetBondedDevicesCount command out parameters event operation code
                                              */
    gBleGapEvtGetBondedDeviceNameOpCode_c,   /*! Gap_GetBondedDeviceName command out parameters event operation code */

    gBleGapEvtGenericEventInitializationCompleteOpCode_c, /*! gapGenericCallback (type = gInitializationComplete_c)
                                                             event operation code */
    gBleGapEvtGenericEventInternalErrorOpCode_c, /*! gapGenericCallback (type = gInternalError_c) event operation code
                                                  */
    gBleGapEvtGenericEventAdvertisingSetupFailedOpCode_c, /*! gapGenericCallback (type = gAdvertisingSetupFailed_c)
                                                             event operation code */
    gBleGapEvtGenericEventAdvertisingParametersSetupCompleteOpCode_c, /*! gapGenericCallback (type =
                                                                         gAdvertisingParametersSetupComplete_c) event
                                                                         operation code */
    gBleGapEvtGenericEventAdvertisingDataSetupCompleteOpCode_c,       /*! gapGenericCallback (type =
                                                                         gAdvertisingDataSetupComplete_c) event operation code
                                                                       */
    gBleGapEvtGenericEventWhiteListSizeReadOpCode_c,          /*! gapGenericCallback (type = gWhiteListSizeRead_c) event
                                                                 operation code */
    gBleGapEvtGenericEventDeviceAddedToWhiteListOpCode_c,     /*! gapGenericCallback (type = gDeviceAddedToWhiteList_c)
                                                                 event operation code */
    gBleGapEvtGenericEventDeviceRemovedFromWhiteListOpCode_c, /*! gapGenericCallback (type =
                                                                 gDeviceRemovedFromWhiteList_c) event operation code */
    gBleGapEvtGenericEventWhiteListClearedOpCode_c, /*! gapGenericCallback (type = gWhiteListCleared_c) event operation
                                                       code */
    gBleGapEvtGenericEventRandomAddressReadyOpCode_c,       /*! gapGenericCallback (type = gRandomAddressReady_c) event
                                                               operation code */
    gBleGapEvtGenericEventCreateConnectionCanceledOpCode_c, /*! gapGenericCallback (type = gCreateConnectionCanceled_c)
                                                               event operation code */
    gBleGapEvtGenericEventPublicAddressReadOpCode_c,        /*! gapGenericCallback (type = gPublicAddressRead_c) event
                                                               operation code */
    gBleGapEvtGenericEventAdvTxPowerLevelReadOpCode_c,      /*! gapGenericCallback (type = gAdvTxPowerLevelRead_c) event
                                                               operation code */
    gBleGapEvtGenericEventPrivateResolvableAddressVerifiedOpCode_c, /*! gapGenericCallback (type =
                                                                       gPrivateResolvableAddressVerified_c) event
                                                                       operation code */
    gBleGapEvtGenericEventRandomAddressSetOpCode_c, /*! gapGenericCallback (type = gRandomAddressSet_c) event operation
                                                       code */

    gBleGapEvtAdvertisingEventAdvertisingStateChangedOpCode_c,  /*! gapAdvertisingCallback (type =
                                                                   gAdvertisingStateChanged_c) event operation code */
    gBleGapEvtAdvertisingEventAdvertisingCommandFailedOpCode_c, /*! gapAdvertisingCallback (type =
                                                                   gAdvertisingCommandFailed_c) event operation code */

    gBleGapEvtScanningEventScanStateChangedOpCode_c,  /*! gapScanningCallback (type = gScanStateChanged_c) event
                                                         operation code */
    gBleGapEvtScanningEventScanCommandFailedOpCode_c, /*! gapScanningCallback (type = gScanCommandFailed_c) event
                                                         operation code */
    gBleGapEvtScanningEventDeviceScannedOpCode_c, /*! gapScanningCallback (type = gDeviceScanned_c) event operation code
                                                   */

    gBleGapEvtConnectionEventConnectedOpCode_c, /*! gapConnectionCallback (type = gConnected_c) event operation code */
    gBleGapEvtConnectionEventPairingRequestOpCode_c,         /*! gapConnectionCallback (type = gPairingRequest_c) event
                                                                operation code */
    gBleGapEvtConnectionEventSlaveSecurityRequestOpCode_c,   /*! gapConnectionCallback (type = gSlaveSecurityRequest_c)
                                                                event operation code */
    gBleGapEvtConnectionEventPairingResponseOpCode_c,        /*! gapConnectionCallback (type = gPairingResponse_c) event
                                                                operation code */
    gBleGapEvtConnectionEventAuthenticationRejectedOpCode_c, /*! gapConnectionCallback (type =
                                                                gAuthenticationRejected_c) event operation code */
    gBleGapEvtConnectionEventPasskeyRequestOpCode_c,         /*! gapConnectionCallback (type = gPasskeyRequest_c) event
                                                                operation code */
    gBleGapEvtConnectionEventOobRequestOpCode_c, /*! gapConnectionCallback (type = gOobRequest_c) event operation code
                                                  */
    gBleGapEvtConnectionEventPasskeyDisplayOpCode_c,     /*! gapConnectionCallback (type = gPasskeyDisplay_c) event
                                                            operation code */
    gBleGapEvtConnectionEventKeyExchangeRequestOpCode_c, /*! gapConnectionCallback (type = gKeyExchangeRequest_c) event
                                                            operation code */
    gBleGapEvtConnectionEventKeysReceivedOpCode_c, /*! gapConnectionCallback (type = gKeysReceived_c) event operation
                                                      code */
    gBleGapEvtConnectionEventLongTermKeyRequestOpCode_c, /*! gapConnectionCallback (type = gLongTermKeyRequest_c) event
                                                            operation code */
    gBleGapEvtConnectionEventEncryptionChangedOpCode_c,  /*! gapConnectionCallback (type = gEncryptionChanged_c) event
                                                            operation code */
    gBleGapEvtConnectionEventPairingCompleteOpCode_c,    /*! gapConnectionCallback (type = gPairingComplete_c) event
                                                            operation code */
    gBleGapEvtConnectionEventDisconnectedOpCode_c, /*! gapConnectionCallback (type = gDisconnected_c) event operation
                                                      code */
    gBleGapEvtConnectionEventRssiReadOpCode_c, /*! gapConnectionCallback (type = gRssiRead_c) event operation code */
    gBleGapEvtConnectionEventTxPowerLevelReadOpCode_c, /*! gapConnectionCallback (type = gTxPowerLevelRead_c) event
                                                          operation code */
    gBleGapEvtConnectionEventPowerReadFailureOpCode_c, /*! gapConnectionCallback (type = gPowerReadFailureOpCode_c)
                                                          event operation code */
    gBleGapEvtConnectionEventParameterUpdateRequestOpCode_c,  /*! gapConnectionCallback (type =
                                                                 gConnEvtParameterUpdateRequest_c) event operation code
                                                               */
    gBleGapEvtConnectionEventParameterUpdateCompleteOpCode_c, /*! gapConnectionCallback (type =
                                                                 gConnEvtParameterUpdateComplete_c) event operation code
                                                               */
    gBleGapEvtConnectionEventLeDataLengthChangedOpCode_c,     /*! gapConnectionCallback (type =
                                                                 gConnEvtLeDataLengthChanged_c) event operation code */
    gBleGapEvtConnectionEventLeScOobDataRequestOpCode_c, /*! gapConnectionCallback (type = gConnEvtLeScOobDataRequest_c)
                                                            event operation code */
    gBleGapEvtConnectionEventLeScDisplayNumericValueOpCode_c,  /*! gapConnectionCallback (type =
                                                                  gConnEvtLeScDisplayNumericValue_c) event operation code
                                                                */
    gBleGapEvtConnectionEventLeScKeypressNotificationOpCode_c, /*! gapConnectionCallback (type =
                                                                  gConnEvtLeScKeypressNotification_c) event operation
                                                                  code */

    gBleGapEvtGenericEventControllerResetCompleteOpCode_c,  /*! gapGenericCallback (type = gControllerResetComplete_c)
                                                               event operation code */
    gBleGapEvtGenericEventLeScPublicKeyRegeneratedOpCode_c, /*! gapGenericCallback (type = gLeScPublicKeyRegenerated_c)
                                                               event operation code */
    gBleGapEvtGenericEventLeScLocalOobDataOpCode_c, /*! gapGenericCallback (type = gLeScLocalOobData_c) event operation
                                                       code */
    gBleGapEvtGenericEventControllerPrivacyStateChangedOpCode_c, /*! gapGenericCallback (type =
                                                                    gControllerPrivacyStateChanged_c) event operation
                                                                    code */

    gBleGapEvtGetBondedDevicesIdentityInformationOpCode_c,       /*! Gap_GetBondedDevicesIdentityInformation command out
                                                                    parameters event operation code */

} fsciBleGapOpCode_t;

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
 * \brief  Calls the GAP function associated with the operation code received over UART.
 *         The GAP function parameters are extracted from the received FSCI payload.
 *
 * \param[in]    pData               Packet (containing FSCI header and FSCI
 *                                   payload) received over UART
 * \param[in]    param               Pointer given when this function is registered in
 *                                   FSCI
 * \param[in]    fsciInterfaceId     FSCI interface on which the packet was received
 *
 ********************************************************************************** */
void fsciBleGapHandler(void *pData, void *param, uint32_t fsciInterfaceId);

/*! *********************************************************************************
 * \brief  Creates a GAP FSCI packet without payload and sends it over UART.
 *
 * \param[in]    opCode      GAP command operation code.
 *
 ********************************************************************************** */
void fsciBleGapNoParamCmdMonitor(fsciBleGapOpCode_t opCode);

/*! *********************************************************************************
 * \brief  Creates a GAP FSCI packet with one byte of payload and sends it over UART.
 *
 * \param[in]    opCode      GAP command operation code.
 * \param[in]    param       One byte parameter.
 *
 ********************************************************************************** */
void fsciBleGapUint8ParamCmdMonitor(fsciBleGapOpCode_t opCode, uint8_t param);

/*! *********************************************************************************
 * \brief  Creates a GAP FSCI packet with deviceId as payload and sends it over UART.
 *
 * \param[in]    opCode      GAP command operation code.
 * \param[in]    deviceId    Device identifier of the peer.
 *
 ********************************************************************************** */
void fsciBleGapDeviceIdParamCmdMonitor(fsciBleGapOpCode_t opCode, deviceId_t deviceId);

/*! *********************************************************************************
 * \brief  Gap_CheckNotificationStatus and Gap_CheckIndicationsStatus commands
 *         monitoring function.
 *
 * \param[in]    opCode          GAP command operation code.
 * \param[in]    deviceId        The peer GATT Client.
 * \param[in]    handle          The handle of the CCCD.
 *
 ********************************************************************************** */
void fsciBleGapCheckNotificationsAndIndicationsCmdMonitor(fsciBleGapOpCode_t opCode,
                                                          deviceId_t deviceId,
                                                          uint16_t handle,
                                                          bool_t *pOutIsActive);

/*! *********************************************************************************
 * \brief  Gap_AddDeviceToWhiteList and Gap_RemoveDeviceFromWhiteList commands
 *         monitoring function.
 *
 * \param[in]    opCode          GAP command operation code.
 * \param[in]    address         The address of the white-listed device.
 * \param[in]    addressType     The device address type (public or random).
 *
 ********************************************************************************** */
void fsciBleGapAddressParamsCmdMonitor(fsciBleGapOpCode_t opCode,
                                       bleAddressType_t addressType,
                                       bleDeviceAddress_t address);

/*! *********************************************************************************
 * \brief  Gap_SaveCustomPeerInformation and Gap_LoadCustomPeerInformation commands
 *         monitoring function.
 *
 * \param[in]    opCode      GAP command operation code.
 * \param[in]    deviceId    Device ID of the GAP peer.
 * \param[in]    aInfo       Pointer to the beginning of the data.
 * \param[in]    offset      Offset from the beginning of the reserved memory area.
 * \param[in]    infoSize    Data size (maximum equal to gcReservedFlashSizeForCustomInformation_d).
 *
 ********************************************************************************** */
void fsciBleGapSaveOrLoadCustomPeerInformationCmdMonitor(
    fsciBleGapOpCode_t opCode, deviceId_t deviceId, void *aInfo, uint16_t offset, uint16_t infoSize);

/*! *********************************************************************************
* \brief  Ble_HostInitialize command monitoring function.
*
* \param[in]    genericCallback             Callback used to propagate GAP controller
                                            events to the application.
* \param[in]    hostToControllerInterface   LE Controller uplink interface function pointer
*
********************************************************************************** */
void fsciBleGapHostInitializeCmdMonitor(gapGenericCallback_t genericCallback,
                                        hciHostToControllerInterface_t hostToControllerInterface);

/*! *********************************************************************************
 * \brief  Gap_RegisterDeviceSecurityRequirements command monitoring function.
 *
 * \param[in]    pSecurity       Pointer to the application-allocated
 *                               gapDeviceSecurityRequirements_t structure.
 *
 ********************************************************************************** */
void fsciBleGapRegisterDeviceSecurityRequirementsCmdMonitor(gapDeviceSecurityRequirements_t *pSecurity);

/*! *********************************************************************************
 * \brief  Gap_SetAdvertisingParameters command monitoring function.
 *
 * \param[in]    pAdvertisingParameters      Pointer to gapAdvertisingParameters_t
 *                                           structure.
 *
 ********************************************************************************** */
void fsciBleGapSetAdvertisingParametersCmdMonitor(gapAdvertisingParameters_t *pAdvertisingParameters);

/*! *********************************************************************************
 * \brief  Gap_SetAdvertisingData command monitoring function.
 *
 * \param[in]    pStaticAdvertisingData      Pointer to gapAdvertisingData_t structure
 *                                           or NULL.
 * \param[in]    pStaticScanResponseData     Pointer to gapScanResponseData_t structure
 *                                           or NULL.
 *
 ********************************************************************************** */
void fsciBleGapSetAdvertisingDataCmdMonitor(gapAdvertisingData_t *pAdvertisingData,
                                            gapScanResponseData_t *pScanResponseData);

/*! *********************************************************************************
* \brief  Gap_StartAdvertising command monitoring function.
*
* \param[in]    advertisingCallback     Callback used by the application to receive
                                        advertising events.
* \param[in]    connectionCallback      Callback used by the application to receive
                                        connection events.
*
********************************************************************************** */
void fsciBleGapStartAdvertisingCmdMonitor(gapAdvertisingCallback_t advertisingCallback,
                                          gapConnectionCallback_t connectionCallback);

/*! *********************************************************************************
 * \brief  Gap_Authorize command monitoring function.
 *
 * \param[in]    deviceId    The peer being authorized.
 * \param[in]    handle      The attribute handle.
 * \param[in]    access      The type of access granted (gAccessRead_c or gAccessWrite_c).
 *
 ********************************************************************************** */
void fsciBleGapAuthorizeCmdMonitor(deviceId_t deviceId, uint16_t handle, gattDbAccessType_t access);

/*! *********************************************************************************
 * \brief  Gap_SaveCccd command monitoring function.
 *
 * \param[in]    deviceId    The peer GATT Client.
 * \param[in]    handle      The handle of the CCCD as defined in the GATT Database.
 * \param[in]    cccd        The bit mask representing the CCCD value to be saved.
 *
 ********************************************************************************** */
void fsciBleGapSaveCccdCmdMonitor(deviceId_t deviceId, uint16_t handle, gattCccdFlags_t cccd);

/*! *********************************************************************************
 * \brief  Gap_GetBondedStaticAddresses command monitoring function.
 *
 * \param[in]    aOutDeviceAddresses     Array of addresses to be filled.
 * \param[in]    maxDevices              Maximum number of addresses to be obtained.
 * \param[in]    pOutActualCount         The actual number of addresses written.
 *
 ********************************************************************************** */
void fsciBleGapGetBondedStaticAddressesCmdMonitor(bleDeviceAddress_t *aOutDeviceAddresses,
                                                  uint8_t maxDevices,
                                                  uint8_t *pOutActualCount);

/*! *********************************************************************************
 * \brief  Gap_GetBondedDevicesIdentityInformation command monitoring function.
 *
 * \param[in]    aOutIdentityAddresses     Array of addresses to be filled.
 * \param[in]    maxDevices                Maximum number of addresses to be obtained.
 * \param[in]    pOutActualCount           The actual number of addresses written.
 *
 ********************************************************************************** */
void fsciBleGapGetBondedDevicesIdentityInformationCmdMonitor(gapIdentityInformation_t *aOutIdentityAddresses,
                                                             uint8_t maxDevices,
                                                             uint8_t *pOutActualCount);

/*! *********************************************************************************
 * \brief  Gap_Pair command monitoring function.
 *
 * \param[in]    deviceId            The peer to pair with.
 * \param[in]    pPairingParameters  Pairing parameters as required by the SMP.
 *
 ********************************************************************************** */
void fsciBleGapPairCmdMonitor(deviceId_t deviceId, gapPairingParameters_t *pPairingParameters);

/*! *********************************************************************************
 * \brief  Gap_SendSlaveSecurityRequest command monitoring function.
 *
 * \param[in]    deviceId            The GAP peer to pair with.
 * \param[in]    bondAfterPairing    Specifies if bonding is supported.
 * \param[in]    securityModeLevel   The level of security requested.
 *
 ********************************************************************************** */
void fsciBleGapSendSlaveSecurityRequestCmdMonitor(deviceId_t deviceId,
                                                  bool_t bondAfterPairing,
                                                  gapSecurityModeAndLevel_t securityModeLevel);

/*! *********************************************************************************
 * \brief  Gap_AcceptPairingRequest command monitoring function.
 *
 * \param[in]    deviceId            The peer requesting authentication.
 * \param[in]    pPairingParameters  Pairing parameters as required by the SMP.
 *
 ********************************************************************************** */
void fsciBleGapAcceptPairingRequestCmdMonitor(deviceId_t deviceId, gapPairingParameters_t *pPairingParameters);

/*! *********************************************************************************
 * \brief  Gap_RejectAuthentication command monitoring function.
 *
 * \param[in]    deviceId    The GAP peer who requested authentication.
 * \param[in]    reason      The reason of rejection.
 *
 ********************************************************************************** */
void fsciBleGapRejectPairingCmdMonitor(deviceId_t deviceId, gapAuthenticationRejectReason_t reason);

/*! *********************************************************************************
 * \brief  Gap_EnterPasskey command monitoring function.
 *
 * \param[in]    deviceId    The GAP peer that requested a passkey entry.
 * \param[in]    passkey     The peer's secret passkey.
 *
 ********************************************************************************** */
void fsciBleGapEnterPasskeyCmdMonitor(deviceId_t deviceId, uint32_t passkey);

/*! *********************************************************************************
 * \brief  Gap_ProvideOob command monitoring function.
 *
 * \param[in]    deviceId    The pairing device.
 * \param[in]    aOob        Pointer to OOB data (array of gcSmpOobSize_d size).
 *
 ********************************************************************************** */
void fsciBleGapProvideOobCmdMonitor(deviceId_t deviceId, uint8_t *aOob);

/*! *********************************************************************************
 * \brief  Gap_SendSmpKeys command monitoring function.
 *
 * \param[in]    deviceId    The GAP peer who initiated the procedure.
 * \param[in]    pKeys       The SMP keys of the local device.
 *
 ********************************************************************************** */
void fsciBleGapSendSmpKeysCmdMonitor(deviceId_t deviceId, gapSmpKeys_t *pKeys);

/*! *********************************************************************************
 * \brief  Gap_ProvideLongTermKey command monitoring function.
 *
 * \param[in]    deviceId    The GAP peer who requested encryption.
 * \param[in]    aLtk        The Long Term Key.
 * \param[in]    ltkSize     The Long Term Key size.
 *
 ********************************************************************************** */
void fsciBleGapProvideLongTermKeyCmdMonitor(deviceId_t deviceId, uint8_t *aLtk, uint8_t ltkSize);

/*! *********************************************************************************
 * \brief  Gap_LoadEncryptionInformation command monitoring function.
 *
 * \param[in]    deviceId        Device ID of the peer.
 * \param[in]    aOutLtk         Array of size gcMaxLtkSize_d to be filled with the LTK.
 * \param[in]    pOutLtkSize     The LTK size.
 *
 ********************************************************************************** */
void fsciBleGapLoadEncryptionInformationCmdMonitor(deviceId_t deviceId, uint8_t *aOutLtk, uint8_t *pOutLtkSize);

/*! *********************************************************************************
 * \brief  Gap_SetLocalPasskey command monitoring function.
 *
 * \param[in]    passkey     The SMP passkey.
 *
 ********************************************************************************** */
void fsciBleGapSetLocalPasskeyCmdMonitor(uint32_t passkey);

/*! *********************************************************************************
 * \brief  Gap_StartScanning command monitoring function.
 *
 * \param[in]    pScanningParameters     The scanning parameters, may be NULL.
 * \param[in]    scanningCallback        The scanning callback.
 *
 ********************************************************************************** */
void fsciBleGapStartScanningCmdMonitor(gapScanningParameters_t *pScanningParameters,
                                       gapScanningCallback_t scanningCallback);

/*! *********************************************************************************
 * \brief  Gap_Connect command monitoring function.
 *
 * \param[in]    pParameters     Create Connection command parameters.
 * \param[in]    connCallback    Callback used to receive connection events.
 *
 ********************************************************************************** */
void fsciBleGapConnectCmdMonitor(gapConnectionRequestParameters_t *pParameters, gapConnectionCallback_t connCallback);

/*! *********************************************************************************
 * \brief  Gap_SaveCustomPeerInformation command monitoring function.
 *
 * \param[in]    deviceId    Device ID of the GAP peer.
 * \param[in]    aInfo       Pointer to the beginning of the data.
 * \param[in]    offset      Offset from the beginning of the reserved memory area.
 * \param[in]    infoSize    Data size (maximum equal to gcReservedFlashSizeForCustomInformation_d).
 *
 ********************************************************************************** */
void fsciBleGapSaveCustomPeerInformationCmdMonitor(deviceId_t deviceId,
                                                   void *aInfo,
                                                   uint16_t offset,
                                                   uint16_t infoSize);

/*! *********************************************************************************
 * \brief  Gap_LoadCustomPeerInformation command monitoring function.
 *
 * \param[in]    deviceId    Device ID of the GAP peer.
 * \param[in]    aOutInfo    Pointer to the beginning of the allocated memory.
 * \param[in]    offset      Offset from the beginning of the reserved memory area.
 * \param[in]    infoSize    Data size (maximum equal to gcReservedFlashSizeForCustomInformation_d).
 *
 ********************************************************************************** */
void fsciBleGapLoadCustomPeerInformationCmdMonitor(deviceId_t deviceId,
                                                   void *aOutInfo,
                                                   uint16_t offset,
                                                   uint16_t infoSize);

/*! *********************************************************************************
 * \brief  Gap_CheckIfBonded command monitoring function.
 *
 * \param[in]    deviceId        Device ID of the GAP peer.
 * \param[in]    pOutIsBonded    Boolean to be filled with the bonded flag.
 *
 ********************************************************************************** */
void fsciBleGapCheckIfBondedCmdMonitor(deviceId_t deviceId, bool_t *pOutIsBonded);

/*! *********************************************************************************
 * \brief  Gap_CreateRandomDeviceAddress command monitoring function.
 *
 * \param[in]    aIrk            The Identity Resolving Key to be used for a private
 *                               resolvable address or NULL for a private non-resolvable
 *                               address (fully random).
 * \param[in]    aRandomPart     If aIrk is not NULL, this is a 3-byte array containing
 *                               the Random Part of a Private Resolvable Address, in LSB
 *                               to MSB order; the most significant two bits of the most
 *                               significant byte (aRandomPart[3] & 0xC0) are ignored.
 *                               This may be NULL, in which case the Random Part is
 *                               randomly generated internally.
 *
 ********************************************************************************** */
void fsciBleGapCreateRandomDeviceAddressCmdMonitor(uint8_t *aIrk, uint8_t *aRandomPart);

/*! *********************************************************************************
 * \brief  Gap_SaveDeviceName command monitoring function.
 *
 * \param[in]    deviceId    Device ID for the active peer which name is saved.
 * \param[in]    aName       Array of characters holding the name.
 * \param[in]    cNameSize   Number of characters to be saved.
 *
 ********************************************************************************** */
void fsciBleGapSaveDeviceNameCmdMonitor(deviceId_t deviceId, uchar_t *aName, uint8_t cNameSize);

/*! *********************************************************************************
 * \brief  Gap_GetBondedDevicesCount command monitoring function.
 *
 * \param[in]    pOutBondedDevicesCount  Pointer to integer to be written.
 *
 ********************************************************************************** */
void fsciBleGapGetBondedDevicesCountCmdMonitor(uint8_t *pOutBondedDevicesCount);

/*! *********************************************************************************
 * \brief  Gap_GetBondedDeviceName command monitoring function.
 *
 * \param[in]    nvmIndex        Index of the device in NVM bonding area.
 * \param[in]    aOutName        Destination array to copy the name into.
 * \param[in]    maxNameSize     Maximum number of characters to be copied,
 *
 ********************************************************************************** */
void fsciBleGapGetBondedDeviceNameCmdMonitor(uint8_t nvmIndex, uchar_t *aOutName, uint8_t maxNameSize);

/*! *********************************************************************************
 * \brief  Gap_ReadRadioPowerLevel command monitoring function.
 *
 * \param[in]    txReadType      Radio power level read type.
 * \param[in]    deviceId        Device identifier.
 *
 ********************************************************************************** */
void fsciBleGapReadRadioPowerLevelCmdMonitor(gapRadioPowerLevelReadType_t txReadType, deviceId_t deviceId);

/*! *********************************************************************************
 * \brief  Gap_UpdateLeDataLength command monitoring function.
 *
 * \param[in]    txReadType      Radio power level read type.
 * \param[in]    deviceId        Device identifier.
 *
 ********************************************************************************** */
void fsciBleGapUpdateLeDataLengthCmdMonitor(deviceId_t deviceId, uint16_t txOctets, uint16_t txTime);

/*! *********************************************************************************
 * \brief  Gap_EnableHostPrivacy command monitoring function.
 *
 * \param[in]    enable      TRUE to enable, FALSE to disable.
 * \param[in]    aIrk        Local IRK to be used for Resolvable Private Address generation
 *                           or NULL for Non-Resolvable Private Address generation. Ignored if enable is FALSE.
 *
 ********************************************************************************** */
void fsciBleGapEnableHostPrivacyCmdMonitor(bool_t enable, uint8_t *aIrk);

/*! *********************************************************************************
 * \brief  Gap_EnableHostPrivacy command monitoring function.
 *
 * \param[in] enable             TRUE to enable, FALSE to disable.
 * \param[in] aOwnIrk            Local IRK. Ignored if enable is FALSE, otherwise shall not be NULL.
 * \param[in] peerIdCount        Size of aPeerIdentities array. Ignored if enable is FALSE.
 * \param[in] aPeerIdentities    Array of peer identity addresses and IRKs. Ignored if enable is FALSE,
 *                               otherwise shall not be NULL.
 *
 ********************************************************************************** */
void fsciBleGapEnableControllerPrivacyCmdMonitor(bool_t enable,
                                                 uint8_t *aOwnIrk,
                                                 uint8_t peerIdCount,
                                                 gapIdentityInformation_t *aPeerIdentities);

/*! *********************************************************************************
 * \brief  Gap_LeScValidateNumericValue command monitoring function.
 *
 * \param[in] deviceId           The DeviceID for which the command is intended.
 * \param[in] valid              TRUE if user has indicated that numeric values are matched, FALSE otherwise.
 *
 ********************************************************************************** */
void fsciBleGapLeScValidateNumericValueCmdMonitor(deviceId_t deviceId, bool_t valid);

/*! *********************************************************************************
 * \brief  Gap_LeScSetPeerOobData command monitoring function.
 *
 * \param[in] deviceId           Device ID of the peer.
 * \param[in] pPeerOobData       OOB data received from the peer.
 *
 ********************************************************************************** */
void fsciBleGapLeScSetPeerOobDataCmdMonitor(deviceId_t deviceId, gapLeScOobData_t *pPeerOobData);

/*! *********************************************************************************
 * \brief  Gap_LeScSendKeypressNotification command monitoring function.
 *
 * \param[in] deviceId               Device ID of the peer.
 * \param[in] keypressNotification   Value of the Keypress Notification.
 *
 ********************************************************************************** */
void fsciBleGapLeScSendKeypressNotificationCmdMonitor(deviceId_t deviceId,
                                                      gapKeypressNotification_t keypressNotification);

/*! *********************************************************************************
 * \brief  Gap_VerifyPrivateResolvableAddress command monitoring function.
 *
 * \param[in]    nvmIndex    Index of the device in NVM bonding area whose IRK must
 *                           be checked.
 * \param[in]    aAddress    The Private Resolvable Address to be verified.
 *
 ********************************************************************************** */
void fsciBleGapVerifyPrivateResolvableAddressCmdMonitor(uint8_t nvmIndex, bleDeviceAddress_t aAddress);

/*! *********************************************************************************
 * \brief  Gap_SetRandomAddress command monitoring function.
 *
 * \param[in]    aAddress    The Private Resolvable, Private Non-Resolvable or Static
 *                           Random Address.
 *
 ********************************************************************************** */
void fsciBleGapSetRandomAddressCmdMonitor(bleDeviceAddress_t aAddress);

/*! *********************************************************************************
 * \brief  Gap_SetScanMode command monitoring function.
 *
 * \param[in]    discoveryMode       The scan mode to be activated.
 * \param[in]    pAutoConnectParams  Parameters for the Auto Connect Scan Mode.
 *
 ********************************************************************************** */
void fsciBleGapSetScanModeCmdMonitor(gapScanMode_t scanMode, gapAutoConnectParams_t *pAutoConnectParams);

/*! *********************************************************************************
 * \brief  Gap_SetDefaultPairingParameters command monitoring function.
 *
 * \param[in]    pPairingParameters  Pairing parameters as required by the SMP or NULL.
 *
 ********************************************************************************** */
void fsciBleGapSetDefaultPairingParametersCmdMonitor(gapPairingParameters_t *pPairingParameters);

/*! *********************************************************************************
 * \brief  Gap_UpdateConnectionParameters command monitoring function.
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    intervalMin         The minimum value for the connection event interval.
 * \param[in]    intervalMax         The maximum value for the connection event interval.
 * \param[in]    slaveLatency        The slave latency parameter.
 * \param[in]    timeoutMultiplier   The connection timeout parameter.
 * \param[in]    minCeLength         The minimum connection event length.
 * \param[in]    maxCeLength         The maximum connection event length.
 *
 ********************************************************************************** */
void fsciBleGapUpdateConnectionParametersCmdMonitor(deviceId_t deviceId,
                                                    uint16_t intervalMin,
                                                    uint16_t intervalMax,
                                                    uint16_t slaveLatency,
                                                    uint16_t timeoutMultiplier,
                                                    uint16_t minCeLength,
                                                    uint16_t maxCeLength);

/*! *********************************************************************************
 * \brief  Gap_EnableUpdateConnectionParameters command monitoring function.
 *
 * \param[in]    deviceId            The DeviceID for which the command is intended.
 * \param[in]    enable              Allow/disallow the parameters update.
 *
 ********************************************************************************** */
void fsciBleGapEnableUpdateConnectionParametersCmdMonitor(deviceId_t deviceId, bool_t enable);

#ifdef __cplusplus
}
#endif

#endif /* _FSCI_BLE_ATT_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
