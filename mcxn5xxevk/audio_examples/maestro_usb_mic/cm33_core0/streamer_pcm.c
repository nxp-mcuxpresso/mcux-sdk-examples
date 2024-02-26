/*
 * Copyright 2021-2023 NXP
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
#if (defined(DEMO_CODEC_WM8962) && (DEMO_CODEC_WM8962 == 1))
extern codec_config_t boardCodecConfig;
#endif
#include "audio_microphone.h"
#include "fsl_sai.h"

AT_NONCACHEABLE_SECTION_INIT(static pcm_rtos_t pcmHandle) = {0};
extern codec_handle_t codecHandle;
extern usb_audio_microphone_struct_t s_audioMicrophone;
extern uint8_t audioMicDataBuff[AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE];

/*! @brief SAI Receive IRQ handler.
 *
 * This function is used to handle or clear error state.
 */
static void SAI_UserRxIRQHandler(void)
{
    /* Clear the FEF (Rx FIFO overflow) flag. */
    SAI_RxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    SAI_RxSoftwareReset(DEMO_SAI, kSAI_ResetTypeFIFO);
}

/*! @brief SAI IRQ handler.
 *
 * This function checks FIFO overrun/underrun errors and clears error state.
 */
void SAI1_IRQHandler(void)
{
    if (DEMO_SAI->RCSR & kSAI_FIFOErrorFlag)
        SAI_UserRxIRQHandler();
}

/*! @brief SAI EDMA receive callback
 *
 * This function is called by the EDMA interface after a block of data has been
 * successfully read from the SAI.
 */
static void saiRxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    BaseType_t reschedule = -1;
    xSemaphoreGiveFromISR(pcmHandle.semaphoreRX, &reschedule);
    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    edma_config_t dmaConfig;

    /* Interrupt and DMA initialization */
    NVIC_SetPriority(DEMO_I2C_IRQ, 5);

    NVIC_SetPriority(DEMO_SAI_IRQ, 5U);

    NVIC_SetPriority(DEMO_DMA_RX_IRQ, 4U);

    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_DMA, &dmaConfig);
    /* Create DMA handle. */
    EDMA_CreateHandle(&pcmHandle.dmaRxHandle, DEMO_DMA, DEMO_DMA_RX_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_DMA_RX_CHANNEL, DEMO_SAI_RX_SOURCE);
#endif
    /* SAI init */
    SAI_Init(DEMO_SAI);

    pcmHandle.semaphoreRX = xSemaphoreCreateBinary();

    EnableIRQ(DEMO_SAI_IRQ);
}

int streamer_pcm_rx_open(uint32_t num_buffers)
{
    SAI_TransferRxCreateHandleEDMA(DEMO_SAI, &(pcmHandle.saiRxHandle), saiRxCallback, (void *)&pcmHandle,
                                   &(pcmHandle.dmaRxHandle));
    return 0;
}

void streamer_pcm_rx_close(void)
{
    /* Stop playback.  This will flush the SAI transmit buffers. */
    SAI_TransferTerminateReceiveEDMA(DEMO_SAI, &(pcmHandle.saiRxHandle));
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    /* Release the DMA channel mux */
    EDMA_SetChannelMux(DEMO_DMA, DEMO_DMA_RX_CHANNEL, DEMO_SAI_RX_SOURCE);
#endif
    vSemaphoreDelete(pcmHandle.semaphoreRX);
}

int streamer_pcm_write(uint8_t *data, uint32_t size)
{
#if (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
    DCACHE_CleanInvalidateByRange((uint32_t)data, size);
#endif
    if (s_audioMicrophone.start)
    {
        memcpy(audioMicDataBuff + s_audioMicrophone.tdWriteNumber, data, size);
        s_audioMicrophone.tdWriteNumber += size;
        if (s_audioMicrophone.tdWriteNumber >= AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            s_audioMicrophone.tdWriteNumber = 0;
        }
    }

    // Enable SAI Tx due to clock availability for the codec (see board schematic).
    if (pcmHandle.dummy_tx_enable)
        SAI_TxEnable(DEMO_SAI, true);

    return 0;
}

int streamer_pcm_read(uint8_t *data, uint32_t size)
{
    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcmHandle.saiRx.dataSize = size - (size % 32);
    pcmHandle.saiRx.data     = data;

    /* Start the first transfer */
    if (pcmHandle.isFirstRx)
    {
        SAI_TransferReceiveEDMA(DEMO_SAI, &(pcmHandle.saiRxHandle), &(pcmHandle.saiRx));
        pcmHandle.isFirstRx = 0;
    }

    /* Wait for the previous transfer to finish */
    if (xSemaphoreTake(pcmHandle.semaphoreRX, portMAX_DELAY) != pdTRUE)
        return -1;

#if (defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U))
    DCACHE_InvalidateByRange((uint32_t)(pcmHandle.saiRx.data), pcmHandle.saiRx.dataSize);
#endif
    /* Start the consecutive transfer */
    SAI_TransferReceiveEDMA(DEMO_SAI, &(pcmHandle.saiRxHandle), &(pcmHandle.saiRx));

    return 0;
}

/*! @brief Map an integer sample rate (Hz) to internal SAI enum */
static sai_sample_rate_t _pcm_map_sample_rate(uint32_t sample_rate)
{
    switch (sample_rate)
    {
        case 8000:
            return kSAI_SampleRate8KHz;
        case 11025:
            return kSAI_SampleRate11025Hz;
        case 12000:
            return kSAI_SampleRate12KHz;
        case 16000:
            return kSAI_SampleRate16KHz;
        case 24000:
            return kSAI_SampleRate24KHz;
        case 22050:
            return kSAI_SampleRate22050Hz;
        case 32000:
            return kSAI_SampleRate32KHz;
        case 44100:
            return kSAI_SampleRate44100Hz;
        case 48000:
        default:
            return kSAI_SampleRate48KHz;
    }
}

/*! @brief Map an integer bit width (bits) to internal SAI enum */
static sai_word_width_t _pcm_map_word_width(uint32_t bit_width)
{
    switch (bit_width)
    {
        case 8:
            return kSAI_WordWidth8bits;
        case 16:
            return kSAI_WordWidth16bits;
        case 24:
            return kSAI_WordWidth24bits;
        case 32:
            return kSAI_WordWidth32bits;
        default:
            return kSAI_WordWidth16bits;
    }
}

/*! @brief Map an integer number of channels to internal SAI enum */
static sai_mono_stereo_t _pcm_map_channels(uint8_t num_channels)
{
    if (num_channels >= 2)
        return kSAI_Stereo;
    else
        return kSAI_MonoRight;
}

int streamer_pcm_setparams(
    uint32_t sample_rate, uint32_t bit_width, uint8_t num_channels, bool transfer, bool dummy_tx, int volume)
{
    sai_transfer_format_t format = {0};
    sai_transceiver_t saiConfig;
    uint32_t masterClockHz = 0U;

    pcmHandle.isFirstRx    = transfer ? pcmHandle.isFirstRx : 1U;
    pcmHandle.sample_rate  = sample_rate;
    pcmHandle.bit_width    = bit_width;
    pcmHandle.num_channels = num_channels;
    pcmHandle.dummy_tx_enable |= dummy_tx;

#if (!defined(CODEC_DA7212_ENABLE))
    if (sample_rate % 8000 == 0 || sample_rate % 6000 == 0)
    {
        /* Configure Audio PLL clock to 786.432 MHz to  to be divisible by 48000 Hz */
        const clock_audio_pll_config_t audioPllConfig48 = {
            .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
            .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
            .numerator   = 96,  /* 30 bit numerator of fractional loop divider. */
            .denominator = 125, /* 30 bit denominator of fractional loop divider */
        };
        CLOCK_InitAudioPll(&audioPllConfig48);
    }
    else
    {
        /* Configure Audio PLL clock to 722.5344 MHz to be divisible by 44100 Hz */
        const clock_audio_pll_config_t audioPllConfig = {
            .loopDivider = 30,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
            .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
            .numerator   = 66,  /* 30 bit numerator of fractional loop divider. */
            .denominator = 625, /* 30 bit denominator of fractional loop divider */
        };
        CLOCK_InitAudioPll(&audioPllConfig);
    }
#endif

#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
    masterClockHz = OVER_SAMPLE_RATE * format.sampleRate_Hz;
#else
    masterClockHz = DEMO_SAI_CLK_FREQ;
#endif

    format.channel       = 0U;
    format.bitWidth      = _pcm_map_word_width(bit_width);
    format.sampleRate_Hz = _pcm_map_sample_rate(sample_rate);
    format.stereo        = _pcm_map_channels(num_channels);
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER)
    format.masterClockHz = masterClockHz;
#endif

    /* I2S transfer mode configurations */
    if (!transfer)
    {
        /* I2S receive mode configurations */
        SAI_GetClassicI2SConfig(&saiConfig, _pcm_map_word_width(bit_width), format.stereo, 1U << DEMO_SAI_CHANNEL);
        saiConfig.syncMode    = DEMO_SAI_RX_SYNC_MODE;
        saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;

        SAI_TransferRxSetConfigEDMA(DEMO_SAI, &(pcmHandle.saiRxHandle), &saiConfig);
        /* set bit clock divider */
        SAI_RxSetBitClockRate(DEMO_SAI, masterClockHz, _pcm_map_sample_rate(sample_rate),
                              _pcm_map_word_width(bit_width), DEMO_CHANNEL_NUM);

        if (dummy_tx)
        {
            saiConfig.syncMode    = DEMO_SAI_TX_SYNC_MODE;
            saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;
            SAI_TransferTxSetConfigEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle), &saiConfig);
            /* set bit clock divider */
            SAI_TxSetBitClockRate(DEMO_SAI, masterClockHz, _pcm_map_sample_rate(sample_rate),
                                  _pcm_map_word_width(bit_width), DEMO_CHANNEL_NUM);
            /* Enable SAI transmit and FIFO error interrupts. */
            SAI_TxEnableInterrupts(DEMO_SAI, kSAI_FIFOErrorInterruptEnable);
        }

        /* Enable SAI receive and FIFO error interrupts. */
        SAI_RxEnableInterrupts(DEMO_SAI, kSAI_FIFOErrorInterruptEnable);

        /* master clock configurations */
        BOARD_MASTER_CLOCK_CONFIG();
    }

    streamer_pcm_mute(true);
#if (defined(DEMO_CODEC_WM8962) && (DEMO_CODEC_WM8962 == 1))
    CODEC_Init(&codecHandle, &boardCodecConfig);
#endif
    CODEC_SetFormat(&codecHandle, masterClockHz, format.sampleRate_Hz, format.bitWidth);

    streamer_pcm_mute(false);

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

    /* Turn on/off the codec line in module to unmute/mute mic */
    ret = CODEC_SetPower(&codecHandle, kCODEC_ModuleLinein, !mute);
    if (ret != kStatus_Success)
    {
        return 1;
    }

    return 0;
}
