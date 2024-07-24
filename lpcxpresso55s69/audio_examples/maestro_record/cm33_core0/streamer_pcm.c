
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

pcm_rtos_t pcmHandle = {0};

extern codec_handle_t codecHandle;

volatile int rx_data_valid = 0;

static void TxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcmHandle.semaphoreTX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

static void RxCallback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcmHandle.semaphoreRX, &reschedule);
    rx_data_valid++;

    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&(pcmHandle.i2sTxDmaHandle), DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&(pcmHandle.i2sRxDmaHandle), DEMO_DMA, DEMO_I2S_RX_CHANNEL);

    NVIC_SetPriority(DMA0_IRQn, 2U);

    pcmHandle.isFirstRx = 0;
    pcmHandle.isFirstTx = 0;
    rx_data_valid       = 0;
}

int streamer_pcm_tx_open(uint32_t num_buffers)
{
    pcmHandle.semaphoreTX = xSemaphoreCreateBinary();
    return 0;
}

int streamer_pcm_rx_open(uint32_t num_buffers)
{
    pcmHandle.semaphoreRX = xSemaphoreCreateBinary();
    return 0;
}

void streamer_pcm_start(void)
{
    /* Interrupts already enabled - nothing to do.
     * App/streamer can begin writing data to SAI. */
    if (pcmHandle.isFirstTx == 1)
    {
        I2S_Enable(DEMO_I2S_TX);
    }
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

void streamer_pcm_rx_close(void)
{
    if (pcmHandle.i2sRxHandle.state != 0)
    {
        I2S_TransferAbortDMA(DEMO_I2S_RX, &(pcmHandle.i2sRxHandle));
    }
    vSemaphoreDelete(pcmHandle.semaphoreRX);
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
    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcmHandle.i2sRxTransfer.dataSize = size - (size % 32);
    pcmHandle.i2sRxTransfer.data     = data;

    /* Start the first transfer */
    if (pcmHandle.isFirstRx)
    {
        I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &(pcmHandle.i2sRxHandle), pcmHandle.i2sRxTransfer);
        pcmHandle.isFirstRx = 0;
        rx_data_valid--;
    }

    /* Start the consecutive transfer */
    while (I2S_RxTransferReceiveDMA(DEMO_I2S_RX, &(pcmHandle.i2sRxHandle), pcmHandle.i2sRxTransfer) == kStatus_I2S_Busy)
    {
        /* Wait for transfer to finish */
        if (xSemaphoreTake(pcmHandle.semaphoreRX, portMAX_DELAY) != pdTRUE)
        {
            return -1;
        }
    }

    /* Enable I2S Tx due to clock availability for the codec (see board schematic).*/
    if (pcmHandle.dummy_tx_enable)
        I2S_Enable(DEMO_I2S_TX);

    if (rx_data_valid > 0)
    {
        rx_data_valid--;
        /* Signal that data are already ready for processing */
        return 0;
    }
    else
    {
        /* Signal that data are not ready for processing yet */
        return 1;
    }
}

int streamer_pcm_setparams(
    uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool tx, bool dummy_tx, int volume)
{
    int ret     = 0;
    int divider = 0;

    pcmHandle.isFirstTx    = tx ? 1U : pcmHandle.isFirstTx;
    pcmHandle.isFirstRx    = tx ? pcmHandle.isFirstRx : 1U;
    pcmHandle.sample_rate  = sample_rate;
    pcmHandle.bit_width    = bit_width;
    pcmHandle.num_channels = num_channels;
    pcmHandle.dummy_tx_enable |= dummy_tx;

    if (sample_rate % 8000 == 0 || sample_rate % 6000 == 0)
    {
        const pll_setup_t pll0Setup = {
            .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(8U) | SYSCON_PLL0CTRL_SELP(31U),
            .pllndec = SYSCON_PLL0NDEC_NDIV(125U),
            .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
            .pllsscg = {0x0U, (SYSCON_PLL0SSCG1_MDIV_EXT(3072U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
            .pllRate = 24576000U,
            .flags   = PLL_SETUPFLAG_WAITLOCK};
        /*!< Configure PLL to the desired values */
        CLOCK_SetPLL0Freq(&pll0Setup);
    }
    else if (sample_rate % 11025 == 0)
    {
        const pll_setup_t pll0Setup = {
            .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(8U) | SYSCON_PLL0CTRL_SELP(31U),
            .pllndec = SYSCON_PLL0NDEC_NDIV(202U),
            .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
            .pllsscg = {0x0U, (SYSCON_PLL0SSCG1_MDIV_EXT(4561U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
            .pllRate = 22579200U,
            .flags   = PLL_SETUPFLAG_WAITLOCK};
        CLOCK_SetPLL0Freq(&pll0Setup); /*!< Configure PLL0 to the desired values */
    }

    divider = (CLOCK_GetPll0OutFreq() / sample_rate / bit_width / DEMO_CHANNEL_NUM);

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
        I2S_TxGetDefaultConfig(&pcmHandle.tx_config);
        pcmHandle.tx_config.divider     = divider;
        pcmHandle.tx_config.masterSlave = DEMO_I2S_TX_MODE;
        pcmHandle.tx_config.oneChannel  = (pcmHandle.num_channels == 1);
        I2S_TxInit(DEMO_I2S_TX, &(pcmHandle.tx_config));
        I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &(pcmHandle.i2sTxHandle), &(pcmHandle.i2sTxDmaHandle), TxCallback,
                                      (void *)&pcmHandle);
    }
    else
    {
        I2S_RxGetDefaultConfig(&pcmHandle.rx_config);
        pcmHandle.rx_config.divider     = divider;
        pcmHandle.rx_config.masterSlave = DEMO_I2S_RX_MODE;
        pcmHandle.rx_config.oneChannel  = (pcmHandle.num_channels == 1);
        I2S_RxInit(DEMO_I2S_RX, &pcmHandle.rx_config);
        I2S_RxTransferCreateHandleDMA(DEMO_I2S_RX, &(pcmHandle.i2sRxHandle), &(pcmHandle.i2sRxDmaHandle), RxCallback,
                                      (void *)&pcmHandle);

        if (dummy_tx)
        {
            I2S_TxGetDefaultConfig(&pcmHandle.tx_config);
            pcmHandle.tx_config.divider     = divider;
            pcmHandle.tx_config.masterSlave = DEMO_I2S_TX_MODE;
            I2S_TxInit(DEMO_I2S_TX, &(pcmHandle.tx_config));
        }
    }

    ret = streamer_pcm_set_volume(0);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    ret = CODEC_SetFormat(&codecHandle, CLOCK_GetPll0OutFreq(), sample_rate, bit_width);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    ret = streamer_pcm_set_volume(DEMO_VOLUME);
    if (ret != kStatus_Success)
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
        CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    else
        CODEC_SetVolume(&codecHandle, channel, volume > CODEC_VOLUME_MAX_VALUE ? CODEC_VOLUME_MAX_VALUE : volume);

    return 0;
}
