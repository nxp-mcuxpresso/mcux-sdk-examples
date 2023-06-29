/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2020 NXP
*
* \file
*
* This is a source file for the main application Zigbee + BLE.
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/************************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/

/* Framework include */
#include "fsl_os_abstraction.h"
#include "MemManager.h"
#include "TimersManager.h"
#include "RNG_Interface.h"
#include "Messaging.h"
#include "Flash_Adapter.h"
#include "SecLib.h"
#include "Panic.h"
#include "LED.h"
#include "Keyboard.h"
#include "SerialManager.h"
#include "PDM.h"
#include "OtaSupport.h"
#include "PWR_Interface.h"
#include "PWR_Configuration.h"
#include "fsl_xcvr.h"

/* KSDK */
#include "board.h"
#include "fsl_reset.h"
#include "MicroSpecific.h"

/* zigbee includes */
#include "app_main.h"
#include "app_end_device_node.h"
#include "dbg.h"
#include "app_buttons.h"
#include "app_leds.h"
#include "app_ota_client.h"
#include "zigbee_config.h"
#include "ZTimer.h"
#include "bdb_start.h"
#include "app_zcl_task.h"
#include "app_reporting.h"

/* ble includes */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "ApplMain.h"
#include "ble_init.h"
#include "controller_interface.h"
#include "wireless_uart_peripheral.h"

/* dual mode include */
#include "app_dual_mode_switch.h"
#include "app_dual_mode_low_power.h"

#include "MacSched.h"

/* Selective OTA */
#if defined (SOTA_ENABLED)
#include "blob_manager_app.h"
#include "blob_utils.h"
#endif

/************************************************************************************
*************************************************************************************
* Private Macro
*************************************************************************************
************************************************************************************/
#ifdef APP_SERIAL_LOGS_ENABLED
#define APP_SERIAL_PRINT(string) Serial_Print(gAppSerMgrIf, string, gAllowToBlock_d);
#define APP_SERIAL_PRINT_HEX(value, nbBytes) Serial_PrintHex(gAppSerMgrIf, value , nbBytes, gPrtHexNoFormat_c);
#else
#define APP_SERIAL_PRINT(...)
#define APP_SERIAL_PRINT_HEX(...)
#endif

#ifndef APP_DUAL_MODE_DEBUG
#define APP_DUAL_MODE_DEBUG FALSE
#endif

#define ZIGBEE_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE 4000 /* 4 ms */

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

typedef struct 
{
    bool_t isBleAppRunning;
    bool_t zigbeeInitialized;
    bool_t bleAppStopInprogress;
    bool_t zigbeeAppFactoryReset;
    uint32_t zigbeeWarmBootInitTime;
} sDualModeAppStates;


/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/

#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
static void BLE_SignalFromISRCallback(void);
#endif
#endif /* SOTA_ENABLED */

static void App_InitBle(void);
static void App_InitZigbee(bool_t bColdStart);
static void dm_switch_runZigbeeTasks(void);
static void dm_switch_preSleepCallBack(void);
static void dm_switch_wakeupCallBack(void);
static void dm_switch_15_4WakeUpReInitContinue(void);

#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0)
static void App_KeyboardCallBack(uint8_t events);
#endif

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static uint8_t platformInitialized = 0;
static sDualModeAppStates dualModeStates;

#ifdef APP_SERIAL_LOGS_ENABLED
static uint8_t gAppSerMgrIf;
#endif

/************************************************************************************
*************************************************************************************
* Exported functions/variables
*************************************************************************************
************************************************************************************/

/* BLE Application input queues */
extern anchor_t mHostAppInputQueue;
extern anchor_t mAppCbInputQueue;

/* PWR externs */
extern uint32_t PWR_Get32kTimestamp(void);

/* BLE application extern function */
extern osaStatus_t AppIdle_TaskInit(void);
extern void App_Thread (uint32_t param);
extern void App_GenericCallback (gapGenericEvent_t* pGenericEvent);
extern void BleApp_Init(void);
extern void BleApp_HandleKeys(key_event_t events);
extern void BleApp_Start(void);
extern void BleApp_Stop(void);
extern void BleAppDrv_Init(bool reinit);
#if !(defined EC_P256_DSPEXT && (EC_P256_DSPEXT == 1))
extern void App_SecLibMultCallback(computeDhKeyParam_t *pData);
#endif
/* Zigbee application extern function */
extern void APP_vBdbInit(bool_t bColdStart);

/* Shell extern functions */
extern void app_shell_init(void);

/* Zigbee extern function */
extern uint8* ZPS_pu8AplZdoGetVsOUI(void);

#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
extern void (*pfBLE_SignalFromISR)(void);
#endif /* gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

/* BLE app event */
osaEventId_t  mAppEvent;

extern struct fwk_cfg framework_configuration;
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/

/*! *********************************************************************************
* \brief  This is the first task created by the OS. This task will initialize
*         the system
*
* \param[in]  param
*
********************************************************************************** */
void main_task(uint32_t param)
{
    if (!platformInitialized)
    {
        platformInitialized = 1;

        BOARD_SetFaultBehaviour();

        /* Framework init */
        MEM_Init();
        TMR_Init();
        LED_Init();

        /* Cryptographic and RNG hardware initialization */
        SecLib_Init();
#if !(defined EC_P256_DSPEXT && (EC_P256_DSPEXT == 1))
        SecLib_SetExternalMultiplicationCb(App_SecLibMultCallback);
#endif        /* RNG software initialization and PRNG initial seeding (from hardware) */
        RNG_Init();
        RNG_SetPseudoRandomNoSeed(NULL);

#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0)
        KBD_Init(App_KeyboardCallBack);
#endif
        /* Init the flash driver */
        NV_Init();

        PWR_Init();
        PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
        PWR_vForceRadioRetention(TRUE);
        /* Disable the device to sleep until adversting is started*/
        PWR_DisallowDeviceToSleep();
        /* Disable the device to sleep until the zigbee app is OK to sleep*/
        PWR_DisallowDeviceToSleep();

        PWR_RegisterLowPowerExitCallback(dm_switch_wakeupCallBack);
        PWR_RegisterLowPowerEnterCallback(dm_switch_preSleepCallBack);
	
        /* PDM init */
        PDM_Init();
        /* Init ADC */
        BOARD_InitAdc();

#ifdef APP_SERIAL_LOGS_ENABLED
        /* UI */
        Serial_InitManager();

        /* Register Serial Manager interface */
        Serial_InitInterface(&gAppSerMgrIf, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE);

        Serial_SetBaudRate(gAppSerMgrIf, BOARD_DEBUG_UART_BAUDRATE);
#endif


        /* Init the Idle task */
        AppIdle_TaskInit();

        /* Create application event */
        mAppEvent = OSA_EventCreate(TRUE);
        if( NULL == mAppEvent )
        {
            panic(0,0,0,0);
            return;
        }

#if defined (SOTA_ENABLED)
        BLOBM_Init();
#endif
        App_NvmInit();

        /* Init the app shell */
        app_shell_init();
        /* Init the dual mode low power state machine */
        dm_lp_init();

        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init BLE stack in progress ... \n");
        App_InitBle();

        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init ZIGBEE stack in progress ... \n");
        App_InitZigbee(TRUE);

        /* Init the default value will be recalculated after each warm boot */
        dualModeStates.zigbeeWarmBootInitTime = ZIGBEE_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE;
        /* One more slot of 625 us is required compare to a ble only app */
        (*(uint8_t *) &framework_configuration.lp_cfg.wakeup_advance) = 3;

        /* Enable MAC scheduler. After this point, you cannot register a protocol anymore. */
        sched_enable();
    }
    /* Call application task */
    App_Thread( param );
}

/*! *********************************************************************************
* \brief  Runs the dual mode switch Idle Task
*
********************************************************************************** */
void dm_switch_IdleTask(void)
{
    if (PWR_CheckIfDeviceCanGoToSleep() && BleApp_CanGotoSleep())
    {
        PWR_EnterLowPower();
    }
    else
    {
        dm_switch_runZigbeeTasks();
    }
}

/*! *********************************************************************************
* \brief  Process any event and takes action depending of the event
*
* \param[in] pParam
*
********************************************************************************** */
void dm_switch_processEvent(void *pParam)
{
    eDualModeEvent event = (eDualModeEvent) pParam;
    switch (event)
    {
        case eBleAdvStopEvent:
        case eBleDisconnectionEvent:
        case eBleNotRunningEvent:
            /* Are we in "stopBle" state ? */
            if (!dualModeStates.bleAppStopInprogress)
            {
                /* If not restart adv */
                BleApp_Start();
            }
            else
            {
                /* It was disallowed in BleApp_Stop */
                PWR_AllowDeviceToSleep();
            }
            
            if (dualModeStates.bleAppStopInprogress && dualModeStates.zigbeeAppFactoryReset)
            {
                dualModeStates.zigbeeAppFactoryReset = FALSE;
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
                RESET_SystemReset();
            }
            dualModeStates.bleAppStopInprogress = FALSE;
            break;
        case e15_4FactoryResetEvent:
            dualModeStates.bleAppStopInprogress = TRUE;
            dualModeStates.zigbeeAppFactoryReset = TRUE;
            BleApp_Stop();
            break;
        case e15_4WakeUpReinitContinueEvent:
            dm_switch_15_4WakeUpReInitContinue();
            break;
        default:
            break;
    }
}

uint32_t dm_switch_get15_4InitWakeUpTime(void)
{
    return dualModeStates.zigbeeWarmBootInitTime;
}

void dm_switch_init15_4AfterWakeUp(void)
{
    uint32_t tick1 = 0;
    uint32_t tick2 = 0;
    if (dualModeStates.zigbeeWarmBootInitTime == ZIGBEE_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE)
    {
        /* Get a 32K tick */
        PWR_Start32kCounter();
        tick1 = PWR_Get32kTimestamp();
    }
    /* Re-init the zigbee stack which will contains calls to radio APIs.
       15.4 is set as active by default */
    App_InitZigbee(FALSE);

    /* set the correct state */
    vDynRequestState(E_DYN_SLAVE, E_DYN_STATE_INACTIVE);    /* 15.4 */
    vDynRequestState(E_DYN_MASTER, E_DYN_STATE_ACTIVE);     /* BLE */

    /* Enable MAC scheduler */
    sched_enable();

    /* Post an event to complete the re-init of the zigbee app */
    (void)App_PostCallbackMessage(dm_switch_processEvent, (void *) e15_4WakeUpReinitContinueEvent);

    if (dualModeStates.zigbeeWarmBootInitTime == ZIGBEE_WARM_BOOT_INIT_DURATION_DEFAULT_VALUE)
    {
        tick2 = PWR_Get32kTimestamp();
        dualModeStates.zigbeeWarmBootInitTime = ((tick2-tick1)*15625u)>>9;
        /* Add a margin of 1 ms */
        dualModeStates.zigbeeWarmBootInitTime += 1000;
        SWITCH_DBG_LOG("dualModeStates.zigbeeWarmBootInitTime = %d", dualModeStates.zigbeeWarmBootInitTime);
    }
}


/*! *********************************************************************************
* \brief  Zigbee wtimer wake up callback
*
********************************************************************************** */
void vWakeCallBack(void)
{
    /* Disallow the device to sleep until the zigbee app is OK to sleep*/
    PWR_DisallowDeviceToSleep();
    /* Did we sleep ? */
    if (dualModeStates.zigbeeInitialized)
    {
        SWITCH_DBG_LOG("zb initialized");
        APP_ZCL_vStartTimers();
        ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST);
    }
    else
    {
        SWITCH_DBG_LOG("zb not initialized");
        /* Notify the dual mode low power mode */
        dm_lp_processEvent((void *) e15_4WakeUpEnded);
    }
}

/************************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/

#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
/* Called by BLE when a connect is received */
static void BLE_SignalFromISRCallback(void)
{
#if (cPWR_UsePowerDownMode) && (!cPWR_NoPowerDownDisabledOnConnect)
    PWR_DisallowDeviceToSleep();
#endif /* cPWR_UsePowerDownMode */
}
#endif /* !gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

/*****************************************************************************
* Init the BLE stack and the BLE application
* Return value: None
*****************************************************************************/
static void App_InitBle()
{
    bool_t bleStackInitStatus = FALSE;
#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
    pfBLE_SignalFromISR = BLE_SignalFromISRCallback;
#endif /* !gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

    BleAppDrv_Init(FALSE);
    /* Prepare application input queue.*/
    MSG_InitQueue(&mHostAppInputQueue);
    MSG_InitQueue(&mAppCbInputQueue);

    do
    {
        if (Ble_Initialize(App_GenericCallback) != gBleSuccess_c)
            break;
        bleStackInitStatus = TRUE;
    } while(0);

    if (!bleStackInitStatus)
    {
        panic(0,0,0,0);
    }
    /* Advertising will be started automatically */
    dualModeStates.isBleAppRunning = TRUE;
}

static void App_InitZigbee(bool_t bColdStart)
{
    SWITCH_DBG_LOG("Start ==>");
    /* Initialise LEDs and buttons */
    APP_vLedInitialise();
    APP_vInitResources();
    APP_vInitZigbeeResources();
    /* Initialise application */
    APP_vInitialiseEndDevice(bColdStart);
    if(bColdStart)
    {
        BDB_vStart();
        dualModeStates.zigbeeInitialized = TRUE;
    }
    SWITCH_DBG_LOG("End <==");
}

/*****************************************************************************
* Handles all key events for this device.
* Interface assumptions: None
* Return value: None
*****************************************************************************/
#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0)
static void App_KeyboardCallBack
  (
  uint8_t events  /*IN: Events from keyboard module  */
  )
{
    switch (events)
    {
        case gKBD_EventPressPB1_c:
        {
            APP_tsEvent sButtonEvent;
            sButtonEvent.eType = APP_E_EVENT_BUTTON_DOWN;
            sButtonEvent.uEvent.sButton.u8Button = APP_E_BUTTONS_BUTTON_1;

            if(ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent) == FALSE)
            {
                DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Button: Failed to post Event %d \r\n", sButtonEvent.eType);
            }
            break;
        }
        case gKBD_EventPressPB2_c:
        {
            if (dualModeStates.isBleAppRunning)
            {
                SWITCH_DBG_LOG("BLE app will be stopped");
                dualModeStates.bleAppStopInprogress = TRUE;
                BleApp_Stop();
                dualModeStates.isBleAppRunning = FALSE;
            }
            else if (!dualModeStates.bleAppStopInprogress)
            {
                SWITCH_DBG_LOG("BLE app will be restarted");
                BleApp_Start();
                dualModeStates.isBleAppRunning = TRUE;
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif

static void dm_switch_runZigbeeTasks(void)
{
    if (dualModeStates.zigbeeInitialized)
    {
        APP_vRunZigbee();
        APP_taskEndDevicNode();
        ZTIMER_vTask();
    }
}

static void dm_switch_preSleepCallBack(void)
{
    SWITCH_DBG_LOG("sleeping");
    DBG_vPrintf(APP_DUAL_MODE_DEBUG, "sleeping\n");
    /* Stop BLE APP running timers */
    BleApp_StopRunningTimers();

    /* Inform the low power dual mode module that we will sleep.
       It disables the MAC scheduler.
       Disabling the scheduler must be done before calling vMMAC_Disable()
       so it works correctly */
    dm_lp_preSleep();

    if (dualModeStates.zigbeeInitialized)
    {
        vSetReportDataForMinRetention();
        /* If the power mode is with RAM held do the following
         * else not required as the entry point will init everything */
        vSetOTAPersistedDatForMinRetention();

        /* sleep memory held */
        vAppApiSaveMacSettings();
        dualModeStates.zigbeeInitialized = FALSE;
    }

    /* Deinitialize the OTA support */
    OTA_DeInitExternalMemory();
    /* Deinitialize debug console */
    BOARD_DeinitDebugConsole();
    /* DeInitialize application support for drivers */
    BOARD_DeInitAdc();
    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
    /* DeInit the necessary clocks */
    BOARD_SetClockForPowerMode();
}

static void dm_switch_wakeupCallBack(void)
{
    RNG_Init();
    SecLib_Init();
    OTA_InitExternalMemory();
    dm_lp_wakeup();
    BleAppDrv_Init(true);
    SWITCH_DBG_LOG("woken up \n");
}

static void dm_switch_15_4WakeUpReInitContinue(void)
{
    /* Initialise ZCL */
    APP_ZCL_vInitialise(FALSE);
    /* Initialise other software modules */
    APP_vBdbInit(FALSE);
    APP_ZCL_vStartTimers();
    ZTIMER_eStart(u8TimerPoll, POLL_TIME_FAST);
    BDB_vRestart();
    APP_vAlignStatesAfterSleep();
    dualModeStates.zigbeeInitialized = TRUE;
}
