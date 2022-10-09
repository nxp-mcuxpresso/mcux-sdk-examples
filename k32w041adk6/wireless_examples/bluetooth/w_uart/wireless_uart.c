/*! *********************************************************************************
 * \addtogroup Wireless UART Application
 * @{
 ********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Wireless UART Application
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
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"
#include "SerialManager.h"
#include "MemManager.h"
#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#endif

#include "board.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#else
#define UUID128(name, ...)\
    extern uint8_t name[16];
#include "gatt_uuid128.h"
#undef UUID128
#endif

/* Profile / Services */
#include "wireless_uart_interface.h"
#include "battery_interface.h"
/* Wrappers */
#include "ble_conn_manager.h"
#include "ble_service_discovery.h"
/* define gWURolePeripheral_d if you wish to start directly in peripheral 
 *  mode without button press.
 */
//#define gWURolePeripheral_d
#define APP_DBG_LVL DBG_LEVEL_WARNING

/* Concatenate macros */
#ifndef QU
#define QU(x) #x
#endif
#ifndef QUH
#define QUH(x) QU(x)
#endif

#include "ApplMain.h"
#include "wireless_uart.h"

#ifdef CPU_JN518X
#include "fsl_xcvr.h"
#endif

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    #include "erpc_host.h"
    #include "dynamic_gatt_database.h"
#endif

#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#endif
#if (cPWR_FullPowerDownMode)
#include "fsl_xcvr.h"
#endif

/* Check the GAP configuration. The device is either a GAP Central or a GAP Peripheral */
#if (gWuart_CentralRole_c + gWuart_PeripheralRole_c == 0)
#error "The device GAP role is misconfigured."
#endif

/************************************************************************************
 *************************************************************************************
 * Private macros
 *************************************************************************************
 ************************************************************************************/
#ifndef gAppBaudRate_c
#define gAppBaudRate_c ((uint32_t)gUARTBaudRate115200_c)
#endif
#define mAppUartBufferSize_c            gAttMaxWriteDataSize_d(gAttMaxMtu_c) /* Local Buffer Size */

#if (cPWR_FullPowerDownMode)
#define mAppUartFlushIntervalInMs_c       (50)     /* Flush Rx data to Air interface less often (in Ms) */
#else
#define mAppUartFlushIntervalInMs_c       (7)     /* Flush Timeout in Ms */
#endif

#define mBatteryLevelReportInterval_c   (10)    /* battery level report interval in seconds  */

#define mTemperatureReportInterval_c    (10)    /* temperature measurement interval in seconds */

/************************************************************************************
 *************************************************************************************
 * Private type definitions
 *************************************************************************************
 ************************************************************************************/
typedef enum appEvent_tag
{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryNotFound_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
} appEvent_t;

typedef enum appState_tag
{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppServiceDiscRetry_c,
    mAppRunning_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t  deviceId;
    bool_t      isBonded;
    wucConfig_t clientInfo;
    appState_t  appState;
    gapRole_t   gapRole;
} appPeerInfo_t;

typedef struct advState_tag
{
    bool_t advOn;
} advState_t;

/*! Bitfield for Adc measurements */
typedef enum {
    gAdcMeasureNone_c                   = 0,
    gAdcMeasureTemperature_c            = BIT0,  /* Temperature measurement to do */
    gAdcMeasureBatteryLevelFirst_c      = BIT1,  /* First battery level measurement for BLE config */
    gAdcMeasureBatteryLevel_c           = BIT2,  /* Battery level measurement */
} gAdcMeasure_t;

/************************************************************************************
 *************************************************************************************
 * Private memory declarations
 *************************************************************************************
 ************************************************************************************/

static appPeerInfo_t maPeerInformation[gAppMaxConnections_c];
#ifdef gWURolePeripheral_d
static gapRole_t     mGapRole = gGapPeripheral_c;
#else
static gapRole_t     mGapRole;
#endif

/* Adv Parameters */
#if gWuart_PeripheralRole_c == 1
static advState_t mAdvState;
#endif
#if gWuart_CentralRole_c == 1
static bool_t   mScanningOn = FALSE;
#endif

static int mConnectionNb = 0;

static uint16_t mCharMonitoredHandles[1] = { (uint16_t)value_uart_stream };

/* Service Data*/
static wusConfig_t mWuServiceConfig;
static bool_t      mBasValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t mBasServiceConfig = {(uint16_t)service_battery, 0, mBasValidClientList, gAppMaxConnections_c};

static tmrTimerID_t mAppTimerId = gTmrInvalidTimerID_c;
#if (cPWR_FullPowerDownMode==0) || gPWR_SerialUartRxWakeup
static tmrTimerID_t mUartStreamFlushTimerId = gTmrInvalidTimerID_c;
#endif
#if (cPWR_FullPowerDownMode==0)
static tmrTimerID_t mBatteryMeasurementTimerId = gTmrInvalidTimerID_c;
static tmrTimerID_t mTemperatureMeasurementTimerId = gTmrInvalidTimerID_c;
#endif

/* If the board has only one button, multiplex the required functionalities on it using an application timer */
#if (gKBD_KeysCount_c == 1)
static tmrTimerID_t mSwitchPressTimerId = gTmrInvalidTimerID_c;
#endif

static uint8_t gAppSerMgrIf;
static uint16_t mAppUartBufferSize = mAppUartBufferSize_c;
static volatile bool_t mAppUartNewLine = FALSE;
static volatile bool_t mAppDapaPending = FALSE;

#if (gKBD_KeysCount_c == 1)
static uint8_t mSwitchPressCnt = 0;
#endif

static uint32_t mAdcMeasureToDo = 0;
#if (cPWR_FullPowerDownMode )
static uint32_t mAppTemperatureTime = 0; /* time of last temperature measurement */
static uint32_t mAppBatteryLevelTime = 0; /* time of last battery level measurement */
#endif

/************************************************************************************
 *************************************************************************************
 * Private functions prototypes
 *************************************************************************************
 ************************************************************************************/

/* Gatt and Att callbacks */
#if gWuart_PeripheralRole_c == 1
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent);
#endif

#if gWuart_CentralRole_c == 1
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent);
#endif

static void BleApp_ConnectionCallback
(
    deviceId_t peerDeviceId,
    gapConnectionEvent_t *pConnectionEvent
);
static void BleApp_GattServerCallback
(
    deviceId_t deviceId,
    gattServerEvent_t *pServerEvent
);

static void BleApp_GattClientCallback
(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
);

static void BleApp_ServiceDiscoveryCallback
(
    deviceId_t      peerDeviceId,
    servDiscEvent_t *pEvent
);

static void BleApp_Config(void);

#if gWuart_PeripheralRole_c == 1
static void BleApp_Advertise(void);
#endif

void BleApp_StateMachineHandler
(
    deviceId_t peerDeviceId,
    appEvent_t event
);

static void BleApp_StoreServiceHandles
(
    deviceId_t       peerDeviceId,
    gattService_t   *pService
);

#if gWuart_CentralRole_c == 1
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t *pData);
#endif

/* Timer Callbacks */

#if gWuart_CentralRole_c == 1
static void ScanningTimerCallback(void *pParam);
#endif

#if (cPWR_FullPowerDownMode==0) || gPWR_SerialUartRxWakeup
static void UartStreamFlushTimerCallback(void *pData);
#endif

#if (cPWR_FullPowerDownMode)
static void AdcMeasurementCallback(void *pParam);
static void AdcMeasurement_TimeCheck(uint32_t measureTodo, uint32_t* lastMeasureTime, uint32_t measureInterval);
#else
static void BatteryMeasurementTimerCallback(void *pParam);
static void TemperatureMeasurementTimerCallback(void *pParam);
#endif

#if (gKBD_KeysCount_c == 1)
static void SwitchPressTimerCallback(void *pParam);
#endif

/* Uart Tx/Rx Callbacks*/
static void Uart_RxCallBack(void *pData);
static void Uart_TxCallBack(void *pBuffer);

static void BleApp_FlushUartStream(void *pParam);
static void BleApp_ReceivedUartStream(deviceId_t peerDeviceId, uint8_t *pStream, uint16_t streamLength);

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
static void BleApp_HandleControllerNotification(bleNotificationEvent_t *pNotificationEvent);

#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
static void BleApp_ControllerNotificationCallback(bleCtrlNotificationEvent_t *pNotificationEvent);
#endif
#endif
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit);
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);
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
    /* Initialize application support for drivers */
    BOARD_InitAdc();

    /* UI */
#if (defined(KW37A4_SERIES) || defined(KW37Z4_SERIES) || defined(KW38A4_SERIES) || defined(KW38Z4_SERIES) || defined(KW39A4_SERIES))
    Serial_InitManager();
#else
    SerialManager_Init();
#endif

    /* Register Serial Manager interface */
    (void)Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);

    (void)Serial_SetBaudRate(gAppSerMgrIf, gAppBaudRate_c);

    /* Install Controller Events Callback handler */
    (void)Serial_SetRxCallBack(gAppSerMgrIf, Uart_RxCallBack, NULL);

  #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    /* Init eRPC host */
    init_erpc_host();
  #endif
#endif
}
/*! *********************************************************************************
 * \brief    Starts the BLE application.
 *
 * \param[in]    gapRole    GAP Start Role (Central or Peripheral).
 ********************************************************************************** */
void BleApp_Start(gapRole_t gapRole)
{
    APP_DEBUG_TRACE("%s role=%x\r\n", __FUNCTION__, mGapRole);
    switch (gapRole)
    {
#if gWuart_CentralRole_c == 1
        case gGapCentral_c:
        {
            (void)Serial_Print(gAppSerMgrIf, "\n\rScanning...\n\r", gAllowToBlock_d);
            mAppUartNewLine = TRUE;
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            Gap_ControllerEnhancedNotification(gNotifScanEventOver_c | gNotifScanAdvPktRx_c |
                                               gNotifScanRspRx_c | gNotifScanReqTx_c | gNotifConnCreated_c, 0);
#endif
            gPairingParameters.localIoCapabilities = gIoKeyboardDisplay_c;
            (void)App_StartScanning(&gScanParams, BleApp_ScanningCallback, gGapDuplicateFilteringEnable_c, gGapScanContinuously_d, gGapScanPeriodicDisabled_d);
            break;
        }
#endif
#if gWuart_PeripheralRole_c == 1
        case gGapPeripheral_c:
        {
            (void)Serial_Print(gAppSerMgrIf, "\n\rAdvertising...\n\r", gAllowToBlock_d);
            mAppUartNewLine = TRUE;
#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            Gap_ControllerEnhancedNotification(gNotifAdvEventOver_c | gNotifAdvTx_c |
                                               gNotifAdvScanReqRx_c | gNotifAdvConnReqRx_c | gNotifConnCreated_c, 0);
#endif
            gPairingParameters.localIoCapabilities = gIoDisplayOnly_c;
            
            /* start ADV only if it is not already started! */
            if (!mAdvState.advOn)
            {
              BleApp_Advertise();
            }
            break;
        }
#endif
        default:
        {
            ; /* No action required */
            break;
        }
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    uint8_t mPeerId = 0;
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, events);
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
#if (gKBD_KeysCount_c == 1)
            /* increment the switch press counter */
            mSwitchPressCnt++;

            if(FALSE == TMR_IsTimerActive(mSwitchPressTimerId))
            {
                /* Start the switch press timer */
                (void)TMR_StartLowPowerTimer(mSwitchPressTimerId,
                                             gTmrLowPowerSingleShotMillisTimer_c,
                                             gSwitchPressTimeout_c,
                                             SwitchPressTimerCallback, NULL);
            }
#else
            LED_StopFlashingAllLeds();
            Led1Flashing();
            BleApp_Start(mGapRole);
#endif
            break;
        }

        case gKBD_EventLongPB1_c:
        {
            for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (maPeerInformation[mPeerId].deviceId != gInvalidDeviceId_c)
                {
                    (void)Gap_Disconnect(maPeerInformation[mPeerId].deviceId);
                }
            }

            break;
        }

#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
        case gKBD_EventPressPB2_c:
        {
            /* Switch current role */
            if (mGapRole == gGapCentral_c)
            {
                (void)Serial_Print(gAppSerMgrIf, "\n\rSwitched role to GAP Peripheral.\n\r", gAllowToBlock_d);
                mAppUartNewLine = TRUE;
                if (mScanningOn)
                {
                    Gap_StopScanning();
                    mScanningOn = FALSE;
                }
                mGapRole = gGapPeripheral_c;
            }
            else
            {
                (void)Serial_Print(gAppSerMgrIf, "\n\rSwitched role to GAP Central.\n\r", gAllowToBlock_d);
                mAppUartNewLine = TRUE;
                if (mAdvState.advOn)
                {
                    Gap_StopAdvertising();
                    mAdvState.advOn = FALSE;
                }
                mGapRole = gGapCentral_c;
            }

            break;
        }
#endif
        case gKBD_EventLongPB2_c:
            break;

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
void BleApp_GenericCallback(gapGenericEvent_t *pGenericEvent)
{
    /* Call BLE Conn Manager */
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, pGenericEvent->eventType);
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            BleApp_Config();
        }
        break;

#if gWuart_PeripheralRole_c == 1
        case gAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;
#endif

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
        case gControllerNotificationEvent_c:
        {
            BleApp_HandleControllerNotification(&pGenericEvent->eventData.notifEvent);
        }
        break;
#endif

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

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit)
{
    APP_DEBUG_TRACE("%s \r\n", __FUNCTION__);
#if (cPWR_FullPowerDownMode)
    if ( reinit )
    {
        /* check if any ADC measurement need to be done  */
        AdcMeasurement_TimeCheck(gAdcMeasureTemperature_c, &mAppTemperatureTime, mTemperatureReportInterval_c);

        if(mConnectionNb > 0)
            AdcMeasurement_TimeCheck(gAdcMeasureBatteryLevel_c, &mAppBatteryLevelTime, mBatteryLevelReportInterval_c);

        SerialInterface_Reinit(gAppSerMgrIf);

        Serial_SetBaudRate(gAppSerMgrIf,  gAppBaudRate_c); /* might be 9600kbps just as well */

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
        /* UI */
        Serial_InitManager();

        /* Register Serial Manager interface */
        Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);

        Serial_SetBaudRate(gAppSerMgrIf, gAppBaudRate_c);

        /* Install Controller Events Callback handler */
        Serial_SetRxCallBack(gAppSerMgrIf, Uart_RxCallBack, NULL);
    }
}

#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void)
{
    BleAppDrv_Init(true);
}

static void BleAppDrv_DeInit(void)
{
    /* DeInitialize application support for drivers */
    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
}
#endif  /*cPWR_UsePowerDownMode */
#endif  /* CPU_JN518X*/

static void BleApp_Config(void)
{
    uint8_t mPeerId = 0;
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)

    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0, 0, 0, 0);
        return;
    }

#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks */
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(mCharMonitoredHandles), mCharMonitoredHandles);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        maPeerInformation[mPeerId].appState = mAppIdle_c;
        maPeerInformation[mPeerId].deviceId = gInvalidDeviceId_c;
        maPeerInformation[mPeerId].clientInfo.hService = gGattDbInvalidHandleIndex_d;
        maPeerInformation[mPeerId].clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;
    }

    /* By default, always start node as GAP central */
#if gWuart_CentralRole_c == 1
    mGapRole = gGapCentral_c;
#else
    mGapRole = gGapPeripheral_c;
#endif

#if (gKBD_KeysCount_c == 1)
    (void)Serial_Print(gAppSerMgrIf, "\n\rWireless UART starting as GAP Central.\n\r", gAllowToBlock_d);
    (void)Serial_Print(gAppSerMgrIf, "\n\rWithin one second, either:\n\r", gAllowToBlock_d);
    (void)Serial_Print(gAppSerMgrIf, " - double press the switch to change the role or\n\r", gAllowToBlock_d);
    (void)Serial_Print(gAppSerMgrIf, " - single press to start the application with the selected role.\n\r", gAllowToBlock_d);
#else
#if gWuart_CentralRole_c == 1
#define ModeString "Central"
#else
#define ModeString "Peripheral"
#endif
            
#define AppString "\n\rWireless UART" \
                        "\r\n\tLP=" QUH(cPWR_FullPowerDownMode)\
                        "\r\n\tPairing=" QUH(gAppUsePairing_d) \
                        "\r\n\tBonding=" QUH(gAppUseBonding_d) \
                        "\r\n\tPrivacy=" QUH(gAppUsePrivacy_d)

#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
    (void)Serial_Print(gAppSerMgrIf, AppString "\r\nStarting as GAP " ModeString", press the role switch to change it.\n\r", gAllowToBlock_d);
#else
    (void)Serial_Print(gAppSerMgrIf, AppString "\r\nStarting as GAP " ModeString".\n\r", gAllowToBlock_d);
#endif
#endif /* gKBD_KeysCount_c == 1 */

#if gWuart_PeripheralRole_c == 1
    mAdvState.advOn = FALSE;
#endif

    mConnectionNb = 0;

#if gWuart_CentralRole_c == 1
    mScanningOn = FALSE;
#endif
    /* Start services */
    (void)Wus_Start(&mWuServiceConfig);

    /* do battery level measurement */
    /* init ADC */
    BOARD_InitAdc();
    mAdcMeasureToDo |= gAdcMeasureBatteryLevelFirst_c;
#if (cPWR_FullPowerDownMode)
    App_PostCallbackMessage(AdcMeasurementCallback, NULL);
#else
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
    App_PostCallbackMessage(BatteryMeasurementTimerCallback, NULL);
    mTemperatureMeasurementTimerId = TMR_AllocateTimer();
    TMR_StartLowPowerTimer(mTemperatureMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                       TmrSeconds(mTemperatureReportInterval_c), TemperatureMeasurementTimerCallback, NULL);
#endif

    /* Allocate application timer */
    mAppTimerId = TMR_AllocateTimer();
#if (cPWR_FullPowerDownMode==0) || gPWR_SerialUartRxWakeup
    mUartStreamFlushTimerId = TMR_AllocateTimer();
#endif
#ifndef CPU_JN518X
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
#endif

#if (gKBD_KeysCount_c == 1)
    mSwitchPressTimerId = TMR_AllocateTimer();
#endif

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
    Controller_RegisterEnhancedEventCallback(BleApp_ControllerNotificationCallback);
#endif
#endif
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
#if gWuart_PeripheralRole_c == 1
static void BleApp_Advertise(void)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);
    BOARD_DbgDiagIoConf();
    BOARD_DbgDiagEnable();

#if (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
    PWR_AllowDeviceToSleep();
#endif
}
#endif

/*! *********************************************************************************
 * \brief        Handles BLE Scanning callback from host stack.
 *
 * \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
 ********************************************************************************** */
#if gWuart_CentralRole_c == 1
static void BleApp_ScanningCallback(gapScanningEvent_t *pScanningEvent)
{
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, pScanningEvent->eventType);
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
        {
            if (BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice))
            {
                gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                FLib_MemCpy(gConnReqParams.peerAddress,
                            pScanningEvent->eventData.scannedDevice.aAddress,
                            sizeof(bleDeviceAddress_t));

                (void)Gap_StopScanning();
#if gAppUsePrivacy_d
                gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                (void)App_Connect(&gConnReqParams, BleApp_ConnectionCallback);
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                /* Start advertising timer */
                (void)TMR_StartLowPowerTimer(mAppTimerId,
                                             gTmrLowPowerSecondTimer_c,
                                             TmrSeconds(gScanningTime_c),
                                             ScanningTimerCallback, NULL);

                Led1Flashing();
            }
            /* Node is not scanning */
            else
            {
                (void)TMR_StopTimer(mAppTimerId);

                Led1Flashing();
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            Serial_Print(gAppSerMgrIf, "\n\rScanning Command Failed.\n\r", gAllowToBlock_d);
            APP_WARNING_TRACE("Scanning Command Failed.\n\r");
            break;
        }
        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
#if gWuart_PeripheralRole_c == 1
static void BleApp_AdvertisingCallback(gapAdvertisingEvent_t *pAdvertisingEvent)
{
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, pAdvertisingEvent->eventType);
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
            LED_StopFlashingAllLeds();
            Led1Flashing();

            if (!mAdvState.advOn)
            {
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            Serial_Print(gAppSerMgrIf, "\n\rAdvertising Command Failed.\n\r", gAllowToBlock_d);
        	APP_WARNING_TRACE("Advertising Command Failed\r\n");
            //panic(0,0,0,0);
#if (cPWR_UsePowerDownMode)
            PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
            PWR_AllowDeviceToSleep();
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
#endif

/*! *********************************************************************************
* \brief        Handles BLE Connection callback from host stack.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pConnectionEvent    Pointer to gapConnectionEvent_t.
********************************************************************************** */
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t *pConnectionEvent)
{
    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Save peer device ID */
            maPeerInformation[peerDeviceId].deviceId = peerDeviceId;

            /* Advertising stops when connected */
#if gWuart_PeripheralRole_c == 1
            mAdvState.advOn = FALSE;
#endif

            /* Subscribe client*/
            (void)Wus_Subscribe(peerDeviceId);
            (void)Bas_Subscribe(&mBasServiceConfig, peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAppTimerId);
            mConnectionNb ++;
            APP_DEBUG_TRACE("mConnectionNb=%d\r\n", mConnectionNb);


            /* do periodic Battery level measurement */
#if (cPWR_FullPowerDownMode)
            AdcMeasurement_TimeCheck(gAdcMeasureBatteryLevel_c, &mAppBatteryLevelTime, mBatteryLevelReportInterval_c);
#else
            if(!TMR_IsTimerActive(mBatteryMeasurementTimerId))
            {
                /* Start battery measurements */
                (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                             TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
            }
#endif

#if gAppUsePairing_d
#if gAppUseBonding_d

            if (mGapRole == gGapCentral_c)
            {
                (void)Gap_CheckIfBonded(peerDeviceId, &maPeerInformation[peerDeviceId].isBonded, NULL);

                if ((maPeerInformation[peerDeviceId].isBonded) &&
                    (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                            (void *) &maPeerInformation[peerDeviceId].clientInfo, 0, sizeof(wucConfig_t))))
                {
                    /* Restored custom connection information. Encrypt link */
                    (void)Gap_EncryptLink(peerDeviceId);
                }
                else
                {
                    /* Pair after connect if not bonded - it is possible that a
                       Slave Security Request will not arrive. */
                    (void)Gap_Pair(peerDeviceId, &gPairingParameters);
                }
            }

#endif /* gAppUseBonding_d*/
#endif /* gAppUsePairing_d */

            (void)Serial_Print(gAppSerMgrIf, "Connected to device ", gAllowToBlock_d);
            (void)Serial_PrintDec(gAppSerMgrIf, peerDeviceId);

            if (mGapRole == gGapCentral_c)
            {
                (void)Serial_Print(gAppSerMgrIf, " as master.\n\r", gAllowToBlock_d);
            }
            else
            {
                (void)Serial_Print(gAppSerMgrIf, " as slave.\n\r", gAllowToBlock_d);
            }

            mAppUartNewLine = TRUE;

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
            Gap_ControllerEnhancedNotification(/*gNotifConnEventOver_c |*/ gNotifConnRxPdu_c, peerDeviceId);
#endif

            maPeerInformation[peerDeviceId].gapRole = mGapRole;

#if (cPWR_UsePowerDownMode)
           PWR_ChangeDeepSleepMode(cPWR_ClockGating);
           PWR_AllowDeviceToSleep();
#endif

#if gAppUsePairing_d
            /* In case of Pairing/bonding enabled wait the end of the pairing process */
#else
            /* run the state machine */
            BleApp_StateMachineHandler(peerDeviceId, mAppEvt_PeerConnected_c);
#endif
        }
        break;

        case gConnEvtDisconnected_c:
        {
            if (mConnectionNb > 0) mConnectionNb--;
            APP_DEBUG_TRACE("mConnectNb=%d\r\n", mConnectionNb);
            (void)Serial_Print(gAppSerMgrIf, "Disconnected from device ", gAllowToBlock_d);
            (void)Serial_PrintDec(gAppSerMgrIf, peerDeviceId);
            (void)Serial_Print(gAppSerMgrIf, ".\n\r", gAllowToBlock_d);

            maPeerInformation[peerDeviceId].appState = mAppIdle_c;
            maPeerInformation[peerDeviceId].clientInfo.hService = gGattDbInvalidHandleIndex_d;
            maPeerInformation[peerDeviceId].clientInfo.hUartStream = gGattDbInvalidHandleIndex_d;

            /* Unsubscribe client */
            (void)Wus_Unsubscribe();
            (void)Bas_Unsubscribe(&mBasServiceConfig, peerDeviceId);
#if (!cPWR_FullPowerDownMode)
            /* stop periodic Battery level measurement, required only when connected */
            (void)TMR_StopTimer(mBatteryMeasurementTimerId);
#endif

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            /* UI */
            LED_TurnOffAllLeds();
#if (!cPWR_UsePowerDownMode)
            LED_StartFlash(LED_ALL);
#endif

            /* mark device id as invalid */
            maPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;

            /* recalculate minimum of maximum MTU's of all connected devices */
            mAppUartBufferSize                       = mAppUartBufferSize_c;

            for (uint8_t mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
            {
                if (gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId)
                {
                    uint16_t tempMtu;

                    (void)Gatt_GetMtu(mPeerId, &tempMtu);
                    tempMtu = gAttMaxWriteDataSize_d(tempMtu);

                    if (tempMtu < mAppUartBufferSize)
                    {
                        mAppUartBufferSize = tempMtu;
                    }
                }
            }
#ifndef  gWURolePeripheral_d
            if (mGapRole == gGapPeripheral_c)
#endif
            {
                BleApp_Start(mGapRole);
            }
        }
        break;

#if gAppUsePairing_d
        case gConnEvtEncryptionChanged_c:
        {
            if (pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState)
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_PairingComplete_c);
            }
            else
            {
                APP_DEBUG_TRACE("gConnEvtEncryptionChanged_c newEncryptionState error\r\n");
            }
        }
        break;

        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_PairingComplete_c);
            }
            else
            {
                APP_DEBUG_TRACE("PAIRING FAILURE \n");
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            APP_DEBUG_TRACE("Authentication Rejected reason = 0x%x\r\n", pConnectionEvent->eventData.authenticationRejectedEvent.rejectReason);
            if (mGapRole == gGapCentral_c)
            {
                (void)Gap_Pair(peerDeviceId, &gPairingParameters);
            }
        }
        break;

#endif /* gAppUsePairing_d */

        default:
        {
            ; /* No action required */
        }
        break;
    }

    /* Connection Manager to handle Host Stack interactions */
    switch (maPeerInformation[peerDeviceId].gapRole)
    {
#if gWuart_CentralRole_c == 1
        case gGapCentral_c:
            BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);
            break;
#endif
#if gWuart_PeripheralRole_c == 1
        case gGapPeripheral_c:
            BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);
            break;
#endif

        default:
            ; /* No action required */
            break;
    }
}

static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t *pEvent)
{
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, pEvent->eventType);
    switch (pEvent->eventType)
    {
        case gServiceDiscovered_c:
        {
            if (pEvent->eventData.pService->uuidType == gBleUuidType128_c)
            {
                if (FLib_MemCmp((void *)&uuid_service_wireless_uart, (void *)&pEvent->eventData.pService->uuid, sizeof(bleUuid_t)))
                {
                    BleApp_StoreServiceHandles(peerDeviceId, pEvent->eventData.pService);
                }
            }
        }
        break;

        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                if (gGattDbInvalidHandleIndex_d != maPeerInformation[peerDeviceId].clientInfo.hService)
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryComplete_c);
                }
                else
                {
                    BleApp_StateMachineHandler(peerDeviceId,
                                               mAppEvt_ServiceDiscoveryNotFound_c);
                }
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId,
                                           mAppEvt_ServiceDiscoveryFailed_c);
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
    APP_DEBUG_TRACE("%s procRes=%x\r\n", __FUNCTION__, procedureResult);
    switch (procedureResult)
    {
        case gGattProcError_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
            break;

        case gGattProcSuccess_c:
            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
            break;

        default:
            ; /* No action required */
            break;
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);
}

/*! *********************************************************************************
 * \brief        Handles GATT server callback from host stack.
 *
 * \param[in]    deviceId        Client peer device ID.
 * \param[in]    pServerEvent    Pointer to gattServerEvent_t.
 ********************************************************************************** */
static void BleApp_GattServerCallback(
    deviceId_t deviceId,
    gattServerEvent_t *pServerEvent)
{
    uint16_t tempMtu = 0;
    APP_DEBUG_TRACE("%s Evt=%x\r\n", __FUNCTION__, pServerEvent);
    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            if (pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_uart_stream)
            {
                BleApp_ReceivedUartStream(deviceId, pServerEvent->eventData.attributeWrittenEvent.aValue,
                                          pServerEvent->eventData.attributeWrittenEvent.cValueLength);
            }

            break;
        }

        case gEvtMtuChanged_c:
        {
            /* update stream length with minimum of  new MTU */
            (void)Gatt_GetMtu(deviceId, &tempMtu);
            tempMtu = gAttMaxWriteDataSize_d(tempMtu);

            mAppUartBufferSize = mAppUartBufferSize <= tempMtu ? mAppUartBufferSize : tempMtu;
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

#if gWuart_CentralRole_c == 1
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement,
                                        void *pData,
                                        uint8_t iDataLen)
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
#endif

/*! *********************************************************************************
 * \brief        Checks Scan data for a device to connect.
 *
 * \param[in]    pData    Pointer to gapScannedDevice_t.
 ********************************************************************************** */
#if gWuart_CentralRole_c == 1
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t *pData)
{
    uint8_t index = 0;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t) pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

        /* Search for Wireless UART Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c)
            || (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = MatchDataInAdvElementList(&adElement,
                                                   &uuid_service_wireless_uart, 16);
        }

        /* Move on to the next AD element type */
        index += adElement.length + sizeof(uint8_t);
    }

    return foundMatch;
}
#endif

/*! *********************************************************************************
 * \brief        Stores handles used by the application.
 *
 * \param[in]    pService    Pointer to gattService_t.
 ********************************************************************************** */
static void BleApp_StoreServiceHandles(deviceId_t peerDeviceId, gattService_t *pService)
{
    /* Found Wireless UART Service */
    maPeerInformation[peerDeviceId].clientInfo.hService = pService->startHandle;

    if (pService->cNumCharacteristics > 0U &&
        pService->aCharacteristics != NULL)
    {
        /* Found Uart Characteristic */
        maPeerInformation[peerDeviceId].clientInfo.hUartStream =
            pService->aCharacteristics[0].value.handle;
    }
}

static void BleApp_SendUartStream(uint8_t *pRecvStream, uint8_t streamSize)
{
    gattCharacteristic_t characteristic = {gGattCharPropNone_c, {0}, 0, 0};
    uint8_t              mPeerId = 0;

    /* send UART stream to all peers */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if (gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId &&
            mAppRunning_c == maPeerInformation[mPeerId].appState)
        {
            SERIAL_DBG_LOG("peerId=%d streamSz=%d", mPeerId, streamSize);
            characteristic.value.handle = maPeerInformation[mPeerId].clientInfo.hUartStream;
            (void)GattClient_WriteCharacteristicValue(mPeerId, &characteristic,
                    streamSize, pRecvStream, TRUE,
                    FALSE, FALSE, NULL);
        }
    }
}

void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    uint16_t tempMtu = 0;
    union
    {
        uint8_t     *pUuidArray;
        bleUuid_t   *pUuidObj;
    } temp; /* MISRA rule 11.3 */

    temp.pUuidArray = uuid_service_wireless_uart;

    /* invalid client information */
    if (gInvalidDeviceId_c == maPeerInformation[peerDeviceId].deviceId)
    {
        return;
    }

    switch (maPeerInformation[peerDeviceId].appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c || event == mAppEvt_PairingComplete_c)
            {
                /* Let the central device initiate the Exchange MTU procedure*/
#ifndef gWURolePeripheral_d
                if (mGapRole == gGapCentral_c)
                {
                    /* Moving to Exchange MTU State */
                    maPeerInformation[peerDeviceId].appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                }
                else
#endif
                {
                    /* Moving to Service Discovery State*/
                    maPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                    /* Start Service Discovery*/
                    (void)BleServDisc_FindService(peerDeviceId,
                                                  gBleUuidType128_c,
                                                  temp.pUuidObj);
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* update stream length with minimum of maximum MTU's of connected devices */
                (void)Gatt_GetMtu(peerDeviceId, &tempMtu);
                tempMtu = gAttMaxWriteDataSize_d(tempMtu);

                mAppUartBufferSize = mAppUartBufferSize <= tempMtu ? mAppUartBufferSize : tempMtu;

                /* Moving to Service Discovery State*/
                maPeerInformation[peerDeviceId].appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                (void)BleServDisc_FindService(peerDeviceId,
                                              gBleUuidType128_c,
                                              temp.pUuidObj);
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
                /* Moving to Running State*/
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
#if gAppUseBonding_d
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(maPeerInformation[peerDeviceId].deviceId,
                                                    (void *) &maPeerInformation[peerDeviceId].clientInfo, 0,
                                                    sizeof(wucConfig_t));
#endif
            }
            else if (event == mAppEvt_ServiceDiscoveryNotFound_c)
            {
                /* Moving to Service discovery Retry State*/
                maPeerInformation[peerDeviceId].appState = mAppServiceDiscRetry_c;
                /* Restart Service Discovery for all services */
                (void)BleServDisc_Start(peerDeviceId);
            }
            else if (event == mAppEvt_ServiceDiscoveryFailed_c)
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppServiceDiscRetry_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Running State*/
                maPeerInformation[peerDeviceId].appState = mAppRunning_c;
            }
            else if ((event == mAppEvt_ServiceDiscoveryNotFound_c) ||
                     (event == mAppEvt_ServiceDiscoveryFailed_c))
            {
                (void)Gap_Disconnect(peerDeviceId);
            }
            else
            {
                /* ignore other event types */
            }
        }
        break;

        case mAppRunning_c:
            break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
 * \brief        Handles scanning timer callback.
 *
 * \param[in]    pParam        Callback parameters.
 ********************************************************************************** */
#if gWuart_CentralRole_c == 1
static void ScanningTimerCallback(void *pParam)
{
    APP_DEBUG_TRACE("%s\r\n", __FUNCTION__);
    /* Stop scanning */
    (void)Gap_StopScanning();
}
#endif

/*! *********************************************************************************
 * \brief        Handles the switch press timer callback.
 *
 * \param[in]    pParam        Callback parameters.
 ********************************************************************************** */
#if (gKBD_KeysCount_c == 1)
static void SwitchPressTimerCallback(void *pParam)
{
#if (gWuart_CentralRole_c == 1) && (gWuart_PeripheralRole_c == 1)
  if(mSwitchPressCnt >= gSwitchPressThreshold_c)
  {
      /* Switch the current role */
      if (mGapRole == gGapCentral_c)
      {
          (void)Serial_Print(gAppSerMgrIf, "\n\rSwitched role to GAP Peripheral.\n\r", gAllowToBlock_d);
          mAppUartNewLine = TRUE;
          mGapRole = gGapPeripheral_c;
      }
      else
      {
          (void)Serial_Print(gAppSerMgrIf, "\n\rSwitched role to GAP Central.\n\r", gAllowToBlock_d);
          mAppUartNewLine = TRUE;
          mGapRole = gGapCentral_c;
      }
  }
  else
  {
      /* start the application using the selected role */
      LED_StopFlashingAllLeds();
      Led1Flashing();
      BleApp_Start(mGapRole);
  }
  /* reset the switch press counter */
  mSwitchPressCnt = 0;
#endif
}
#endif

static void BleApp_FlushUartStream(void *pParam)
{
    uint8_t  mPeerId = 0;
    bool_t   mValidDevices = FALSE;
    static int alloc_fail = 0;

    /* Valid devices are in Running state */
    for (mPeerId = 0; mPeerId < (uint8_t)gAppMaxConnections_c; mPeerId++)
    {
        if ((gInvalidDeviceId_c != maPeerInformation[mPeerId].deviceId) &&
            (mAppRunning_c == maPeerInformation[mPeerId].appState))
        {
            mValidDevices = TRUE;
            break;
        }
    }

    if (mValidDevices)
    {
        bool_t continue_read = false;
        do {
            uint8_t *pMsg = NULL;
            uint16_t bytesRead = 0;
            /* Allocate buffer for GATT Write */
            pMsg = MEM_BufferAlloc(mAppUartBufferSize);

            if (NULL == pMsg)
            {
                alloc_fail++;
                /* Allocation failed */
                SERIAL_DBG_LOG("Allocation failed %d times", alloc_fail);
                break;
            }
            /* Collect the data from the serial manager buffer */
            if (Serial_Read(gAppSerMgrIf, pMsg, mAppUartBufferSize, &bytesRead) == gSerial_Success_c)
            {
                if (bytesRead != 0U)
                {
                    /* Send data over the air */
                    BleApp_SendUartStream(pMsg, (uint8_t)bytesRead);
                }
                continue_read =  (bytesRead == mAppUartBufferSize);
                /* Free Buffer */
                (void)MEM_BufferFree(pMsg);
            }
            else
                break;
        } while (continue_read);
    }

    mAppDapaPending = FALSE;
}

static void BleApp_ReceivedUartStream(deviceId_t peerDeviceId, uint8_t *pStream, uint16_t streamLength)
{
    static deviceId_t previousDeviceId = gInvalidDeviceId_c;

    char additionalInfoBuff[10] = { '\r', '\n', '[', '0', '0', '-', 'M', ']', ':', ' '};
    uint8_t *pBuffer = NULL;
    uint32_t messageHeaderSize = 0;

    if (mAppUartNewLine || (previousDeviceId != peerDeviceId))
    {
        streamLength += (uint32_t)sizeof(additionalInfoBuff);
    }

    /* Allocate buffer for asynchronous write */
    pBuffer = MEM_BufferAlloc(streamLength);

    if (pBuffer != NULL)
    {
        /* if this is a message from a previous device, print device ID */
        if (mAppUartNewLine || (previousDeviceId != peerDeviceId))
        {
            messageHeaderSize = sizeof(additionalInfoBuff);

            if (mAppUartNewLine)
            {
                mAppUartNewLine = FALSE;
            }

            additionalInfoBuff[3] = '0' + (peerDeviceId / 10U);
            additionalInfoBuff[4] = '0' + (peerDeviceId % 10U);

            if (gGapCentral_c != maPeerInformation[peerDeviceId].gapRole)
            {
                additionalInfoBuff[6] = 'S';
            }

            FLib_MemCpy(pBuffer, additionalInfoBuff, sizeof(additionalInfoBuff));
        }

        FLib_MemCpy(pBuffer + messageHeaderSize, pStream, (uint32_t)streamLength - messageHeaderSize);
        (void)Serial_AsyncWrite(gAppSerMgrIf, pBuffer, streamLength, Uart_TxCallBack, pBuffer);
    }

    /* update the previous device ID */
    previousDeviceId = peerDeviceId;
}

#if (cPWR_FullPowerDownMode==0) || gPWR_SerialUartRxWakeup
static void UartStreamFlushTimerCallback(void *pData)
{
    if (!mAppDapaPending)
    {
        mAppDapaPending = TRUE;
        (void)App_PostCallbackMessage(BleApp_FlushUartStream, NULL);
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles UART Receive callback.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
static void Uart_RxCallBack(void *pData)
{
    uint16_t byteCount = 0;

    (void)Serial_RxBufferByteCount(gAppSerMgrIf, &byteCount);

    if (byteCount < mAppUartBufferSize)
    {
#if (cPWR_FullPowerDownMode==0) || gPWR_SerialUartRxWakeup
        /* Restart flush timer */
        (void)TMR_StartLowPowerTimer(mUartStreamFlushTimerId,
                                     gTmrLowPowerSingleShotMillisTimer_c,
                                     mAppUartFlushIntervalInMs_c,
                                     UartStreamFlushTimerCallback, NULL);
#endif
    }
    else
    {
        /* Post App Msg only one at a time */
        if (!mAppDapaPending)
        {
            mAppDapaPending = TRUE;
            (void)App_PostCallbackMessage(BleApp_FlushUartStream, NULL);
        }
    }
}

/*! *********************************************************************************
* \brief        Handles UART Transmit callback.
*
* \param[in]    pData        Parameters.
********************************************************************************** */
static void Uart_TxCallBack(void *pBuffer)
{
    (void)MEM_BufferFree(pBuffer);
}

#if (cPWR_FullPowerDownMode)
/*! *********************************************************************************
* \brief        Check if it is time for a specific ADC measurement
*
* \param[in]    measureTodo         the ADC measurement to check if it is time to do it
* \param[in]    lastMeasureTime     Last the ADC measurement was done
* \param[in]    measureInterval     Measurement time interval
********************************************************************************** */

static void AdcMeasurement_TimeCheck(uint32_t measureTodo, uint32_t* lastMeasureTime, uint32_t measureInterval)
{
    uint32_t        current_time;
    bool_t          do_measurement = FALSE;

    current_time = TMR_RTCGetTimestampSeconds();

    /* check if it is time for  measurement */
    if(current_time > *lastMeasureTime)
    {
        if((current_time - *lastMeasureTime) >= measureInterval)
            do_measurement = TRUE;
    }
    else if(current_time >= (*lastMeasureTime + measureInterval))
    {
        do_measurement = TRUE;
    }

    if(do_measurement)
    {
        *lastMeasureTime = current_time;
        if(!mAdcMeasureToDo)
        {
            /* init ADC if still no measurement scheduled*/
            BOARD_InitAdc();

            App_PostCallbackMessage(AdcMeasurementCallback, NULL);
        }
        mAdcMeasureToDo |= measureTodo;
    }
}


/*! *********************************************************************************
* \brief        Handles Adc measurement callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdcMeasurementCallback(void * pParam)
{
    BOARD_EnableAdc();

    /* do the measurements required */

    if(mAdcMeasureToDo & gAdcMeasureBatteryLevelFirst_c)
    {
        /* Assume that when reaching this point , at least 230us needed for ADC init has elapsed */
        mBasServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
        Bas_Start(&mBasServiceConfig);
    }

    if ( mAdcMeasureToDo & gAdcMeasureTemperature_c)
    {
        int32_t temperature = BOARD_GetTemperature();
#ifdef CPU_JN518X
        /* Update Temperature in XCVR for calibration purpose */
        XCVR_TemperatureUpdate(temperature);
#endif
    }

    if(mConnectionNb > 0)
    {
        if ( mAdcMeasureToDo & gAdcMeasureBatteryLevel_c)
        {
            mBasServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
            Bas_RecordBatteryMeasurement(&mBasServiceConfig);
        }
    }
    /* all pending measurements are done */
    mAdcMeasureToDo  = 0;

    BOARD_DeInitAdc();
}

#else

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    /* Assume that when reaching this point , at least 230us needed for ADC init has elapsed */
    BOARD_EnableAdc();
    mBasServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    if(mAdcMeasureToDo & gAdcMeasureBatteryLevelFirst_c)
    {
        /* when no low power, mAdcMeasureToDo is set only once to distinguish first battery measurement from next ones */
        /* not used for temperature measurement. So it can be reset to  0 */
        mAdcMeasureToDo = 0;
        Bas_Start(&mBasServiceConfig);
    }
    else
    {
        Bas_RecordBatteryMeasurement(&mBasServiceConfig);
    }

}

/*! *********************************************************************************
* \brief        Handles Temperature measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void TemperatureMeasurementTimerCallback(void * pParam)
{
    /* Assume that when reaching this point , at least 230us needed for ADC init has elapsed */
    BOARD_EnableAdc();
    BOARD_GetTemperature();
}

#endif

#if defined(gUseControllerNotifications_c) && (gUseControllerNotifications_c)
static void BleApp_HandleControllerNotification(bleNotificationEvent_t *pNotificationEvent)
{
    switch(pNotificationEvent->eventType)
    {
        case gNotifEventNone_c:
        {
            Serial_Print(gAppSerMgrIf, "Configured notification status ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->status);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifConnEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Event Over device ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->deviceId);
            Serial_Print(gAppSerMgrIf, " on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, " and event counter ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint16_t)pNotificationEvent->ce_counter);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifConnRxPdu_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Rx PDU from device ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->deviceId);
            Serial_Print(gAppSerMgrIf, " on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, " with event counter ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint16_t)pNotificationEvent->ce_counter);
            Serial_Print(gAppSerMgrIf, " and timestamp ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->timestamp);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifAdvEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Event Over.\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifAdvTx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Tx on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifAdvScanReqRx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Rx Scan Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifAdvConnReqRx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Rx Conn Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifScanEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Event Over on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifScanAdvPktRx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Rx Adv Pkt on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifScanRspRx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Rx Scan Rsp on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, " with RSSI ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, (uint8_t)pNotificationEvent->rssi);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifScanReqTx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Tx Scan Req on channel ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->channel);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        case gNotifConnCreated_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Created with device ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->deviceId);
            Serial_Print(gAppSerMgrIf, " with timestamp ", gAllowToBlock_d);
            Serial_PrintDec(gAppSerMgrIf, pNotificationEvent->timestamp);
            Serial_Print(gAppSerMgrIf, "\n\r", gAllowToBlock_d);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}

#if defined(gUseControllerNotificationsCallback_c) && (gUseControllerNotificationsCallback_c)
static void BleApp_ControllerNotificationCallback(bleCtrlNotificationEvent_t *pNotificationEvent)
{
    switch(pNotificationEvent->event_type)
    {
        case gNotifConnEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifConnRxPdu_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Rx PDU\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvTx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Tx\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvScanReqRx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Rx Scan Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifAdvConnReqRx_c:
        {
            Serial_Print(gAppSerMgrIf, "ADV Rx Conn Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanEventOver_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Ev Over\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanAdvPktRx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Rx Adv\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanRspRx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Rx Scan Rsp\n\r", gNoBlock_d);
            break;
        }

        case gNotifScanReqTx_c:
        {
            Serial_Print(gAppSerMgrIf, "SCAN Tx Scan Req\n\r", gNoBlock_d);
            break;
        }

        case gNotifConnCreated_c:
        {
            Serial_Print(gAppSerMgrIf, "CONN Created\n\r", gNoBlock_d);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
}
#endif
#endif

/*! *********************************************************************************
 * @}
 ********************************************************************************** */
