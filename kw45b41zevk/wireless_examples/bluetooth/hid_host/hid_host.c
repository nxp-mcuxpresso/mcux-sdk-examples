/*! *********************************************************************************
* \addtogroup HID Host
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the HID Host application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"

/* Framework / Drivers */
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_shell.h"
#include "fsl_format.h"
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "RNG_Interface.h"
#include "fwk_platform_ble.h"
#include "fsl_component_mem_manager.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

/* Profile / Services */
#include "hid_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "app_conn.h"
#include "app_scanner.h"
#include "hid_host.h"
#include "board.h"
#include "app.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
/*  appEvent_t  */
#define mAppEvt_PeerConnected_c             0x00U
#define mAppEvt_PairingComplete_c           0x01U
#define mAppEvt_ServiceDiscoveryComplete_c  0x02U
#define mAppEvt_ServiceDiscoveryFailed_c    0x03U
#define mAppEvt_GattProcComplete_c          0x04U
#define mAppEvt_GattProcError_c             0x05U

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef uint8_t appEvent_t;

typedef enum appState_tag{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppReadDescriptorA_c,
    mAppReadDescriptorB_c,
    mAppRunning_c,
}appState_t;

typedef struct appCustomInfo_tag
{
    hidcConfig_t     hidClientConfig;
    /* Add persistent information here */
}appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
}appPeerInfo_t;

typedef struct mouseHidReport_tag{
  uint8_t buttonStatus;
  uint8_t xAxis;
  uint8_t yAxis;
}mouseHidReport_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation[gAppMaxConnections_c];

#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;

#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U))
static bool_t mAttemptRpaResolvingAtConnect = FALSE;
#endif /* gAppUsePrivacy_d */

#endif /* gAppUseBonding_d */

static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;

/* Application scanning parameters */
static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringEnable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Host Stack callbacks */
static void BleApp_ScanningCallback
(
    gapScanningEvent_t* pScanningEvent
);

static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t* pConnectionEvent
);

static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);

static void BleApp_GattNotificationCallback
(
    deviceId_t          serverDeviceId,
    uint16_t characteristicValueHandle,
    uint8_t* aValue,
    uint16_t valueLength
);

static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t deviceId,
    servDiscEvent_t* pEvent
);

static void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData);

static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
);

static bleResult_t BleApp_ConfigureNotifications
(
    deviceId_t peerDeviceId,
    uint16_t handle
);

static void BluetoothLEHost_Initialized(void);

static void BleApp_StateMachineHandler_IdleState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ExchangeMtuState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ServiceDiscState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ReadDescriptorAState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_ReadDescriptorBState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

static void BleApp_StateMachineHandler_RunningState
(
    deviceId_t peerDeviceId,
    uint8_t event
);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /* gAppButtonCnt_c > 0 */

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
SHELL_HANDLE_DEFINE(g_shellHandle);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mScanningOn)
    {
        (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            BleApp_Start();
            break;
        }

        case kBUTTON_EventLongPress:
        {
            for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
            {
                if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                {
                    (void)Gap_Disconnect(mPeerInformation[i].deviceId);
                }
            }
            break;
        }

        default:
        {
            ; /* For MISRA Compliance */
            break;
        }
    }
    
    return kStatus_BUTTON_Success; 
}
#endif /* gAppButtonCnt_c */

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
	LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif /* gAppButtonCnt_c */
    
    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief        Prints a hex string.
*
* \param[in]    pHex    pointer to hex value.
* \param[in]    len     hex value length.
********************************************************************************** */
void BleApp_PrintHex(uint8_t *pHex, uint8_t len)
{
    for (int32_t i = (int32_t)len - 1; i>= 0; i--)
    {
        (void)SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, (char const*)FORMAT_Hex2Ascii(pHex[i]));
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    uint8_t bondedDevicesCnt = 0;
#endif /* gAppUsePrivacy_d && gAppUseBonding_d */

    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if(mFoundDeviceToConnect == FALSE)
            {
                mFoundDeviceToConnect = BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice);

                if (mFoundDeviceToConnect == TRUE)
                {
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
                    
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U))
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif /* gAppUsePrivacy_d */
                }
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;
            if (mScanningOn)
            {
                mFoundDeviceToConnect = FALSE;
                LedStopFlashingAllLeds();

#if (gAppLedCnt_c == 1)
                LedSetColor(0, kLED_Blue);
#endif /* gAppLedCnt_c */

                Led1Flashing();
                (void)shell_write("\r\nScanning...\r\n");
            }
            else /* Scanning is turned OFF */
            {
                if(mFoundDeviceToConnect == TRUE)
                {
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U))
                    if(gConnReqParams.peerAddressType == gBleAddrTypeRandom_c)
                    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
                        /* Check if there are any bonded devices */
                        Gap_GetBondedDevicesCount(&bondedDevicesCnt);

                        if(bondedDevicesCnt == 0)
                        {
                            /* display the unresolved RPA address */
                            shell_writeHex(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                        }
                        else
                        {
                            mAttemptRpaResolvingAtConnect = TRUE;
                        }
#else /* gAppUseBonding_d */
                        /* If bonding is disabled and we receive an RPA address there is nothing to do but display it */
                        shell_writeHex(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUseBonding_d */
                    }
#else /* gAppUsePrivacy_d */
                    /* Display the peer address */
                    shell_writeHex(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUsePrivacy_d */

                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
            }
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent)
{
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
            if(mAttemptRpaResolvingAtConnect == TRUE)
            {
                /* If the peer RPA was resolved, the IA is displayed, otherwise the peer RPA address is displayed */
                shell_writeHex(pConnectionEvent->eventData.connectedEvent.peerAddress, gcBleDeviceAddressSize_c);
                /* clear the flag */
                mAttemptRpaResolvingAtConnect = FALSE;
            }
#endif /* gAppUsePrivacy_d && gAppUseBonding_d */
            /* UI */
            LedStopFlashingAllLeds();

#if (gAppLedCnt_c == 1)
            LedSetColor(0, kLED_White);
#endif /* gAppLedCnt_c */

            Led1On();
            (void)shell_write("\r\nConnected to device ");
            (void)shell_writeDec(peerDeviceId);
            (void)shell_write("!\r\n");

            mPeerInformation[peerDeviceId].deviceId = peerDeviceId;
            mPeerInformation[peerDeviceId].isBonded = FALSE;

#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
            bool_t isBonded = FALSE;
            (void)Gap_CheckIfBonded(peerDeviceId, &isBonded, NULL);

            if ((isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*)&mPeerInformation[peerDeviceId].customInfo, 0, (uint16_t)(sizeof(appCustomInfo_t)))))
            {
                mRestoringBondedLink = TRUE;
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }
            else
            {
                mRestoringBondedLink = FALSE;
                mAuthRejected = FALSE;
            }
#endif /* gAppUseBonding_d */
            BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
            mPeerInformation[peerDeviceId].appState = mAppIdle_c;

            /* Reset the custom information for the current peer device */
            FLib_MemSet(&(mPeerInformation[peerDeviceId].customInfo), 0x00, sizeof(appCustomInfo_t));

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            /* UI */
            LedStopFlashingAllLeds();
            LedStartFlashingAllLeds();
            (void)shell_write("\r\nDisconnected from device ");
            (void)shell_writeDec(peerDeviceId);
            (void)shell_write("!\r\n");

            /* If peer device disconnects the link during Service Discovery, free the allocated buffer */
            if(mpCharProcBuffer != NULL)
            {
                (void)MEM_BufferFree(mpCharProcBuffer);
                mpCharProcBuffer = NULL;
            }

            /* Check if the last device was disconnected and if so re-enter GAP Limited Discovery Mode */
            uint8_t connectedDevicesNumber = 0U;
            for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
            {
                if (mPeerInformation[i].deviceId != gInvalidDeviceId_c)
                {
                    connectedDevicesNumber++;
                }
            }
            if(connectedDevicesNumber == 0U)
            {
                /* Restart scanning */
                BleApp_Start();
            }

        }
        break;

#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
                mPeerInformation[peerDeviceId].isBonded = TRUE;
#endif /* gAppUseBonding_d */
                BleApp_StateMachineHandler(mPeerInformation[peerDeviceId].deviceId, mAppEvt_PairingComplete_c);
                (void)shell_write("\r\n-->  GAP Event: Device Paired.\r\n");
            }
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
            else
            {
                mPeerInformation[peerDeviceId].isBonded = FALSE;
                (void)shell_write("\r\n-->  GAP Event: Pairing Unsuccessful.\r\n");
            }
#endif /* gAppUseBonding_d */
        }
        break;

#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                mPeerInformation[peerDeviceId].isBonded = TRUE;
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd) )
                    {
                        (void)Gap_Disconnect(peerDeviceId);
                    }
                }
                else
                {
                    mRestoringBondedLink = FALSE;
                }
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            /* Start Pairing Procedure */
            mAuthRejected = TRUE;
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

        default:
        {
          ; /* For MISRA Compliance */
        }
        break;
    }
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t deviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(deviceId, pEvent->eventData.pService);
        }
        break;

        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(deviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(deviceId, mAppEvt_ServiceDiscoveryFailed_c);
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}


/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId      GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error               Callback result.
********************************************************************************** */
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    if ((mPeerInformation[serverDeviceId].isBonded) || (mPeerInformation[serverDeviceId].appState != mAppRunning_c))
    {
#endif /* gAppUseBonding_d */
        if (procedureResult == gGattProcError_c)
        {
            attErrorCode_t attError = (attErrorCode_t)((uint8_t)error);

            if ((attError == gAttErrCodeInsufficientEncryption_c)     ||
                (attError == gAttErrCodeInsufficientAuthorization_c)  ||
                (attError == gAttErrCodeInsufficientAuthentication_c))
            {
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
                /* Start Pairing Procedure */
                (void)Gap_Pair(serverDeviceId, &gPairingParameters);
#endif /* gAppUsePairing_d */
            }

            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
        }
        else if (procedureResult == gGattProcSuccess_c)
        {
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
        }
        else
        {
            /* For MISRA Compliance */
        }

        /* Signal Service Discovery Module */
        BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    }
#endif /* gAppUseBonding_d */
}

/*! *********************************************************************************
* \brief        Handles GATT client notification callback from host stack.
*
* \param[in]    serverDeviceId              GATT Server device ID.
* \param[in]    characteristicValueHandle   Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */
static void BleApp_GattNotificationCallback
(
    deviceId_t  serverDeviceId,
    uint16_t    characteristicValueHandle,
    uint8_t*    aValue,
    uint16_t    valueLength
)
{
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    if (mPeerInformation[serverDeviceId].isBonded)
    {
#endif /* gAppUseBonding_d */

      if ((characteristicValueHandle == mPeerInformation[serverDeviceId].customInfo.hidClientConfig.hHidReport) || 
          (characteristicValueHandle == mPeerInformation[serverDeviceId].customInfo.hidClientConfig.hHidBootReport))
      {
          mouseHidReport_t *pReport = (mouseHidReport_t *)(void *)aValue;
          (void)shell_write("\r\nReceived HID Report from device ");
          (void)shell_writeDec(serverDeviceId);
          (void)shell_write(": X: ");
          shell_writeHex(&pReport->xAxis, (uint8_t)sizeof(uint8_t));
          (void)shell_write(" Y: ");
          shell_writeHex(&pReport->yAxis, (uint8_t)sizeof(uint8_t));
      }
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    }
#endif /* gAppUseBonding_d */
}

/*! *********************************************************************************
* \brief        Stores handles for the specified service.
*
* \param[in]    pService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    deviceId_t      peerDeviceId,
    gattService_t   *pService
)
{
    uint8_t i, j;

    if ((pService->uuidType == gBleUuidType16_c) &&
        (pService->uuid.uuid16 == gBleSig_HidService_d))
    {
        /* Found HID Service */
        mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if (pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c)
            {
                if(pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Report_d)
                {
                    /* Found HID Report Char */
                    mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReport = pService->aCharacteristics[i].value.handle;

                    for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                    {
                        if (( pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                        ( pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d))
                        {
                            mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd =  pService->aCharacteristics[i].aDescriptors[j].handle;
                        }
                    }
                }
                else if(pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_BootMouseInputReport_d)
                {
                    /* Found HID Boot Mode Report Char */
                    mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidBootReport = pService->aCharacteristics[i].value.handle;

                    for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                    {
                        if (( pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c) &&
                        ( pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16 == gBleSig_CCCD_d))
                        {
                            mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidBootReportCccd =  pService->aCharacteristics[i].aDescriptors[j].handle;
                        }
                    }
                }
                else
                {
                    /* For MISRA Compliance */
                }
            }
       }
    }
}

static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint8_t i;
    bool_t status = FALSE;

    for (i = 0; i < (pElement->length - 1U); i += iDataLen)
    {
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }
    return status;
}

static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData)
{
    uint32_t index = 0;
    uint8_t name[10];
    uint32_t nameLength = 0U;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for HID Service */
        if ((adElement.adType == gAdIncomplete16bitServiceList_c) ||
          (adElement.adType == gAdComplete16bitServiceList_c))
        {
            uint16_t uuid = gBleSig_HidService_d;
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid, (uint8_t)sizeof(uint16_t));
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = (uint32_t)MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + (uint8_t)sizeof(uint8_t);
    }

    if (foundMatch)
    {
        /* UI */
        (void)shell_write("\r\nFound device: \r\n");
        if (nameLength != 0U)
        {
            (void)shell_writeN((char*)name, nameLength - 1U);
            (void)SHELL_NEWLINE();
        }
    }
    return foundMatch;
}

static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, uint8_t event)
{
    switch (mPeerInformation[peerDeviceId].appState)
    {
        case mAppIdle_c:
        {
            BleApp_StateMachineHandler_IdleState(peerDeviceId, event);
        }
        break;

        case mAppExchangeMtu_c:
        {
            BleApp_StateMachineHandler_ExchangeMtuState(peerDeviceId, event);
        }
        break;

        case mAppServiceDisc_c:
        {
            BleApp_StateMachineHandler_ServiceDiscState(peerDeviceId, event);
        }
        break;

        case mAppReadDescriptorA_c:
        {
            BleApp_StateMachineHandler_ReadDescriptorAState(peerDeviceId, event);
        }
        break;
        
        case mAppReadDescriptorB_c:
        {
            BleApp_StateMachineHandler_ReadDescriptorBState(peerDeviceId, event);
        }
        break;

        case mAppRunning_c:
        {
            BleApp_StateMachineHandler_RunningState(peerDeviceId, event);
        }
        break;
    default:
        ; /* For MISRA Compliance */
        break;
    }
}

static bleResult_t BleApp_ConfigureNotifications(deviceId_t peerDeviceId, uint16_t handle)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = gCccdNotification_c;

    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( mpCharProcBuffer != NULL )
    {
        mpCharProcBuffer->handle = handle;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        (void)GattClient_WriteCharacteristicDescriptor(peerDeviceId,
                                                       mpCharProcBuffer,
                                                       (uint16_t)sizeof(value), (void*)&value);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

static void BluetoothLEHost_Initialized(void)
{
    char initStr[] = "BLE HID Host>";

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

   /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    for (uint8_t i = 0U; i < gAppMaxConnections_c; i++)
    {
        mPeerInformation[i].deviceId = gInvalidDeviceId_c;
        mPeerInformation[i].appState = mAppIdle_c;
    }

    mScanningOn = FALSE;
	mFoundDeviceToConnect = FALSE;
	
    /* UI */
    (void)SHELL_Init((shell_handle_t)g_shellHandle, (serial_handle_t)gSerMgrIf, initStr);
    (void)shell_write("\r\nPress SCANSW to connect to a HID Device!\r\n");
}

static void BleApp_StateMachineHandler_IdleState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_PeerConnected_c)
    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
        if (mRestoringBondedLink &&
            (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d) )
        {
            /* Moving to Running State and wait for Link encryption result */
            mPeerInformation[peerDeviceId].appState = mAppRunning_c;
        }
        else
#endif /* gAppUseBonding_d */
        {
            /* Moving to Exchange MTU State */
            mPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
            (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
        }
    }
}

static void BleApp_StateMachineHandler_ExchangeMtuState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        /* Moving to Primary Service Discovery State*/
        mPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

        /* Start Service Discovery*/
        if (gBleSuccess_c != BleServDisc_Start(peerDeviceId) )
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
    else if (event == mAppEvt_GattProcError_c)
    {
        (void)Gap_Disconnect(peerDeviceId);
    }
    else
    {
        ; /* For MISRA rule 15.7 compliance */
    }
}

static void BleApp_StateMachineHandler_ServiceDiscState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_ServiceDiscoveryComplete_c)
    {
        /* Moving to Primary Service Discovery State*/
        mPeerInformation[peerDeviceId].appState = mAppReadDescriptorB_c;

        if (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d)
        {
            mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
            if (mpCharProcBuffer == NULL)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                mpCharProcBuffer->handle = mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd;
                mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
                (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation[peerDeviceId].deviceId, mpCharProcBuffer ,23);
            }
        }
    }
    else if (event == mAppEvt_ServiceDiscoveryFailed_c)
    {
        (void)Gap_Disconnect(peerDeviceId);
    }
    else
    {
        ; /* For MISRA rule 15.7 compliance */
    }
}

static void BleApp_StateMachineHandler_ReadDescriptorAState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd != gGattDbInvalidHandle_d)
        {
            if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd) )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* Moving to Running State*/
                mPeerInformation[peerDeviceId].appState = mAppRunning_c;
            }
        }
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */
        (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer , gAttDefaultMtu_c);
    }
    else
    {
        /* For MISRA Compliance */
    }
}

static void BleApp_StateMachineHandler_ReadDescriptorBState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidBootReportCccd != gGattDbInvalidHandle_d)
        {
            if( gBleSuccess_c != BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidBootReportCccd) )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* Moving to  mAppReadDescriptorA_c State*/
                mPeerInformation[peerDeviceId].appState = mAppReadDescriptorA_c;
            }
        }
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        /* Continue after pairing is complete */
        (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer , gAttDefaultMtu_c);
    }
    else
    {
        /* For MISRA Compliance */
    }
}

static void BleApp_StateMachineHandler_RunningState(deviceId_t peerDeviceId, uint8_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mpCharProcBuffer != NULL)
        {
            (void)MEM_BufferFree(mpCharProcBuffer);
            mpCharProcBuffer = NULL;
        }

#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
        /* Write data in NVM */
        (void)Gap_SaveCustomPeerInformation(peerDeviceId,
                                      (void*) &mPeerInformation[peerDeviceId].customInfo, 0,
                                      (uint16_t)(sizeof(appCustomInfo_t)));
#endif /* gAppUseBonding_d */
    }
    else if (event == mAppEvt_PairingComplete_c)
    {
        if( gBleSuccess_c !=  BleApp_ConfigureNotifications(peerDeviceId, mPeerInformation[peerDeviceId].customInfo.hidClientConfig.hHidReportCccd) )
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
    else
    {
        /* For MISRA Compliance */
    }
}
/*! *********************************************************************************
* @}
********************************************************************************** */
