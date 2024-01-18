/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _APP_DEFINITIONS_H_
#define _APP_DEFINITIONS_H_

/*${header:start}*/
#include "fsl_common.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/* SAI instance and clock */
#define DEMO_CODEC_WM8962  1
#define DEMO_SAI           SAI1
#define DEMO_SAI_CHANNEL   (0)
#define DEMO_SAI_BITWIDTH  (kSAI_WordWidth16bits)
#define DEMO_SAI_IRQ       SAI1_IRQn
#define SAI_UserIRQHandler SAI1_IRQHandler
#define DEMO_CHANNEL_NUM   2
#define DEMO_VOLUME        75

/* IRQ */
#define DEMO_DMA_RX_IRQ DMA1_DMA17_IRQn
#define DEMO_I2C_IRQ    LPI2C1_IRQn

/* DMA */
#define DEMO_DMA            DMA0
#define DEMO_DMAMUX         DMAMUX
#define DEMO_DMA_RX_CHANNEL (1U)
#define DEMO_SAI_RX_SOURCE  kDmaRequestMuxSai1Rx

#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeSync

#define DEMO_WM8962_I2C_INSTANCE       1
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (3U)
#define DEMO_SAI_MASTER_SLAVE          kSAI_Master

/* Select Audio/Video PLL (180.6 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (7U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (DEMO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

/*${macro:end}*/

#endif /* _APP_DEFINITIONS_H_ */
