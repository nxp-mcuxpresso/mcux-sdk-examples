/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_MICROPHONE_H_
#define _AUDIO_MICROPHONE_H_

#include "fsl_codec_common.h"

/* Audio data format */
#define AUDIO_IN_SAMPLING_RATE_KHZ  (48)
#define AUDIO_FORMAT_CHANNELS       (0x02U)
#define AUDIO_FORMAT_BITS           (16)
#define AUDIO_FORMAT_SIZE           (0x02)

/* transfer length during 1 ms */
#define BUFFER_SIZE (AUDIO_IN_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)

#define BUFFER_NUM                      16
#define UX_DEMO_BYTES_PER_SAMPLE        4
#define UX_DEMO_BYTES_SYNC_THRESHOLD    64
#define UX_DEMO_MAX_FRAME_SIZE          256

#define DEMO_SAI_INSTANCE_INDEX     (1U)
#define DEMO_SAI_TX_SOURCE          kDmaRequestMuxSai1Tx
#define DEMO_SAI_RX_SOURCE          kDmaRequestMuxSai1Rx
#define DEMO_DMA_INDEX              (0U)
#define DEMO_DMAMUX_INDEX           (0U)
#define DEMO_DMA_TX_CHANNEL         (0U)
#define DEMO_DMA_RX_CHANNEL         (1U)

/* Select Audio/Video PLL (786.48 MHz) as sai clock source */
#define DEMO_SAI_CLOCK_SOURCE_SELECT  (4U)
/* Clock divider for sai clock source */
#define DEMO_SAI_CLOCK_SOURCE_DIVIDER (16U)
/* Get frequency of sai clock */
#define DEMO_SAI_CLK_FREQ             CLOCK_GetRootClockFreq(kCLOCK_Root_Sai1)

/* Select OSC_24M as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT  (1U)

#define ENABLE_IOMUXC_GPR_SAI_MCLK \
    IOMUXC_GPR->GPR0 |= IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK

#define DISABLE_IOMUXC_GPR_SAI_MCLK \
    IOMUXC_GPR->GPR0 &= (~IOMUXC_GPR_GPR0_SAI1_MCLK_DIR_MASK)

/* Audio data Tx buffer */
extern uint8_t audioTxBuff[];

/* Audio data Rx buffer */
extern uint8_t audioRxBuff[];

/*!
 * @brief Initialize board for microphone example.
 */
void audio_microphone_setup(void);

/*!
 * @brief Enable interrupt and start to receive audio data.
 */
void audio_microphone_start(void);

/*!
 * @brief Set rx resync flag.
 */
void audio_set_rx_resync(void);

/*!
 * @brief Ger the received buffer's offset and length.
 *
 * @param offset The data's start address is audioRxBuff + offset.
 * @param length The data's size.
 */

void audio_get_receive_buffer(unsigned long *offset, unsigned long *length);

/*!
 * @brief Clock setting for LPI2C.
 */
static inline void set_clock_lpi2c(void)
{
    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, DEMO_LPI2C_CLOCK_SOURCE_SELECT);
}

/*!
 * @brief Clock setting for SAI.
 */
static inline void set_clock_sai(void)
{
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, DEMO_SAI_CLOCK_SOURCE_SELECT);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, DEMO_SAI_CLOCK_SOURCE_DIVIDER);
}

#endif /* _AUDIO_MICROPHONE_H_ */
