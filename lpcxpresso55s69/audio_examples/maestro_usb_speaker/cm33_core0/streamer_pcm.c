/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_definitions.h"
#include "board.h"
#include "streamer_pcm.h"
#include "fsl_debug_console.h"
#include "fsl_codec_common.h"
#include "audio_speaker.h"

pcm_rtos_t pcmHandle = {0};

extern codec_handle_t codecHandle;
extern usb_audio_speaker_struct_t g_UsbDeviceAudioSpeaker;
extern uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME];
static uint8_t audioPlayDMATempBuff[AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME] = {0};

static void TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcmHandle.semaphoreTX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    DMA_Init(DEMO_DMA);
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&(pcmHandle.i2sTxDmaHandle), DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    NVIC_SetPriority(DMA0_IRQn, 2U);
}

int streamer_pcm_tx_open(uint32_t num_buffers)
{
    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle), &(pcmHandle.i2sTxDmaHandle), TxCallback,
                                  (void *)&pcmHandle);
    pcmHandle.semaphoreTX = xSemaphoreCreateBinary();
    return 0;
}

void streamer_pcm_tx_close(void)
{
    /* Stop playback. This will flush the I2S transmit buffers. */
    if (pcmHandle.i2sTxHandle.state != 0)
    {
        I2S_TransferAbortDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle));
    }
    vSemaphoreDelete(pcmHandle.semaphoreTX);
}

int streamer_pcm_write(uint8_t *data, uint32_t size)
{
    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcmHandle.i2sTxTransfer.dataSize = size - (size % 32);
    pcmHandle.i2sTxTransfer.data     = data;

    /* Start the consecutive transfer */
    while (I2S_TxTransferSendDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle), pcmHandle.i2sTxTransfer) == kStatus_I2S_Busy)
    {
        /* Wait for transfer to finish */
        if (xSemaphoreTake(pcmHandle.semaphoreTX, portMAX_DELAY) != pdTRUE)
        {
            return -1;
        }
    }

    if (pcmHandle.isFirstTx)
    {
        /* Need to queue two transmit buffers so when the first one
         * finishes transfer, the other immediatelly starts */
        I2S_TxTransferSendDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle), pcmHandle.i2sTxTransfer);
        pcmHandle.isFirstTx = 0;
    }

    return 0;
}

int streamer_pcm_read(uint8_t *data, uint32_t size)
{
    uint32_t audioSpeakerPreReadDataCount = 0U;
    uint32_t preAudioSendCount            = 0U;
    static i2s_transfer_t xfer            = {0};

    if ((USB_AudioSpeakerBufferSpaceUsed() < (g_UsbDeviceAudioSpeaker.audioPlayTransferSize)) &&
        (g_UsbDeviceAudioSpeaker.startPlayFlag == 1U))
    {
        g_UsbDeviceAudioSpeaker.startPlayFlag          = 0;
        g_UsbDeviceAudioSpeaker.speakerDetachOrNoInput = 1;
    }
    if (0U != g_UsbDeviceAudioSpeaker.startPlayFlag)
    {
        USB_DeviceCalculateFeedback();
        xfer.dataSize     = g_UsbDeviceAudioSpeaker.audioPlayTransferSize;
        xfer.data         = audioPlayDataBuff + g_UsbDeviceAudioSpeaker.tdReadNumberPlay;
        preAudioSendCount = g_UsbDeviceAudioSpeaker.audioSendCount[0];
        g_UsbDeviceAudioSpeaker.audioSendCount[0] += g_UsbDeviceAudioSpeaker.audioPlayTransferSize;
        if (preAudioSendCount > g_UsbDeviceAudioSpeaker.audioSendCount[0])
        {
            g_UsbDeviceAudioSpeaker.audioSendCount[1] += 1U;
        }
        g_UsbDeviceAudioSpeaker.audioSendTimes++;
        g_UsbDeviceAudioSpeaker.tdReadNumberPlay += g_UsbDeviceAudioSpeaker.audioPlayTransferSize;
        if (g_UsbDeviceAudioSpeaker.tdReadNumberPlay >= g_UsbDeviceAudioSpeaker.audioPlayBufferSize)
        {
            g_UsbDeviceAudioSpeaker.tdReadNumberPlay = 0;
        }
        audioSpeakerPreReadDataCount = g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0];
        g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0] += g_UsbDeviceAudioSpeaker.audioPlayTransferSize;
        if (audioSpeakerPreReadDataCount > g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[0])
        {
            g_UsbDeviceAudioSpeaker.audioSpeakerReadDataCount[1] += 1U;
        }
    }
    else
    {
        if (0U != g_UsbDeviceAudioSpeaker.audioPlayTransferSize)
        {
            xfer.dataSize = g_UsbDeviceAudioSpeaker.audioPlayTransferSize;
        }
        else
        {
            xfer.dataSize = AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME / 8U;
        }
        xfer.data = audioPlayDMATempBuff;
    }

    memcpy(data, xfer.data, size);

    return 0;
}

int streamer_pcm_setparams(
    uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool tx, bool dummy_tx, int volume)
{
    int ret     = 0;
    int divider = (CLOCK_GetPll0OutFreq() / sample_rate / bit_width / num_channels);

    pcmHandle.isFirstTx    = tx ? 1U : pcmHandle.isFirstTx;
    pcmHandle.sample_rate  = sample_rate;
    pcmHandle.bit_width    = bit_width;
    pcmHandle.num_channels = num_channels;

    /*
     * masterSlave = kI2S_MasterSlaveNormalMaster;
     * mode = kI2S_ModeI2sClassic;
     * rightLow = false;
     * leftJust = false;
     * pdmData = false;
     * sckPol = false;
     * wsPol = false;
     * divider = 1;
     * oneChannel = false;
     * dataLength = 16;
     * frameLength = 32;
     * position = 0;
     * watermark = 4;
     * txEmptyZero = true;
     * pack48 = false;
     */
    if (tx)
    {
        /* Flush the I2S transmit buffers. */
        if (pcmHandle.i2sTxHandle.state != 0)
        {
            I2S_TransferAbortDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle));
        }
        I2S_TxGetDefaultConfig(&(pcmHandle.tx_config));
        /* As slave, divider need to set to 1 according to the spec. */
        pcmHandle.tx_config.divider     = DEMO_I2S_TX_MODE == kI2S_MasterSlaveNormalSlave ? 1 : divider;
        pcmHandle.tx_config.dataLength  = bit_width;
        pcmHandle.tx_config.frameLength = bit_width * num_channels;
        pcmHandle.tx_config.masterSlave = DEMO_I2S_TX_MODE;
        I2S_TxInit(DEMO_I2S_TX, &(pcmHandle.tx_config));
    }

    if (streamer_pcm_mute(true) == 1)
    {
        return 1;
    }

    ret = CODEC_SetFormat(&codecHandle, DEMO_I2S_MASTER_CLOCK_FREQUENCY, sample_rate, bit_width);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    if (streamer_pcm_set_volume(volume) == 1)
    {
        return 1;
    }

    return 0;
}

void streamer_pcm_getparams(uint32_t *sample_rate, uint32_t *bit_width, uint8_t *num_channels)
{
    *sample_rate  = pcmHandle.sample_rate;
    *bit_width    = pcmHandle.bit_width;
    *num_channels = pcmHandle.num_channels;
}

int streamer_pcm_mute(bool mute)
{
    status_t ret;
    ret = CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, mute);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    return 0;
}

int streamer_pcm_set_volume(int volume)
{
    int channel;

    channel = (pcmHandle.num_channels == 1) ? kCODEC_PlayChannelHeadphoneLeft :
                                              kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight;

    if (volume <= 0)
        CODEC_SetMute(&codecHandle, channel, true);
    else
        CODEC_SetVolume(&codecHandle, channel, volume > CODEC_VOLUME_MAX_VALUE ? CODEC_VOLUME_MAX_VALUE : volume);

    return 0;
}
