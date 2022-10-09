/*! *********************************************************************************
* \addtogroup HID Device
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the HID Device application
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
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
#include "dynamic_gatt_database.h"
#else
#include "gatt_db_handles.h"
#endif

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "hid_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "ApplMain.h"
#include "hid_device.h"

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
#include "erpc_host.h"
#include "dynamic_gatt_database.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define AXIS_MIN  100U
#define AXIS_MAX  500U
#define MOUSE_STEP 10U

#define mBatteryLevelReportInterval_c   (10U)        /* battery level report interval in seconds  */
#define mHidReportInterval_c   (400U)        /* battery level report interval in msec  */

#if gAppUsePrivacy_d
#define mPrivacyDisableDurationSec_c    (30U) /* timeout for re-enabling privacy in seconds */
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

#if gAppUsePrivacy_d
typedef enum
{
    reqDisabled_c,
    reqOn_c,
    reqOff_c
}privacyReqType_t;
#endif

typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
}advState_t;

typedef struct mouseHidReport_tag{
  uint8_t buttonStatus;
  uint8_t xAxis;
  uint8_t yAxis;
}mouseHidReport_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static advState_t  mAdvState;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Service Data*/
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};
static hidConfig_t hidServiceConfig = {(uint16_t)service_hid, (uint8_t)gHid_ReportProtocolMode_c};
static uint16_t cpHandles[] = { (uint16_t)value_hid_control_point };

/* Application specific data*/
static tmrTimerID_t mAdvTimerId;
static tmrTimerID_t mHidDemoTimerId;
static tmrTimerID_t mBatteryMeasurementTimerId;
#if gAppUsePrivacy_d
static tmrTimerID_t mPrivacyDisableTimerId;
static privacyReqType_t mAppPrivacyChangeReq = reqDisabled_c;
#endif

static uint16_t xAxis = AXIS_MIN;
static uint16_t yAxis = AXIS_MIN;

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
static void AdvertisingTimerCallback (void * pParam);
static void TimerHidMouseCallback (void * pParam);
static void BatteryMeasurementTimerCallback (void * pParam);
#if gAppUsePrivacy_d
static void PrivacyEnableTimerCallback (void * pParam);
static void BleApp_PrivacyEnable (void * pParam);
#endif

static void BleApp_Advertise(void);

/* Mouse events */
static void SendReport(mouseHidReport_t * pReport);
static void MoveMouseLeft(uint8_t pixels);
static void MoveMouseRight(uint8_t pixels);
static void MoveMouseUp(uint8_t pixels);
static void MoveMouseDown(uint8_t pixels);

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

#if defined(MULTICORE_APPLICATION_CORE) && (MULTICORE_APPLICATION_CORE == 1U)
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
            hidProtocolMode_t protocolMode = 0xFFU;

            /* Toggle Protocol Mode */
            (void)Hid_GetProtocolMode((uint16_t)service_hid, &protocolMode);
            protocolMode = (protocolMode == gHid_BootProtocolMode_c)?gHid_ReportProtocolMode_c:gHid_BootProtocolMode_c;
            (void)Hid_SetProtocolMode((uint16_t)service_hid, protocolMode);
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
        {
#if gAppUsePrivacy_d
            if( mAdvState.advOn )
            {
                mAppPrivacyChangeReq = reqOff_c;
                /* Stop Advertising Timer*/
                TMR_StopTimer(mAdvTimerId);
                Gap_StopAdvertising();
            }
            else if( gBleSuccess_c == BleConnManager_DisablePrivacy() )
            {
                TMR_StartLowPowerTimer(mPrivacyDisableTimerId, gTmrLowPowerSingleShotMillisTimer_c,
                    TmrSeconds(mPrivacyDisableDurationSec_c), PrivacyEnableTimerCallback, NULL);
            }
#endif
            break;
        }
        default:
            ; /* For MISRA Compliance */
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

#if gAppUsePrivacy_d
        case gControllerPrivacyStateChanged_c:
        {
            if( mPeerDeviceId == gInvalidDeviceId_c )
            {
                BleApp_Start();
            }
            break;
        }
#endif

        default:
            ; /* For MISRA Compliance */
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


    mAdvState.advOn = FALSE;

    /* Start services */
    basServiceConfig.batteryLevel = BOARD_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);
    (void)Hid_Start(&hidServiceConfig);

    /* Allocate application timers */
    mAdvTimerId = TMR_AllocateTimer();
    mHidDemoTimerId = TMR_AllocateTimer();
    mBatteryMeasurementTimerId = TMR_AllocateTimer();

#if gAppUsePrivacy_d
    mPrivacyDisableTimerId = TMR_AllocateTimer();
#endif
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
        {
            ; /* For MISRA Compliance */
        }
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

#if gAppUsePrivacy_d
            if( !mAdvState.advOn )
            {
                if( reqOff_c == mAppPrivacyChangeReq )
                {
                    if( gBleSuccess_c == BleConnManager_DisablePrivacy() )
                    {
                        TMR_StartLowPowerTimer(mPrivacyDisableTimerId, gTmrLowPowerSingleShotMillisTimer_c,
                            TmrSeconds(mPrivacyDisableDurationSec_c), PrivacyEnableTimerCallback, NULL);
                    }
                    mAppPrivacyChangeReq = reqDisabled_c;
                }
                else if( reqOn_c == mAppPrivacyChangeReq )
                {
                    BleConnManager_EnablePrivacy();
                    mAppPrivacyChangeReq = reqDisabled_c;
                }
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
            ; /* For MISRA Compliance */
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
            (void)Hid_Subscribe(peerDeviceId);

            /* UI */
            LED_StopFlashingAllLeds();
            Led1On();

            /* Stop Advertising Timer*/
            (void)TMR_StopTimer(mAdvTimerId);

            /* Start HID demo */
            (void)TMR_StartLowPowerTimer(mHidDemoTimerId, gTmrLowPowerSingleShotMillisTimer_c,
                       mHidReportInterval_c, TimerHidMouseCallback, NULL);

            /* Start battery measurements */
            (void)TMR_StartLowPowerTimer(mBatteryMeasurementTimerId, gTmrLowPowerIntervalMillisTimer_c,
                       TmrSeconds(mBatteryLevelReportInterval_c), BatteryMeasurementTimerCallback, NULL);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Hid_Unsubscribe();

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Restart advertising */
            BleApp_Start();
        }
        break;
    default:
        ; /* For MISRA Compliance */
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

            if (handle == (uint16_t)value_hid_control_point)
            {
                status = Hid_ControlPointHandler((uint16_t)service_hid, pServerEvent->eventData.attributeWrittenEvent.aValue[0]);
            }
            (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, status);
        }
        break;
    default:
        ; /* For MISRA Compliance */
        break;
    }
}


/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Calback parameters.
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
            ; /* For MISRA Compliance */
        break;
    }
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Handles HID Mouse timer callback.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void TimerHidMouseCallback(void * pParam)
{
    if ((xAxis < AXIS_MAX) && (yAxis == AXIS_MIN))
    {
        MoveMouseRight(MOUSE_STEP);
        xAxis += MOUSE_STEP;
    }

    if ((xAxis == AXIS_MAX) && (yAxis < AXIS_MAX))
    {
        MoveMouseDown(MOUSE_STEP);
        yAxis += MOUSE_STEP;
    }

    if ((xAxis > AXIS_MIN) && (yAxis == AXIS_MAX))
    {
        MoveMouseLeft(MOUSE_STEP);
        xAxis -= MOUSE_STEP;
    }

    if ((xAxis == AXIS_MIN) && (yAxis > AXIS_MIN))
    {
        MoveMouseUp(MOUSE_STEP);
        yAxis -= MOUSE_STEP;
    }

    /* Start measurements */
    (void)TMR_StartLowPowerTimer(mHidDemoTimerId, gTmrLowPowerSingleShotMillisTimer_c,
           mHidReportInterval_c, TimerHidMouseCallback, NULL);
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

#if gAppUsePrivacy_d
/*! *********************************************************************************
* \brief        Posts a message to app task to re-enable privacy.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void PrivacyEnableTimerCallback(void * pParam)
{
    App_PostCallbackMessage(BleApp_PrivacyEnable, NULL);
}

/*! *********************************************************************************
* \brief        Handles re-enablement of privacy.
*
* \param[in]    pParam        Calback parameters.
********************************************************************************** */
static void BleApp_PrivacyEnable(void * pParam)
{
    if( mAdvState.advOn )
    {
        mAppPrivacyChangeReq = reqOn_c;
        /* Stop Advertising Timer*/
        TMR_StopTimer(mAdvTimerId);
        Gap_StopAdvertising();
    }
    else
    {
        BleConnManager_EnablePrivacy();
    }
}
#endif

/*! *********************************************************************************
* \brief        Sends HID Report Over-the-Air.
*
* \param[in]    pReport        Pointer to mouseHidReport_t.
********************************************************************************** */
static void SendReport(mouseHidReport_t *pReport)
{
    hidProtocolMode_t protocolMode = 0xFFU;

    /* Toggle Protocol Mode */
    (void)Hid_GetProtocolMode((uint16_t)service_hid, &protocolMode);

    if (protocolMode == gHid_BootProtocolMode_c)
    {
        (void)Hid_SendBootMouseInputReport(hidServiceConfig.serviceHandle, sizeof(mouseHidReport_t), pReport);
    }
    else if (protocolMode == gHid_ReportProtocolMode_c)
    {
        (void)Hid_SendInputReport(hidServiceConfig.serviceHandle, sizeof(mouseHidReport_t), pReport);
    }
    else
    {
        ; /* For MISRA Compiance */
    }
}

static void MoveMouseLeft(uint8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.xAxis = (uint8_t)(-pixels);
    SendReport(&mouseReport);
}

static void MoveMouseRight(uint8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.xAxis = pixels;
    SendReport(&mouseReport);
}

static void MoveMouseUp(uint8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.yAxis = (uint8_t)(-pixels);
    SendReport(&mouseReport);
}

static void MoveMouseDown(uint8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.yAxis = pixels;
    SendReport(&mouseReport);
}

/*! *********************************************************************************
* @}
********************************************************************************** */
