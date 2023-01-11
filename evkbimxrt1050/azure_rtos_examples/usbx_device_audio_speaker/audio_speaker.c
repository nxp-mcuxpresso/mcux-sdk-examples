/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "pin_mux.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_adapter_audio.h"
#include "fsl_debug_console.h"
#include "usb.h"
#include "audio_speaker.h"

#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100
#endif

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
hal_audio_config_t audioTxConfig;
hal_audio_dma_config_t dmaTxConfig;
hal_audio_dma_mux_config_t dmaMuxTxConfig;
hal_audio_ip_config_t ipTxConfig;

uint32_t audio_tx_resync;
uint32_t rxindex;
uint32_t txindex;
volatile uint32_t sendCount;
volatile uint32_t receiveCount;
volatile long audio_transfer_byte_count;

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioTxBuff[BUFFER_SIZE * BUFFER_NUM];

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
codec_handle_t codecHandle;

wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format           = {.mclk_HZ    = 12288000U,
                         .sampleRate = kWM8960_AudioSampleRate48KHz,
                         .bitWidth   = kWM8960_AudioBitWidth16bit},
    .master_slave     = false,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};

/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM)
 *                              = 24 * (32 + 77/100)
 *                              = 786.48 MHz
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 1, 2, 4, 8, 16. */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

static void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        ENABLE_IOMUXC_GPR_SAI_MCLK;
    }
    else
    {
        DISABLE_IOMUXC_GPR_SAI_MCLK;
    }
}

void audio_clean_tx_buff(void)
{
    memset(audioTxBuff, 0, BUFFER_SIZE * BUFFER_NUM);
}

void audio_speaker_board_setup(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    CLOCK_InitAudioPll(&audioPllConfig);
    BOARD_InitDebugConsole();

    /*Clock setting for LPI2C*/
    set_clock_lpi2c();

    /*Clock setting for SAI*/
    set_clock_sai();

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    dmaMuxTxConfig.dmaMuxConfig.dmaMuxInstance   = DEMO_DMAMUX_INDEX;
    dmaMuxTxConfig.dmaMuxConfig.dmaRequestSource = DEMO_SAI_TX_SOURCE;
    dmaTxConfig.dmaMuxConfig                     = &dmaMuxTxConfig;
    dmaTxConfig.instance                         = DEMO_DMA_INDEX;
    dmaTxConfig.channel                          = DEMO_DMA_TX_CHANNEL;
    dmaTxConfig.priority                         = kHAL_AudioDmaChannelPriorityDefault;
    dmaTxConfig.dmaChannelMuxConfig              = NULL;
    ipTxConfig.sai.lineMask                      = 1U << 0U;
    ipTxConfig.sai.syncMode                      = kHAL_AudioSaiModeAsync;
    audioTxConfig.dmaConfig                      = &dmaTxConfig;
    audioTxConfig.ipConfig                       = &ipTxConfig;
    audioTxConfig.instance                       = DEMO_SAI_INSTANCE_INDEX;
    audioTxConfig.srcClock_Hz                    = DEMO_SAI_CLK_FREQ;
    audioTxConfig.sampleRate_Hz                  = (uint32_t)kHAL_AudioSampleRate48KHz;
    audioTxConfig.msaterSlave                    = kHAL_AudioMaster;
    audioTxConfig.bclkPolarity                   = kHAL_AudioSampleOnRisingEdge;
    audioTxConfig.frameSyncWidth                 = kHAL_AudioFrameSyncWidthHalfFrame;
    audioTxConfig.frameSyncPolarity              = kHAL_AudioBeginAtFallingEdge;
    audioTxConfig.dataFormat                     = kHAL_AudioDataFormatI2sClassic;
    audioTxConfig.fifoWatermark                  = (uint8_t)(FSL_FEATURE_SAI_FIFO_COUNTn() - 1);
    audioTxConfig.bitWidth                       = (uint8_t)kHAL_AudioWordWidth16bits;
    audioTxConfig.lineChannels                   = kHAL_AudioStereo;
}

static void txCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    hal_audio_transfer_t xfer = {0};

    sendCount++;

    audio_transfer_byte_count += BUFFER_SIZE;
    xfer.data = audioTxBuff + txindex * BUFFER_SIZE;
    xfer.dataSize = BUFFER_SIZE;
    txindex   = (txindex + 1U) % BUFFER_NUM;
    HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &xfer);
}

void audio_speaker_start(void)
{
    int i;
    hal_audio_transfer_t xfer = {0};

    PRINTF("Init Audio SAI and CODEC\r\n");

    /*Initialize audio interface and codec.*/
    HAL_AudioTxInit((hal_audio_handle_t)audioTxHandle, &audioTxConfig);

    /* Use default setting to init codec */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }

    /* Start playback */
    memset(audioTxBuff, 0, BUFFER_SIZE * BUFFER_NUM);
    xfer.dataSize = BUFFER_SIZE;
    for (i = 0; i < BUFFER_NUM; i++)
    {
        xfer.data = audioTxBuff + i * BUFFER_SIZE;
        HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &xfer);
    }

    HAL_AudioTxInstallCallback((hal_audio_handle_t)&audioTxHandle[0], txCallback, NULL);
}

void audio_send(unsigned char *frame_buffer, unsigned long frame_length)
{
    /* Write audio data to the audio buffer.  */
    if (rxindex + frame_length <= BUFFER_NUM * BUFFER_SIZE)
    {
        memcpy(audioTxBuff + rxindex, frame_buffer, frame_length);
    }
    else
    {
        memcpy(audioTxBuff + rxindex, frame_buffer, BUFFER_NUM * BUFFER_SIZE - rxindex);
        memcpy(audioTxBuff, frame_buffer + BUFFER_NUM * BUFFER_SIZE - rxindex, frame_length + rxindex - BUFFER_NUM * BUFFER_SIZE);
    }
    rxindex = (rxindex + frame_length) % (BUFFER_NUM * BUFFER_SIZE);
}

void audio_set_tx_resync(void)
{
    audio_tx_resync = 1;
}

size_t audio_get_tx_cout(void)
{
    status_t    status;
    size_t      sai_transferred_byte_count;
    size_t      transferred_byte_count = 0;
    int         current_tx_pos;
    int         byte_delta;

    if (audio_tx_resync)
    {
        status = HAL_AudioTransferGetSendCount((hal_audio_handle_t)&audioTxHandle[0],
                                               &sai_transferred_byte_count);
        if (kStatus_Success == status)
        {
            rxindex = ((sendCount % BUFFER_NUM) * BUFFER_SIZE + sai_transferred_byte_count + (BUFFER_NUM * BUFFER_SIZE / 2)) % (BUFFER_NUM * BUFFER_SIZE);
            rxindex &= ~(UX_DEMO_BYTES_PER_SAMPLE - 1);

            audio_transfer_byte_count = 0 - sai_transferred_byte_count;
            audio_tx_resync = 0;
        }
    }

    status = HAL_AudioTransferGetSendCount((hal_audio_handle_t)&audioTxHandle[0],
                                          &sai_transferred_byte_count);
    if (kStatus_Success == status)
    {
        current_tx_pos = (sendCount % BUFFER_NUM) * BUFFER_SIZE + sai_transferred_byte_count;

        byte_delta = current_tx_pos > rxindex ? current_tx_pos - rxindex :
                     current_tx_pos + BUFFER_NUM * BUFFER_SIZE - rxindex;

        if (byte_delta > BUFFER_NUM / 2 * BUFFER_SIZE + UX_DEMO_BYTES_SYNC_THRESHOLD)
        {
            audio_transfer_byte_count += UX_DEMO_BYTES_PER_SAMPLE;
        }
        else if (byte_delta < BUFFER_NUM / 2 * BUFFER_SIZE - UX_DEMO_BYTES_SYNC_THRESHOLD)
        {
            audio_transfer_byte_count -= UX_DEMO_BYTES_PER_SAMPLE;
        }

        transferred_byte_count = audio_transfer_byte_count + sai_transferred_byte_count;

        audio_transfer_byte_count =  0 - sai_transferred_byte_count;
    }

    return transferred_byte_count;
}
