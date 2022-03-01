/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pdm.h"
#include "fsl_sai.h"
#include "fsl_debug_console.h"
#include "fsl_codec_common.h"
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                      PDM
#define DEMO_SAI                      SAI1
#define DEMO_SAI_CLK_FREQ             24576000
#define DEMO_SAI_CHANNEL              0
#define DEMO_SAI_MASTER_SLAVE         kSAI_Master
#define DEMO_SAI_CLOCK_SOURCE         (kSAI_BclkSourceMclkDiv)
#define DEMO_PDM_CLK_FREQ             24576000
#define DEMO_PDM_FIFO_WATERMARK       (7)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_PDM_SAMPLE_CLOCK_RATE    (6144000U) /* 6.144MHZ */
/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate48KHz)
/* demo audio master clock */
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (2U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH kSAI_WordWidth32bits
#define BOARD_MasterClockConfig()
#define BUFFER_SIZE   (1024U)
#define BUFFER_NUMBER (4U)
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
static void saiCallback(I2S_Type *base, sai_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static sai_handle_t s_saiTxHandle = {0};
static pdm_handle_t s_pdmHandle;
SDK_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUMBER], 4);
static volatile uint32_t s_readIndex        = 0U;
static volatile uint32_t s_writeIndex       = 0U;
static volatile uint32_t s_bufferValidBlock = BUFFER_NUMBER;
static const pdm_config_t pdmConfig         = {
    .enableDoze        = false,
    .fifoWatermark     = DEMO_PDM_FIFO_WATERMARK,
    .qualityMode       = DEMO_PDM_QUALITY_MODE,
    .cicOverSampleRate = DEMO_PDM_CIC_OVERSAMPLE_RATE,
};
static pdm_channel_config_t channelConfig = {
#if (defined(FSL_FEATURE_PDM_HAS_DC_OUT_CTRL) && (FSL_FEATURE_PDM_HAS_DC_OUT_CTRL))
    .outputCutOffFreq = kPDM_DcRemoverCutOff40Hz,
#else
    .cutOffFreq = kPDM_DcRemoverCutOff152Hz,
#endif
    .gain = kPDM_DfOutputGain7,
};

codec_handle_t codecHandle;

extern codec_config_t boardCodecConfig;
/*******************************************************************************
 * Code
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .i2cConfig        = {.codecI2CInstance = 5, .codecI2CSourceClock = 24000000U},
    .route            = kWM8960_RoutePlaybackandRecord,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format = {.mclk_HZ = 24576000, .sampleRate = kWM8960_AudioSampleRate48KHz, .bitWidth = kWM8960_AudioBitWidth32bit},
    .master_slave = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8960, .codecDevConfig = &wm8960Config};

/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 77/100)  / 2
 *                              = 393.24MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,  /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,   /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 77,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 100, /* 30 bit denominator of fractional loop divider */
};

void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR0 |= 1 << 8U;
    }
    else
    {
        IOMUXC_GPR->GPR0 &= ~(1 << 8U);
    }
}


static void saiCallback(I2S_Type *base, sai_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_TxError == status)
    {
        /* Handle the error. */
    }
    else
    {
        s_bufferValidBlock++;
    }
}

static void pdmCallback(PDM_Type *base, pdm_handle_t *handle, status_t status, void *userData)
{
#if (defined(FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ) && (FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ == 1U))
    if ((status == kStatus_PDM_FIFO_ERROR) || (status == kStatus_PDM_Output_ERROR) || (status == kStatus_PDM_CLK_LOW))
#else
    if ((status == kStatus_PDM_FIFO_ERROR) || (status == kStatus_PDM_Output_ERROR))
#endif
    {
        /* handle error */
    }
    else
    {
        if (s_bufferValidBlock)
        {
            s_bufferValidBlock--;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sai_transfer_t saiXfer;
    pdm_transfer_t pdmXfer;
    sai_transceiver_t config;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_InitAudioPll(&audioPllConfig);

    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c5, 1);
    /* audio pll  */
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);
    /* 0SC400M */
    /* 24.576m mic root clock */
    CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 16);

    BOARD_EnableSaiMclkOutput(true);

    PRINTF("PDM SAI interrupt transfer example started!\n\r");

    memset(s_buffer, 0U, sizeof(s_buffer));

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandle(DEMO_SAI, &s_saiTxHandle, saiCallback, NULL);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, 1U << DEMO_SAI_CHANNEL);

    config.bitClock.bclkSource = DEMO_SAI_CLOCK_SOURCE;
    config.masterSlave         = DEMO_SAI_MASTER_SLAVE;
#if defined BOARD_SAI_RXCONFIG
    config.syncMode = DEMO_SAI_TX_SYNC_MODE;
#endif

    SAI_TransferTxSetConfig(DEMO_SAI, &s_saiTxHandle, &config);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
#if defined BOARD_SAI_RXCONFIG
    BOARD_SAI_RXCONFIG(&config, DEMO_SAI_RX_SYNC_MODE);
#endif

    /* master clock configurations */
    BOARD_MasterClockConfig();

#if defined DEMO_BOARD_CODEC_INIT
    DEMO_BOARD_CODEC_INIT();
#else
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }
#endif
    /* Set up pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
        return -1;
    }
    PDM_TransferCreateHandle(DEMO_PDM, &s_pdmHandle, pdmCallback, NULL);
    PDM_TransferSetChannelConfig(DEMO_PDM, &s_pdmHandle, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig,
                                 FSL_FEATURE_PDM_FIFO_WIDTH);
    PDM_TransferSetChannelConfig(DEMO_PDM, &s_pdmHandle, DEMO_PDM_ENABLE_CHANNEL_RIGHT, &channelConfig,
                                 FSL_FEATURE_PDM_FIFO_WIDTH);
    PDM_Reset(DEMO_PDM);

    while (1)
    {
        if (s_bufferValidBlock > 0)
        {
            pdmXfer.data     = (uint8_t *)((uint32_t)s_buffer + s_readIndex * BUFFER_SIZE);
            pdmXfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == PDM_TransferReceiveNonBlocking(DEMO_PDM, &s_pdmHandle, &pdmXfer))
            {
                s_readIndex++;
            }
            if (s_readIndex == BUFFER_NUMBER)
            {
                s_readIndex = 0U;
            }
        }
        if (s_bufferValidBlock < BUFFER_NUMBER)
        {
            saiXfer.data     = (uint8_t *)((uint32_t)s_buffer + s_writeIndex * BUFFER_SIZE);
            saiXfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == SAI_TransferSendNonBlocking(DEMO_SAI, &s_saiTxHandle, &saiXfer))
            {
                s_writeIndex++;
            }
            if (s_writeIndex == BUFFER_NUMBER)
            {
                s_writeIndex = 0U;
            }
        }
    }
}
