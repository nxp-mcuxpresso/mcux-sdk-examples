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
#include "fsl_sai.h"
#include "music.h"
#include "fsl_codec_common.h"
#include "fsl_asrc.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SAI instance and clock */
#define DEMO_CODEC_WM8960
#define DEMO_SAI         SAI1
#define DEMO_SAI_IRQ     SAI1_IRQn
#define SAI_TxIRQHandler SAI1_IRQHandler

/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (1U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (63U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

#define DEMO_AUDIO_MASTER_CLOCK          24576000U
#define DEMO_ASRC_OUTPUT_SOURCE_CLOCK_HZ (16 * 48000 * 2)
#define DEMO_ASRC_PERIPHERAL_CLOCK       200000000U
#define DEMO_ASRC                        ASRC
#define DEMO_ASRC_CHANNEL_PAIR           kASRC_ChannelPairA

#define DEMO_AUDIO_SAMPLE_RATE_IN  (kSAI_SampleRate48KHz)
#define DEMO_AUDIO_SAMPLE_RATE_OUT (kSAI_SampleRate32KHz)

/* I2C instance and clock */
#define DEMO_I2C LPI2C1

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (DEMO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

/* DMA */
#define EXAMPLE_DMA           DMA0
#define EXAMPLE_CHANNEL       (0U)
#define EXAMPLE_SAI_TX_SOURCE kDmaRequestMuxSai1Tx

#define OVER_SAMPLE_RATE (384U)

/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (2U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH          kSAI_WordWidth16bits
#define DEMO_ASRC_CONVERT_BUFFER_SIZE (100 * 1000U)
#ifndef DEMO_ASRC_INPUT_CLOCK_SOURCE
#define DEMO_ASRC_INPUT_CLOCK_SOURCE kASRC_ClockSourceBitClock0_SAI1_TX
#endif
#ifndef DEMO_ASRC_OUTPUT_CLOCK_SOURCE
#define DEMO_ASRC_OUTPUT_CLOCK_SOURCE kASRC_ClockSourceBitClock0_SAI1_TX
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
sai_handle_t txHandle              = {0};
static volatile bool isSAIFinished = false;
extern codec_config_t boardCodecConfig;
#if (defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)) || \
    (defined(FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) && (FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER))
sai_master_clock_t mclkConfig = {
#if defined(FSL_FEATURE_SAI_HAS_MCR) && (FSL_FEATURE_SAI_HAS_MCR)
    .mclkOutputEnable = true,
#if !(defined(FSL_FEATURE_SAI_HAS_NO_MCR_MICS) && (FSL_FEATURE_SAI_HAS_NO_MCR_MICS))
    .mclkSource = kSAI_MclkSourceSysclk,
#endif
#endif
};
#endif
codec_handle_t codecHandle;
uint8_t asrcConvertBuffer[DEMO_ASRC_CONVERT_BUFFER_SIZE] = {0U};
asrc_channel_pair_config_t s_asrcChannelPairConfig       = {
    .audioDataChannels         = kASRC_ChannelsNumber2,
    .inClockSource             = DEMO_ASRC_INPUT_CLOCK_SOURCE,
    .inSourceClock_Hz          = DEMO_ASRC_OUTPUT_SOURCE_CLOCK_HZ,
    .outClockSource            = DEMO_ASRC_OUTPUT_CLOCK_SOURCE,
    .outSourceClock_Hz         = DEMO_ASRC_OUTPUT_SOURCE_CLOCK_HZ,
    .sampleRateRatio           = kASRC_RatioUseInternalMeasured,
    .inDataWidth               = kASRC_DataWidth16Bit,
    .inDataAlign               = kASRC_DataAlignLSB,
    .outDataWidth              = kASRC_DataWidth16Bit,
    .outDataAlign              = kASRC_DataAlignLSB,
    .outSignExtension          = kASRC_NoSignExtension,
    .outFifoThreshold          = FSL_ASRC_CHANNEL_PAIR_FIFO_DEPTH / 2,
    .inFifoThreshold           = FSL_ASRC_CHANNEL_PAIR_FIFO_DEPTH / 2,
    .bufStallWhenFifoEmptyFull = false,
};
/*******************************************************************************
 * Code
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .i2cConfig        = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 24000000U},
    .route            = kWM8960_RoutePlaybackandRecord,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
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
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR0 |= 1 << 8U;
    }
    else
    {
        IOMUXC_GPR->GPR0 &= ~(1 << 8U);
    }
}

static void sai_callback(I2S_Type *base, sai_handle_t *handle, status_t status, void *userData)
{
    if (status == kStatus_SAI_TxIdle)
    {
        isSAIFinished = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sai_transfer_t xfer;
    asrc_transfer_t asrcConvert;
    sai_transceiver_t config;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    CLOCK_InitAudioPll(&audioPllConfig);
    BOARD_InitDebugConsole();

    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);
    /* audio pll  */
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);
    /* 0SC400M */
    CLOCK_SetRootClockMux(kCLOCK_Root_Asrc, 2);
    /* divider 2 */
    CLOCK_SetRootClockDiv(kCLOCK_Root_Asrc, 2);
    /* audio pll */
    CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 16);

    BOARD_EnableSaiMclkOutput(true);

    /* ASRC baremental driver need non-4byte align access */
    SCB->CCR &= ~SCB_CCR_UNALIGN_TRP_Msk;

    PRINTF("ASRC m2m polling example\n\r");

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandle(DEMO_SAI, &txHandle, sai_callback, NULL);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, kSAI_Channel0Mask);
    SAI_TransferTxSetConfig(DEMO_SAI, &txHandle, &config);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE_IN, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
    /* asrc init */
    ASRC_Init(DEMO_ASRC, DEMO_ASRC_PERIPHERAL_CLOCK);
    ASRC_SetChannelPairConfig(DEMO_ASRC, DEMO_ASRC_CHANNEL_PAIR, &s_asrcChannelPairConfig, DEMO_AUDIO_SAMPLE_RATE_IN,
                              DEMO_AUDIO_SAMPLE_RATE_OUT);

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    PRINTF("Playback raw audio data\r\n");
    PRINTF("    sample rate : %d\r\n", DEMO_AUDIO_SAMPLE_RATE_IN);
    PRINTF("    channel number: %d\r\n", DEMO_AUDIO_DATA_CHANNEL);
    PRINTF("    frequency: 215HZ.\r\n\r\n");

    /*  xfer structure */
    xfer.data     = (uint8_t *)music;
    xfer.dataSize = MUSIC_LEN;
    SAI_TransferSendNonBlocking(DEMO_SAI, &txHandle, &xfer);
    /* Wait until finished */
    while (isSAIFinished != true)
    {
    }
    isSAIFinished = false;

    asrcConvert.inData      = music;
    asrcConvert.inDataSize  = MUSIC_LEN;
    asrcConvert.outData     = asrcConvertBuffer;
    asrcConvert.outDataSize = ASRC_GetOutSamplesSize(DEMO_ASRC, DEMO_ASRC_CHANNEL_PAIR, DEMO_AUDIO_SAMPLE_RATE_IN,
                                                     DEMO_AUDIO_SAMPLE_RATE_OUT, MUSIC_LEN);
    ASRC_TransferBlocking(ASRC, DEMO_ASRC_CHANNEL_PAIR, &asrcConvert);

    PRINTF("Playback converted audio data\r\n");
    PRINTF("    sample rate : %d\r\n", DEMO_AUDIO_SAMPLE_RATE_OUT);
    PRINTF("    channel number: %d\r\n", DEMO_AUDIO_DATA_CHANNEL);
    PRINTF("    frequency: 215HZ.\r\n\r\n");

    /* reset sample rate configuration */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE_OUT, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
    if (CODEC_SetFormat(&codecHandle, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE_OUT, DEMO_AUDIO_BIT_WIDTH) !=
        kStatus_Success)
    {
        assert(false);
    }
    /*  xfer structure */
    xfer.data     = (uint8_t *)asrcConvertBuffer;
    xfer.dataSize = DEMO_ASRC_CONVERT_BUFFER_SIZE;
    SAI_TransferSendNonBlocking(DEMO_SAI, &txHandle, &xfer);
    /* Wait until finished */
    while (isSAIFinished != true)
    {
    }

    PRINTF("ASRC m2m polling example finished\n\r ");
    while (1)
    {
    }
}
