/*
 * Copyright 2017 NXP
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
#include "fsl_wm8960.h"
#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_codec_adapter.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SAI              (I2S3)
#define DEMO_SAI_CLK_FREQ     12288000
#define DEMO_SAI_MASTER_SLAVE kSAI_Slave

#define I2C_RELEASE_SDA_GPIO  GPIO5
#define I2C_RELEASE_SDA_PIN   19U
#define I2C_RELEASE_SCL_GPIO  GPIO5
#define I2C_RELEASE_SCL_PIN   18U
#define I2C_RELEASE_BUS_COUNT 100U

#define DEMO_IRQn       I2S3_IRQn
#define EXAMPLE_DMA     SDMAARM3
#define EXAMPLE_CHANNEL (1)
#define SAI_TX_SOURCE   (5)
/*set Bclk source to Mclk clock*/
#define DEMO_SAI_CLOCK_SOURCE   (1U)
#define SDMA_FREQ_EQUALS_ARM    (1U)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#define SAI_UserIRQHandler      I2S3_IRQHandler

#define OVER_SAMPLE_RATE (384U)
/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate16KHz)
/* demo audio master clock */
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
#define DEMO_AUDIO_MASTER_CLOCK OVER_SAMPLE_RATE *DEMO_AUDIO_SAMPLE_RATE
#else
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#endif
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (2U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH kSAI_WordWidth16bits
#ifndef DEMO_CODEC_INIT_DELAY_MS
#define DEMO_CODEC_INIT_DELAY_MS (1000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
void BOARD_MASTER_CLOCK_CONFIG(void);
static void callback(I2S_Type *base, sai_sdma_handle_t *handle, status_t status, void *userData);

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
    .format           = {.mclk_HZ    = 12000000U,
               .sampleRate = kWM8960_AudioSampleRate16KHz,
               .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = true,
    .masterClock =
        {
            .sysclkSource = kWM8960_SysClkSourceInternalPLL,
            .sysclkFreq   = DEMO_AUDIO_MASTER_CLOCK,
        },
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};
sai_master_clock_t mclkConfig;

AT_NONCACHEABLE_SECTION_ALIGN(sai_sdma_handle_t txHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_handle_t dmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(sdma_context_data_t context, 4);
volatile bool isFinished = false;
extern codec_config_t boardCodecConfig;
codec_handle_t codecHandle;

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
    mclkConfig.mclkHz           = 12000000U;
    mclkConfig.mclkSourceClkHz  = 12000000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}
static void callback(I2S_Type *base, sai_sdma_handle_t *handle, status_t status, void *userData)
{
    isFinished = true;
}

void DelayMS(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sai_transfer_t xfer;
    sdma_config_t dmaConfig = {0};
    uint32_t temp           = 0;
    sai_transceiver_t saiConfig;
    status_t error = kStatus_Success;

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

    CLOCK_SetRootMux(kCLOCK_RootSai3, kCLOCK_SaiRootmuxOsc24M);      /* Set SAI source to 24000000HZ*/
    CLOCK_SetRootDivider(kCLOCK_RootSai3, 1U, 2U);                   /* Set root clock to 24000000HZ / 2 = 12MHz */
    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */

    /* SAI bit clock source */
    AUDIOMIX_AttachClk(AUDIOMIX, kAUDIOMIX_Attach_SAI3_MCLK1_To_SAI3_ROOT);

    PRINTF("SAI SDMA example started!\n\r");
    /* Create SDMA handle */
    SDMA_GetDefaultConfig(&dmaConfig);
#ifdef SDMA_FREQ_EQUALS_ARM
    dmaConfig.ratio = kSDMA_ARMClockFreq;
#endif

    SDMA_Init(EXAMPLE_DMA, &dmaConfig);
    SDMA_CreateHandle(&dmaHandle, EXAMPLE_DMA, EXAMPLE_CHANNEL, &context);
    SDMA_SetChannelPriority(EXAMPLE_DMA, EXAMPLE_CHANNEL, 2);

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleSDMA(DEMO_SAI, &txHandle, callback, NULL, &dmaHandle, SAI_TX_SOURCE);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, kSAI_Channel0Mask);
    saiConfig.bitClock.bclkSource = (sai_bclk_source_t)DEMO_SAI_CLOCK_SOURCE;
    saiConfig.masterSlave         = DEMO_SAI_MASTER_SLAVE;
    SAI_TransferTxSetConfigSDMA(DEMO_SAI, &txHandle, &saiConfig);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    /* delay for codec output stable */
    DelayMS(DEMO_CODEC_INIT_DELAY_MS);

    /* Enable SAI interrupt to clear the FIFO error */
    EnableIRQ(DEMO_IRQn);
    SAI_TxEnableInterrupts(DEMO_SAI, kSAI_FIFOErrorInterruptEnable);

    /*  xfer structure */
    temp          = (uint32_t)music;
    xfer.data     = (uint8_t *)temp;
    xfer.dataSize = MUSIC_LEN;
    SAI_TransferSendSDMA(DEMO_SAI, &txHandle, &xfer);

    /* Wait until finished */
    while (isFinished != true)
    {
    }

    /* Once transfer finish, disable SAI instance. */
    SAI_TransferAbortSendSDMA(DEMO_SAI, &txHandle);
    SAI_Deinit(DEMO_SAI);

    error = CODEC_Deinit(&codecHandle);
    if ((error != kStatus_CODEC_NotSupport) && (error != kStatus_Success))
    {
        assert(false);
    }

    PRINTF("\n\r SAI SDMA example finished!\n\r ");
    while (1)
    {
    }
}

void SAI_UserIRQHandler(void)
{
    SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    SDK_ISR_EXIT_BARRIER;
}
