/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "fsl_lpi2c.h"

#include "srtm_audio_service.h"
#include "srtm_dispatcher.h"
#include "srtm_peercore.h"
#include "srtm_message.h"
#include "srtm_rpmsg_endpoint.h"
#include "srtm_i2c_service.h"
#include "srtm_sai_edma_adapter.h"
#include "srtm_pdm_edma_adapter.h"
#include "srtm_channel_struct.h"

#include "app_srtm.h"
#include "board.h"
#include "rsc_table.h"
#include "fsl_mu.h"
#include "fsl_sema42.h"

static srtm_sai_adapter_t saiAdapter;
srtm_sai_adapter_t pdmAdapter;
static srtm_dispatcher_t disp;
static srtm_peercore_t core;
static srtm_service_t i2cService;
static srtm_service_t audioService;
static uint8_t edmaUseCnt = 0U;
static SemaphoreHandle_t monSig;
volatile app_srtm_state_t srtmState;
static struct rpmsg_lite_instance *rpmsgHandle;
static app_rpmsg_monitor_t rpmsgMonitor;
static void *rpmsgMonitorParam;
static TimerHandle_t linkupTimer;
#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
#define BUFFER_LEN (64 * 1024)
#if (defined(__ICCARM__))
static uint8_t g_buffer[BUFFER_LEN] @"AudioBuf";
#else
static uint8_t g_buffer[BUFFER_LEN] __attribute__((section("AudioBuf,\"w\",%nobits @")));
#endif
static srtm_sai_edma_local_buf_t g_local_buf = {
    .buf       = (uint8_t *)&g_buffer,
    .bufSize   = BUFFER_LEN,
    .periods   = SRTM_SAI_EDMA_MAX_LOCAL_BUF_PERIODS,
    .threshold = 1,

};
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static srtm_status_t APP_SRTM_I2C_Read(srtm_i2c_adapter_t adapter,
                                       uint32_t base_addr,
                                       srtm_i2c_type_t type,
                                       uint16_t slaveAddr,
                                       uint8_t *buf,
                                       uint16_t len,
                                       uint16_t flags);

static srtm_status_t APP_SRTM_I2C_Write(srtm_i2c_adapter_t adapter,
                                        uint32_t base_addr,
                                        srtm_i2c_type_t type,
                                        uint16_t slaveAddr,
                                        uint8_t *buf,
                                        uint16_t len,
                                        uint16_t flags);

static srtm_status_t APP_SRTM_I2C_SwitchChannel(srtm_i2c_adapter_t adapter,
                                                uint32_t base_addr,
                                                srtm_i2c_type_t type,
                                                uint16_t slaveAddr,
                                                srtm_i2c_switch_channel channel);

/* LPI2C1, LPI2C2 in AONMIX(Always On Domain) */
static struct _i2c_bus platform_i2c_buses[] = {
    {.bus_id         = 0,
     .base_addr      = LPI2C1_BASE,
     .type           = SRTM_I2C_TYPE_LPI2C,
     .switch_idx     = I2C_SWITCH_NONE,
     .switch_channel = SRTM_I2C_SWITCH_CHANNEL_UNSPECIFIED},
    {.bus_id         = 1,
     .base_addr      = LPI2C2_BASE,
     .type           = SRTM_I2C_TYPE_LPI2C,
     .switch_idx     = I2C_SWITCH_NONE,
     .switch_channel = SRTM_I2C_SWITCH_CHANNEL_UNSPECIFIED},
};

static struct _srtm_i2c_adapter i2c_adapter = {.read          = APP_SRTM_I2C_Read,
                                               .write         = APP_SRTM_I2C_Write,
                                               .switchchannel = APP_SRTM_I2C_SwitchChannel,
                                               .bus_structure = {
                                                   .buses      = platform_i2c_buses,
                                                   .bus_num    = sizeof(platform_i2c_buses) / sizeof(struct _i2c_bus),
                                                   .switch_num = 0,
                                               }};

void APP_SRTM_SetRpmsgMonitor(app_rpmsg_monitor_t monitor, void *param)
{
    rpmsgMonitor      = monitor;
    rpmsgMonitorParam = param;
}

static srtm_status_t APP_SRTM_I2C_Write(srtm_i2c_adapter_t adapter,
                                        uint32_t base_addr,
                                        srtm_i2c_type_t type,
                                        uint16_t slaveAddr,
                                        uint8_t *buf,
                                        uint16_t len,
                                        uint16_t flags)
{
    status_t retVal   = kStatus_Fail;
    uint32_t needStop = (flags & SRTM_I2C_FLAG_NEED_STOP) ? kLPI2C_TransferDefaultFlag : kLPI2C_TransferNoStopFlag;

    switch (type)
    {
        case SRTM_I2C_TYPE_LPI2C:
            retVal = BOARD_LPI2C_Send((LPI2C_Type *)base_addr, slaveAddr, 0, 0, buf, len, needStop);
            break;
        default:
            break;
    }
    return (retVal == kStatus_Success) ? SRTM_Status_Success : SRTM_Status_TransferFailed;
}

static srtm_status_t APP_SRTM_I2C_Read(srtm_i2c_adapter_t adapter,
                                       uint32_t base_addr,
                                       srtm_i2c_type_t type,
                                       uint16_t slaveAddr,
                                       uint8_t *buf,
                                       uint16_t len,
                                       uint16_t flags)
{
    status_t retVal   = kStatus_Fail;
    uint32_t needStop = (flags & SRTM_I2C_FLAG_NEED_STOP) ? kLPI2C_TransferDefaultFlag : kLPI2C_TransferNoStopFlag;

    switch (type)
    {
        case SRTM_I2C_TYPE_LPI2C:
            retVal = BOARD_LPI2C_Receive((LPI2C_Type *)base_addr, slaveAddr, 0, 0, buf, len, needStop);
            break;
        default:
            break;
    }
    return (retVal == kStatus_Success) ? SRTM_Status_Success : SRTM_Status_TransferFailed;
}

static srtm_status_t APP_SRTM_I2C_SwitchChannel(srtm_i2c_adapter_t adapter,
                                                uint32_t base_addr,
                                                srtm_i2c_type_t type,
                                                uint16_t slaveAddr,
                                                srtm_i2c_switch_channel channel)
{
    uint8_t txBuff[1];

    assert(channel < SRTM_I2C_SWITCH_CHANNEL_UNSPECIFIED);
    txBuff[0] = 1 << (uint8_t)channel;
    return adapter->write(adapter, base_addr, type, slaveAddr, txBuff, sizeof(txBuff),
                          SRTM_I2C_FLAG_NEED_STOP); // APP_SRTM_I2C_Write
}

static void APP_SRTM_InitI2CDevice(void)
{
    lpi2c_master_config_t masterConfig;

    LPI2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz = LPI2C1_BAUDRATE;
    LPI2C_MasterInit(LPI2C1, &masterConfig, I2C_SOURCE_CLOCK_FREQ_LPI2C1);
    masterConfig.baudRate_Hz = LPI2C2_BAUDRATE;
    LPI2C_MasterInit(LPI2C2, &masterConfig, I2C_SOURCE_CLOCK_FREQ_LPI2C2);
}

static void APP_SRTM_InitI2CService(void)
{
    APP_SRTM_InitI2CDevice();
    i2cService = SRTM_I2CService_Create(&i2c_adapter);
    SRTM_Dispatcher_RegisterService(disp, i2cService);
}

#if defined(BOARD_USE_DDR_RETENTION) && BOARD_USE_DDR_RETENTION
void APP_SRTM_PreCopyCallback()
{
    /* Sema42 lock */
    SEMA42_Lock(APP_SEMA42, APP_SEMA42_GATE, APP_CORTEX_M_DID);

    /* Put DDR exit retention if used by A55 side */
    if (R32(DDR_RETENTION_BASE) & DDR_RETENTION_A55_FLAG)
    {
        /* Put DDR exit retention */
        BOARD_DRAMExitRetention();
    }

    /* M33 is using ddr, clear DDR_RETENTION_M33_FLAG */
    CLRBIT32(DDR_RETENTION_BASE, DDR_RETENTION_M33_FLAG);

    /* Sema42 unlock */
    SEMA42_Unlock(APP_SEMA42, APP_SEMA42_GATE);
}

void APP_SRTM_PostCopyCallback()
{
    /* Sema42 lock */
    SEMA42_Lock(APP_SEMA42, APP_SEMA42_GATE, APP_CORTEX_M_DID);

    /* Put ddr into retention if not used by A55 side */
    if (R32(DDR_RETENTION_BASE) & DDR_RETENTION_A55_FLAG)
    {
        /* Put ddr into retention */
        BOARD_DRAMEnterRetention();
    }

    /* M33 is not using ddr, clear DDR_RETENTION_M33_FLAG */
    SETBIT32(DDR_RETENTION_BASE, DDR_RETENTION_M33_FLAG);

    /* Unlock */
    SEMA42_Unlock(APP_SEMA42, APP_SEMA42_GATE);
}

/*
 * As IMX93 don't have hardware method to get peer core power state, Only life cycle service cannot express the complete
 * power change process. So Mcore will default peer core to actived state, in time for the peer side to enter the
 * low-power state. For LPA example, Mcore need ensure that ddr is available when sending messages via rpmsg machine.
 */
/* rpmsg_lite_send pre call back */
void APP_SRTM_SendMessagePreCallback(srtm_channel_t channel, void *data, uint32_t len)
{
    APP_SRTM_PreCopyCallback();
}

/* rpmsg_lite_send post call back */
void APP_SRTM_SendMessagePostCallback(srtm_channel_t channel, void *data, uint32_t len)
{
    APP_SRTM_PostCopyCallback();
}
#endif

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

static void APP_SRTM_Linkup(void)
{
    srtm_channel_t chan;
    srtm_rpmsg_endpoint_config_t rpmsgConfig;

    /* Create SRTM peer core */
    core = SRTM_PeerCore_Create(PEER_CORE_ID);
    /* Set peer core state to activated */
    SRTM_PeerCore_SetState(core, SRTM_PeerCore_State_Activated);

    /* Common RPMsg channel config */
    rpmsgConfig.localAddr   = RL_ADDR_ANY;
    rpmsgConfig.peerAddr    = RL_ADDR_ANY;
    rpmsgConfig.rpmsgHandle = rpmsgHandle;

    rpmsgConfig.epName = APP_SRTM_AUDIO_CHANNEL_NAME;
    chan               = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);
#if defined(BOARD_USE_DDR_RETENTION) && BOARD_USE_DDR_RETENTION
    chan->sendDataPreCallback  = APP_SRTM_SendMessagePreCallback;
    chan->sendDataPostCallback = APP_SRTM_SendMessagePostCallback;
#endif
    assert((audioService != NULL) && (saiAdapter != NULL));
    SRTM_AudioService_BindChannel(audioService, saiAdapter, chan);

    rpmsgConfig.epName = APP_SRTM_PDM_CHANNEL_NAME;
    chan               = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);
    assert((audioService != NULL) && (pdmAdapter != NULL));
    SRTM_AudioService_BindChannel(audioService, pdmAdapter, chan);

    /* Create and add SRTM I2C channel to peer core */
    rpmsgConfig.epName = APP_SRTM_I2C_CHANNEL_NAME;
    chan               = SRTM_RPMsgEndpoint_Create(&rpmsgConfig);
    SRTM_PeerCore_AddChannel(core, chan);

    SRTM_Dispatcher_AddPeerCore(disp, core);
}

static void APP_SRTM_NotifyPeerCoreReady(struct rpmsg_lite_instance *rpmsgHandle, bool ready)
{
    /* deinit and init app task(str_echo/pingpong rpmsg) in APP_SRTM_StateReboot only */
    if (rpmsgMonitor && (srtmState == APP_SRTM_StateReboot))
    {
        rpmsgMonitor(rpmsgHandle, ready, rpmsgMonitorParam);
    }
}

static void APP_SRTM_InitPeerCore(void)
{
    copyResourceTable();

    rpmsgHandle = rpmsg_lite_remote_init((void *)RPMSG_LITE_SRTM_SHMEM_BASE, RPMSG_LITE_SRTM_LINK_ID, RL_NO_FLAGS);
    assert(rpmsgHandle);

    APP_SRTM_NotifyPeerCoreReady(rpmsgHandle, true);

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

static void APP_SRTM_ResetServices(void)
{
    /* When A Core resets, we need to avoid async event to send to A Core, to do in here */
}

static void APP_SRTM_DeinitPeerCore(void)
{
    /* Stop linkupTimer if it's started. */
    xTimerStop(linkupTimer, portMAX_DELAY);

    /* Notify application for the peer core disconnection. */
    APP_SRTM_NotifyPeerCoreReady(rpmsgHandle, false);

    if (core)
    {
        /* Need to let services know peer core is now down. */
        APP_SRTM_ResetServices();

        SRTM_Dispatcher_RemovePeerCore(disp, core);
        SRTM_PeerCore_Destroy(core);
        core = NULL;
    }

    if (rpmsgHandle)
    {
        rpmsg_lite_deinit(rpmsgHandle);
        rpmsgHandle = NULL;
    }
}

static void APP_SRTM_InitPdmDevice(bool enable)
{
    edma_config_t dmaConfig;
    if (enable)
    {
        if (edmaUseCnt == 0U)
        {
            EDMA_GetDefaultConfig(&dmaConfig);
            EDMA_Init(APP_SRTM_DMA, &dmaConfig);
        }
        edmaUseCnt++;
    }
    else
    {
        edmaUseCnt--;
        if (edmaUseCnt == 0U)
        {
            EDMA_Deinit(APP_SRTM_DMA);
        }
    }
}

static void APP_SRTM_InitAudioDevice(void)
{
    edma_config_t dmaConfig;

    /* Initialize DMA4 for SAI */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DMA4, &dmaConfig);

    /* Initialize DMAMUX for SAI */
    EDMA_SetChannelMux(DMA4, APP_SAI_TX_DMA_CHANNEL, kDma4RequestMuxSai3Tx);
    EDMA_SetChannelMux(DMA4, APP_SAI_RX_DMA_CHANNEL, kDma4RequestMuxSai3Rx);

    APP_SRTM_InitPdmDevice(true);
}

static uint32_t APP_SRTM_ConfPdmDevice(srtm_audio_format_type_t format, uint32_t srate)
{
    return CLOCK_GetIpFreq(kCLOCK_Root_Pdm);
}

static void APP_SRTM_InitAudioService(void)
{
    srtm_sai_edma_config_t saiTxConfig;
    srtm_sai_edma_config_t saiRxConfig;
    srtm_pdm_edma_config_t pdmConfig;

    APP_SRTM_InitAudioDevice();

    memset(&saiTxConfig, 0, sizeof(saiTxConfig));
    memset(&saiRxConfig, 0, sizeof(saiRxConfig));
    memset(&pdmConfig, 0, sizeof(srtm_pdm_edma_config_t));

    /*  Set IRQ Priorities. */
    NVIC_SetPriority(APP_DMA4_IRQN(APP_SAI_TX_DMA_CHANNEL), APP_SAI_TX_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_DMA4_IRQN(APP_SAI_RX_DMA_CHANNEL), APP_SAI_RX_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_SRTM_SAI_IRQn, APP_SAI_IRQ_PRIO);
    NVIC_SetPriority(APP_DMA3_IRQN(APP_PDM_RX_DMA_CHANNEL), APP_PDM_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_DMA4_IRQN(APP_MEM2MEM_W_DMA_CHANNEL), APP_M2M_DMA_IRQ_PRIO);
    NVIC_SetPriority(APP_DMA4_IRQN(APP_MEM2MEM_R_DMA_CHANNEL), APP_M2M_DMA_IRQ_PRIO);

    /* Create SAI EDMA adapter */
    SAI_GetClassicI2SConfig(&saiTxConfig.config, kSAI_WordWidth16bits, kSAI_Stereo, kSAI_Channel0Mask);
    saiTxConfig.config.syncMode           = kSAI_ModeSync; /* Tx in Sync mode */
    saiTxConfig.config.fifo.fifoWatermark = FSL_FEATURE_SAI_FIFO_COUNTn(APP_SRTM_SAI) - 1;
    saiTxConfig.mclk                      = CLOCK_GetIpFreq(kCLOCK_Root_Sai3);
#if defined(DEMO_SAI_TX_CONFIG_StopOnSuspend)
    saiTxConfig.stopOnSuspend = DEMO_SAI_TX_CONFIG_StopOnSuspend;
#else
    saiTxConfig.stopOnSuspend = false; /* Keep playing audio on APD suspend. */
#endif
    saiTxConfig.threshold = 1U; /* Every period transmitted triggers periodDone message to A core. */
    saiTxConfig.guardTime =
        1000; /* Unit:ms. This is a lower limit that M core should reserve such time data to wakeup A core. */
    saiTxConfig.dmaChannel = APP_SAI_TX_DMA_CHANNEL;

    SAI_GetClassicI2SConfig(&saiRxConfig.config, kSAI_WordWidth16bits, kSAI_Stereo, kSAI_Channel0Mask);
    saiRxConfig.config.syncMode           = kSAI_ModeAsync; /* Rx in async mode */
    saiRxConfig.config.fifo.fifoWatermark = 1;
    saiRxConfig.mclk                      = saiTxConfig.mclk;
#if defined(DEMO_SAI_TX_CONFIG_StopOnSuspend)
    saiTxConfig.stopOnSuspend = DEMO_SAI_TX_CONFIG_StopOnSuspend;
#else
    saiRxConfig.stopOnSuspend = false; /* Keep recording data on APD suspend. */
#endif
    saiRxConfig.threshold  = UINT32_MAX; /* Every period received triggers periodDone message to A core. */
    saiRxConfig.dmaChannel = APP_SAI_RX_DMA_CHANNEL;

    saiAdapter = SRTM_SaiEdmaAdapter_Create(SAI3, (EDMA_Type *)DMA4, &saiTxConfig, &saiRxConfig);
    assert(saiAdapter);

#if SRTM_SAI_EDMA_LOCAL_BUF_ENABLE
    SRTM_SaiEdmaAdapter_SetTxLocalBuf(saiAdapter, &g_local_buf);
    SRTM_SaiEdmaAdapter_SetTxPreCopyCallback(saiAdapter, APP_SRTM_PreCopyCallback);
    SRTM_SaiEdmaAdapter_SetTxPostCopyCallback(saiAdapter, APP_SRTM_PostCopyCallback);
#endif

    /* Creat PDM SDMA adapter */
    pdmConfig.stopOnSuspend = false; // Keep recording on A core suspend.
    pdmConfig.dmaChannel    = APP_PDM_RX_DMA_CHANNEL;
    // pdmConfig.channelPriority           = APP_PDM_RX_DMA_CHANNEL_PRIORITY;
    // pdmConfig.eventSource               = APP_PDM_RX_DMA_SOURCE;
    pdmConfig.extendConfig.audioDevInit = APP_SRTM_InitPdmDevice;
    pdmConfig.extendConfig.audioDevConf = APP_SRTM_ConfPdmDevice;
    pdmConfig.pdmSrcClk                 = CLOCK_GetIpFreq(kCLOCK_Root_Pdm);
    pdmConfig.config.qualityMode        = APP_PDM_QUALITY_MODE;
    pdmConfig.config.enableDoze         = false;
    pdmConfig.config.fifoWatermark      = FSL_FEATURE_PDM_FIFO_DEPTH / 2U;
    pdmConfig.config.cicOverSampleRate  = APP_PDM_CICOVERSAMPLE_RATE;
    pdmConfig.channelConfig.gain        = APP_PDM_CHANNEL_GAIN;
    pdmConfig.channelConfig.cutOffFreq  = APP_PDM_CHANNEL_CUTOFF_FREQ;
    pdmAdapter                          = SRTM_PdmEdmaAdapter_Create(APP_SRTM_PDM, APP_SRTM_DMA, &pdmConfig);
    assert(pdmAdapter);

    audioService = SRTM_AudioService_Create(saiAdapter, NULL);
    SRTM_AudioService_AddAudioInterface(audioService, pdmAdapter);
    SRTM_Dispatcher_RegisterService(disp, audioService);
}

static void APP_SRTM_InitServices(void)
{
    APP_SRTM_InitI2CService();
    APP_SRTM_InitAudioService();
}

static void SRTM_DispatcherTask(void *pvParameters)
{
    SRTM_Dispatcher_Run(disp);
}

static void SRTM_MonitorTask(void *pvParameters)
{
    app_srtm_state_t state = APP_SRTM_StateShutdown;

    /* Initialize services and add to dispatcher */
    APP_SRTM_InitServices();

    /* Start SRTM dispatcher */
    SRTM_Dispatcher_Start(disp);

    /* Monitor peer core state change */
    while (true)
    {
        xSemaphoreTake(monSig, portMAX_DELAY);

        if (state == srtmState)
        {
            continue;
        }

        switch (srtmState)
        {
            case APP_SRTM_StateRun:
                assert(state == APP_SRTM_StateShutdown);
                PRINTF("Start SRTM communication\r\n");
                SRTM_Dispatcher_Stop(disp);

                APP_SRTM_InitPeerCore();
                SRTM_Dispatcher_Start(disp);
                state = APP_SRTM_StateRun;
                break;

            case APP_SRTM_StateLinkedUp:
                if (state == APP_SRTM_StateRun)
                {
                    PRINTF("Handle Peer Core Linkup\r\n");
                    SRTM_Dispatcher_Stop(disp);
                    APP_SRTM_Linkup();
                    SRTM_Dispatcher_Start(disp);
                }
                break;

            case APP_SRTM_StateReboot:
                assert(state == APP_SRTM_StateRun);

                PRINTF("Handle Peer Core Reboot\r\n");

                SRTM_Dispatcher_Stop(disp);
                /* Remove peer core from dispatcher */
                APP_SRTM_DeinitPeerCore();

                /* enable clock of MU before accessing registers of MU */
                MU_Init(RPMSG_LITE_M33_A55_MU);

#if !(defined(FSL_FEATURE_MU_NO_BOOT) && (0 != FSL_FEATURE_MU_NO_BOOT))
                /* Relase core */
                MU_BootOtherCore(RPMSG_LITE_M33_A55_MU, (mu_core_boot_mode_t)0);
#endif

                /* Initialize peer core and add to dispatcher */
                APP_SRTM_InitPeerCore();

                /* Restore srtmState to Run. */
                srtmState = APP_SRTM_StateRun;

                SRTM_Dispatcher_Start(disp);

                /* hold A core for next reboot */
                // MU_HoldOtherCoreReset(RPMSG_LITE_M33_A55_MU);

                /* Do not need to change state. It's still Run. */
                break;

            default:
                assert(false);
                break;
        }
    }
}

void APP_SRTM_Init(void)
{
    monSig = xSemaphoreCreateBinary();
    assert(monSig);

    linkupTimer =
        xTimerCreate("Linkup", pdMS_TO_TICKS(APP_LINKUP_TIMER_PERIOD_MS), pdFALSE, NULL, APP_LinkupTimerCallback);
    assert(linkupTimer);

    /* Create SRTM dispatcher */
    disp = SRTM_Dispatcher_Create();
    xTaskCreate(SRTM_MonitorTask, "SRTM monitor", 256U, NULL, APP_SRTM_MONITOR_TASK_PRIO, NULL);
    xTaskCreate(SRTM_DispatcherTask, "SRTM dispatcher", 512U, NULL, APP_SRTM_DISPATCHER_TASK_PRIO, NULL);
}

void APP_SRTM_StartCommunication(void)
{
    srtmState = APP_SRTM_StateRun;
    xSemaphoreGive(monSig);
}

/* true: idle; false: busy(playing audio) */
bool APP_SRTM_IsAudioServiceIdle(void)
{
    srtm_audio_state_t TxState, RxState;
    static m_core_mode_e curPwrState = RunMode;

    SRTM_SaiEdmaAdapter_GetAudioServiceState(saiAdapter, &TxState, &RxState);

    if (TxState == SRTM_AudioStateClosed && RxState == SRTM_AudioStateClosed)
    {
        if (curPwrState != StopMode)
        {
            PRINTF("\r\nNo audio playback, M core enters STOP mode!\r\n");
            curPwrState = StopMode;
        }

        return true;
    }
    else
    {
        if (curPwrState != RunMode)
        {
            PRINTF("\r\nPlayback is running, M core enters RUN mode!\r\n");
            curPwrState = RunMode;
        }
        return false;
    }
}
