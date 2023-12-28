/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "le_audio_common.h"

#include <sys/check.h>

int audio_data_stereo_split(int samples, int bits, uint8_t *input, uint8_t *output_left, uint8_t *output_right)
{
    int bytes_pre_sample = bits / 8;

    for(int i = 0; i < samples; i++)
    {
        for(int j = 0; j < bytes_pre_sample; j++)
        {
            *output_left++ = *input++;
        }
        for(int j = 0; j < bytes_pre_sample; j++)
        {
            *output_right++ = *input++;
        }
    }

    return 0;
}

int audio_data_make_stereo(int samples, int bits, uint8_t *input_left, uint8_t *input_right, uint8_t *output)
{
    int bytes_pre_sample = bits / 8;

    for(int i = 0; i < samples; i++)
    {
        for(int j = 0; j < bytes_pre_sample; j++)
        {
            *output++ = *input_left++;
        }
        for(int j = 0; j < bytes_pre_sample; j++)
        {
            *output++ = *input_right++;
        }
    }

    return 0;
}

int bt_audio_codec_cfg_set_frame_duration(struct bt_audio_codec_cfg *codec_cfg, int frame_duration)
{
	uint8_t frame_duration_u8;

	if((frame_duration != BT_AUDIO_CODEC_CONFIG_LC3_DURATION_7_5) && (frame_duration != BT_AUDIO_CODEC_CONFIG_LC3_DURATION_10))
	{
		return -EINVAL;
	}

	frame_duration_u8 = (uint8_t)frame_duration;

	return bt_audio_codec_cfg_set_val(codec_cfg, BT_AUDIO_CODEC_CONFIG_LC3_DURATION, &frame_duration_u8,
					  sizeof(frame_duration_u8));
}

static void init_net_buf_simple_from_codec_cfg_meta(struct net_buf_simple *buf,
					       struct bt_audio_codec_cfg *codec_cfg)
{
	buf->__buf = codec_cfg->meta;
	buf->data = codec_cfg->meta;
	buf->size = sizeof(codec_cfg->meta);
	buf->len = codec_cfg->meta_len;
}

int bt_audio_codec_cfg_meta_set_val(struct bt_audio_codec_cfg *codec_cfg, uint8_t type,
			       const uint8_t *data, size_t data_len)
{
	CHECKIF(codec_cfg == NULL) {
		PRINTF("codec_cfg is NULL");
		return -EINVAL;
	}

	CHECKIF(data == NULL) {
		PRINTF("data is NULL");
		return -EINVAL;
	}

	CHECKIF(data_len == 0U || data_len > UINT8_MAX) {
		PRINTF("Invalid data_len %zu", data_len);
		return -EINVAL;
	}

	for (uint16_t i = 0U; i < codec_cfg->meta_len;) {
		uint8_t *len = &codec_cfg->meta[i++];
		const uint8_t data_type = codec_cfg->meta[i++];
		const uint8_t value_len = *len - sizeof(data_type);

		if (data_type == type) {
			uint8_t *value = &codec_cfg->meta[i];

			if (data_len == value_len) {
				memcpy(value, data, data_len);
			} else {
				const int16_t diff = data_len - value_len;
				uint8_t *old_next_data_start;
				uint8_t *new_next_data_start;
				uint8_t data_len_to_move;

				/* Check if this is the last value in the buffer */
				if (value + value_len == codec_cfg->meta + codec_cfg->meta_len) {
					data_len_to_move = 0U;
				} else {
					old_next_data_start = value + value_len + 1;
					new_next_data_start = value + data_len + 1;
					data_len_to_move = codec_cfg->meta_len -
							   (old_next_data_start - codec_cfg->meta);
				}

				if (diff < 0) {
					/* In this case we need to move memory around after the copy
					 * to fit the new shorter data
					 */

					memcpy(value, data, data_len);
					if (data_len_to_move > 0U) {
						memmove(new_next_data_start, old_next_data_start,
							data_len_to_move);
					}
				} else {
					/* In this case we need to move memory around before
					 * the copy to fit the new longer data
					 */
					if ((codec_cfg->meta_len + diff) >
					    ARRAY_SIZE(codec_cfg->meta)) {
						PRINTF("Cannot fit meta_len %zu in buf with len "
							"%u and size %u",
							data_len, codec_cfg->meta_len,
							ARRAY_SIZE(codec_cfg->meta));
						return -ENOMEM;
					}

					if (data_len_to_move > 0) {
						memmove(new_next_data_start, old_next_data_start,
							data_len_to_move);
					}

					memcpy(value, data, data_len);
				}

				codec_cfg->meta_len += diff;
				*len += diff;
			}

			return codec_cfg->meta_len;
		}

		i += value_len;
	}

	/* If we reach here, we did not find the data in the buffer, so we simply add it */
	if ((codec_cfg->meta_len + data_len) <= ARRAY_SIZE(codec_cfg->meta)) {
		struct net_buf_simple buf;

		init_net_buf_simple_from_codec_cfg_meta(&buf, codec_cfg);

		net_buf_simple_add_u8(&buf, data_len + sizeof(type));
		net_buf_simple_add_u8(&buf, type);
		if (data_len > 0) {
			net_buf_simple_add_mem(&buf, data, data_len);
		}
		codec_cfg->meta_len = buf.len;
	} else {
		PRINTF("Cannot fit meta %zu in codec_cfg with len %u and size %u", data_len,
			codec_cfg->meta_len, ARRAY_SIZE(codec_cfg->meta));
		return -ENOMEM;
	}

	return codec_cfg->meta_len;
}

int lc3_preset_get_octets_per_frame_value(int sample_rate, int frame_duration)
{
	int octets = 0;

    if (frame_duration == 7500)
    {
        switch(sample_rate)
        {
            case 8000:	octets = 26; break;
            case 16000:	octets = 30; break;
            case 24000:	octets = 45; break;
            case 32000:	octets = 60; break;
            case 44100:	octets = 97; break;
            case 48000:	octets = 75; break;
            default:
                /* Not support. */
                while(1);
        }
    }
    else if (frame_duration == 10000)
    {
        switch(sample_rate)
        {
            case 8000:	octets = 30; break;
            case 16000:	octets = 40; break;
            case 24000:	octets = 60; break;
            case 32000:	octets = 80; break;
            case 44100:	octets = 130; break;
            case 48000:	octets = 100; break;
            default:
                /* Not support. */
                while(1);
        }
    }
	else
    {
        /* Not support. */
        while(1);
    }

	return octets;
}

uint8_t lc3_preset_get_sample_rate_value(int sample_rate)
{
	uint8_t value = 0;

	switch(sample_rate)
	{
		case 8000:	value = BT_AUDIO_CODEC_CONFIG_LC3_FREQ_8KHZ; break;
		case 16000:	value = BT_AUDIO_CODEC_CONFIG_LC3_FREQ_16KHZ; break;
		case 24000:	value = BT_AUDIO_CODEC_CONFIG_LC3_FREQ_24KHZ; break;
		case 32000:	value = BT_AUDIO_CODEC_CONFIG_LC3_FREQ_32KHZ; break;
		case 48000:	value = BT_AUDIO_CODEC_CONFIG_LC3_FREQ_48KHZ; break;
		default:
			/* Not support. */
			while(1);
	}

	return value;
}

uint8_t lc3_preset_get_duration_value(int duration)
{
	uint8_t value = 0;
	/* Here we only use 10ms interval setting. */
	switch(duration)
	{
		case  7500:	value = BT_AUDIO_CODEC_CONFIG_LC3_DURATION_7_5; break;
		case 10000:	value = BT_AUDIO_CODEC_CONFIG_LC3_DURATION_10; break;
		default:
			/* Not support. */
			while(1);
	}
	return value;
}

int lc3_preset_get_rtn_value(int sample_rate, int frame_duration)
{
	int rtn = 0;

    if (frame_duration == 7500)
    {
        switch(sample_rate)
        {
            case 8000:	rtn = 2; break;
            case 16000:	rtn = 2; break;
            case 24000:	rtn = 2; break;
            case 32000:	rtn = 2; break;
            case 44100:	rtn = 4; break;
            case 48000:	rtn = 4; break;
            default:
                /* Not support. */
                while(1);
        }
    }
    else if (frame_duration == 10000)
    {
        switch(sample_rate)
        {
            case 8000:	rtn = 2; break;
            case 16000:	rtn = 2; break;
            case 24000:	rtn = 2; break;
            case 32000:	rtn = 2; break;
            case 44100:	rtn = 4; break;
            case 48000:	rtn = 4; break;
            default:
                /* Not support. */
                while(1);
        }
    }
    else
    {
        /* Not support. */
        while(1);
    }

	return rtn;
}

int lc3_preset_get_latency_value(int sample_rate, int frame_duration)
{
	int latency = 0;

    if (frame_duration == 7500)
    {
        switch(sample_rate)
        {
            case 8000:	latency = 8; break;
            case 16000:	latency = 8; break;
            case 24000:	latency = 8; break;
            case 32000:	latency = 8; break;
            case 44100:	latency = 24; break;
            case 48000:	latency = 15; break;
            default:
                /* Not support. */
                while(1);
        }
    }
    else if (frame_duration == 10000)
    {
        switch(sample_rate)
        {
            case 8000:	latency = 10; break;
            case 16000:	latency = 10; break;
            case 24000:	latency = 10; break;
            case 32000:	latency = 10; break;
            case 44100:	latency = 31; break;
            case 48000:	latency = 20; break;
            default:
                /* Not support. */
                while(1);
        }
    }
    else
    {
        /* Not support. */
        while(1);
    }

	return latency;
}

static int le_audio_sink_role = 0;

void le_audio_sink_role_set(int role)
{
	if(le_audio_sink_role == 0)
	{
		le_audio_sink_role = role;
	}
}

int  le_audio_sink_role_get(void)
{
	return le_audio_sink_role;
}