/*! *********************************************************************************
* \addtogroup Thermometer
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Thermometer application
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
#include "MemManager.h"

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
#include "current_time_interface.h"
#include "health_thermometer_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "health_thermometer.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/
/*! Thermometer Intermediate Temperature Update Rate in ms */
#define mIntTempUpdateRate_c 1000

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

/*! Events posted to application task from Timer callbacks */
typedef enum
{
    gAdvTmrCallbackEvent_c = 0,
    gTmrMeasurementCallbackEvent_c,
    gBattMeasurementCallbackEvent_c,
    gLocalTimeTickCallbackEvent_c,
#if gAppUseTimeService_d
    gReadTimeCharCallbackEvent_c,
#endif
} tmrCallbackEvent_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static advState_t  mAdvState;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
static htsUserData_t    mUserData;

static htsConfig_t htsServiceConfig = {(uint16_t)service_health_therm,
                                       mIntTempUpdateRate_c,
                                       gHts_UnitInCelsius_c,
                                       &mUserData};
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static uint16_t cpHandles[] = { (uint16_t)value_measure_int};

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;
static tmrTimerID_t mLocalTimeTickTimerId;
#if gAppUseTimeService_d
static tmrTimerID_t mReadTimeCharTimerId;
#endif

/* Counts number of interm temp between temp measurements */
static uint8_t mIntermediateTempCounter = 0;

#if gAppUseTimeService_d
static uint8_t mOutCharReadBuffer[mCharReadBufferLength_c];
static uint16_t mOutCharReadByteCount;
static bool_t isTimeSynchronized = FALSE;
#endif

static uint32_t localTime = mInitialTime_c;

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

/* Handles Timer callbacks */
static void BleApp_HandleTimerCallback(appCallbackParam_t pParam);

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
        {
            (void)Hts_SetMeasurementInterval(&htsServiceConfig, mIntTempUpdateRate_c);
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
* \brief        Handles the events posted from Timer callbacks to application task
*
********************************************************************************** */
static void BleApp_HandleTimerCallback(appCallbackParam_t pParam)
{
    switch((tmrCallbackEvent_t)(uint32_t)pParam)
    {
        case gAdvTmrCallbackEvent_c:
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
        break;

        case gTmrMeasurementCallbackEvent_c:
        {
            uint32_t randomNumber = 0;
            htsMeasurement_t tempMeas;
            ctsDayDateTime_t timeMeas;

            RNG_GetRandomNo(&randomNumber);

            tempMeas.tempTypePresent = TRUE;
            tempMeas.tempType = gHts_Armpit_c;

            tempMeas.timeStampPresent = TRUE;

            tempMeas.unit = gHts_UnitInCelsius_c;
            timeMeas = Cts_EpochToDayDateTime(localTime);
            FLib_MemCpy(&tempMeas.timeStamp, &timeMeas.dateTime, sizeof(ctsDateTime_t));
            tempMeas.temperature = 35U + (randomNumber & 0x07U);

            if (mIntermediateTempCounter < 4U)
            {
                (void)Hts_RecordIntermediateTemperature((uint16_t)service_health_therm, &tempMeas);
            }
            else
            {
                (void)Hts_RecordTemperatureMeasurement((uint16_t)service_health_therm, &tempMeas);
            }

            mIntermediateTempCounter += 1U;
            mIntermediateTempCounter = mIntermediateTempCounter % 5U;
        }
        break;

        case gBattMeasurementCallbackEvent_c:
        {
            basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
            (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
        }
        break;

        case gLocalTimeTickCallbackEvent_c:
        {
            localTime++;
        }
        break;

#if gAppUseTimeService_d
        case gReadTimeCharCallbackEvent_c:
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
                mCharReadBufferLength_c,
                &mOutCharReadByteCount
            );
        }
        break;
#endif

        default:
            ; /* For MISRA compliance */
        break;
    }
}

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BleApp_Config(void)
{
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
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

#if gAppUseTimeService_d
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
#endif

    mAdvState.advOn = FALSE;

    /* Start services */
    mUserData.measIntervalIndPending = FALSE;
#if gHts_EnableStoredMeasurements_d
    mUserData.cMeasurements = 0;
    mUserData.pStoredMeasurements = MEM_BufferAlloc(10);
#endif

    (void)Hts_Start(&htsServiceConfig);

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
            if(isBonded == FALSE && gAppUsePairing_d == 0)
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
            (void)Hts_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start measurements */
            (void)TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(1), TimerMeasurementCallback, NULL);

            /* The Server shall indicate the new Measurement Interval value the next time there is a connection*/
            if (mUserData.measIntervalIndPending)
            {
                Hts_SendMeasurementIntervalIndication((uint16_t)value_measure_int);
                mUserData.measIntervalIndPending = FALSE;
            }

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
            (void)Hts_Unsubscribe();

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
        case gEvtAttributeWritten_c:
        {
            if( pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_measure_int)
            {
                attErrorCode_t result;
                uint16_t    newValue = Utils_ExtractTwoByteValue(pServerEvent->eventData.attributeWrittenEvent.aValue);

                result = Hts_MeasurementIntervalWriting(&htsServiceConfig, (uint16_t)value_measure_int, newValue);

                if (result == gAttErrCodeNoError_c)
                {
                    (void)TMR_StopTimer(mMeasurementTimerId);

                    if (newValue != 0U)
                    {
                        /* Restart timer. Divide by 6 to compensate for interm temp */
                        (void)TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                            (uint32_t)newValue * 1000U / 6U, TimerMeasurementCallback, NULL);
                    }
                }

                /* Send response OTA */
                (void)GattServer_SendAttributeWrittenStatus(deviceId,
                                                      (uint16_t)value_measure_int,
                                                      (uint8_t)result);
            }
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
* \param[in]    serverDeviceId          GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error           Callback result.
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
                    uint16_t charValueLength = mOutCharReadByteCount - 3;
                    uint16_t handle = Utils_ExtractTwoByteValue(mOutCharReadBuffer+1);
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
    App_PostCallbackMessage(BleApp_HandleTimerCallback, (void*)(gAdvTmrCallbackEvent_c));
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void * pParam)
{
    App_PostCallbackMessage(BleApp_HandleTimerCallback, (void*)(gTmrMeasurementCallbackEvent_c));
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    App_PostCallbackMessage(BleApp_HandleTimerCallback, (void*)(gBattMeasurementCallbackEvent_c));
}

static void LocalTimeTickTimerCallback (void * pParam)
{
    App_PostCallbackMessage(BleApp_HandleTimerCallback, (void*)(gLocalTimeTickCallbackEvent_c));
}

#if gAppUseTimeService_d
static void ReadTimeCharTimerCallback (void * pParam)
{
    App_PostCallbackMessage(BleApp_HandleTimerCallback, (void*)(gReadTimeCharCallbackEvent_c));
}
#endif

/*! *********************************************************************************
* @}
********************************************************************************** */
