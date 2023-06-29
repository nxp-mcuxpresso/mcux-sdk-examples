/*! *********************************************************************************
* Copyright (c) 2014, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
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
#ifdef FSL_RTOS_FREE_RTOS
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#endif
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
#include "NVM_Interface.h"

/* KSDK */
#include "board.h"
#include "fsl_reset.h"
#include "MicroSpecific.h"

#ifdef ENABLE_SUBG_IF
#include "fsl_iocon.h"
#endif

/* zigbee includes */
#include "app_main.h"
#include "app_coordinator.h"
#include "dbg.h"
#include "app_buttons.h"
#include "app_leds.h"
#include "zigbee_config.h"
#include "ZTimer.h"
#include "bdb_start.h"

/* ble includes */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "ApplMain.h"
#include "ble_init.h"
#include "controller_interface.h"

/* dual mode include */
#include "app_dual_mode_switch.h"

#ifndef ENABLE_SUBG_IF
#include "MacSched.h"
#endif

/* Selective OTA */
#if defined (SOTA_ENABLED)
#include "blob_manager_app.h"
#include "blob_utils.h"
#endif

#ifdef ENABLE_SUBG_IF
#include "MDI_ReadFirmVer.h"
#include "MDI_Programmer.h"
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

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/

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

static void App_InitBle();
static void App_InitZigbee();
static void dm_switch_RunZigbeeTasks();

#if gKeyBoardSupported_d && (gKBD_KeysCount_c > 0)
static void App_KeyboardCallBack(uint8_t events);
#endif

/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/

static uint8_t platformInitialized = 0;
static bool_t bleStackStopInprogress = FALSE;
static bool_t zigbeeStackFactoryReset = FALSE;

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
/* Zigbee queue */
extern tszQueue APP_msgAppEvents;

/* BLE application extern function */
extern osaStatus_t AppIdle_TaskInit(void);
extern void App_Thread (uint32_t param);
extern void App_GenericCallback (gapGenericEvent_t* pGenericEvent);
extern void BleApp_Init(void);
extern void BleApp_HandleKeys(key_event_t events);
extern void BleApp_Start(void);
extern void BleApp_Stop(void);
#if !(defined EC_P256_DSPEXT && (EC_P256_DSPEXT == 1))
extern void App_SecLibMultCallback(computeDhKeyParam_t *pData);
#endif
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

#ifdef ENABLE_SUBG_IF
        /* subGHz scenario:
         * - enable ZB and BLE (DUAL_MODE_APP)
         * - don't enable protocol switching (gEnableBleInactivityTimeNotify=0)
         * - 2.4GHz MAC is used only for timers but configures radio in ZB mode
         * - enable BLE last so that the radio is put and remains in BLE mode
         */
        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init ZIGBEE stack in progress ... \r\n");
        App_InitZigbee();
#endif

        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init BLE stack in progress ... \r\n");
        App_InitBle();

#ifndef ENABLE_SUBG_IF
        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init ZIGBEE stack in progress ... \r\n");
        App_InitZigbee();
        /* Enable MAC scheduler. After this point, you cannot register a protocol anymore. */
        sched_enable();
#endif
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
    dm_switch_RunZigbeeTasks();
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
            if (!bleStackStopInprogress)
            {
                /* If not restart adv */
                BleApp_Start();
            }
            else if (bleStackStopInprogress && zigbeeStackFactoryReset)
            {
                zigbeeStackFactoryReset = FALSE;
                APP_vFactoryResetRecords();
                MICRO_DISABLE_INTERRUPTS();
                RESET_SystemReset();
            }
            bleStackStopInprogress = FALSE;
            break;
        case e15_4FactoryResetEvent:
            bleStackStopInprogress = TRUE;
            zigbeeStackFactoryReset = TRUE;
            BleApp_Stop();
            break;
        default:
            break;
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

#if !defined (SOTA_ENABLED)
#if !gUseHciTransportDownward_d
    pfBLE_SignalFromISR = BLE_SignalFromISRCallback;
#endif /* !gUseHciTransportDownward_d */
#endif /* SOTA_ENABLED */

    BleApp_Init();
    /* Prepare application input queue.*/
    MSG_InitQueue(&mHostAppInputQueue);
    MSG_InitQueue(&mAppCbInputQueue);

    /* BLE Host Stack Init */
    if (Ble_Initialize(App_GenericCallback) != gBleSuccess_c)
    {
        panic(0,0,0,0);
        return;
    }

}

#ifdef ENABLE_SUBG_IF
/****************************************************************************
 *
 * NAME: APP_vCheckMtgState
 *
 * DESCRIPTION:
 * Initialises MTG comms
 *
 * RETURNS:
 * void
 *
 ****************************************************************************/
static void APP_vCheckMtgState(void)
{
    vMDI_RecoverDioSettings();
    vMDI_PrintIOConfig();

    /* Reset the Radio, don't care */
    vMDI_Reset868MtGRadio();

    /* Send SYNC request, wait for SYNC resp */
    vMDI_SyncWithMtG();
}

static void App_InitPins(void)
{
    /* USART0 RX/TX pin */
    IOCON_PinMuxSet(IOCON, 0, 8, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 9, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);

    /* Debugger signals (SWCLK, SWDIO) - need to turn it OFF to reduce power consumption in power modes*/
    IOCON_PinMuxSet(IOCON, 0, 12, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 13, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);

#ifndef ENABLE_SUBG_IF
    /* I2C0  */
    IOCON_PinMuxSet(IOCON, 0, 10, IOCON_FUNC5 | IOCON_DIGITAL_EN | IOCON_STDI2C_EN);  /* I2C0_SCL */
    IOCON_PinMuxSet(IOCON, 0, 11, IOCON_FUNC5 | IOCON_DIGITAL_EN | IOCON_STDI2C_EN);  /* I2C0_SDA */
#else
    /* USART1 RX/TX pin */
    IOCON_PinMuxSet(IOCON, 0, 10, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_GPIO_MODE | IOCON_INPFILT_OFF | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 11, IOCON_FUNC2 | IOCON_MODE_INACT | IOCON_GPIO_MODE | IOCON_INPFILT_OFF | IOCON_DIGITAL_EN);

    /* MSCL, MSDA, RST */
    IOCON_PinMuxSet(IOCON, 0, 18, IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 4, IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGITAL_EN);
    IOCON_PinMuxSet(IOCON, 0, 6, IOCON_FUNC0 |  IOCON_MODE_INACT | IOCON_DIGITAL_EN);
#endif

}
#endif

/*****************************************************************************
* Init the zigbee stack and the zigbee application
* Return value: None
*****************************************************************************/
static void App_InitZigbee()
{
#ifdef ENABLE_SUBG_IF
    App_InitPins();
    APP_vCheckMtgState();
#endif

    /* Initialise LEDs and buttons */
    APP_vLedInitialise();
    APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialiseCoordinator();
    BDB_vStart();
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
        case gKBD_EventPB1_c:
        {
            /* USER INTERFACE BUTTON, is used to send toggle cmd */
            APP_tsEvent sButtonEvent;
            sButtonEvent.eType = APP_E_EVENT_NONE;
            if (sBDB.sAttrib.bbdbNodeIsOnANetwork)
            {
                sButtonEvent.eType = APP_E_EVENT_SERIAL_TOGGLE;
                if(!ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent))
                {
                    DBG_vPrintf(APP_DUAL_MODE_DEBUG, "TOGGLE_FAILURE\n");
                }
            }
            else
            {
                DBG_vPrintf(APP_DUAL_MODE_DEBUG, "NETWORK_NOT_CREATED\n");
            }
            break;
        }
        case gKBD_EventPB2_c:
        {
            /* ISP BUTTON, is used to bind */
            APP_tsEvent sButtonEvent;
            sButtonEvent.eType = APP_E_EVENT_NONE;
            if (sBDB.sAttrib.bbdbNodeIsOnANetwork)
            {
                sButtonEvent.eType = APP_E_EVENT_SERIAL_FIND_BIND_START;
                if(!ZQ_bQueueSend(&APP_msgAppEvents, &sButtonEvent))
                {
                    DBG_vPrintf(APP_DUAL_MODE_DEBUG, "FIND_&_BIND_FAILURE\n");
                }
            }
            else
            {
                DBG_vPrintf(APP_DUAL_MODE_DEBUG, "NETWORK_NOT_CREATED\n");
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

static void dm_switch_RunZigbeeTasks()
{
    APP_vRunZigbee();
    ZTIMER_vTask();
    APP_taskCoordinator();
}
