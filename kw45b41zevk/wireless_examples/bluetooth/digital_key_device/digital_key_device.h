/*! *********************************************************************************
 * \defgroup Digital Key Device
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* \file digital_key_device.h
*
* Copyright 2020-2022 NXP
*
* NXP Confidential Proprietary
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from NXP.
********************************************************************************** */

#ifndef DIGITAL_KEY_DEVICE_H
#define DIGITAL_KEY_DEVICE_H

#include "fsl_format.h"
#include "fsl_shell.h"

/*************************************************************************************
**************************************************************************************
* Public macros
**************************************************************************************
*************************************************************************************/
#define gScanningTime_c                30

#define gWaitForDataTime_c             5

#ifndef gAppDeepSleepMode_c
#define gAppDeepSleepMode_c            1
#endif

/* Enable/Disable Controller Adv/Scan/Connection Notifications */
#ifndef gUseControllerNotifications_c
#define gUseControllerNotifications_c 0
#endif

/* Receive Notifications in a callback registered in the Controller
   instead of Host GAP Generic callback */
#ifndef gUseControllerNotificationsCallback_c
#define gUseControllerNotificationsCallback_c 0
#endif

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c == 1))
  /* switch press timer timeout */
  #ifndef gSwitchPressTimeout_c
  #define gSwitchPressTimeout_c    (1000UL)
  #endif
  /* switch press threshold (number of key presses to toggle the GAP role) */
  #ifndef gSwitchPressThreshold_c
  #define gSwitchPressThreshold_c    (2)
  #endif
#endif

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum appEvent_tag{
    mAppEvt_KBD_EventPressPB1_c,
    mAppEvt_KBD_EventLongPB1_c,
    mAppEvt_KBD_EventVeryLongPB1_c,
    mAppEvt_GenericCallback_PeerDisconnected_c,
    mAppEvt_GenericCallback_LePhyEvent_c,
    mAppEvt_GenericCallback_LeScLocalOobData_c,
    mAppEvt_GenericCallback_RandomAddressReady_c,
    mAppEvt_GenericCallback_CtrlNotifEvent_c,
    mAppEvt_GenericCallback_BondCreatedEvent_c,
    mAppEvt_ConnectionCallback_ConnEvtConnected_c,
    mAppEvt_ConnectionCallback_ConnEvtDisconnected_c,
    mAppEvt_ConnectionCallback_ConnEvtLeScOobDataRequest_c,
    mAppEvt_ConnectionCallback_ConnEvtPairingComplete_c,
    mAppEvt_ConnectionCallback_ConnEvtEncryptionChanged_c,
    mAppEvt_ConnectionCallback_ConnEvtAuthenticationRejected_c,
    mAppEvt_ServiceDiscoveryCallback_DiscoveryFinishedWithSuccess_c,
    mAppEvt_ServiceDiscoveryCallback_DiscoveryFinishedFailed_c,
    mAppEvt_GattClientCallback_GattProcError_c,
    mAppEvt_GattClientCallback_GattProcReadCharacteristicValue_c,
    mAppEvt_GattClientCallback_GattProcReadUsingCharacteristicUuid_c,
    mAppEvt_GattClientCallback_GattProcComplete_c,
    mAppEvt_L2capPsmDataCallback_c,
    mAppEvt_L2capPsmControlCallback_LePsmConnectionComplete_c,
    mAppEvt_L2capPsmControlCallback_LePsmDisconnectNotification_c,
    mAppEvt_L2capPsmControlCallback_NoPeerCredits_c,
    mAppEvt_Shell_Reset_Command_c,
    mAppEvt_Shell_FactoryReset_Command_c,
    mAppEvt_Shell_ShellStartDiscovery_Command_c,
    mAppEvt_Shell_StopDiscovery_Command_c,
    mAppEvt_Shell_Disconnect_Command_c,
    mAppEvt_Shell_SetBondingData_Command_c,
    mAppEvt_Shell_ListBondedDev_Command_c,
    mAppEvt_Shell_RemoveBondedDev_Command_c,
    mAppEvt_PeerConnected_c,
    mAppEvt_PeerDisconnected_c,
    mAppEvt_EncryptionChanged_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_PairingLocalOobData_c,
    mAppEvt_PairingPeerOobDataReq_c,
    mAppEvt_PairingPeerOobDataRcv_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c,
    mAppEvt_ReadCharacteristicValueComplete_c,
    mAppEvt_PsmChannelCreated_c,
    mAppEvt_SentSPAKEResponse_c,
    mAppEvt_ReceivedSPAKEVerify_c,
    mAppEvt_ReceivedPairingReady_c,
    mAppEvt_AuthenticationRejected_c
} appEvent_t;

typedef struct appEventL2capPsmData_tag
{
    deviceId_t     deviceId;
    uint16_t       lePsm;
    uint16_t       packetLength;
    uint8_t*       pPacket;
} appEventL2capPsmData_t;

typedef struct appConnectionCallbackEventData_tag
{
    deviceId_t peerDeviceId;
    gapConnectedEvent_t pConnectedEvent;
} appConnectionCallbackEventData_t;

typedef struct appEventData_tag
{
    appEvent_t   appEvent;              /*!< Event type. */
    union {
        void*      pData;
        deviceId_t peerDeviceId;
    } eventData;                        /*!< Event data, selected according to event type. */
} appEventData_t;


/* APP -  pointer to function for BLE events */
typedef void (*pfBleCallback_t)(void* pData);
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
extern gapConnectionRequestParameters_t gConnReqParams;
extern gapScanningParameters_t          gScanParams;
/* This global will be TRUE if the user adds or removes a bond */
extern bool_t                           gPrivacyStateChangedByUser;

/* Application callback */
extern pfBleCallback_t mpfBleEventHandler;
/************************************************************************************
*************************************************************************************
* Public prototypes
*************************************************************************************
************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
void BleApp_Start(void);
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent);
void BleApp_RegisterEventHandler(pfBleCallback_t pfBleEventHandler);
void BleApp_FactoryReset(void);
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
void BleApp_PrintHex(uint8_t *pHex, uint8_t len);
void BleApp_PrintHexLe(uint8_t *pHex, uint8_t len);
#endif

#ifdef __cplusplus
}
#endif


#endif /* DIGITAL_KEY_DEVICE_H */

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
