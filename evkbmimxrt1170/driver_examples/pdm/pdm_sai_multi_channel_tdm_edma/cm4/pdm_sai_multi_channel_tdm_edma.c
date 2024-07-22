/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_pdm.h"
#include "fsl_pdm_edma.h"
#include "fsl_edma.h"
#include "fsl_sai.h"
#include "fsl_sai_edma.h"
#include "fsl_debug_console.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_dmamux.h"
#include "fsl_cs42448.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                     PDM
#define DEMO_SAI                     SAI1
#define DEMO_SAI_CLK_FREQ            24576000
#define DEMO_SAI_CLOCK_SOURCE        (kSAI_BclkSourceMclkDiv)
#define DEMO_PDM_CLK_FREQ            49152000
#define DEMO_PDM_FIFO_WATERMARK      (4)
#define DEMO_PDM_QUALITY_MODE        kPDM_QualityModeHigh
#define DEMO_PDM_CIC_OVERSAMPLE_RATE (0U)
#define DEMO_PDM_ENABLE_CHANNEL_0    (0U)
#define DEMO_PDM_ENABLE_CHANNEL_1    (1U)
#define DEMO_PDM_ENABLE_CHANNEL_2    (2U)
#define DEMO_PDM_ENABLE_CHANNEL_3    (3U)
#define DEMO_PDM_ENABLE_CHANNEL_4    (4U)
#define DEMO_PDM_ENABLE_CHANNEL_5    (5U)
#define DEMO_PDM_ENABLE_CHANNEL_6    (6U)
#define DEMO_PDM_ENABLE_CHANNEL_7    (7U)
#define DEMO_PDM_SAMPLE_CLOCK_RATE   (6144000U) /* 6.144MHZ */
#define DEMO_PDM_CHANNEL_GAIN        kPDM_DfOutputGain7

/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate48KHz)
/* demo audio master clock */
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (8U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH kSAI_WordWidth32bits

#define DEMO_CS42448_I2C_INSTANCE 6
#define DEMO_CODEC_POWER_GPIO     GPIO9
#define DEMO_CODEC_POWER_GPIO_PIN 9
#define DEMO_CODEC_RESET_GPIO     GPIO11
#define DEMO_CODEC_RESET_GPIO_PIN 11

#define DEMO_EDMA               DMA1
#define DEMO_DMAMUX             DMAMUX1
#define DEMO_PDM_EDMA_CHANNEL_0 0
#define DEMO_PDM_EDMA_CHANNEL_1 1
#define DEMO_SAI_EDMA_CHANNEL   2
#define DEMO_PDM_REQUEST_SOURCE kDmaRequestMuxPdm
#define DEMO_SAI_REQUEST_SOURCE kDmaRequestMuxSai1Tx
#define BOARD_MasterClockConfig()
#define BUFFER_SIZE   (1024)
#define BUFFER_NUMBER (2)
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
void BORAD_CodecReset(bool state);
static void pdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
static void saiCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_handle_t s_pdmRxHandle_0, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_pdmDmaHandle_0, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_saiDmaHandle, 4);
#if defined(DEMO_QUICKACCESS_SECTION_CACHEABLE) && DEMO_QUICKACCESS_SECTION_CACHEABLE
AT_NONCACHEABLE_SECTION_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t s_edmaTcd_0[2], 32U);
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t s_edmaTcd_1[2], 32U);
#else
AT_QUICKACCESS_SECTION_DATA_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t s_edmaTcd_0[2], 32U);
AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t s_edmaTcd_1[2], 32U);
#endif
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUMBER], 4);

pdm_edma_transfer_t pdmXfer[2] = {
    {
        .data         = s_buffer,
        .dataSize     = BUFFER_SIZE,
        .linkTransfer = &pdmXfer[1],
    },

    {.data = &s_buffer[BUFFER_SIZE], .dataSize = BUFFER_SIZE, .linkTransfer = &pdmXfer[0]},
};

static volatile uint32_t s_bufferValidBlock = BUFFER_NUMBER;
static volatile uint32_t s_readIndex        = 0U;
static volatile uint32_t s_writeIndex       = 0U;
static const pdm_config_t pdmConfig         = {
#if defined(FSL_FEATURE_PDM_HAS_DECIMATION_FILTER_BYPASS) && FSL_FEATURE_PDM_HAS_DECIMATION_FILTER_BYPASS
    .enableFilterBypass = false,
#endif
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

codec_handle_t codecHandle;
extern codec_config_t boardCodecConfig;

/*******************************************************************************
 * Code
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = BORAD_CodecReset,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CS42448_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .format       = {.mclk_HZ = 24576000, .sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
/*
 * AUDIO PLL setting: Frequency = Fref * (DIV_SELECT + NUM / DENOM) / (2^POST)
 *                              = 24 * (32 + 768/1000)  / 2
 *                              = 393.216MHZ
 */
const clock_audio_pll_config_t audioPllConfig = {
    .loopDivider = 32,   /* PLL loop divider. Valid range for DIV_SELECT divider value: 27~54. */
    .postDivider = 1,    /* Divider after the PLL, should only be 0, 1, 2, 3, 4, 5 */
    .numerator   = 768,  /* 30 bit numerator of fractional loop divider. */
    .denominator = 1000, /* 30 bit denominator of fractional loop divider */
};
static uint32_t s_pdmGain           = DEMO_PDM_CHANNEL_GAIN;
static volatile bool s_increaseGain = true;

void GPIO13_Combined_0_31_IRQHandler(void)
{
    GPIO_PortClearInterruptFlags(GPIO13, 1U << 0U);

    if (s_increaseGain)
    {
        s_pdmGain++;
    }
    else
    {
        s_pdmGain--;
    }

    if (s_pdmGain == kPDM_DfOutputGain15)
    {
        s_increaseGain = false;
    }

    if (s_pdmGain == kPDM_DfOutputGain0)
    {
        s_increaseGain = true;
    }

    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_0, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_1, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_2, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_3, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_4, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_5, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_6, (pdm_df_output_gain_t)s_pdmGain);
    PDM_SetChannelGain(DEMO_PDM, DEMO_PDM_ENABLE_CHANNEL_7, (pdm_df_output_gain_t)s_pdmGain);
}

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


void BORAD_CodecReset(bool state)
{
    if (state)
    {
        GPIO_PinWrite(DEMO_CODEC_RESET_GPIO, DEMO_CODEC_RESET_GPIO_PIN, 1U);
    }
    else
    {
        GPIO_PinWrite(DEMO_CODEC_RESET_GPIO, DEMO_CODEC_RESET_GPIO_PIN, 0U);
    }
}
static void pdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData)
{
    if (s_bufferValidBlock)
    {
        s_bufferValidBlock--;
    }
}

static void saiCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_TxError == status)
    {
        /* Handle the error. */
    }
    else
    {
        if (s_bufferValidBlock < BUFFER_NUMBER)
        {
            s_bufferValidBlock++;
        }
    }
}

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
    SDK_ISR_EXIT_BARRIER;
}

void SAI_UserIRQHandler(void)
{
    SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    edma_config_t dmaConfig = {0};
    sai_transfer_t saiXfer;
    sai_transceiver_t config;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    EnableIRQ(GPIO13_Combined_0_31_IRQn);

    CLOCK_InitAudioPll(&audioPllConfig);

    CLOCK_SetRootClockMux(kCLOCK_Root_Lpi2c6, 1);
    /* audio pll  */
    CLOCK_SetRootClockMux(kCLOCK_Root_Sai1, 4);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Sai1, 16);
    /* 0SC400M */
    /* 24.576m mic root clock */
    CLOCK_SetRootClockMux(kCLOCK_Root_Mic, 6);
    CLOCK_SetRootClockDiv(kCLOCK_Root_Mic, 8);

    BOARD_EnableSaiMclkOutput(true);

    DMAMUX_Init(DEMO_DMAMUX);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_PDM_EDMA_CHANNEL_0, DEMO_PDM_REQUEST_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_PDM_EDMA_CHANNEL_0);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_SAI_EDMA_CHANNEL, DEMO_SAI_REQUEST_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_SAI_EDMA_CHANNEL);

    /* enable codec power */
    GPIO_PinWrite(DEMO_CODEC_POWER_GPIO, DEMO_CODEC_POWER_GPIO_PIN, 1U);

    PRINTF("PDM SAI multi channel TDM edma example started!\n\r");

    /* Create EDMA handle */
    /*
     * dmaConfig.enableRoundRobinArbitration = false;
     * dmaConfig.enableHaltOnError = true;
     * dmaConfig.enableContinuousLinkMode = false;
     * dmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_EDMA, &dmaConfig);
    EDMA_CreateHandle(&s_pdmDmaHandle_0, DEMO_EDMA, DEMO_PDM_EDMA_CHANNEL_0);
    EDMA_CreateHandle(&s_saiDmaHandle, DEMO_EDMA, DEMO_SAI_EDMA_CHANNEL);

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &s_saiTxHandle, saiCallback, NULL, &s_saiDmaHandle);
    /* TDM mode configurations */
    SAI_GetTDMConfig(&config, kSAI_FrameSyncLenOneBitClk, DEMO_AUDIO_BIT_WIDTH, DEMO_AUDIO_DATA_CHANNEL,
                     kSAI_Channel0Mask);
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }
    config.frameSync.frameSyncEarly = true;
    SAI_TransferTxSetConfigEDMA(DEMO_SAI, &s_saiTxHandle, &config);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* master clock configurations */
    BOARD_MasterClockConfig();

    /* Setup pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);

    PDM_TransferCreateHandleEDMA(DEMO_PDM, &s_pdmRxHandle_0, pdmCallback, NULL, &s_pdmDmaHandle_0);
    PDM_TransferInstallEDMATCDMemory(&s_pdmRxHandle_0, s_edmaTcd_0, 2);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_0, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_1, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_2, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_3, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_4, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_5, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_6, &channelConfig);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle_0, DEMO_PDM_ENABLE_CHANNEL_7, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
        return -1;
    }
    PDM_Reset(DEMO_PDM);

    PDM_TransferReceiveEDMA(DEMO_PDM, &s_pdmRxHandle_0, pdmXfer);

    while (1)
    {
        /* wait one buffer busy to send data */
        if (s_bufferValidBlock < BUFFER_NUMBER)
        {
            saiXfer.data     = (uint8_t *)((uint32_t)s_buffer + s_writeIndex * BUFFER_SIZE);
            saiXfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == SAI_TransferSendEDMA(DEMO_SAI, &s_saiTxHandle, &saiXfer))
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
