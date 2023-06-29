/*! *********************************************************************************
* \addtogroup Blood Pressure Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Blood Pressure Sensor application
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
#include "MemManager.h"
#include "Panic.h"

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
#include "blood_pressure_interface.h"
#include "current_time_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "blood_pressure_sensor.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define mBatteryLevelReportInterval_c   (10U)           /* battery level report interval in seconds  */
#define mReadTimeCharInterval_c         (3600U)         /* interval between time sync in seconds */
#define mCharReadBufferLength_c         (13U)           /* length of the buffer */
#define mInitialTime_c                  (1451606400U)   /* initial timestamp - 01/01/2016 00:00:00 GMT */
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

/* Service Data*/
#if gBps_EnableMultiUserSupport_d || gBps_EnableStoredMeasurements_d
static bpsUserData_t        mUserData;
#endif

static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static bpsConfig_t      bpsServiceConfig = {(uint16_t)service_blood_pressure, gBps_InitDefaultConfig,
                                        &mUserData};

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;
static tmrTimerID_t mLocalTimeTickTimerId;
#if gAppUseTimeService_d
static tmrTimerID_t mReadTimeCharTimerId;
#endif

static uint8_t mMeasurementSelect = 0U;
static uint8_t mCurrentUserId = 1U;

#if gAppUseTimeService_d
static uint8_t mOutCharReadBuffer[mCharReadBufferLength_c];
static uint16_t mOutCharReadByteCount;
#endif

static uint32_t localTime = mInitialTime_c;
#if gAppUseTimeService_d
static bool_t isTimeSynchronized = FALSE;
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
#if gAppUseTimeService_d
static void BleApp_GattClientCallback (deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t procedureResult, bleResult_t error);
#endif
static void BleApp_Config(void);

/* Timer Callbacks */
static void AdvertisingTimerCallback (void *pParam);
static void TimerMeasurementCallback (void *pParam);
static void BatteryMeasurementTimerCallback (void *pParam);
static void LocalTimeTickTimerCallback (void *pParam);
#if gAppUseTimeService_d
static void ReadTimeCharTimerCallback (void *pParam);
#endif

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
        if (gcBondedDevices > 0)
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
#if gBps_EnableMultiUserSupport_d
        {
            /* Scroll through User IDs*/
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                mUserData.userId = (mUserData.userId + 1U) % gBps_MaxNumOfUsers_c;
            }
        }
#endif
        break;
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

#if gAppUseTimeService_d
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
#endif

    mAdvState.advOn = FALSE;

    /* Start services */
#if gBps_EnableMultiUserSupport_d
    mUserData.userId = mCurrentUserId;
#endif
#if gBps_EnableStoredMeasurements_d
    mUserData.cMeasurements = 0;
    mUserData.pStoredMeasurements = MEM_BufferAlloc(10);
#endif
    (void)Bps_Start(&bpsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mMeasurementTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
    mLocalTimeTickTimerId = TMR_AllocateTimer();

    (void)TMR_StartLowPowerTimer(mLocalTimeTickTimerId, gTmrLowPowerIntervalMillisTimer_c,
               TmrSeconds(1), LocalTimeTickTimerCallback, NULL);
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
#if gAppUseTimeService_d
            bool_t isBonded = FALSE;

            Gap_CheckIfBonded(mPeerDeviceId, &isBonded, NULL);
            if(isBonded == FALSE)
            {
                if(isTimeSynchronized == FALSE)
                {
                    bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

                  /* Read CTS Characteristic. If the device doesn't have time services
                   gAttErrCodeAttributeNotFound_c will be received. */
                    GattClient_ReadUsingCharacteristicUuid
                    (
                        peerDeviceId,
                        gBleUuidType16_c,
                        &uuid,
                        NULL,
                        mOutCharReadBuffer,
                        13,
                        &mOutCharReadByteCount
                    );
                }
            }
#endif
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Subscribe client*/
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Bps_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start measurements */
            mMeasurementSelect = 0;
            (void)TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(1), TimerMeasurementCallback, NULL);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
#if gAppUseTimeService_d
            /* Start time readings */
            TMR_StartLowPowerTimer(mReadTimeCharTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mReadTimeCharInterval_c), ReadTimeCharTimerCallback, NULL);
#endif
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Bps_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Restart advertising*/
            BleApp_Start();

#if gAppUseTimeService_d
            TMR_StopTimer(mReadTimeCharTimerId);
            isTimeSynchronized = FALSE;
#endif
        }
        break;

        case gConnEvtEncryptionChanged_c:
        {
#if gAppUseTimeService_d
            if(isTimeSynchronized == FALSE)
              {
                  bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

                  /* Read CTS Characteristic. If the device doesn't have time services
                   gAttErrCodeAttributeNotFound_c will be received. */
                  GattClient_ReadUsingCharacteristicUuid
                  (
                      peerDeviceId,
                      gBleUuidType16_c,
                      &uuid,
                      NULL,
                      mOutCharReadBuffer,
                      13,
                      &mOutCharReadByteCount
                  );
              }
#endif
        }
        break;

        case gConnEvtPairingComplete_c:
        {
#if gAppUseTimeService_d
            if(isTimeSynchronized == FALSE)
            {
                bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

                /* Read CTS Characteristic. If the device doesn't have time services
                 gAttErrCodeAttributeNotFound_c will be received. */
                GattClient_ReadUsingCharacteristicUuid
                (
                    peerDeviceId,
                    gBleUuidType16_c,
                    &uuid,
                    NULL,
                    mOutCharReadBuffer,
                    13,
                    &mOutCharReadByteCount
                );
            }
#endif
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
    switch (pServerEvent->eventType)
    {
        case gEvtHandleValueConfirmation_c:
        {
            /* BPM received by client. Look for pending stored BPMs */
#if gBps_EnableStoredMeasurements_d
            if (mUserData.cMeasurements > 0U)
            {
                Bps_SendStoredBloodPressureMeasurement(&bpsServiceConfig);
            }
#endif
        }
        break;
        default:
            ; /* For MISRA compliance */
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
#if gAppUseTimeService_d
static void BleApp_GattClientCallback(
    deviceId_t              serverDeviceId,
    gattProcedureType_t     procedureType,
    gattProcedureResult_t   procedureResult,
    bleResult_t             error
)
{

    if (procedureResult == gGattProcError_c)
    {
        attErrorCode_t attError = (attErrorCode_t) (error & 0xFF);
        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
            //handle auth. errors
        }

        else
        {
            //characteristic not found
        }
    }
    else if (procedureResult == gGattProcSuccess_c)
    {
        switch(procedureType)
        {
            case gGattProcReadUsingCharacteristicUuid_c:
            {
                if (mOutCharReadByteCount > 2)
                {
                    ctsDayDateTime_t time;
                    uint8_t* pValue = &mOutCharReadBuffer[3];

                    time.dateTime.year          = Utils_ExtractTwoByteValue(&pValue[0]);
                    time.dateTime.month         = pValue[2];
                    time.dateTime.day           = pValue[3];
                    time.dateTime.hours         = pValue[4];
                    time.dateTime.minutes       = pValue[5];
                    time.dateTime.seconds       = pValue[6];
                    time.dayOfWeek              = pValue[7];

                    localTime = Cts_DayDateTimeToEpochTime(time);

                    isTimeSynchronized = TRUE;
                }
            }
            break;

            default:
                ; /* For MISRA compliance */
                break;
            }
    }
}
#endif

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
    bpsMeasurement_t bp;
    cuffPressureMeasurement_t cp;
    ctsDayDateTime_t timeVal;
    
    FLib_MemSet((void*)&bp, 0, sizeof(bpsMeasurement_t));
    FLib_MemSet((void*)&cp, 0, sizeof(cuffPressureMeasurement_t));

    RNG_GetRandomNo(&randomNumber);

    cp.unit = gBps_UnitInkPa_c;

#if gBps_EnableMultiUserSupport_d
    cp.userIdPresent = TRUE;
    cp.userId = mUserData.userId;
#else
    cp.userIdPresent = FALSE;
#endif

    cp.cuffPressure = 120U + ((uint16_t)randomNumber & 0x0FU);
    cp.measurementStatusPresent = TRUE;
    cp.measurementStatus = gBps_BodyMoveDuringDetection_c;
    cp.measurementStatus |= gBps_CuffFitsProperly_c;

    (void)Bps_RecordCuffPressureMeasurement((uint16_t)service_blood_pressure, &cp);

    bp.unit = gBps_UnitInkPa_c;
    bp.systolicValue = 120U + ((uint16_t)randomNumber & 0x0FU);
    bp.diastolicValue = 80U + ((uint16_t)randomNumber & 0x0FU);
    bp.meanArterialPressure = (bp.systolicValue + bp.diastolicValue * 2U)/3U;

#if gBps_EnableMultiUserSupport_d
    bp.userIdPresent = TRUE;
    bp.userId = mUserData.userId;
#else
    bp.userIdPresent = FALSE;
#endif

    bp.pulseRatePresent = FALSE;
    bp.timeStampPresent = FALSE;
    bp.measurementStatusPresent = FALSE;

    if (mMeasurementSelect == 5U)
    {
        bp.pulseRatePresent = TRUE;
        bp.pulseRate = 90U + ((uint16_t)randomNumber & 0x07U) ;

        bp.timeStampPresent = TRUE;
        timeVal = Cts_EpochToDayDateTime(localTime);
        FLib_MemCpy(&bp.timeStamp, &timeVal.dateTime, sizeof(ctsDateTime_t));

        bp.measurementStatusPresent = TRUE;
        bp.measurementStatus = gBps_BodyMoveDuringDetection_c;
        bp.measurementStatus |= gBps_CuffFitsProperly_c;
        mMeasurementSelect = 0U;
    }
    (void)Bps_RecordBloodPressureMeasurement((uint16_t)service_blood_pressure, &bp);

    mMeasurementSelect += 0x01U;
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

static void LocalTimeTickTimerCallback (void * pParam)
{
    localTime++;
}

#if gAppUseTimeService_d
static void ReadTimeCharTimerCallback (void * pParam)
{
    bleUuid_t uuid = { .uuid16 = gBleSig_CurrentTime_d };

    /* Read CTS Characteristic. If the device doesn't have time services
     gAttErrCodeAttributeNotFound_c will be received. */
    GattClient_ReadUsingCharacteristicUuid
    (
        mPeerDeviceId,
        gBleUuidType16_c,
        &uuid,
        NULL,
        mOutCharReadBuffer,
        13,
        &mOutCharReadByteCount
    );
}
#endif

/*! *********************************************************************************
* @}
********************************************************************************** */
