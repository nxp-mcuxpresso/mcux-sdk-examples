/*! *********************************************************************************
* \addtogroup Extended Advertising Central
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2020 - 2024 NXP
*
*
* \file
*
* This file is the source file for the extended advertising central application
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
#include "fsl_component_serial_manager.h"
#include "fsl_component_mem_manager.h"
#include "FunctionLib.h"
#include "fsl_component_timer_manager.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "temperature_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
#include "app.h"
#include "app_conn.h"
#include "app_scanner.h"
#include "fsl_format.h"
#include "adv_ext_central.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mAppExtAdvListSize_c  (10U)
#define mBlePeriodicAdvInvalidSyncHandle_c  (0x1FFFU)
#define mPeriodicExtAdvInvalidIndex_c  (0xFFU)

#define AppPrintString(pBuff) (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, (uint8_t *)&(pBuff)[0], strlen(pBuff))

#define App_ExtractTwoByteValue(buf) \
    (((uint16_t)(*(buf))) | ( ((uint16_t)((buf)[1])) << 8U) )
      
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

typedef struct extAdvListElement_tag
{
    bleDeviceAddress_t aAddress;
    uint8_t SID;
    uint32_t dataCRC;
    uint32_t perDataCRC;
    uint16_t periodicAdvInterval;
    uint16_t syncHandle;
}extAdvListElement_t;

typedef struct perAdvListElement_tag
{
    uint16_t syncHandle;
    uint32_t dataCRC;
}perAdvListElement_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;
#endif

static bool_t   mScanningOn = FALSE;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static bool_t   mDbafScanning = FALSE;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
static bool_t   mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;

/*serial manager handle*/
static serial_handle_t gAppSerMgrIf;
/*write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_writeHandle);
/*read handle*/
static SERIAL_MANAGER_READ_HANDLE_DEFINE(s_readHandle);

/* Timers */
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);
static char* maAdvEvtConnStrings[] = { "\n\rNon Connectable", "\n\rConnectable" };
static char* maAdvEvtScannStrings[] = { "\n\rNon Scannable", "\n\rScannable" };
static char* maAdvEvtDirStrings[] = { "\n\rUndirected", "\n\rDirected" };
static char* maAdvEvtDataTypeStrings[] = { "\n\rAdv Data", "\n\rScan Req Data" };
static char* maAdvEvtAdvTypeStrings[] = { "\n\rExtended Advertising", "Legacy Advertising" };
static char** maAdvPropStrings[] = {maAdvEvtConnStrings, maAdvEvtScannStrings, maAdvEvtDirStrings, maAdvEvtDataTypeStrings, maAdvEvtAdvTypeStrings};
static char* maLePhyStrings[] = { "", "gLePhy1M_c", "gLePhy2M_c", "gLePhyCoded_c" };
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static char* maScanStrings[] = {"Passive", "Active", "DBAF"};
#else
static char* maScanStrings[] = {"Passive", "Active"};
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
#if mAE_CentralDebug_c
static char* maScanEventStrings[] = {\
    "gScanStateChanged_c",\
    "gScanCommandFailed_c",\
    "gDeviceScanned_c",\
    "gExtDeviceScanned_c",\
    "gPeriodicDeviceScanned_c",\
    "gPeriodicAdvSyncEstablished_c",\
    "gPeriodicAdvSyncLost_c",\
    "gPeriodicAdvSyncTerminated_c"\
};
#endif
static extAdvListElement_t maAppExtAdvList[mAppExtAdvListSize_c];
static uint8_t mAppExtAdvListIndex;
static uint8_t mPerExtAdvIndexPending = mPeriodicExtAdvInvalidIndex_c;

static appScanningParams_t mAppScanParams = {
    &gScanParams,
    gGapDuplicateFilteringDisable_c,
    gGapScanContinuously_d,
    gGapScanPeriodicDisabled_d
};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Host Stack callbacks */
static void BleApp_ScanningCallback( gapScanningEvent_t* pScanningEvent );
static void BleApp_ConnectionCallback( deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent );
static void BleApp_GattClientCallback( deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t procedureResult, bleResult_t error );
static void BleApp_GattNotificationCallback( deviceId_t serverDeviceId, uint16_t characteristicValueHandle, uint8_t* aValue, uint16_t valueLength );
static void BleApp_ServiceDiscoveryCallback( deviceId_t peerDeviceId, servDiscEvent_t* pEvent );
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);
static void BluetoothLEHost_Initialized(void);
void BleApp_StateMachineHandler( deviceId_t peerDeviceId, appEvent_t event );
static bool_t CheckScanEvent(gapScanningEvent_t* pScanningEvent);
static void BleApp_StoreServiceHandles( gattService_t *pGattService );
static void BleApp_StoreDescValues( gattAttribute_t *pDesc );
static void BleApp_PrintTemperature( uint16_t temperature );
static bleResult_t BleApp_ConfigureNotifications(void);
static void ScanningTimeoutTimerCallback(void* pParam);
static void DisconnectTimerCallback(void* pParam);
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
static void AppPrintDec(uint32_t nb);
static void AppPrintHexLe( uint8_t *pHex, uint8_t len);
static bool_t CheckForAEPeripheralDevice(uint8_t* pData, uint16_t dataLength);
static void AppHandleExtAdvEvent( gapExtScannedDevice_t* pExtScannedDevice);
static void AppHandlePeriodicDeviceScanEvent( gapPeriodicScannedDevice_t* pGapPeriodicScannedDevice);
void AppTerminatePeriodicAdvSync(void);
static void BleApp_SerialInit(void);

static void BleApp_HandleScanStateChanged(void);
static void BleApp_HandleDeviceScanned(gapScanningEvent_t* pScanningEvent);
static void BleApp_HandleExtDeviceScanned(gapScanningEvent_t* pScanningEvent);

static void BleApp_HandleIdleState(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_HandleExchangeMtuState(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_HandleServiceDiscState(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_HandleReadDescriptorState(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_HandleRunningState(deviceId_t peerDeviceId, appEvent_t event);

static void BleApp_HandlePeriodicAdv( gapExtScannedDevice_t* pExtScannedDevice, uint8_t advIndex);
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /*gAppButtonCnt_c > 0*/
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static void Uart_PrintMenu(void *pData);
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    For Hard Fault Debug Purposes.
*
********************************************************************************** */
#if mAE_CentralDebug_c
void HardFault_Handler(void)
{
    while(1);
}
#endif
/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    BleApp_SerialInit();
    LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif
    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    /* Start scanning */
    mAppExtAdvListIndex = 0;
    mPerExtAdvIndexPending = mPeriodicExtAdvInvalidIndex_c;
    (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    (void)buttonHandle;
    (void)callbackParam;
    (void)message;
    switch (message->event)
    {
        /* Start the application */
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        {
            if (!mScanningOn)
            {
                gScanParams.type = gScanTypeActive_c;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
                if (mDbafScanning == TRUE)
                {
                    gConnReqParams.filterPolicy = gUseOnlyDecisionPDUs_c;
                    gScanParams.filterPolicy = (uint8_t)gScanOnlyDecisionPDUs_c;
                    (void)Gap_SetDecisionInstructions(1U, &gDbafDecisionInstructions);
                }
                else
                {
                    gScanParams.filterPolicy = (uint8_t)gScanAll_c;
                    gConnReqParams.filterPolicy = gUseDeviceAddress_c;
                    BleApp_Start();
                }
#else
                BleApp_Start();
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
            }
        }
        break;
        /* Start the application */
    case kBUTTON_EventLongPress:
        {
            if (!mScanningOn)
            {
                gScanParams.type = gScanTypePassive_c;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
                if (mDbafScanning == TRUE)
                {
                    gConnReqParams.filterPolicy = gUseOnlyDecisionPDUs_c;
                    gScanParams.filterPolicy = (uint8_t)gScanOnlyDecisionPDUs_c;
                    (void)Gap_SetDecisionInstructions(1U, &gDbafDecisionInstructions);
                }
                else
                {
                    gScanParams.filterPolicy = (uint8_t)gScanAll_c;
                    gConnReqParams.filterPolicy = gUseDeviceAddress_c;
                    BleApp_Start();
                }
#else
                BleApp_Start();
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
            }
        }
        break;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
        /* Toggle Extended/DBAF scanning */
    case kBUTTON_EventDoubleClick:
        {
            mDbafScanning = !mDbafScanning;
            
            if (mDbafScanning == TRUE)
            {
                /* Set connection parameters for Decision IND PDUs */
                gConnReqParams.filterPolicy = gUseOnlyDecisionPDUs_c;
                /* Set scanning filter policy for DBAF Decision Indication PDUs */
                gScanParams.filterPolicy = (uint8_t)gScanOnlyDecisionPDUs_c;
            }
            else
            {
                /* Set default value for connection request filter policy */
                gConnReqParams.filterPolicy = gUseDeviceAddress_c;
                /* Set default value for scanning filter policy */
                gScanParams.filterPolicy = (uint8_t)gScanAll_c;
            }
            
            (void)App_PostCallbackMessage(Uart_PrintMenu, NULL);
        }
        break;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
    default:
        {
            ; /* No action required */
        }
        break;
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
        if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
        {
            AppPrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
        }
        break;
    case gRandomAddressReady_c:
        AppPrintString("\n\rOwn Address ");
        AppPrintHexLe(pGenericEvent->eventData.addrReady.aAddress, 6);
        break;
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    case gDecisionInstructionsSetupComplete_c:
        BleApp_Start();
        break;
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
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
    (void)TM_Open((timer_handle_t)mAppTimerId);

    /* Update UI */
    AppPrintString("\n\rExtended Advertising Application - Central");
#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
    Uart_PrintMenu(NULL);
#else
    AppPrintString("\r\nPress WAKESW to Start Active Scanning!");
    AppPrintString("\r\nPress WAKESW Long to Start Passive Scanning!\r\n");
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */

    LedStopFlashingAllLeds();
    Led1On();
    Led2On();
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */

static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
#if mAE_CentralDebug_c
    AppPrintString("\n\rScan Callback - ");
    AppPrintString(maScanEventStrings[pScanningEvent->eventType]);
#endif
    switch (pScanningEvent->eventType)
    {
    case gScanStateChanged_c:
        {
            BleApp_HandleScanStateChanged();
        }
        break;

    case gScanCommandFailed_c:
        {
            ; /* No action required */
        }
        break;
    case gDeviceScanned_c:
        {
            BleApp_HandleDeviceScanned(pScanningEvent);
        }
        break;
    case gExtDeviceScanned_c:
        {
            BleApp_HandleExtDeviceScanned(pScanningEvent);
        }
        break;
    case gPeriodicDeviceScanned_c:
        {
#if mAE_CentralDebug_c
            AppPrintString("\r\nPeriodic Advertising Found");
#endif
            AppHandlePeriodicDeviceScanEvent(&pScanningEvent->eventData.periodicScannedDevice);
        }
        break;
    case gPeriodicAdvSyncEstablished_c:
        {
            AppPrintString("\r\nPeriodic Adv Sync Established");
            if((mPerExtAdvIndexPending != mPeriodicExtAdvInvalidIndex_c) && (mPerExtAdvIndexPending < mAppExtAdvListIndex))
            {
                maAppExtAdvList[mPerExtAdvIndexPending].syncHandle = pScanningEvent->eventData.syncEstb.syncHandle;
                maAppExtAdvList[mPerExtAdvIndexPending].perDataCRC = 0;
            }
            mPerExtAdvIndexPending = mPeriodicExtAdvInvalidIndex_c;
        }
        break;
    case gPeriodicAdvSyncLost_c:
        {
            AppPrintString("\r\nPeriodic Adv Sync Lost");
            uint8_t i;
            for( i=0 ; i<mAppExtAdvListIndex ; i++)
            {
                if(maAppExtAdvList[i].syncHandle == pScanningEvent->eventData.syncLost.syncHandle)
                {
                    maAppExtAdvList[i].syncHandle = mBlePeriodicAdvInvalidSyncHandle_c;
                    maAppExtAdvList[i].periodicAdvInterval = 0;
                    break;
                }
            }
        }
        break;
    case gPeriodicAdvSyncTerminated_c:
        AppPrintString("\r\nPeriodic Adv Sync Terminated");
        if((mPerExtAdvIndexPending != mPeriodicExtAdvInvalidIndex_c) && (mPerExtAdvIndexPending < mAppExtAdvListIndex))
        {
            maAppExtAdvList[mPerExtAdvIndexPending].syncHandle = mBlePeriodicAdvInvalidSyncHandle_c;

        }
        mPerExtAdvIndexPending = mPeriodicExtAdvInvalidIndex_c;
        if (!mScanningOn)
        {
            AppTerminatePeriodicAdvSync();
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
            AppPrintString("\r\nConnected!\r\n");
            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

            /* Update UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_Blue);
            Led1Flashing();
#else /* gAppLedCnt_c */
            Led2Flashing();
#endif /* gAppLedCnt_c */

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            bool_t isBonded = FALSE;
            (void)Gap_CheckIfBonded(peerDeviceId, &isBonded, NULL);

            if ((isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId, (void*) &mPeerInformation.customInfo, 0U, (uint16_t)sizeof (appCustomInfo_t))))
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

            /* If peer device disconnects the link during Service Discovery, free the allocated buffer */
            if(mpCharProcBuffer != NULL)
            {
                (void)MEM_BufferFree(mpCharProcBuffer);
                mpCharProcBuffer = NULL;
            }

            /* Update UI */
            AppPrintString("\r\nDisconnected!\r\n");
            /* UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_White);
            Led1On();
#else /* gAppLedCnt_c */
            Led2Off();
            Led2On();
#endif /* gAppLedCnt_c */
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
            }
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            else
            {
                mPeerInformation.isBonded = FALSE;
            }
#endif
        }
        break;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                mPeerInformation.isBonded = TRUE;
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    /* Try to enable temperature notifications, disconnect on failure */
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
* \param[in]    pGattService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles
(
gattService_t   *pGattService
)
{
    uint8_t i,j;

    if ((pGattService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pGattService->uuid.uuid128, uuid_service_temperature, 16))
    {
        /* Found Temperature Service */
        mPeerInformation.customInfo.tempClientConfig.hService = pGattService->startHandle;

        for (i = 0; i < pGattService->cNumCharacteristics; i++)
        {
            if ((pGattService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pGattService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Temperature_d))
            {
                /* Found Temperature Char */
                mPeerInformation.customInfo.tempClientConfig.hTemperature = pGattService->aCharacteristics[i].value.handle;

                for (j = 0; j < pGattService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (pGattService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c)
                    {
                        switch (pGattService->aCharacteristics[i].aDescriptors[j].uuid.uuid16)
                        {
                            /* Found Temperature Char Presentation Format Descriptor */
                        case gBleSig_CharPresFormatDescriptor_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempDesc = pGattService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            /* Found Temperature Char CCCD */
                        case gBleSig_CCCD_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempCccd = pGattService->aCharacteristics[i].aDescriptors[j].handle;
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
        FLib_MemCpy(&mPeerInformation.customInfo.tempClientConfig.tempFormat, pDesc->paValue, pDesc->valueLength);
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
static void BleApp_GattClientCallback
(
deviceId_t              serverDeviceId,
gattProcedureType_t     procedureType,
gattProcedureResult_t   procedureResult,
bleResult_t             error
)
{
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
    if ((mPeerInformation.isBonded) || (mPeerInformation.appState != mAppRunning_c))
    {
#endif
        if (procedureResult == gGattProcError_c)
        {
            attErrorCode_t attError = (attErrorCode_t)(uint8_t)(error);

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
    if (mPeerInformation.isBonded)
    {
#endif
        if (characteristicValueHandle == mPeerInformation.customInfo.tempClientConfig.hTemperature)
        {
            BleApp_PrintTemperature(App_ExtractTwoByteValue(aValue));
            /* Restart Wait For Data timer */
            (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer | (uint8_t)kTimerModeSingleShot, gWaitForDataTime_c);
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

    for (i = 0; i < ((uint32_t)pElement->length - 1UL); i += iDataLen)
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
* \param[in]    pScanningEvent                   Pointer to the gapScanningEvent_t structure provided by the scanning callback.
*
* \return       TRUE if the scanned device implements the Temperature Custom Service,
                FALSE otherwise
********************************************************************************** */
static bool_t CheckScanEvent(gapScanningEvent_t* pScanningEvent)
{
    uint32_t index = 0;
    uint8_t name[13];
    uint8_t lineEnd[] = "\n\r";
    uint8_t nameLength = 0;
    bool_t foundMatch = FALSE;
    uint8_t* pData;
    uint16_t dataLength;
    uint8_t* pAddress;
    if(pScanningEvent->eventType == gDeviceScanned_c)
    {
        pData = pScanningEvent->eventData.scannedDevice.data;
        dataLength = pScanningEvent->eventData.scannedDevice.dataLength;
        pAddress = pScanningEvent->eventData.scannedDevice.aAddress;
    }
    else
    {
        pData = pScanningEvent->eventData.extScannedDevice.pData;
        dataLength = pScanningEvent->eventData.extScannedDevice.dataLength;
        pAddress = pScanningEvent->eventData.extScannedDevice.aAddress;
    }

    while (index < dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData[index];
        adElement.adType = (gapAdType_t)pData[index + 1U];
        adElement.aData = &pData[index + 2U];

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
            FLib_MemCpy(&name[nameLength], lineEnd, sizeof(lineEnd));
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch && (nameLength > 0U))
    {
        /* Update UI */
        AppPrintString("\r\nFound device: \r\n");
        AppPrintString((char *)name);
        AppPrintHexLe(pAddress, 6);
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        Process scanning events to search for the "EA*PRPH" name
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to advertising data.
*
* \return       TRUE if the scanned device name is "EA*PRPH",
                FALSE otherwise
********************************************************************************** */
static bool_t CheckForAEPeripheralDevice(uint8_t* pData, uint16_t dataLength)
{
    uint32_t index = 0;
    uint8_t nameLength = 0;
    bool_t foundMatch = FALSE;
    uint8_t eaString[] = "EA*PRPH";
    while (index < dataLength)
    {
        gapAdStructure_t adElement;
        adElement.length = pData[index];
        adElement.adType = (gapAdType_t)pData[index + 1U];
        adElement.aData = &pData[index + 2U];
        if ((adElement.adType == gAdShortenedLocalName_c) ||
            (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length - 1U, 8U);
            foundMatch =  FLib_MemCmp (adElement.aData, eaString, nameLength);
            if(foundMatch)
            {
                break;
            }
        }
        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    switch (mPeerInformation.appState)
    {
    case mAppIdle_c:
        {
            BleApp_HandleIdleState(peerDeviceId, event);
        }
        break;

    case mAppExchangeMtu_c:
        {
            BleApp_HandleExchangeMtuState(peerDeviceId, event);
        }
        break;

    case mAppServiceDisc_c:
        {
            BleApp_HandleServiceDiscState(peerDeviceId, event);
        }
        break;

    case mAppReadDescriptor_c:
        {
            BleApp_HandleReadDescriptorState(peerDeviceId, event);
        }
        break;

    case mAppRunning_c:
        {
            BleApp_HandleRunningState(peerDeviceId, event);
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

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static char* mLePhyModeStrings[] =
    {
        "Invalid\n\r",
        "1M\n\r",
        "2M\n\r",
        "Coded\n\r",
    };
    uint8_t txPhy = (pPhyEvent->txPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->txPhy : (0U);
    uint8_t rxPhy = (pPhyEvent->rxPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->rxPhy : (0U);
    AppPrintString("Phy Update Complete.\n\r");
    AppPrintString("TxPhy ");
    AppPrintString(mLePhyModeStrings[txPhy]);
    AppPrintString("RxPhy ");
    AppPrintString(mLePhyModeStrings[rxPhy]);
}

/*! *********************************************************************************
* \brief        Prints a uint32_t value in decimal.
*
********************************************************************************** */
static void AppPrintDec(uint32_t nb)
{
    (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, FORMAT_Dec2Str(nb), strlen((char const *)FORMAT_Dec2Str(nb)));
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
* \brief        Computes Extended Adv Data CRC to decide whether adv data changed or not
*
* \param[in]    pData                Pointer to advertising data.
* \param[in]    dataLength           data length.
********************************************************************************** */
static uint32_t AppCalculateAdvDataCRC(const void* pData, uint16_t dataLength)
{
    uint32_t CRC = 0;
    const uint32_t* p32 = (const uint32_t*)pData;
    const uint8_t* p8;
    while(dataLength >= sizeof(uint32_t))
    {
        CRC += *p32++;
        dataLength -=  (uint16_t)sizeof(uint32_t);
    }
    p8 = (const uint8_t*)p32;
    while(dataLength != 0U)
    {
        CRC += *p8++;
        dataLength --;
    }
    return CRC;
}
/*! *********************************************************************************
* \brief        Prints Extended Adv Event
*
* \param[in]    pExtScannedDevice                pointer to gapExtScannedDevice_t structure provided by the scanning callback.
********************************************************************************** */
static void AppPrintExtAdvEvent( gapExtScannedDevice_t* pExtScannedDevice)
{
    uint16_t  dataLength;
    uint8_t i;
    uint8_t* pData;
    AppPrintString("\r\nExtended Advertising Found");
    AppPrintString("\n\rAdv Properties:");
    for(i = 0U ; i< 5U ; i++)
    {
        uint8_t j;
        j = ((pExtScannedDevice->advEventProperties & ((bleAdvReportEventProperties_t)(1U)<<i)) != 0U)? (1U):(0U);
        AppPrintString(maAdvPropStrings[i][j]);
    }
    AppPrintString("\n\rAdv Address ");
    if((pExtScannedDevice->advEventProperties & (((bleAdvReportEventProperties_t)1U)<<i)) != 0U)
    {
        AppPrintString("Anonymous");
    }
    else
    {
        AppPrintHexLe(pExtScannedDevice->aAddress, 6);
    }
    AppPrintString("\n\rData Set Id = ");AppPrintDec((uint32_t)pExtScannedDevice->SID);
    AppPrintString("\n\rPrimaryPHY = ");AppPrintString(maLePhyStrings[pExtScannedDevice->primaryPHY]);
    AppPrintString("\n\rSecondaryPHY = ");AppPrintString(maLePhyStrings[pExtScannedDevice->secondaryPHY]);

    AppPrintString("\n\rperiodicAdvInterval = ");AppPrintDec((uint32_t)pExtScannedDevice->periodicAdvInterval);

    AppPrintString(maAdvEvtDataTypeStrings[((pExtScannedDevice->advEventProperties & ((1U)<<3))>>3)]);
    dataLength = 0;
    pData = pExtScannedDevice->pData;
    while( dataLength < pExtScannedDevice->dataLength )
    {
        dataLength += ((uint16_t)*pData + 1U);
        if(pData[1] == (uint8_t)gAdManufacturerSpecificData_c)
        {
            AppPrintString((char *)&pData[2]);
        }
        pData =  &pExtScannedDevice->pData[dataLength];
    }
}
/*! *********************************************************************************
* \brief        Handles Extended Advertising Event Management
*
* \param[in]    pExtScannedDevice                pointer to gapExtScannedDevice_t structure provided by the scanning callback.
********************************************************************************** */
static void AppHandleExtAdvEvent( gapExtScannedDevice_t* pExtScannedDevice)
{
    uint8_t advIndex;
    bool_t advPresent = FALSE;
    bool_t advDataChanged = FALSE;
    bool_t handlePriodicAdv = FALSE;
    uint32_t advCRC = AppCalculateAdvDataCRC(pExtScannedDevice->pData, pExtScannedDevice->dataLength);
    if(mAppExtAdvListIndex != 0U)
    {
        uint8_t i;

        for( i=0 ; i<mAppExtAdvListIndex ; i++)
        {
            if(FLib_MemCmp (pExtScannedDevice->aAddress, maAppExtAdvList[i].aAddress, sizeof(bleDeviceAddress_t)))
            {
                if(pExtScannedDevice->SID == maAppExtAdvList[i].SID)
                {
                    advPresent = TRUE;
                    advIndex = i;
                    if(advCRC != maAppExtAdvList[i].dataCRC )
                    {
                        advDataChanged = TRUE;
                    }
                    if(maAppExtAdvList[i].periodicAdvInterval != pExtScannedDevice->periodicAdvInterval)
                    {
                        handlePriodicAdv = TRUE;
                    }
                    break;

                }
                else
                {
                    continue;
                }

            }
            else
            {
                continue;
            }

        }
    }
    if(advPresent == FALSE)
    {

        if(mAppExtAdvListIndex < mAppExtAdvListSize_c)
        {
            FLib_MemCpy (maAppExtAdvList[mAppExtAdvListIndex].aAddress, pExtScannedDevice->aAddress, sizeof(bleDeviceAddress_t));
            maAppExtAdvList[mAppExtAdvListIndex].SID = pExtScannedDevice->SID;
            maAppExtAdvList[mAppExtAdvListIndex].dataCRC = advCRC;
            maAppExtAdvList[mAppExtAdvListIndex].periodicAdvInterval = 0;
            maAppExtAdvList[mAppExtAdvListIndex].syncHandle = mBlePeriodicAdvInvalidSyncHandle_c;
            advIndex = mAppExtAdvListIndex++ ;
            if(pExtScannedDevice->periodicAdvInterval != 0U)
            {
                handlePriodicAdv = TRUE;
            }
        }
        else
        {
            advIndex =   mAppExtAdvListSize_c;
        }
    }
    if( (advPresent == FALSE) || (advDataChanged == TRUE))
    {

        if(advIndex < mAppExtAdvListSize_c)
        {
            maAppExtAdvList[advIndex].dataCRC = advCRC;
        }
        AppPrintExtAdvEvent(pExtScannedDevice);

    }

    if((handlePriodicAdv == TRUE) && (advIndex < mAppExtAdvListSize_c))
    {
        BleApp_HandlePeriodicAdv(pExtScannedDevice, advIndex);
    }
}

/*! *********************************************************************************
* \brief        Handles gPeriodicDeviceScanned_c event in the scanning callback
*
* \param[in]    pGapPeriodicScannedDevice                pointer to the gapPeriodicScannedDevice_t structure provided by the scanning callback.
********************************************************************************** */
static void AppHandlePeriodicDeviceScanEvent( gapPeriodicScannedDevice_t* pGapPeriodicScannedDevice)
{

    uint16_t  dataLength;
    uint8_t* pData;
    bool_t perAdvPresent = FALSE;
    bool_t perAdvDataChanged = FALSE;
    uint8_t advIndex;
    uint32_t advCRC = AppCalculateAdvDataCRC(pGapPeriodicScannedDevice->pData, pGapPeriodicScannedDevice->dataLength);
    if(mAppExtAdvListIndex != 0U)
    {
        for( advIndex=0 ; advIndex < mAppExtAdvListIndex ; advIndex++)
        {
            if(maAppExtAdvList[advIndex].syncHandle == pGapPeriodicScannedDevice->syncHandle)
            {
                perAdvPresent = TRUE;
                if(maAppExtAdvList[advIndex].perDataCRC != advCRC)
                {
                    perAdvDataChanged = TRUE;
                    maAppExtAdvList[advIndex].perDataCRC = advCRC;
                }
                break;
            }
        }
    }

    if((perAdvPresent == TRUE) && (perAdvDataChanged == TRUE))
    {
        AppPrintString("\r\nPeriodic Adv Found");
        AppPrintString("\n\rsyncHandle = ");AppPrintDec((uint32_t)pGapPeriodicScannedDevice->syncHandle);
        AppPrintString("\n\rPeriodic Data");
        dataLength = 0;
        pData = pGapPeriodicScannedDevice->pData;
        while ( dataLength < pGapPeriodicScannedDevice->dataLength )
        {
            dataLength += ((uint16_t)*pData + 1U);
            if(pData[1] == (uint8_t)gAdManufacturerSpecificData_c)
            {
                AppPrintString((char *)&pData[2]);
            }
            pData =  &pGapPeriodicScannedDevice->pData[dataLength];
        }
    }
}
/*! *********************************************************************************
* \brief        Stops all Periodic Adv Syncs created during the scanning callback.If there are more than one started, this function
*               is called again on gPeriodicAdvSyncTerminated_c event in scanning callback until all syncs are terminated.
*
* \param[in]    none.
********************************************************************************** */
void AppTerminatePeriodicAdvSync(void)
{
    uint8_t i;
    if( mPerExtAdvIndexPending == mPeriodicExtAdvInvalidIndex_c)
    {
        for( i=0 ; i<mAppExtAdvListIndex ; i++)
        {
            if(maAppExtAdvList[i].syncHandle != mBlePeriodicAdvInvalidSyncHandle_c)
            {
                if(gBleSuccess_c == Gap_PeriodicAdvTerminateSync(maAppExtAdvList[i].syncHandle))
                {
                    mPerExtAdvIndexPending = i;
                }
            }
        }
    }
}

static void BleApp_SerialInit(void)
{
    serial_manager_status_t status;

    /* UI */
    gAppSerMgrIf = (serial_handle_t)&gSerMgrIf[0];

    /*open wireless uart read/write handle*/
    status = SerialManager_OpenWriteHandle((serial_handle_t)gAppSerMgrIf, (serial_write_handle_t)s_writeHandle);
    assert(kStatus_SerialManager_Success == status);
    (void)status;

    status = SerialManager_OpenReadHandle((serial_handle_t)gAppSerMgrIf, (serial_read_handle_t)s_readHandle);
    assert(kStatus_SerialManager_Success == status);
}

static void BleApp_HandleScanStateChanged(void)
{
    mScanningOn = !mScanningOn;
    /* Node starts scanning */
    if (mScanningOn)
    {
        mFoundDeviceToConnect = FALSE;
        AppPrintString("\r\n");
        AppPrintString(maScanStrings[gScanParams.type]);
        AppPrintString(" Scanning Started\r\n");
        /* Start scanning timer */
        (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimeoutTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer | (uint8_t)kTimerModeSingleShot, gScanningTime_c);

         Led1Flashing();
    }
    /* Node is not scanning */
    else
    {
        (void)TM_Stop((timer_handle_t)mAppTimerId);
        Led1Off();
        Led1On();
        /* Connect with the previously scanned peer device */
        if (mFoundDeviceToConnect)
        {
            (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
        }
        else
        {
            AppPrintString("\r\nStop Scanning\r\n");
        }
        AppTerminatePeriodicAdvSync();
    }
}

static void BleApp_HandleDeviceScanned(gapScanningEvent_t* pScanningEvent)
{
    bool_t AE_peripheral = CheckForAEPeripheralDevice(pScanningEvent->eventData.scannedDevice.data, pScanningEvent->eventData.scannedDevice.dataLength);
    if(AE_peripheral)
    {
        AppPrintString("\r\nFound AE peripheral device");
        AppPrintHexLe(pScanningEvent->eventData.scannedDevice.aAddress, 6);
        if( FALSE == mFoundDeviceToConnect )
        {
            /* Check if the scanned device implements the Temperature Custom Profile */
            mFoundDeviceToConnect = CheckScanEvent(pScanningEvent);
            if (mFoundDeviceToConnect)
            {
                /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress, pScanningEvent->eventData.scannedDevice.aAddress, sizeof(bleDeviceAddress_t));
                gConnReqParams.initiatingPHYs = (uint8_t)gLePhy1MFlag_c;
                (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
            }
        }
    }
}

static void BleApp_HandleExtDeviceScanned(gapScanningEvent_t* pScanningEvent)
{
    bool_t AE_peripheral = CheckForAEPeripheralDevice(pScanningEvent->eventData.extScannedDevice.pData, pScanningEvent->eventData.extScannedDevice.dataLength);
    if(AE_peripheral)
    {
#if mAE_CentralDebug_c
        AppPrintString("\r\nExtended Advertising Found");
#endif
        AppHandleExtAdvEvent(&pScanningEvent->eventData.extScannedDevice);
        if( FALSE == mFoundDeviceToConnect )
        {
            if((pScanningEvent->eventData.extScannedDevice.advEventProperties & (bleAdvReportEventProperties_t)gAdvEventConnectable_c) != 0U )
            {
                /* Check if the scanned device implements the Temperature Custom Profile */
                mFoundDeviceToConnect = CheckScanEvent(pScanningEvent);
                if (mFoundDeviceToConnect)
                {
                    /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.extScannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress, pScanningEvent->eventData.extScannedDevice.aAddress, sizeof(bleDeviceAddress_t));
                    gConnReqParams.initiatingPHYs = ((1U)<<(pScanningEvent->eventData.extScannedDevice.secondaryPHY - 1U)) | ((1U)<<(pScanningEvent->eventData.extScannedDevice.primaryPHY - 1U));
                    (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.extScannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
    }
}

static void BleApp_HandleIdleState(deviceId_t peerDeviceId, appEvent_t event)
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

static void BleApp_HandleExchangeMtuState(deviceId_t peerDeviceId, appEvent_t event)
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

static void BleApp_HandleServiceDiscState(deviceId_t peerDeviceId, appEvent_t event)
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
                mpCharProcBuffer->paValue = (uint8_t*)&mpCharProcBuffer[1];
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

static void BleApp_HandleReadDescriptorState(deviceId_t peerDeviceId, appEvent_t event)
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

static void BleApp_HandleRunningState(deviceId_t peerDeviceId, appEvent_t event)
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
        /* Start Wait For Data timer */
        (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mAppTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer | (uint8_t)kTimerModeSingleShot, gWaitForDataTime_c);
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

static void BleApp_HandlePeriodicAdv( gapExtScannedDevice_t* pExtScannedDevice, uint8_t advIndex)
{
    if(pExtScannedDevice->periodicAdvInterval != 0U)
    {
        if( mPerExtAdvIndexPending == mPeriodicExtAdvInvalidIndex_c)
        {
            gapPeriodicAdvSyncReq_t gapPeriodicAdvSyncReq = {0U};
            gapPeriodicAdvSyncReq.options.filterPolicy = (gapCreateSyncReqFilterPolicy_t)gUseCommandParameters_c;
            gapPeriodicAdvSyncReq.options.reportingEnabled = 0U;
            gapPeriodicAdvSyncReq.SID = pExtScannedDevice->SID;
            gapPeriodicAdvSyncReq.peerAddressType = pExtScannedDevice->addressType;
            FLib_MemCpy (gapPeriodicAdvSyncReq.peerAddress, pExtScannedDevice->aAddress, sizeof(bleDeviceAddress_t));
            gapPeriodicAdvSyncReq.skipCount = 0;
            uint32_t timeout = pExtScannedDevice->periodicAdvInterval;
            timeout = (timeout * 5U)>>2U;/* timeout in ms N*1.25*/
            timeout = timeout*5U;/* at 5 adv missed timeout should expire*/
            timeout = timeout/10U;/* timeout unit is 10ms*/
            gapPeriodicAdvSyncReq.timeout = (uint16_t)timeout;
            AppPrintString("\n\rGap_PeriodicAdvCreateSync ");
            if(gBleSuccess_c == Gap_PeriodicAdvCreateSync( &gapPeriodicAdvSyncReq))
            {
                maAppExtAdvList[advIndex].syncHandle = gBlePeriodicAdvOngoingSyncCancelHandle;
                maAppExtAdvList[advIndex].periodicAdvInterval = pExtScannedDevice->periodicAdvInterval;
                mPerExtAdvIndexPending = advIndex;
                AppPrintString("Succeded");

            }
            else
            {
                AppPrintString("Failed");
            }
        }

    }
    else
    {
        if( mPerExtAdvIndexPending == mPeriodicExtAdvInvalidIndex_c)
        {
            if(mBlePeriodicAdvInvalidSyncHandle_c != maAppExtAdvList[advIndex].syncHandle )
            {
                AppPrintString("\n\rGap_PeriodicAdvTerminateSync ");
                if(gBleSuccess_c == Gap_PeriodicAdvTerminateSync(maAppExtAdvList[advIndex].syncHandle))
                {
                    maAppExtAdvList[advIndex].periodicAdvInterval = pExtScannedDevice->periodicAdvInterval;
                    mPerExtAdvIndexPending = advIndex;
                    AppPrintString("Succeded");

                }
                else
                {
                    AppPrintString("Failed");
                }
            }
        }
    }
}

#if defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE)
static void Uart_PrintMenu(void *pData)
{
    (void)pData;
    if (mDbafScanning == TRUE)
    {
        AppPrintString("\r\nPress WAKESW to Start DBAF Active Scanning!");
        AppPrintString("\r\nPress WAKESW Long to Start DBAF Passive Scanning!");
        AppPrintString("\r\nDouble press WAKESW to toggle scanning for Decision IND PDUs (DBAF)!");
    }
    else
    {
        AppPrintString("\r\nPress WAKESW to Start Active Scanning!");
        AppPrintString("\r\nPress WAKESW Long to Start Passive Scanning!");
        AppPrintString("\r\nDouble press WAKESW to toggle scanning for Decision IND PDUs (DBAF)!");
    }
    AppPrintString("\r\n");
}
#endif /* defined(gBLE60_DecisionBasedAdvertisingFilteringSupport_d) && (gBLE60_DecisionBasedAdvertisingFilteringSupport_d == TRUE) */
/*! *********************************************************************************
* @}
********************************************************************************** */
