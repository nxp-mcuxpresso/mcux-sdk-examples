/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pdm.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                      PDM
#define DEMO_PDM_CLK_FREQ             24576000U
#define DEMO_PDM_FIFO_WATERMARK       (FSL_FEATURE_PDM_FIFO_DEPTH - 1U)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_PDM_HWVAD_SIGNAL_GAIN    (6U)
#define DEMO_AUDIO_SAMPLE_RATE        (16000)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_hwvadFlag       = false;
static volatile uint32_t s_readIndex   = 0U;
static volatile uint32_t s_detectTimes = 50U;
static const pdm_config_t pdmConfig    = {
#if defined(FSL_FEATURE_PDM_HAS_DECIMATION_FILTER_BYPASS) && FSL_FEATURE_PDM_HAS_DECIMATION_FILTER_BYPASS
    .enableFilterBypass = false,
#endif
    .enableDoze        = false,
    .fifoWatermark     = DEMO_PDM_FIFO_WATERMARK,
    .qualityMode       = DEMO_PDM_QUALITY_MODE,
    .cicOverSampleRate = DEMO_PDM_CIC_OVERSAMPLE_RATE,
};

static const pdm_channel_config_t channelConfig = {
#if (defined(FSL_FEATURE_PDM_HAS_DC_OUT_CTRL) && (FSL_FEATURE_PDM_HAS_DC_OUT_CTRL))
    .outputCutOffFreq = kPDM_DcRemoverCutOff40Hz,
#else
    .cutOffFreq = kPDM_DcRemoverCutOff152Hz,
#endif
#ifdef DEMO_PDM_CHANNEL_GAIN
    .gain = DEMO_PDM_CHANNEL_GAIN,
#else
    .gain       = kPDM_DfOutputGain7,
#endif
};

static const pdm_hwvad_config_t hwvadConfig = {
    .channel           = DEMO_PDM_ENABLE_CHANNEL_LEFT,
    .initializeTime    = 10U,
    .cicOverSampleRate = 0U,
    .inputGain         = 0U,
    .frameTime         = 10U,
    .cutOffFreq        = kPDM_HwvadHpfBypassed,
    .enableFrameEnergy = false,
    .enablePreFilter   = true,
};

static const pdm_hwvad_noise_filter_t noiseFilterConfig = {
    .enableAutoNoiseFilter = false,
    .enableNoiseMin        = true,
    .enableNoiseDecimation = true,
    .noiseFilterAdjustment = 0U,
    .noiseGain             = 7U,
    .enableNoiseDetectOR   = true,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 768/1000)  / 2
 *                              = 393.216MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,   /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,    /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 768,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 1000, /* 30 bit denominator of fractional loop divider */
};

void PDM_HWVAD_EVENT_IRQHandler(void)
{
    if (PDM_GetHwvadInterruptStatusFlags(DEMO_PDM) & kPDM_HwvadStatusVoiceDetectFlag)
    {
        s_hwvadFlag = true;
        PDM_ClearHwvadInterruptStatusFlags(DEMO_PDM, kPDM_HwvadStatusVoiceDetectFlag);
    }
#if (defined FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ && FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ)
    else
    {
        PDM_ClearHwvadInterruptStatusFlags(DEMO_PDM, kPDM_HwvadStatusInputSaturation);
    }
#endif
}

#if !(defined FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ && FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ)
void PDM_HWVAD_ERROR_IRQHandler(void)
{
    PDM_ClearHwvadInterruptStatusFlags(DEMO_PDM, kPDM_HwvadStatusInputSaturation);
}
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClockMux(kCLOCK_Root_Bus_Lpsr, 0);
    CLOCK_InitAudioPll(&audioPllConfig);

    /* 24.576m mic root clock */
    CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 16);

    PRINTF("PDM hwvad example started!\n\r");

    /* Set up pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_SetChannelConfig(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    PDM_SetChannelConfig(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_RIGHT, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
        return -1;
    }
    /* envelope based mode */
    PDM_SetHwvadInEnvelopeBasedMode(DEMO_PDM, &hwvadConfig, &noiseFilterConfig, NULL, DEMO_PDM_HWVAD_SIGNAL_GAIN);
    PDM_EnableHwvadInterrupts(DEMO_PDM, kPDM_HwvadErrorInterruptEnable | kPDM_HwvadInterruptEnable);
    NVIC_ClearPendingIRQ(PDM_HWVAD_EVENT_IRQn);
#if !(defined FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ && FSL_FEATURE_PDM_HAS_NO_INDEPENDENT_ERROR_IRQ)
    NVIC_ClearPendingIRQ(PDM_HWVAD_ERROR_IRQn);
    EnableIRQ(PDM_HWVAD_ERROR_IRQn);
#endif
    EnableIRQ(PDM_HWVAD_EVENT_IRQn);

    while (s_detectTimes)
    {
        /* wait voice detect */
        while (!s_hwvadFlag)
        {
        }
        s_hwvadFlag = false;
        s_detectTimes--;
        PRINTF("\r\nVoice detected\r\n");
    }

    PDM_Deinit(DEMO_PDM);

    PRINTF("\n\rPDM hwvad example finished!\n\r ");
    while (1)
    {
    }
}
