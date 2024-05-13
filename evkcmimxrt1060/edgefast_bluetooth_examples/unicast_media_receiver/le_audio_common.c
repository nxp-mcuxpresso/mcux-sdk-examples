/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "le_audio_common.h"

#include <sys/check.h>
#include "fsl_debug_console.h"

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
            case 44100:	rtn = 5; break;
            case 48000:	rtn = 5; break;
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
            case 44100:	rtn = 5; break;
            case 48000:	rtn = 5; break;
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