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

#if APP_SRTM_CODEC_AK4497_USED
#include "fsl_i2c.h"
#include "srtm_i2c_codec_adapter.h"
#include "fsl_ak4497.h"
#endif

#if APP_SRTM_CODEC_WM8524_USED
#include "srtm_wm8524_adapter.h"
#endif
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MS2TICK(ms) ((ms + portTICK_PERIOD_MS - 1) / portTICK_PERIOD_MS)
#define BUFFER_LEN      (80 * 1024)
uint8_t g_buffer[BUFFER_LEN];
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

#if APP_SRTM_CODEC_AK4497_USED
static bool powerOnAudioBoard = false;
#endif
static srtm_dispatcher_t disp;
static srtm_peercore_t core;
static srtm_service_t audioService;

SemaphoreHandle_t monSig;
struct rpmsg_lite_instance *rpmsgHandle;

void *rpmsgMonitorParam;
TimerHandle_t linkupTimer;
srtm_sai_adapter_t saiAdapter;
srtm_codec_adapter_t codecAdapter;
#if APP_SRTM_CODEC_AK4497_USED
srtm_i2c_codec_config_t i2cCodecConfig;
ak4497_config_t ak4497Config;
#endif
#if APP_SRTM_CODEC_WM8524_USED
srtm_wm8524_config_t wm8524Config;
#endif
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
#if APP_SRTM_CODEC_AK4497_USED
static void APP_SRTM_DsdSaiSet(void)
{
    CLOCK_SetRootDivider(kCLOCK_RootSai1, 1U, 16U); /* DSD mode, to get 22.5792MHZ. */
    IOMUXC_SetPinMux(IOMUXC_SAI1_RXD7_SAI1_TX_DATA4, 0U);
}
static void APP_SRTM_PcmSaiSet(void)
{
    CLOCK_SetRootDivider(kCLOCK_RootSai1, 1U,
                         8U); /* To get 49.152MHZ which supports 768Khz sample frequency music stream. */
    IOMUXC_SetPinMux(IOMUXC_SAI1_RXD7_SAI1_TX_SYNC, 0U);
}
#endif

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

static uint32_t APP_SRTM_SaiClockSet(mclk_type_t type)
{
    uint32_t sai_source_clk = 0;
#if APP_SRTM_CODEC_WM8524_USED
    /* Due to A core supports AK4497 playback by default, so when using the WM8524 to playback, M core needs to ensure
     * the AUDIO PLL1/2 are alive. */
    APP_SRTM_ClockGateControl(true);
#endif
    if (type == SRTM_CLK22M)
    {
#if APP_SRTM_CODEC_AK4497_USED
        CLOCK_SetRootMux(kCLOCK_RootSai1,
                         kCLOCK_SaiRootmuxAudioPll2); /* Set SAI source to Audio PLL2 361267200Hz to get 22.5792MHz */
        sai_source_clk = APP_AUDIO_PLL2_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai1)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai1));
#endif
#if APP_SRTM_CODEC_WM8524_USED
        CLOCK_SetRootMux(kCLOCK_RootSai3,
                         kCLOCK_SaiRootmuxAudioPll2); /* Set SAI source to Audio PLL2 361267200Hz to get 22.5792MHz */
        sai_source_clk = APP_AUDIO_PLL2_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai3));
#endif
    }
    else
    {
#if APP_SRTM_CODEC_AK4497_USED
        CLOCK_SetRootMux(kCLOCK_RootSai1,
                         kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to Audio PLL1 393216000Hz to get 49.152Mhz*/
        sai_source_clk = APP_AUDIO_PLL1_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai1)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai1));
#endif
#if APP_SRTM_CODEC_WM8524_USED
        CLOCK_SetRootMux(kCLOCK_RootSai3,
                         kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to Audio PLL1 393216000Hz to get 24.576Mhz*/
        sai_source_clk = APP_AUDIO_PLL1_FREQ / (CLOCK_GetRootPreDivider(kCLOCK_RootSai3)) /
                         (CLOCK_GetRootPostDivider(kCLOCK_RootSai3));
#endif
    }
    return sai_source_clk;
}

#if APP_SRTM_CODEC_AK4497_USED
static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < APP_SRTM_I2C_DELAY; i++)
    {
        __NOP();
    }
}
void APP_SRTM_I2C_ReleaseBus(void)
{
    uint8_t i                    = 0;
    gpio_pin_config_t pin_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};

    IOMUXC_SetPinMux(IOMUXC_I2C3_SCL_GPIO5_IO18, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SCL_GPIO5_IO18, IOMUXC_SW_PAD_CTL_PAD_DSE(6U) | IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
                                                        IOMUXC_SW_PAD_CTL_PAD_ODE_MASK |
                                                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_I2C3_SDA_GPIO5_IO19, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SDA_GPIO5_IO19, IOMUXC_SW_PAD_CTL_PAD_DSE(6U) | IOMUXC_SW_PAD_CTL_PAD_FSEL(2U) |
                                                        IOMUXC_SW_PAD_CTL_PAD_ODE_MASK |
                                                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    GPIO_PinInit(APP_AUDIO_I2C_SCL_GPIO, APP_AUDIO_I2C_SCL_PIN, &pin_config);
    GPIO_PinInit(APP_AUDIO_I2C_SDA_GPIO, APP_AUDIO_I2C_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(APP_AUDIO_I2C_SDA_GPIO, APP_AUDIO_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(APP_AUDIO_I2C_SCL_GPIO, APP_AUDIO_I2C_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(APP_AUDIO_I2C_SDA_GPIO, APP_AUDIO_I2C_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(APP_AUDIO_I2C_SCL_GPIO, APP_AUDIO_I2C_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(APP_AUDIO_I2C_SCL_GPIO, APP_AUDIO_I2C_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(APP_AUDIO_I2C_SDA_GPIO, APP_AUDIO_I2C_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(APP_AUDIO_I2C_SCL_GPIO, APP_AUDIO_I2C_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(APP_AUDIO_I2C_SDA_GPIO, APP_AUDIO_I2C_SDA_PIN, 1U);
    i2c_release_bus_delay();
}
/*
 * Audio board must be powered after the i.MX8MM EVK is powered.
 * To achieve this, the power gate of the audio board power is controled by the
 * PORT0_1 pin of the PCA6416APW device on the EVK board.
 * Therefore, the "APP_SRTM_PowerOnAudioBoard()" would be specially added in
 * this case.
 */
static void APP_SRTM_PowerOnAudioBoard(void)
{
    uint8_t temp = 0;
    i2c_master_transfer_t masterXfer;

    /* Prepare transfer structure. */
    masterXfer.slaveAddress   = 0x20; /* The PCA6416APW IC address */
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = 0x6U; /* Configure the PORT0 of the PCA6416APW */
    masterXfer.subaddressSize = 1U;
    temp                      = 0xFD; /* Set P0_1 to output direction */
    masterXfer.data           = &temp;
    masterXfer.dataSize       = 1U;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    I2C_MasterTransferBlocking(APP_SRTM_I2C, &masterXfer);

    i2c_release_bus_delay(); /* Ensure the I2C bus free. */
    temp                      = 0xFF;
    masterXfer.subaddress     = 0x2U;       /* Select the outputregister for port 0 */
    masterXfer.direction      = kI2C_Write; /* Make P0_1 output high level */
    masterXfer.subaddressSize = 1U;
    masterXfer.data           = &temp;
    I2C_MasterTransferBlocking(APP_SRTM_I2C, &masterXfer);
}
static void APP_SRTM_InitI2C(I2C_Type *base, uint32_t baudrate, uint32_t clockrate)
{
    i2c_master_config_t masterConfig;

    /*
     * masterConfig->baudRate_Bps = 100000U;
     * masterConfig->enableHighDrive = false;
     * masterConfig->enableStopHold = false;
     * masterConfig->glitchFilterWidth = 0U;
     * masterConfig->enableMaster = true;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = baudrate;
    /*  Set I2C Master IRQ Priority. */
    NVIC_SetPriority(APP_SRTM_I2C_IRQn, APP_SRTM_I2C_IRQ_PRIO);

    I2C_MasterInit(base, &masterConfig, clockrate);
}

static status_t APP_SRTM_ReadCodecRegMap(void *handle, uint32_t reg, uint32_t *val)
{
    status_t status = kStatus_Success;
    LPM_IncreseBlockSleepCnt();
    status = AK4497_ReadReg((ak4497_handle_t *)((uint32_t) & ((codec_handle_t *)handle)->codecDevHandle), reg,
                            (uint8_t *)val);
    LPM_DecreaseBlockSleepCnt();

    return status;
}

static status_t APP_SRTM_WriteCodecRegMap(void *handle, uint32_t reg, uint32_t val)
{
    status_t status = kStatus_Success;
    LPM_IncreseBlockSleepCnt();
    status = AK4497_WriteReg((ak4497_handle_t *)((uint32_t) & ((codec_handle_t *)handle)->codecDevHandle), reg, val);
    LPM_DecreaseBlockSleepCnt();

    return status;
}
#endif
#if APP_SRTM_CODEC_WM8524_USED
static void APP_SRTM_WM8524_Mute_GPIO(uint32_t output)
{
    GPIO_PinWrite(APP_CODEC_MUTE_PIN, APP_CODEC_MUTE_PIN_NUM, output);
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
        sdma_config_t dmaConfig = {0};

        /* Create SDMA handle */
        SDMA_GetDefaultConfig(&dmaConfig);
        dmaConfig.ratio = kSDMA_ARMClockFreq; /* SDMA2 & SDMA3 must set the clock ratio to 1:1. */
        SDMA_Init(APP_SRTM_DMA, &dmaConfig);

#if APP_SRTM_CODEC_AK4497_USED

        APP_SRTM_InitI2C(APP_SRTM_I2C, APP_SRTM_I2C_BAUDRATE, APP_SRTM_I2C_CLOCK_FREQ);
        if (!powerOnAudioBoard)
        {
            APP_SRTM_PowerOnAudioBoard();
            powerOnAudioBoard = true;
            /* After power up the audio board, need to wait for a while to make sure the codec on the audio board is
             * stable and the I2C communication is ok.*/
            SDK_DelayAtLeastUs(500000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }
#endif
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
    saiTxConfig.dataLine2                  = 4U; /* SAI channel 4 is used as the another channel for the DSD stream */
    saiTxConfig.config.fifo.fifoWatermark  = FSL_FEATURE_SAI_FIFO_COUNT / 2U;
    saiTxConfig.mclkConfig.mclkSourceClkHz = APP_SAI_CLK_FREQ;
    saiTxConfig.mclkConfig.mclkHz =
        saiTxConfig.mclkConfig.mclkSourceClkHz;     /* Set the output mclk equal to its source clk by default */
    saiTxConfig.mclkConfig.mclkOutputEnable = true; /* Enable the MCLK output */

    saiTxConfig.stopOnSuspend = true; /* Audio data is in DRAM which is not accessable in A53 suspend. */
    saiTxConfig.threshold     = 1U;   /* Under the threshold value would trigger periodDone message to A53 */
    saiTxConfig.guardTime =
        1000; /* Unit:ms. This is a lower limit that M core should reserve such time data to wakeup A core. */
    saiTxConfig.dmaChannel                = APP_SAI_TX_DMA_CHANNEL;
    saiTxConfig.ChannelPriority           = APP_SAI_TX_DMA_CHANNEL_PRIORITY;
    saiTxConfig.eventSource               = APP_SAI_TX_DMA_SOURCE;
    saiTxConfig.extendConfig.audioDevInit = APP_SRTM_InitAudioDevice;
#if APP_SRTM_CODEC_AK4497_USED
    saiTxConfig.extendConfig.dsdSaiSetting = APP_SRTM_DsdSaiSet;
    saiTxConfig.extendConfig.pcmSaiSetting = APP_SRTM_PcmSaiSet;
#endif

    saiTxConfig.extendConfig.clkSetting = APP_SRTM_SaiClockSet;
    saiTxConfig.extendConfig.clkGate    = APP_SRTM_ClockGateControl;

    saiAdapter = SRTM_SaiSdmaAdapter_Create(APP_SRTM_SAI, APP_SRTM_DMA, &saiTxConfig, NULL);
    assert(saiAdapter);
#if APP_SRTM_CODEC_AK4497_USED
    AK4497_DefaultConfig(&ak4497Config);
    ak4497Config.i2cConfig.codecI2CInstance    = APP_CODEC_I2C_INSTANCE;
    ak4497Config.i2cConfig.codecI2CSourceClock = APP_SRTM_I2C_CLOCK_FREQ;
    ak4497Config.slaveAddress                  = AK4497_I2C_ADDR;

    boardCodecConfig.codecDevConfig = &ak4497Config;
    boardCodecConfig.codecDevType   = kCODEC_AK4497;

    CODEC_Init(&codecHandle, &boardCodecConfig);

    /* Create I2C Codec adaptor */
    i2cCodecConfig.addrType    = kCODEC_RegAddr8Bit;
    i2cCodecConfig.slaveAddr   = ak4497Config.slaveAddress;
    i2cCodecConfig.i2cHandle   = (((ak4497_handle_t *)((uint32_t)(codecHandle.codecDevHandle)))->i2cHandle);
    i2cCodecConfig.regWidth    = kCODEC_RegWidth8Bit;
    i2cCodecConfig.writeRegMap = APP_SRTM_WriteCodecRegMap;
    i2cCodecConfig.readRegMap  = APP_SRTM_ReadCodecRegMap;
    codecAdapter               = SRTM_I2CCodecAdapter_Create(&codecHandle, &i2cCodecConfig);
    assert(codecAdapter);
#endif
#if APP_SRTM_CODEC_WM8524_USED
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
#endif
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
