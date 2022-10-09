/*! *********************************************************************************
* \addtogroup Cycling Power Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Cycling Power Sensor application
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
#include "gatt_db_app_interface.h"
#include "ble_general.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "cycling_power_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "cycling_power_sensor.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private constants & macros
*************************************************************************************
************************************************************************************/

#define mBatteryLevelReportInterval_c   (10)        /* battery level report interval in seconds  */
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
    slowAdvState_c,
    dataAdvState_c
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

/* Connection Parameters */
static  gapConnectionParameters_t   mConnectionParams;
static  gapConnectionParameters_t   mNewConnParams;
/* Service Configuration*/
static const cpsSensorLocation_t mSensorLocations[2] = {(uint8_t)gCps_LeftCrank_c, (uint8_t)gCps_RightCrank_c};
static cpsUserData_t   mCpsUserData = {0, 0, 0, 0,
                                        gCps_CrankLength_c,
                                        gCps_ChainLength_c,
                                        gCps_ChainWeight_c,
                                        gCps_SpanLength_c,
                                        mSensorLocations, (uint8_t)sizeof(mSensorLocations)};
static cpsConfig_t cpsServiceConfig = {(uint8_t)service_cp,
                                        Cps_AllFeatures,
                                        gCps_LeaveAsDefault_c,
                                        &mCpsUserData,
                                        (uint8_t)gCps_LeftCrank_c,
                                        gCccdEmpty_c,
                                        FALSE,
                                        FALSE};

static uint16_t cpHandles[3] = { (uint16_t)value_cp_ctrl_point, (uint16_t)desc_cp_meas, (uint16_t)cccd_cp_vector};

static bool_t           basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mMeasurementTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;

static  cpsPowerVector_t mPowerVectorMeasurement;
static  cpsMeasurement_t measurement;
static  bool_t  mMeasurementSwitch = FALSE;
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
static void BleApp_Advertise(void);
static void BleApp_AdvertiseData(void);

/* Timer Callbacks */
static void AdvertisingTimerCallback (void *pParam);
static void TimerMeasurementCallback (void *pParam);
static void BatteryMeasurementTimerCallback (void *pParam);

static bool_t BleApp_AcceptConnectionParameters(gapConnectionParameters_t* pConnParams);
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
        case gKBD_EventLongPB1_c:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }
        case gKBD_EventPressPB2_c:
        {
            mMeasurementSwitch = !mMeasurementSwitch;
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

        case gAdvertisingDataSetupComplete_c:
        {
            (void)App_StartAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
        }
        break;

        case gAdvertisingParametersSetupComplete_c:
        {
            if(mAdvState.advType == dataAdvState_c)
            {
                BleApp_AdvertiseData();
            }
            else
            {
                /* Setup Advertising and scanning data */
                (void)Gap_SetAdvertisingData(&gAppAdvertisingData, &gAppScanRspData);
            }
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
    (void)Cps_Start(&cpsServiceConfig);

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mMeasurementTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will satrt after
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
            gAdvParams.advertisingType = gAdvConnectableUndirected_c;
            timeout = gFastConnWhiteListAdvTime_c;
        }
        break;
#endif
#endif
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.advertisingType = gAdvConnectableUndirected_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gFastConnAdvTime_c - gFastConnWhiteListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            gAdvParams.advertisingType = gAdvConnectableUndirected_c;
            timeout = gReducedPowerAdvTime_c;
        }
        break;

        case dataAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMaxAdvInterval_c;
            gAdvParams.advertisingType = gAdvNonConnectable_c;
            timeout = gMeasurementReportInterval;
        }
        break;

    default:
        ; /* For MISRA compliance */
        break;
    }

    /* Set advertising parameters*/
    (void)Gap_SetAdvertisingParameters(&gAdvParams);

    if (mAdvState.advType == dataAdvState_c)
    {
        return;
    }

    /* Start advertising timer */
    (void)TMR_StartLowPowerTimer(mAdvTimerId, gTmrLowPowerSecondTimer_c,
               TmrSeconds(timeout), AdvertisingTimerCallback, NULL);
}

static void BleApp_AdvertiseData(void)
{
    const uint8_t adData0[1] =  { (gapAdTypeFlags_t)(gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c) };
    uint16_t advInterval = 800;
    uint8_t  measurementData[10] = {UuidArray(gBleSig_CyclingPowerService_d),
                                    gCps_CrankRevolutionDataPresent_c, 0x00, 0, 0, 0, 0, 0, 0};
    gapAdStructure_t advScanStruct[3] = {
        {
        .length = NumberOfElements(adData0) + 1U,
        .adType = gAdFlags_c,
        .aData = (void *)adData0
        },
        {
        .adType = gAdAdvertisingInterval_c,
        .length = 3,
        .aData = (void *)&advInterval
        },
        {
        .adType = gAdServiceData16bit_c,
        .length = sizeof(measurementData) + 1U,
        .aData = measurementData
        }
    };

    /* Create measurement */
    FLib_MemCpy(&measurementData[4], &measurement.instantaneousPower, sizeof(uint16_t));

    gapAdvertisingData_t advData =
    {
        NumberOfElements(advScanStruct),
        (void *)advScanStruct
    };

    (void)Gap_SetAdvertisingData(&advData, NULL);

    /* Start advertising timer */
    (void)TMR_StartLowPowerTimer(mAdvTimerId, gTmrLowPowerSecondTimer_c,
                           gMeasurementReportInterval, AdvertisingTimerCallback, NULL);
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

            /* Keep connection data */
            FLib_MemCpy(&mConnectionParams, &pConnectionEvent->eventData.connectedEvent.connParameters, sizeof(gapConnectionParameters_t));
            FLib_MemCpy(&mNewConnParams, &pConnectionEvent->eventData.connectedEvent.connParameters, sizeof(gapConnectionParameters_t));

            /* Subscribe client*/
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Cps_Subscribe(peerDeviceId);

            cpsServiceConfig.maskContent = gCps_LeaveAsDefault_c;
            cpsServiceConfig.offsetCompensationOngoing = FALSE;

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start Measurements */
            (void)TMR_StartLowPowerTimer(mMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                                   gMeasurementReportInterval, TimerMeasurementCallback, NULL);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, gAppMaxConnections_c);
            (void)Cps_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            if (mAdvState.advOn)
            {
                (void)Gap_StopAdvertising();
                mAdvState.advOn = FALSE;
                (void)TMR_StopTimer(mAdvTimerId);
            }

            /* Restart advertising*/
            BleApp_Start();
        }
        break;

        case gConnEvtParameterUpdateComplete_c:
        {
            uint8_t status = gAttErrCodeInappropiateConnParam_c;

            mNewConnParams.connInterval = pConnectionEvent->eventData.connectionUpdateComplete.connInterval;
            mNewConnParams.connLatency = pConnectionEvent->eventData.connectionUpdateComplete.connLatency;
            mNewConnParams.supervisionTimeout = pConnectionEvent->eventData.connectionUpdateComplete.supervisionTimeout;

            if ((pConnectionEvent->eventData.connectionUpdateComplete.status == gBleSuccess_c) &&
                (cpsServiceConfig.procInProgress == TRUE) &&
                (BleApp_AcceptConnectionParameters(&mNewConnParams) ||
                cpsServiceConfig.pendingCccdFlags == gCccdEmpty_c))
            {
                (void)Gap_SaveCccd(mPeerDeviceId, (uint16_t)cccd_cp_vector, (gattCccdFlags_t)cpsServiceConfig.pendingCccdFlags);
                status = (uint8_t)gAttErrCodeNoError_c;
            }

            /* Connection parameters are updated. Finish ATT transaction */
            if (cpsServiceConfig.procInProgress == TRUE)
            {
                cpsServiceConfig.procInProgress = FALSE;
                (void)GattServer_SendAttributeWrittenStatus(mPeerDeviceId, (uint16_t)cccd_cp_vector, status);
            }
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
    uint16_t handle = pServerEvent->eventData.attributeWrittenEvent.handle;
    uint8_t status = (uint8_t)gAttErrCodeNoError_c;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        {
            switch (handle)
            {
                case (uint16_t)value_cp_ctrl_point:
                {
                    Cps_ControlPointHandler(&cpsServiceConfig, &pServerEvent->eventData.attributeWrittenEvent);
                    break;
                }
                case (uint16_t)desc_cp_meas:
                {
                    /*Refactor here*/
                    (void)GattDb_WriteAttribute(handle,
                                          pServerEvent->eventData.attributeWrittenEvent.cValueLength,
                                          pServerEvent->eventData.attributeWrittenEvent.aValue);
                    (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);

                    if(pServerEvent->eventData.attributeWrittenEvent.aValue[0] == 1U)
                    {
                        mAdvState.advType = dataAdvState_c;
                        BleApp_Advertise();
                    }
                    else
                    {
                        (void)Gap_StopAdvertising();
                        mAdvState.advOn = FALSE;
                        (void)TMR_StopTimer(mAdvTimerId);
                    }
                    break;
                }

                case (uint16_t)cccd_cp_vector:
                {
                    if (pServerEvent->eventData.attributeWrittenEvent.aValue[0] == 1U)
                    {
                        /* Check exiting connection parameters */
                        if(BleApp_AcceptConnectionParameters(&mConnectionParams))
                        {
                            (void)Gap_SaveCccd(deviceId, handle,
                                         (gattCccdFlags_t)pServerEvent->eventData.attributeWrittenEvent.aValue[0]);

                            (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
                        }
                        else
                        {
                            cpsServiceConfig.pendingCccdFlags = pServerEvent->eventData.attributeWrittenEvent.aValue[0];
                            cpsServiceConfig.procInProgress = TRUE;
                            (void)Gap_UpdateConnectionParameters(deviceId, gFastConnMinInterval_c, gFastConnMaxInterval_c,
                                                                    gFastConnSlaveLatency_c, gFastConnTimeoutMultiplier_c,
                                                                    gcConnectionEventMinDefault_c, gcConnectionEventMaxDefault_c);
                        }
                    }
                    else
                    {
                        /* Revert old connection parameters*/
                        if(mConnectionParams.connInterval == mNewConnParams.connInterval )
                        {
                            (void)Gap_SaveCccd(deviceId, handle,
                                         (gattCccdFlags_t)pServerEvent->eventData.attributeWrittenEvent.aValue[0]);

                            (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
                        }
                        else
                        {
                            cpsServiceConfig.pendingCccdFlags = pServerEvent->eventData.attributeWrittenEvent.aValue[0];
                            cpsServiceConfig.procInProgress = TRUE;
                            (void)Gap_UpdateConnectionParameters(deviceId, mConnectionParams.connInterval, mConnectionParams.connInterval,
                                                                    mConnectionParams.connLatency, gFastConnTimeoutMultiplier_c,
                                                                    gcConnectionEventMinDefault_c, gcConnectionEventMaxDefault_c);
                        }
                    }
                    break;
                }

                default:
                    ; /* For MISRA compliance */
                    break;
            }

        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            cpsServiceConfig.procInProgress = FALSE;
            break;
        }
        case gEvtError_c:
        {
            if (pServerEvent->eventData.procedureError.error == gGattClientConfirmationTimeout_c)
            {
                /* Procedure timeout at ATT */
                cpsServiceConfig.procInProgress = FALSE;
            }
            break;
        }
    default:
        ; /* For MISRA compliance */
        break;
    }
}

static bool_t BleApp_AcceptConnectionParameters(gapConnectionParameters_t* pConnParams)
{
    if (pConnParams->connInterval > gFastConnMaxInterval_c)
    {
        return FALSE;
    }
    return TRUE;
}

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void * pParam)
{
    bool_t startAdvertising = TRUE;
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

        case dataAdvState_c:
        {
            BleApp_AdvertiseData();
            startAdvertising = FALSE;
        }
        break;

        default:
            ; /* For MISRA compliance */
        break;
    }
    if(startAdvertising)
    {
        BleApp_Advertise();
    }
}

/*! *********************************************************************************
* \brief        Handles measurement timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void TimerMeasurementCallback(void * pParam)
{
    uint32_t randomNumber = 0;
    cpsMaskContent_t mask;

    if ( cpsServiceConfig.offsetCompensationOngoing)
    {
        /* Finish Offset Compensation procedure*/
        (void)Cps_FinishOffsetCompensation(&cpsServiceConfig, 32);

    }

    RNG_GetRandomNo(&randomNumber);
    mCpsUserData.cumulativeCrankRevs += (uint16_t)randomNumber & 0x0FU;
    mCpsUserData.cumulativeWheelRevs += randomNumber & 0x0FU;
    mCpsUserData.accumulatedEnergy += (uint16_t)randomNumber & 0x0FU;

    /* Map Mask Content to Measurement Flags */
    mask = (cpsServiceConfig.maskContent & gCps_TurnOffPedalPowerBalance_c) |
            ((cpsServiceConfig.maskContent & gCps_TurnOffAccumulatedTorque_c) << 1) |
            ((cpsServiceConfig.maskContent & (BIT2|BIT3|BIT4)) << 2) |
            ((cpsServiceConfig.maskContent & (BIT5|BIT6|BIT7|BIT8)) << 3);

    if (!mMeasurementSwitch)
    {
        /* Create first measurement */
        measurement.flags = gCps_PedalPowerBalancePresent_c |
                            gCps_AccumulatedTorquePresent_c |
                            gCps_WheelRevolutionDataPresent_c |
                            gCps_CrankRevolutionDataPresent_c |
                            gCps_AccumulatedEnergyPresent_c |
                            gCps_OffsetCompensationIndicator_c;


        measurement.flags &= ~mask;

        measurement.pedalPowerBalance  = 50;
        measurement.accumulatedEnergy   = mCpsUserData.accumulatedEnergy;
        measurement.cumulativeCrankRevs = mCpsUserData.cumulativeCrankRevs;
        measurement.cumulativeWheelRevs = mCpsUserData.cumulativeWheelRevs;
        measurement.lastCrankEventTime += 1024U;
        measurement.lastWheelEventTime += 1024U;
        measurement.instantaneousPower  = (int16_t)((uint16_t)(randomNumber & 0x3FU));

        /* Send measurement over-the-air */
        (void)Cps_RecordMeasurement((uint16_t)service_cp, &measurement);
    }
    else
    {
        /* Create first measurement */
        measurement.flags = gCps_ExtremeForceMagnitudesPresent_c |
                            gCps_ExtremeAnglesPresent_c |
                            gCps_TopDeadSpotAnglesPresent_c |
                            gCps_BotomDeadSpotAnglesPresent_c;

        measurement.flags &= ~mask;

        measurement.magnitude.forceMagnitude.maxForceMagnitude  = 0;
        measurement.magnitude.forceMagnitude.minForceMagnitude  = 0;
        measurement.extremeAngles.maxAngle = 0;
        measurement.extremeAngles.minAngle = 0;
        measurement.topDeadSpotAngle = 0;
        measurement.bottomDeadSpotAngle = 0;

        /* Send measurement over-the-air */
        (void)Cps_RecordMeasurement((uint16_t)service_cp, &measurement);
    }

    mPowerVectorMeasurement.flags = gCps_VectorCrankRevDataPresent_c |
                                    gCps_VectorFirstCrankMeasAnglePresent_c |
                                    gCps_VectorInstantForceMagArrayPresent_c |
                                    gCps_DirectionTangential_c;

    mPowerVectorMeasurement.crankRevCumulativeRev = mCpsUserData.cumulativeCrankRevs;
    mPowerVectorMeasurement.crankRevLastEventTime += 1024U;
    mPowerVectorMeasurement.firstCrankMeasAngle  = (mPowerVectorMeasurement.firstCrankMeasAngle + 3U) % 360U;
    mPowerVectorMeasurement.instantMagnitudeArray[0].force = (int16_t)((uint16_t)(randomNumber & 0x0CU));
    mPowerVectorMeasurement.arrayLength = 1U;

    /* Send measurement over-the-air */
    (void)Cps_RecordPowerVector((uint16_t)service_cp, &mPowerVectorMeasurement);
}

/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}

/*! *********************************************************************************
* @}
********************************************************************************** */
