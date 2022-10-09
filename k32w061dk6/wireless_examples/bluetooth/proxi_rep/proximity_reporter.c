/*! *********************************************************************************
* \addtogroup Proximity Reporter
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the Proximity Reporter application
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
#if defined (CPU_JN518X)
#include "SerialManager.h"
#endif
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
#include "link_loss_interface.h"
#include "tx_power_interface.h"
#include "immediate_alert_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "proximity_reporter.h"

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
#if defined (CPU_JN518X)
uint8_t gAppSerMgrIf;
#endif

/* Adv State */
static advState_t  mAdvState;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};
static llsConfig_t serviceLinkLossConfig = {(uint16_t)service_link_loss, gLls_NoAlert_c};
static txsConfig_t serviceTxPowerConfig = {(uint16_t)service_tx_power, 0x00};
static iasConfig_t serviceIasConfig = {(uint16_t)service_immediate_alert, gLls_NoAlert_c};
static uint16_t cpHandles[] = { (uint16_t)value_alert_level_ctrl_point, (uint16_t)value_alert_level_lls };

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;

static tmrTimerID_t mBatteryMeasurementTimerId;

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
static void BatteryMeasurementTimerCallback (void *pParam);

static void BleApp_Advertise(void);

/* Application specific */
void AlertUI(uint8_t alertLevel);

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

#if defined (CPU_JN518X)
    /* UI */
    Serial_InitManager();

    /* Register Serial Manager interface */
    Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);

    Serial_SetBaudRate(gAppSerMgrIf, gUARTBaudRate115200_c);
#else
#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1)
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
        }
        break;
        case gKBD_EventLongPB1_c:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }
        case gKBD_EventPressPB2_c:
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
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);

    mAdvState.advOn = FALSE;

    /* Start services */

    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Start Link Loss Service*/
    (void)Lls_Start(&serviceLinkLossConfig);

    /* Start Tx Power Service*/
    (void)Txs_Start(&serviceTxPowerConfig);

    /* Start IAS */
    (void)Ias_Start(&serviceIasConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
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
            (void)Lls_Subscribe(peerDeviceId);
            (void)Txs_Subscribe(peerDeviceId);
            (void)Ias_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();
            AlertUI(gLls_NoAlert_c);

            /* Get Tx Power from controller */
            (void)Gap_ReadTxPowerLevelInConnection(peerDeviceId);

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
            (void)Lls_Unsubscribe();
            (void)Txs_Unsubscribe();
            (void)Ias_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Connection terminated without any prior warning */
            if (pConnectionEvent->eventData.disconnectedEvent.reason == gHciConnectionTimeout_c)
            {
                /* Start alerting to the previously set link loss alert level */
                llsAlertLevel_t alertLevel;
                if (gBleSuccess_c == Lls_GetAlertLevel((uint16_t)service_link_loss, &alertLevel))
                {
                    AlertUI(alertLevel);
                }
            }
            else
            {
                /* Reset link loss alert level to No Alert */
                Lls_SetAlertLevel(serviceLinkLossConfig.serviceHandle, gLls_NoAlert_c);

                /* Restart advertising */
                BleApp_Start();
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
    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            if (pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_alert_level_ctrl_point)
            {
                if (Ias_SetAlertLevel((uint16_t)service_immediate_alert, pServerEvent->eventData.attributeWrittenEvent.aValue[0]) == gBleSuccess_c)
                {
                    AlertUI(pServerEvent->eventData.attributeWrittenEvent.aValue[0]);
                }
            }
        }
        break;
#if defined CPU_JN518X
        case gEvtAttributeWritten_c:
        {
            if (pServerEvent->eventData.attributeWrittenEvent.handle == (uint16_t)value_alert_level_lls)
            {
                if (Lls_SetAlertLevel((uint16_t)service_link_loss, pServerEvent->eventData.attributeWrittenEvent.aValue[0]) == gBleSuccess_c)
                {
                    AlertUI(pServerEvent->eventData.attributeWrittenEvent.aValue[0]);
                }
                (void)GattServer_SendAttributeWrittenStatus(deviceId, (uint16_t)value_alert_level_lls, (uint8_t)gAttErrCodeNoError_c);
            }
        }
        break;
#endif
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
* \brief        Updates UI according to the alert level.
*
* \param[in]    alertLevel        Alert Level.
********************************************************************************** */
void AlertUI(uint8_t alertLevel)
{
#ifndef CPU_QN908X
    StopLed3Flashing();
    StopLed4Flashing();
    StopLed2Flashing();

    switch (alertLevel)
    {
        case gLls_NoAlert_c:
            #ifdef CPU_JN518X
            Serial_Print(gAppSerMgrIf, "\n\rNo Alert\n\r", gAllowToBlock_d);
            #endif
            Led2Off();
            break;
        case gLls_MildAlert_c:
            #ifdef CPU_JN518X
            Serial_Print(gAppSerMgrIf, "\n\rMild Alert\n\r", gAllowToBlock_d);
            #endif
            Led2On();
            break;
        case gLls_HighAlert_c:
            #ifdef CPU_JN518X
            Serial_Print(gAppSerMgrIf, "\n\rHigh Alert\n\r", gAllowToBlock_d);
            #endif
            Led2Flashing();
            break;
        default:
            ; /* For MISRA compliance */
            break;
    }
#else
    StopLed2Flashing();
    StopLed3Flashing();
    Led1Off();

    switch (alertLevel)
    {
        case gLls_NoAlert_c:
            Led1On(); //red solid
            break;
        case gLls_MildAlert_c:
            Led3Flashing(); //blue flashing
            break;
        case gLls_HighAlert_c:
            Led2Flashing(); //green flashing
            break;
        default:
            ; /* For MISRA compliance */
            break;
    }
#endif
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
