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

#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_I2S_TX                     (I2S1)
#define DEMO_I2S_CLOCK_DIVIDER          (DEMO_I2S_MASTER_CLOCK_FREQUENCY / DEMO_AUDIO_SAMPLE_RATE / DEMO_AUDIO_BIT_WIDTH / 2U)
#define DEMO_I2S_TX_CHANNEL             (3)
#define DEMO_I2S_TX_MODE                kI2S_MasterSlaveNormalMaster

#define DEMO_DMA                   (DMA0)
#define DEMO_DMIC_RX_CHANNEL       16U
#define DEMO_DMIC_RX_CHANNEL_1     18U
#define DEMO_DMIC_CHANNEL          kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_1        kDMIC_Channel2
#define DEMO_DMIC_CHANNEL_ENABLE   DMIC_CHANEN_EN_CH0(1)
#define DEMO_DMIC_CHANNEL_1_ENABLE DMIC_CHANEN_EN_CH2(1)

#define DEMO_AUDIO_BIT_WIDTH   16U
#define DEMO_AUDIO_BYTE_WIDTH  2U
#define DEMO_AUDIO_SAMPLE_RATE 16000U
#define DEMO_MIC_FRAME_SIZE    30U
#define DEMO_AUDIO_PROTOCOL    kCODEC_BusI2S
#define DEMO_MIC_CHANNEL_NUM   2U

#define FIFO_DEPTH   (15U)
#define DEMO_VOLUME  (70)
#define ALL_CHANNELS (0xFF)
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
