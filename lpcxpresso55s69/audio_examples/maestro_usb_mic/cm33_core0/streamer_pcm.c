/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app_definitions.h"
#include "board.h"
#include "streamer_pcm_app.h"
#include "fsl_debug_console.h"
#include "fsl_codec_common.h"
#include "audio_microphone.h"

pcm_rtos_t pcmHandle = {0};
extern codec_handle_t codecHandle;
extern usb_audio_microphone_struct_t s_audioMicrophone;
extern uint8_t audioMicDataBuff[AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE];

static void RxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    pcm_rtos_t *pcm       = (pcm_rtos_t *)userData;
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcm->semaphoreRX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&pcmHandle.i2sRxDmaHandle, DEMO_DMA, DEMO_I2S_RX_CHANNEL);
}

pcm_rtos_t *streamer_pcm_open(uint32_t num_buffers)
{
    return &pcmHandle;
}

pcm_rtos_t *streamer_pcm_rx_open(uint32_t num_buffers)
{
    I2S_RxTransferCreateHandleDMA(DEMO_I2S_RX, &pcmHandle.i2sRxHandle, &pcmHandle.i2sRxDmaHandle, RxCallback,
                                  (void *)&pcmHandle);
    pcmHandle.semaphoreRX = xSemaphoreCreateBinary();
    return &pcmHandle;
}

void streamer_pcm_start(pcm_rtos_t *pcm)
{
    /* Interrupts already enabled - nothing to do.
     * App/streamer can begin writing data to I2S. */
}

void streamer_pcm_close(pcm_rtos_t *pcm)
{
    I2S_TransferAbortDMA(DEMO_I2S_TX, &pcm->i2sRxHandle);
    CLOCK_DisableClock(kCLOCK_FlexComm7);
    return;
}

void streamer_pcm_rx_close(pcm_rtos_t *pcm)
{
    I2S_TransferAbortDMA(DEMO_I2S_RX, &pcm->i2sRxHandle);
    CLOCK_DisableClock(kCLOCK_FlexComm6);
    vSemaphoreDelete(pcmHandle.semaphoreRX);
    return;
}

int streamer_pcm_write(pcm_rtos_t *pcm, uint8_t *data, uint32_t size)
{
    if (s_audioMicrophone.start)
    {
        memcpy(audioMicDataBuff + s_audioMicrophone.tdWriteNumber, data, size);
        s_audioMicrophone.tdWriteNumber += size;
        if (s_audioMicrophone.tdWriteNumber >= AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            s_audioMicrophone.tdWriteNumber = 0;
        }
    }

    return 0;
}

int streamer_pcm_read(pcm_rtos_t *pcm, uint8_t *data, uint32_t size)
{
    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcm->i2sRxTransfer.dataSize = size - (size % 32);
    pcm->i2sRxTransfer.data     = data;

    /* Start the first transfer */
    if (pcm->isFirstRx)
    {
        /* Need to queue two receive buffers so when the first one is filled,
         * the other one immediatelly starts to be filled */
        I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &pcm->i2sRxHandle, pcm->i2sRxTransfer);
        I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &pcm->i2sRxHandle, pcm->i2sRxTransfer);
        pcm->isFirstRx = 0;
    }
    else
    {
        /* Wait for the previous transfer to finish */
        if (xSemaphoreTake(pcm->semaphoreRX, portMAX_DELAY) != pdTRUE)
            return -1;
    }

    /* Start the consecutive transfer */
    I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &pcm->i2sRxHandle, pcm->i2sRxTransfer);

    /* Enable I2S Tx due to clock availability for the codec (see board schematic).*/
    if (pcm->dummy_tx_enable)
        I2S_Enable(DEMO_I2S_TX);

    return 0;
}

int streamer_pcm_setparams(
    pcm_rtos_t *pcm, uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool tx, bool dummy_tx, int volume)
{
    int ret = 0;
    /* channelNum multiplied by two, because the codec gets data from two channel (CH0 0x00 CH0 0x00...) */
    uint8_t channelNum = num_channels == 1 ? 2 : num_channels;
    int divider        = (CLOCK_GetPll0OutFreq() / sample_rate / bit_width / num_channels);

    pcm->isFirstRx    = tx ? pcm->isFirstRx : 1U;
    pcm->sample_rate  = sample_rate;
    pcm->bit_width    = bit_width;
    pcm->num_channels = num_channels;
    pcm->dummy_tx_enable |= dummy_tx;

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
    if (!tx)
    {
        I2S_RxGetDefaultConfig(&pcmHandle.rx_config);
        pcmHandle.rx_config.oneChannel = num_channels == 1;
        /* As slave, divider need to set to 1 according to the spec. */
        pcmHandle.rx_config.divider     = DEMO_I2S_RX_MODE == kI2S_MasterSlaveNormalSlave ? 1 : divider;
        pcmHandle.rx_config.dataLength  = bit_width;
        pcmHandle.rx_config.frameLength = bit_width * channelNum;
        pcmHandle.rx_config.masterSlave = DEMO_I2S_RX_MODE;
        I2S_RxInit(DEMO_I2S_RX, &pcmHandle.rx_config);

        if (dummy_tx)
        {
            I2S_TxGetDefaultConfig(&pcmHandle.tx_config);
            pcmHandle.tx_config.oneChannel = num_channels == 1;
            /* As slave, divider need to set to 1 according to the spec. */
            pcmHandle.tx_config.divider     = DEMO_I2S_TX_MODE == kI2S_MasterSlaveNormalSlave ? 1 : divider;
            pcmHandle.tx_config.dataLength  = bit_width;
            pcmHandle.tx_config.frameLength = bit_width * channelNum;
            pcmHandle.tx_config.masterSlave = DEMO_I2S_TX_MODE;
            I2S_TxInit(DEMO_I2S_TX, &pcmHandle.tx_config);
        }
    }

    if (streamer_pcm_mute(pcm, true) == 1)
    {
        return 1;
    }

    ret = CODEC_SetFormat(&codecHandle, DEMO_I2S_MASTER_CLOCK_FREQUENCY, sample_rate, bit_width);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    if (streamer_pcm_mute(pcm, false) == 1)
    {
        return 1;
    }

    return 0;
}

void streamer_pcm_getparams(pcm_rtos_t *pcm, uint32_t *sample_rate, uint32_t *bit_width, uint8_t *num_channels)
{
    *sample_rate  = pcm->sample_rate;
    *bit_width    = pcm->bit_width;
    *num_channels = pcm->num_channels;
}

int streamer_pcm_mute(pcm_rtos_t *pcm, bool mute)
{
    status_t ret;

    /* Turn on/off the codec ADC module to unmute/mute */
    ret = CODEC_SetPower(&codecHandle, kCODEC_ModuleADC, !mute);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    return 0;
}

int streamer_pcm_set_volume(pcm_rtos_t *pcm, int volume)
{
    return 0;
}
