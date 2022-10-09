/*! *********************************************************************************
* \addtogroup Temperature Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Temperature Sensor application
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
#include "MemManager.h"
#include "Panic.h"
#include "SerialManager.h"
#if defined (CPU_JN518X)
#include "pin_mux.h"
#endif
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#endif

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#if !defined(MULTICORE_APPLICATION_CORE) || (!MULTICORE_APPLICATION_CORE)
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "temperature_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "temperature_sensor.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#if defined (CPU_JN518X) && (defined (cPWR_FullPowerDownMode) && cPWR_FullPowerDownMode)
#define SERIAL_INTERFACE_ON_SENSOR 0
#else
#define SERIAL_INTERFACE_ON_SENSOR 1
#endif
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct advState_tag{
    bool_t      advOn;
}advState_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static advState_t  mAdvState;

static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
static bool_t           basValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t      disServiceConfig = {(uint16_t)service_device_info};

static tmsConfig_t tmsServiceConfig = {(uint16_t)service_temperature, 0};

/* Application specific data*/
static tmrTimerID_t appTimerId;

#if SERIAL_INTERFACE_ON_SENSOR
static uint8_t gAppSerMgrIf;
#endif

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);
static void BleApp_Config(void);
#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit);
#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);
#endif
#endif
/* Timer Callbacks */
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static void AdvertisingTimerCallback(void *pParam);
static void DisconnectTimerCallback(void *pParam);
#endif

static void BleApp_Advertise(void);
static void BleApp_SendTemperature(void);

static bool SleepTimeoutSequence(void);

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
    APP_DBG_LOG(""); 
#if defined CPU_JN518X
    BleAppDrv_Init(false);
#if (cPWR_UsePowerDownMode)
    PWR_RegisterLowPowerExitCallback(BleAppDrv_InitCB);
    PWR_RegisterLowPowerEnterCallback(BleAppDrv_DeInit);
#endif
#else
    /* Initialize application support for drivers */
    BOARD_InitAdc();
#if SERIAL_INTERFACE_ON_SENSOR
#if (defined(KW37A4_SERIES) || defined(KW37Z4_SERIES) || defined(KW38A4_SERIES) || defined(KW38Z4_SERIES) || defined(KW39A4_SERIES))
    Serial_InitManager();
#else
    SerialManager_Init();
#endif
    Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);
    Serial_SetBaudRate(gAppSerMgrIf, APP_SERIAL_INTERFACE_SPEED);
#endif
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
    Led1On();

    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
        /* Device is not connected and not advertising */
        if (!mAdvState.advOn)
        {
            gPairingParameters.localIoCapabilities = gIoDisplayOnly_c;
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && !defined (CPU_JN518X)
    #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
        #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
            (void)PWR_ChangeBlackBoxDeepSleepMode(gAppDeepSleepMode_c);
        #endif
    #else
            (void)PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
    #endif
#endif

            /* Set advertising parameters, advertising to start on gAdvertisingParametersSetupComplete_c */
            BleApp_Advertise();
        }
    }
    else
    {
        /* Device is connected, send temperature value */
        BleApp_SendTemperature();
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
    APP_DBG_LOG(""); 
#if gLoggingActive_d
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            BleApp_Start();
            break;
        }

        case gKBD_EventPressPB2_c:
        {
            DbgLogDump(true);
            break;
        }
        default:
        {
            ; /* No action required */
            break;
        }
    }
#else
    /* Start application on any key */
    BleApp_Start();
#endif
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    APP_DBG_LOG("Evt=%x", pGenericEvent->eventType);
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
        case gInitializationComplete_c:
        {
            /* Configure application and start services */
            BleApp_Config();
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
        }
        break;

        case gAdvertisingDataSetupComplete_c:
        {
            /* Start advertising if data and parameters were successfully set */
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
#if SERIAL_INTERFACE_ON_SENSOR
//            Serial_Print(gAppSerMgrIf, "\r\ngAdvertisingSetupFailed_c\r\n", gNoBlock_d);
#endif
//            panic(0,0,0,0);
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
    APP_DBG_LOG("");

    /* init ADC, done periodically */
    BOARD_ADCWakeupInit();

    if ( reinit )
    {
  #if gKeyBoardSupported_d
        KBD_PrepareExitLowPower();
  #endif
  #if SERIAL_INTERFACE_ON_SENSOR
        SerialInterface_Reinit(gAppSerMgrIf);
  #endif
    }
    else
    {
        (void) reinit;
#if SERIAL_INTERFACE_ON_SENSOR
        /* UI */
        Serial_InitManager();
        /* Register Serial Manager interface */
        Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);
#endif
    }
#if SERIAL_INTERFACE_ON_SENSOR
    Serial_SetBaudRate(gAppSerMgrIf, BOARD_DEBUG_UART_BAUDRATE); /* might be 9600kbps just as well */
#endif
}

#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void)
{
    APP_DBG_LOG("");
    BleAppDrv_Init(true);
}

static void BleAppDrv_DeInit()
{
    APP_DBG_LOG("");
    /* DeInitialize application support for drivers */
    BOARD_DeInitAdc();

    /* Deinitialize debug console */
    BOARD_DeinitDebugConsole();

    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
}
#endif /* cPWR_UsePowerDownMode */
#endif /* CPU_JN518X */

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);

    /* Set Tx power level */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    mAdvState.advOn = FALSE;

    /* Start services */
    tmsServiceConfig.initialTemperature = (int16_t)(100 * BOARD_GetTemperature());
    (void)Tms_Start(&tmsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timer */
    appTimerId = TMR_AllocateTimer();
#if SERIAL_INTERFACE_ON_SENSOR
    (void)Serial_Print(gAppSerMgrIf, "\n\rTemperature sensor -> Press switch to start advertising.\n\r", gAllowToBlock_d);
#endif
#if !defined CPU_JN518X
    /* Set low power mode */
  #if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
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
#endif
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
     APP_DBG_LOG("");
    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);
#if defined (CPU_JN518X)
#if (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
    PWR_AllowDeviceToSleep();
#endif
#endif
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    APP_DBG_LOG("Evt=%x", pAdvertisingEvent->eventType);
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;

            if(!mAdvState.advOn)
            {
#if SERIAL_INTERFACE_ON_SENSOR
                (void)Serial_Print(gAppSerMgrIf, "Advertising stopped.\n\r", gAllowToBlock_d);
#endif
                /* Go to sleep */
                Led1Off();
                if (!SleepTimeoutSequence())
                {
                    /* UI */
                    LED_StopFlashingAllLeds();
                    Led1Flashing();
                    Led2Flashing();
                    Led3Flashing();
                    Led4Flashing();
                }
            }
            else
            {
#if SERIAL_INTERFACE_ON_SENSOR
                (void)Serial_Print(gAppSerMgrIf, "Advertising started.\n\r", gAllowToBlock_d);
#endif
                
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)                
                /* Start advertising timer */
                (void)TMR_StartLowPowerTimer(appTimerId,
                       gTmrLowPowerSecondTimer_c,
                       TmrSeconds(gAdvTime_c),
                       AdvertisingTimerCallback, NULL);
                Led1On();
#else
                /* UI */
                LED_StopFlashingAllLeds();
                Led1Flashing();
#endif
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            /* Panic UI */
            Led2On();
            panic(0,0,0,0);
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
    APP_DBG_LOG("Evt=%x", pConnectionEvent->eventType);
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;
            (void)TMR_StopTimer(appTimerId);

            /* Subscribe client*/
            mPeerDeviceId = peerDeviceId;
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Subscribe(peerDeviceId);

#if SERIAL_INTERFACE_ON_SENSOR
            (void)Serial_Print(gAppSerMgrIf, "Connected!\n\r", gAllowToBlock_d);
#endif
            /* Set low power mode */
            if (SleepTimeoutSequence())
            {
                LED_StopFlashingAllLeds();
            }
            /* UI */
            Led1On();

            PWR_AllowDeviceToSleep();
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            mPeerDeviceId = gInvalidDeviceId_c;
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Unsubscribe();

#if SERIAL_INTERFACE_ON_SENSOR
            (void)Serial_Print(gAppSerMgrIf, "Disconnected!\n\r", gAllowToBlock_d);
#endif
            
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && !defined (CPU_JN518X)
            /* UI */
            Led1Off();
            /* Go to sleep */
            SleepTimeoutSequence();
#else
            /* restart advertising*/
            BleApp_Start();
#endif
        }
        break;

        case gConnEvtEncryptionChanged_c:   /* Fallthrough */
        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT server callback from host stack.
*
* \param[in]    deviceId        Peer device ID.
* \param[in]    pServerEvent    Pointer to gattServerEvent_t.
********************************************************************************** */
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent)
{
    APP_DBG_LOG("Evt=%x", pServerEvent->eventType);
    switch (pServerEvent->eventType)
    {
        case gEvtCharacteristicCccdWritten_c:
        {
            /* Notify the temperature value when CCCD is written */
            BleApp_SendTemperature();
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
/*! *********************************************************************************
* \brief        Stops advertising when the application timeout has expired.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void* pParam)
{
    APP_DBG_LOG("");
    /* Stop advertising */
    if (mAdvState.advOn)
    {
        (void)Gap_StopAdvertising();
    }
}

/*! *********************************************************************************
* \brief        Handles disconnect timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    APP_DBG_LOG("");
    /* Terminate connection */
    if (mPeerDeviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(mPeerDeviceId);
    }
}
#endif

/*! *********************************************************************************
* \brief        Sends temperature value over-the-air.
*
********************************************************************************** */
static void BleApp_SendTemperature(void)
{
    APP_DBG_LOG("");
    (void)TMR_StopTimer(appTimerId);

    /* Update with initial temperature */
    (void)Tms_RecordTemperatureMeasurement((uint16_t)service_temperature,
                                           (int16_t)(BOARD_GetTemperature() * 100));

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    /* Start Sleep After Data timer */
    (void)TMR_StartLowPowerTimer(appTimerId,
                           gTmrLowPowerSecondTimer_c,
                           TmrSeconds(gGoToSleepAfterDataTime_c),
                           DisconnectTimerCallback, NULL);
#endif
}

static bool SleepTimeoutSequence(void)
{
    APP_DBG_LOG("");
    /* Does nothing if cPWR_UsePowerDownMode unset */
    bool res = false;
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)

  #if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE)
    #if defined(gErpcLowPowerApiServiceIncluded_c) && (gErpcLowPowerApiServiceIncluded_c)
        (void)PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
    #endif
  #else
    (void)PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
  #endif
    res = true;
#endif
    return res;
}

/*! *********************************************************************************
* @}
********************************************************************************** */
