/*
 * Copyright 2023 NXP
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

#define LC3_CODEC_DATA_CONFIG(_data, _type, len, pdata) \
	_data.data.type = _type; \
	_data.data.data_len = len; \
	_data.data.data = _data.value;	\
	memcpy(_data.value, pdata, len)

int bt_audio_codec_cfg_set_frame_duration(struct bt_audio_codec_cfg *codec_cfg, int frame_duration);
int bt_audio_codec_cfg_meta_set_val(struct bt_audio_codec_cfg *codec_cfg, uint8_t type,
			       const uint8_t *data, size_t data_len);

int     lc3_preset_get_octets_per_frame_value(int sample_rate, int frame_duration);
uint8_t lc3_preset_get_sample_rate_value(int sample_rate);
uint8_t lc3_preset_get_duration_value(int duration);
int     lc3_preset_get_rtn_value(int sample_rate, int frame_duration);
int     lc3_preset_get_latency_value(int sample_rate, int frame_duration);

#define AUDIO_SINK_ROLE_LEFT   1
#define AUDIO_SINK_ROLE_RIGHT  2

void le_audio_sink_role_set(int role);
int  le_audio_sink_role_get(void);

#endif /* __LE_AUDIO_COMMON_H_ */