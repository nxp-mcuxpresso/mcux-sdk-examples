/*! *********************************************************************************
* \addtogroup LowPowerRefDes
* @{
********************************************************************************** */
/*! *********************************************************************************
* Copyright 2019-2024 NXP
* All rights reserved.
*
* \file
*
* This file is the header file for the lowpower central reference design application
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
#include "fsl_component_panic.h"
#include "fsl_component_timer_manager.h"
#include "fsl_shell.h"
#include "fsl_format.h"
#include "fsl_component_mem_manager.h"
#include "app.h"
#include "FunctionLib.h"
#include "PWR_Interface.h"
#include "fwk_platform.h"
#include "fwk_debug.h"

/* BLE Host Stack */
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"

/* Profile / Services */
#include "temperature_interface.h"

#include "ble_conn_manager.h"
#include "ble_service_discovery.h"

#include "app_conn.h"
#include "app_scanner.h"
#include "lowpower_central.h"

#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
#include "enhanced_notification.h"
#endif

/************************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/

#if (gAppLowPowerConstraintInNoBleActivity_c<4)
#define LP_CENTRAL_APP_MESSAGE_NO_ACTIVITY       "\r\nGoing into lowpower (RAMRET)\r\n"
#else
#define LP_CENTRAL_APP_MESSAGE_NO_ACTIVITY       "\r\nGoing into lowpower (RAMOFF)\r\n"
#endif

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
typedef struct bacConfig_tag
{
    uint16_t                    hService;
    uint16_t                    hBattery;
    uint16_t                    hBatteryCccd;
    uint16_t                    hBatteryDesc;
    gattDbCharPresFormat_t      batteryFormat;
} bacConfig_t;

typedef enum appEvent_tag{
    mAppEvt_PeerConnected_c,
    mAppEvt_PairingComplete_c,
    mAppEvt_ServiceDiscoveryComplete_c,
    mAppEvt_ServiceDiscoveryFailed_c,
    mAppEvt_GattProcComplete_c,
    mAppEvt_GattProcError_c
}appEvent_t;

typedef enum appState_tag{
    mAppIdle_c,
    mAppExchangeMtu_c,
    mAppServiceDisc_c,
    mAppReadDescriptor_c,
    mAppConfigureNotification_c,
    mAppRunning_c,
}appState_t;

typedef struct appCustomInfo_tag
{
    tmcConfig_t     tempClientConfig;
    bacConfig_t     batteryClientConfig;
    bool_t          tempDescRead;
    bool_t          tempNotifyEnabled;
    bool_t          battDescRead;
    bool_t          battNotifyEnabled;
    /* Add persistent information here */
}appCustomInfo_t;

typedef struct appPeerInfo_tag
{
    deviceId_t      deviceId;
    appCustomInfo_t customInfo;
    bool_t          isBonded;
    appState_t      appState;
}appPeerInfo_t;

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
static appPeerInfo_t mPeerInformation;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
static bool_t mRestoringBondedLink = FALSE;
static bool_t mAuthRejected = FALSE;
#endif

static bool_t   mScanningOn = FALSE;
static bool_t   mFoundDeviceToConnect = FALSE;

#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
static bool_t mWaitForControllerPrivacy = FALSE;
#endif

/* Buffer used for Characteristic related procedures */
static gattAttribute_t      *mpCharProcBuffer = NULL;


#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
/*shell handle*/
static SHELL_HANDLE_DEFINE(mShellHandle);
#endif

/* Timers */
static TIMER_MANAGER_HANDLE_DEFINE(mAppTimerId);

/************************************************************************************
*************************************************************************************
* Private functions prototypes
*************************************************************************************
************************************************************************************/

/* Host Stack callbacks */
static void BleApp_ScanningCallback(gapScanningEvent_t* pScanningEvent);
static void BleApp_ConnectionCallback(deviceId_t peerDeviceId, gapConnectionEvent_t* pConnectionEvent);
static void BleApp_GattClientCallback(deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t procedureResult, bleResult_t error);
static void BleApp_GattNotificationCallback(deviceId_t serverDeviceId, uint16_t characteristicValueHandle, uint8_t* aValue, uint16_t valueLength);
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent);

/* Application private prototypes */
static void BluetoothLEHost_Initialized(void);
static void BleApp_Start(void);
static void BleApp_StartInit(void);
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event);
static void BleApp_StoreServiceHandles(gattService_t *pService);
static void BleApp_StoreDescValues(gattAttribute_t *pDesc);
static void BleApp_PrintTemperature(uint16_t temperature);
static void BleApp_PrintBattery(uint8_t battery);
static void BleApp_SetLowPowerModeConstraint(uint8_t lowpower_mode_constraint);
static void BleApp_ReleaseLowPowerModeConstraint(uint8_t lowpower_mode_constraint);
static void BleApp_LowpowerInit(void);

#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d ==1)
static bool_t BleApp_CheckExtScanEvent(gapExtScannedDevice_t* pData);
#endif
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData);

static bleResult_t BleApp_ConfigureTemperatureNotifications(void);
static bleResult_t BleApp_ConfigureBatteryNotifications(void);

static void BleApp_PrintLePhyEvent(gapPhyEvent_t* pPhyEvent);

/* Timer callbacks */
static void ScanningTimeoutTimerCallback(void* pParam);
static void DisconnectTimerCallback(void* pParam);
#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
static void WakeUpTimerCallback(void* pParam);
#endif

static appScanningParams_t mAppScanParams = {
    .pHostScanParams = &gScanParams,
    .enableDuplicateFiltering = gGapDuplicateFilteringEnable_c,
    .duration = gGapScanContinuously_d,
    .period = gGapScanPeriodicDisabled_d
};

/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
#define SHELL_NEWLINE()    SHELL_Write((shell_handle_t)mShellHandle, "\n\r", 2U)
#define shell_write(a)     SHELL_Write((shell_handle_t)mShellHandle, (char *)a, strlen(a))
#define shell_writeN(a,b)  SHELL_Write((shell_handle_t)mShellHandle, a, b)
#define shell_writeDec(a)  SHELL_Write((shell_handle_t)mShellHandle, (char *)FORMAT_Dec2Str(a), strlen((const char *)FORMAT_Dec2Str(a)))
#else
#define SHELL_NEWLINE()
#define shell_write(a)
#define shell_writeN(a,b)
#define shell_writeDec(a)
#endif

/*! *********************************************************************************
* \brief    Initializes application specific functionality before the BLE stack init.
*
********************************************************************************** */
void BluetoothLEHost_AppInit(void)
{
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
    /* shell_init is done in hardware_init, logo display is delayed to now
       to be sure Serial Manager task is ready to be used */
    SHELL_Init((shell_handle_t)mShellHandle ,(serial_handle_t)gSerMgrIf, "BLE Lowpower Central>");
#endif

    /*Install callback for button*/
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
    BUTTON_InstallCallback((button_handle_t)g_buttonHandle[0], BleApp_HandleKeys0, NULL);
#if (gAppButtonCnt_c > 1)
    BUTTON_InstallCallback((button_handle_t)g_buttonHandle[1], BleApp_HandleKeys1, NULL);
#endif /*gAppButtonCnt_c > 1*/
#endif /*gAppButtonCnt_c > 0*/

    /* Register generic callback */
    BluetoothLEHost_SetGenericCallback(BleApp_GenericCallback);

    /* Initialize Bluetooth Host Stack */
    BluetoothLEHost_Init(BluetoothLEHost_Initialized);
}

/*! *********************************************************************************
* \brief    Starts the BLE application.
*
********************************************************************************** */
static void BleApp_Start(void)
{
    if (!mScanningOn)
    {
#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
        /* enable controller scan notifications */
        BleApp_EnableControllerNotifications(gNotifSetScan_c);
#endif

        /* Set Lowpower Constraint for Scanning activity */
        BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInScanning_c);

        /* Start scanning */
        (void)BluetoothLEHost_StartScanning(&mAppScanParams, BleApp_ScanningCallback);
    }

    APP_DBG_LOG("");
    DBG_LOG_DUMP();
}

/*! *********************************************************************************
* \brief    Starts the BLE application for the first time
*
********************************************************************************** */
static void BleApp_StartInit(void)
{
    /* Start the BLE activity, typical in this case, Scanning */
    BleApp_Start();

    /* Update Lowpower constraint for this application */
    BleApp_LowpowerInit();
}

/*! *********************************************************************************
* \brief        Handles keyboard events.
*
* \param[in]    events    Key event structure.
********************************************************************************** */
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
button_status_t BleApp_HandleKeys0(void *buttonHandle, button_callback_message_t *message,void *callbackParam)
{
    APP_DBG_LOG("0");
    switch (message->event)
    {
        case kBUTTON_EventOneClick:
        case kBUTTON_EventShortPress:
        {
            if (mScanningOn)
            {
                APP_DBG_LOG("Stopping scan");
                (void)Gap_StopScanning();
            }
            else
            {
                APP_DBG_LOG("Starting scan");
                BleApp_Start();
            }

            break;
        }

        case kBUTTON_EventLongPress:
            /* No action required */
            break;

        default:
            /* No action required */
            break;
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
                shell_write("\r\nLP disabled.\r\n");
            }
            else
            {
                /* Release the WFI constraint so the device can enter deeper power modes on idle */
                BleApp_ReleaseLowPowerModeConstraint(gAppWFIConstraint_c);
                lpEnabled = true;
                shell_write("\r\nLP enabled.\r\n");
            }
            break;

        case kBUTTON_EventLongPress:
            shell_write("\r\nButton1 long press !\r\n");
            break;

        default:
            /* No action required */
            break;
    }
    DBG_LOG_DUMP();

    return kStatus_BUTTON_Success;
}
#endif /*gAppButtonCnt_c > 0*/

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
        case gLePhyEvent_c:
        {
            if(pGenericEvent->eventData.phyEvent.phyEventType == gPhyUpdateComplete_c )
            {
                BleApp_PrintLePhyEvent(&pGenericEvent->eventData.phyEvent);
            }
        }
        break;

#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1)
        case gControllerPrivacyStateChanged_c:
        {
            if(pGenericEvent->eventData.newControllerPrivacyState == TRUE)
            {
                if(mWaitForControllerPrivacy == TRUE)
                {
                    /* Controller Privacy is ready, we can start scan */
                    BleApp_StartInit();
                    mWaitForControllerPrivacy = FALSE;
                }
            }
        }
        break;
#endif

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

/*! *********************************************************************************
* \brief        Configures BLE Stack after initialization. Usually used for
*               configuring advertising, scanning, white list, services, et al.
*
********************************************************************************** */
static void BluetoothLEHost_Initialized(void)
{
    /* Configure as GAP Central */
    BleConnManager_GapCommonConfig();

    /* Register for callbacks*/
    (void)App_RegisterGattClientProcedureCallback(BleApp_GattClientCallback);
    (void)App_RegisterGattClientNotificationCallback(BleApp_GattNotificationCallback);
    BleServDisc_RegisterCallback(BleApp_ServiceDiscoveryCallback);

    /* Initialize private variables */
    mPeerInformation.appState = mAppIdle_c;
    mScanningOn = FALSE;
    mFoundDeviceToConnect = FALSE;

    /* Allocate scan timeout timer */
    (void)TM_Open((timer_handle_t)mAppTimerId);

    /* Update UI */

    PLATFORM_ResetStatus_t resetStatus;

    (void)PLATFORM_GetResetCause(&resetStatus);
    if( resetStatus == PLATFORM_LowPowerWakeup )
    {
        shell_write("\r\nWake up from RAMOFF\r\n");
    }
    else
    {
        shell_write("\r\ntrying to connect to a lowpower peripheral refdes application\r\n");
#if (defined(gAppButtonCnt_c) && (gAppButtonCnt_c > 0))
        shell_write("Short press SW2 to start/stop scanning\r\n");
#if (gAppButtonCnt_c > 1)
        shell_write("Short press SW3 to enable/disable low power\r\n");
        shell_write("Long press SW3 to print a message\r\n");
#endif /* gAppButtonCnt_c > 1 */
#endif
    }

#if (defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d == 1U)) && \
    (defined(gAppUseBonding_d) && (gAppUseBonding_d == 1U))
    if (gcBondedDevices == 0U)
    {
        /* if no devices are bonded, we can start scan immediately */
        BleApp_StartInit();
    }
    else
    {
        /* will wait event from Controller before starting scan */
        mWaitForControllerPrivacy = TRUE;
    }
#else
    BleApp_StartInit();
#endif
}

/*! *********************************************************************************
* \brief        Handles BLE Scanning callback from host stack.
*
* \param[in]    pScanningEvent    Pointer to gapScanningEvent_t.
********************************************************************************** */
static void BleApp_ScanningCallback (gapScanningEvent_t* pScanningEvent)
{
    switch (pScanningEvent->eventType)
    {
        case gDeviceScanned_c:
#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d ==1)
        case gExtDeviceScanned_c:
#endif
        {
            /* Check if the scanned device implements the Temperature Custom Profile */
            if( FALSE == mFoundDeviceToConnect )
            {
#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d ==1)
                 if (gExtDeviceScanned_c == pScanningEvent->eventType)
                  {
                    mFoundDeviceToConnect = BleApp_CheckExtScanEvent(&pScanningEvent->eventData.extScannedDevice);
                  }
                 else
#endif
                 {
                    mFoundDeviceToConnect = BleApp_CheckScanEvent(&pScanningEvent->eventData.scannedDevice);
                 }

                if (mFoundDeviceToConnect)
                {
                    /* Set connection parameters and stop scanning. Connect on gScanStateChanged_c. */
                    gConnReqParams.peerAddressType = pScanningEvent->eventData.scannedDevice.addressType;
                    FLib_MemCpy(gConnReqParams.peerAddress,
                                pScanningEvent->eventData.scannedDevice.aAddress,
                                sizeof(bleDeviceAddress_t));

                    (void)Gap_StopScanning();
#if defined(gAppUsePrivacy_d) && (gAppUsePrivacy_d)
                    gConnReqParams.usePeerIdentityAddress = pScanningEvent->eventData.scannedDevice.advertisingAddressResolved;
#endif
                }
            }
        }
        break;

        case gScanStateChanged_c:
        {
            mScanningOn = !mScanningOn;

            /* Node starts scanning */
            if (mScanningOn)
            {
                mFoundDeviceToConnect = FALSE;

                shell_write("\r\nScanning...\r\n");
                APP_DBG_LOG("Scanning");

                /* Start scanning timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, ScanningTimeoutTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer , gAppScanningTimeout_c);
            }
            /* Node scanning */
            else
            {
                (void)TM_Stop((timer_handle_t)mAppTimerId);

                shell_write("\r\nScanning stopped!\r\n");
                APP_DBG_LOG("Scanning stopped");

                /* Release Lowpower Constraint for Scanning activity */
                BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInScanning_c);

                /* Connect with the previously scanned peer device */
                if (mFoundDeviceToConnect)
                {
                    /* Set Lowpower Constraint for Connected activity
                           TODO : if connection establihement fails, make sure to release the constraint */
                    BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);

                    (void)BluetoothLEHost_Connect(&gConnReqParams, BleApp_ConnectionCallback);
                }
                else
                {
#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d == 1)
                    shell_write(LP_CENTRAL_APP_MESSAGE_NO_ACTIVITY);
#endif

#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
                    (void)TM_InstallCallback((timer_handle_t)mAppTimerId, WakeUpTimerCallback, NULL);
                    (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSingleShot , gAppWakeUpTimerAfterNoActivityMs);
#endif
                }
            }
        }
        break;

        case gScanCommandFailed_c:
        {
            ; /* No action required */
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
    BleConnManager_GapCentralEvent(peerDeviceId, pConnectionEvent);

    switch (pConnectionEvent->eventType)
    {
        case gConnEvtConnected_c:
        {
            /* Update UI */
            shell_write("\r\nConnected!\r\n");
            APP_DBG_LOG("Connected");

            mPeerInformation.deviceId = peerDeviceId;
            mPeerInformation.isBonded = FALSE;

            /* Apply Lowpower Mode constraint, currently done in Connection request  */
            //BleApp_SetLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);

#if defined(gAppMonitorControllerNotifications_d) && (gAppMonitorControllerNotifications_d == 1)
            /* enable controller conn notifications */
            BleApp_EnableControllerNotifications(gNotifSetConn_c);
#endif

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
            (void)Gap_CheckIfBonded(peerDeviceId, &mPeerInformation.isBonded, NULL);

            if ((mPeerInformation.isBonded) &&
                (gBleSuccess_c == Gap_LoadCustomPeerInformation(peerDeviceId,
                    (void*) &mPeerInformation.customInfo, 0, sizeof (appCustomInfo_t))))
            {
                mRestoringBondedLink = TRUE;
                /* Restored custom connection information. Encrypt link */
                (void)Gap_EncryptLink(peerDeviceId);
            }
            else
            {
                mRestoringBondedLink = FALSE;
                mAuthRejected = FALSE;
            }
#endif
            BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PeerConnected_c);
        }
        break;

        case gConnEvtDisconnected_c:
        {
            mPeerInformation.deviceId = gInvalidDeviceId_c;
            mPeerInformation.appState = mAppIdle_c;

            /* Reset Service Discovery to be sure*/
            BleServDisc_Stop(peerDeviceId);

            (void)TM_Stop((timer_handle_t)mAppTimerId);

            /* Update UI */
            shell_write("\r\nDisconnected!\r\n");
            APP_DBG_LOG("Disconnected");

            BleApp_ReleaseLowPowerModeConstraint(gAppLowPowerConstraintInConnected_c);

#if gAppRestartScanAfterConnect
            BleApp_Start();
#else

#if defined(gAppLowpowerEnabled_d) && (gAppLowpowerEnabled_d == 1)
            shell_write(LP_CENTRAL_APP_MESSAGE_NO_ACTIVITY);
#endif

#if defined(gAppWakeUpTimerAfterNoActivityMs) && (gAppWakeUpTimerAfterNoActivityMs != 0U)
            (void)TM_InstallCallback((timer_handle_t)mAppTimerId, WakeUpTimerCallback, NULL);
            (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSingleShot , gAppWakeUpTimerAfterNoActivityMs);
#endif
#endif  // gAppRestartScanAfterConnect

#if defined(LP_APP_TEST_REMOVE_BONDING)
            /* test only : remove Bonding for stress test */
            Gap_RemoveAllBonds();
#endif
        }
        break;

#if gAppUsePairing_d
        case gConnEvtPairingComplete_c:
        {
            /* Notify state machine handler on pairing complete */
            if (pConnectionEvent->eventData.pairingCompleteEvent.pairingSuccessful)
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                mAuthRejected = FALSE;
#endif
                shell_write("Pairing successful\r\n");
                BleApp_StateMachineHandler(mPeerInformation.deviceId, mAppEvt_PairingComplete_c);
            }
            else
            {
                shell_write("Pairing unsuccessful\r\n");
            }
        }
        break;

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
        case gConnEvtEncryptionChanged_c:
        {
            if( pConnectionEvent->eventData.encryptionChangedEvent.newEncryptionState )
            {
                if ( (TRUE == mRestoringBondedLink) &&
                     (FALSE == mAuthRejected) )
                {
                    /* Try to enable temperature notifications, disconnect on failure */
                    if( gBleSuccess_c != BleApp_ConfigureTemperatureNotifications() )
                    {
                        (void)Gap_Disconnect(peerDeviceId);
                    }
                    else
                    {
                        mRestoringBondedLink = FALSE;
                    }
                }
            }
        }
        break;

        case gConnEvtAuthenticationRejected_c:
        {
            mAuthRejected = TRUE;
            /* Start Pairing Procedure */
            (void)Gap_Pair(peerDeviceId, &gPairingParameters);
        }
        break;
#endif /* gAppUseBonding_d */
#endif /* gAppUsePairing_d */

    default:
        ; /* No action required */
        break;
    }
}

/*! *********************************************************************************
* \brief        Handles discovered services.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    pEvent              Pointer to servDiscEvent_t.
********************************************************************************** */
static void BleApp_ServiceDiscoveryCallback(deviceId_t peerDeviceId, servDiscEvent_t* pEvent)
{
    switch(pEvent->eventType)
    {
        /* Store the discovered handles for later use. */
        case gServiceDiscovered_c:
        {
            BleApp_StoreServiceHandles(pEvent->eventData.pService);
        }
        break;

        /* Service discovery has finished, run the state machine. */
        case gDiscoveryFinished_c:
        {
            if (pEvent->eventData.success)
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryComplete_c);
            }
            else
            {
                BleApp_StateMachineHandler(peerDeviceId, mAppEvt_ServiceDiscoveryFailed_c);
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

/*! *********************************************************************************
* \brief        Stores handles for the specified service.
*
* \param[in]    pService    Pointer to gattService_t.
********************************************************************************** */
static void BleApp_StoreServiceHandles(gattService_t *pService)
{
    uint8_t i,j;

    if ((pService->uuidType == gBleUuidType128_c) &&
        FLib_MemCmp(pService->uuid.uuid128, uuid_service_temperature, 16))
    {
        /* Found Primary Temperature Service */
        mPeerInformation.customInfo.tempClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_Temperature_d))
            {
                /* Found Temperature Characteristic */
                mPeerInformation.customInfo.tempClientConfig.hTemperature = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c)
                    {
                        switch (pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16)
                        {
                            /* Found Temperature Characteristic Presentation Format Descriptor */
                            case gBleSig_CharPresFormatDescriptor_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempDesc = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            /* Found Temperature Characteristic CCCD */
                            case gBleSig_CCCD_d:
                            {
                                mPeerInformation.customInfo.tempClientConfig.hTempCccd = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            default:
                                ; /* No action required */
                                break;
                        }
                    }
                }
            }
        }
    }
    else if((pService->uuidType == gBleUuidType16_c) &&
             (pService->uuid.uuid16 == (uint16_t)gBleSig_BatteryService_d) )
    {
        /* Found Primary Battery service */
        mPeerInformation.customInfo.batteryClientConfig.hService = pService->startHandle;

        for (i = 0; i < pService->cNumCharacteristics; i++)
        {
            if ((pService->aCharacteristics[i].value.uuidType == gBleUuidType16_c) &&
                (pService->aCharacteristics[i].value.uuid.uuid16 == gBleSig_BatteryLevel_d))
            {
                /* Found Battery Level Characteristic */
                mPeerInformation.customInfo.batteryClientConfig.hBattery = pService->aCharacteristics[i].value.handle;

                for (j = 0; j < pService->aCharacteristics[i].cNumDescriptors; j++)
                {
                    if (pService->aCharacteristics[i].aDescriptors[j].uuidType == gBleUuidType16_c)
                    {
                        switch (pService->aCharacteristics[i].aDescriptors[j].uuid.uuid16)
                        {
                            /* Found Battery Characteristic Presentation Format Descriptor */
                            case gBleSig_CharPresFormatDescriptor_d:
                            {
                                mPeerInformation.customInfo.batteryClientConfig.hBatteryDesc = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            /* Found Battery Characteristic CCCD */
                            case gBleSig_CCCD_d:
                            {
                                mPeerInformation.customInfo.batteryClientConfig.hBatteryCccd = pService->aCharacteristics[i].aDescriptors[j].handle;
                                break;
                            }
                            default:
                                ; /* No action required */
                                break;
                        }
                    }
                }
            }
        }
    }
    else
    {
        ; /* service is skipped */
    }
}

/*! *********************************************************************************
* \brief        Stores the value of the specified attribute.
*
* \param[in]    pDesc       Pointer to gattAttribute_t.
********************************************************************************** */
static void BleApp_StoreDescValues(gattAttribute_t *pDesc)
{
    if (pDesc->handle == mPeerInformation.customInfo.tempClientConfig.hTempDesc)
    {
        /* Store temperature format*/
        FLib_MemCpy(&mPeerInformation.customInfo.tempClientConfig.tempFormat,
                    pDesc->paValue,
                    pDesc->valueLength);
        mPeerInformation.customInfo.tempDescRead = TRUE;
    }
    else
    {
        /* in current implementation, must be battery service handle */
        assert(pDesc->handle == mPeerInformation.customInfo.batteryClientConfig.hBatteryDesc);

        /* Store battery level format*/
        FLib_MemCpy(&mPeerInformation.customInfo.batteryClientConfig.batteryFormat,
                    pDesc->paValue,
                    pDesc->valueLength);
        mPeerInformation.customInfo.battDescRead = TRUE;
    }
}

/*! *********************************************************************************
* \brief        Displays the temperature value to shell.
*
* \param[in]    temperature     Temperature value
********************************************************************************** */
static void BleApp_PrintTemperature(uint16_t temperature)
{
    APP_DBG_LOG("");

    shell_write("Temperature: ");
    shell_writeDec((uint32_t)temperature / 100UL);

    /* Add 'C' for Celsius degrees - UUID 0x272F.
       www.bluetooth.com/specifications/assigned-numbers/units */
    if (mPeerInformation.customInfo.tempClientConfig.tempFormat.unitUuid16 == 0x272FU)
    {
        shell_write(" C\r\n");
    }
    else
    {
        shell_write("\r\n");
    }
}

/*! *********************************************************************************
* \brief        Displays the battery level to shell.
*
* \param[in]    battery     Battery level
********************************************************************************** */
static void BleApp_PrintBattery(uint8_t battery)
{
    APP_DBG_LOG("");

    shell_write("Battery: ");
    shell_writeDec((uint32_t)battery);

    /* Add '%' for percentage - UUID 0x27AD.
       www.bluetooth.com/specifications/assigned-numbers/units */
    if (mPeerInformation.customInfo.batteryClientConfig.batteryFormat.unitUuid16 == 0x27ADU)
    {
        shell_write(" %\r\n");
    }
    else
    {
        shell_write("\r\n");
    }
}

/*! *********************************************************************************
* \brief        Handles GATT client callback from host stack.
*
* \param[in]    serverDeviceId      GATT Server device ID.
* \param[in]    procedureType       Procedure type.
* \param[in]    procedureResult     Procedure result.
* \param[in]    error               Callback result.
********************************************************************************** */
static void BleApp_GattClientCallback(deviceId_t serverDeviceId, gattProcedureType_t procedureType, gattProcedureResult_t procedureResult, bleResult_t error)
{
    if (procedureResult == gGattProcError_c)
    {
#if (defined(gAppUsePairing_d) && (gAppUsePairing_d == 1))
        attErrorCode_t attError = (attErrorCode_t)(uint8_t)(error);

        if (attError == gAttErrCodeInsufficientEncryption_c     ||
            attError == gAttErrCodeInsufficientAuthorization_c  ||
            attError == gAttErrCodeInsufficientAuthentication_c)
        {
            /* Start Pairing Procedure */
            (void)Gap_Pair(serverDeviceId, &gPairingParameters);
        }
#endif
        BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcError_c);
    }
    else
    {
        if (procedureResult == gGattProcSuccess_c)
        {
            switch(procedureType)
            {
                case gGattProcReadCharacteristicDescriptor_c:
                {
                    if( mpCharProcBuffer != NULL )
                    {
                        /* Store the value of the descriptor */
                        BleApp_StoreDescValues(mpCharProcBuffer);
                    }
                }
                break;

                case gGattProcWriteCharacteristicDescriptor_c:
                {
                    if( mpCharProcBuffer != NULL )
                    {
                        if( mPeerInformation.customInfo.tempNotifyEnabled == FALSE )
                        {
                            mPeerInformation.customInfo.tempNotifyEnabled = TRUE;
                        }
                        else
                        {
                            mPeerInformation.customInfo.battNotifyEnabled = TRUE;
                        }
                    }
                }
                break;

                default:
                {
                    ; /* No action required */
                }
                break;
            }

            BleApp_StateMachineHandler(serverDeviceId, mAppEvt_GattProcComplete_c);
        }
    }

    /* Signal Service Discovery Module */
    BleServDisc_SignalGattClientEvent(serverDeviceId, procedureType, procedureResult, error);

}

/*! *********************************************************************************
* \brief        Handles GATT client notification callback from host stack.
*
* \param[in]    serverDeviceId              GATT Server device ID.
* \param[in]    characteristicValueHandle   Handle.
* \param[in]    aValue                      Pointer to value.
* \param[in]    valueLength                 Value length.
********************************************************************************** */
static void BleApp_GattNotificationCallback(deviceId_t serverDeviceId, uint16_t characteristicValueHandle, uint8_t* aValue, uint16_t valueLength)
{
    APP_DBG_LOG("");
    if (characteristicValueHandle == mPeerInformation.customInfo.tempClientConfig.hTemperature)
    {
        BleApp_PrintTemperature(Utils_ExtractTwoByteValue(aValue));

        /* Restart Wait For Data timer */
        (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer , gAppDisconnectTimeout_c);
    }
    else if (characteristicValueHandle == mPeerInformation.customInfo.batteryClientConfig.hBattery)
    {
        /* battery value is only 1 byte */
        BleApp_PrintBattery(*aValue);
        /* Restart Wait For Data timer */
        (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
        (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer , gAppDisconnectTimeout_c);
    }
    else
    {
        ; /* nothing to do with this notification */
    }

    DBG_LOG_DUMP();
}

/*! *********************************************************************************
* \brief        Detect whether the provided data is found in the advertising data.
*
* \param[in]    pElement                    Pointer a to AD structure.
* \param[in]    pData                       Data to look for.
* \param[in]    iDataLen                    Size of data to look for.
*
* \return       TRUE if data matches, FALSE if not
********************************************************************************** */
static bool_t MatchDataInAdvElementList(gapAdStructure_t *pElement, void *pData, uint8_t iDataLen)
{
    uint32_t i;
    bool_t status = FALSE;

    for (i = 0; i < (uint32_t)pElement->length - 1UL; i += iDataLen)
    {
        /* Compare input data with advertising data. */
        if (FLib_MemCmp(pData, &pElement->aData[i], iDataLen))
        {
            status = TRUE;
            break;
        }
    }
    return status;
}

/*! *********************************************************************************
* \brief        Process scanning events to search for the Temperature Custom Service.
                In case extended advertisement is scanned.
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to gapExtScannedDevice_t.
*
* \return       TRUE if the scanned device implements the Temperature Custom Service,
                FALSE otherwise
********************************************************************************** */
#if defined(gAppExtAdvEnable_d) && (gAppExtAdvEnable_d ==1)
static bool_t BleApp_CheckExtScanEvent(gapExtScannedDevice_t* pData)
{
    uint32_t index = 0U;
    uint8_t name[10];
    uint8_t nameLength = 0U;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->pData[index];
        adElement.adType = (gapAdType_t)pData->pData[index + 1U];
        adElement.aData = &pData->pData[index + 2U];

         /* Search for Temperature Custom Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c) ||
          (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid_service_temperature, 16);
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch && (nameLength > 0U))
    {
        /* Update UI */
        shell_write("\r\nFound ExtAdv device: \r\n");
        shell_writeN((char *)name, (uint16_t)nameLength - 1U);
        SHELL_NEWLINE();
        shell_writeN((char *)FORMAT_Hex2Ascii(pData->aAddress[5]), 2);
        for(uint8_t i = 0U; i < 5U; i++)
        {
            shell_write(":");
            shell_writeN((char *)FORMAT_Hex2Ascii(pData->aAddress[4 - i]), 2);
        }
        SHELL_NEWLINE();
        shell_write("Tx Power: ");
        shell_writeN((char *)FORMAT_Dec2Str(pData->txPower),2);           /*Prints the txPower*/
        SHELL_NEWLINE();
        shell_write("Primary PHY: ");                   /*Prints the Primary PHY config*/
        shell_writeN((char *)FORMAT_Dec2Str(pData->primaryPHY),2);       /*1: 1M, 2: 2M, 3: Coded*/
        SHELL_NEWLINE();
        shell_write("Secondary PHY: ");                 /*Prints the Secondary PHY config*/
        shell_writeN((char *)FORMAT_Dec2Str(pData->secondaryPHY),2);     /*1: 1M, 2: 2M, 3: Coded*/
        SHELL_NEWLINE();
    }
    return foundMatch;
}
#endif

/*! *********************************************************************************
* \brief        Process scanning events to search for the Temperature Custom Service.
*               This function is called from the scanning callback.
*
* \param[in]    pData                   Pointer to gapScannedDevice_t.
*
* \return       TRUE if the scanned device implements the Temperature Custom Service,
                FALSE otherwise
********************************************************************************** */
static bool_t BleApp_CheckScanEvent(gapScannedDevice_t* pData)
{
    uint32_t index = 0U;
    uint8_t name[10];
    uint8_t nameLength = 0U;
    bool_t foundMatch = FALSE;

    while (index < pData->dataLength)
    {
        gapAdStructure_t adElement;

        adElement.length = pData->data[index];
        adElement.adType = (gapAdType_t)pData->data[index + 1U];
        adElement.aData = &pData->data[index + 2U];

         /* Search for Temperature Custom Service */
        if ((adElement.adType == gAdIncomplete128bitServiceList_c) ||
          (adElement.adType == gAdComplete128bitServiceList_c))
        {
            foundMatch = MatchDataInAdvElementList(&adElement, &uuid_service_temperature, 16);
        }

        if ((adElement.adType == gAdShortenedLocalName_c) ||
          (adElement.adType == gAdCompleteLocalName_c))
        {
            nameLength = MIN(adElement.length, 10U);
            FLib_MemCpy(name, adElement.aData, nameLength);
        }

        /* Move on to the next AD element type */
        index += (uint32_t)adElement.length + sizeof(uint8_t);
    }

    if (foundMatch && (nameLength > 0U))
    {
        /* Update UI */
        shell_write("\r\nFound device: \r\n");
        shell_writeN((char *)name, (uint16_t)nameLength - 1U);
        SHELL_NEWLINE();
        shell_writeN((char *)FORMAT_Hex2Ascii(pData->aAddress[5]), 2);
        for(uint8_t i = 0U; i < 5U; i++)
        {
            shell_write(":");
            shell_writeN((char *)FORMAT_Hex2Ascii(pData->aAddress[4 - i]), 2);
        }
    }
    return foundMatch;
}

/*! *********************************************************************************
* \brief        State machine handler of the Temperature Collector application.
*
* \param[in]    peerDeviceId        Peer device ID.
* \param[in]    event               Event type.
********************************************************************************** */
static void BleApp_StateMachineHandler(deviceId_t peerDeviceId, appEvent_t event)
{
    switch (mPeerInformation.appState)
    {
        case mAppIdle_c:
        {
            if (event == mAppEvt_PeerConnected_c)
            {
#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                if( (mPeerInformation.customInfo.tempClientConfig.hTemperature != gGattDbInvalidHandle_d) &&\
                    (mPeerInformation.customInfo.batteryClientConfig.hBattery != gGattDbInvalidHandle_d) )
                {
                    /* Moving to Notification Configuration state and wait for Link encryption result */
                    mPeerInformation.appState = mAppConfigureNotification_c;
                }
                else
#endif
                {
                    /* Moving to Exchange MTU State */
                    mPeerInformation.appState = mAppExchangeMtu_c;
                    (void)GattClient_ExchangeMtu(peerDeviceId, gAttMaxMtu_c);
                }
            }
        }
        break;

        case mAppExchangeMtu_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                /* Moving to Service Discovery State*/
                mPeerInformation.appState = mAppServiceDisc_c;

                /* Start Service Discovery*/
                (void)BleServDisc_Start(peerDeviceId);
            }
            else
            {
                if (event == mAppEvt_GattProcError_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;

        case mAppServiceDisc_c:
        {
            if (event == mAppEvt_ServiceDiscoveryComplete_c)
            {
                /* Moving to Primary Service Discovery State*/
                mPeerInformation.appState = mAppReadDescriptor_c;

                if (mPeerInformation.customInfo.tempClientConfig.hTempDesc != 0U)
                {
                    mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);

                    if (mpCharProcBuffer == NULL)
                    {
                        assert(0);
                        (void)Gap_Disconnect(peerDeviceId);
                    }
                    else
                    {
                        mpCharProcBuffer->handle = mPeerInformation.customInfo.tempClientConfig.hTempDesc;
                        mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
                        (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation.deviceId, mpCharProcBuffer, gAttDefaultMtu_c);
                    }
                }
            }
            else
            {
                if (event == mAppEvt_ServiceDiscoveryFailed_c)
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
        }
        break;

        case mAppReadDescriptor_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if( mPeerInformation.customInfo.tempDescRead == TRUE )
                {
                    if( mPeerInformation.customInfo.battDescRead == FALSE )
                    {
                        /* once Temperature Descriptor has been read, read Battery Descriptor */
                        mpCharProcBuffer->handle = mPeerInformation.customInfo.batteryClientConfig.hBatteryDesc;
                        mpCharProcBuffer->paValue = (uint8_t*)(mpCharProcBuffer + 1);
                        (void)GattClient_ReadCharacteristicDescriptor(mPeerInformation.deviceId, mpCharProcBuffer, gAttDefaultMtu_c);
                    }
                    else
                    {
                        /* Try to enable Temperature notifications, disconnect on failure
                           Battery notifications will be enabled in mAppConfigureNotification_c state */
                        if( (gBleSuccess_c == BleApp_ConfigureTemperatureNotifications()) )
                        {
                            mPeerInformation.appState = mAppConfigureNotification_c;
                        }
                        else
                        {
                            (void)Gap_Disconnect(peerDeviceId);
                        }
                    }
                }
            }
            else
            {
                if (event == mAppEvt_PairingComplete_c)
                {
                    /* Continue after pairing is complete */
                    (void)GattClient_ReadCharacteristicDescriptor(peerDeviceId, mpCharProcBuffer, gAttDefaultMtu_c);
                }
            }
        }
        break;

        case mAppConfigureNotification_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                assert( mPeerInformation.customInfo.batteryClientConfig.hBatteryCccd != 0U );

                /* Try to enable Battery notifications, disconnect on failure */
                if( (gBleSuccess_c == BleApp_ConfigureBatteryNotifications()) )
                {
                    /* notification configuration is complete, move to running state */
                    mPeerInformation.appState = mAppRunning_c;
                }
                else
                {
                    (void)Gap_Disconnect(peerDeviceId);
                }
            }
            else
            {
                if(event == mAppEvt_PairingComplete_c)
                {
                    /* Try to enable Temperature notifications, disconnect on failure
                       Battery notifications will be enabled at next mAppEvt_GattProcComplete_c event */
                    if( gBleSuccess_c != BleApp_ConfigureTemperatureNotifications() )
                    {
                        (void)Gap_Disconnect(peerDeviceId);
                    }
                }
            }
        }
        break;

        case mAppRunning_c:
        {
            if (event == mAppEvt_GattProcComplete_c)
            {
                if (mpCharProcBuffer != NULL)
                {
                    (void)MEM_BufferFree(mpCharProcBuffer);
                    mpCharProcBuffer = NULL;
                }

#if defined(gAppUseBonding_d) && (gAppUseBonding_d)
                /* Write data in NVM */
                (void)Gap_SaveCustomPeerInformation(mPeerInformation.deviceId,
                                                    (void *)&mPeerInformation.customInfo, 0,
                                                    sizeof(appCustomInfo_t));
#endif
                /* Start Wait For Data timer */
                (void)TM_InstallCallback((timer_handle_t)mAppTimerId, DisconnectTimerCallback, NULL);
                (void)TM_Start((timer_handle_t)mAppTimerId, kTimerModeLowPowerTimer | kTimerModeSetSecondTimer , gAppDisconnectTimeout_c);
            }
            else
            {
                if (event == mAppEvt_PairingComplete_c)
                {
                    /* Pairing should be completed before being in mAppRunning state */
                    assert(0);
                }
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

/*! *********************************************************************************
* \brief        Enable temperature notifications by writing peer's CCCD of the
                Temperature characteristic.
*
* \return       gBleSuccess_c or gBleOutOfMemory_c
********************************************************************************** */
static bleResult_t BleApp_ConfigureTemperatureNotifications(void)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = (uint16_t)gCccdNotification_c;

    /* Allocate buffer for the write operation */
    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( mpCharProcBuffer != NULL )
    {
        /* Prepare CCCD write request */
        mpCharProcBuffer->handle = mPeerInformation.customInfo.tempClientConfig.hTempCccd;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        /* Write Temperature CCCD */
        (void)GattClient_WriteCharacteristicDescriptor(mPeerInformation.deviceId,
                                                       mpCharProcBuffer,
                                                       (uint16_t)sizeof(value), (void*)&value);
    }
    else
    {
        assert(0);
        result = gBleOutOfMemory_c;
    }

    return result;
}

/*! *********************************************************************************
* \brief        Enable battery notifications by writing peer's CCCD of the
                Battery characteristic.
*
* \return       gBleSuccess_c or gBleOutOfMemory_c
********************************************************************************** */
static bleResult_t BleApp_ConfigureBatteryNotifications(void)
{
    bleResult_t result = gBleSuccess_c;
    uint16_t value = (uint16_t)gCccdNotification_c;

    /* Allocate buffer for the write operation */
    if( mpCharProcBuffer == NULL )
    {
        mpCharProcBuffer = MEM_BufferAlloc(sizeof(gattAttribute_t) + gAttDefaultMtu_c);
    }

    if( mpCharProcBuffer != NULL )
    {
        /* Prepare CCCD write request */
        mpCharProcBuffer->handle = mPeerInformation.customInfo.batteryClientConfig.hBatteryCccd;
        mpCharProcBuffer->uuid.uuid16 = gBleSig_CCCD_d;
        mpCharProcBuffer->valueLength = 0;
        /* Write Battery CCCD */
        (void)GattClient_WriteCharacteristicDescriptor(mPeerInformation.deviceId,
                                                       mpCharProcBuffer,
                                                       (uint16_t)sizeof(value), (void*)&value);
    }
    else
    {
        assert(0);
        result = gBleOutOfMemory_c;
    }

    return result;
}

/*! *********************************************************************************
* \brief        Stop scanning after a given time (gAppScanningTimeout_c).
*               Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void ScanningTimeoutTimerCallback(void* pParam)
{
    /* Stop scanning */
    if (mScanningOn)
    {
         APP_DBG_LOG("");
        (void)Gap_StopScanning();
    }
    else
    {
         APP_DBG_LOG("error");
         assert(0);
    }
}

/*! *********************************************************************************
* \brief        Disconnect from peer device if no data was received for a given time.
*               (gAppDisconnectTimeout_c). Called on timer task.
*
* \param[in]    pParam              not used
********************************************************************************** */
static void DisconnectTimerCallback(void* pParam)
{
    /* Disconnect from peer device */
    if (mPeerInformation.deviceId != gInvalidDeviceId_c)
    {
         APP_DBG_LOG("");
        (void)Gap_Disconnect(mPeerInformation.deviceId);
         DBG_LOG_DUMP();
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
    /* Start scanning again  */
    BleApp_Start();
}
#endif

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
    uint8_t txPhy = (pPhyEvent->txPhy <= gLePhyCoded_c) ? pPhyEvent->txPhy : 0;
    uint8_t rxPhy = (pPhyEvent->rxPhy <= gLePhyCoded_c) ? pPhyEvent->rxPhy : 0;
#if defined(gAppUseSerialManager_c) && (gAppUseSerialManager_c > 0)
    shell_write("Phy Update Complete.\n\r");
    shell_write("TxPhy ");
    shell_write(mLePhyModeStrings[txPhy]);
    shell_write("RxPhy ");
    shell_write(mLePhyModeStrings[rxPhy]);
#else
    NOT_USED(txPhy);
    NOT_USED(rxPhy);
    NOT_USED(mLePhyModeStrings);
#endif
}
/*! *********************************************************************************
* @}
********************************************************************************** */
