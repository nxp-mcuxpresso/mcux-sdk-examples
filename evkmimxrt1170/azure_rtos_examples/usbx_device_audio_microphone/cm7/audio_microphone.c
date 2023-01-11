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
#include "audio_microphone.h"

#define DEMO_CODEC_VOLUME       (100)

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
HAL_AUDIO_HANDLE_DEFINE(audioTxHandle);
hal_audio_config_t audioTxConfig;
hal_audio_dma_config_t dmaTxConfig;
hal_audio_dma_mux_config_t dmaMuxTxConfig;
hal_audio_ip_config_t ipTxConfig;

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
HAL_AUDIO_HANDLE_DEFINE(audioRxHandle);
hal_audio_config_t audioRxConfig;
hal_audio_dma_config_t dmaRxConfig;
hal_audio_ip_config_t ipRxConfig;
hal_audio_dma_mux_config_t dmaMuxRxConfig;

uint32_t rxindex                    = 0;
uint32_t txindex                    = 0;
uint32_t usb_transfer_packet_count  = 0;
uint32_t audio_rx_resync            = 0;
volatile uint32_t sendCount         = 0;
volatile uint32_t receiveCount      = 0;

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioTxBuff[BUFFER_SIZE * (BUFFER_NUM + 1)];

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRxBuff[BUFFER_SIZE * (BUFFER_NUM + 1)];

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

void audio_microphone_setup(void)
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

    dmaMuxRxConfig.dmaMuxConfig.dmaMuxInstance   = DEMO_DMAMUX_INDEX;
    dmaMuxRxConfig.dmaMuxConfig.dmaRequestSource = DEMO_SAI_RX_SOURCE;
    dmaRxConfig.dmaMuxConfig                     = &dmaMuxRxConfig;
    dmaRxConfig.instance                         = DEMO_DMA_INDEX;
    dmaRxConfig.channel                          = DEMO_DMA_RX_CHANNEL;
    dmaRxConfig.priority                         = kHAL_AudioDmaChannelPriorityDefault;
    dmaRxConfig.dmaChannelMuxConfig              = NULL;
    ipRxConfig.sai.lineMask                      = 1U << 0U;
    ipRxConfig.sai.syncMode                      = kHAL_AudioSaiModeSync;
    audioRxConfig.dmaConfig                      = &dmaRxConfig;
    audioRxConfig.ipConfig                       = &ipRxConfig;
    audioRxConfig.instance                       = DEMO_SAI_INSTANCE_INDEX;
    audioRxConfig.srcClock_Hz                    = DEMO_SAI_CLK_FREQ;
    audioRxConfig.sampleRate_Hz                  = (uint32_t)kHAL_AudioSampleRate48KHz;
    audioRxConfig.msaterSlave                    = kHAL_AudioMaster;
    audioRxConfig.bclkPolarity                   = kHAL_AudioSampleOnRisingEdge;
    audioRxConfig.frameSyncWidth                 = kHAL_AudioFrameSyncWidthHalfFrame;
    audioRxConfig.frameSyncPolarity              = kHAL_AudioBeginAtFallingEdge;
    audioRxConfig.dataFormat                     = kHAL_AudioDataFormatI2sClassic;
    audioRxConfig.fifoWatermark                  = (uint8_t)((uint32_t)FSL_FEATURE_SAI_FIFO_COUNTn() / 2U);
    audioRxConfig.bitWidth                       = (uint8_t)kHAL_AudioWordWidth16bits;
    audioRxConfig.lineChannels                   = kHAL_AudioStereo;
}

static void rxCallback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    hal_audio_transfer_t xfer = {0};

    receiveCount++;

    xfer.data = audioRxBuff + rxindex * BUFFER_SIZE;
    xfer.dataSize = BUFFER_SIZE;
    rxindex   = (rxindex + 1U) % BUFFER_NUM;
    HAL_AudioTransferReceiveNonBlocking(handle, &xfer);
}

void audio_microphone_start(void)
{
    int i;
    hal_audio_transfer_t xfer = {0};

    PRINTF("Init Audio SAI and CODEC\r\n");

    /*Initialize audio interface and codec.*/
    HAL_AudioTxInit((hal_audio_handle_t)audioTxHandle, &audioTxConfig);
    HAL_AudioRxInit((hal_audio_handle_t)audioRxHandle, &audioRxConfig);

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

    memset(audioTxBuff, 0, sizeof(audioTxBuff));
    memset(audioRxBuff, 0, sizeof(audioRxBuff));

    xfer.dataSize = BUFFER_SIZE;
    for (i = 0; i < BUFFER_NUM; i++)
    {
        xfer.data = audioTxBuff + i * BUFFER_SIZE;
        HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audioTxHandle[0], &xfer);
    }
    HAL_AudioTxInstallCallback((hal_audio_handle_t)&audioTxHandle[0], rxCallback, NULL);

    xfer.dataSize = BUFFER_SIZE;
    for (i = 0; i < BUFFER_NUM; i++)
    {
        xfer.data = audioRxBuff + i * BUFFER_SIZE;
        HAL_AudioTransferReceiveNonBlocking((hal_audio_handle_t)&audioRxHandle[0], &xfer);
    }
    HAL_AudioRxInstallCallback((hal_audio_handle_t)&audioRxHandle[0], rxCallback, NULL);
}

void audio_set_rx_resync(void)
{
    audio_rx_resync = 1;
}

void audio_get_receive_buffer(unsigned long *offset, unsigned long *length)
{
    status_t    status;
    size_t      sai_transferred_byte_count;
    int         current_rx_pos;
    int         byte_delta;
    int         audio_frame_length_adjusted = BUFFER_SIZE;

    if (audio_rx_resync)
    {
        status = HAL_AudioTransferGetReceiveCount((hal_audio_handle_t)&audioRxHandle[0],
                                                  &sai_transferred_byte_count);
        if (kStatus_Success == status)
        {
            audio_rx_resync = 0;
            txindex = ((receiveCount % BUFFER_NUM) * BUFFER_SIZE + sai_transferred_byte_count + (BUFFER_NUM * BUFFER_SIZE / 2)) % (BUFFER_NUM * BUFFER_SIZE);
            txindex &= ~(UX_DEMO_BYTES_PER_SAMPLE - 1);
            usb_transfer_packet_count = 0;
        }
    }

    usb_transfer_packet_count++;

    status = HAL_AudioTransferGetReceiveCount((hal_audio_handle_t)&audioRxHandle[0],
                                             &sai_transferred_byte_count);
    if (kStatus_Success == status)
    {
        current_rx_pos = (receiveCount % BUFFER_NUM) * BUFFER_SIZE + sai_transferred_byte_count;

        byte_delta = current_rx_pos > txindex ? current_rx_pos - txindex :
                     current_rx_pos + BUFFER_NUM * BUFFER_SIZE - txindex;

        if (byte_delta > BUFFER_NUM / 2 * BUFFER_SIZE + UX_DEMO_BYTES_SYNC_THRESHOLD)
        {
            audio_frame_length_adjusted += UX_DEMO_BYTES_PER_SAMPLE;
        }
        else if (byte_delta < BUFFER_NUM / 2 * BUFFER_SIZE - UX_DEMO_BYTES_SYNC_THRESHOLD)
        {
            audio_frame_length_adjusted -= UX_DEMO_BYTES_PER_SAMPLE;
        }
    }

    if (txindex + audio_frame_length_adjusted >= BUFFER_NUM * BUFFER_SIZE)
    {
        memcpy(audioRxBuff + BUFFER_NUM * BUFFER_SIZE, audioRxBuff,
               txindex + audio_frame_length_adjusted - BUFFER_NUM * BUFFER_SIZE);
    }

    *length = audio_frame_length_adjusted;
    *offset = txindex;
    txindex = (txindex + (uint32_t)audio_frame_length_adjusted) % (BUFFER_NUM * BUFFER_SIZE);
}
