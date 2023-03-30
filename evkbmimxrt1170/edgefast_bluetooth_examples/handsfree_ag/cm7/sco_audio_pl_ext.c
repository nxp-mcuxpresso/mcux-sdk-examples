
/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sco_audio_pl.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_debug_console.h"
#include "ringtone.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Sco loop back, data from sco input to sco output */
#define SCO_SAI_LOOPBACK (0)
/* Codec loop back data from mic to speaker */
#define CODEC_SAI_LOOPBACK (0)

#define OVER_SAMPLE_RATE        (256U)

#define BUFFER_SIZE      (1024U)
#define BUFFER_NUMBER    (4U)
#define AUDIO_DUMMY_SIZE (64U)
#define HFP_STREAMER_TASK_PRIORITY (6U)

#define HFP_CODEC_DAC_VOLUME (100U) /* Range: 0 ~ 100 */
#define HFP_CODEC_HP_VOLUME  (70U)  /* Range: 0 ~ 100 */

/* --------------------------------------------- External Global Variables */

extern codec_config_t boardCodecScoConfig;
extern codec_config_t boardCodecScoConfig1;

extern hal_audio_config_t txSpeakerConfig;
extern hal_audio_config_t rxMicConfig;
extern hal_audio_config_t txMicConfig;
extern hal_audio_config_t rxSpeakerConfig;

/* --------------------------------------------- Exported Global Variables */

/* --------------------------------------------- Static Global Variables */

extern uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);

static HAL_AUDIO_HANDLE_DEFINE(tx_speaker_handle);
static HAL_AUDIO_HANDLE_DEFINE(rx_mic_handle);
static HAL_AUDIO_HANDLE_DEFINE(tx_mic_handle);
static HAL_AUDIO_HANDLE_DEFINE(rx_speaker_handle);

static codec_handle_t codec_handle;

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t MicBuffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t SpeakerBuffer[BUFFER_NUMBER * BUFFER_SIZE], 4);
AT_NONCACHEABLE_SECTION(uint32_t g_AudioTxDummyBuffer[AUDIO_DUMMY_SIZE / 4U]);

OSA_SEMAPHORE_HANDLE_DEFINE(xSemaphoreScoAudio);

static uint32_t txMic_index = 0U, rxMic_index = 0U;
volatile uint32_t emptyMicBlock = BUFFER_NUMBER;
static uint32_t txSpeaker_index = 0U, rxSpeaker_index = 0U;
volatile uint32_t emptySpeakerBlock = BUFFER_NUMBER;
static uint32_t rxSpeaker_test = 0U, rxMic_test = 0U;
static volatile uint8_t s_ringTone = 0;
static uint32_t cpy_index = 0U, tx_index = 0U;
static volatile uint8_t sco_audio_setup = 0;
static SCO_AUDIO_EP_INFO s_ep_info;
static API_RESULT audio_setup_pl_ext(uint8_t isRing, SCO_AUDIO_EP_INFO *ep_info);
API_RESULT sco_audio_start_pl_ext(void);

/* --------------------------------------------- Functions */
/* overwrite sco_audio_init_pl_ext of sco_audio_pl.c.
 * The follow functions can be overwritten too
 * if the actual example need implement them to
 * use different audio data.
 * sco_audio_init_pl_ext, sco_audio_shutdown_pl_ext,
 * sco_audio_setup_pl_ext, sco_audio_start_pl_ext,
 * sco_audio_stop_pl_ext, sco_audio_write_pl_ext.
 */

static void rxMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
        emptyMicBlock--;
        rxMic_test++;
        OSA_SemaphorePost(xSemaphoreScoAudio);
    }
}

#if SCO_SAI_LOOPBACK
static void txMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
#if SCO_SAI_LOOPBACK
        emptySpeakerBlock++;
#else
        emptyMicBlock++;
#endif
        rxMic_test++;
        OSA_SemaphorePost(xSemaphoreScoAudio);
    }
}
#else
static void txMicCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    static volatile uint8_t s_8978ConsumerActualData = 0;
    hal_audio_transfer_t xfer;
    if (s_ringTone == 1U)
    {
        s_8978ConsumerActualData = 0;
        xfer.dataSize            = AUDIO_DUMMY_SIZE;
        xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
        HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
    }
    else
    {
        if (s_8978ConsumerActualData)
        {
            s_8978ConsumerActualData = 0;
            emptyMicBlock++;
            OSA_SemaphorePost(xSemaphoreScoAudio);

            if (emptyMicBlock < (BUFFER_NUMBER))
            {
                s_8978ConsumerActualData = 1;
                xfer.data                = MicBuffer + txMic_index * BUFFER_SIZE;
                xfer.dataSize            = BUFFER_SIZE;

                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
                {
                    txMic_index++;
                }
                if (txMic_index == BUFFER_NUMBER)
                {
                    txMic_index = 0U;
                }
            }
            else
            {
                printf("mic dummy\r\n");
                s_8978ConsumerActualData = 0;
                xfer.dataSize            = AUDIO_DUMMY_SIZE;
                xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
            }
        }
        else
        {
            if (emptyMicBlock < (BUFFER_NUMBER - 2))
            {
                s_8978ConsumerActualData = 1;
                xfer.data                = MicBuffer + txMic_index * BUFFER_SIZE;
                xfer.dataSize            = BUFFER_SIZE;

                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
                {
                    txMic_index++;
                }
                if (txMic_index == BUFFER_NUMBER)
                {
                    txMic_index = 0U;
                }
            }
            else
            {
                printf("mic dummy\r\n");
                s_8978ConsumerActualData = 0;
                xfer.dataSize            = AUDIO_DUMMY_SIZE;
                xfer.data                = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);
            }
        }
    }
}
#endif
static void txSpeakerCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    static volatile uint8_t s_consumerActualData = 0;
    hal_audio_transfer_t xfer;
    if (s_ringTone == 1U)
    {
        if ((emptySpeakerBlock > 0U) && (cpy_index < MUSIC_LEN / BUFFER_SIZE))
        {
            /* Fill in the buffers. */
            memcpy((uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (cpy_index % BUFFER_NUMBER)],
                   (uint8_t *)&music[cpy_index * BUFFER_SIZE], sizeof(uint8_t) * BUFFER_SIZE);
            emptySpeakerBlock--;
            cpy_index++;
        }

        if (emptySpeakerBlock < BUFFER_NUMBER)
        {
            /*  xfer structure */
            xfer.data     = (uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (tx_index % BUFFER_NUMBER)];
            xfer.dataSize = BUFFER_SIZE;
            /* Wait for available queue. */
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
            {
                tx_index++;
            }
            emptySpeakerBlock++;
        }
    }
    else
    {
        if (s_consumerActualData)
        {
            s_consumerActualData = 0;
            emptySpeakerBlock++;
            OSA_SemaphorePost(xSemaphoreScoAudio);

            if (emptySpeakerBlock < (BUFFER_NUMBER))
            {
                s_consumerActualData = 1;
                xfer.data            = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
                xfer.dataSize        = BUFFER_SIZE;

                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
                {
                    txSpeaker_index++;
                }
                if (txSpeaker_index == BUFFER_NUMBER)
                {
                    txSpeaker_index = 0U;
                }
            }
            else
            {
                s_consumerActualData = 0;
                xfer.dataSize        = AUDIO_DUMMY_SIZE;
                xfer.data            = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);
            }
        }
        else
        {
            if (emptySpeakerBlock < (BUFFER_NUMBER - 2))
            {
                s_consumerActualData = 1;
                xfer.data            = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
                xfer.dataSize        = BUFFER_SIZE;
                if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
                {
                    txSpeaker_index++;
                }
                if (txSpeaker_index == BUFFER_NUMBER)
                {
                    txSpeaker_index = 0U;
                }
            }
            else
            {
                s_consumerActualData = 0;
                xfer.dataSize        = AUDIO_DUMMY_SIZE;
                xfer.data            = (uint8_t *)&g_AudioTxDummyBuffer[0];
                HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);
            }
        }
    }
}

static void rxSpeakerCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    if (kStatus_HAL_AudioError == completionStatus)
    {
        /* Handle the error. */
    }
    else
    {
        emptySpeakerBlock--;
        OSA_SemaphorePost(xSemaphoreScoAudio);
        rxSpeaker_test++;
    }
}

static void Deinit_Board_Audio(void)
{
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    HAL_AudioTxDeinit((hal_audio_handle_t)&tx_speaker_handle[0]);
    HAL_AudioRxDeinit((hal_audio_handle_t)&rx_mic_handle[0]);
    HAL_AudioTxDeinit((hal_audio_handle_t)&tx_mic_handle[0]);
    HAL_AudioRxDeinit((hal_audio_handle_t)&rx_speaker_handle[0]);
    (void)BOARD_SwitchAudioFreq(0U);
}

/*Initialize sco audio interface and codec.*/
static void Init_Board_Sco_Audio(uint32_t samplingRate, UCHAR bitWidth)
{
    uint32_t src_clk_hz;

    if (samplingRate > 0U)
    {
        PRINTF("Init Audio SCO SAI and CODEC samplingRate :%d  bitWidth:%d \r\n", samplingRate, bitWidth);

        /* Enable clock */
        src_clk_hz = BOARD_SwitchAudioFreq(samplingRate);

        /* Audio streamer */
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        txSpeakerConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        txSpeakerConfig.srcClock_Hz = src_clk_hz;
#endif
        txSpeakerConfig.sampleRate_Hz     = samplingRate;
        txSpeakerConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
        txSpeakerConfig.frameSyncPolarity = kHAL_AudioBeginAtRisingEdge,
        txSpeakerConfig.lineChannels      = kHAL_AudioMonoLeft;
        txSpeakerConfig.dataFormat        = kHAL_AudioDataFormatDspModeB;
        HAL_AudioTxInit((hal_audio_handle_t)&tx_speaker_handle[0], &txSpeakerConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_speaker_handle[0], txSpeakerCallback, NULL);

#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        rxMicConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        rxMicConfig.srcClock_Hz = src_clk_hz;
#endif
        rxMicConfig.sampleRate_Hz = samplingRate;
        HAL_AudioRxInit((hal_audio_handle_t)&rx_mic_handle[0], &rxMicConfig);
        HAL_AudioRxInstallCallback((hal_audio_handle_t)&rx_mic_handle[0], rxMicCallback, NULL);

        txMicConfig.srcClock_Hz   = src_clk_hz;
        txMicConfig.sampleRate_Hz = samplingRate;
        HAL_AudioTxInit((hal_audio_handle_t)&tx_mic_handle[0], &txMicConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_mic_handle[0], txMicCallback, NULL);

        rxSpeakerConfig.srcClock_Hz   = src_clk_hz;
        rxSpeakerConfig.sampleRate_Hz = samplingRate;
        HAL_AudioRxInit((hal_audio_handle_t)&rx_speaker_handle[0], &rxSpeakerConfig);
        HAL_AudioRxInstallCallback((hal_audio_handle_t)&rx_speaker_handle[0], rxSpeakerCallback, NULL);

        /* Codec */
        if (CODEC_Init(&codec_handle, &boardCodecScoConfig) != kStatus_Success)
        {
            PRINTF("codec init failed!\r\n");
        }
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        CODEC_SetFormat(&codec_handle, txSpeakerConfig.srcClock_Hz, txSpeakerConfig.sampleRate_Hz, txSpeakerConfig.bitWidth);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, HFP_CODEC_DAC_VOLUME);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, HFP_CODEC_HP_VOLUME);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
    }
}
static void Init_Board_RingTone_Audio(uint32_t samplingRate, UCHAR bitWidth)
{
    uint32_t src_clk_hz;

    if (samplingRate > 0U)
    {
        PRINTF("Init Audio CODEC for RingTone\r\n");

        /* Enable clock */
        src_clk_hz = BOARD_SwitchAudioFreq(samplingRate);

        /* Audio streamer */
        txSpeakerConfig.sampleRate_Hz     = samplingRate;
        txSpeakerConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
        txSpeakerConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
        txSpeakerConfig.lineChannels      = kHAL_AudioStereo;
        txSpeakerConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
        txSpeakerConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
        txSpeakerConfig.srcClock_Hz = src_clk_hz;
#endif
        HAL_AudioTxInit((hal_audio_handle_t)&tx_speaker_handle[0], &txSpeakerConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&tx_speaker_handle[0], txSpeakerCallback, NULL);

        CODEC_Init(&codec_handle, &boardCodecScoConfig1);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        CODEC_SetFormat(&codec_handle, txSpeakerConfig.srcClock_Hz, txSpeakerConfig.sampleRate_Hz, txSpeakerConfig.bitWidth);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, HFP_CODEC_DAC_VOLUME);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, HFP_CODEC_HP_VOLUME);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
    }
}

static API_RESULT audio_setup_pl_ext(uint8_t isRing, SCO_AUDIO_EP_INFO *ep_info)
{
    txMic_index     = 0U;
    rxMic_index     = 0U;
    emptyMicBlock   = BUFFER_NUMBER;
    txSpeaker_index = 0U, rxSpeaker_index = 0U;
    emptySpeakerBlock = BUFFER_NUMBER;
    if (isRing)
    {
        Init_Board_RingTone_Audio(ep_info->sampl_freq, ep_info->sample_len);
    }
    else
    {
        Init_Board_Sco_Audio(ep_info->sampl_freq, ep_info->sample_len);
    }
    return API_SUCCESS;
}

static uint32_t count = 0;

void SCO_Edma_Task(void *handle)
{
    hal_audio_transfer_t xfer;

    while (1)
    {
        OSA_SemaphoreWait(xSemaphoreScoAudio, osaWaitForever_c);
        count++;
#ifdef SCO_DEBUG_MSG
        if (count % 300 == 0)
        {
            PRINTF("@(%d  %d)", emptyMicBlock, emptySpeakerBlock);
            PRINTF("#( %d %d)", rxSpeaker_test, rxMic_test);
        }
#endif
#if SCO_SAI_LOOPBACK
        if (emptySpeakerBlock > 0)
        {
            xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
            {
                rxSpeaker_index++;
            }
            if (rxSpeaker_index == BUFFER_NUMBER)
            {
                rxSpeaker_index = 0U;
            }
        }
        if (emptySpeakerBlock < BUFFER_NUMBER)
        {
            xfer.data     = SpeakerBuffer + txSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer))
            {
                txSpeaker_index++;
            }
            if (txSpeaker_index == BUFFER_NUMBER)
            {
                txSpeaker_index = 0U;
            }
        }
#else
        if (emptyMicBlock > 0U)
        {
            xfer.data     = MicBuffer + rxMic_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_mic_handle[0], &xfer))
            {
                rxMic_index++;
            }
            if (rxMic_index == BUFFER_NUMBER)
            {
                rxMic_index = 0U;
            }
        }
#if CODEC_SAI_LOOPBACK
        if (emptyMicBlock < BUFFER_NUMBER)
        {
            xfer.data     = MicBuffer + txMic_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
            {
                txMic_index++;
            }
            if (txMic_index == BUFFER_NUMBER)
            {
                txMic_index = 0U;
            }
        }
#else
        if (emptySpeakerBlock > 0U)
        {
            xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
            xfer.dataSize = BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
            {
                rxSpeaker_index++;
            }
            if (rxSpeaker_index == BUFFER_NUMBER)
            {
                rxSpeaker_index = 0U;
            }
        }

#endif
#endif
    }
}
void sco_audio_init_pl_ext(void)
{
    return;
}

void sco_audio_shutdown_pl_ext(void)
{
    return;
}

API_RESULT sco_audio_setup_pl_ext(SCO_AUDIO_EP_INFO *ep_info)
{
    sco_audio_setup = 1;
    if (s_ringTone == 0U)
    {
        audio_setup_pl_ext(false, ep_info);
    }
    memcpy(&s_ep_info, ep_info, sizeof(SCO_AUDIO_EP_INFO));
    return API_SUCCESS;
}

API_RESULT sco_audio_start_pl_ext(void)
{
    static uint32_t taskCreated = 0;
    hal_audio_transfer_t xfer;
    BaseType_t result = 0;
    if (s_ringTone == 1U)
    {
        return API_SUCCESS;
    }
    if (taskCreated == 0)
    {
        OSA_SemaphoreCreate(xSemaphoreScoAudio, 0);
        result = xTaskCreate(SCO_Edma_Task, "SCO_Edma", 1024, NULL, HFP_STREAMER_TASK_PRIORITY, NULL);
        assert(pdPASS == result);
        taskCreated = 1U;
        (void)result;
    }
    for (uint8_t index = 0; index < BUFFER_NUMBER; ++index)
    {
        xfer.data     = MicBuffer + rxMic_index * BUFFER_SIZE;
        xfer.dataSize = BUFFER_SIZE;

        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_mic_handle[0], &xfer))
        {
            rxMic_index++;
        }
        if (rxMic_index == BUFFER_NUMBER)
        {
            rxMic_index = 0U;
        }
    }
    emptyMicBlock = BUFFER_NUMBER;

    for (uint8_t index = 0; index < BUFFER_NUMBER; ++index)
    {
        xfer.data     = SpeakerBuffer + rxSpeaker_index * BUFFER_SIZE;
        xfer.dataSize = BUFFER_SIZE;

        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&rx_speaker_handle[0], &xfer))
        {
            rxSpeaker_index++;
        }
        if (rxSpeaker_index == BUFFER_NUMBER)
        {
            rxSpeaker_index = 0U;
        }
    }
    emptySpeakerBlock = BUFFER_NUMBER;
    //BOARD_SCO_EnableSaiMclkOutput(true);

    /* play dummy data to codec */
    xfer.dataSize = AUDIO_DUMMY_SIZE;
    xfer.data     = (uint8_t *)&g_AudioTxDummyBuffer[0];
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer);

    /* play dummy data to 8978 */
    xfer.dataSize = AUDIO_DUMMY_SIZE;
    xfer.data     = (uint8_t *)&g_AudioTxDummyBuffer[0];
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_mic_handle[0], &xfer);

    return API_SUCCESS;
}

API_RESULT sco_audio_stop_pl_ext(void)
{
    Deinit_Board_Audio();
    sco_audio_setup = 0;
    EM_usleep(200U * 1000U);
    return API_SUCCESS;
}

#ifdef HCI_SCO
void sco_audio_spkr_play_pl_ext(UCHAR *m_data, UINT16 m_datalen)
{
    /* Write to Codec */
}
#endif /* HCI_SCO */
API_RESULT platform_audio_play_ringtone()
{
    cpy_index = 0;
    tx_index  = 0U;
    hal_audio_transfer_t xfer;
    emptySpeakerBlock = BUFFER_NUMBER;
    if (s_ringTone == 0)
    {
        if (sco_audio_setup == 1)
        {
            Deinit_Board_Audio();
        }

        SCO_AUDIO_EP_INFO ep_info;
        ep_info.sampl_freq = 16000U;
        ep_info.sample_len = 16U;
        audio_setup_pl_ext(true, &ep_info);
        s_ringTone = 1;
    }

    memset(SpeakerBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);
    if ((emptySpeakerBlock > 0U) && (cpy_index < MUSIC_LEN / BUFFER_SIZE))
    {
        /* Fill in the buffers. */
        memcpy((uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (cpy_index % BUFFER_NUMBER)],
               (uint8_t *)&music[cpy_index * BUFFER_SIZE], sizeof(uint8_t) * BUFFER_SIZE);
        emptySpeakerBlock--;
        cpy_index++;
    }
    if (emptySpeakerBlock < BUFFER_NUMBER)
    {
        /*  xfer structure */
        xfer.data     = (uint8_t *)&SpeakerBuffer[BUFFER_SIZE * (tx_index % BUFFER_NUMBER)];
        xfer.dataSize = BUFFER_SIZE;
        /* Wait for available queue. */
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&tx_speaker_handle[0], &xfer))
        {
            tx_index++;
        }
        emptySpeakerBlock++;
    }
    return API_SUCCESS;
}

API_RESULT sco_audio_set_speaker_volume(UCHAR volume)
{
    if (sco_audio_setup == 0)
    {
        return API_FAILURE;
    }
    /* HFP support 0- 15, codec support 0-100*/
    if (kStatus_Success == CODEC_SetVolume(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, ((volume * 6U) + 9U)))
    {
        return API_SUCCESS;
    }

    return API_FAILURE;
}
void sco_audio_play_ringtone_pl_ext(void)
{
    platform_audio_play_ringtone();
}
void sco_audio_play_ringtone_exit_pl_ext(void)
{
    if (s_ringTone == 1)
    {
        Deinit_Board_Audio();
        memset(SpeakerBuffer, 0x0, BUFFER_NUMBER * BUFFER_SIZE);
        s_ringTone = 0U;
        if (sco_audio_setup == 1)
        {
            audio_setup_pl_ext(false, &s_ep_info);
            sco_audio_start_pl_ext();
        }
    }
}
