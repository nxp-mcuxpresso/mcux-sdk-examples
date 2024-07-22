/*
 * Copyright 2021 NXP
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
#define DEMO_I2C                        (I2C4)
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY (24576000)
#define DEMO_I2S_TX                     (I2S7)
#define DEMO_I2S_RX                     (I2S6)
#define DEMO_DMA                        (DMA0)
#define DEMO_I2S_TX_CHANNEL             (19)
#define DEMO_I2S_RX_CHANNEL             (16)
#define DEMO_I2S_RX_MODE                (kI2S_MasterSlaveNormalSlave)
#define DEMO_I2S_TX_MODE                (kI2S_MasterSlaveNormalMaster)
#define DEMO_CHANNEL_NUM                (2)
#define DEMO_MIC_CHANNEL_NUM            (2)
#define DEMO_MIC_FRAME_SIZE             (30)
#define DEMO_AUDIO_BIT_WIDTH            (16)
#define DEMO_AUDIO_SAMPLE_RATE          (16000)

#define DEMO_VOLUME (75)
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
