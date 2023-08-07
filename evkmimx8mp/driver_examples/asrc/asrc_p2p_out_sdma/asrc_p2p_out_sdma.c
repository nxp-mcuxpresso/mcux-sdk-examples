/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_sai_sdma.h"
#include "music.h"
#include "fsl_codec_common.h"
#include "fsl_asrc_sdma.h"
#include "fsl_asrc.h"
#include "fsl_sdma_script.h"
#include "fsl_wm8960.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_codec_adapter.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SAI                   (I2S3)
#define DEMO_ASRC                  ASRC
#define DEMO_SAI_CLK_FREQ          24576000U
#define DEMO_CODEC_WM8960          (1)
#define DEMO_IRQn                  I2S3_IRQn
#define DEMO_DMA                   SDMAARM3
#define DEMO_ASRC_IN_SDMA_CHANNEL  2
#define DEMO_ASRC_OUT_SDMA_CHANNEL 3
#define DEMO_ASRC_IN_SDMA_SOURCE   16
#define DEMO_ASRC_OUT_SDMA_SOURCE  17
#define DEMO_ASRC_CONTEXT          kASRC_Context0
#define DEMO_SAI_TX_SOURCE         (5)
#define SAI_UserIRQHandler         I2S3_IRQHandler
/*set Bclk source to Mclk clock*/
#define DEMO_SAI_CLOCK_SOURCE     (1U)
#define DEMO_SAI_CHANNEL_NUM      (0U)
#define SDMA_FREQ_EQUALS_ARM      (1U)
#define DEMO_ASRC_IN_SAMPLE_RATE  48000
#define DEMO_ASRC_OUT_SAMPLE_RATE 16000

#define I2C_RELEASE_SDA_GPIO  GPIO5
#define I2C_RELEASE_SDA_PIN   19U
#define I2C_RELEASE_SCL_GPIO  GPIO5
#define I2C_RELEASE_SCL_PIN   18U
#define I2C_RELEASE_BUS_COUNT 100U

#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth16bits
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ

/* convert buffer size, please note that the maximum size  of once transfer that SDMA can support is 64k */
#define DEMO_AUDIO_BUFFER_SIZE (16000U)
#define DEMO_AUDIO_SAMPLES     (MUSIC_LEN / 2U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
void BOARD_MASTER_CLOCK_CONFIG(void);
static void asrcConvertAudio(void *in, uint32_t dataSize);
static void startSai(bool start);
/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 24576000U,
               .sampleRate = kWM8960_AudioSampleRate48KHz,
               .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};
sai_master_clock_t mclkConfig;

volatile bool isConvertFinished = false;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;
/*! @brief ASRC sdma handle */
AT_NONCACHEABLE_SECTION_ALIGN(static asrc_sdma_handle_t s_asrcHandle, 4U);
AT_NONCACHEABLE_SECTION_ALIGN(static sdma_handle_t s_asrcInSDMAHandle, 4U);
AT_NONCACHEABLE_SECTION_ALIGN(static sdma_handle_t s_asrcOutSDMAHandle, 4U);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_context_data_t s_asrcInSDMAcontext, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_context_data_t s_asrcOutSDMAcontext, 4);

/*! @brief ASRC context config */
static asrc_context_config_t s_asrcContextConfig;
static asrc_p2p_sdma_config_t s_asrcSDMAConfig = {
    .eventSource      = DEMO_SAI_TX_SOURCE,
    .channel          = 0U,
    .enableContinuous = true,
    .startPeripheral  = startSai,
};
const short g_sdma_multi_fifo_script[] = FSL_SDMA_MULTI_FIFO_SCRIPT;
/*******************************************************************************
 * Code
 ******************************************************************************/

static void i2c_release_bus_delay(void)
{
    uint32_t i = 0;
    for (i = 0; i < I2C_RELEASE_BUS_COUNT; i++)
    {
        __NOP();
    }
}

void BOARD_I2C_ReleaseBus(void)
{
    uint8_t i                    = 0;
    gpio_pin_config_t pin_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};

    IOMUXC_SetPinMux(IOMUXC_I2C3_SCL_GPIO5_IO18, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SCL_GPIO5_IO18, IOMUXC_SW_PAD_CTL_PAD_DSE(3U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                                                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    IOMUXC_SetPinMux(IOMUXC_I2C3_SDA_GPIO5_IO19, 0U);
    IOMUXC_SetPinConfig(IOMUXC_I2C3_SDA_GPIO5_IO19, IOMUXC_SW_PAD_CTL_PAD_DSE(3U) | IOMUXC_SW_PAD_CTL_PAD_FSEL_MASK |
                                                        IOMUXC_SW_PAD_CTL_PAD_HYS_MASK);
    GPIO_PinInit(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, &pin_config);
    GPIO_PinInit(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, &pin_config);

    /* Drive SDA low first to simulate a start */
    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    /* Send 9 pulses on SCL and keep SDA high */
    for (i = 0; i < 9; i++)
    {
        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
        i2c_release_bus_delay();

        GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
        i2c_release_bus_delay();
        i2c_release_bus_delay();
    }

    /* Send stop */
    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 0U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SCL_GPIO, I2C_RELEASE_SCL_PIN, 1U);
    i2c_release_bus_delay();

    GPIO_PinWrite(I2C_RELEASE_SDA_GPIO, I2C_RELEASE_SDA_PIN, 1U);
    i2c_release_bus_delay();
}

void BOARD_MASTER_CLOCK_CONFIG(void)
{
    mclkConfig.mclkOutputEnable = true;
    mclkConfig.mclkHz           = DEMO_AUDIO_MASTER_CLOCK;
    mclkConfig.mclkSourceClkHz  = DEMO_SAI_CLK_FREQ;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}
static void asrcInCallback(ASRC_Type *base, asrc_sdma_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_ASRCQueueIdle)
    {
        isConvertFinished = true;
    }
}

static void startSai(bool start)
{
    if (start)
    {
        SAI_TxEnable(DEMO_SAI, true);
    }
    else
    {
        SAI_TxEnable(DEMO_SAI, false);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t *temp              = (uint8_t *)music;
    sdma_config_t dmaConfig    = {0};
    s_asrcSDMAConfig.watermark = FSL_FEATURE_SAI_FIFO_COUNTn(DEMO_SAI) / 2U;
    sai_transceiver_t config;

    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_I2C_ReleaseBus();
    BOARD_I2C_ConfigurePins();
    BOARD_InitDebugConsole();

    CLOCK_SetRootMux(kCLOCK_RootSai3, kCLOCK_SaiRootmuxAudioPll1); /* Set SAI source to AUDIO PLL1 393216000HZ*/
    CLOCK_SetRootDivider(kCLOCK_RootSai3, 1U, 16U);                /* Set root clock to 393216000HZ / 16 = 24.576MHz */

    /* SAI bit clock source */
    AUDIOMIX_AttachClk(AUDIOMIX, kAUDIOMIX_Attach_SAI3_MCLK1_To_SAI3_ROOT);

    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */

    PRINTF("ASRC peripheral to peripheral SDMA example.\n\r");

    /* Create SDMA handle */
    SDMA_GetDefaultConfig(&dmaConfig);
#ifdef SDMA_FREQ_EQUALS_ARM
    dmaConfig.ratio = kSDMA_ARMClockFreq;
#endif

    SDMA_Init(DEMO_DMA, &dmaConfig);

    SDMA_LoadScript(DEMO_DMA, FSL_SDMA_SCRIPT_CODE_START_ADDR, (void *)g_sdma_multi_fifo_script,
                    FSL_SDMA_SCRIPT_CODE_SIZE);

    /* SAI init */
    SAI_Init(DEMO_SAI);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, kSAI_Channel0Mask);
    config.bitClock.bclkSource = (sai_bclk_source_t)DEMO_SAI_CLOCK_SOURCE;
    SAI_TxSetConfig(DEMO_SAI, &config);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_ASRC_OUT_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();

    /* Enable DMA enable bit */
    SAI_TxEnableDMA(DEMO_SAI, kSAI_FIFORequestDMAEnable, true);

    /* create ASRC sdma handle */
    SDMA_CreateHandle(&s_asrcInSDMAHandle, DEMO_DMA, DEMO_ASRC_IN_SDMA_CHANNEL, &s_asrcInSDMAcontext);
    SDMA_SetChannelPriority(DEMO_DMA, DEMO_ASRC_IN_SDMA_CHANNEL, 4);
    SDMA_CreateHandle(&s_asrcOutSDMAHandle, DEMO_DMA, DEMO_ASRC_OUT_SDMA_CHANNEL, &s_asrcOutSDMAcontext);
    SDMA_SetChannelPriority(DEMO_DMA, DEMO_ASRC_OUT_SDMA_CHANNEL, 3);

    ASRC_Init(DEMO_ASRC);
    ASRC_GetContextDefaultConfig(&s_asrcContextConfig, DEMO_AUDIO_DATA_CHANNEL, DEMO_ASRC_IN_SAMPLE_RATE,
                                 DEMO_ASRC_OUT_SAMPLE_RATE);
    ASRC_TransferInCreateHandleSDMA(DEMO_ASRC, &s_asrcHandle, asrcInCallback, &s_asrcInSDMAHandle,
                                    DEMO_ASRC_IN_SDMA_SOURCE, DEMO_ASRC_CONTEXT, NULL, NULL);
    ASRC_TransferOutCreateHandleSDMA(DEMO_ASRC, &s_asrcHandle, NULL, &s_asrcOutSDMAHandle, DEMO_ASRC_OUT_SDMA_SOURCE,
                                     DEMO_ASRC_CONTEXT, &s_asrcSDMAConfig, NULL);
    if (ASRC_TransferSetContextConfigSDMA(DEMO_ASRC, &s_asrcHandle, &s_asrcContextConfig) != kStatus_Success)
    {
        PRINTF("ASRC context configure failed, please check.\r\n");
        return -1;
    }

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    PRINTF("\r\nPlayback converted audio data\r\n");
    PRINTF("    sample rate : %d\r\n", DEMO_ASRC_OUT_SAMPLE_RATE);
    PRINTF("    channel number: %d\r\n", DEMO_AUDIO_DATA_CHANNEL);
    PRINTF("    frequency: 1kHZ.\r\n\r\n");

    asrcConvertAudio(temp, MUSIC_LEN);

    PRINTF("ASRC peripheral to peripheral SDMA example finished.\n\r ");

    ASRC_Deinit(DEMO_ASRC);

    while (1)
    {
    }
}

static void asrcConvertAudio(void *in, uint32_t dataSize)
{
    asrc_transfer_t xfer;

    xfer.inDataAddr  = in;
    xfer.inDataSize  = dataSize;
    xfer.outDataAddr = (uint32_t *)SAI_TxGetDataRegisterAddress(DEMO_SAI, DEMO_SAI_CHANNEL_NUM);
    xfer.outDataSize =
        ASRC_GetContextOutSampleSize(DEMO_ASRC_IN_SAMPLE_RATE, dataSize, 2U, DEMO_ASRC_OUT_SAMPLE_RATE, 2U);
    ASRC_TransferSDMA(DEMO_ASRC, &s_asrcHandle, &xfer);

    /* Wait until finished */
    while (!isConvertFinished)
    {
    }
}

void SAI_UserIRQHandler(void)
{
    SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    SDK_ISR_EXIT_BARRIER;
}
