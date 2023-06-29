/*! *********************************************************************************
* \addtogroup Glucose Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Glucose Sensor application
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

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "current_time_interface.h"
#include "glucose_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "glucose_sensor.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define mBatteryLevelReportInterval_c   (10U)            /* battery level report interval in seconds  */
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
static glsUserData_t        mUserData;
static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static glsConfig_t glsServiceConfig = {(uint16_t)service_glucose, Gls_AllFeatures,
                                        &mUserData};
static uint16_t cpHandles[] = { (uint16_t)value_ra_ctrl_point };

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;
static tmrTimerID_t mLocalTimeTickTimerId;

static bool_t mToggleUnits = FALSE;

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
static void BatteryMeasurementTimerCallback (void *pParam);
static void LocalTimeTickTimerCallback (void *pParam);

static void BleApp_TakeMeasurement(void);
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
            if(gInvalidDeviceId_c == mPeerDeviceId)
            {
                BleApp_Start();
            }
            else
            {
                BleApp_TakeMeasurement();
            }
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
        case gKBD_EventVeryLongPB1_c:
        {
            mToggleUnits = !mToggleUnits;
            break;
        }
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

#if gAppUseTimeService_d
    App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
#endif

    mAdvState.advOn = FALSE;

    /* Start services */
    mUserData.cMeasurements = 0;
    mUserData.pStoredMeasurements = MEM_BufferAlloc(gGls_MaxNumOfStoredMeasurements_c * sizeof(glsFullMeasurement_t));
    (void)Gls_Start(&glsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
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
            (void)Gls_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Gls_Unsubscribe(&glsServiceConfig);

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Restart advertising*/
            BleApp_Start();
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
    uint16_t handle;
    uint8_t status;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;
            status = (uint8_t)gAttErrCodeNoError_c;

            if (handle == (uint16_t)value_ra_ctrl_point)
            {
                Gls_ControlPointHandler(&glsServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
            }
            else
            {
                (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
            }
            break;
        }
        case gEvtHandleValueConfirmation_c:
        {
            glsServiceConfig.procInProgress = FALSE;
            break;
        }
        case gEvtError_c:
        {
            if (pServerEvent->eventData.procedureError.error == gGattClientConfirmationTimeout_c)
            {
                /* Procedure timeout at ATT */
                glsServiceConfig.procInProgress = FALSE;
            }
            break;
        }
    default:
        ; /* For MISRA compliance */
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId          GATT Server device ID.
* \param[in]    procedureType    	Procedure type.
* \param[in]    procedureResult    	Procedure result.
* \param[in]    error    		Callback result.
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

static void BleApp_TakeMeasurement(void)
{
    uint32_t randomNumber = 0;
    glsFullMeasurement_t measurement;
    ctsDayDateTime_t timeValue;

    RNG_GetRandomNo(&randomNumber);
    randomNumber = randomNumber & 0xFFU;

    measurement.flags = gGls_TimeOffsetPresent_c | gGls_ConcTypeSampleLocPresent_c |\
        gGls_UnitInMolpL_c | gGls_SensorStatusPresent_c |\
        gGls_ContextInfoFollows_c | gGls_SensorStatusPresent_c;

    measurement.typeAndSampleLoc.type = gGls_TypeVenousWholeBlood_c;
    measurement.typeAndSampleLoc.sampleLoc = gGls_LocFinger_c;
    measurement.sensorStatusAnnce = gGls_NoAnnce_c;
    measurement.seqNumber = (glsServiceConfig.pUserData->lastSeqNumber)++;

    timeValue = Cts_EpochToDayDateTime(localTime);
    FLib_MemCpy(&measurement.dateTime, &timeValue.dateTime, sizeof(ctsDateTime_t));

    measurement.timeOffset = 10;
    measurement.glucCon = 0xD050U + (uint16_t)randomNumber;

    /* Context Extension */
    measurement.contextFlags = gGls_CarbPresent_c | gGls_MealPresent_c | gGls_TesterHealthPresent_c |\
        gGls_ExerciseDurationIntesityPresent_c | gGls_MedicationPresent_c|\
        gGls_HbA1cPresent_c | gGls_ExtendedFlagsPresent_c;

    measurement.extendedFlags = 0;
    measurement.carbId = gGls_CarbIdBreakfast_c;
    measurement.carbUnits = 0xD040U + (uint16_t)randomNumber;
    measurement.meal = gGls_MealPreprandial_c;
    measurement.testerHealth.tester = gGls_TesterSelf_c;
    measurement.testerHealth.health = gGls_HealthNoIssues_c;
    measurement.exerciseDuration = 0x000AU;
    measurement.exerciseIntensity = 0x02U;
    measurement.medicationId = gGls_MedicationIdRapidActingInsulin_c;

    measurement.hbA1c = 0x0060U;

    if (mToggleUnits)
    {
        measurement.contextFlags |= gGls_MedicationUnitsInKg_c;
        measurement.medicationUnits = 0xA020U;
    }
    else
    {
        measurement.contextFlags |= gGls_MedicationUnitsInLitres_c;
        measurement.medicationUnits = 0xD020U;
    }


    (void)Gls_RecordGlucoseMeasurement(&glsServiceConfig, &measurement);
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

/*! *********************************************************************************
* @}
********************************************************************************** */
