
/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "clock_config.h"
#include "board.h"
#include "fsl_adapter_audio.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if defined(CODEC_WM8962_ENABLE) || defined(CODEC_WM8904_ENABLE)
#define HFP_CODEC_DAC_VOLUME (85U) /* Range: 0 ~ 100 */
#elif defined(CODEC_WM8960_ENABLE)
#define HFP_CODEC_DAC_VOLUME  (100U) /* Range: 0 ~ 100 */
#else
#define HFP_CODEC_DAC_VOLUME  (100U) /* Range: 0 ~ 100 */
#endif

#define HFP_CODEC_HP_VOLUME  (70U)  /* Range: 0 ~ 100 */

/* Simple configuration */
#define SIMPLE_BIT_RATE 16000U
#define SIMPLE_BITS     16U
#define SIMPLE_DURATION 10U
#define SIMPLE_TX_CHANNELS 2U
#define SIMPLE_RX_CHANNELS 1U

#define CODEC_TX_SIMPLE_COUNT    (SIMPLE_BIT_RATE * SIMPLE_DURATION / 1000U)
#define CODEC_TX_BUFFER_SIZE     (SIMPLE_TX_CHANNELS * SIMPLE_BIT_RATE * SIMPLE_BITS * SIMPLE_DURATION / 8000U)
#define CODEC_TX_BUFFER_COUNT    (8U)

#define CODEC_RX_SIMPLE_COUNT    (SIMPLE_BIT_RATE * SIMPLE_DURATION / 1000U)
#define CODEC_RX_BUFFER_SIZE     (SIMPLE_RX_CHANNELS * SIMPLE_BIT_RATE * SIMPLE_BITS * SIMPLE_DURATION / 8000U)
#define CODEC_RX_BUFFER_COUNT    (8U)

typedef void (*codec_rx_callback_t)(uint8_t *rx_buffer);
typedef void (*codec_tx_callback_t)(void);

extern codec_config_t boardCodecScoConfig1;

extern hal_audio_config_t codecTxConfig;
extern hal_audio_config_t codecRxConfig;

static uint32_t src_clk_hz;

AT_NONCACHEABLE_SECTION_ALIGN(static HAL_AUDIO_HANDLE_DEFINE(codecRxHandle), 4);
AT_NONCACHEABLE_SECTION_ALIGN(static HAL_AUDIO_HANDLE_DEFINE(codecTxHandle), 4);

static codec_handle_t codec_handle;

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t codecRxBuffer[CODEC_RX_BUFFER_SIZE * CODEC_RX_BUFFER_COUNT], 4);
volatile uint32_t codecRxBufferPrimeIndex = 0;
volatile uint32_t codecRxBufferReadIndex = 0;
volatile uint32_t codecRxBufferNotifyIndex = 0;
volatile uint32_t codecRxBufferPrimeCount = 0;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t codecTxBuffer[CODEC_TX_BUFFER_SIZE * CODEC_TX_BUFFER_COUNT], 4);
volatile uint32_t codecTxBufferPrimeIndex = 0;
volatile uint32_t codecTxBufferWriteIndex = 0;
volatile uint32_t codecTxBufferPrimeCount = 0;

static OSA_SEMAPHORE_HANDLE_DEFINE(s_audioTaskSync);

static volatile uint8_t s_audioInitialized = 0u;

static void APP_CodecTxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);
static void APP_CodecRxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam);

static int BOARD_AudioDeinit(void);
extern uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);

volatile uint32_t s_txCount = 0;
volatile uint32_t s_rxCount = 0;

volatile uint32_t s_txToggle = 0;
volatile uint32_t s_rxToggle = 0;

volatile static uint32_t s_currentVol = HFP_CODEC_HP_VOLUME;

static codec_rx_callback_t rx_callback;
static codec_tx_callback_t tx_callback;

uint8_t *BOARD_GetRxReadBuffer(void);
static uint8_t *BOARD_GetRxNotifyBuffer(void);

volatile uint64_t tx_prime_count = 0;
volatile uint64_t tx_played_count = 0;
volatile uint64_t tx_queued_count = 0;

volatile uint64_t rx_new_count = 0;
volatile uint64_t rx_read_count = 0;

static void APP_CodecTxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    s_txToggle = 1000 - s_txToggle;
    s_txCount ++;
    if (codecTxBufferPrimeCount > 0)
    {
        codecTxBufferPrimeCount --;
    }
    tx_played_count += CODEC_TX_BUFFER_SIZE;
    OSA_SemaphorePost(s_audioTaskSync);
}

static void APP_CodecRxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    s_rxToggle = 1000 - s_rxToggle;
    s_rxCount ++;
    if (codecRxBufferPrimeCount > 0)
    {
        codecRxBufferPrimeCount --;
    }
    rx_new_count += CODEC_RX_BUFFER_SIZE;
    OSA_SemaphorePost(s_audioTaskSync);
}

int BOARD_AudioInit(uint32_t samplingRate, uint32_t bitWidth)
{
    if (s_audioInitialized > 1)
    {
        BOARD_AudioDeinit();
    }
    PRINTF("Init Audio SAI and CODEC, samplingRate :%d  bitWidth:%d \r\n", samplingRate, bitWidth);
    /* Enable clock */
    src_clk_hz = BOARD_SwitchAudioFreq(samplingRate);

    /* Audio streamer */
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
(defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
    codecTxConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
    codecTxConfig.srcClock_Hz = src_clk_hz;
#endif
    codecTxConfig.sampleRate_Hz     = samplingRate;
    codecTxConfig.frameSyncWidth    = kHAL_AudioFrameSyncWidthHalfFrame,
    codecTxConfig.frameSyncPolarity = kHAL_AudioBeginAtFallingEdge,
    codecTxConfig.lineChannels      = kHAL_AudioStereo;
    codecTxConfig.dataFormat        = kHAL_AudioDataFormatI2sClassic;
    HAL_AudioTxInit((hal_audio_handle_t)&codecTxHandle[0], &codecTxConfig);
    HAL_AudioTxInstallCallback((hal_audio_handle_t)&codecTxHandle[0], APP_CodecTxCallback, NULL);

#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
(defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
    codecRxConfig.srcClock_Hz = OVER_SAMPLE_RATE * samplingRate;
#else
    codecRxConfig.srcClock_Hz = src_clk_hz;
#endif
    codecRxConfig.sampleRate_Hz = samplingRate;
    HAL_AudioRxInit((hal_audio_handle_t)&codecRxHandle[0], &codecRxConfig);
    HAL_AudioRxInstallCallback((hal_audio_handle_t)&codecRxHandle[0], APP_CodecRxCallback, NULL);

    /* Codec */
    if (CODEC_Init(&codec_handle, &boardCodecScoConfig1) != kStatus_Success)
    {
        PRINTF("codec init failed!\r\n");
    }
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    CODEC_SetFormat(&codec_handle, codecTxConfig.srcClock_Hz, codecTxConfig.sampleRate_Hz, codecTxConfig.bitWidth);
    CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, HFP_CODEC_DAC_VOLUME);
    s_currentVol = HFP_CODEC_HP_VOLUME;
    PRINTF("Set default headphone volume %d\r\n", s_currentVol);
    CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, s_currentVol);
    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);

    s_audioInitialized = 1U;

    return 0;
}

static int BOARD_AudioDeinit(void)
{
    if (s_audioInitialized > 0)
    {
        s_audioInitialized = 0;

        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        HAL_AudioTxDeinit((hal_audio_handle_t)&codecTxHandle[0]);
        HAL_AudioRxDeinit((hal_audio_handle_t)&codecRxHandle[0]);
        (void)BOARD_SwitchAudioFreq(0U);

        codecTxBufferPrimeCount = 0;
        codecTxBufferPrimeIndex = 0;
        codecTxBufferWriteIndex = 0;
        codecRxBufferPrimeCount = 0;
        codecRxBufferPrimeIndex = 0;
        codecRxBufferReadIndex = 0;
        codecRxBufferNotifyIndex = 0;

        tx_prime_count = 0;
        tx_played_count = 0;
        tx_queued_count = 0;
    }

    return 0;
}

static void BOARD_CodecTask(void* param)
{
    hal_audio_transfer_t xfer;
    uint8_t need2Callback = 0U;
    OSA_SR_ALLOC();

    while (true)
    {
        OSA_SemaphoreWait(s_audioTaskSync, osaWaitForever_c);

        OSA_ENTER_CRITICAL();
        if (s_audioInitialized < 1)
        {
            OSA_EXIT_CRITICAL();
            continue;
        }
        OSA_EXIT_CRITICAL();

        OSA_ENTER_CRITICAL();
        if (codecTxBufferPrimeCount < CODEC_TX_BUFFER_COUNT)
        {
            xfer.data     = &codecTxBuffer[codecTxBufferPrimeIndex * CODEC_TX_BUFFER_SIZE];
            xfer.dataSize = CODEC_TX_BUFFER_SIZE;
            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&codecTxHandle[0], &xfer))
            {
                tx_queued_count += CODEC_TX_BUFFER_SIZE;
                codecTxBufferPrimeIndex ++;
            }
            if (codecTxBufferPrimeIndex >= CODEC_TX_BUFFER_COUNT)
            {
                codecTxBufferPrimeIndex = 0U;
            }
            codecTxBufferPrimeCount ++;
            need2Callback = 1U;
        }
        OSA_EXIT_CRITICAL();

        if (need2Callback > 0U)
        {
            need2Callback = 0U;
            if (NULL != tx_callback)
            {
                tx_callback();
            }
        }

        OSA_ENTER_CRITICAL();
        if (codecRxBufferPrimeCount < CODEC_RX_BUFFER_COUNT)
        {
            xfer.data     = codecRxBuffer + codecRxBufferPrimeIndex * CODEC_RX_BUFFER_SIZE;
            xfer.dataSize = CODEC_RX_BUFFER_SIZE;

            if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&codecRxHandle[0], &xfer))
            {
                codecRxBufferPrimeIndex++;
            }
            if (codecRxBufferPrimeIndex >= CODEC_RX_BUFFER_COUNT)
            {
                codecRxBufferPrimeIndex = 0U;
            }
            codecRxBufferPrimeCount ++;
            need2Callback = 1U;
        }
        OSA_EXIT_CRITICAL();

        if (need2Callback > 0U)
        {
            need2Callback = 0U;
            if (NULL != rx_callback)
            {
                rx_callback(BOARD_GetRxNotifyBuffer());
            }
        }
    }
}

static int BOARD_AudioTaskInit(void)
{
    static uint8_t s_audioStarted = 0u;
    BaseType_t result = 0;
    osa_status_t ret = KOSA_StatusSuccess;

    if (0u == s_audioStarted)
    {
        s_audioStarted = 1U;

        ret = OSA_SemaphoreCreate(s_audioTaskSync, 0);
        assert(KOSA_StatusSuccess == result);

        result = xTaskCreate(BOARD_CodecTask, "BOARD_CodecTask", 1024, NULL, 5, NULL);
        assert(pdPASS == result);

    }
    codecTxBufferPrimeCount = 0;
    codecTxBufferPrimeIndex = 0;
    codecTxBufferWriteIndex = 0;
    codecRxBufferPrimeCount = 0;
    codecRxBufferPrimeIndex = 0;
    codecRxBufferReadIndex = 0;
    codecRxBufferNotifyIndex = 0;

    tx_prime_count = 0;
    tx_played_count = 0;
    tx_queued_count = 0;

    rx_new_count = 0;
    rx_read_count = 0;

    (void)result;
    (void)ret;

    return 0;
}

void BOARD_PrimeTxWriteBuffer(const uint8_t * buffer, uint32_t length)
{
    uint32_t prime_length = 0;

    if ((length + codecTxBufferWriteIndex) >= sizeof(codecTxBuffer))
    {
        prime_length = sizeof(codecTxBuffer) - codecTxBufferWriteIndex;
    }
    else
    {
        prime_length = length;
    }
    if (buffer != NULL)
    {
        memcpy(&codecTxBuffer[codecTxBufferWriteIndex], buffer, prime_length);
    }
    else
    {
        memset(&codecTxBuffer[codecTxBufferWriteIndex], 0, prime_length);
    }
    codecTxBufferWriteIndex = codecTxBufferWriteIndex + prime_length;

    if (prime_length < length)
    {
        if (buffer != NULL)
        {
            buffer = buffer + prime_length;
        }

        prime_length = length - prime_length;

        if (buffer != NULL)
        {
            memcpy(&codecTxBuffer[0], buffer, prime_length);
        }
        else
        {
            memset(&codecTxBuffer[0], 0, prime_length);
        }
        codecTxBufferWriteIndex = prime_length;
    }
    tx_prime_count += length;
}

void BOARD_StartStream(void)
{
    hal_audio_transfer_t xfer;
    OSA_SR_ALLOC();

    OSA_ENTER_CRITICAL();
    for (uint8_t index = 0; index < (CODEC_TX_BUFFER_COUNT - codecTxBufferPrimeCount); ++index)
    {
        xfer.data     = &codecTxBuffer[codecTxBufferPrimeIndex * CODEC_TX_BUFFER_SIZE];
        xfer.dataSize = CODEC_TX_BUFFER_SIZE;
        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&codecTxHandle[0], &xfer))
        {
            tx_queued_count += CODEC_TX_BUFFER_SIZE;
            codecTxBufferPrimeIndex ++;
        }
        if (codecTxBufferPrimeIndex >= CODEC_TX_BUFFER_COUNT)
        {
            codecTxBufferPrimeIndex = 0U;
        }
        codecTxBufferPrimeCount ++;
    }

    for (uint8_t index = 0; index < (CODEC_RX_BUFFER_COUNT - codecRxBufferPrimeCount); ++index)
    {
        xfer.data     = codecRxBuffer + codecRxBufferPrimeIndex * CODEC_RX_BUFFER_SIZE;
        xfer.dataSize = CODEC_RX_BUFFER_SIZE;

        if (kStatus_HAL_AudioSuccess == HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&codecRxHandle[0], &xfer))
        {
            codecRxBufferPrimeIndex++;
        }
        if (codecRxBufferPrimeIndex >= CODEC_RX_BUFFER_COUNT)
        {
            codecRxBufferPrimeIndex = 0U;
        }
        codecRxBufferPrimeCount++;
    }

    OSA_EXIT_CRITICAL();
}

uint8_t *BOARD_GetRxReadBuffer(void)
{
    uint8_t * buffer = &codecRxBuffer[codecRxBufferReadIndex * CODEC_RX_BUFFER_SIZE];

    codecRxBufferReadIndex ++;

    if (codecRxBufferReadIndex >= CODEC_RX_BUFFER_COUNT)
    {
        codecRxBufferReadIndex = 0U;
    }
    rx_read_count += CODEC_RX_BUFFER_SIZE;

    return buffer;
}

static uint8_t *BOARD_GetRxNotifyBuffer(void)
{
    uint8_t * buffer = &codecRxBuffer[codecRxBufferNotifyIndex * CODEC_RX_BUFFER_SIZE];

    codecRxBufferNotifyIndex ++;

    if (codecRxBufferNotifyIndex >= CODEC_RX_BUFFER_COUNT)
    {
        codecRxBufferNotifyIndex = 0U;
    }
    return buffer;
}

int BOARD_StartCodec(codec_tx_callback_t tx_cb, codec_rx_callback_t rx_cb, uint32_t simpleBitRate, uint32_t simpleBits)
{
    BOARD_AudioDeinit();

    BOARD_AudioInit(simpleBitRate, simpleBits);

    memset(codecTxBuffer, 0, sizeof(codecTxBuffer));
    memset(codecRxBuffer, 0, sizeof(codecRxBuffer));

    BOARD_AudioTaskInit();

    rx_callback = rx_cb;
    tx_callback = tx_cb;

    return 0;
}

int BOARD_StopCodec(void)
{
    BOARD_AudioDeinit();

    return 0;
}

int BOARD_VolSet(uint32_t vol)
{
    if (s_audioInitialized < 1)
    {
        return -1;
    }

    if (vol > 100)
    {
        return -2;
    }

    s_currentVol = vol;
    CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, s_currentVol);
    PRINTF("Current VOL is %d\r\n", s_currentVol);

    return 0;
}

int BOARD_VolUp(void)
{
    if (s_audioInitialized < 1)
    {
        return -1;
    }

    if (s_currentVol <= 90)
    {
        s_currentVol = s_currentVol + 10;
    }
    else
    {
        s_currentVol = 100;
    }

    CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, s_currentVol);
    PRINTF("Current VOL is %d\r\n", s_currentVol);

    return 0;
}

int BOARD_VolDown(void)
{
    if (s_audioInitialized < 1)
    {
        return -1;
    }

    if (s_currentVol >= 10)
    {
        s_currentVol = s_currentVol - 10;
    }
    else
    {
        s_currentVol = 0;
    }

    CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, s_currentVol);
    PRINTF("Current VOL is %d\r\n", s_currentVol);

    return 0;
}

int BOARD_SpeakerMute(void)
{
    if (s_audioInitialized < 1)
    {
        return -1;
    }

    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
    PRINTF("Speaker mute\r\n");

    return 0;
}

int BOARD_SpeakerUnmute(void)
{
    if (s_audioInitialized < 1)
    {
        return -1;
    }

    CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
    PRINTF("Speaker unmute\r\n");

    return 0;
}