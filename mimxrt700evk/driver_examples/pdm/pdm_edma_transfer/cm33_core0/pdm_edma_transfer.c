/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_pdm.h"
#include "fsl_debug_console.h"
#include "fsl_pdm_edma.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_edma_soc.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                      PDM
#define DEMO_PDM_CLK_FREQ             CLOCK_GetMicfilClkFreq()
#define DEMO_PDM_FIFO_WATERMARK       (FSL_FEATURE_PDM_FIFO_DEPTH / 2U)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeMedium
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (16U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_AUDIO_SAMPLE_RATE        (48000U)

#define DEMO_EDMA             DMA0
#define DEMO_EDMA_CHANNEL     0U
#define DEMO_PDM_EDMA_CHANNEL kDmaRequestMuxMicfil
#define DEMO_PDM_ERROR_IRQn   EDMA0_CH0_IRQn

#define DEMO_QUICKACCESS_SECTION_CACHEABLE 1U
#define BUFFER_SIZE (256)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void pdmEdmallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_handle_t pdmRxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t dmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(static int16_t rxBuff[BUFFER_SIZE], 4);
#if defined(DEMO_QUICKACCESS_SECTION_CACHEABLE) && DEMO_QUICKACCESS_SECTION_CACHEABLE
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t s_edmaTcd[1], 32U);
#else
AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t s_edmaTcd[1], 32U);
#endif
#if (defined(FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ) && (FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ == 1U))
static volatile bool s_lowFreqFlag = false;
#endif
static volatile bool s_fifoErrorFlag = false;
static volatile bool s_pdmRxFinished = false;

static const pdm_config_t pdmConfig = {
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
    .gain       = kPDM_DfOutputGain4,
#endif
};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void pdmEdmallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData)
{
    s_pdmRxFinished = true;
}

void DEMO_PDM_ERROR_IRQHandler(void)
{
    uint32_t status = 0U;

#if (defined(FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ) && (FSL_FEATURE_PDM_HAS_STATUS_LOW_FREQ == 1U))
    if (PDM_GetStatus(DEMO_PDM) & PDM_STAT_LOWFREQF_MASK)
    {
        PDM_ClearStatus(DEMO_PDM, PDM_STAT_LOWFREQF_MASK);
        s_lowFreqFlag = true;
    }
#endif
    status = PDM_GetFifoStatus(DEMO_PDM);
    if (status)
    {
        PDM_ClearFIFOStatus(DEMO_PDM, status);
        s_fifoErrorFlag = true;
    }

#if defined(FSL_FEATURE_PDM_HAS_RANGE_CTRL) && FSL_FEATURE_PDM_HAS_RANGE_CTRL
    status = PDM_GetRangeStatus(DEMO_PDM);
    if (status)
    {
        PDM_ClearRangeStatus(DEMO_PDM, status);
    }
#else
    status = PDM_GetOutputStatus(DEMO_PDM);
    if (status)
    {
        PDM_ClearOutputStatus(DEMO_PDM, status);
    }
#endif
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0U, j = 0U;
    pdm_edma_transfer_t xfer;
    edma_config_t dmaConfig = {0};

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_MICFIL0);
    CLOCK_SetClkDiv(kCLOCK_DivMicfil0Clk, 15U);

    POWER_DisablePD(kPDRUNCFG_APD_DMA0_1_PKC_ETF);
    POWER_DisablePD(kPDRUNCFG_PPD_DMA0_1_PKC_ETF);
    POWER_ApplyPD();

    RESET_ClearPeripheralReset(kPDM_RST_SHIFT_RSTn);
    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    EDMA_EnableRequest(DEMO_EDMA, DEMO_PDM_EDMA_CHANNEL);

    PRINTF("PDM edma example started!\n\r");

    memset(rxBuff, 0, sizeof(rxBuff));

    /* Create EDMA handle */
    /*
     * dmaConfig.enableRoundRobinArbitration = false;
     * dmaConfig.enableHaltOnError = true;
     * dmaConfig.enableContinuousLinkMode = false;
     * dmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_EDMA, &dmaConfig);
    EDMA_CreateHandle(&dmaHandle, DEMO_EDMA, DEMO_EDMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_EDMA, DEMO_EDMA_CHANNEL, DEMO_PDM_EDMA_CHANNEL);
#endif
    /* Set up pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_TransferCreateHandleEDMA(DEMO_PDM, &pdmRxHandle, pdmEdmallback, NULL, &dmaHandle);
    PDM_TransferInstallEDMATCDMemory(&pdmRxHandle, s_edmaTcd, 1);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &pdmRxHandle, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
        return -1;
    }
    PDM_Reset(DEMO_PDM);
    PDM_EnableInterrupts(DEMO_PDM, kPDM_ErrorInterruptEnable);
    EnableIRQ(DEMO_PDM_ERROR_IRQn);
    xfer.data         = (uint8_t *)rxBuff;
    xfer.dataSize     = BUFFER_SIZE * 2U;
    xfer.linkTransfer = NULL;

    PDM_TransferReceiveEDMA(DEMO_PDM, &pdmRxHandle, &xfer);
    /* wait data read finish */
    while (!s_pdmRxFinished)
    {
    }

    PRINTF("PDM recieve one channel data:\n\r");
    for (i = 0U; i < BUFFER_SIZE; i++)
    {
        PRINTF("%4d ", rxBuff[i]);
        if (++j > 32U)
        {
            j = 0U;
            PRINTF("\r\n");
        }
    }

    PDM_Deinit(DEMO_PDM);

    PRINTF("\r\nPDM edma example finished!\n\r ");
    while (1)
    {
    }
}
