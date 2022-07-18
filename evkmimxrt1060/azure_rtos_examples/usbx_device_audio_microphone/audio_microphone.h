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

#endif /* _AUDIO_MICROPHONE_H_ */
