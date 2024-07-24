/*! *********************************************************************************
* \addtogroup BLE OTAP Client L2CAP CoC
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2015 Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
*
*
* \file
*
* This file is the source file for the BLE OTAP Client L2CAP CoC application
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
#include "fsl_component_button.h"
#include "fsl_component_led.h"
#include "fsl_component_timer_manager.h"
#include "fsl_component_panic.h"
#include "FunctionLib.h"
#include "sensors.h"

#include "OtaSupport.h"

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
#include "otap_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "board_comp.h"
#include "app_conn.h"
#include "app_advertiser.h"
#include "app.h"
#include "otap_client_l2cap_credit.h"
#include "otap_client.h"

/************************************************************************************
*************************************************************************************
* Extern functions
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private macros
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
#if (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U)) &&\
    (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) &&\
    (defined(gBleEnableControllerPrivacy_d) && (gBleEnableControllerPrivacy_d > 0))
    fastFilterAcceptListAdvState_c,
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
    fastAdvState_c,
    slowAdvState_c
}advType_t;

typedef struct advState_tag
{
    bool_t      advOn;
    advType_t   advType;
} advState_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static deviceId_t  mPeerDeviceId = gInvalidDeviceId_c;

/* Advertising related local variables */
static advState_t   mAdvState;
static TIMER_MANAGER_HANDLE_DEFINE(mAdvTimerId);
static uint32_t     mAdvTimerTimeout = 0;

/* Service Data */
static bool_t      basValidClientList[gAppMaxConnections_c] = { FALSE };
static basConfig_t basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t disServiceConfig = {(uint16_t)service_device_info};

/* Application Data */
static TIMER_MANAGER_HANDLE_DEFINE(mBatteryMeasurementTimerId);

/* Advertising parameters */
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

static void BluetoothLEHost_GenericCallback(gapGenericEvent_t *pGenericEvent);
static void BluetoothLEHost_Initialized(void);
static void BleApp_Advertise (void);
static void AdvertisingTimerCallback (void *pParam);
static void BatteryMeasurementTimerCallback (void *pParam);

#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam);
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
    /*Install callback for button*/
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    (void)BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], (button_callback_t)BleApp_HandleKeys0, NULL);
#endif

    /* Start flashing all leds */
    LedStartFlashingAllLeds();

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
        /* Device is not connected and not advertising*/
        if (!mAdvState.advOn)
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
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
static button_status_t BleApp_HandleKeys0(void *pButtonHandle, button_callback_message_t *pMessage,void *pCallbackParam)
{
    switch (pMessage->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            for (uint8_t i = 0; i < (uint8_t)gAppLedCnt_c; i++)
            {
                LedOff(i);
            }
            Led1Flashing();

            BleApp_Start();
            break;
        }

        default:
        {
            ; /* For MISRA compliance */
        }
        break;
    }
    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

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
* \brief        Configures BLE Stack after initialization.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    bleResult_t status = gBleUnexpectedError_c;
    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register stack callbacks */
    (void)App_RegisterGattServerCallback (BleApp_GattServerCallback);


    mAdvState.advOn = FALSE;

    /* Start services */
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    status = Bas_Start(&basServiceConfig);
    
    if (status == gBleSuccess_c)
    {
        status = Dis_Start(&disServiceConfig);
    }

    if (OtapClient_Config() == FALSE)
    {
        /* An error occurred in configuring the OTAP Client */
        status = gBleUnexpectedError_c;
    }

    /* Allocate application timer */
    (void)TM_Open(mAdvTimerId);
    (void)TM_Open(mBatteryMeasurementTimerId);
    
    if (status != gBleSuccess_c)
    {
        /* An error occurred during initialization */
        panic(0,0,0,0);
    }

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
            mAdvTimerTimeout = gFastConnFilterAcceptListAdvTime_c;
        }
        break;
#endif /* gAppUseBonding_d && gAppUsePrivacy_d && gBleEnableControllerPrivacy_d */
        case fastAdvState_c:
        {
            gAdvParams.minInterval = gFastConnMinAdvInterval_c;
            gAdvParams.maxInterval = gFastConnMaxAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimerTimeout = gFastConnAdvTime_c - gFastConnFilterAcceptListAdvTime_c;
        }
        break;

        case slowAdvState_c:
        {
            gAdvParams.minInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.maxInterval = gReducedPowerMinAdvInterval_c;
            gAdvParams.filterPolicy = gProcessAll_c;
            mAdvTimerTimeout = gReducedPowerAdvTime_c;
        }
        break;

        default:
            ; /* For MISRA compliance */
        break;
    }

     /* Set advertising parameters, data and start advertising */
    (void)BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
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

            if(mAdvState.advOn)
            {
                 /* UI */
                LedStopFlashingAllLeds();
                Led1Flashing();
                /* Start advertising timer */
                (void)TM_InstallCallback((timer_handle_t)mAdvTimerId, AdvertisingTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAdvTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer, mAdvTimerTimeout);
            }
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            Led2On();
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
            /* Advertising stops when connected */
            mAdvState.advOn = FALSE;
            (void)TM_Stop((timer_handle_t)mAdvTimerId);

            /* Subscribe client*/
            mPeerDeviceId = peerDeviceId;
            (void)Bas_Subscribe(&basServiceConfig, peerDeviceId);
            (void)OtapCS_Subscribe(peerDeviceId);

            /* UI */
            /* Flash RGB */
            LedStopFlashingAllLeds();
            Led1On();

            OtapClient_HandleConnectionEvent (peerDeviceId);

            /* Start battery measurements */
            (void)TM_InstallCallback((timer_handle_t)mBatteryMeasurementTimerId, BatteryMeasurementTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mBatteryMeasurementTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer, mBatteryLevelReportInterval_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* Unsubscribe client */
            mPeerDeviceId = gInvalidDeviceId_c;
            (void)Bas_Unsubscribe(&basServiceConfig, peerDeviceId);
            (void)OtapCS_Unsubscribe();

            /* UI */
            /* Switch off RGB LED */

            LedStopFlashingAllLeds();
            LedStartFlashingAllLeds();

            /* Restart advertising*/
            BleApp_Start();

            OtapClient_HandleDisconnectionEvent (peerDeviceId);
        }
        break;

#if gAppUsePairing_d
        case gConnEvtEncryptionChanged_c:
        {
#if gAppUseBonding_d
            OtapClient_HandleEncryptionChangedEvent(peerDeviceId);
#endif
        }
        break;
#endif /* gAppUsePairing_d */

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
        case gEvtMtuChanged_c:
        {
            OtapClient_AttMtuChanged (deviceId,
                                  pServerEvent->eventData.mtuChangedEvent.newMtu);
        }
        break;
        case gEvtCharacteristicCccdWritten_c:
        {
            OtapClient_CccdWritten (deviceId,
                                pServerEvent->eventData.charCccdWrittenEvent.handle,
                                pServerEvent->eventData.charCccdWrittenEvent.newCccd);
        }
        break;

        case gEvtAttributeWritten_c:
        {
            OtapClient_AttributeWritten (deviceId,
                                     pServerEvent->eventData.attributeWrittenEvent.handle,
                                     pServerEvent->eventData.attributeWrittenEvent.cValueLength,
                                     pServerEvent->eventData.attributeWrittenEvent.aValue);
        }
        break;

        case gEvtAttributeWrittenWithoutResponse_c:
        {
            OtapClient_AttributeWrittenWithoutResponse (deviceId,
                                                    pServerEvent->eventData.attributeWrittenEvent.handle,
                                                    pServerEvent->eventData.attributeWrittenEvent.cValueLength,
                                                    pServerEvent->eventData.attributeWrittenEvent.aValue);
        }
        break;

        case gEvtHandleValueConfirmation_c:
        {
            OtapClient_HandleValueConfirmation (deviceId);
        }
        break;

        case gEvtError_c:
        {
            uint8_t attError = (uint8_t) ((uint16_t)pServerEvent->eventData.procedureError.error & 0xFFU);
            if (attError == (uint8_t)gAttErrCodeInsufficientEncryption_c     ||
                attError == (uint8_t)gAttErrCodeInsufficientAuthorization_c  ||
                attError == (uint8_t)gAttErrCodeInsufficientAuthentication_c)
            {
#if gAppUsePairing_d
#if gAppUseBonding_d
                bool_t isBonded = FALSE;

                /* Check if the devices are bonded and if this is true than the bond may have
                 * been lost on the peer device or the security properties may not be sufficient.
                 * In this case try to restart pairing and bonding. */
                if (gBleSuccess_c == Gap_CheckIfBonded(deviceId, &isBonded, NULL) &&
                    TRUE == isBonded)
#endif /* gAppUseBonding_d */
                {
                    (void)Gap_SendPeripheralSecurityRequest(deviceId, &gPairingParameters);
                }
#endif /* gAppUsePairing_d */
            }
        }
    break;
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
            ; /* For MISRA compliance */
        break;
    }
    
    BleApp_Advertise();
}

/*! *********************************************************************************
* \brief        Reads the battery level at mBatteryLevelReportInterval_c time interval.
*
********************************************************************************** */
static void BatteryMeasurementTimerCallback(void * pParam)
{
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();
    (void)Bas_RecordBatteryMeasurement(&basServiceConfig);
}


/*! *********************************************************************************
* @}
********************************************************************************** */
