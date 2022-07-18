/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _AUDIO_SPEAKER_H_
#define _AUDIO_SPEAKER_H_

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

/*!
 * @brief Initialize board for speaker example.
 */
void audio_speaker_board_setup(void);

/*!
 * @brief Enable interrupt and start to transmit audio data.
 */
void audio_speaker_start(void);

/*!
 * @brief Clean tx buffer.
 */
void audio_clean_tx_buff(void);

/*!
 * @brief Set Tx resync flag.
 */
void audio_set_tx_resync(void);

/*!
 * @brief Get the count of the transmitted data.\
 *
 * @return The transmitted data count.
 */
size_t audio_get_tx_cout(void);

/*!
 * @brief Enable interrupt and start to transmit audio data.\
 *
 * @param frame_buffer The frame buffer to send.
 * @param frame_length The buffer size to send.
 */
void audio_send(unsigned char *frame_buffer, unsigned long frame_length);

#endif /* _AUDIO_SPEAKER_H_ */
