/*! *********************************************************************************
* \addtogroup HID Device
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2014 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
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
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "RNG_Interface.h"
#include "sensors.h"
#include "fwk_platform_ble.h"

/* BLE Host Stack */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_database.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "hid_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"
#include "app_conn.h"
#include "app_advertiser.h"

/* Application */
#include "hid_device.h"
#include "board.h"
#include "app.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
#define AXIS_MIN  100
#define AXIS_MAX  500
#define MOUSE_STEP 10

#define mBatteryLevelReportInterval_c   (10U)        /* battery level report interval in seconds  */
#define mHidReportInterval_c            (400U)       /* HID level report interval in msec  */

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
#define mSwiftPairTimeout_c             (40U)        /* Swift Pair advertising timeout in seconds */
#endif /* gSwiftPairMode_d */

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef enum
{
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
    fastFilterAcceptListAdvState_c,
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
    fastAdvState_c,
    slowAdvState_c
}advType_t;

typedef struct advState_tag{
    bool_t      advOn;
    advType_t   advType;
}advState_t;

typedef struct mouseHidReport_tag{
  uint8_t buttonStatus;
  int8_t xAxis;
  int8_t yAxis;
}mouseHidReport_t;
/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Adv State */
static advState_t  mAdvState;
static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Advertising parameters */
static appAdvertisingParams_t appAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
static appAdvertisingParams_t appSwiftPairAdvParams = {
    &gAdvParams,
    &gAppSwiftPairAdvertisingData,
    NULL
};
#endif /* gSwiftPairMode_d */

/* Service Data*/
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};
static hidConfig_t hidServiceConfig = {(uint16_t)service_hid, (uint8_t)gHid_ReportProtocolMode_c};
static uint16_t cpHandles[] = { (uint16_t)value_hid_control_point };

/* Application specific data*/
static TIMER_MANAGER_HANDLE_DEFINE(mAdvTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mHidDemoTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mBatteryMeasurementTimerId);

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
static TIMER_MANAGER_HANDLE_DEFINE(mSwiftPairTimerId);
static bool_t mSwiftModeActive = FALSE;
#endif /* gSwiftPairMode_d */

static int16_t xAxis = AXIS_MIN;
static int16_t yAxis = AXIS_MIN;

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent);

/* Timer Callbacks */
static void AdvertisingTimerCallback (void * pParam);
static void TimerHidMouseCallback (void * pParam);
static void BatteryMeasurementTimerCallback (void * pParam);

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
static void SwiftPairTimerCallback(void * pParam);
#endif /* gSwiftPairMode_d */

static void BleApp_Advertise(void);
static void BluetoothLEHost_Initialized(void);

/* Mouse events */
static void SendReport(mouseHidReport_t * pReport);
static void MoveMouseLeft(int8_t pixels);
static void MoveMouseRight(int8_t pixels);
static void MoveMouseUp(int8_t pixels);
static void MoveMouseDown(int8_t pixels);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /* gAppButtonCnt_c > 1 */
#endif /* gAppButtonCnt_c > 0 */

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the bluetooth demo
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{   
    LedStartFlashingAllLeds();

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /* gAppButtonCnt_c > 1 */
#endif /* gAppButtonCnt_c > 0 */

    /* Set generic callback */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
void BleApp_Start(void)
{
    if ((mPeerDeviceId == gInvalidDeviceId_c) && (mAdvState.advOn == FALSE))
    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        if (gcBondedDevices > 0U)
        {
            mAdvState.advType = fastFilterAcceptListAdvState_c;
        }
        else
        {
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
            mAdvState.advType = fastAdvState_c;
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        }
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
        BleApp_Advertise();
    }
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            BleApp_Start();
            break;
        }

        case kBUTTON_EventLongPress:
        {
            if (mPeerDeviceId != gInvalidDeviceId_c)
            {
                (void)Gap_Disconnect(mPeerDeviceId);
            }
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
    
    return kStatus_BUTTON_Success; 
}

button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    switch (message->event)
    {
         case kBUTTON_EventOneClick:
         case kBUTTON_EventShortPress:
        {
            hidProtocolMode_t protocolMode = 0xFFU;

            /* Toggle Protocol Mode */
            (void)Hid_GetProtocolMode((uint16_t)service_hid, &protocolMode);
            protocolMode = (protocolMode == gHid_BootProtocolMode_c)?gHid_ReportProtocolMode_c:gHid_BootProtocolMode_c;
            (void)Hid_SetProtocolMode((uint16_t)service_hid, protocolMode);
            break;
        }

        default:
        {
            ; /* No action required */
            break;
        }
    }
    
    return kStatus_BUTTON_Success; 
}
#endif /* gAppButtonCnt_c */

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);
    (void)GattServer_RegisterHandlesForWriteNotifications(NumberOfElements(cpHandles), cpHandles);

    mAdvState.advOn = FALSE;

    /* Start services */
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);
    (void)Hid_Start(&hidServiceConfig);

    /* Allocate application timers */
    (void)TM_Open(mAdvTimerId);
    (void)TM_Open(mHidDemoTimerId);
    (void)TM_Open(mBatteryMeasurementTimerId);

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
    (void)TM_Open(mSwiftPairTimerId);

    mSwiftModeActive = TRUE;

    /* If Swift Pair is enabled, start advertising instantly */
    BleApp_Start();

    /* Start Swift Pair timer */
    (void)TM_InstallCallback((timer_handle_t)mSwiftPairTimerId, SwiftPairTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mSwiftPairTimerId,
                   kTimerModeLowPowerTimer      |
                   kTimerModeSetSecondTimer     |
                   kTimerModeIntervalTimer,
                   mSwiftPairTimeout_c);
#endif /* gSwiftPairMode_d */
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
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        case fastFilterAcceptListAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessFilterAcceptListOnly_c;
            timeout = gFastConnFilterAcceptListAdvTime_c;
        }
        break;
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
        case fastAdvState_c:
        {
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
            if (mSwiftModeActive == FALSE)
            {
#endif /* gSwiftPairMode_d */
                gAdvParams.minInterval = gFastConnMinAdvInterval_c;
                gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
                gAdvParams.filterPolicy = gProcessAll_c;
                timeout = gFastConnAdvTime_c - gFastConnFilterAcceptListAdvTime_c;
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
            }
            else
            {
                gAdvParams.minInterval = gSwiftPairFastConnMinAdvInterval_c;
                gAdvParams.maxInterval = gSwiftPairFastConnMaxAdvInterval_c;
                gAdvParams.filterPolicy = gProcessAll_c;
                timeout = gFastConnAdvTime_c;
            }
#endif /* gSwiftPairMode_d */
        }
        break;

        case slowAdvState_c:
        {
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
            if (mSwiftModeActive == FALSE)
            {
#endif /* gSwiftPairMode_d */
                gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
                gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
                gAdvParams.filterPolicy = gProcessAll_c;
                timeout = gReducedPowerAdvTime_c;
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
            }
            else
            {
                gAdvParams.minInterval = gSwiftPairReducedPowerMinAdvInterval_c;
                gAdvParams.maxInterval = gSwiftPairReducedPowerMinAdvInterval_c;
                gAdvParams.filterPolicy = gProcessAll_c;
                timeout = gReducedPowerAdvTime_c;
            }
#endif /* gSwiftPairMode_d */
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
        }
        break;
    }

    /* Set advertising parameters, data and start advertising */
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
    if (mSwiftModeActive == FALSE)
    {
#endif /* gSwiftPairMode_d */
        (void)BluetoothLEHost_StartAdvertising(&appAdvParams,BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
    }
    else
    {
        (void)BluetoothLEHost_StartAdvertising(&appSwiftPairAdvParams,BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
    }
#endif /* gSwiftPairMode_d */

    /* Start advertising timer */
    (void)TM_InstallCallback((timer_handle_t)mAdvTimerId, AdvertisingTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mAdvTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, timeout);
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
            /* UI */

            /* Turn off all LEDs */
            LedStopFlashingAllLeds();

#if (gAppLedCnt_c == 1)
            LedSetColor(0, kLED_Blue);
#endif /* gAppLedCnt_c */

            /* Start LED1 */
            Led1Flashing();

            if(!mAdvState.advOn)
            {
#if (gAppLedCnt_c == 1)
                LedSetColor(0, kLED_White);
                Led1Flashing();
#else /* gAppLedCnt_c */
                Led2Flashing();
#endif /* gAppLedCnt_c */
            }
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

            /* Turn off all LEDs */
            LedStopFlashingAllLeds();

#if (gAppLedCnt_c == 1)
            LedSetColor(0, kLED_White);
#endif /* gAppLedCnt_c */

            /* Turn on LED1 */
            Led1On();

            /* Stop Advertising Timer*/
            (void)TM_Stop((timer_handle_t)mAdvTimerId);

            /* Start HID demo */
            (void)TM_InstallCallback((timer_handle_t)mHidDemoTimerId, TimerHidMouseCallback, NULL);
            (void)TM_Start((timer_handle_t)mHidDemoTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSingleShot, mHidReportInterval_c);

            /* Start battery measurements */
            (void)TM_InstallCallback((timer_handle_t)mBatteryMeasurementTimerId, BatteryMeasurementTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mBatteryMeasurementTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, mBatteryLevelReportInterval_c);
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
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void * pParam)
{
    /* Stop and restart advertising with new parameters */
    (void)Gap_StopAdvertising();

    switch (mAdvState.advType)
    {
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
        case fastFilterAcceptListAdvState_c:
        {
            mAdvState.advType = fastAdvState_c;
        }
        break;
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
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
* \param[in]    pParam        Callback parameters.
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
    (void)TM_InstallCallback((timer_handle_t)mHidDemoTimerId, TimerHidMouseCallback, NULL);
    (void)TM_Start((timer_handle_t)mHidDemoTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSingleShot, mHidReportInterval_c);
}


/*! *********************************************************************************
* \brief        Handles battery measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}

#if (defined(gSwiftPairMode_d) && (gSwiftPairMode_d == 1))
/*! *********************************************************************************
* \brief        Handles Swift Pair timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void SwiftPairTimerCallback(void * pParam)
{
    if (mSwiftModeActive == TRUE)
    {
        /* Exit Swift Pair mode */
        mSwiftModeActive = FALSE;

        /* Remove Swift Pair sections from advertising data */
        (void)Gap_SetAdvertisingData(&gAppAdvertisingData, NULL);
    }
    else
    {
        /* Stop advertising */
        (void)Gap_StopAdvertising();
        
        /* Stop Swift Pair timer */
        (void)TM_Stop((timer_handle_t)mSwiftPairTimerId);
    }
}
#endif /* gSwiftPairMode_d */

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
        (void)Hid_SendBootMouseInputReport(hidServiceConfig.serviceHandle, (uint16_t)sizeof(mouseHidReport_t), pReport);
    }
    else if (protocolMode == gHid_ReportProtocolMode_c)
    {
        (void)Hid_SendInputReport(hidServiceConfig.serviceHandle, (uint16_t)sizeof(mouseHidReport_t), pReport);
    }
    else
    {
        ; /* For MISRA Compliance */
    }
}

static void MoveMouseLeft(int8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.xAxis = -pixels;
    SendReport(&mouseReport);
}

static void MoveMouseRight(int8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.xAxis = pixels;
    SendReport(&mouseReport);
}

static void MoveMouseUp(int8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.yAxis = -pixels;
    SendReport(&mouseReport);
}

static void MoveMouseDown(int8_t pixels)
{
    mouseHidReport_t mouseReport = {0,0,0};
    mouseReport.yAxis = pixels;
    SendReport(&mouseReport);
}

/*! *********************************************************************************
* @}
********************************************************************************** */
