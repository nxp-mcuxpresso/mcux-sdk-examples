/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "streamer_pcm.h"
#include "fsl_codec_common.h"
#include "app_definitions.h"
#if (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
#include "fsl_cache.h"
#endif
#include "fsl_pdm.h"
#include "fsl_pdm_edma.h"

#define RECORD_BUFFER_SIZE (1920)
#define BUFFER_NUM         (3U)
#define BUFFER_SIZE        (RECORD_BUFFER_SIZE * BUFFER_NUM)

AT_NONCACHEABLE_SECTION_INIT(static pcm_rtos_t pcmHandle) = {0};
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_SIZE], 16);
extern codec_handle_t codecHandle;

AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t s_edmaTcd[BUFFER_NUM], 32U);

static pdm_edma_transfer_t s_receiveXfer[BUFFER_NUM];
static volatile uint32_t s_readIndex = 0U;
static const pdm_config_t pdmConfig  = {
     .enableDoze        = false,
     .fifoWatermark     = DEMO_PDM_FIFO_WATERMARK,
     .qualityMode       = DEMO_PDM_QUALITY_MODE,
     .cicOverSampleRate = DEMO_PDM_CIC_OVERSAMPLE_RATE,
};

static const pdm_channel_config_t channelConfig = {
#if (defined(FSL_FEATURE_PDM_HAS_DC_OUT_CTRL) && (FSL_FEATURE_PDM_HAS_DC_OUT_CTRL))
    .outputCutOffFreq = kPDM_DcRemoverCutOff40Hz,
#else
    .cutOffFreq = kPDM_DcRemoverCutOff152Hz,
#endif
#ifdef DEMO_PDM_CHANNEL_GAIN
    .gain = DEMO_PDM_CHANNEL_GAIN,
#else
    .gain       = kPDM_DfOutputGain7,
#endif
};

void PDM_ERROR_IRQHandler(void)
{
    uint32_t fifoStatus = 0U;

#if (defined(FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ) && (FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ == 1U))
    if (PDM_GetStatus(DEMO_PDM) & PDM_STAT_LOWFREQF_MASK)
    {
        PDM_ClearStatus(DEMO_PDM, PDM_STAT_LOWFREQF_MASK);
    }
#endif

    fifoStatus = PDM_GetFifoStatus(DEMO_PDM);
    if (fifoStatus)
    {
        PDM_ClearFIFOStatus(DEMO_PDM, fifoStatus);
    }
    __DSB();
}

/*! @brief SAI Transmit IRQ handler.
 *
 * This function is used to handle or clear error state.
 */
static void SAI_UserTxIRQHandler(void)
{
    /* Clear the FEF (Tx FIFO underrun) flag. */
    SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    SAI_TxSoftwareReset(DEMO_SAI, kSAI_ResetTypeFIFO);

    SDK_ISR_EXIT_BARRIER;
}

/*! @brief SAI IRQ handler.
 *
 * This function checks FIFO overrun/underrun errors and clears error state.
 */
void SAI1_IRQHandler(void)
{
    if (DEMO_SAI->TCSR & kSAI_FIFOErrorFlag)
        SAI_UserTxIRQHandler();
}

/*! @brief SAI EDMA transmit callback
 *
 * This function is called by the EDMA interface after a block of data has been
 * successfully written to the SAI.
 */
static void saiTxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    pcm_rtos_t *pcm       = (pcm_rtos_t *)userData;
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcm->semaphoreTX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

static void pdmRxCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData)
{
    pcm_rtos_t *pcm       = (pcm_rtos_t *)userData;
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcm->semaphoreRX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    edma_config_t dmaConfig;

    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_DMA, &dmaConfig);
    /* Create DMA handle. */
    EDMA_CreateHandle(&pcmHandle.dmaTxHandle, DEMO_DMA, DEMO_SAI_EDMA_CHANNEL);
    EDMA_CreateHandle(&pcmHandle.dmaRxHandle, DEMO_DMA, DEMO_PDM_EDMA_CHANNEL);

    NVIC_SetPriority(EDMA_0_CH0_IRQn, 2U);
    NVIC_SetPriority(EDMA_0_CH1_IRQn, 2U);

    for (int ibuf = 0; ibuf < BUFFER_NUM; ibuf++)
    {
        s_receiveXfer[ibuf].data     = &s_buffer[ibuf * RECORD_BUFFER_SIZE];
        s_receiveXfer[ibuf].dataSize = RECORD_BUFFER_SIZE;
        int ibuf_next                = ibuf + 1;
        if (ibuf_next == BUFFER_NUM)
            ibuf_next = 0;
        s_receiveXfer[ibuf].linkTransfer = &s_receiveXfer[ibuf_next];
    }

    pcmHandle.isFirstRx = 1;
    pcmHandle.isFirstTx = 1;
}

int streamer_pcm_tx_open(uint32_t num_buffers)
{
    pcmHandle.semaphoreTX = xSemaphoreCreateBinary();
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_SAI_EDMA_CHANNEL, DEMO_SAI_EDMA_SOURCE);
#endif
    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &pcmHandle.saiTxHandle, saiTxCallback, (void *)&pcmHandle,
                                   &pcmHandle.dmaTxHandle);
    return 0;
}

int streamer_pcm_rx_open(uint32_t num_buffers)
{
    pcmHandle.semaphoreRX = xSemaphoreCreateBinary();
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_PDM_EDMA_CHANNEL, DEMO_PDM_EDMA_SOURCE);
#endif
    /* Setup pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_TransferCreateHandleEDMA(DEMO_PDM, &pcmHandle.pdmRxHandle, pdmRxCallback, (void *)&pcmHandle,
                                 &pcmHandle.dmaRxHandle);
    PDM_TransferInstallEDMATCDMemory(&pcmHandle.pdmRxHandle, s_edmaTcd, BUFFER_NUM);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &pcmHandle.pdmRxHandle, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        /* Handle error */
        return -1;
    }
    PDM_EnableInterrupts(DEMO_PDM, kPDM_ErrorInterruptEnable);
    PDM_Reset(DEMO_PDM);
    for (int i = 0; i < BUFFER_NUM; i++)
    {
        PDM_TransferReceiveEDMA(DEMO_PDM, &pcmHandle.pdmRxHandle, &s_receiveXfer[i]);
    }
    return 0;
}

void streamer_pcm_tx_close(void)
{
    /* Stop playback.  This will flush the SAI transmit buffers. */
    SAI_TransferTerminateSendEDMA(DEMO_SAI, &pcmHandle.saiTxHandle);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    /* Release the DMA channel mux */
    EDMA_SetChannelMux(DEMO_DMA, DEMO_SAI_EDMA_CHANNEL, DEMO_SAI_EDMA_SOURCE);
#endif

    vSemaphoreDelete(pcmHandle.semaphoreTX);
}

void streamer_pcm_rx_close(void)
{
    /* Stop playback.  This will flush the SAI transmit buffers. */
    PDM_TransferTerminateReceiveEDMA(DEMO_PDM, &pcmHandle.pdmRxHandle);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    /* Release the DMA channel mux */
    EDMA_SetChannelMux(DEMO_DMA, DEMO_PDM_EDMA_CHANNEL, DEMO_PDM_EDMA_SOURCE);
#endif
    vSemaphoreDelete(pcmHandle.semaphoreRX);
}

int streamer_pcm_write(uint8_t *data, uint32_t size)
{
    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcmHandle.saiTx.dataSize = size - (size % 32);
    pcmHandle.saiTx.data     = data;

    if (pcmHandle.isFirstTx)
    {
        pcmHandle.isFirstTx = 0;
    }
    else
    {
        /* Wait for the previous transfer to finish */
        if (xSemaphoreTake(pcmHandle.semaphoreTX, portMAX_DELAY) != pdTRUE)
            return -1;
    }

    /* Start the transfer */
    SAI_TransferSendEDMA(DEMO_SAI, &pcmHandle.saiTxHandle, &pcmHandle.saiTx);
    SAI_TransferSendEDMA(DEMO_SAI, &pcmHandle.saiTxHandle, &pcmHandle.saiTx);

    return 0;
}

int streamer_pcm_read(uint8_t *data, uint32_t size)
{
    if (pcmHandle.isFirstRx)
    {
        pcmHandle.isFirstRx = 0;
    }
    else
    {
        /* Wait for the previous transfer to finish */
        if (xSemaphoreTake(pcmHandle.semaphoreRX, portMAX_DELAY) != pdTRUE)
            return -1;
        /* Copy data from the DMIC buffer */
        memcpy(data, &s_buffer[s_readIndex], size);
        s_readIndex += size;
        if (s_readIndex >= BUFFER_SIZE)
            s_readIndex -= BUFFER_SIZE;
    }

    return 0;
}

int streamer_pcm_setparams(
    uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool transfer, bool dummy_tx, int volume)
{
    sai_transceiver_t config;

    pcmHandle.sample_rate     = sample_rate;
    pcmHandle.bit_width       = bit_width;
    pcmHandle.num_channels    = num_channels;
    pcmHandle.dummy_tx_enable = dummy_tx;

    /* I2S transfer mode configurations */
    if (transfer)
    {
        /* I2S mode configurations */
        SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_MonoLeft, 1U << DEMO_SAI_CHANNEL);
        config.bitClock.bclkSource = (sai_bclk_source_t)DEMO_SAI_CLOCK_SOURCE;
        config.masterSlave         = DEMO_SAI_MASTER_SLAVE;
        SAI_TransferTxSetConfigEDMA(DEMO_SAI, &pcmHandle.saiTxHandle, &config);

        /* set bit clock divider */
        SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                              DEMO_AUDIO_DATA_CHANNEL);
        SAI_TxEnableInterrupts(DEMO_SAI, kSAI_FIFOErrorInterruptEnable);

        /* master clock configurations */
        BOARD_MASTER_CLOCK_CONFIG();
    }

    CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, DEMO_VOLUME);

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
    CODEC_SetMute(&codecHandle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, mute);

    return 0;
}

int streamer_pcm_set_volume(int volume)
{
    return 0;
}
