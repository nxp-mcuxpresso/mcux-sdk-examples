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
#include "radio.h"

/* KSDK */
#include "board.h"

/* MAC include */
#include "MiniMac.h"

/* zigbee includes */
#include "app_main.h"
#include "app_router_node.h"
#include "dbg.h"
#include "app_buttons.h"
#include "app_ota_client.h"
#include "zigbee_config.h"
#include "ZTimer.h"

/* ble includes */
#include "gatt_interface.h"
#include "gatt_server_interface.h"
#include "gatt_client_interface.h"
#include "gap_interface.h"
#include "ApplMain.h"
#include "ble_init.h"
#include "controller_interface.h"
#include "otap_client.h"

/* dual mode include */
#include "app_dual_mode_switch.h"

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

/************************************************************************************
*************************************************************************************
* Private definitions
*************************************************************************************
************************************************************************************/
typedef enum {
    eZigbeeMode = 0,
    eBLEMode,
} eDualModeZbBle;

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
static void App_SwitchConnectivityMode();
static void App_DisplayRunningVersion();
static void ZigbeeApp_handleKeys(uint8_t events);
static void dm_switch_RunZigbeeTasks();
static void SetAppMode(eDualModeZbBle new_mode);
static void dm_switch_start_ble(void);
static void dm_switch_to_zigbee(void);
static void dm_switch_to_ble(void);
static void dm_switch_reset_zigbee_ota_state(void);
static void dm_switch_reset_ble_ota_state(void);

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
static bool_t zigbeeStackStopInprogress = FALSE;
static uint8_t app_running_con_mode = eBLEMode;
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
extern bool_t BleApp_Stop(void);
#if !(defined EC_P256_DSPEXT && (EC_P256_DSPEXT == 1))
extern void App_SecLibMultCallback(computeDhKeyParam_t *pData);
#endif

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
        App_DisplayRunningVersion();

        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init BLE stack in progress ... \n");
        App_InitBle();

        DBG_vPrintf(APP_DUAL_MODE_DEBUG, "Init ZIGBEE stack in progress ... \n");
        App_InitZigbee();

        if (app_running_con_mode == eBLEMode)
        {
            zigbeeStackStopInprogress = TRUE;
            dm_switch_to_ble();
        }
        else
        {
            bleStackStopInprogress = TRUE;
            dm_switch_to_zigbee();
        }
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
    if (app_running_con_mode == eZigbeeMode)
    {
        dm_switch_RunZigbeeTasks();
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
            dm_switch_start_ble();
            dm_switch_to_zigbee();
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

#ifndef SOTA_ENABLED
extern uint8_t au8OtaHeader[OTA_MAX_HEADER_SIZE];
extern tsOTA_ImageHeader sOtaGetHeader(uint8 *pu8HeaderBytes);
#endif

static void App_DisplayRunningVersion()
{
#if defined (SOTA_ENABLED)
    uint32_t blobVersion;
    uint16_t blobIds[SOTA_MAX_BLOB_NB_WITH_SSBL];
    uint8_t nbBlob = 0;
    uint8_t i;

    /* All blob composing the global application */
    nbBlob = Sota_GetAllBlobIds(blobIds, SOTA_MAX_BLOB_NB_WITH_SSBL);

    for (i=0; i<nbBlob; i++)
    {
        Sota_GetBlobVersion(blobIds[i], &blobVersion);
        APP_SERIAL_PRINT("\n Blob ID = 0x");
        APP_SERIAL_PRINT_HEX((uint8_t *) &blobIds[i] ,2);
        APP_SERIAL_PRINT(", version = 0x");
        APP_SERIAL_PRINT_HEX((uint8_t *) &blobVersion ,4);
        APP_SERIAL_PRINT("\n");
    }
#else
    tsOTA_ImageHeader sOTAHeader = sOtaGetHeader(au8OtaHeader);
    NOT_USED(sOTAHeader);
    APP_SERIAL_PRINT("\n Running File version is = 0x");
    APP_SERIAL_PRINT_HEX((uint8_t *) &sOTAHeader.u32FileVersion ,4);
    APP_SERIAL_PRINT("\n");
#endif
}

static void SetAppMode(eDualModeZbBle new_mode)
{
    const uint8_t zb_irq_array[] = {ZIGBEE_MAC_IRQn, ZIGBEE_MODEM_IRQn};
    const uint8_t ble_irq_array[] = {BLE_LL_ALL_IRQn, BLE_WAKE_UP_TIMER_IRQn};
    int enable_list_sz;
    int disable_list_sz;
    const uint8_t * irq_en_list;
    const uint8_t * irq_dis_list;

    if (new_mode == eZigbeeMode)
    {
        irq_en_list = &zb_irq_array[0];
        enable_list_sz = sizeof(zb_irq_array)/sizeof(uint8_t);
        irq_dis_list = &ble_irq_array[0];
        disable_list_sz = sizeof(ble_irq_array)/sizeof(uint8_t);
        vMiniMac_ConfigureDoze(0, 0);
        vRadio_BLEtoZB();
    }
    else
    {
        irq_en_list = &ble_irq_array[0];
        enable_list_sz = sizeof(ble_irq_array)/sizeof(uint8_t);
        irq_dis_list = &zb_irq_array[0];
        disable_list_sz = sizeof(zb_irq_array)/sizeof(uint8_t);
        vRadio_ZBtoBLE();
    }
    for (int i = 0; i < disable_list_sz; i++)
    {
        NVIC_DisableIRQ(irq_dis_list[i]);
    }
    for (int i = 0; i < enable_list_sz; i++)
    {
        NVIC_EnableIRQ(irq_en_list[i]);
    }

    /* Set the running mode */
    app_running_con_mode = new_mode;
}

/*****************************************************************************
* Init the zigbee stack and the zigbee application
* Return value: None
*****************************************************************************/
static void App_InitZigbee()
{
    APP_vInitResources();
    APP_vInitZigbeeResources();
    APP_vInitialiseRouter();
    BDB_vStart();
}

/*****************************************************************************
* Allows to switch from Zigbee to BLE or from BLE to Zigbee
* In the case of BLE the switch will be done asynchronous 
* Return value: None
*****************************************************************************/
static void App_SwitchConnectivityMode()
{
    if (app_running_con_mode == eBLEMode)
    {
        bleStackStopInprogress = TRUE;
        APP_SERIAL_PRINT("\n Stopping BLE mode ... \n");
        BleApp_Stop();
    }
    else
    {
        zigbeeStackStopInprogress = TRUE;
        dm_switch_to_ble();
    }
}

/*****************************************************************************
* Handles only the zigbee events
* Return value: None
*****************************************************************************/
static void ZigbeeApp_handleKeys(uint8_t events)
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
        default:
            /* Others are not supported yet */
            break;
    }
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
        case gKBD_EventPressPB2_c:
        {
            App_SwitchConnectivityMode();
            break;
        }
        default:
        {
            if (app_running_con_mode == eBLEMode)
            {
                BleApp_Start();
            }
            else
            {
                ZigbeeApp_handleKeys(events);
            }
            break;
        }
    }
}
#endif

static void dm_switch_RunZigbeeTasks()
{
    if (app_running_con_mode == eZigbeeMode)
    {
        APP_vRunZigbee();
        ZTIMER_vTask();
        APP_taskRouter();
    }
}

static void dm_switch_to_zigbee()
{
    if (bleStackStopInprogress)
    {
        /* Reset the BLE OTA state machine before switch to zigbee */
        dm_switch_reset_ble_ota_state();
        APP_SERIAL_PRINT("\n Switching to Zigbee mode ... \n");
        bleStackStopInprogress = FALSE;
        SetAppMode(eZigbeeMode);
        APP_SERIAL_PRINT("\n To reset the Zigbee device keep pressing BP1 and press the reset button ... \n");
        BDB_vRestart();
    }
}

static void dm_switch_to_ble()
{
    if (zigbeeStackStopInprogress)
    {
        APP_SERIAL_PRINT("\n Switching to BLE mode ... \n");
        /* Set the MAC in doze mode */
        vMiniMac_ConfigureDoze(0xffffffff, 0xfffffffe);
        while (eMiniMac_FrameInProgress() != E_FIP_NONE)
        {
            /* Empty loop */
        }
        /* Reset the zigbee OTA state machine */
        dm_switch_reset_zigbee_ota_state();
        zigbeeStackStopInprogress = FALSE;
        SetAppMode(eBLEMode);
        APP_SERIAL_PRINT("\n You can Press BP1 to start advertising ... \n");
    }
}

static void dm_switch_start_ble(void)
{
    if (!bleStackStopInprogress)
    {
        BleApp_Start();
    }
}

static void dm_switch_reset_zigbee_ota_state(void)
{
    if (zigbeeStackStopInprogress)
    {
        vOTAResetPersist();
    }
}

static void dm_switch_reset_ble_ota_state(void)
{
    if (bleStackStopInprogress && OtapClient_GetDownloadState())
    {
        OtapClient_ResetOtaClient();
    }
}