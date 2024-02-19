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
#define DEMO_DMA              DMA0
#define DEMO_DMA_RX_CHANNEL   (1U)
#define DEMO_DMA_RX_IRQ       EDMA_0_CH1_IRQn
#define DEMO_SAI_RX_SOURCE    kDma0RequestMuxSai1Rx
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI              SAI1
#define DEMO_SAI_CHANNEL      0
#define DEMO_SAI_IRQ          SAI1_IRQn
#define DEMO_SAI_MASTER_SLAVE kSAI_Slave
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 50U
#endif

#define DEMO_I2C_IRQ LP_FLEXCOMM2_IRQn

#define DEMO_CHANNEL_NUM 2
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ CLOCK_GetSaiClkFreq(1U)
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
