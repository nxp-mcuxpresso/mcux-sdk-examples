/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_DEFINITIONS_H_
#define _APP_DEFINITIONS_H_

/*${header:start}*/
#include "fsl_wm8962.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/* SAI instance and clock */
#define DEMO_CODEC_WM8962             1
#define DEMO_PDM                      PDM
#define DEMO_SAI                      SAI1
#define DEMO_SAI_MASTER_SLAVE         kSAI_Master
#define DEMO_SAI_CLK_FREQ             24576000
#define DEMO_SAI_CLOCK_SOURCE         (kSAI_BclkSourceMclkDiv)
#define DEMO_PDM_CLK_FREQ             24576000
#define DEMO_PDM_FIFO_WATERMARK       (4)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_PDM_SAMPLE_CLOCK_RATE    (6144000U) /* 6.144MHZ */

#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth32bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ

/* DMA */
#define DEMO_EDMA          DMA0
#define DEMO_DMAMUX        DMAMUX0
#define DEMO_RX_CHANNEL    (0)
#define DEMO_TX_CHANNEL    (1)
#define DEMO_SAI_TX_SOURCE kDmaRequestMuxSai1Tx
#define DEMO_SAI_RX_SOURCE kDmaRequestMuxPdm

/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (0U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (63U)

#define DEMO_MIC_CHANNEL_NUM 2
#define DEMO_MIC_SAMPLE_SIZE 32
#define DEMO_MIC_SAMPLE_RATE 16000
#define DEMO_MIC_FRAME_SIZE  30
#define DEMO_CODEC_CHANNEL   kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight

#define DEMO_VOLUME (60)
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
