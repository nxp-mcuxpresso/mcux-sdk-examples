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
#include "fsl_cs42448.h"
#include "fsl_codec_adapter.h"
#include "fsl_clock.h"
#include "fsl_edma_soc.h"
#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                     PDM
#define DEMO_PDM_CLK_FREQ            CLOCK_GetMicfilClkFreq()
#define DEMO_PDM_FIFO_WATERMARK      (FSL_FEATURE_PDM_FIFO_DEPTH / 2U)
#define DEMO_PDM_QUALITY_MODE        kPDM_QualityModeHigh
#define DEMO_PDM_CHANNEL_GAIN        (kPDM_DfOutputGain7)
#define DEMO_PDM_CIC_OVERSAMPLE_RATE (16U)
#define DEMO_PDM_ENABLE_CHANNEL_0    (0U)
#define DEMO_PDM_ENABLE_CHANNEL_1    (1U)
#define DEMO_PDM_ENABLE_CHANNEL_2    (2U)
#define DEMO_PDM_ENABLE_CHANNEL_3    (3U)
#define DEMO_PDM_ENABLE_CHANNEL_4    (4U)
#define DEMO_PDM_ENABLE_CHANNEL_5    (5U)
#define DEMO_PDM_ENABLE_CHANNEL_6    (6U)
#define DEMO_PDM_ENABLE_CHANNEL_7    (7U)

/* SAI instance and clock */
#define DEMO_SAI                SAI0
#define DEMO_SAI_CHANNEL        (0)
#define DEMO_SAI_TX_SYNC_MODE   kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE   kSAI_ModeSync
#define DEMO_SAI_MASTER_SLAVE   kSAI_Master
#define DEMO_AUDIO_DATA_CHANNEL (8U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth32bits
/* Note the DMIC has clock limitation, the PDM_CLK can't exceed it's max clock. */
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate24KHz)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#define DEMO_SAI_CLOCK_SOURCE   kSAI_BclkSourceMclkDiv

/* Get frequency of sai clock */
#define DEMO_SAI_CLK_FREQ CLOCK_GetSaiClkFreq()

#define DEMO_EDMA               DMA0
#define DEMO_PDM_EDMA_CHANNEL_0 0U
#define DEMO_SAI_EDMA_CHANNEL   1U
#define DEMO_PDM_EDMA_SOURCE    kDmaRequestMuxMicfil
#define DEMO_SAI_EDMA_SOURCE    kDmaRequestMuxSai0Tx

#define DEMO_CS42448_I2C_INSTANCE 2U
#define DEMO_CODEC_POWER_GPIO     GPIO0
#define DEMO_CODEC_POWER_GPIO_PIN 19
#define DEMO_CODEC_RESET_GPIO     GPIO0
#define DEMO_CODEC_RESET_GPIO_PIN 10

/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ 24000000U

#define DEMO_QUICKACCESS_SECTION_CACHEABLE 1U
#define BUFFER_SIZE   (1024)
#define BUFFER_NUMBER (2)
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_MasterClockConfig(void);
void BORAD_CodecReset(bool state);
static void pdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
static void saiCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = BORAD_CodecReset,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CS42448_I2C_INSTANCE, .codecI2CSourceClock = DEMO_I2C_CLK_FREQ},
    .format       = {.mclk_HZ = 24576000, .sampleRate = DEMO_AUDIO_SAMPLE_RATE, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};

sai_master_clock_t mclkConfig;

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
void BOARD_MasterClockConfig(void)
{
    mclkConfig.mclkOutputEnable = true;
    mclkConfig.mclkHz           = 24576000U;
    mclkConfig.mclkSourceClkHz  = 24576000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
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

    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_MICFIL0);
    CLOCK_SetClkDiv(kCLOCK_DivMicfil0Clk, 15U);

    RESET_ClearPeripheralReset(kSAI0_RST_SHIFT_RSTn);
    RESET_ClearPeripheralReset(kPDM_RST_SHIFT_RSTn);

    /*Clock setting for LPI2C */
    CLOCK_AttachClk(kFCCLK0_to_FLEXCOMM2);

    /* SAI clock 368.64 / 15 = 24.576MHz */
    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_AUDIO_VDD2);
    CLOCK_AttachClk(kAUDIO_VDD2_to_SAI012);
    CLOCK_SetClkDiv(kCLOCK_DivSai012Clk, 15U);

    RESET_ClearPeripheralReset(kSAI0_RST_SHIFT_RSTn);

    SYSCON0->SAI0_MCLK_CTRL |= SYSCON0_SAI0_MCLK_CTRL_SAIMCLKDIR_MASK;

    POWER_DisablePD(kPDRUNCFG_APD_DMA0_1_PKC_ETF);
    POWER_DisablePD(kPDRUNCFG_PPD_DMA0_1_PKC_ETF);
    POWER_ApplyPD();

    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    EDMA_EnableRequest(DEMO_EDMA, DEMO_PDM_EDMA_SOURCE);
    EDMA_EnableRequest(DEMO_EDMA, DEMO_SAI_EDMA_SOURCE);

    RESET_ClearPeripheralReset(kGPIO0_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Gpio0);

    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };

    GPIO_PinInit(DEMO_CODEC_POWER_GPIO, DEMO_CODEC_POWER_GPIO_PIN, &led_config);
    GPIO_PinInit(DEMO_CODEC_RESET_GPIO, DEMO_CODEC_RESET_GPIO_PIN, &led_config);

    /* Enable codec power. */
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

#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_EDMA, DEMO_PDM_EDMA_CHANNEL_0, DEMO_PDM_EDMA_SOURCE);
    EDMA_SetChannelMux(DEMO_EDMA, DEMO_SAI_EDMA_CHANNEL, DEMO_SAI_EDMA_SOURCE);
#endif

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
