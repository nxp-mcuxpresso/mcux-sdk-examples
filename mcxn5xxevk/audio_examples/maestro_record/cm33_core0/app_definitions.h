/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_DEFINITIONS_H_
#define _APP_DEFINITIONS_H_

/*${header:start}*/
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/

#define DEMO_SAI              SAI1
#define DEMO_SAI_CLK_FREQ     CLOCK_GetSaiClkFreq(1U)
#define DEMO_SAI_CHANNEL      0
#define DEMO_SAI_MASTER_SLAVE kSAI_Slave
#define DEMO_SAI_CLOCK_SOURCE (kSAI_BclkSourceMclkDiv)

#define DEMO_PDM                      PDM
#define DEMO_PDM_CLK_FREQ             CLOCK_GetMicfilClkFreq()
#define DEMO_PDM_FIFO_WATERMARK       (FSL_FEATURE_PDM_FIFO_DEPTH / 2)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_PDM_CHANNEL_GAIN         kPDM_DfOutputGain4

#define DEMO_DMA              DMA0
#define DEMO_PDM_EDMA_CHANNEL 0
#define DEMO_SAI_EDMA_CHANNEL 1
#define DEMO_PDM_EDMA_SOURCE  kDma0RequestMuxMicfil0FifoRequest
#define DEMO_SAI_EDMA_SOURCE  kDma0RequestMuxSai1Tx

/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate16KHz)
#define DEMO_MIC_CHANNEL_NUM   1U
#define DEMO_MIC_FRAME_SIZE    30U
/* demo audio master clock */
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (1U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH kSAI_WordWidth32bits
#define DEMO_VOLUME          100
/*${macro:end}*/

#endif /* _APP_H_ */
