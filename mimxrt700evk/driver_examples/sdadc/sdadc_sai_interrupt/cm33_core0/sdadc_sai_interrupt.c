/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sai.h"
#include "fsl_sdadc.h"
#include "fsl_codec_common.h"
#include "fsl_debug_console.h"

#include "fsl_wm8962.h"
#include "fsl_codec_adapter.h"
#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SDADC */
#define DEMO_SDADC_BASE             SDADC
#define DEMO_SDADC_IRQn             SDADC_IRQn
#define DEMO_SDADC_IRQ_HANDLER_FUNC SDADC_IRQHandler

/* SAI */
#define DEMO_SAI_BASE           SAI0
#define DEMO_SAI_CHANNEL        0U
#define DEMO_AUDIO_DATA_CHANNEL 2U
#define DEMO_SOUND_MODE         kSAI_Stereo
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth32bits
#define DEMO_AUDIO_SAMPLE_RATE  kSAI_SampleRate48KHz
#define DEMO_AUDIO_MASTER_CLOCK CLOCK_GetSaiClkFreq()

/* LPI2C and Codec */
#define DEMO_I2C_CLK_FREQ     24000000U
#define DEMO_CODEC_VOLUME     100U
#define DEMO_SAI_CLOCK_SOURCE kSAI_BclkSourceMclkDiv
#define DEMO_SAI_MASTER_SLAVE kSAI_Master
#define BOARD_SAI_RXCONFIG(config, mode)
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync

#define DEMO_BUFFER_SIZE   (SDADC_FIFO_DEPTH * SDADC_FIFO_WIDTH * DEMO_AUDIO_DATA_CHANNEL)
#define DEMO_BUFFER_NUMBER 64U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_MasterClockConfig(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8962_config_t wm8962Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = DEMO_I2C_CLK_FREQ},
    .route =
        {
            .enableLoopBack            = false,
            .leftInputPGASource        = kWM8962_InputPGASourceInput1,
            .leftInputMixerSource      = kWM8962_InputMixerSourceInputPGA,
            .rightInputPGASource       = kWM8962_InputPGASourceInput3,
            .rightInputMixerSource     = kWM8962_InputMixerSourceInputPGA,
            .leftHeadphoneMixerSource  = kWM8962_OutputMixerDisabled,
            .leftHeadphonePGASource    = kWM8962_OutputPGASourceDAC,
            .rightHeadphoneMixerSource = kWM8962_OutputMixerDisabled,
            .rightHeadphonePGASource   = kWM8962_OutputPGASourceDAC,
        },
    .slaveAddress = WM8962_I2C_ADDR,
    .bus          = kWM8962_BusI2S,
    .format =
        {
            .mclk_HZ    = 24576000U,
            .sampleRate = DEMO_AUDIO_SAMPLE_RATE,
            .bitWidth   = DEMO_AUDIO_BIT_WIDTH,
        },
    .masterSlave = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8962, .codecDevConfig = &wm8962Config};

sai_master_clock_t mclkConfig;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t txBuff[DEMO_BUFFER_SIZE * DEMO_BUFFER_NUMBER], 4);
codec_handle_t codecHandle;
sai_handle_t saiTxHandle = {0};
extern codec_config_t boardCodecConfig;
static volatile uint32_t readIndex  = 0U;
static volatile uint32_t writeIndex = 0U;

sdadc_channel_config_t channelConfig[] = {
    {
        .mode   = kSDADC_SingleEnd_Mode,
        .number = kSDADC_Channel0,
        .type   = kSDADC_Channel_PSide_Type,
        .volume =
            {
                .pSideVolume = 20U,
            },
        .samplerate =
            {
                .pSideSampleRate = kSDADC_DecimatorSampleRate48kHz,
            },
        .watermark =
            {
                .pSideWatermark = 0U,
            },
        .enableDacCompensation = true,
        .enableDcFilter        = true,
        .enableDCLoop          = false,
        .enablePolarityInvert  = false,
    },
    {
        .mode   = kSDADC_SingleEnd_Mode,
        .number = kSDADC_Channel1,
        .type   = kSDADC_Channel_PSide_Type,
        .volume =
            {
                .pSideVolume = 20U,
            },
        .samplerate =
            {
                .pSideSampleRate = kSDADC_DecimatorSampleRate48kHz,
            },
        .watermark =
            {
                .pSideWatermark = 0U,
            },
        .enableDacCompensation = true,
        .enableDcFilter        = true,
        .enableDCLoop          = false,
        .enablePolarityInvert  = false,
    },
};

static sdadc_channel_group group[] = {
    [0U] =
        {
            .number = kSDADC_Channel0,
            .type   = kSDADC_Channel_PSide_Type,
        },
    [1U] =
        {
            .number = kSDADC_Channel1,
            .type   = kSDADC_Channel_PSide_Type,
        },
};

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_MasterClockConfig(void)
{
    mclkConfig.mclkOutputEnable = true;
    mclkConfig.mclkHz           = 24576000U;
    mclkConfig.mclkSourceClkHz  = 24576000U;
    SAI_SetMasterClockConfig(DEMO_SAI_BASE, &mclkConfig);
}

static void BOARD_ConfigureAudioPll(void)
{
    const clock_audio_pll_config_t audioPllConfig = {.audio_pll_src  = kCLOCK_AudioPllOscClk,
                                                     .numerator      = 66,
                                                     .denominator    = 125,
                                                     .audio_pll_mult = kCLOCK_AudioPllMult22,
                                                     .enableVcoOut   = true};

    /* Configure Audio PLL clock source. */
    CLOCK_InitAudioPll(&audioPllConfig);  /* 540.672MHz */
    CLOCK_InitAudioPfd(kCLOCK_Pfd3, 18U); /* 540.672MHZ */
    CLOCK_EnableAudioPllPfdClkForDomain(kCLOCK_Pfd3, kCLOCK_AllDomainEnable);
}

/*!
 * @brief SDADC interrupt handler.
 */
void DEMO_SDADC_IRQ_HANDLER_FUNC(void)
{
    if (SDADC_CheckGlobalFifoInterrupted(DEMO_SDADC_BASE))
    {
        if (((readIndex >= writeIndex) || (readIndex <= (writeIndex - 1U))))
        {
            SDADC_CopyConvChannelFifoToBuffer(DEMO_SDADC_BASE, &(group[0U]), sizeof(group) / sizeof(*group),
                                              &txBuff[readIndex * DEMO_BUFFER_SIZE]);
            readIndex++;

            if (readIndex == DEMO_BUFFER_NUMBER)
            {
                readIndex = 0U;
            }
        }
        SDADC_ClearGlobalFifoIntStatusFlag(DEMO_SDADC_BASE);
    }
}

/*!
 * @brief SDADC initialize function.
 */
static void DEMO_SDADC_INIT(void)
{
    sdadc_config_t config;

    /* Do sdadc configuration. */
    SDADC_GetDefaultConfig(&config);
    config.channelCount  = sizeof(group) / sizeof(*group);
    config.channelConfig = &(channelConfig[0U]);
    SDADC_Init(DEMO_SDADC_BASE, &config);

    SDADC_ControlFifoIntEnable(
        DEMO_SDADC_BASE, (kSDADC_Channel0_PSide_FifoFullIntEnable | kSDADC_Channel1_PSide_FifoFullIntEnable), true);
    SDADC_ControlGlobalFifoIntEnable(DEMO_SDADC_BASE, true);

    /* Do sdadc power-up. */
    SDADC_DoInitPowerUp(DEMO_SDADC_BASE, &config, SystemCoreClock);

    EnableIRQ(DEMO_SDADC_IRQn);
}

/*!
 * @brief SAI and codec initialize function.
 */
static void DEMO_SAI_INIT(void)
{
    sai_transceiver_t config;

    /* Do SAI initialization. */
    SAI_Init(DEMO_SAI_BASE);
    SAI_TransferTxCreateHandle(DEMO_SAI_BASE, &saiTxHandle, NULL, NULL);

    /* Do SAI I2S mode configuration. */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, DEMO_SOUND_MODE, 1U << DEMO_SAI_CHANNEL);
    config.serialData.dataFirstBitShifted = (DEMO_AUDIO_BIT_WIDTH - 8U);
    config.bitClock.bclkSource            = DEMO_SAI_CLOCK_SOURCE;
#if defined BOARD_SAI_RXCONFIG
    config.syncMode = DEMO_SAI_TX_SYNC_MODE;
#endif
    config.masterSlave = DEMO_SAI_MASTER_SLAVE;
    SAI_TransferTxSetConfig(DEMO_SAI_BASE, &saiTxHandle, &config);

    /* Set bit clock divider. */
    SAI_TxSetBitClockRate(DEMO_SAI_BASE, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
#if defined BOARD_SAI_RXCONFIG
    BOARD_SAI_RXCONFIG(&config, DEMO_SAI_RX_SYNC_MODE);
#endif

    /* Set master clock. */
    BOARD_MasterClockConfig();

    /* Do codec initialization. */
#if defined DEMO_BOARD_CODEC_INIT
    DEMO_BOARD_CODEC_INIT();
#else
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }
#endif
}

/*!
 * @brief Main function.
 */
int main(void)
{
    sai_transfer_t transfer;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_ConfigureAudioPll();
    BOARD_InitDebugConsole();

    /* SDADC clock setting. */
    CLOCK_AttachClk(kAUDIO_PLL_to_SDADC);
    CLOCK_SetClkDiv(kCLOCK_DivSdadcClk, 22U);

    /* LPI2C clock setting. */
    CLOCK_AttachClk(kFCCLK0_to_FLEXCOMM2);

    /* SAI clock setting. */
    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_AUDIO_VDD2);
    CLOCK_AttachClk(kAUDIO_VDD2_to_SAI012);
    CLOCK_SetClkDiv(kCLOCK_DivSai012Clk, 22U);

    SYSCON0->SAI0_MCLK_CTRL |= SYSCON0_SAI0_MCLK_CTRL_SAIMCLKDIR_MASK;

    PRINTF("\r\n SDADC SAI Interrupt Example.");

    memset(txBuff, 0U, sizeof(txBuff));

    DEMO_SAI_INIT();
    DEMO_SDADC_INIT();

    while (1)
    {
        if (readIndex != writeIndex)
        {
            transfer.data     = (uint8_t *)(&txBuff[writeIndex * DEMO_BUFFER_SIZE]);
            transfer.dataSize = DEMO_BUFFER_SIZE;

            if (SAI_TransferSendNonBlocking(DEMO_SAI_BASE, &saiTxHandle, &transfer) != kStatus_SAI_QueueFull)
            {
                writeIndex++;

                if (writeIndex == DEMO_BUFFER_NUMBER)
                {
                    writeIndex = 0U;
                }
            }
        }
    }
}
