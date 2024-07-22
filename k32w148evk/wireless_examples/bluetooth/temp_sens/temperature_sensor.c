/*! *********************************************************************************
* \addtogroup Temperature Sensor
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
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
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_mem_manager.h"
#include "fsl_component_panic.h"
#include "fsl_component_serial_manager.h"
#include "FunctionLib.h"
#if defined(K32W232H_SERIES) || defined(KW45B41Z82_SERIES) || \
    defined(KW45B41Z83_SERIES) || defined(K32W1480_SERIES) || defined(MCXW345_SERIES) || \
    defined(MCXW716C_SERIES) || defined(MCXW716A_SERIES)
#include "sensors.h"
#endif

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"


/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "temperature_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "app.h"
#include "app_conn.h"
#include "app_advertiser.h"
#include "temperature_sensor.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

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
static TIMER_MANAGER_HANDLE_DEFINE(appTimerId);

/*temp sensor serial manager handle*/
static serial_handle_t gAppSerMgrIf;
/*temp sensor write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_writeHandle);

static appAdvertisingParams_t mAppAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};
/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);

/* Timer Callbacks */
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
static void AdvertisingTimerCallback(void *pParam);
static void DisconnectTimerCallback(void *pParam);
#endif

static void BleApp_Advertise(void);
static void BleApp_SendTemperature(void);

static void AppPrintString( const char* pBuff);
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent);
#endif
static void BluetoothLEHost_Initialized(void);
static void BleApp_SerialInit(void);
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent);
static void BleApp_Start(void);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam);
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
    /* Initialize application support for drivers */
    BleApp_SerialInit();
    LedStartFlashingAllLeds();
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 1))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys0, NULL);
#endif
#endif
    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_SetGenericCallback(BluetoothLEHost_GenericCallback);
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    (void)buttonHandle;
    (void)callbackParam;
    (void)message;
    /* Start application on any key */
    BleApp_Start();
    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
static void BleApp_Start(void)
{
    Led1On();

    if (mPeerDeviceId == gInvalidDeviceId_c)
    {
        /* Device is not connected and not advertising */
        if (!mAdvState.advOn)
        {
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
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, filter accept list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);

    mAdvState.advOn = FALSE;

    /* Start services */
    SENSORS_TriggerTemperatureMeasurement();
    (void)SENSORS_RefreshTemperatureValue();
    /* Multiply temperature value by 10. SENSORS_GetTemperature() reports temperature
    value in tenths of degrees Celsius. Temperature characteristic value is degrees
    Celsius with a resolution of 0.01 degrees Celsius (GATT Specification
    Supplement v6). */
    tmsServiceConfig.initialTemperature = (int16_t)(10 * SENSORS_GetTemperature());
    (void)Tms_Start(&tmsServiceConfig);

    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timer */
    (void)TM_Open(appTimerId);

    AppPrintString("\r\nTemperature sensor -> Press switch to start advertising.\r\n");
}

/*! *********************************************************************************
* \brief        Configures GAP Advertise parameters. Advertise will start after
*               the parameters are set.
*
********************************************************************************** */
static void BleApp_Advertise(void)
{
    /* Start advertising */
    (void)BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
}

/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BluetoothLEHost_GenericCallback (gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
    case gAdvertisingSetupFailed_c:
        {
            panic(0,0,0,0);
        }
        break;
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
    case gLePhyEvent_c:
        if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
        {
            AppPrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
        }
        break;
#endif
    default:
        {
            ; /* No action required */
        }
        break;
    }
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

            if(!mAdvState.advOn)
            {
                AppPrintString("Advertising stopped.\r\n");
                /* UI */
                LedStopFlashingAllLeds();
                LedStartFlashingAllLeds();
            }
            else
            {
                AppPrintString("Advertising started.\r\n");

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
                /* Start advertising timer */
                (void)TM_InstallCallback((timer_handle_t)appTimerId, AdvertisingTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)appTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gAdvTime_c);

#endif /* gAppLowpowerEnabled_d */
                /* UI */
                LedStopFlashingAllLeds();
                Led1Flashing();
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
    /* Connection Manager to handle Host Stack interactions */
    BleConnManager_GapPeripheralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;
            (void)TM_Stop((timer_handle_t)appTimerId);

            /* Subscribe client*/
            mPeerDeviceId = peerDeviceId;
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Subscribe(peerDeviceId);

            AppPrintString("Connected!\r\n");
            LedStopFlashingAllLeds();

            /* UI */
            Led1On();
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            mPeerDeviceId = gInvalidDeviceId_c;
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)Tms_Unsubscribe();

            AppPrintString("Disconnected!\r\n");

#if !defined(gAppLowpowerEnabled_d) || (gAppLowpowerEnabled_d == 0U)
            /* restart advertising*/
            BleApp_Start();
#endif
        }
        break;

        case gConnEvtEncryptionChanged_c:   /* Fall-through */
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

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
/*! *********************************************************************************
* \brief        Stops advertising when the application timeout has expired.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void* pParam)
{
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
    /* Terminate connection */
    if (mPeerDeviceId != gInvalidDeviceId_c)
    {
        (void)Gap_Disconnect(mPeerDeviceId);
    }
}
#endif /* gAppLowpowerEnabled_d */

/*! *********************************************************************************
* \brief        Sends temperature value over-the-air.
*
********************************************************************************** */
static void BleApp_SendTemperature(void)
{
    (void)TM_Stop((timer_handle_t)appTimerId);

    /* Update with initial temperature */
    SENSORS_TriggerTemperatureMeasurement();
    (void)SENSORS_RefreshTemperatureValue();

    /* Multiply temperature value by 10. SENSORS_GetTemperature() reports temperature value in tenths of degrees Celsius.
    Temperature characteristic value is degrees Celsius with a resolution of 0.01 degrees Celsius (GATT Specification
    Supplement v6). */
    (void)Tms_RecordTemperatureMeasurement((uint16_t)service_temperature,
                                           (int16_t)(SENSORS_GetTemperature() * 10));

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d > 0U)
    /* Start Sleep After Data timer */
    (void)TM_InstallCallback((timer_handle_t)appTimerId, DisconnectTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)appTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, gGoToSleepAfterDataTime_c);
#endif
}

/*! *********************************************************************************
* \brief        Prints a string.
*
********************************************************************************** */
static void AppPrintString( const char* pBuff)
{
    uint32_t buffLength = strlen(pBuff);
    union{
        const char* pcBuff;
        uint8_t* pBuff;
    }buff;
    buff.pcBuff = pBuff;
    (void)SerialManager_WriteBlocking((serial_write_handle_t)s_writeHandle, buff.pBuff, buffLength );
}
/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void PrintLePhyEvent(void(*pfPrint)(const char *pBuff),gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static const char* mLePhyModeStrings[] =
    {
        "Invalid\r\n",
        "1M\r\n",
        "2M\r\n",
        "Coded\r\n",
    };
    uint8_t txPhy = (pPhyEvent->txPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->txPhy : 0U;
    uint8_t rxPhy = (pPhyEvent->rxPhy <= (uint8_t)gLePhyCoded_c) ? pPhyEvent->rxPhy : 0U;
    pfPrint("Phy Update Complete.\r\n");
    pfPrint("TxPhy ");
    pfPrint(mLePhyModeStrings[txPhy]);
    pfPrint("RxPhy ");
    pfPrint(mLePhyModeStrings[rxPhy]);
}
#endif

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
#if defined(gAppPrintLePhyEvent_c) && (gAppPrintLePhyEvent_c)
static void AppPrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    PrintLePhyEvent(AppPrintString, pPhyEvent);
}
#endif

/*! *********************************************************************************
* \brief        Initializes serial interface.
*
********************************************************************************** */
static void BleApp_SerialInit(void)
{
    serial_manager_status_t status;

    /* UI */
    gAppSerMgrIf = (serial_handle_t)&gSerMgrIf[0];

    /*open wireless uart write handle*/
    status = SerialManager_OpenWriteHandle((serial_handle_t)gAppSerMgrIf, (serial_write_handle_t)s_writeHandle);
    assert(kStatus_SerialManager_Success == status);
    (void)status;
}
/*! *********************************************************************************
* @}
********************************************************************************** */
