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
#include "fsl_common.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
/* SAI instance and clock */
#define DEMO_SAI           SAI1
#define DEMO_SAI_CHANNEL   (0)
#define DEMO_SAI_BITWIDTH  (kSAI_WordWidth32bits)
#define DEMO_SAI_IRQ       SAI1_IRQn
#define SAI_UserIRQHandler SAI1_IRQHandler
#define DEMO_CHANNEL_NUM   8
#define DEMO_VOLUME        100

/* IRQ */
#define DEMO_SAI_TX_IRQ SAI1_IRQn

/* DMA */
#define DEMO_DMA           DMA0
#define DEMO_DMAMUX        DMAMUX
#define DEMO_TX_CHANNEL    (0U)
#define DEMO_SAI_TX_SOURCE kDmaRequestMuxSai1Tx

#define DEMO_SAMPLE_RATE 96000
#define DEMO_BIT_WIDTH   32
#define DEMO_BYTE_WIDTH  4
#define DEMO_CHANNEL_NUM 8

#define DEMO_CS42448_I2C_INSTANCE      3
#define DEMO_CODEC_POWER_GPIO          GPIO1
#define DEMO_CODEC_POWER_GPIO_PIN      0
#define DEMO_CODEC_RESET_GPIO          GPIO1
#define DEMO_CODEC_RESET_GPIO_PIN      2
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (11U)
#define DEMO_SAI_MASTER_SLAVE          kSAI_Master

/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (3U)
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

/*!
 * @brief Function for changing codec settings according to selected parameters.
 *
 * @param[in] nchannel Number of chnnels.
 */
int BOARD_CodecChangeSettings(uint8_t nchannel);

void BORAD_CodecReset(bool state);

#endif /* _APP_DEFINITIONS_H_ */
