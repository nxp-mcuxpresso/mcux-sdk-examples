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
#define DEMO_SAI                       SAI1
#define DEMO_SAI_CHANNEL               0
#define DEMO_SAI_CLK_FREQ              CLOCK_GetSaiClkFreq(1U)
#define DEMO_SAI_IRQ                   SAI1_IRQn
#define DEMO_SAITxIRQHandler           SAI1_IRQHandler
#define DEMO_SAI_TX_SYNC_MODE          kSAI_ModeAsync
#define DEMO_SAI_TX_BIT_CLOCK_POLARITY kSAI_PolarityActiveLow
#define DEMO_SAI_MCLK_OUTPUT           true
#define DEMO_SAI_MASTER_SLAVE          kSAI_Slave

#define DEMO_I2C_IRQ LP_FLEXCOMM2_IRQn

#define DEMO_DMA                 DMA0
#define DEMO_DMA_TX_IRQ          EDMA_0_CH0_IRQn
#define DEMO_TX_EDMA_CHANNEL     0
#define DEMO_SAI_TX_EDMA_CHANNEL kDma0RequestMuxSai1Tx
#define DEMO_CHANNEL_NUM         2

#define SUPPORT_48KHZ
/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
