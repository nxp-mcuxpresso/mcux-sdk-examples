/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio_i2s.h"

#include "fsl_os_abstraction.h"

#include "fsl_adapter_audio.h"

AT_NONCACHEABLE_SECTION_ALIGN(HAL_AUDIO_HANDLE_DEFINE(audio_tx_handle), 4);

extern uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);
extern hal_audio_config_t audioTxConfig;

typedef struct _block {
    uint32_t len;
    uint8_t buff[AUDIO_I2S_BUFF_SIZE];
} block_t;

struct _audio_i2s {
    bool tx_enable;

    bool tx_working;

    audio_i2s_tx_callback_t tx_callback;

    int tx_block_input;
    int tx_block_output;
    int tx_block_count;

    block_t tx_blocks[AUDIO_I2S_BUFF_COUNT];

    block_t tx_dumy_block;
};
AT_NONCACHEABLE_SECTION_ALIGN(struct _audio_i2s audio_i2s, 4);

static void internal_audio_tx_callback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    int err = 0;
    block_t *block;
    hal_audio_transfer_t xfer;

    if(kStatus_HAL_AudioIdle == completionStatus)
    {
        if(audio_i2s.tx_block_count > 0)
        {
            block = &audio_i2s.tx_blocks[audio_i2s.tx_block_output];

            xfer.dataSize = block->len;
            xfer.data     = block->buff;

            if (kStatus_HAL_AudioSuccess != HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audio_tx_handle[0], &xfer))
            {
                return;
            }
            
            audio_i2s.tx_block_count--;
            audio_i2s.tx_block_output = (audio_i2s.tx_block_output + 1) % ARRAY_SIZE(audio_i2s.tx_blocks);
        }
        else
        {
            block = &audio_i2s.tx_dumy_block;

            xfer.dataSize = block->len;
            xfer.data     = block->buff;

            if (kStatus_HAL_AudioSuccess != HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audio_tx_handle[0], &xfer))
            {
                return;
            }
        }
    }
    else
    {
        err = -1;
    }

    if(audio_i2s.tx_callback)
    {
        audio_i2s.tx_callback(err);
    }
}

int audio_i2s_init(int sample_rate, int channels, int bits, int mode)
{
    if(channels != 2)
    {
        return -1;
    }

    if(mode & AUDIO_I2S_MODE_TX)
    {
        audioTxConfig.sampleRate_Hz = sample_rate;
        audioTxConfig.lineChannels  = kHAL_AudioStereo;
        audioTxConfig.srcClock_Hz   = BOARD_SwitchAudioFreq(sample_rate);

        HAL_AudioTxInit((hal_audio_handle_t)&audio_tx_handle[0], &audioTxConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&audio_tx_handle[0], internal_audio_tx_callback, NULL);

        audio_i2s.tx_enable  = true;
        audio_i2s.tx_working = false;

        audio_i2s.tx_block_input  = 0;
        audio_i2s.tx_block_output = 0;
        audio_i2s.tx_block_count  = 0;

        /* a 2ms mute audio. */
        audio_i2s.tx_dumy_block.len  = 2 * (sample_rate / 1000 * channels * (bits / 8));
        memset(audio_i2s.tx_dumy_block.buff, 0, audio_i2s.tx_dumy_block.len);
    }

    return 0;
}

int audio_i2s_deinit(void)
{
    if(audio_i2s.tx_enable)
    {
        HAL_AudioTxDeinit((hal_audio_handle_t)&audio_tx_handle[0]);

        audio_i2s.tx_block_input  = 0;
        audio_i2s.tx_block_output = 0;
        audio_i2s.tx_block_count  = 0;

        audio_i2s.tx_enable  = false;
        audio_i2s.tx_working = false;
    }

    return 0;
}

int audio_i2s_install_callback(audio_i2s_tx_callback_t tx_callback)
{
    if(audio_i2s.tx_enable)
    {
        audio_i2s.tx_callback = tx_callback;
    }

    return 0;
}

int audio_i2s_write(uint8_t *data, int len)
{
    block_t *block = NULL;

    if(audio_i2s.tx_block_count < ARRAY_SIZE(audio_i2s.tx_blocks))
    {
        block = &audio_i2s.tx_blocks[audio_i2s.tx_block_input];
        
        memcpy(block->buff, data, len);
        block->len = len;
        
        audio_i2s.tx_block_count++;
        audio_i2s.tx_block_input = (audio_i2s.tx_block_input + 1) % ARRAY_SIZE(audio_i2s.tx_blocks);

        return 0;
    }

    return -1;
}

int audio_i2s_start(void)
{
    block_t *block;
    hal_audio_transfer_t xfer;

    if(audio_i2s.tx_enable)
    {
        int count = audio_i2s.tx_block_count;
        count = (count < 2) ? count : 2; /* only prepare two transfer. */
        for(int i = 0; i < count; i++)
        {
            block = &audio_i2s.tx_blocks[audio_i2s.tx_block_output];

            xfer.dataSize = block->len;
            xfer.data     = block->buff;

            if (kStatus_HAL_AudioSuccess != HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audio_tx_handle[0], &xfer))
            {
                return -1;
            }

            audio_i2s.tx_block_count--;
            audio_i2s.tx_block_output = (audio_i2s.tx_block_output + 1) % ARRAY_SIZE(audio_i2s.tx_blocks);
        }

        audio_i2s.tx_working = true;
    }

    return 0;
}

int audio_i2s_stop(void)
{
    if(audio_i2s.tx_working)
    {
        if(kStatus_HAL_AudioSuccess != HAL_AudioTransferAbortSend((hal_audio_handle_t)&audio_tx_handle[0]))
        {
            return -1;
        }

        audio_i2s.tx_working = false;
    }

    return 0;
}

bool audio_i2s_is_working(void)
{
    if(audio_i2s.tx_working)
    {
        return true;
    }

    return false;
}