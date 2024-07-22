/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "hw_codec.h"

#include "fsl_codec_common.h"

static codec_handle_t codec_handle;
extern codec_config_t boardCodecConfig;
extern uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);

#define VOL_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define VOL_MIN(a, b) (((a) < (b)) ? (a) : (b))

static uint8_t volume_step = 10;
static uint8_t volume      = 90;

int hw_codec_init(int sample_rate, int channels, int bits)
{
    status_t status;
    uint32_t mclk;

    mclk = BOARD_SwitchAudioFreq(sample_rate);

    status = CODEC_Init(&codec_handle, &boardCodecConfig);
    if (kStatus_Success != status)
	{
		return HW_CODEC_ERROR;
	}

    if(hw_codec_mute())
    {
        return HW_CODEC_ERROR;
    }

    status = CODEC_SetFormat(&codec_handle, mclk, sample_rate, bits);
    if (kStatus_Success != status)
	{
		return HW_CODEC_ERROR;
	}

    if(hw_codec_unmute())
    {
        return HW_CODEC_ERROR;
    }

    if(hw_codec_vol_set(volume))
    {
        return HW_CODEC_ERROR;
    }

    (void)channels;

    return 0;
}

int hw_codec_deinit()
{
    if(hw_codec_mute())
    {
        return HW_CODEC_ERROR;
    }

    if (CODEC_Deinit(&codec_handle) != kStatus_Success)
    {
        return HW_CODEC_ERROR;
    }

    return 0;
}

int hw_codec_vol_set_step(uint8_t step)
{
    volume_step = step;

    return 0;
}

int hw_codec_vol_get(void)
{
    return volume;
}

int hw_codec_vol_set(uint8_t vol)
{
    status_t status;

    status = CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, vol);
    if (kStatus_Success != status)
	{
		return HW_CODEC_ERROR;
	}

    volume = vol;

    return 0;
}

int hw_codec_vol_up(void)
{
	volume = VOL_MIN(volume + volume_step, 100);

	return hw_codec_vol_set(volume);
}

int hw_codec_vol_down(void)
{
	volume = VOL_MAX(volume - volume_step, 0);

	return hw_codec_vol_set(volume);
}

int hw_codec_mute(void)
{
    status_t status;

    status = CODEC_SetMute(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, true);
    if (kStatus_Success != status)
	{
		return HW_CODEC_ERROR;
	}

    return 0;
}

int hw_codec_unmute(void)
{
    status_t status;

    status = CODEC_SetMute(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, false);
    if (kStatus_Success != status)
	{
		return HW_CODEC_ERROR;
	}

    return 0;
}