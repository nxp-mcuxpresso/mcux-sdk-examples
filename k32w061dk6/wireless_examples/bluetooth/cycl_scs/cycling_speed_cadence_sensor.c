/*! *********************************************************************************
* \addtogroup Cycling Speed Cadence Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Cycling Speed Cadence Sensor application
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
#include "RNG_Interface.h"
#include "Keyboard.h"
#include "LED.h"
#include "TimersManager.h"
#include "FunctionLib.h"
#include "Panic.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "cycling_speed_cadence_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "cycling_speed_cadence_sensor.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/
#define mSpeedCadenceReportInterval_c   (1000U)      /* Speed and Cadence report interval in miliseconds  */
#define mBatteryLevelReportInterval_c   (10U)        /* Battery level report interval in seconds  */
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
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Configuration*/
static cscsSensorLocation_t mSensorLocations[] = {(uint8_t)gCscs_FrontWheel_c, gCscs_LeftCrank_c, gCscs_RightCrank_c, (uint8_t)gCscs_LeftPedal_c};
static cscsUserData_t   mCscsUserData = {0, mSensorLocations, 0, 4};
static cscsConfig_t cscsServiceConfig = {(uint16_t)service_csc,
                                        gCscs_WheelRevDataSupported_c |
                                        gCscs_CrankRevDataSupported_c |
                                        gCscs_MultipleSensorLocationsSupported_c,
                                        &mCscsUserData,
                                        (uint8_t)gCscs_FrontWheel_c};
static uint16_t cpHandles[] = { (uint16_t)value_sc_ctrl_point };

static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;

static bool_t mReverseWheel = FALSE;
static uint16_t lastEventTime = 0;

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
static void AdvertisingTimerCallback (void *pParam);
static void TimerMeasurementCallback (void *pParam);
static void BatteryMeasurementTimerCallback (void *pParam);

static void BleApp_Advertise(void);

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
    /* Initialize application support for drivers */
    BOARD_InitAdc();

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
    /* Init eRPC host */
    init_erpc_host();
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        if (gcBondedDevices > 0U)
        {
            mAdvState.advType = fastWhiteListAdvState_c;
        }
        else
#endif
        {
#endif
            mAdvState.advType = fastAdvState_c;
#if gAppUseBonding_d
        }
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
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            BleApp_Start();
            break;
        }
        case gKBD_EventPressPB2_c:
        {
            mReverseWheel = !mReverseWheel;
            break;
        }
        case gKBD_EventLongPB1_c:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }
        case gKBD_EventLongPB2_c:
        default:
            ; /* For MISRA compliance */
            break;
    }
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
void BleApp_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
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
            ; /* For MISRA compliance */
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
static void BleApp_Config(void)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
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
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);

    mAdvState.advOn = FALSE;

    /* Start services */
    (void)Cscs_Start(&cscsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mMeasurementTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    uint32_t timeout = 0;

    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessWhiteListOnly_c;
            timeout = gFastConnWhiteListAdvTime_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gReducedPowerAdvTime_c;
        }
        break;

    default:
        ; /* For MISRA compliance */
        break;
    }

    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);

    /* Start advertising timer */
    (void)TMR_StartLowPowerTimer(mAdvTimerId,gTmrLowPowerSecondTimer_c,
               TmrSeconds(timeout), AdvertisingTimerCallback, NULL);
}

/*! *********************************************************************************
* \brief        Handles BLE Advertising callback from host stack.
*
* \param[in]    pAdvertisingEvent    Pointer to gapAdvertisingEvent_t.
********************************************************************************** */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent)
{
    switch (pAdvertisingEvent->eventType)
    {
        case gAdvertisingStateChanged_c:
        {
            mAdvState.advOn = !mAdvState.advOn;
            LED_StopFlashingAllLeds();
            Led1Flashing();

            if(!mAdvState.advOn)
            {
                Led2Flashing();
                Led3Flashing();
                Led4Flashing();
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
            ; /* For MISRA compliance */
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
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            mPeerDeviceId = peerDeviceId;

            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Subscribe client*/
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Cscs_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start measurements */
            (void)TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       mSpeedCadenceReportInterval_c, TimerMeasurementCallback, NULL);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Cscs_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Restart advertising*/
            BleApp_Start();
        }
        break;
    default:
        ; /* For MISRA compliance */
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

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            status = (uint8_t)gAttErrCodeNoError_c;

            if (handle == (uint16_t)value_sc_ctrl_point)
            {
                Cscs_ControlPointHandler(&cscsServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }
            else
            {
                (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
            }
            break;
        }
        case gEvtHandleValueConfirmation_c:
        {
            cscsServiceConfig.procInProgress = FALSE;
            break;
        }
        case gEvtError_c:
        {
            if (pServerEvent->eventData.procedureError.error == gGattClientConfirmationTimeout_c)
            {
                /* Procedure timeout at ATT */
                cscsServiceConfig.procInProgress = FALSE;
            }
            break;
        }
    default:
        ; /* For MISRA compliance */
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
    /* Stop and restart advertising with new parameters */
    (void)Gap_StopAdvertising();

    switch (mAdvState.advType)
    {
#if gAppUseBonding_d
#if defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0)
        case fastWhiteListAdvState_c:
        {
            mAdvState.advType = fastAdvState_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            mAdvState.advType = slowAdvState_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
        break;
    }
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void * pParam)
{
    uint32_t randomNumber = 0;
    cscsMeasurement_t measurement;

    if (cscsServiceConfig.calibrationInProgress)
    {
        /* Finish calibration procedure */
        (void)Cscs_FinishSensorCalibration(&cscsServiceConfig, TRUE);
    }

    RNG_GetRandomNo(&randomNumber);

    mCscsUserData.cumulativeCrankRevs += (uint16_t)randomNumber & 0x03U;
    lastEventTime += (uint16_t)mSpeedCadenceReportInterval_c * 1000U /((uint16_t)1U<<10U)  ;
    if (mReverseWheel)
    {
        mCscsUserData.cumulativeWheelRevs = ((mCscsUserData.cumulativeWheelRevs)>10U)?
                                            mCscsUserData.cumulativeWheelRevs-10U:0U;
    }
    else
    {
        mCscsUserData.cumulativeWheelRevs += (randomNumber & 0x03U) + 1U;
    }

    measurement.crankRevDataPresent = TRUE;
    measurement.cumulativeCrankRevs = mCscsUserData.cumulativeCrankRevs;
    measurement.lastCrankEventTime = lastEventTime;

    measurement.wheelRevDataPresent = TRUE;
    measurement.cumulativeWheelRevs = mCscsUserData.cumulativeWheelRevs;
    measurement.lastWheelEventTime = lastEventTime;

    (void)Cscs_RecordMeasurement((uint16_t)service_csc, &measurement);

}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}
/*! *********************************************************************************
* @}
********************************************************************************** */
