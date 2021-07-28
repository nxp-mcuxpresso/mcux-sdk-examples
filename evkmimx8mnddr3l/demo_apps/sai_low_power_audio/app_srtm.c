/*
 * Copyright 2018-2020, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fsl_gpio.h"
#include "fsl_mu.h"
#include "fsl_sdma.h"
#include "fsl_iomuxc.h"
#include "srtm_dispatcher.h"
#include "srtm_peercore.h"
#include "srtm_message.h"
#include "srtm_audio_service.h"
#include "app_srtm.h"
#include "board.h"
#include "lpm.h"
#include "srtm_sai_sdma_adapter.h"
#include "srtm_rpmsg_endpoint.h"
#include "fsl_codec_adapter.h"
#include "srtm_wm8524_adapter.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MS2TICK(ms) ((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)

#define BUFFER_LEN (128 * 1024)
#if (defined(__ICCARM__))
uint8_t g_buffer[BUFFER_LEN] @"AudioBuf";
#else
uint8_t g_buffer[BUFFER_LEN] __attribute__((section("AudioBuf,\"w\",%nobits @")));
#endif
srtm_sai_sdma_local_buf_t g_local_buf = {
    .buf       = (uint8_t *)&g_buffer,
    .bufSize   = BUFFER_LEN,
    .periods   = SRTM_SAI_SDMA_MAX_LOCAL_BUF_PERIODS,
    .threshold = 1,

};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
app_rpmsg_monitor_t rpmsgMonitor;
volatile app_srtm_state_t srtmState = APP_SRTM_StateRun;

codec_handle_t codecHandle;
codec_config_t boardCodecConfig;

static srtm_dispatcher_t disp;
static srtm_peercore_t core;
static srtm_service_t audioService;

SemaphoreHandle_t monSig;
struct rpmsg_lite_instance *rpmsgHandle;

void *rpmsgMonitorParam;
TimerHandle_t linkupTimer;
srtm_sai_adapter_t saiAdapter;
srtm_codec_adapter_t codecAdapter;
srtm_wm8524_config_t wm8524Config;
/*******************************************************************************
 * Code
 ******************************************************************************/
bool APP_SRTM_ServiceIdle(void)
{
    srtm_audio_state_t TxState, RxState;

    SRTM_SaiSdmaAdapter_GetAudioServiceState(saiAdapter, &TxState, &RxState);
    if (TxState == SRTM_AudioStateClosed && RxState == SRTM_AudioStateClosed)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static void APP_SRTM_WM8524_Mute_GPIO(uint32_t output)
{
    GPIO_PinWrite(APP_CODEC_MUTE_PIN, APP_CODEC_MUTE_PIN_NUM, output);
}
static uint32_t APP_SRTM_SaiClockSet(mclk_type_t type)
{
    uint32_t sai_source_clk = 0;
    if (type == SRTM_CLK22M)
    {
        CLOCK_SetRootMux(kCLOCK_RootSai3,
                         kCLOCK_SaiRootmuxAudioPll2); /* Set SAI source to Audio PLL2 361267197HZ to get 22.5792MHz */
        sai_source_clk = APP_AUDIO_PLL2_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai3));
    }
    else
    {
        CLOCK_SetRootMux(kCLOCK_RootSai3,
                         kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to Audio PLL1 393215996HZ to get 24.576Mhz*/
        sai_source_clk = APP_AUDIO_PLL1_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai3));
    }
    return sai_source_clk;
}

static void APP_SRTM_ClockGateControl(bool isEnable)
{
    if (isEnable)
    {
        /* Judge if the Audio PLL1/PLL2  are ON, if not, enable them. */
        if ((CCM_ANALOG->AUDIO_PLL1_GEN_CTRL & CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_CLKE_MASK) == 0U)
        {
            *&(CCM_ANALOG->AUDIO_PLL1_GEN_CTRL) =
                CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_RST_MASK | CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_CLKE_MASK;
            while (!(CCM_ANALOG->AUDIO_PLL1_GEN_CTRL & CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_LOCK_MASK))
            {
            }
        }

        if ((CCM_ANALOG->AUDIO_PLL2_GEN_CTRL & CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_CLKE_MASK) == 0U)
        {
            *&(CCM_ANALOG->AUDIO_PLL2_GEN_CTRL) =
                CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_RST_MASK | CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_CLKE_MASK;
            while (!(CCM_ANALOG->AUDIO_PLL2_GEN_CTRL & CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_LOCK_MASK))
            {
            }
        }
    }
    else
    {
        /* Judge if the Audio PLL1/PLL2  are OFF, if not, disable them. */
        if (CCM_ANALOG->AUDIO_PLL1_GEN_CTRL & CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_CLKE_MASK)
        {
            *&(CCM_ANALOG->AUDIO_PLL1_GEN_CTRL) &= ~CCM_ANALOG_AUDIO_PLL1_GEN_CTRL_PLL_CLKE_MASK;
        }

        if (CCM_ANALOG->AUDIO_PLL2_GEN_CTRL & CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_CLKE_MASK)
        {
            *&(CCM_ANALOG->AUDIO_PLL2_GEN_CTRL) &= ~CCM_ANALOG_AUDIO_PLL2_GEN_CTRL_PLL_CLKE_MASK;
        }
    }
}
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
    rpmsgConfig.epName      = APP_SRTM_AUDIO_CHANNEL_NAME;
    chan                    = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    SRTM_Dispatcher_AddPeerCore(disp, core);
}

static void APP_SRTM_InitPeerCore(void)
{
    copyResourceTable();

    rpmsgHandle = rpmsg_lite_remote_init((void *)RPMSG_LITE_SRTM_SHMEM_BASE, RPMSG_LITE_SRTM_LINK_ID, RL_NO_FLAGS);
    assert(rpmsgHandle);
    if (rpmsg_lite_is_link_up(rpmsgHandle))
    {
        /* If resume context has already linked up, don't need to announce channel
         * again. */
        APP_SRTM_Linkup();
    }
    else
    {
        /* Start timer to poll linkup status. */
        xTimerStart(linkupTimer, portMAX_DELAY);
    }
}

static void APP_SRTM_InitAudioDevice(bool enable)
{
    if (enable)
    {
        sdma_config_t dmaConfig;

        SDMA_GetDefaultConfig(&dmaConfig);
        dmaConfig.ratio = kSDMA_ARMClockFreq; /* SDMA2 & SDMA3 must set the clock ratio to 1:1. */
        SDMA_Init(APP_SRTM_DMA, &dmaConfig);
    }
    else
    {
        SDMA_Deinit(APP_SRTM_DMA);
    }
}

static void APP_SRTM_InitAudioService(void)
{
    srtm_sai_sdma_config_t saiTxConfig;

    memset(&saiTxConfig, 0, sizeof(srtm_sai_sdma_config_t));
    /* Create SAI SDMA adapter */
    SAI_GetClassicI2SConfig(&saiTxConfig.config, kSAI_WordWidth16bits, kSAI_Stereo,
                            kSAI_Channel0Mask); /* SAI channel 0 used by default */
    saiTxConfig.dataLine1                  = 0U;
    saiTxConfig.config.fifo.fifoWatermark  = FSL_FEATURE_SAI_FIFO_COUNT / 2;
    saiTxConfig.mclkConfig.mclkSourceClkHz = APP_SAI_CLK_FREQ;
    saiTxConfig.mclkConfig.mclkHz =
        saiTxConfig.mclkConfig.mclkSourceClkHz;     /* Set the output mclk equal to its source clk by default */
    saiTxConfig.mclkConfig.mclkOutputEnable = true; /* Enable the MCLK output */

    saiTxConfig.stopOnSuspend = true; /* Audio data is in DRAM which is not accessable in A53 suspend. */
    saiTxConfig.threshold     = 1U;   /* Under the threshold value would trigger periodDone message to A53 */
    saiTxConfig.guardTime =
        1000; /* Unit:ms. This is a lower limit that M core should reserve such time data to wakeup A core. */
    saiTxConfig.dmaChannel                 = APP_SAI_TX_DMA_CHANNEL;
    saiTxConfig.ChannelPriority            = APP_SAI_TX_DMA_CHANNEL_PRIORITY;
    saiTxConfig.eventSource                = APP_SAI_TX_DMA_SOURCE;
    saiTxConfig.extendConfig.clkSetting    = APP_SRTM_SaiClockSet;
    saiTxConfig.extendConfig.clkGate       = APP_SRTM_ClockGateControl;
    saiTxConfig.extendConfig.audioDevInit  = APP_SRTM_InitAudioDevice;
    saiTxConfig.extendConfig.dsdSaiSetting = NULL;
    saiTxConfig.extendConfig.pcmSaiSetting = NULL;

    saiAdapter = SRTM_SaiSdmaAdapter_Create(APP_SRTM_SAI, APP_SRTM_DMA, &saiTxConfig, NULL);
    assert(saiAdapter);

    /* Set the direction of the mute pin to output */
    gpio_pin_config_t config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};
    GPIO_PinInit(APP_CODEC_MUTE_PIN, APP_CODEC_MUTE_PIN_NUM, &config);
    /* Create WM8524 codec adapter */
    wm8524Config.config.setMute     = APP_SRTM_WM8524_Mute_GPIO;
    wm8524Config.config.setProtocol = NULL;
    wm8524Config.config.protocol    = kWM8524_ProtocolI2S;

    boardCodecConfig.codecDevConfig = &wm8524Config.config;
    boardCodecConfig.codecDevType   = kCODEC_WM8524;

    CODEC_Init(&codecHandle, &boardCodecConfig);

    codecAdapter = SRTM_Wm8524Adapter_Create(&codecHandle, &wm8524Config);
    assert(codecAdapter);

    /*  Set SAI DMA IRQ Priority. */
    NVIC_SetPriority(APP_SRTM_DMA_IRQn, APP_SAI_TX_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_SRTM_SAI_IRQn, APP_SAI_IRQ_PRIO);

    /* Create and register audio service */
    SRTM_SaiSdmaAdapter_SetTxLocalBuf(saiAdapter, &g_local_buf);
    audioService = SRTM_AudioService_Create(saiAdapter, codecAdapter);
    SRTM_Dispatcher_RegisterService(disp, audioService);
}

static void APP_SRTM_InitServices(void)
{
    APP_SRTM_InitAudioService();
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
        PRINTF("\r\nFailed to create SRTM_MonitorTask task\r\n");
        while (1)
            ;
    }
    if (xTaskCreate(SRTM_DispatcherTask, "SRTM dispatcher", 512U, NULL, APP_SRTM_DISPATCHER_TASK_PRIO, NULL) != pdPASS)
    {
        PRINTF("\r\nFailed to create SRTM_DispatcherTask task\r\n");
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
