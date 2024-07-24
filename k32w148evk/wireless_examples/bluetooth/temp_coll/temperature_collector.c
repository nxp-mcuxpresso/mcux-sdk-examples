/*! *********************************************************************************
* \addtogroup Temperature Collector
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the Temperature Collector application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "EmbeddedTypes.h"
#include "RNG_Interface.h"
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_panic.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_serial_manager.h"
#include "fsl_format.h"
#include "FunctionLib.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "temperature_interface.h"

/* Wrappers */
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "app_conn.h"
#include "app_scanner.h"
#include "board.h"
#include "app.h"
#include "temperature_collector.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define    gKBD_EventPB1_c       1U    /* Pushbutton 1 */
#define    gKBD_EventPB2_c       2U    /* Pushbutton 2 */
#define    gKBD_EventLongPB1_c   3U    /* Pushbutton 1 */
#define    gKBD_EventLongPB2_c   4U    /* Pushbutton 2 */
#define    gKBD_EventInvalid_c   0xFFU /* Invalid key event */
#define LedTurnOffAllLeds() LedStopFlashingAllLeds()
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

typedef enum appEvent_tag{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
}appEvent_t;

typedef enum appState_tag{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppReadDescriptor_c,
    mAppRunning_c,
}appState_t;

typedef struct appCustomInfo_tag
{
    tmcConfig_t     tempClientConfig;
    /* Add persistent information here */
}appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
}appPeerInfo_t;

typedef uint8_t key_event_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;
#if gAppUsePrivacy_d
static bool_t mAttemptRpaResolvingAtConnect = FALSE;
#endif
#endif

static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;

/* Timers */
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);

/*temp collector serial manager handle*/
static serial_handle_t gAppSerMgrIf;
/*temp collector write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_writeHandle);

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
    deviceId_t peerDeviceId,
    servDiscEvent_t* pEvent
);

static void BleApp_StateMachineHandlerRunning
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StateMachineHandlerReadDescriptor
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StateMachineHandlerServiceDisc
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StateMachineHandlerExchangeMtu
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StateMachineHandlerIdle
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static bool_t CheckScanEvent(gapScannedDevice_t* pData);

static void BleApp_StoreServiceHandles
(
    gattService_t   *pService
);

static void BleApp_StoreDescValues
(
    gattAttribute_t     *pDesc
);

static void BleApp_PrintTemperature
(
    uint16_t temperature
);

static bleResult_t BleApp_ConfigureNotifications(void);

static void ScanningTimeoutTimerCallback(void* pParam);
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
static void DisconnectTimerCallback(void* pParam);
#endif

static void AppPrintString(const char* pBuff);
static void AppPrintHexLe( uint8_t* pHex, uint8_t len);
static void AppPrintDec(uint32_t dec);
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
#endif

static void BluetoothLEHost_Initialized(void);
static void BleApp_SerialInit(void);
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void BleApp_HandleKeys(key_event_t events);
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    BleApp_SerialInit();
    LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    {
        union
        {
            void* pCbkParam;
            uint32_t buttonPressed;
        }cbkParam;
        cbkParam.buttonPressed = 1UL;
        (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, cbkParam.pCbkParam);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
        cbkParam.buttonPressed = 2UL;
        (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys0, cbkParam.pCbkParam);
#endif
    }
#endif
    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (!mScanningOn)
    {
        /* Start scanning */
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
    union
    {
        void* pCbkParam;
        uint32_t buttonPressed;
    }cbkParam;
    (void)buttonHandle;
    key_event_t keyEvent = gKBD_EventInvalid_c;
    cbkParam.pCbkParam = callbackParam;
    switch (message->event)
    {

    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        {
            if(cbkParam.buttonPressed == 2U)
            {
                keyEvent = gKBD_EventPB2_c;
            }
            else
            {
                keyEvent = gKBD_EventPB1_c;
            }
        }
        break;
    case kBUTTON_EventLongPress:
        {
            if(cbkParam.buttonPressed == 2U)
            {
                keyEvent = gKBD_EventLongPB2_c;
            }
            else
            {
                keyEvent = gKBD_EventLongPB1_c;
            }
        }
        break;

    default:
        {
            ; /* No action required */
            break;
        }
    }

    if(keyEvent !=  gKBD_EventInvalid_c)
    {
        BleApp_HandleKeys(keyEvent);
    }
    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
    case gLePhyEvent_c:
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
        if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
        {
            AppPrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
        }
#endif
        break;

    default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, filter accept list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    /* Initialize private variables */
    mPeerInformation.appState = mAppIdle_c;
    mScanningOn = FALSE;
    mFoundDeviceToConnect = FALSE;

    /* Allocate scan timeout timer */
    (void)TM_Open(mAppTimerId);

    AppPrintString("\r\nTemperature collector -> Press SCANSW to connect to a Temperature Sensor.\r\n");


}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
#if gAppUsePrivacy_d && gAppUseBonding_d
    uint8_t bondedDevicesCnt = 0U;
#endif

    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            /* Check if the scanned device implements the Temperature Custom Profile */
            if( FALSE == mFoundDeviceToConnect )
            {
                mFoundDeviceToConnect = CheckScanEvent(&pScanningEvent->eventData.scannedDevice);

                if (mFoundDeviceToConnect)
                {
                    /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                mFoundDeviceToConnect = FALSE;

                AppPrintString("Scanning...\r\n");

                /* Start scanning timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimeoutTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gScanningTime_c);
                LedStopFlashingAllLeds();
                Led1Flashing();
            }
            /* Node is not scanning */
            else
            {
                (void)TM_Stop((timer_handle_t)mAppTimerId);

                /* Connect with the previously scanned peer device */
                if (mFoundDeviceToConnect)
                {
#if gAppUsePrivacy_d
                    if(gConnReqParams.peerAddressType == gBleAddrTypeRandom_c)
                    {
#if gAppUseBonding_d
                        /* Check if there are any bonded devices */
                        (void)Gap_GetBondedDevicesCount(&bondedDevicesCnt);

                        if(bondedDevicesCnt == 0U)
                        {
                            /* display the unresolved RPA address */
                            AppPrintHexLe( gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                            AppPrintString("\r\n");
                        }
                        else
                        {
                            mAttemptRpaResolvingAtConnect = TRUE;
                        }
#else
                        /* If bonding is disabled and we receive an RPA address there is nothing to do but display it */
                        AppPrintHexLe( gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                        AppPrintString("\r\n");
#endif /* gAppUseBonding_d */
                    }
                    else
                    {
                        /* display the public/resolved address */
                        AppPrintHexLe( gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                        AppPrintString("\r\n");
                    }
#else
                    /* Display the peer address */
                    AppPrintHexLe( gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                    AppPrintString("\r\n");
#endif /* gAppUsePrivacy_d */

                    AppPrintString("Scan stopped.\r\n");
                    AppPrintString("Connecting...\r\n");
                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
                else
                {
                    AppPrintString("Scan stopped.\r\n");
                    LedStopFlashingAllLeds();
                    LedStartFlashingAllLeds();
                }
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            ; /* No action required */
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
#if gAppUsePrivacy_d && gAppUseBonding_d
            if(mAttemptRpaResolvingAtConnect == TRUE)
            {
                /* If the peer RPA was resolved, the IA is displayed, otherwise the peer RPA address is displayed */
                AppPrintHexLe( pConnectionEvent->eventData.connectedEvent.peerAddress, gcBleDeviceAddressSize_c);
                AppPrintString("\r\n");
                /* clear the flag */
                mAttemptRpaResolvingAtConnect = FALSE;
            }
#endif
            /* Update UI */
            LedStopFlashingAllLeds();
            Led1On();

            AppPrintString("Connected!\r\n");

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            bool_t isBonded = FALSE;
            (void)Gap_CheckIfBonded(peerDeviceId, &isBonded, NULL);

            if (isBonded &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.customInfo, 0U, (uint16_t)sizeof(appCustomInfo_t))))
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
#endif
            BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;
            mPeerInformation.appState = mAppIdle_c;

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            AppPrintString("Disconnected!\r\n");

            /* If peer device disconnects the link during Service Discovery, free the allocated buffer */
            if(mpCharProcBuffer != NULL)
            {
                (void)MEM_BufferFree(mpCharProcBuffer);
                mpCharProcBuffer = NULL;
            }

            LedTurnOffAllLeds();
            LedStartFlashingAllLeds();
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            /* Notify state machine handler on pairing complete */
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                mPeerInformation.isBonded = TRUE;
                mAuthRejected = FALSE;
#endif
                BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PairingComplete_c);
                AppPrintString("\r\n-->  GAP Event: Device Paired.\r\n");
            }
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            else
            {
                 mPeerInformation.isBonded = FALSE;
                 AppPrintString("\r\n-->  GAP Event: Pairing Unsuccessful.\r\n");
            }
#endif
        }
        break;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    /* Try to enable temperature notifications, disconnect on failure */
                    mPeerInformation.isBonded = TRUE;
                    if( gBleSuccess_c != BleApp_ConfigureNotifications() )
                    {
                        (void)Gap_Disconnect(peerDeviceId);
                    }
                    else
                    {
                        mRestoringBondedLink = FALSE;
                    }
                }
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            mAuthRejected = TRUE;
            /* Start Pairing Procedure */
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

    default:
        ; /* No action required */
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles discovered services.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pEvent              Pointer to servDiscEvent_t.
********************************************************************************** */
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        /* Store the discovered handles for later use. */
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(pEvent->eventData.pService);
        }
        break;

        /* Service discovery has finished, run the state machine. */
        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryFailed_c);
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
* \brief        Stores handles for the specified service.
*
* \param[in]    pService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
    gattService_t   *pService
)
{
    uint8_t i,j;

    if ((pService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pService->uuid.uuid128, uuid_service_temperature, 16))
    {
        /* Found Temperature Service */
        mPeerInformation.customInfo.tempClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Temperature_d))
            {
                /* Found Temperature Char */
                mPeerInformation.customInfo.tempClientConfig.hTemperature = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c)
                    {
                        switch (pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16)
                        {
                            /* Found Temperature Char Presentation Format Descriptor */
                            case gBleSig_CharPresFormatDescriptor_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempDesc = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            /* Found Temperature Char CCCD */
                            case gBleSig_CCCD_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempCccd = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            default:
                                ; /* No action required */
                                break;
                        }
                    }
                }
            }
        }
    }
}

/*! *********************************************************************************
* \brief        Stores the value of the specified attribute.
*
* \param[in]    pDesc       Pointer to gattAttribute_t.
********************************************************************************** */
static void BleApp_StoreDescValues
(
    gattAttribute_t     *pDesc
)
{
    if (pDesc->handle == mPeerInformation.customInfo.tempClientConfig.hTempDesc)
    {
        /* Store temperature format*/
        FLib_MemCpy(&mPeerInformation.customInfo.tempClientConfig.tempFormat,
                    pDesc->paValue,
                    pDesc->valueLength);
    }

}

/*! *********************************************************************************
* \brief        Displays the temperature value to shell.
*
* \param[in]    temperature     Temperature value
********************************************************************************** */
static void BleApp_PrintTemperature
(
    uint16_t temperature
)
{
    AppPrintString("Temperature: ");
    AppPrintDec((uint32_t)temperature / 100UL);
    /* Add 'C' for Celsius degrees - UUID 0x272F.
       www.bluetooth.com/specifications/assigned-numbers/units */
    if (mPeerInformation.customInfo.tempClientConfig.tempFormat.unitUuid16 == 0x272FU)
    {
        AppPrintString(" C\r\n");
    }
    else
    {
        AppPrintString("\r\n");
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
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    /* Do not process GATT Server messages unless a trusted relationship was established */
    if ((mPeerInformation.isBonded) || (mPeerInformation.appState != mAppRunning_c))
    {
#endif
        if (procedureResult == gGattProcError_c)
        {
            attErrorCode_t attError = (attErrorCode_t)(uint8_t)(error);

            if (attError == gAttErrCodeInsufficientEncryption_c     ||
                attError == gAttErrCodeInsufficientAuthorization_c  ||
                attError == gAttErrCodeInsufficientAuthentication_c)
            {
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U))
                /* Start Pairing Procedure */
                (void)Gap_Pair(serverDeviceId, &gPairingParameters);
#endif
            }

            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
        }
        else
        {
            if (procedureResult == gGattProcSuccess_c)
            {
                switch(procedureType)
                {
                    case gGattProcReadCharacteristicDescriptor_c:
                    {
                        if (mpCharProcBuffer != NULL)
                        {
                            /* Store the value of the descriptor */
                            BleApp_StoreDescValues(mpCharProcBuffer);
                        }
                        break;
                    }

                    default:
                    {
                        ; /* No action required */
                        break;
                    }
                }

                BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
            }
        }

        /* Signal Service Discovery Module */
        BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    }
#endif
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
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    /* Do not process GATT Server notifications unless a trusted relationship was established */
    if (mPeerInformation.isBonded == TRUE)
    {
#endif
        if (characteristicValueHandle == mPeerInformation.customInfo.tempClientConfig.hTemperature)
        {
            BleApp_PrintTemperature(Utils_ExtractTwoByteValue(aValue));

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
            /* Restart Wait For Data timer */
            (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gWaitForDataTime_c);
#endif
        }
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    }
#endif
}

/*! *********************************************************************************
* \brief        Detect whether the provided data is found in the advertising data.
*
* \param[in]    pElement                    Pointer a to AD structure.
* \param[in]    pData                       Data to look for.
* \param[in]    iDataLen                    Size of data to look for.
*
* \return       TRUE if data matches, FALSE if not
********************************************************************************** */
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint32_t i;
    bool_t status = FALSE;

    for (i = 0; i < (uint32_t)pElement->length - 1UL; i += iDataLen)
    {
        /* Compare input data with advertising data. */
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }
    return status;
}

/*! *********************************************************************************
* \brief        Process scanning events to search for the Temperature Custom Service.
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to gapScannedDevice_t.
*
* \return       TRUE if the scanned device implements the Temperature Custom Service,
                FALSE otherwise
********************************************************************************** */
static bool_t CheckScanEvent(gapScannedDevice_t* pData)
{
    uint32_t index = 0;
    char name[11];
    uint8_t nameLength = 0;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for Temperature Custom Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c) ||
          (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid_service_temperature, 16);
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch && (nameLength > 0U))
    {
        /* Update UI */
        AppPrintString("Found device: \r\n");
        name[nameLength-1U] = '\0';
        AppPrintString((const char*)name);
        AppPrintString("\r\n");
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application for mAppRunning_c state.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandlerRunning(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mpCharProcBuffer != NULL)
        {
            (void)MEM_BufferFree(mpCharProcBuffer);
            mpCharProcBuffer = NULL;
        }

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        /* Write data in NVM */
        (void)Gap_SaveCustomPeerInformation(mPeerInformation.deviceId,
                                            (void *)&mPeerInformation.customInfo, 0U,
                                            (uint16_t)sizeof(appCustomInfo_t));
#endif
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
        /* Start Wait For Data timer */
        (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gWaitForDataTime_c);
#endif
    }
    else
    {
        if (event == mAppEvt_PairingComplete_c)
        {
            /* Try to enable temperature notifications, disconnect on failure */
            if( gBleSuccess_c != BleApp_ConfigureNotifications() )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
        }
    }
}
/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application for mAppReadDescriptor_c state.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandlerReadDescriptor(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        if (mPeerInformation.customInfo.tempClientConfig.hTempCccd != 0U)
        {
            /* Try to enable temperature notifications, disconnect on failure */
            if( gBleSuccess_c != BleApp_ConfigureNotifications() )
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* Moving to Running State*/
                mPeerInformation.appState = mAppRunning_c;
            }
        }
    }
    else
    {
        if (event == mAppEvt_PairingComplete_c)
        {
            /* Continue after pairing is complete */
            (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, gAttDefaultMtu_c);
        }
    }
}
/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application for mAppServiceDisc_c state.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandlerServiceDisc(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_ServiceDiscoveryComplete_c)
    {
        /* Moving to Primary Service Discovery State*/
        mPeerInformation.appState = mAppReadDescriptor_c;

        if (mPeerInformation.customInfo.tempClientConfig.hTempDesc != 0U)
        {
            mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
            if (mpCharProcBuffer == NULL)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                mpCharProcBuffer->handle = mPeerInformation.customInfo.tempClientConfig.hTempDesc;
                mpCharProcBuffer->paValue = (uint8_t*)(&mpCharProcBuffer[1]);
                (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation.deviceId, mpCharProcBuffer, gAttDefaultMtu_c);
            }
        }
    }
    else
    {
        if (event == mAppEvt_ServiceDiscoveryFailed_c)
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
}
/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application for mAppExchangeMtu_c state.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandlerExchangeMtu(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_GattProcComplete_c)
    {
        /* Moving to Service Discovery State*/
        mPeerInformation.appState = mAppServiceDisc_c;

        /* Start Service Discovery*/
        (void)BleServDisc_Start(peerDeviceId);
    }
    else
    {
        if (event == mAppEvt_GattProcError_c)
        {
            (void)Gap_Disconnect(peerDeviceId);
        }
    }
}
/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application for mAppIdle_c state.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandlerIdle(deviceId_t peerDeviceId, appEvent_t event)
{
    if (event == mAppEvt_PeerConnected_c)
    {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        if (mPeerInformation.customInfo.tempClientConfig.hTemperature != gGattDbInvalidHandle_d)
        {
            /* Moving to Running State and wait for Link encryption result */
            mPeerInformation.appState = mAppRunning_c;
        }
        else
#endif
        {
            /* Moving to Exchange MTU State */
            mPeerInformation.appState = mAppExchangeMtu_c;
            (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
        }
    }
}

/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    switch (mPeerInformation.appState)
    {
    case mAppIdle_c:
        BleApp_StateMachineHandlerIdle(peerDeviceId, event);
        break;

    case mAppExchangeMtu_c:
        BleApp_StateMachineHandlerExchangeMtu(peerDeviceId, event);
        break;

    case mAppServiceDisc_c:
        BleApp_StateMachineHandlerServiceDisc(peerDeviceId, event);
        break;

    case mAppReadDescriptor_c:
        BleApp_StateMachineHandlerReadDescriptor(peerDeviceId, event);
        break;

    case mAppRunning_c:
        BleApp_StateMachineHandlerRunning(peerDeviceId, event);
        break;

    default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Enable temperature notifications by writing peer's CCCD of the
                Temperature characteristic.
*
* \return       gBleSuccess_c or gBleOutOfMemory_c
********************************************************************************** */
static bleResult_t BleApp_ConfigureNotifications(void)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = (uint16_t)gCccdNotification_c;

    /* Allocate buffer for the write operation */
    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( mpCharProcBuffer != NULL )
    {
        /* Populate the write request */
        mpCharProcBuffer->handle = mPeerInformation.customInfo.tempClientConfig.hTempCccd;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        (void)GattClient_WriteCharacteristicDescriptor(mPeerInformation.deviceId,
                                                       mpCharProcBuffer,
                                                       (uint16_t)sizeof(value), (void*)&value);
    }
    else
    {
        result = gBleOutOfMemory_c;
    }

    return result;
}

/*! *********************************************************************************
* \brief        Stop scanning after a given time (gScanningTime_c).
                Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void ScanningTimeoutTimerCallback(void* pParam)
{
    /* Stop scanning */
    if (mScanningOn)
    {
        (void)Gap_StopScanning();
    }
}

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
/*! *********************************************************************************
* \brief        Disconnect from peer device if no data was received for a given time.
                (gWaitForDataTime_c). Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    /* Disconnect from peer device */
    if (mPeerInformation.deviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(mPeerInformation.deviceId);
    }
}
#endif  /* gAppLowpowerEnabled_d */

/*! *********************************************************************************
* \brief        Prints a string.
*
********************************************************************************** */
static void AppPrintString( const char* pBuff)
{
    uint32_t buffLength = strlen(pBuff);
    union{
        const char* pcBuff;
        uint8_t* pBuff;
    }buff;
    buff.pcBuff = pBuff;
    (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, buff.pBuff, buffLength );
}


/*! *********************************************************************************
* \brief        Prints a len octets number in hexadecimal.
*
* \param[in]    pHex                Pointer to the number.
* \param[in]    len                 Number length.
********************************************************************************** */
static void AppPrintHexLe( uint8_t *pHex, uint8_t len)
{
    for(uint8_t i = len; i > 0U; i-- )
    {
        (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle,
                                    FORMAT_Hex2Ascii(pHex[i - 1U]),
                                    2U);
    }
}

/*! *********************************************************************************
* \brief        Prints a number in decimal.
*
********************************************************************************** */
static void AppPrintDec(uint32_t dec)
{
    uint8_t *pDec;
    pDec = FORMAT_Dec2Str(dec);
    (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, pDec, strlen((char const *)pDec));
}

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void PrintLePhyEvent(void(*pfPrint)(const char *pBuff),gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static const char* mLePhyModeStrings[] =
    {
        "Invalid\r\n",
        "1M\r\n",
        "2M\r\n",
        "Coded\r\n",
    };
    uint8_t txPhy = (pPhyEvent->txPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->txPhy : 0U;
    uint8_t rxPhy = (pPhyEvent->rxPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->rxPhy : 0U;
    pfPrint("Phy Update Complete.\r\n");
    pfPrint("TxPhy ");
    pfPrint(mLePhyModeStrings[txPhy]);
    pfPrint("RxPhy ");
    pfPrint(mLePhyModeStrings[rxPhy]);
}
#endif

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    PrintLePhyEvent(AppPrintString, pPhyEvent);
}
#endif

/*! *********************************************************************************
* \brief        Initializes serial interface.
*
********************************************************************************** */
static void BleApp_SerialInit(void)
{
    serial_manager_status_t status;

    /* UI */
    gAppSerMgrIf = (serial_handle_t)&gSerMgrIf[0];

    /*open wireless uart write handle*/
    status = SerialManager_OpenWriteHandle((serial_handle_t)gAppSerMgrIf, (serial_write_handle_t)s_writeHandle);
    assert(kStatus_SerialManager_Success == status);
    (void)status;
}

/*! *********************************************************************************
* \brief        Handle the buttons.
*
* \param[in]    events   the button event.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static void BleApp_HandleKeys(key_event_t events)
{
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
    /* Start automatically if low-power is enabled */
    BleApp_Start();
#else
    switch (events)
    {
        /* Start on button press if low-power is disabled */
        case gKBD_EventPB1_c:
        {
            BleApp_Start();
            break;
        }

        /* Disconnect on long button press */
        case gKBD_EventLongPB1_c:
        {
            if (mPeerInformation.deviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerInformation.deviceId);
            }
            break;
        }

        case gKBD_EventPB2_c:  /* Fall-through */
        case gKBD_EventLongPB2_c:   /* Fall-through */
        default:
        {
            ; /* No action required */
            break;
        }
    }
#endif
}
#endif /* (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0)) */
/*! *********************************************************************************
* @}
********************************************************************************** */
