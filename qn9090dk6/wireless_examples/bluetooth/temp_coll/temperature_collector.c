/*! *********************************************************************************
* \addtogroup Temperature Collector
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
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
#include "Keyboard.h"
#include "LED.h"
#include "Panic.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "shell.h"

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#include "PWR_Configuration.h"
#if defined (CPU_JN518X)
#include "pin_mux.h"
#endif
/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#else
#define UUID128(name, ...) uint8_t name[16] = { __VA_ARGS__ };
#include "gatt_uuid128.h"
#undef UUID128
#endif


/* Profile / Services */
#include "temperature_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "ApplMain.h"
#include "temperature_collector.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
#include "erpc_host.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

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

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
static bool_t mRestoringBondedLink = FALSE;
#if gAppUsePrivacy_d
static bool_t mAttemptRpaResolvingAtConnect = FALSE;
#endif
#endif

static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;

/* Timers */
static tmrTimerID_t mAppTimerId;
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

static void BleApp_Config(void);

void BleApp_StateMachineHandler
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

#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit);
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);

#define GET_DEEP_DOWN_MODE()    ((PMC->AOREG2 & (uint32_t)BIT(31)) != 0)
#define SET_DEEP_DOWN_MODE()    PMC->AOREG2 |= ((uint32_t)BIT(31))
#define CLR_DEEP_DOWN_MODE()    PMC->AOREG2 &= ~((uint32_t)BIT(31))

#endif
#endif

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void DisconnectTimerCallback(void* pParam);
#endif


/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BleApp_Init(void)
{
#if defined CPU_JN518X
    BleAppDrv_Init(false);

  #if (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(BleAppDrv_InitCB);
    PWR_RegisterLowPowerEnterCallback(BleAppDrv_DeInit);
  #endif
#else
    /* Init shell and set prompt */
    shell_init("BLE Temp Collector>");

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    /* Init eRPC host */
    init_erpc_host();
#endif
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    APP_DBG_LOG(""); 
    if (!mScanningOn)
    {
#if !defined (CPU_JN518X)        
        /* Set low power mode */
  #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
      #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
        #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
            (void)PWR_ChangeBlackBoxDeepSleepMode(gAppDeepSleepMode_c);
        #endif
      #else
        (void)PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
      #endif
  #endif
#else
        (void)PWR_ChangeDeepSleepMode(cPWR_ClockGating);  /* while Scanning it is not efficient to go to Deep Sleep */
        PWR_AllowDeviceToSleep();
#endif
        /* Start scanning */
        (void)App_StartScanning(&gScanParams, BleApp_ScanningCallback, gGapDuplicateFilteringEnable_c, gGapScanContinuously_d, gGapScanPeriodicDisabled_d);
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    APP_DBG_LOG("Ev=%x", events); 

    switch (events)
    {
        /* Start on button press if low-power is disabled */
        case gKBD_EventPressPB1_c:
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

        case gKBD_EventPressPB2_c:  /* Fall-through */
        {
#if gLoggingActive_d
        //DbgLogDump(true);
#endif
        break;
        }
        case gKBD_EventLongPB2_c:   /* Fall-through */
        default:
        {
            ; /* No action required */
            break;
        }
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    APP_DBG_LOG("Ev=%x", pGenericEvent->eventType); 

    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
#if cPWR_FullPowerDownMode && cPWR_EnableDeepSleepMode_4
#if defined (CPU_JN518X)
            if (GET_DEEP_DOWN_MODE())
            {
                BleApp_Start();
            }
#endif
#endif
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit)
{
    const char * temp_coll_prompt = "BLE Temp Collector>";
    APP_DBG_LOG("");
#if (cPWR_FullPowerDownMode)
    if ( reinit )
    {
        shell_reinit();
#if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
#endif
#if gLEDSupported_d
        LED_PrepareExitLowPower();
#endif
    }
    else
#endif
    {
        (void) reinit;
        /* Init shell and set prompt */
#if cPWR_FullPowerDownMode && cPWR_EnableDeepSleepMode_4
        if (GET_DEEP_DOWN_MODE())
        {
            shell_init("");  /* void prompt string to prevent any output here */
            shell_change_prompt((char*)temp_coll_prompt, true);
        }
        else
#endif
        {
            shell_init((char*)temp_coll_prompt);        /* UI */
        }
    }
}

#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void)
{
    APP_DBG_LOG("");
    BleAppDrv_Init(true);
}

static void BleAppDrv_DeInit(void)
{
    APP_DBG_LOG("");
    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
}
#endif  /*cPWR_UsePowerDownMode */
#endif  /* CPU_JN518X*/



/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
    APP_DBG_LOG(""); 

    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    /* Set Tx power level */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    /* Initialize private variables */
    mPeerInformation.appState = mAppIdle_c;
    mScanningOn = FALSE;
    mFoundDeviceToConnect = FALSE;

    /* Allocate scan timeout timer */
    mAppTimerId = TMR_AllocateTimer();
#if cPWR_FullPowerDownMode && cPWR_EnableDeepSleepMode_4
#if defined (CPU_JN518X)
    /* Initla reset, not woken form DeepDown mode */
    if (!(GET_DEEP_DOWN_MODE()))
#endif
#endif
    {
        /* Update UI */
        shell_write("\r\nPress SCANSW to connect to a Temperature Sensor!\r\n");
    }
#if !defined (CPU_JN518X)
  #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    /* Allow entering sleep mode until any user interaction */
    #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
        #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
            (void)PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
            PWR_AllowBlackBoxToSleep();
        #endif
        (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
        PWR_AllowDeviceToSleep();
    #else
        (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
        PWR_AllowDeviceToSleep();
    #endif
  #endif
#else
        (void)PWR_ChangeDeepSleepMode(cPWR_ClockGating);
        PWR_AllowDeviceToSleep();
#endif
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
    APP_DBG_LOG("Ev=%x", pScanningEvent->eventType); 

#if gAppUsePrivacy_d && gAppUseBonding_d
    uint8_t bondedDevicesCnt = 0;
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

                shell_write("\r\nScanning...\r\n");

                /* Start scanning timer */
                (void)TMR_StartLowPowerTimer(mAppTimerId,
                           gTmrLowPowerSecondTimer_c,
                           TmrSeconds(gScanningTime_c),
                           ScanningTimeoutTimerCallback, NULL);
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                Led1On();
#else
                LED_StopFlashingAllLeds();
                Led1Flashing();
#endif
            }
            /* Node is not scanning */
            else
            {
                (void)TMR_StopTimer(mAppTimerId);

                /* Connect with the previously scanned peer device */
                if (mFoundDeviceToConnect)
                {
#if gAppUsePrivacy_d
                    if(gConnReqParams.peerAddressType == gBleAddrTypeRandom_c)
                    {
#if gAppUseBonding_d
                        /* Check if there are any bonded devices */
                        Gap_GetBondedDevicesCount(&bondedDevicesCnt);

                        if(bondedDevicesCnt == 0)
                        {
                            /* display the unresolved RPA address */
                            shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                        }
                        else
                        {
                            mAttemptRpaResolvingAtConnect = TRUE;
                        }
#else
                        /* If bonding is disabled and we receive an RPA address there is nothing to do but display it */
                        shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUseBonding_d */
                    }
                    else
                    {
                        /* display the public/resolved address */
                        shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
                    }
#else
                    /* Display the peer address */
                    shell_writeHexLe(gConnReqParams.peerAddress, gcBleDeviceAddressSize_c);
#endif /* gAppUsePrivacy_d */

                    (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
                else
                {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                    Led1Off();
  #if !defined (CPU_JN518X)
                    /* Go to sleep */
                    #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
                        #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
                            (void)PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
                        #endif
                    #else
                        (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
                    #endif
  #else
                        (void)PWR_ChangeDeepSleepMode(cPWR_ClockGating);

  #endif
#else
                    LED_StopFlashingAllLeds();
                    Led1Flashing();
                    Led2Flashing();
                    Led3Flashing();
                    Led4Flashing();
#endif
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
    APP_DBG_LOG("Ev=%x", pConnectionEvent->eventType); 
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
                shell_writeHexLe(pConnectionEvent->eventData.connectedEvent.peerAddress, gcBleDeviceAddressSize_c);
                /* clear the flag */
                mAttemptRpaResolvingAtConnect = FALSE;
            }
#endif
            /* Update UI */
            LED_StopFlashingAllLeds();
            Led1On();
            shell_write("\r\nConnected!\r\n");

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

            /* Set low power mode */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#if !defined (CPU_JN518X)
            #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
                #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
                    (void)PWR_ChangeBlackBoxDeepSleepMode(gAppDeepSleepMode_c);
                    PWR_AllowBlackBoxToSleep();
                #endif
            #else
                (void)PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
                PWR_AllowDeviceToSleep();
            #endif
#else
                (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
                PWR_AllowDeviceToSleep();
#endif
#endif

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            (void)Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL);

            if ((mPeerInformation.isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.customInfo, 0, sizeof (appCustomInfo_t))))
            {
                mRestoringBondedLink = TRUE;
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
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

            /* Update UI */
            shell_write("\r\nDisconnected!\r\n");

#if !defined (CPU_JN518X)
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
            /* Go to sleep */
            #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
                #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
                    (void)PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
                #endif
            #else
                (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
            #endif
            Led1Off();
#else
            LED_TurnOffAllLeds();
            LED_StartFlash(LED_ALL);
#endif
#else
#if cPWR_FullPowerDownMode && cPWR_EnableDeepSleepMode_4
            /* Allow PD1/DeepSleep : on next PB1 key press scanning will resume*/
            SET_DEEP_DOWN_MODE();
            shell_write("\r\nDeep Sleeping - Press SCANSW to resume scanning\r\n");

            PWR_ChangeDeepSleepMode(cPWR_DeepSleep_RamOffOsc32kOff);
            Led1Off();
            PWR_AllowDeviceToSleep();
            PWR_PreventEnterLowPower(false);
#else
            (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);

            LED_TurnOffAllLeds();
            LED_StartFlash(LED_ALL);   
#endif
#endif
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            /* Notify state machine handler on pairing complete */
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PairingComplete_c);
            }
        }
        break;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                if( mRestoringBondedLink )
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
    APP_DBG_LOG("Ev=%x", pEvent->eventType);

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
    APP_DBG_LOG(""); 

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
    APP_DBG_LOG(""); 

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
    APP_DBG_LOG(""); 
    shell_write("Temperature: ");
    shell_writeDec((uint32_t)temperature / 100UL);

    /* Add 'C' for Celsius degrees - UUID 0x272F.
       www.bluetooth.com/specifications/assigned-numbers/units */
    if (mPeerInformation.customInfo.tempClientConfig.tempFormat.unitUuid16 == 0x272FU)
    {
        shell_write(" C\r\n");
    }
    else
    {
        shell_write("\r\n");
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
    APP_DBG_LOG(""); 
    if (procedureResult == gGattProcError_c)
    {
        attErrorCode_t attError = (attErrorCode_t)(uint8_t)(error);

        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
            /* Start Pairing Procedure */
            (void)Gap_Pair(serverDeviceId, &gPairingParameters);
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
    APP_DBG_LOG(""); 
    if (characteristicValueHandle == mPeerInformation.customInfo.tempClientConfig.hTemperature)
    {
        BleApp_PrintTemperature(Utils_ExtractTwoByteValue(aValue));

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        /* Restart Wait For Data timer */
        (void)TMR_StartLowPowerTimer(mAppTimerId,
                       gTmrLowPowerSecondTimer_c,
                       TmrSeconds(gWaitForDataTime_c),
                       DisconnectTimerCallback, NULL);
#endif
    }
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
    APP_DBG_LOG(""); 

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
    uint8_t name[10];
    uint8_t nameLength = 0;
    bool_t foundMatch = FALSE;
    APP_DBG_LOG(""); 

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
        shell_write("\r\nFound device: \r\n");
        shell_writeN((char*)name, (uint16_t)nameLength - 1U);
        SHELL_NEWLINE();
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
    APP_DBG_LOG(""); 
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
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
        break;

        case mAppExchangeMtu_c:
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
        break;

        case mAppServiceDisc_c:
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
                        mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
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
        break;

        case mAppReadDescriptor_c:
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
        break;

        case mAppRunning_c:
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
                                                    (void *)&mPeerInformation.customInfo, 0,
                                                    sizeof(appCustomInfo_t));
#endif
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
                /* Start Wait For Data timer */
                (void)TMR_StartLowPowerTimer(mAppTimerId,
                               gTmrLowPowerSecondTimer_c,
                               TmrSeconds(gWaitForDataTime_c),
                               DisconnectTimerCallback, NULL);
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
    APP_DBG_LOG(""); 

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
    APP_DBG_LOG(""); 
    /* Stop scanning */
    if (mScanningOn)
    {
        (void)Gap_StopScanning();
    }
}

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
/*! *********************************************************************************
* \brief        Disconnect from peer device if no data was received for a given time.
                (gWaitForDataTime_c). Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    APP_DBG_LOG(""); 
    /* Disconnect from peer device */
    if (mPeerInformation.deviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(mPeerInformation.deviceId);
    }
}
#endif  /* cPWR_UsePowerDownMode */
/*! *********************************************************************************
* @}
********************************************************************************** */
