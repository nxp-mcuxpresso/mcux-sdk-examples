/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "streamer_pcm.h"
#include "fsl_codec_common.h"
#include "fsl_wm8962.h"
#include "app_definitions.h"
#include "fsl_cache.h"
#include "fsl_pdm.h"
#include "fsl_pdm_edma.h"

volatile int8_t PDM_started = 0;  /* Indicates that the PDM transfer has already started. */
volatile int8_t SAI_started = 0;  /* Indicates that the SAI transfer has already started. */
volatile int rx_data_valid  = 0;  /* Indicates that RX data are ready for processing. */
volatile int tx_data_valid  = 0;  /* Indicates that RX data are ready for processing. */

#define RECORD_BUFFER_SIZE (3840) // 16kHz * 4bytes * 2channels * 30ms
#define BUFFER_NUM         (3U)
#define BUFFER_SIZE        (RECORD_BUFFER_SIZE * BUFFER_NUM)

AT_NONCACHEABLE_SECTION_INIT(static pcm_rtos_t pcmHandle) = {0};
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_SIZE], 32);
extern codec_handle_t codecHandle;

// Queue for storing data buffer addresses
typedef struct
{
    int num_buffers; /* Number of all required buffers */
    int head;
    int tail;
    int count;
    uint8_t *buff_addr[SAI_XFER_QUEUE_SIZE + 2];
} Streamer_buff_addr_T;

static Streamer_buff_addr_T streamer_buff_addr = {
    .num_buffers = SAI_XFER_QUEUE_SIZE + 2, // num_buffers must be one greater than AUDIO_SRC_BUFFER_NUM
    .head        = 0,
    .tail        = 0,
};

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
    .gain = kPDM_DfOutputGain5,
};

int Streamer_buff_addr_Push(uint8_t *buff_addr)
{
    if ((streamer_buff_addr.num_buffers <= 0) || (buff_addr == NULL))
    {
        return -1;
    }

    if (streamer_buff_addr.count == streamer_buff_addr.num_buffers)
    {
        return -1; /* Queue is full*/
    }

    streamer_buff_addr.buff_addr[streamer_buff_addr.head] = buff_addr;
    streamer_buff_addr.head = (streamer_buff_addr.head + 1) % streamer_buff_addr.num_buffers;
    streamer_buff_addr.count++;

    return 0;
}

uint8_t *Streamer_buff_addr_Pull(void)
{
    uint8_t *buff_addr = NULL;
    if (0 == streamer_buff_addr.count)
    {
        return buff_addr; /* Queue is empty */
    }

    buff_addr               = streamer_buff_addr.buff_addr[streamer_buff_addr.tail];
    streamer_buff_addr.tail = (streamer_buff_addr.tail + 1) % streamer_buff_addr.num_buffers;
    streamer_buff_addr.count--;

    return buff_addr;
}

/*! @brief SAI EDMA transmit callback
 *
 * This function is called by the EDMA interface after a block of data has been
 * successfully written to the SAI.
 */
static void saiTxCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_TxError == status)
    {
        /* Handle the error. */
    }
    BaseType_t reschedule = -1;
    tx_data_valid++;
    if (PDM_started == 0)
    {
        PDM_EnableDMA(DEMO_PDM, true);
        PDM_started = 1;
        tx_data_valid--;
    }
    OSA_SemaphorePost(pcmHandle.semaphoreTX);
    portYIELD_FROM_ISR(reschedule);
}

static void pdmRxCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData)
{
    BaseType_t reschedule = -1;
    rx_data_valid++;
    portYIELD_FROM_ISR(reschedule);
}

void streamer_pcm_init(void)
{
    edma_config_t dmaConfig;

    NVIC_SetPriority(PDM_ERROR_IRQn, 5);
    NVIC_SetPriority(SAI1_IRQn, 5);
    NVIC_SetPriority(DMA_ERROR_IRQn, 5);

    NVIC_SetPriority(DMA1_DMA17_IRQn, 4U);
    NVIC_SetPriority(DMA0_DMA16_IRQn, 4U);

    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_EDMA, &dmaConfig);
    /* Create DMA handle. */
    EDMA_CreateHandle(&(pcmHandle.dmaTxHandle), DEMO_EDMA, DEMO_TX_CHANNEL);
    EDMA_CreateHandle(&(pcmHandle.dmaRxHandle), DEMO_EDMA, DEMO_RX_CHANNEL);

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle), saiTxCallback, (void *)&pcmHandle,
                                   &(pcmHandle.dmaTxHandle));

    /* Setup pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_TransferCreateHandleEDMA(DEMO_PDM, &(pcmHandle.pdmRxHandle), pdmRxCallback, (void *)&pcmHandle,
                                 &(pcmHandle.dmaRxHandle));
    PDM_TransferInstallEDMATCDMemory(&(pcmHandle.pdmRxHandle), s_edmaTcd, BUFFER_NUM);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &(pcmHandle.pdmRxHandle), DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &(pcmHandle.pdmRxHandle), DEMO_PDM_ENABLE_CHANNEL_RIGHT, &channelConfig);
    PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE);
    PDM_EnableInterrupts(DEMO_PDM, kPDM_ErrorInterruptEnable);
    PDM_Reset(DEMO_PDM);

    for (int ibuf = 0; ibuf < BUFFER_NUM; ibuf++)
    {
        s_receiveXfer[ibuf].data     = &s_buffer[ibuf * RECORD_BUFFER_SIZE];
        s_receiveXfer[ibuf].dataSize = RECORD_BUFFER_SIZE;
        int ibuf_next                = ibuf + 1;
        if (ibuf_next == BUFFER_NUM)
            ibuf_next = 0;
        s_receiveXfer[ibuf].linkTransfer = &s_receiveXfer[ibuf_next];
    }

    pcmHandle.isFirstRx      = 1;
    pcmHandle.isFirstTx      = 1;
    PDM_started              = 0;
    SAI_started              = 0;
    streamer_buff_addr.count = 0;
    streamer_buff_addr.head  = 0;
    streamer_buff_addr.tail  = 0;
    rx_data_valid            = 0;
    tx_data_valid            = 0;
    s_readIndex              = 0;

    PDM_TransferReceiveEDMA(DEMO_PDM, &(pcmHandle.pdmRxHandle), s_receiveXfer);
    PDM_EnableDMA(DEMO_PDM, false);
}

int streamer_pcm_tx_open(uint32_t num_buffers)
{
    OSA_SemaphoreCreateBinary(pcmHandle.semaphoreTX);
    return 0;
}

int streamer_pcm_rx_open(uint32_t num_buffers)
{
    return 0;
}

void streamer_pcm_tx_close(void)
{
    /* Stop playback.  This will flush the SAI transmit buffers. */
    SAI_TransferTerminateSendEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle));
    OSA_SemaphoreDestroy(pcmHandle.semaphoreTX);
}

void streamer_pcm_rx_close(void)
{
    /* Stop playback.  This will flush the SAI transmit buffers. */
    PDM_TransferTerminateReceiveEDMA(DEMO_PDM, &(pcmHandle.pdmRxHandle));
    PDM_Deinit(DEMO_PDM);
}

int streamer_pcm_write(uint8_t *data, uint32_t size)
{
    if (pcmHandle.isFirstTx)
    {
        pcmHandle.isFirstTx = 0;
        // Discard the first data buffer - due to RX-TX synchronization.
        return 1;
    }

    /* Ensure write size is a multiple of 32, otherwise EDMA will assert
     * failure.  Round down for the last chunk of a file/stream. */
    pcmHandle.saiTx.dataSize = size - (size % ((pcmHandle.bit_width == 16) ? 32 : 64));
    pcmHandle.saiTx.data     = data;

    DCACHE_CleanByRange((uint32_t)(pcmHandle.saiTx.data), pcmHandle.saiTx.dataSize);

    if (SAI_started == 0)
    {
        // Start the SAI transmission
        SAI_TransferSendEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle), &(pcmHandle.saiTx));
        SAI_started = 1;
    }

    /* Start the consecutive transfer */
    while (SAI_TransferSendEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle), &(pcmHandle.saiTx)) == kStatus_SAI_QueueFull)
    {
        /* Wait for transfer to finish */
        if (OSA_SemaphoreWait(pcmHandle.semaphoreTX, osaWaitForever_c) != KOSA_StatusSuccess)
        {
            return -1;
        }
    }

    return 0;
}

int streamer_pcm_read(uint8_t *data, uint32_t size)
{
    int ret = 1;

    if (size != RECORD_BUFFER_SIZE)
    {
        return -1;
    }

    if (pcmHandle.isFirstRx)
    {
        // Do not start PDM transmission - due to RX-TX synchronization (VIT and Voiceseeker initialization takes too
        // long in the first cycle).
        pcmHandle.isFirstRx = 0;
    }
    else
    {
        if ((rx_data_valid > 0) && (tx_data_valid > 0))
        {
            rx_data_valid--;
            tx_data_valid--;
            /* Copy data from the DMIC buffer */
            memcpy(Streamer_buff_addr_Pull(), &s_buffer[s_readIndex], size);
            s_readIndex += size;
            if (s_readIndex >= BUFFER_SIZE)
                s_readIndex -= BUFFER_SIZE;

            /* Signal that data are already ready for processing */
            ret = 0;
        }
    }

    // Store the data buffer address in the queue
    if (Streamer_buff_addr_Push(data) != 0)
    {
        return -1;
    }

    /* Signal that data are not already ready for processing yet */
    return ret;
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
        SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, kSAI_Channel0Mask);
        config.bitClock.bclkSource = (sai_bclk_source_t)DEMO_SAI_CLOCK_SOURCE;
        config.masterSlave         = DEMO_SAI_MASTER_SLAVE;
        SAI_TransferTerminateSendEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle));
        SAI_TransferTxSetConfigEDMA(DEMO_SAI, &(pcmHandle.saiTxHandle), &config);
        /* set bit clock divider */
        SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                              (pcmHandle.num_channels == 1) ? 2 : pcmHandle.num_channels);
        /* Enable SAI transmit and FIFO error interrupts. */
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
    int channel;

    switch (pcmHandle.num_channels)
    {
        case 0:
            return 0;
        case 1:
            channel = kCODEC_PlayChannelHeadphoneLeft;
            break;
        case 2:
            channel = kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight;
            break;
        case 8:
            /* Intentional fall */
        default:
            channel = ~0U;
            break;
    }

    if (volume <= 0)
        CODEC_SetMute(&codecHandle, ~0U, true);
    else
        CODEC_SetVolume(&codecHandle, channel, volume > CODEC_VOLUME_MAX_VALUE ? CODEC_VOLUME_MAX_VALUE : volume);

    return 0;
}
