/*! *********************************************************************************
* \addtogroup EATT Peripheral
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2021-2024 NXP
*
*
* \file
*
* This file is the source file for the EATT peripheral application
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
#include "fsl_format.h"
#include "FunctionLib.h"
#include "sensors.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gatt_db_app_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"
#include "board.h"
#include "app_conn.h"
#include "app_advertiser.h"
#include "app.h"
#include "eatt_peripheral.h"

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************/
#define mMaxCbCredits_c                 (65535U)    /* maximum number of credits for an L2CAP channel */

#define mBatteryLevelReportInterval_c   (10U)       /* battery level report interval in seconds  */

#define mServiceAReportInterval_c       (5U)        /* service A report interval in seconds  */
#define mServiceBReportInterval_c       (5U)        /* service B report interval in seconds  */

#define mServiceABearerIdx              (0U)        /* index of the bearerId value to be used to send Service A data - first available bearerId */
#define mServiceBBearerIdx              (1U)        /* index of the bearerId value to be used to send Service B data - second available bearerId */

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
static uint16_t cpHandles[] = { (uint16_t)value_A1_control_point, (uint16_t)value_B1_control_point };

/* Advertising parameters */
static appAdvertisingParams_t appAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};

/* Application specific data*/
static TIMER_MANAGER_HANDLE_DEFINE(mAdvTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mBatteryMeasurementTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mServiceADataTimerId);
static TIMER_MANAGER_HANDLE_DEFINE(mServiceBDataTimerId);

static uint16_t localMtu = gEattMaxMtu_c;
static uint16_t initialCredits = mMaxCbCredits_c;

static uint8_t  mPeerCBearers;
static bearerId_t mABearerIds[gGapEattMaxBearers] = {gUnenhancedBearerId_c};

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/
/* Gatt and Att callbacks */
static void BleApp_AdvertisingCallback (gapAdvertisingEvent_t* pAdvertisingEvent);
static void BleApp_ConnectionCallback (deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattServerCallback (deviceId_t deviceId, gattServerEvent_t* pServerEvent);
static void BluetoothLEHost_Initialized(void);
static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);

/* Timer Callbacks */
static void AdvertisingTimerCallback (void * pParam);
static void BatteryMeasurementTimerCallback (void * pParam);
static void ServiceAMeasurementTimerCallback(void * pParam);
static void ServiceBMeasurementTimerCallback(void * pParam);

/* Application functions */
static void BleApp_Advertise(void);
static void BluetoothLEHost_Initialized(void);
static void Service_SendNotifications(uint16_t handle, bearerId_t bearerId);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage, void *pCallbackParam);
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
void BluetoothLEHost_AppInit(void)
{
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#endif

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
    if (mPeerDeviceId == gInvalidDeviceId_c)
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
* \param[in]    pButtonHandle    button handle
* \param[in]    pMessage         Button press event
* \param[in]    pCallbackParam   parameter
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam)
{
    switch (pMessage->event)
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
#endif

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

    /* Start services */
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    mAdvState.advOn = FALSE;

    /* Allocate application timers */
    (void)TM_Open(mAdvTimerId);
    (void)TM_Open(mBatteryMeasurementTimerId);
    (void)TM_Open(mServiceADataTimerId);
    (void)TM_Open(mServiceBDataTimerId);

    /* UI */
    LedStartFlashingAllLeds();
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
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            timeout = gFastConnAdvTime_c - gFastConnFilterAcceptListAdvTime_c;
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

    /* Set advertising parameters, data and start advertising */
    (void)BluetoothLEHost_StartAdvertising(&appAdvParams,BleApp_AdvertisingCallback, BleApp_ConnectionCallback);

    /* Start advertising timer */
    (void)TM_InstallCallback((timer_handle_t)mAdvTimerId, AdvertisingTimerCallback, NULL);
    (void)TM_Start((timer_handle_t)mAdvTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, timeout);
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
            if(mAdvState.advOn)
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                LedSetColor(0, kLED_Blue);
#endif /*gAppLedCnt_c == 1*/  
                Led1Flashing();
            }
            else
            {
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
                LedSetColor(0, kLED_White);
#endif /*gAppLedCnt_c == 1*/
                /* Turn on the other LEDs */
                LedStartFlashingAllLeds();
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            panic(0,0,0,0);
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
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
            mPeerDeviceId = peerDeviceId;

            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;

            /* Stop Advertising Timer*/
            (void)TM_Stop(mAdvTimerId);

            /* Subscribe client*/
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);

            /* UI */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c > 0))
            LedStopFlashingAllLeds();
            /* Turn on LED1 */
#if (defined(gAppLedCnt_c) && (gAppLedCnt_c == 1))
            LedSetColor(0, kLED_White);
#endif /*gAppLedCnt_c == 1*/            
            Led1On();
#endif /*gAppLedCnt_c > 0*/

            /* Start battery measurements */
            (void)TM_InstallCallback((timer_handle_t)mBatteryMeasurementTimerId, BatteryMeasurementTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mBatteryMeasurementTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, mBatteryLevelReportInterval_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);

            mPeerDeviceId = gInvalidDeviceId_c;

            /* Restart advertising */
            BleApp_Start();
        }
        break;

        case gConnEvtEattConnectionRequest_c:
        {
            (void)Gap_EattConnectionAccept(peerDeviceId, TRUE, localMtu, initialCredits, TRUE);
        }
        break;

        case gConnEvtEattConnectionComplete_c:
        {
            /* save the data from the peer if the EATT connection was successful */
           if (gSuccessful_c == pConnectionEvent->eventData.eattConnectionComplete.status)
           {
              mPeerCBearers = pConnectionEvent->eventData.eattConnectionComplete.cBearers;
              FLib_MemCpy(mABearerIds, pConnectionEvent->eventData.eattConnectionComplete.aBearerIds, mPeerCBearers);
           }
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
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
    uint16_t handle;

    switch (pServerEvent->eventType)
    {
        case gEvtAttributeWritten_c:
        case gEvtAttributeWrittenWithoutResponse_c:
        {
            handle = pServerEvent->eventData.attributeWrittenEvent.handle;

            (void)GattServer_SendAttributeWrittenStatus(deviceId, handle, (uint8_t)gAttErrCodeNoError_c);
        }
        break;

        case gEvtCharacteristicCccdWritten_c:
        {
            handle = pServerEvent->eventData.charCccdWrittenEvent.handle;

            if (handle == (uint16_t)cccd_A1)
            {
                /* subscribe to service A - start timer to send data */
                (void)TM_InstallCallback((timer_handle_t)mServiceADataTimerId, ServiceAMeasurementTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mServiceADataTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, mServiceAReportInterval_c);
            }

            if (handle == (uint16_t)cccd_B1)
            {
                /* subscribe to service B - start timer to send data */
                (void)TM_InstallCallback((timer_handle_t)mServiceBDataTimerId, ServiceBMeasurementTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mServiceBDataTimerId, (uint8_t)kTimerModeLowPowerTimer | (uint8_t)kTimerModeSetSecondTimer, mServiceBReportInterval_c);
            }
        }
        break;

        default:
        {
            ; /* For MISRA Compliance */
        }
        break;
    }
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

/*! *********************************************************************************
* \brief        Handles service A measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void ServiceAMeasurementTimerCallback(void * pParam)
{
    uint16_t  handle;
    bleResult_t result;
    bleUuid_t uuid = Uuid16(gBleSig_Report_d);
    static uint16_t char_value = 0U;

    /* Get handle of characteristic */
    result = GattDb_FindCharValueHandleInService((uint16_t)service_A, gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Update characteristic value and send notification */
        result = GattDb_WriteAttribute(handle, 2, (uint8_t const*)&char_value);
        char_value++;
        if (char_value == 10U)
        {
            char_value = 0U;
        }

        if (result == gBleSuccess_c)
        {
            Service_SendNotifications(handle, mABearerIds[mServiceABearerIdx]);
        }
    }
}

/*! *********************************************************************************
* \brief        Handles service B measurement timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void ServiceBMeasurementTimerCallback(void * pParam)
{
    uint16_t  handle;
    bleResult_t result;
    bleUuid_t uuid = Uuid16(gBleSig_Report_d);
    static uint16_t char_value = 0U;

    /* Get handle of  characteristic */
    result = GattDb_FindCharValueHandleInService((uint16_t)service_B, gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Update characteristic value and send notification */
        result = GattDb_WriteAttribute(handle, 2, (uint8_t const*)&char_value);
        char_value++;
        if (char_value == 10U)
        {
            char_value = 0U;
        }

        if (result == gBleSuccess_c)
        {
            Service_SendNotifications(handle, mABearerIds[mServiceBBearerIdx]);
        }
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
        {
            ; /* For MISRA Compliance */
        }
        break;
    }

    BleApp_Start();
}

/*! *********************************************************************************
* \brief        Send a notification to a peer.
*
* \param[in]    handle        Handle of the attribute to be notified
* \param[in]    bearerId      Id of the bearer to use for the notification
********************************************************************************** */
static void Service_SendNotifications
(
    uint16_t     handle,
    bearerId_t   bearerId
)
{
    uint16_t  handleCccd = 0U;
    bool_t    isNotifActive;

    /* Bearer Id 0 is unenhanced */
    if (bearerId != gUnenhancedBearerId_c)
    {
        /* Get handle of CCCD */
        if (GattDb_FindCccdHandleForCharValueHandle(handle, &handleCccd) == gBleSuccess_c)
        {
            if ((gBleSuccess_c == Gap_CheckNotificationStatus(mPeerDeviceId, handleCccd, &isNotifActive)) &&
                (TRUE == isNotifActive))
            {
                (void)GattServer_EnhancedSendNotification(mPeerDeviceId, bearerId, handle);
            }
        }
    }
}

/*! *********************************************************************************
* @}
********************************************************************************** */
