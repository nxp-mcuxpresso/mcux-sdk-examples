/*! *********************************************************************************
* \addtogroup Heart Rate Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Heart Rate Sensor application
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
/* Framework / Drivers */
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "MemManager.h"
#include "Panic.h"


#if (cPWR_UsePowerDownMode)
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#endif

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"

#if MULTICORE_APPLICATION_CORE
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "heart_rate_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "heart_rate_sensor.h"
#ifdef CPU_JN518X
#include "pin_mux.h"
#endif
#if MULTICORE_APPLICATION_CORE
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define mHeartRateLowerLimit_c          (40) /* Heart beat lower limit, 8-bit value */
#define mHeartRateUpperLimit_c          (201) /* Heart beat upper limit, 8-bit value */
#define mHeartRateRange_c               (mHeartRateUpperLimit_c - mHeartRateLowerLimit_c) /* Range = [ADC16_HB_LOWER_LIMIT .. ADC16_HB_LOWER_LIMIT + ADC16_HB_DYNAMIC_RANGE] */
#define mHeartRateReportInterval_c      (1)        /* heart rate report interval in seconds  */
#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */
/* Define AdvAutoStartDelay_c if you prefer to start Adv on timeout rather than button press */
//#define AdvAutoStartDelay_c              (3)
#ifndef AdvAutoStartDelay_c
#define AdvAutoStartDelay_c              (0)
#endif 
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
    fastWhiteListAdvState_c,
#endif
#endif
    fastAdvState_c,
    slowAdvState_c
}advType_t;

typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
}advState_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static advState_t  mAdvState;
static bool_t      mRestartAdv;
static uint32_t    mAdvTimeout;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {service_battery, 0, basValidClientList, gAppMaxConnections_c};
static hrsUserData_t    hrsUserData;
static hrsConfig_t hrsServiceConfig = {service_heart_rate, TRUE, TRUE, TRUE, gHrs_BodySensorLocChest_c, &hrsUserData};
static uint16_t cpHandles[1] = { value_hr_ctrl_point };

/* Application specific data*/
static bool_t mToggle16BitHeartRate = FALSE;
static bool_t mContactStatus = TRUE;
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;
#if (AdvAutoStartDelay_c > 0)
static tmrTimerID_t mDelayedAdvStartTimerId;
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

/* Timer Callbacks */
static void AdvertisingTimerCallback (void *);
static void TimerMeasurementCallback (void *);
static void BatteryMeasurementTimerCallback (void *);
#if (AdvAutoStartDelay_c > 0)
static void AdvStartTimerCallback(void *);
#endif
static void BleApp_Advertise(void);

#if defined (CPU_JN518X)
static void BleAppDrv_Init(bool reinit);
#if (cPWR_UsePowerDownMode)
static void BleAppDrv_InitCB(void);
static void BleAppDrv_DeInit(void);
#endif
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

#if MULTICORE_APPLICATION_CORE
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
    /* Device is not connected and not advertising*/
    if (!mAdvState.advOn)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        if (gcBondedDevices > 0)
        {
            mAdvState.advType = fastWhiteListAdvState_c;
        }
        else
#endif
#endif
        {
            mAdvState.advType = fastAdvState_c;
        }
#if !defined (CPU_JN518X)
#if (cPWR_UsePowerDownMode)
    #if MULTICORE_APPLICATION_CORE
        #if gErpcLowPowerApiServiceIncluded_c
            PWR_ChangeBlackBoxDeepSleepMode(gAppDeepSleepMode_c);
        #endif
    #else
        PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
    #endif
#endif
#endif
        BleApp_Advertise();
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
void BleApp_HandleKeys(key_event_t events)
{
#if (cPWR_UsePowerDownMode)
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
#if (AdvAutoStartDelay_c == 0)
            if (mPeerDeviceId == gInvalidDeviceId_c)
            {
                BleApp_Start();
            }
#endif
            break;
        }
        case gKBD_EventPressPB2_c:
        {
#if (gLoggingActive_d)
            DbgLogDump(true);
#endif
            break;
        }
        case gKBD_EventLongPB1_c:
        case gKBD_EventLongPB2_c:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }
        default:
            break;
    }
#else
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            if (mPeerDeviceId == gInvalidDeviceId_c)
            {
                BleApp_Start();
            }
            break;
        }
        case gKBD_EventPressPB2_c:
        {
            mToggle16BitHeartRate = (mToggle16BitHeartRate)?FALSE:TRUE;
        }
        break;
        case gKBD_EventLongPB1_c:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }
        case gKBD_EventLongPB2_c:
        {
            mContactStatus = mContactStatus?FALSE:TRUE;
            Hrs_SetContactStatus(service_heart_rate, mContactStatus);
            break;
        }
        default:
            break;
    }
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
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
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
    /* Initialize application support for drivers */
    
    /* init ADC, done periodically */
    BOARD_ADCWakeupInit();
    
  #if (cPWR_UsePowerDownMode)
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
#endif
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
#endif
#endif


/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config()
{
    APP_DBG_LOG("");

#if MULTICORE_APPLICATION_CORE
    if (GattDbDynamic_CreateDatabase() != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }
#endif /* MULTICORE_APPLICATION_CORE */

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Set Tx power level */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);

    /* Register for callbacks*/
    GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    App_RegisterGattServerCallback(BleApp_GattServerCallback);

    mAdvState.advOn = FALSE;

    /* Start services */
    hrsServiceConfig.sensorContactDetected = mContactStatus;
#if gHrs_EnableRRIntervalMeasurements_d
    hrsServiceConfig.pUserData->pStoredRrIntervals = MEM_BufferAlloc(sizeof(uint16_t) * gHrs_NumOfRRIntervalsRecorded_c);
#endif
    Hrs_Start(&hrsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    Bas_Start(&basServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mMeasurementTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
#if (AdvAutoStartDelay_c > 0)
    mDelayedAdvStartTimerId = TMR_AllocateTimer();

    TMR_StartLowPowerTimer(mDelayedAdvStartTimerId,
            gTmrLowPowerSingleShotMillisTimer_c,
            TmrSeconds(AdvAutoStartDelay_c),
            AdvStartTimerCallback,
            NULL);
#endif

#if (cPWR_UsePowerDownMode)
    #if MULTICORE_APPLICATION_CORE
        #if gErpcLowPowerApiServiceIncluded_c
            PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
            PWR_AllowBlackBoxToSleep();
        #endif
        PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
        PWR_AllowDeviceToSleep();
    #else
        PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
        PWR_AllowDeviceToSleep();
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
    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
            mAdvTimeout = gFastConnWhiteListAdvTime_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimeout = gReducedPowerAdvTime_c;
        }
        break;
    }

    /* Set advertising parameters*/
    Gap_SetAdvertisingParameters(&gAdvParams);
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
        case gExtAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;

            if (!mAdvState.advOn && mRestartAdv)
            {
                BleApp_Advertise();
                break;
            }

#if (cPWR_UsePowerDownMode)
            if(!mAdvState.advOn)
            {
                Led1Off();
                #if MULTICORE_APPLICATION_CORE
                    #if gErpcLowPowerApiServiceIncluded_c
                        PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
                    #endif
                #else
                    PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
                #endif
            }
            else
            {
                /* Start advertising timer */
                TMR_StartLowPowerTimer(mAdvTimerId,gTmrLowPowerSecondTimer_c,
                         TmrSeconds(mAdvTimeout), AdvertisingTimerCallback, NULL);
                Led1On();
            }
#else
            LED_StopFlashingAllLeds();
            Led1Flashing();

            if(!mAdvState.advOn)
            {
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
            else
            {
                TMR_StartLowPowerTimer(mAdvTimerId,gTmrLowPowerSecondTimer_c,
                        TmrSeconds(mAdvTimeout), AdvertisingTimerCallback, NULL);
            }
#endif
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
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
            mPeerDeviceId = peerDeviceId;

            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Subscribe client*/
            Bas_Subscribe(&basServiceConfig, peerDeviceId);
            Hrs_Subscribe(peerDeviceId);

            /* Stop Advertising Timer*/
            mAdvState.advOn = FALSE;
            TMR_StopTimer(mAdvTimerId);

            /* Start measurements */
            TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mHeartRateReportInterval_c), TimerMeasurementCallback, NULL);

            /* Start battery measurements */
            TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);

#if (cPWR_UsePowerDownMode)
#if !defined (CPU_JN518X)
             #if MULTICORE_APPLICATION_CORE
                #if gErpcLowPowerApiServiceIncluded_c
                    PWR_ChangeBlackBoxDeepSleepMode(gAppDeepSleepMode_c);
                    PWR_AllowBlackBoxToSleep();
                #endif
             #else
                PWR_ChangeDeepSleepMode(gAppDeepSleepMode_c);
                PWR_AllowDeviceToSleep();
             #endif
#else
           PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
           PWR_AllowDeviceToSleep();
#endif
#else
            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();
#endif
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            Hrs_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            TMR_StopTimer(mMeasurementTimerId);
            TMR_StopTimer(mBatteryMeasurementTimerId);

#if (cPWR_UsePowerDownMode) && !defined (CPU_JN518X)
            /* UI */
            Led1Off();

            /* Go to sleep */
    #if MULTICORE_APPLICATION_CORE
        #if gErpcLowPowerApiServiceIncluded_c
            PWR_ChangeBlackBoxDeepSleepMode(cPWR_DeepSleepMode);
        #endif
    #else
            PWR_ChangeDeepSleepMode(cPWR_DeepSleepMode);
    #endif
#else
            /* Restart advertising*/
            BleApp_Start();
#endif
        }
        break;
    default:
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
    uint16_t handle;
    uint8_t status;
    APP_DBG_LOG("Evt=%x", pServerEvent->eventType);
    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            status = gAttErrCodeNoError_c;

            if (handle == value_hr_ctrl_point)
            {
                status = Hrs_ControlPointHandler(&hrsUserData, pServerEvent->eventData.attributeWrittenEvent.aValue[0]);
            }

            GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
        }
        break;
    default:
        break;
    }
}


/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void * pParam)
{
    APP_DBG_LOG("");
    /* Stop and restart advertising with new parameters */
    Gap_StopAdvertising();

    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            mAdvState.advType = fastAdvState_c;
            mRestartAdv = TRUE;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            mAdvState.advType = slowAdvState_c;
            mRestartAdv = TRUE;
        }
        break;

        default:
        {
            mRestartAdv = FALSE;
        }
        break;
    }
}
#if (AdvAutoStartDelay_c > 0)
static void AdvStartTimerCallback(void * pParam)
{
    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
        BleApp_Start();
    }
}
#endif

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void * pParam)
{
    APP_DBG_LOG("");
    uint16_t hr = BOARD_GetPotentiometerLevel();
    hr = (hr * mHeartRateRange_c) >> 12;

#if gHrs_EnableRRIntervalMeasurements_d
    Hrs_RecordRRInterval(&hrsUserData, (hr & 0x0F));
    Hrs_RecordRRInterval(&hrsUserData,(hr & 0xF0));
#endif

    if (mToggle16BitHeartRate)
    {
        Hrs_RecordHeartRateMeasurement(service_heart_rate, 0x0100 + (hr & 0xFF), &hrsUserData);
    }
    else
    {
        Hrs_RecordHeartRateMeasurement(service_heart_rate, mHeartRateLowerLimit_c + hr, &hrsUserData);
    }

    Hrs_AddExpendedEnergy(&hrsUserData, 100);
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    APP_DBG_LOG("");
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    Bas_RecordBatteryMeasurement(&basServiceConfig);
}

/*! *********************************************************************************
* @}
********************************************************************************** */