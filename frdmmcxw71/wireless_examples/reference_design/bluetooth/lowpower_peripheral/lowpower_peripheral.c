/*! *********************************************************************************
* \addtogroup LowPowerRefDes
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2024 NXP
* All rights reserved.
*
* \file
*
* This file is the source file for the lowpower peripheral reference design application
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
#include "fsl_component_timer_manager.h"
#include "fsl_component_mem_manager.h"
#include "fsl_adapter_gpio.h"
#include "fsl_format.h"
#include "app.h"
#include "FunctionLib.h"
#include "fwk_debug.h"
#include "fwk_platform.h"
#include "board_lp.h"

#include "sensors.h"
#include "PWR_Interface.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_handles.h"
#include "gatt_db_app_interface.h"

/* Profile / Services */
#include "battery_interface.h"
#include "device_info_interface.h"
#include "temperature_interface.h"

/* Connection Manager */
#include "ble_conn_manager.h"

#include "board.h"
#include "lowpower_peripheral.h"
#include "app_conn.h"
#include "app_advertiser.h"

#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
#include "enhanced_notification.h"
#endif

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
#include "fsl_component_serial_manager.h"
#endif


/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#define LP_ADV_USER_REQ_NONE            0
#define LP_ADV_USER_REQ_START           1
#define LP_ADV_USER_REQ_STOP            2

#define LP_TEMP_SERVICE_IDX       0U

#define _unused(x) ((void)(x))

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
#define BLEAPP_SERIALINIT              BleApp_SerialInit
#define BLEAPP_WRITE                   BleApp_SerialWrite
#define BLEAPP_WRITEDEC                BleApp_SerialWriteDec
#else
#define BLEAPP_SERIALINIT(...)
#define BLEAPP_WRITE(...)
#define BLEAPP_WRITEDEC(...)
#endif

#if (gAppLowPowerConstraintInNoBleActivity_c<3)
#define LP_PERIPHERAL_APP_MESSAGE_NO_ACTIVITY       "\r\nGoing into lowpower (RAM Retention)\r\n"
#else
#define LP_PERIPHERAL_APP_MESSAGE_NO_ACTIVITY       "\r\nGoing into lowpower (RAM Off)\r\n"
#endif

#define LP_PERIPHERAL_ADV_START_EVENT                appEvt_AdvertiseStarted_c

/* Debug Macros - stub if not defined */
#ifndef APP_DBG_LOG
#define APP_DBG_LOG(...)
#endif

/* Defines the constraint ID to apply a WFI constraint to the Low Power module */
#define gAppWFIConstraint_c 0U

/* test only : remove Bonding for stress test */
//#define LP_APP_TEST_REMOVE_BONDING

/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/

typedef enum appEvent_tag
{
    appEvt_AdvertiseStarted_c,
    appEvt_AdvertiseStopped_c,
    appEvt_PeerConnected_c,
    appEvt_PeerDisconnected_c,
    appEvt_PairingComplete_c,
} appEvent_t;

typedef enum appSate_tag
{
    appIdle_c,
    appAdvertising_c,
    appConnected_c
} appState_t;

typedef struct appPeerInfo_tag
{
    deviceId_t          deviceId;
    bool_t              isPaired;
    bool_t              tempNotifSent;
    bool_t              battNotifSent;
    timer_handle_t      timeoutTimer;
} appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

/* Application specific data*/
static appPeerInfo_t    appPeerInformation[gAppMaxConnections_c];
static appState_t       appState;
static bool_t           appAdvOn = FALSE;
static TIMER_MANAGER_HANDLE_DEFINE(appAdvTimerId);
static uint8_t          appAdvUserRequest;
static uint8_t          appConnectionNumber = 0U;

#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1)
static bool_t           appWaitForControllerPrivacy = FALSE;
static bool_t           lowPowerConstraintToRelease = 0xFFU;
#endif

/* Service Data*/
static bool_t           basValidClientList[gAppMaxConnections_c] = {FALSE};
static basConfig_t      basServiceConfig = {(uint16_t)service_battery, 0, basValidClientList, gAppMaxConnections_c};
static disConfig_t      disServiceConfig = {(uint16_t)service_device_info};
static tmsConfig_t      tmsServiceConfig = {(uint16_t)service_temperature, 0};

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
static serial_handle_t  gLpSerMgrIf;
/*low power serial read handle*/
static SERIAL_MANAGER_READ_HANDLE_DEFINE(s_lpReadHandle);
/*low power serial write handle*/
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(s_lpWriteHandle);
#endif

static appAdvertisingParams_t mAppAdvParams = {
    &gAdvParams,
    &gAppAdvertisingData,
    &gAppScanRspData
};

#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d == 1)
static appExtAdvertisingParams_t mAppExtAdvParams =
{
    &gAppExtAdvParams
};
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

/* App Private Functions */
static void BluetoothLEHost_Initialized(void);
static void BleApp_GenericCallback(gapGenericEvent_t* pGenericEvent);
static void BleApp_Start(void);
static void BleApp_StartInit(void);
static void BleApp_StopAdvertise(void);
static void BleApp_InitPeerInformation(void);
static void BleApp_ResetPeerDeviceInformation(deviceId_t peerDeviceId);
static void BleApp_HandleConnection(deviceId_t peerDeviceId);
static void BleApp_HandleDisconnection(deviceId_t peerDeviceId);
#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U)
static void BleApp_HandlePairingComplete(deviceId_t peerDeviceId);
#endif
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static bleResult_t BleApp_SendBattery(deviceId_t peerDeviceId);
static bleResult_t BleApp_SendTemperature(deviceId_t peerDeviceId);
static bool_t BleApp_TemperatureNotificationEnabled(void);
static bool_t BleApp_BatteryNotificationEnabled(deviceId_t peerDeviceId);
static void BleApp_SubscribeClientToService(deviceId_t peerDeviceId);
static void BleApp_UnsubscribeClientFromService(deviceId_t peerDeviceId);
static void BleApp_PrintLePhyEvent(gapPhyEvent_t* pPhyEvent);

/* App context Callbacks */
static void BleApp_StateMachineCallback(appCallbackParam_t pParam);
#if defined(gPWR_RamOffDuringAdv_d) && (gPWR_RamOffDuringAdv_d == 1)
static void BleApp_PrintAdvMessageCallback(appCallbackParam_t pParam);
#endif

/* Timer Callbacks */
static void AdvertisingTimerCallback(void *pParam);
static void DisconnectTimerCallback(void *pParam);
#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
static void WakeUpTimerCallback(void* pParam);
#endif

/* Serial Private Functions */
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
static void BleApp_SerialInit(void);
static void BleApp_SerialWrite(const char* pString);
static void BleApp_SerialWriteDec(uint32_t nb);
static void BleApp_SerialRxCb( void *params, serial_manager_callback_message_t *message, serial_manager_status_t status);
#endif

/* Low Power Functions */
static void BleApp_SetLowPowerModeConstraint(uint8_t lowpower_mode_constraint);
static void BleApp_ReleaseLowPowerModeConstraint(uint8_t lowpower_mode_constraint);

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  This is the initialization function for each application. This function
*         should contain all the initialization code required by the Bluetooth demo.
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
    BLEAPP_SERIALINIT();
#endif

    /*Install callback for button*/
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#if (gAppButtonCnt_c > 1)
    BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
    PLATFORM_ResetStatus_t resetStatus;

    /* The application may want to do specific tasks on wakeup from specific reset, if so, do it here */
    (void)PLATFORM_GetResetCause(&resetStatus);
    if( resetStatus == PLATFORM_LowPowerWakeup )
    {
        BLEAPP_WRITE("\r\nWake up from RAMOFF\r\n");
    }
    else
#endif
    {
        BLEAPP_WRITE("\r\nLowpower Peripheral Reference Design Application !!\r\n");
        BLEAPP_WRITE("Connect with a Lowpower Central or Temperature collector device\r\n");
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
        BLEAPP_WRITE("Short press SW2 to start/stop advertising\r\n");
#if (gAppButtonCnt_c > 1)
        BLEAPP_WRITE("Short press SW3 to enable/disable low power\r\n");
        BLEAPP_WRITE("Long press SW3 to print a message\r\n");
#endif /* gAppButtonCnt_c > 1 */
#endif /* gAppButtonCnt_c > 0 */
    }

    appState = appIdle_c;
    BleApp_InitPeerInformation();

    /* Register generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);

    APP_DBG_LOG("");
    DBG_LOG_DUMP();
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    APP_DBG_LOG("button press 0");
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            if(!appAdvOn)
            {
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
                /* if privacy has been disabled when going to no activity RAM RET,
                   we need to restart the privacy to restart the 15min counter.
                   no required when comming from no activity RAM OFF */
                (void)BleConnManager_EnablePrivacy();


                if (gcBondedDevices == 0U)
                {
                    /* send event to State Machine callback to start advertising  */
                    App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)LP_PERIPHERAL_ADV_START_EVENT);
                }
                else
                {
                    /* wait for privacy event */
                    appWaitForControllerPrivacy = TRUE;
                }
#else
                /* send event to State Machine callback to start advertising  */
                App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)LP_PERIPHERAL_ADV_START_EVENT);
#endif
            }
            else
            {
                /* send event to State Machine callback to stop advertising and go to lowpower */
                App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)appEvt_AdvertiseStopped_c);
            }

            break;
        }

        case kBUTTON_EventLongPress:
            /* No action required */
            break;

        default:
        {
            /* No action required */
            break;
        }
    }
    DBG_LOG_DUMP();

    return kStatus_BUTTON_Success;
}

button_status_t BleApp_HandleKeys1(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    static bool lpEnabled = true;

    APP_DBG_LOG("button press 1");

    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
            /* The Button1 is used to enable/disable low power
             * This is done by setting/releasing a WFI constraint to the Low Power module */
            if(lpEnabled)
            {
                /* Apply WFI constraint to Low Power module to disallow any deeper power modes */
                BleApp_SetLowPowerModeConstraint(gAppWFIConstraint_c);
                lpEnabled = false;
                BLEAPP_WRITE("\r\nLP disabled.\r\n");
            }
            else
            {
                /* Release the WFI constraint so the device can enter deeper power modes on idle */
                BleApp_ReleaseLowPowerModeConstraint(gAppWFIConstraint_c);
                lpEnabled = true;
                BLEAPP_WRITE("\r\nLP enabled.\r\n");
            }
            break;

        case kBUTTON_EventLongPress:
            BLEAPP_WRITE("\r\nButton1 long press !\r\n");
            break;

        default:
            /* No action required */
            break;
    }
    DBG_LOG_DUMP();

    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/


/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

static void BleApp_SetLowPowerModeConstraint(uint8_t lowpower_mode_constraint)
{
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
    PWR_ReturnStatus_t status;

    /* lowpower_mode_constraint shall map with PWR_LowpowerMode_t structure */
    status = PWR_SetLowPowerModeConstraint((PWR_LowpowerMode_t)lowpower_mode_constraint);

    assert(status == PWR_Success);
    (void)status;
#else
    (void)lowpower_mode_constraint;
#endif
}

static void BleApp_ReleaseLowPowerModeConstraint(uint8_t lowpower_mode_constraint)
{
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
    PWR_ReturnStatus_t status;

    /* lowpower_mode_constraint shall map with PWR_LowpowerMode_t structure */
    status = PWR_ReleaseLowPowerModeConstraint((PWR_LowpowerMode_t)lowpower_mode_constraint);

    assert(status == PWR_Success);
    (void)status;
#else
    (void)lowpower_mode_constraint;
#endif
}

static void BleApp_LowpowerInit(void)
{
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d>0)
    PWR_ReturnStatus_t status;

    /*
     * Optionally, Allow now Deepest lowpower mode constraint given by gAPP_LowPowerConstraintInNoBleActivity_c
     *    rather than DeepSleep mode.
     * Deep Sleep mode constraint has been set in APP_InitServices(), this is fine
     *    to keep this constraint for typical lowpower application but we want the
     *     lowpower reference design application to be more agressive in term of power saving.

     *   To apply a lower lowpower mode than Deep Sleep mode, we need to
     *     - 1) First, release the Deep sleep mode constraint previously set  by default in app_services_init()
     *     - 2) Apply new lowpower constraint when No BLE activity
     *   In the various BLE states (advertising, scanning, connected mode), a new Lowpower
     *     mode constraint will be applied depending of Application Compilation macro set in app_preinclude.h :
     *     gAppPowerDownInAdvertising, gAppPowerDownInConnected, gAppPowerDownInScanning
     */

    /*  1) Release the Deep sleep mode constraint previously set  by default in app_services_init() */
    status = PWR_ReleaseLowPowerModeConstraint(PWR_DeepSleep);
    assert(status == PWR_Success);
    (void)status;

    /*  2) Apply new Lowpower mode constraint gAppLowPowerConstraintInNoBleActivity_c *
     *       The BleAppStart() call above has already set up the new lowpower constraint
     *         when Advertising request has been sent to controller         */
    BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInNoBleActivity_c);

    APP_DBG_LOG("");
    DBG_LOG_DUMP();
#endif
}


/* ***********************************************************************************
*************************************************************************************
* Gatt and Att Callbacks
*************************************************************************************
*********************************************************************************** */
/*! *********************************************************************************
* \brief        Handles BLE generic callback.
*
* \param[in]    pGenericEvent    Pointer to gapGenericEvent_t.
********************************************************************************** */
static void BleApp_GenericCallback(gapGenericEvent_t* pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1)
        case gControllerPrivacyStateChanged_c:
        {
            if(pGenericEvent->eventData.newControllerPrivacyState == TRUE)
            {
                if(appWaitForControllerPrivacy == TRUE)
                {
                    /* Controller Privacy is ready, we can start adv */
                    BLEAPP_WRITE("\r\nPrivacy's ready\r\n");
                    BleApp_StartInit();
                    appWaitForControllerPrivacy = FALSE;
                }
            }
            else
            {
                if((appWaitForControllerPrivacy == TRUE) && (lowPowerConstraintToRelease != 0xFFU))
                {
                    BleApp_ReleaseLowPowerModeConstraint(lowPowerConstraintToRelease);
                }
            }
        }
        break;
#endif

#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d == 1)
        case gExtAdvertisingParametersSetupComplete_c:
        {
            (void)Gap_SetExtAdvertisingData(0,&gAppAdvertisingData, &gAppScanRspData);
        }
        break;

        case gExtAdvertisingDataSetupComplete_c:
        {
            /* Start advertising if data and parameters were successfully set */
            (void)Gap_StartExtAdvertising(BleApp_AdvertisingCallback, BleApp_ConnectionCallback,0,0,0);
        }
        break;
#endif

        case gLePhyEvent_c:
        {
            if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
            {
                BleApp_PrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
            }
        }
        break;

        case gAdvertisingSetupFailed_c:
        {
            APP_DBG_LOG("ADV Setup failed");
            assert(0);
        }
        break;

#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
        case gControllerNotificationEvent_c:
        {
            BleApp_HandleControllerNotification(&pGenericEvent->eventData.notifEvent);
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
#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d == 1)
        case gExtAdvertisingStateChanged_c:
#endif
        {
            appAdvOn = !appAdvOn;
            switch (appAdvUserRequest)
            {
            case LP_ADV_USER_REQ_START:

                /* An ADV Start has been requested by the user */
                BLEAPP_WRITE("\r\nAdvertising...\r\n");
                APP_DBG_LOG("ADV started");

                /* First time we start the ADV => Start ADV expiration timer */
                (void)TM_InstallCallback((timer_handle_t)appAdvTimerId, AdvertisingTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)appAdvTimerId, kTimerModeSetSecondTimer | kTimerModeLowPowerTimer, gAppAdvTimeout_c);

                break;

            case LP_ADV_USER_REQ_STOP:
                /* An ADV stop has been requested by the user */
                BLEAPP_WRITE("\r\nAdvertising stopped\r\n");
                APP_DBG_LOG("ADV Stopped");

                (void)TM_Stop((timer_handle_t)appAdvTimerId);

#if !defined(gAppUsePrivacy_d) || (gAppUsePrivacy_d == 0)
                /* Release ADV Lowpower Constraint when Advertising stop is confirmed */
                BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInAdvertising_c);
#endif

                if ( appConnectionNumber == 0U )
                {
#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
                /* When Privacy is enabled, a LPTMR is running to update RPA
                   To be able to shutdown 32khz osc in Deep Power Down mode, no LPTMR should run
                   Calling this function will stop the timer and allow power saving */
                (void)BleConnManager_DisablePrivacy();

                if (gcBondedDevices == 0U)
                {
                    /* Release ADV Lowpower Constraint when Advertising stop is confirmed */
                    BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInAdvertising_c);
                }
                else
                {
                    /* In this case, we need to have the confirmation of the Controller before
                     * Going to RAMOFF */
                    appWaitForControllerPrivacy = TRUE;
                    lowPowerConstraintToRelease = gAppLowPowerConstraintInAdvertising_c;
                }
#endif
#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
                    (void)TM_InstallCallback((timer_handle_t)appAdvTimerId, WakeUpTimerCallback, NULL);
                    (void)TM_Start((timer_handle_t)appAdvTimerId, kTimerModeSingleShot | kTimerModeLowPowerTimer, gAppWakeUpTimerAfterNoActivityMs);
#endif
                }
                break;

            default:
                break;
            }

            appAdvUserRequest = LP_ADV_USER_REQ_NONE;
        }
        break;

        case gAdvertisingCommandFailed_c:
        {
            /* Panic UI */
            APP_DBG_LOG("ADV Failed");
            assert(0);
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
        case gConnEvtPairingComplete_c:
        {
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
                BleApp_StateMachineHandler(peerDeviceId, appEvt_PairingComplete_c);
                assert(appPeerInformation[peerDeviceId].isPaired);
            }
            else
            {
                BLEAPP_WRITE("Device ");
                BLEAPP_WRITEDEC((uint32_t)appPeerInformation[peerDeviceId].deviceId);
                BLEAPP_WRITE(" - Pairing failed\r\n");
            }
        }
        break;

        case gConnEvtConnected_c:
        {
            /* Host stack automatically stops ADV when connection is established */
            appAdvOn = FALSE;

            BleApp_StateMachineHandler(peerDeviceId, appEvt_PeerConnected_c);

            /* Set New Lowpower Constraint in Connected mode */
            BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);
            BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInAdvertising_c);
            (void)TM_Stop((timer_handle_t)appAdvTimerId);

            /* If max connection is reached, no need to advertise anymore,
             * we can stop the adv timer */
            if(appConnectionNumber == gAppMaxConnections_c )
            {
#if (gAppMaxConnections_c > 1)
                BLEAPP_WRITE("\r\nMax connection reached\r\n");
#endif
            }
            else
            {
                /* continue to advertise */
                App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)LP_PERIPHERAL_ADV_START_EVENT);
            }
        }
        break;

        case gConnEvtDisconnected_c:
        {
            /* at this stage, connection number cannot be equal to 0 */
            assert(appConnectionNumber);

            BleApp_StateMachineHandler(peerDeviceId, appEvt_PeerDisconnected_c);

            /* Release Connected Lowpower Constraint when Disconnection is completed */
            BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);
#if defined(gAppRestartAdvAfterConnect) && (gAppRestartAdvAfterConnect == 1)

            /* need to restart advertising in case ADV was stopped
             * when max connection has been reached */
            BleApp_StateMachineHandler(gInvalidDeviceId_c, LP_PERIPHERAL_ADV_START_EVENT);
#else
            if( (appConnectionNumber > 0) && (appConnectionNumber < gAppMaxConnections_c) )
            {
                /* need to restart advertising in case ADV was stopped
                 * when max connection has been reached */
                BleApp_StateMachineHandler(gInvalidDeviceId_c, LP_PERIPHERAL_ADV_START_EVENT);
            }
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1)
            else
            {
                /* When Privacy is enabled, a LPTMR is running to update RPA
                   To be able to shutdown 32khz osc in Deep Power Down mode, no LPTMR should run
                   Calling this function will stop the timer and allow power saving */
                (void)BleConnManager_DisablePrivacy();

                if (gcBondedDevices == 0U)
                {
                    /* Release ADV Lowpower Constraint when Advertising stop is confirmed */
                    BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);
                }
                else
                {
                    /* In this case, we need to have the confirmation of the Controller before
                        * Going to RAMOFF */
                    appWaitForControllerPrivacy = TRUE;
                    lowPowerConstraintToRelease = gAppLowPowerConstraintInConnected_c;
                }
            }
#endif
#endif
        }
        break;

        case gConnEvtEncryptionChanged_c:   /* Fallthrough */
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
            //APP_DBG_LOG("CCCD Written");
            bleResult_t result = gBleInvalidState_c;

#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U)

            /* on iOS, this event is sent before bonding is complete
             * so we need to check the device is bonded before sending data
             * but in all cases it's a good habit to check the device is paired
             * before sending data */
            if( appPeerInformation[deviceId].isPaired )
#endif
            {
                /* we check what CCCD has been written */

                if(pServerEvent->eventData.charCccdWrittenEvent.handle == (uint16_t)cccd_temperature)
                {
                    /* check if the peer device is subscribed to the Temperature service */
                    if(appPeerInformation[deviceId].deviceId == LP_TEMP_SERVICE_IDX)
                    {
                        result = BleApp_SendTemperature(deviceId);
                        appPeerInformation[deviceId].tempNotifSent = TRUE;
                    }
                }
                else
                {
                    if(pServerEvent->eventData.charCccdWrittenEvent.handle == (uint16_t)cccd_battery_level)
                    {
                        /* all devices are subscribed to Battery service */
                        result = BleApp_SendBattery(deviceId);
                        appPeerInformation[deviceId].battNotifSent = TRUE;
                    }
                }
#if !defined(gAppDisconnectOnTimeoutOnly_s) || (gAppDisconnectOnTimeoutOnly_s == 0)
                /* Disconnect only if successful */
                if(result == gBleSuccess_c)
                {
                    bool_t notificationComplete = FALSE;

                    /* if the device is subscribed to both services, need to check
                       if both notifications have been sent before disconnecting */
                    if(appPeerInformation[deviceId].deviceId == LP_TEMP_SERVICE_IDX)
                    {
                        if( (appPeerInformation[deviceId].tempNotifSent == TRUE) &&\
                            (appPeerInformation[deviceId].battNotifSent == TRUE) )
                        {
                            notificationComplete = TRUE;
                        }
                    }
                    else
                    {
                        if(appPeerInformation[deviceId].battNotifSent == TRUE)
                        {
                            notificationComplete = TRUE;
                        }
                    }

                    if(notificationComplete == TRUE)
                    {
                        /* Schedule disconnect in 50 ms to make sure data arrives to Gatt Client */
                        (void)TM_Start((timer_handle_t)appPeerInformation[deviceId].timeoutTimer, kTimerModeSingleShot, 50U);
                    }
                }
#else
                NOT_USED(result);
#endif
            }
        }
        break;

        default:
        {
            ; /* No action required */
        }
        break;
    }
}

/* ***********************************************************************************
*************************************************************************************
* App Private Functions
*************************************************************************************
*********************************************************************************** */
/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    SENSORS_TriggerTemperatureMeasurement();

    /* Common GAP configuration */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattServerCallback(BleApp_GattServerCallback);

    /* Start services */
    basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();

    (void)Bas_Start(&basServiceConfig);
    (void)Dis_Start(&disServiceConfig);

    /* Allocate application timer */
    (void)TM_Open((timer_handle_t)appAdvTimerId);

    SENSORS_RefreshTemperatureValue();
    tmsServiceConfig.initialTemperature = (int16_t)(100 * SENSORS_GetTemperature());

    (void)Tms_Start(&tmsServiceConfig);

    APP_DBG_LOG("");
    DBG_LOG_DUMP();

#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))

    if (gcBondedDevices == 0U)
    {
        /* if no devices are bonded, we can start adv immediately */
        BleApp_StartInit();
    }
    else
    {
        /* will wait event from Controller before starting adv */
        appWaitForControllerPrivacy = TRUE;
    }
#else
    BleApp_StartInit();
#endif
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
static void BleApp_Start(void)
{
    /* Invoke app command to start advertising */
    BleApp_StateMachineHandler(gInvalidDeviceId_c, LP_PERIPHERAL_ADV_START_EVENT);

    APP_DBG_LOG("");
    DBG_LOG_DUMP();
}

/*! *********************************************************************************
* \brief    Starts the BLE application for the first time
*
********************************************************************************** */
static void BleApp_StartInit(void)
{
    /* Start the BLE activity, typical in this case, Advertising */
    BleApp_Start();

    /* Update Lowpower constraint for this application */
    BleApp_LowpowerInit();
}

/*! *********************************************************************************
* \brief        Requests to stop advertising.
*
********************************************************************************** */
static void BleApp_StopAdvertise(void)
{
    bleResult_t result = gBleSuccess_c;
    appAdvUserRequest = LP_ADV_USER_REQ_STOP;
    /* since recent changes in host stack, ADV callback won't be called if the
     * advertising has been already stopped and Gap_StopAdvertising returns gBleInvalidState_c
     * so if appAdvOn is TRUE but advertising has already been stopped, it means the app doesn't
     * follow correctly the ADV state, so we assert to catch the bug */
    if( appAdvOn == TRUE )
    {
        result = Gap_StopAdvertising();
    }
    assert(result != gBleInvalidState_c);
    _unused( result );
}

/*! *********************************************************************************
* \brief        Init peer device ids.
*
********************************************************************************** */
static void BleApp_InitPeerInformation(void)
{
    for(int i = 0; i < gAppMaxConnections_c; i++)
    {
        appPeerInformation[i].deviceId = gInvalidDeviceId_c;
        appPeerInformation[i].isPaired = false;
        appPeerInformation[i].timeoutTimer = MEM_BufferAlloc(TIMER_HANDLE_SIZE);
        (void)TM_Open((timer_handle_t)appPeerInformation[i].timeoutTimer);
    }
}

/*! *********************************************************************************
* \brief        Resets all peer device information
*
********************************************************************************** */
static void BleApp_ResetPeerDeviceInformation(deviceId_t peerDeviceId)
{
    /* Reset all peer device information to default value
       and stop the connection timeout timer
       Should be called at disconnection */
    appPeerInformation[peerDeviceId].tempNotifSent = FALSE;
    appPeerInformation[peerDeviceId].battNotifSent = FALSE;
    appPeerInformation[peerDeviceId].isPaired = FALSE;
    (void)TM_Stop((timer_handle_t)appPeerInformation[peerDeviceId].timeoutTimer);
    appPeerInformation[peerDeviceId].deviceId = gInvalidDeviceId_c;
}

/*! *********************************************************************************
* \brief        Registers a new connected device and check different parameters
*               associated.
*
********************************************************************************** */
static void BleApp_HandleConnection(deviceId_t peerDeviceId)
{
    appConnectionNumber++;
    /* register device */
    appPeerInformation[peerDeviceId].deviceId = peerDeviceId;
    BleApp_SubscribeClientToService(peerDeviceId);

#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U)
    /* if the device has already been bonded, consider it is paired */
    (void)Gap_CheckIfBonded(appPeerInformation[peerDeviceId].deviceId, &appPeerInformation[peerDeviceId].isPaired, NULL);
#endif /* gAppUsePairing_d */

    /* start timeout timer */
    (void)TM_InstallCallback((timer_handle_t)appPeerInformation[peerDeviceId].timeoutTimer, DisconnectTimerCallback, &appPeerInformation[peerDeviceId].deviceId);
    (void)TM_Start((timer_handle_t)appPeerInformation[peerDeviceId].timeoutTimer, kTimerModeSetSecondTimer | kTimerModeLowPowerTimer, gAppConnectionTimeoutInSecs_c);

#if (gAppMaxConnections_c > 1)
    BLEAPP_WRITE("Device ");
    BLEAPP_WRITEDEC((uint32_t)appPeerInformation[peerDeviceId].deviceId);
    BLEAPP_WRITE(" - Connected\r\n");
#else
    BLEAPP_WRITE("Connected\r\n");
#endif
}

/*! *********************************************************************************
* \brief        Reverts what has been done in BleApp_HandleConnection()
*
********************************************************************************** */
static void BleApp_HandleDisconnection(deviceId_t peerDeviceId)
{
    appConnectionNumber--;

    /* unregister device */
    BleApp_UnsubscribeClientFromService(peerDeviceId);
    /* reset peer device information */
    BleApp_ResetPeerDeviceInformation(peerDeviceId);

#if (gAppMaxConnections_c > 1)
    BLEAPP_WRITE("Device ");
    BLEAPP_WRITEDEC((uint32_t)peerDeviceId);
    BLEAPP_WRITE(" - Disconnected\r\n");
#else
    BLEAPP_WRITE("Disconnected\r\n");
#endif
}

/*! *********************************************************************************
* \brief        Registers the device as paired.
*
********************************************************************************** */
#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U)
static void BleApp_HandlePairingComplete(deviceId_t peerDeviceId)
{
    appPeerInformation[peerDeviceId].isPaired = TRUE;

#if (gAppMaxConnections_c > 1)
    BLEAPP_WRITE("Device ");
    BLEAPP_WRITEDEC((uint32_t)appPeerInformation[peerDeviceId].deviceId);
    BLEAPP_WRITE(" - Paired\r\n");
#else
    BLEAPP_WRITE("Paired\r\n");
#endif
}
#endif

/*! *********************************************************************************
* \brief        Handles app state machine.
*
* \param[in]    peerDeviceId        peer device's ID.
* \param[in]    event               app event.
********************************************************************************** */
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    APP_DBG_LOG("State: %d Evt: %d", appState, event);
    switch (appState)
    {
    case appIdle_c:
        {
            switch(event)
            {
            case appEvt_AdvertiseStarted_c:
                {
                    /* No need to start advertising if already on */
                    if (!appAdvOn)
                    {
                        /* Set the App on Advertising state */
                        appState = appAdvertising_c;

#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
                        /* enable controller adv notifications */
                        BleApp_EnableControllerNotifications(gNotifSetAdv_c);
#endif
                        /* We request an Adv start (processed in Advertising callback) */
                        appAdvUserRequest = LP_ADV_USER_REQ_START;

                        /* Set New Lowpower Constraint on Advertising start request */
                        BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInAdvertising_c);

                      /* Start advertising */
#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d == 1)
                        BluetoothLEHost_StartExtAdvertising(&mAppExtAdvParams, BleApp_AdvertisingCallback,BleApp_ConnectionCallback);
#else
                        BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
#endif
                    }
                }
                break;

            default:
                {
                    /* should not get another event in this state */
                    assert(0);
                }
            }
        }
        break;

    case appAdvertising_c:
        {
            switch(event)
            {
            case appEvt_PeerConnected_c:
                {
                    appState = appConnected_c;
#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
                    /* enable controller conn notifications */
                    BleApp_EnableControllerNotifications(gNotifSetAdvConn_c);
#endif
                    /* This function will register the device to a service,
                     * handle the connection number, etc (see function for more details) */
                    BleApp_HandleConnection(peerDeviceId);
                }
                break;
            case appEvt_AdvertiseStopped_c:
                {
                    /* If we stop advertising we can go back to Idle state */
                    appState = appIdle_c;

                    /* This function will request to stop the Advertising.
                     * Depending on gAppLowPowerConstraintInNoBleActivity_c, the device will
                     * go to Deep Power Down mode or PowerDown mode */
                    BleApp_StopAdvertise();
                }
                break;

            default:
                {
                    /* should not get another event in this state */
                    assert(0);
                }
            }
        }
        break;

    case appConnected_c:
        {
            switch(event)
            {
            case appEvt_PeerConnected_c:
                {
#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
                    /* enable controller conn notifications */
                    BleApp_EnableControllerNotifications(gNotifSetAdvConn_c);
#endif
                    BleApp_HandleConnection(peerDeviceId);
                }
                break;
            case appEvt_PeerDisconnected_c:
                {
                    /* this function basically reverts what has been done in
                     * BleApp_HandleConnection */
                    BleApp_HandleDisconnection(peerDeviceId);

                    if(appConnectionNumber == 0U)
                    {
                        /* If there's not connection anymore, we set the app state
                         * back to Idle and request an Advertising stop.
                         * Depending on gAppLowPowerConstraintInNoBleActivity_c, the device will
                         * go to Deep Power Down mode (RAMOFF) or PowerDown mode (RAMRET) */
                        appState = appIdle_c;
#if defined(gAppRestartAdvAfterConnect) && (gAppRestartAdvAfterConnect == 1)
                        BleApp_StopAdvertise();
#endif

#if defined(LP_APP_TEST_REMOVE_BONDING)
                        /* test only : remove Bonding for stress test */
                        Gap_RemoveAllBonds();
#endif
                    }
                }
                break;
            case appEvt_AdvertiseStarted_c:
                {
                    appAdvUserRequest = LP_ADV_USER_REQ_START;

                    /* Set New Lowpower Constraint on Advertising start request */
                    BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInAdvertising_c);

                    /* Start advertising */
                    BluetoothLEHost_StartAdvertising(&mAppAdvParams, BleApp_AdvertisingCallback, BleApp_ConnectionCallback);
                }
                break;
            case appEvt_AdvertiseStopped_c:
                {
                    BleApp_StopAdvertise();
                }
                break;
#if defined(gAppUsePairing_d) && (gAppUsePairing_d == 1U)
            case appEvt_PairingComplete_c:
                {
                    /* Basically registers the device as paired */
                    BleApp_HandlePairingComplete(peerDeviceId);
                }
                break;
#endif
            default:
                {
                    /* should not get another event in this state */
                    assert(0);
                }
            }
        }
        break;
    default:
        {
            /* should not be another state */
            assert(0);
        }
        break;
    }
    //DBG_LOG_DUMP();
}

/*! *********************************************************************************
* \brief        Sends battery level over-the-air.
*
********************************************************************************** */
static bleResult_t BleApp_SendBattery(deviceId_t peerDeviceId)
{
    bleResult_t result = gBleInvalidState_c;
    bool_t notif_enable = BleApp_BatteryNotificationEnabled(peerDeviceId);

    APP_DBG_LOG("");

    if( notif_enable )
    {
        basServiceConfig.batteryLevel = SENSORS_GetBatteryLevel();

        result = Bas_RecordBatteryMeasurement(&basServiceConfig);

        if( result == gBleSuccess_c )
        {
#if (gAppMaxConnections_c > 1)
            BLEAPP_WRITE("Device ");
            BLEAPP_WRITEDEC((uint32_t)appPeerInformation[peerDeviceId].deviceId);
            BLEAPP_WRITE(" - Battery level sent: ");
#else
            BLEAPP_WRITE("Battery level sent: ");
#endif
            BLEAPP_WRITEDEC((uint32_t)basServiceConfig.batteryLevel);
            BLEAPP_WRITE("%\r\n");
        }
    }
    else
    {
        APP_DBG_LOG("%d - Battery Notifications not enabled", peerDeviceId);
    }
    return result;
}

/*! *********************************************************************************
* \brief        Sends temperature value over-the-air.
*
********************************************************************************** */
static bleResult_t BleApp_SendTemperature(deviceId_t peerDeviceId)
{
    int16_t temperature_val;
    bleResult_t result = gBleInvalidState_c;
    bool_t notif_enable = BleApp_TemperatureNotificationEnabled();

    APP_DBG_LOG("");

    /* make sur notifications are enabled on client side */
    if( notif_enable )
    {
        /*TODO : Refresh the temperature periodically*/
        SENSORS_TriggerTemperatureMeasurement();
        SENSORS_RefreshTemperatureValue();

        temperature_val = ((int16_t)SENSORS_GetTemperature())/10;

        /* Update with initial temperature */
        result = Tms_RecordTemperatureMeasurement((uint16_t)service_temperature,
                                               temperature_val * 100);
        if( result == gBleSuccess_c )
        {
#if (gAppMaxConnections_c > 1)
            BLEAPP_WRITE("Device ");
            BLEAPP_WRITEDEC((uint32_t)appPeerInformation[peerDeviceId].deviceId);
            BLEAPP_WRITE(" - Temperature sent: ");
#else
            BLEAPP_WRITE("Temperature sent: ");
#endif
            BLEAPP_WRITEDEC((uint32_t)temperature_val);
            BLEAPP_WRITE(" C\r\n");
        }
    }
    else
    {
        APP_DBG_LOG("%d - Temperature Notifications not enabled", peerDeviceId);
    }

    return result;
}

/*! *********************************************************************************
* \brief        Check temperature notifications are enabled on client side.
*
********************************************************************************** */
static bool_t BleApp_TemperatureNotificationEnabled()
{
    bool_t enable = false;
    uint16_t  handle;
    uint16_t  hCccd;
    bleResult_t result;
    bleUuid_t uuid = Uuid16(gBleSig_Temperature_d);

    result = GattDb_FindCharValueHandleInService((uint16_t)service_temperature,
             gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Get handle of CCCD */
        result = GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd);
        if (result == gBleSuccess_c)
        {
            /* Check if notifications are active */
            (void)Gap_CheckNotificationStatus(appPeerInformation[LP_TEMP_SERVICE_IDX].deviceId, hCccd, &enable);
        }
    }
    return enable;
}

/*! *********************************************************************************
* \brief        Check battery notifications are enabled on client side.
*
********************************************************************************** */
static bool_t BleApp_BatteryNotificationEnabled(deviceId_t peerDeviceId)
{
    bool_t enable = false;
    uint16_t  handle;
    uint16_t  hCccd;
    bleResult_t result;
    bleUuid_t uuid = Uuid16(gBleSig_BatteryLevel_d);

    result = GattDb_FindCharValueHandleInService((uint16_t)service_battery,
             gBleUuidType16_c, &uuid, &handle);

    if (result == gBleSuccess_c)
    {
        /* Get handle of CCCD */
        result = GattDb_FindCccdHandleForCharValueHandle(handle, &hCccd);
        if (result == gBleSuccess_c)
        {
            /* Check if notifications are active */
            (void)Gap_CheckNotificationStatus(appPeerInformation[peerDeviceId].deviceId, hCccd, &enable);
        }
    }
    return enable;
}

/*! *********************************************************************************
* \brief        Subscribe a client to a service.
*
* \param[in]    peerDeviceId        Peer device ID.
********************************************************************************** */
static void BleApp_SubscribeClientToService(deviceId_t peerDeviceId)
{
    /* All peer devices are subscribed to Battery Service but
       Only one can be subscribed to Temperature Service */
    if( appPeerInformation[peerDeviceId].deviceId == LP_TEMP_SERVICE_IDX )
    {
        (void)Tms_Subscribe(appPeerInformation[peerDeviceId].deviceId);
    }

    (void)Bas_Subscribe(&basServiceConfig, appPeerInformation[peerDeviceId].deviceId);
}

/*! *********************************************************************************
* \brief        Unsubscribe a client to a service.
*
* \param[in]    peerDeviceId        Peer device ID.
********************************************************************************** */
static void BleApp_UnsubscribeClientFromService(deviceId_t peerDeviceId)
{
    if( appPeerInformation[peerDeviceId].deviceId == LP_TEMP_SERVICE_IDX )
    {
        (void)Tms_Unsubscribe();
    }

    (void)Bas_Unsubscribe(&basServiceConfig, appPeerInformation[peerDeviceId].deviceId);
}

/*! *********************************************************************************
* \brief        Prints phy event.
*
********************************************************************************** */
static void BleApp_PrintLePhyEvent(gapPhyEvent_t* pPhyEvent)
{
    /* String dictionary corresponding to gapLePhyMode_t */
    static const char* mLePhyModeStrings[] =
    {
        "Invalid\n\r",
        "1M\n\r",
        "2M\n\r",
        "Coded\n\r",
    };
    uint8_t txPhy = (pPhyEvent->txPhy <= gLePhyCoded_c) ? pPhyEvent->txPhy : 0U;
    uint8_t rxPhy = (pPhyEvent->rxPhy <= gLePhyCoded_c) ? pPhyEvent->rxPhy : 0U;
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
    BLEAPP_WRITE("Phy Update Complete.\n\r");
    BLEAPP_WRITE("TxPhy ");
    BLEAPP_WRITE(mLePhyModeStrings[txPhy]);
    BLEAPP_WRITE("RxPhy ");
    BLEAPP_WRITE(mLePhyModeStrings[rxPhy]);
#else
    NOT_USED(txPhy);
    NOT_USED(rxPhy);
    NOT_USED(mLePhyModeStrings);
#endif
}

/* ***********************************************************************************
*************************************************************************************
* App Context Callbacks
*************************************************************************************
*********************************************************************************** */

/*! *********************************************************************************
* \brief        Makes sure BleApp_StateMachineHandler is executed in App task context.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void BleApp_StateMachineCallback(appCallbackParam_t pParam)
{
    uint32_t event = (uint32_t)pParam;
    APP_DBG_LOG("Evt: %d", event);
    BleApp_StateMachineHandler(gInvalidDeviceId_c, (appEvent_t)event);
}

/*! *********************************************************************************
* \brief        Print Advertising state in App task context.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
#if defined(gPWR_RamOffDuringAdv_d) && (gPWR_RamOffDuringAdv_d == 1)
static void BleApp_PrintAdvMessageCallback(appCallbackParam_t pParam)
{
    if(appAdvOn == TRUE)
    {
        BLEAPP_WRITE("\r\nAdvertising...\r\n");
        APP_DBG_LOG("ADV started");
    }
    else
    {
        BLEAPP_WRITE("\r\nAdvertising stopped\r\n");
        APP_DBG_LOG("ADV stopped");
    }
    NOT_USED(pParam);
}
#endif

/* ***********************************************************************************
*************************************************************************************
* Timer Callbacks
*************************************************************************************
*********************************************************************************** */

/*! *********************************************************************************
* \brief        Handles advertising timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void AdvertisingTimerCallback(void* pParam)
{
    /* Post to main App task to execute a callback, it makes sure the callback will be executed
     * in App context and prevents race conditions */
    App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)appEvt_AdvertiseStopped_c);
}

/*! *********************************************************************************
* \brief        Handles disconnect timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    deviceId_t* peerDeviceId = (deviceId_t*)pParam;
    if( appPeerInformation[*peerDeviceId].deviceId != gInvalidDeviceId_c )
    {
        /* Terminate connection */
        (void)Gap_Disconnect(appPeerInformation[*peerDeviceId].deviceId);
         APP_DBG_LOG("Connection timeout device id %d", appPeerInformation[*peerDeviceId].deviceId);
    }
}

/*! *********************************************************************************
* \brief        Handles wake up after no activity timer callback.
*
* \param[in]    pParam        Callback parameters.
********************************************************************************** */
#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
static void WakeUpTimerCallback(void* pParam)
{
    APP_DBG_LOG("");
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1)
    /* if privacy has been disabled when going to no activity RAM RET,
       we need to restart the privacy to restart the 15min counter.
       no required when comming from no activity RAM OFF */
    (void)BleConnManager_EnablePrivacy();

    if (gcBondedDevices == 0U)
    {
        /* send event to State Machine callback to start advertising  */
        App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)LP_PERIPHERAL_ADV_START_EVENT);
    }
    else
    {
        /* wait for privacy event */
        appWaitForControllerPrivacy = TRUE;
    }
#else
    /* send event to State Machine callback to start advertising  */
    App_PostCallbackMessage(BleApp_StateMachineCallback, (void*)LP_PERIPHERAL_ADV_START_EVENT);
#endif
}
#endif

/*! *********************************************************************************
* \brief        Serial routines
*
********************************************************************************** */
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
static void BleApp_SerialInit(void)
{
    serial_manager_status_t ret;

    gLpSerMgrIf = gSerMgrIf;

    /*open low power write handle*/
    ret = SerialManager_OpenWriteHandle((serial_handle_t)gLpSerMgrIf, (serial_write_handle_t)s_lpWriteHandle);
    assert(kStatus_SerialManager_Success == ret);

    /* Set RX callback
       In case the device could get RX data from the serial prior to this point, Setting the RX callback
          can be called from APP_InitHardware() function in hardware_init.c file . However, the application
          shall make sure no BLE API (gap, etc..) is called from the callback before the host stack
          initialization (BleApp_Init() is called)  */
    ret = SerialManager_OpenReadHandle((serial_handle_t)gLpSerMgrIf, (serial_read_handle_t)s_lpReadHandle);
    assert(kStatus_SerialManager_Success == ret);
    ret = SerialManager_InstallRxCallback((serial_read_handle_t)s_lpReadHandle, BleApp_SerialRxCb, NULL);
    assert(kStatus_SerialManager_Success == ret);

    NOT_USED(ret);
}

static void BleApp_SerialWrite(const char* pString)
{
    serial_manager_status_t ret;

    int len = strlen(pString);

    ret = SerialManager_WriteBlocking((serial_write_handle_t)s_lpWriteHandle, (uint8_t *)pString, len);
    assert( kStatus_SerialManager_Success == ret );

    NOT_USED(ret);
}

static void BleApp_SerialWriteDec(uint32_t nb)
{
    serial_manager_status_t ret;

    ret = SerialManager_WriteBlocking((serial_write_handle_t)s_lpWriteHandle, FORMAT_Dec2Str(nb), strlen((char const*)FORMAT_Dec2Str(nb)));
    assert( kStatus_SerialManager_Success == ret );

    NOT_USED(ret);
}

static void BleApp_SerialRxCb( void *params, serial_manager_callback_message_t *message, serial_manager_status_t status)
{
    BleApp_SerialWrite(">");

    while (true)
    {
        char     ichar[2];
        uint32_t wlen;

        ichar[0] = 0x0;
        ichar[1] = 0x0;

        (void)SerialManager_TryRead((serial_read_handle_t)s_lpReadHandle, (uint8_t*)ichar, 1U, &wlen);

        if( (wlen == 0u) )
        {
            break;
        }
        if( ichar[0] == 0x0 )
        {
            /* report 'null char' : Likely a wakeup from lowpower, the first byte is lost */
            BleApp_SerialWrite("null char");
            break;
        }

        /* Echo to the host */
        BleApp_SerialWrite((const char*)ichar);
    }

    BleApp_SerialWrite("\r\n");
}
#endif

/*! *********************************************************************************
* @}
********************************************************************************** */
