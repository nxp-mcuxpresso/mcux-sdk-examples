/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LE_AUDIO_COMMON_H_
#define __LE_AUDIO_COMMON_H_

#include <stdint.h>

#include <bluetooth/audio/audio.h>
#include <bluetooth/iso.h>

#include "lc3_codec.h"

/* Codec parameters from source. */
typedef struct _lc3_codec_info_t {
	int sample_rate;
	int frame_duration_us;
	int octets_per_frame;
	int blocks_per_sdu;
	uint32_t chan_allocation;
	int channels;
	int bytes_per_channel_frame;
	int samples_per_frame;
} lc3_codec_info_t;

typedef struct _sdu_packet {
	struct bt_iso_recv_info info;
	uint16_t len;
	uint8_t buff[LC3_FRAME_SIZE_MAX];
} sdu_packet_t;

typedef struct _frame_packet {
	struct bt_iso_recv_info info;
	uint8_t flags;
	uint16_t len;
	int32_t buff[480]; /* 480 (samples) */
} frame_packet_t;

int audio_data_stereo_split(int samples, int bits, uint8_t *input, uint8_t *output_left, uint8_t *output_right);
int audio_data_make_stereo(int samples, int bits, uint8_t *input_left, uint8_t *input_right, uint8_t *output);

#define AUDIO_SINK_ROLE_LEFT   1
#define AUDIO_SINK_ROLE_RIGHT  2

void le_audio_sink_role_set(int role);
int  le_audio_sink_role_get(void);

#endif /* __LE_AUDIO_COMMON_H_ */