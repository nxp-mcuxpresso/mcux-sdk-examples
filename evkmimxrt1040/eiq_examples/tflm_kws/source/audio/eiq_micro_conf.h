/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_MICRO_CONF_H_
#define _EIQ_MICRO_CONF_H_

#include "fsl_sai_edma.h"
#include "fsl_dmamux.h"
#include "fsl_codec_common.h"
#ifdef DEMO_CODEC_WM8962
#include "fsl_wm8962.h"
#else
#include "fsl_wm8960.h"
#endif
#include "fsl_codec_adapter.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_CODEC_VOLUME 100

/* SAI instance and clock */
#define DEMO_SAI SAI1
#define DEMO_SAI_CHANNEL (0)
#define DEMO_SAI_BITWIDTH (kSAI_WordWidth16bits)
#define DEMO_SAI_IRQ SAI1_IRQn

/* DMA */
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 )
#define DEMO_DMA             DMA0
#define DEMO_DMAMUX          DMAMUX0
#elif defined( CPU_MIMXRT1176DVMAA_cm4 ) || defined( CPU_MIMXRT1166DVM6A_cm4 )
#define DEMO_DMA             DMA1
#define DEMO_DMAMUX          DMAMUX1
#else
#define DEMO_DMA             DMA0
#define DEMO_DMAMUX          DMAMUX
#endif

#define DEMO_TX_EDMA_CHANNEL (0U)
#define DEMO_RX_EDMA_CHANNEL (1U)
#define DEMO_SAI_TX_SOURCE   kDmaRequestMuxSai1Tx
#define DEMO_SAI_RX_SOURCE   kDmaRequestMuxSai1Rx

#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 ) || \
    defined( CPU_MIMXRT1176DVMAA_cm4 ) || defined( CPU_MIMXRT1166DVM6A_cm4 )
/* Select Audio/Video PLL (393.24 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (4U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (0U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (31U)
#else
/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (0U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (63U)
#endif

/* Get frequency of sai1 clock */
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 )
#define DEMO_SAI_CLK_FREQ \
    (CLOCK_GetFreq(kCLOCK_AudioPll) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#elif defined( CPU_MIMXRT1176DVMAA_cm4 ) || defined( CPU_MIMXRT1166DVM6A_cm4 )
#define DEMO_SAI_CLK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Sai1)
#else
#define DEMO_SAI_CLK_FREQ \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))
#endif

/* I2C instance and clock */

/* Get frequency of sai1 clock */
#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 )
#define DEMO_I2C LPI2C5
#else
#define DEMO_I2C LPI2C1
#endif

#if defined( CPU_MIMXRT1176DVMAA_cm7 ) || defined( CPU_MIMXRT1166DVM6A_cm7 )
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (1U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (1U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Lpi2c5)
#else
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (DEMO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#endif

#define OVER_SAMPLE_RATE (384U)
/* 16Khz */
#define AUDIO_SAMPLES (16U)
#define AUDIO_SAMPLE_SIZE (2U)

/* 40 ms @ 16 kHz -> 640 (640 * 2 bytes = 1280) */
#define BUFFER_SIZE (640 * 2)
/* 50 * 40 ms = 2000 ms */
#define BUFFER_NUM (50)

/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate16KHz)
/* demo audio master clock */
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
#define DEMO_AUDIO_MASTER_CLOCK OVER_SAMPLE_RATE * DEMO_AUDIO_SAMPLE_RATE
#else
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#endif
/* demo audio data channel */
#ifndef DEMO_CODEC_WM8962
#define DEMO_AUDIO_DATA_CHANNEL (1U)
#else
#define DEMO_AUDIO_DATA_CHANNEL (2U)
#endif
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH kSAI_WordWidth16bits

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Clears Rx interrupt Flags.
 */
static inline void BOARD_ClearRxInterruptFlags(void)
{
    /* Clears RCSR FRIE and FEIE interrupt flags. */
    DEMO_SAI->RCSR &= ~(I2S_RCSR_FRIE_MASK | I2S_RCSR_FEIE_MASK);
}

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _EIQ_MICRO_CONF_H_ */
