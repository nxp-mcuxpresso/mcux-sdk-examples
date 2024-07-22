/*
 * Copyright 2023-2024 NXP
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