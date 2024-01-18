/*
 * Copyright 2018-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_speaker.h"
#include "fsl_codec_adapter.h"
#include "fsl_debug_console.h"
#include "fsl_sai.h"
#include "pin_mux.h"
#include "clock_config.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Speaker instance. */
static EIQ_Speaker_t speaker;
/* Sai handler. */
AT_NONCACHEABLE_SECTION_INIT(static sai_edma_handle_t s_speakerHandle);
/* Ready callback. */
static EIQ_IUpdater_t readyCallback = NULL;
/* Sai data transfer structre. */
static sai_transfer_t xfer = {NULL, 0};
static edma_handle_t dmaHandle = {0};

/*!
 * @brief Starts transfer to the speaker.
 *
 * This function sends first frame buffer to device. BufferReleased is called
 * after frame buffer had been sent.
 */
static void start(void)
{
    SAI_TxSoftwareReset(DEMO_SAI, kSAI_ResetTypeSoftware);

    xfer.dataSize = BUFFER_SIZE;
}

/*!
 * @brief Gets speaker data dimensions.
 *
 * This function gets dimensions of the speaker.
 * width for BUFFER_NUM and height for BUFFER_SIZE
 *
 * @return Display dimensions.
 */
static Dims_t getResolution(void)
{
    Dims_t dims;
    dims.width = BUFFER_NUM;
    dims.height = BUFFER_SIZE;
    return dims;
}

/*!
 * @brief Notifies speaker.
 *
 * This function notifies speaker driver that data in buffer are ready
 * and buffer could be sent.
 */
static void notify(void)
{
    if (xfer.data == NULL)
    {
        PRINTF("SAI_Tx buffer is undefined.\r\n");
    }

    if (SAI_TransferSendEDMA(DEMO_SAI, &s_speakerHandle, &xfer) != kStatus_Success)
    {
        PRINTF("SAI_Tx failed!\r\n");
    }
}

/*!
 * @brief Sets speaker buffer address.
 *
 * @param buffAddr buffer address for sending data.
 */
static void setBuffer(uint32_t buffAddr)
{
    xfer.data = (uint8_t*)buffAddr;
}

/*!
 * @brief Sets ready callback.
 *
 * This function sets external callback which is called when
 * speaker buffer is empty and can be overwrite by new data.
 *
 * @param updater callback.
 */
static void setReadyCallback(EIQ_IUpdater_t updater)
{
    readyCallback = updater;
}

/*!
 * @brief TX callback.
 *
 * @param pointer to I2S base address
 * @param pointer to sai edma handler
 * @param status status code
 * @param pointer to user data
 */
static void callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_TxError == status)
    {
        PRINTF("SAI_Tx failed!\r\n");
        return;
    }

    if (readyCallback != NULL)
    {
        readyCallback();
    }
}

/*!
 * @brief Initializes speaker.
 */
static void init(void)
{
    sai_transceiver_t saiConfig;

    /* Clear TCSR interrupt flags. */
    BOARD_ClearTxInterruptFlags();
    /* Init DMAMUX */
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_TX_EDMA_CHANNEL, (uint8_t)DEMO_SAI_TX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_TX_EDMA_CHANNEL);

    EDMA_CreateHandle(&dmaHandle, DEMO_DMA, DEMO_TX_EDMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_TX_EDMA_CHANNEL, DEMO_SAI_TX_EDMA_CHANNEL);
#endif

    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &s_speakerHandle, callback, NULL, &dmaHandle);

    SAI_GetClassicI2SConfig(&saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_MonoRight, kSAI_Channel0Mask);
    saiConfig.syncMode = kSAI_ModeAsync;
    saiConfig.bitClock.bclkPolarity = kSAI_PolarityActiveLow;
    saiConfig.masterSlave = kSAI_Master;
    SAI_TransferTxSetConfigEDMA(DEMO_SAI, &s_speakerHandle, &saiConfig);

    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
}

/*!
 * @brief Initializes speaker.
 *
 * This function initializes speaker.
 *
 * @return pointer to initialized speaker instance
 */
EIQ_Speaker_t* EIQ_SpeakerInit(void)
{
    speaker.base.getResolution = getResolution;
    speaker.base.notify = notify;
    speaker.base.start = start;
    speaker.setReadyCallback = setReadyCallback;
    speaker.setBuffer = setBuffer;

    init();

    return &speaker;
}
