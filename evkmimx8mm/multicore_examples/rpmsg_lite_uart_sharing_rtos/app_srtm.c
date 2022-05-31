/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fsl_mu.h"
#include "fsl_iomuxc.h"
#include "srtm_dispatcher.h"
#include "srtm_peercore.h"
#include "srtm_message.h"
#include "srtm_uart_adapter.h"
#include "srtm_uart_service.h"
#include "app_srtm.h"
#include "board.h"
#include "srtm_rpmsg_endpoint.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
app_rpmsg_monitor_t rpmsgMonitor;
volatile app_srtm_state_t srtmState = APP_SRTM_StateRun;

static srtm_dispatcher_t disp;
static srtm_peercore_t core;
static srtm_service_t uartService = NULL;
static srtm_uart_adapter_t uartAdapter = NULL;

SemaphoreHandle_t monSig;
struct rpmsg_lite_instance *rpmsgHandle;

void *rpmsgMonitorParam;
TimerHandle_t linkupTimer;

static SERIAL_MANAGER_HANDLE_DEFINE(serialHandle3);
static SERIAL_MANAGER_WRITE_HANDLE_DEFINE(serialWriteHandle3);
static SERIAL_MANAGER_READ_HANDLE_DEFINE(serialReadHandle3);
/* don't have UART0, so +1 to skip UART0(seq is start from 1, such as: UART1, UART2...) */
static serial_handle_t serialHandles[FSL_FEATURE_SOC_IUART_COUNT + 1] = { NULL, NULL, NULL, (serial_handle_t)serialHandle3, NULL}; /* uart3 is used */
static serial_write_handle_t serialWriteHandles[FSL_FEATURE_SOC_IUART_COUNT + 1] = { NULL, NULL, NULL, (serial_write_handle_t)serialWriteHandle3, NULL}; /* uart3 is used */
static serial_read_handle_t serialReadHandles[FSL_FEATURE_SOC_IUART_COUNT + 1] = { NULL, NULL, NULL, (serial_read_handle_t)serialReadHandle3, NULL}; /* uart3 is used */

/*******************************************************************************
 * Code
 ******************************************************************************/
static void APP_SRTM_PollLinkup(srtm_dispatcher_t dispatcher, void *param1, void *param2)
{
    if (srtmState == APP_SRTM_StateRun)
    {
        if (rpmsg_lite_is_link_up(rpmsgHandle))
        {
            srtmState = APP_SRTM_StateLinkedUp;
            xSemaphoreGive(monSig);
        }
        else
        {
            /* Start timer to poll linkup status. */
            xTimerStart(linkupTimer, portMAX_DELAY);
        }
    }
}

static void APP_LinkupTimerCallback(TimerHandle_t xTimer)
{
    srtm_procedure_t proc = SRTM_Procedure_Create(APP_SRTM_PollLinkup, NULL, NULL);

    if (proc)
    {
        SRTM_Dispatcher_PostProc(disp, proc);
    }
}

static void APP_SRTM_NotifyPeerCoreReady(struct rpmsg_lite_instance *rpmsgHandle, bool ready)
{
    if (rpmsgMonitor)
    {
        rpmsgMonitor(rpmsgHandle, ready, rpmsgMonitorParam);
    }
}

static void APP_SRTM_Linkup(void)
{
    uint8_t uart_id = 0;
    srtm_channel_t chan;
    srtm_rpmsg_endpoint_config_t rpmsgConfig;

    APP_SRTM_NotifyPeerCoreReady(rpmsgHandle, true);

    /* Create SRTM peer core */
    core = SRTM_PeerCore_Create(1U); /* Assign CA53 core ID to 1U */

    SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Activated);

    /* Common RPMsg channel config */
    rpmsgConfig.localAddr = RL_ADDR_ANY;
    rpmsgConfig.peerAddr  = RL_ADDR_ANY;

    rpmsgConfig.rpmsgHandle = rpmsgHandle;

    /* Create and add SRTM uart channel to peer core */
    rpmsgConfig.epName      = APP_SRTM_UART_CHANNEL_NAME;
    for (uart_id = 0; uart_id < APP_SRTM_UART_ENDPOINT_MAX_NUM; uart_id++)
    {
        chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
        uartAdapter->bindChanByUartId(chan, SRTM_UART_INVALID_BUS_ID, 0U, uart_id);
        SRTM_PeerCore_AddChannel(core, chan);
    }

    SRTM_Dispatcher_AddPeerCore(disp, core);
}

static void APP_SRTM_InitPeerCore(void)
{
    copyResourceTable();

    rpmsgHandle = rpmsg_lite_remote_init((void *)RPMSG_LITE_SRTM_SHMEM_BASE, RPMSG_LITE_SRTM_LINK_ID, RL_NO_FLAGS);
    assert(rpmsgHandle);
    if (rpmsg_lite_is_link_up(rpmsgHandle))
    {
        APP_SRTM_Linkup();
    }
    else
    {
        /* Start timer to poll linkup status. */
        xTimerStart(linkupTimer, portMAX_DELAY);
    }
}

static uint8_t s_ringBuffer[APP_SRTM_UART_SERIAL_MANAGER_RING_BUFFER_SIZE];

static srtm_status_t APP_SRTM_InitUartDevice(void)
{
    srtm_status_t error = SRTM_Status_Error;
    serial_port_uart_config_t uartConfig[ARRAY_SIZE(serialHandles)] = {0};
    serial_manager_config_t serialManagerConfig[ARRAY_SIZE(serialHandles)] = {0};
    int i = 0;

    uartConfig[APP_SRTM_UART3_INSTANCE].clockRate = APP_SRTM_UART3_CLK_FREQ;
    uartConfig[APP_SRTM_UART3_INSTANCE].baudRate = APP_SRTM_UART3_BAUDRATE;
    uartConfig[APP_SRTM_UART3_INSTANCE].parityMode = kSerialManager_UartParityDisabled;
    uartConfig[APP_SRTM_UART3_INSTANCE].stopBitCount = kSerialManager_UartOneStopBit;
    uartConfig[APP_SRTM_UART3_INSTANCE].enableRx = 1;
    uartConfig[APP_SRTM_UART3_INSTANCE].enableTx = 1;
    uartConfig[APP_SRTM_UART3_INSTANCE].enableRxRTS = 0;
    uartConfig[APP_SRTM_UART3_INSTANCE].enableTxCTS = 0;
    uartConfig[APP_SRTM_UART3_INSTANCE].instance = APP_SRTM_UART3_INSTANCE;

    serialManagerConfig[APP_SRTM_UART3_INSTANCE].type = APP_SRTM_UART_TYPE;
    serialManagerConfig[APP_SRTM_UART3_INSTANCE].ringBuffer     = &s_ringBuffer[0];
    serialManagerConfig[APP_SRTM_UART3_INSTANCE].ringBufferSize = sizeof(s_ringBuffer);
    serialManagerConfig[APP_SRTM_UART3_INSTANCE].blockType = APP_SRTM_UART_SERIAL_MANAGER_BLOCK_TYPE;
    serialManagerConfig[APP_SRTM_UART3_INSTANCE].portConfig = (serial_port_uart_config_t *)&uartConfig[APP_SRTM_UART3_INSTANCE];
    for (i = APP_SRTM_FIRST_UART_INSTANCE; i <= ARRAY_SIZE(serialHandles); i++)
    {
        if (serialHandles[i] != NULL)
        {
            do {
                if (SerialManager_Init((serial_handle_t)serialHandles[i], &serialManagerConfig[i]) != kStatus_SerialManager_Success)
                    break;
                if (SerialManager_OpenWriteHandle((serial_handle_t)serialHandles[i], (serial_write_handle_t)serialWriteHandles[i]) != kStatus_SerialManager_Success)
                    break;
                if (SerialManager_OpenReadHandle((serial_handle_t)serialHandles[i], (serial_read_handle_t)serialReadHandles[i]) != kStatus_SerialManager_Success)
                    break;
                if (SerialManager_InstallRxCallback((serial_read_handle_t)serialReadHandles[i], SRTM_Uart_RxCallBack, serialReadHandles[i]) != kStatus_SerialManager_Success)
                    break;
                if (SerialManager_InstallTxCallback((serial_write_handle_t)serialWriteHandles[i], SRTM_Uart_TxCallBack, serialWriteHandles[i]) != kStatus_SerialManager_Success)
                    break;
                error = SRTM_Status_Success;
            } while(0);
        }
    }
    return error;
}

static void APP_SRTM_InitUartService(void)
{
    if (SRTM_Status_Success == APP_SRTM_InitUartDevice())
    {
        uartAdapter = SRTM_UartAdapter_Create(serialHandles, serialWriteHandles, serialReadHandles, ARRAY_SIZE(serialHandles));
        assert(uartAdapter);

        /* Create and register serial service */
        uartService = SRTM_UartService_Create(uartAdapter);
        SRTM_Dispatcher_RegisterService(disp, uartService);
    }
    else
    {
	PRINTF("%s: %d Failed to Do Init SRTM Serial Service\r\n", __func__, __LINE__);
    }
}

static void APP_SRTM_InitServices(void)
{
    APP_SRTM_InitUartService();
}

static void SRTM_MonitorTask(void *pvParameters)
{
    /* Initialize services and add to dispatcher */
    APP_SRTM_InitServices();

    /* Start SRTM dispatcher */
    SRTM_Dispatcher_Start(disp);

    xSemaphoreGive(monSig);
    while (true)
    {
        xSemaphoreTake(monSig, portMAX_DELAY);
        if (srtmState == APP_SRTM_StateRun)
        {
            SRTM_Dispatcher_Stop(disp);
            APP_SRTM_InitPeerCore();
            SRTM_Dispatcher_Start(disp);
        }
        else
        {
            SRTM_Dispatcher_Stop(disp);
            /* Need to announce channel as we just linked up. */
            APP_SRTM_Linkup();
            SRTM_Dispatcher_Start(disp);
        }
    }
}

static void SRTM_DispatcherTask(void *pvParameters)
{
    SRTM_Dispatcher_Run(disp);
}

void APP_SRTM_Init(void)
{
    MU_Init(MUB);

    monSig = xSemaphoreCreateBinary();
    assert(monSig);
    linkupTimer =
        xTimerCreate("Linkup", APP_MS2TICK(APP_LINKUP_TIMER_PERIOD_MS), pdFALSE, NULL, APP_LinkupTimerCallback);
    assert(linkupTimer);
    /* Create SRTM dispatcher */
    disp = SRTM_Dispatcher_Create();

    if (xTaskCreate(SRTM_MonitorTask, "SRTM monitor", 256U, NULL, APP_SRTM_MONITOR_TASK_PRIO, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    if (xTaskCreate(SRTM_DispatcherTask, "SRTM dispatcher", 512U, NULL, APP_SRTM_DISPATCHER_TASK_PRIO, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
}

void APP_SRTM_Suspend(void)
{
    /* For user use. */
}

void APP_SRTM_Resume(void)
{
    /* For user use. */
}
