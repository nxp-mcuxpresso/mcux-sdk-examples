/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <xtensa/config/core.h>
#include <xtensa/xos.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "fsl_sai_edma.h"
#include "fsl_pdm_edma.h"
#include "pin_mux.h"

#include "board_fusionf1.h"
#include "fsl_common.h"
#include "fsl_reset.h"
#include "dsp_config.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DMA              DMA0
#define DEMO_PDM_EDMA_SOURCE  kDmaRequestMux0MICFIL
#define DEMO_PDM_EDMA_CHANNEL 0U
#define DEMO_SAI_EDMA_SOURCE  kDmaRequestMux0SAI0Tx
#define DEMO_SAI_EDMA_CHANNEL 1U

#define DEMO_PDM                      PDM
#define DEMO_PDM_CLK_FREQ             CLOCK_GetMicfilFreq()
#define DEMO_PDM_FIFO_WATERMARK       (FSL_FEATURE_PDM_FIFO_DEPTH / 2U)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_DFOUTPUT_GAIN        kPDM_DfOutputGain0
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)
#define DEMO_PDM_HWVAD_SIGNAL_GAIN    0

#define DEMO_SAI              SAI0
#define DEMO_SAI_CHANNEL      (0)
#define DEMO_SAI_MASTER_SLAVE kSAI_Master
#define DEMO_SAI_CLOCK_SOURCE (kSAI_BclkSourceMclkDiv)
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeAsync

#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth32bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)
#define DEMO_AUDIO_MASTER_CLOCK CLOCK_GetIpFreq(kCLOCK_Sai0)

#define BOARD_MasterClockConfig()
#define BOARD_SAI_RXCONFIG(config, mode) BOARD_SAI_RxConfig(config, mode);

#define BOARD_XTAL0_CLK_HZ (24000000U)
#ifndef BUFFER_NUM
#define BUFFER_NUM (SAI_XFER_QUEUE_SIZE)
#endif
#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void DMA0_DriverIRQHandler(uint32_t Channel);
void BOARD_SAI_RxConfig(sai_transceiver_t *config, sai_sync_mode_t sync);
static void pdmCallback(PDM_Type *base, pdm_edma_handle_t *handle, status_t status, void *userData);
static void saiCallback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (XCHAL_DCACHE_SIZE > 0)
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUM], 4);
AT_NONCACHEABLE_SECTION_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_handle_t s_pdmRxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_pdmDmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_saiDmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t s_edmaTcd[BUFFER_NUM], 32);
#else
SDK_ALIGN(static uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUM], 4);
SDK_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
SDK_ALIGN(pdm_edma_handle_t s_pdmRxHandle, 4);
SDK_ALIGN(edma_handle_t s_pdmDmaHandle, 4);
SDK_ALIGN(edma_handle_t s_saiDmaHandle, 4);
SDK_ALIGN(edma_tcd_t s_edmaTcd[BUFFER_NUM], 32);
#endif

uint32_t s_bufferValidBlock           = BUFFER_NUM;
static volatile uint32_t s_readIndex  = 0U;
static volatile uint32_t s_writeIndex = 0U;

static const pdm_config_t pdmConfig = {
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
#if defined(DEMO_PDM_DFOUTPUT_GAIN)
    .gain = DEMO_PDM_DFOUTPUT_GAIN,
#else
    .gain       = kPDM_DfOutputGain7,
#endif
};
/*******************************************************************************
 * Code
 ******************************************************************************/
static void XOS_Init(void)
{
    xos_set_clock_freq(XOS_CLOCK_FREQ);
    xos_start_system_timer(-1, 0);

    /* Configure interrupts. */
    xos_register_interrupt_handler(DMA0_0_IRQn, (XosIntFunc *)DMA0_DriverIRQHandler, (void *)0U);
    xos_register_interrupt_handler(DMA0_1_IRQn, (XosIntFunc *)DMA0_DriverIRQHandler, (void *)1U);
    xos_interrupt_enable(DMA0_0_IRQn);
    xos_interrupt_enable(DMA0_1_IRQn);
}

static void BOARD_InitClock(void)
{
    CLOCK_SetXtal0Freq(BOARD_XTAL0_CLK_HZ); /* sets external XTAL OSC freq */

    CLOCK_SetIpSrc(kCLOCK_Micfil, kCLOCK_FusionMicfilClkSrcPll1Pfd2Div);
    RESET_PeripheralReset(kRESET_Micfil);

    /* Use Pll1Pfd2Div clock source 12.288MHz. */
    CLOCK_SetIpSrc(kCLOCK_Sai0, kCLOCK_Cm33SaiClkSrcPll1Pfd2Div);
    RESET_PeripheralReset(kRESET_Sai0);

    CLOCK_EnableClock(kCLOCK_Dma0Ch0);
    CLOCK_EnableClock(kCLOCK_Dma0Ch1);
}

void BOARD_SAI_RxConfig(sai_transceiver_t *config, sai_sync_mode_t sync)
{
    config->syncMode = sync;
    SAI_RxSetConfig(DEMO_SAI, config);
    SAI_RxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
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
        s_bufferValidBlock++;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    pdm_edma_transfer_t pdmXfer;
    edma_config_t dmaConfig = {0};
    sai_transfer_t saiXfer;
    sai_transceiver_t config;

    XOS_Init();
    BOARD_InitBootPins();
    BOARD_InitClock();
    BOARD_InitDebugConsole();

    xos_start_main("main", 7, 0);

    PRINTF("DSP starts on core '%s'\r\n", XCHAL_CORE_ID);
    PRINTF("MIC->DMA->I2S->CODEC running \r\n\r\n");

    /* Create EDMA handle */
    /*
     * dmaConfig.enableRoundRobinArbitration = false;
     * dmaConfig.enableHaltOnError = true;
     * dmaConfig.enableContinuousLinkMode = false;
     * dmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_DMA, &dmaConfig);
    EDMA_CreateHandle(&s_pdmDmaHandle, DEMO_DMA, DEMO_PDM_EDMA_CHANNEL);
    EDMA_CreateHandle(&s_saiDmaHandle, DEMO_DMA, DEMO_SAI_EDMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_PDM_EDMA_CHANNEL, DEMO_PDM_EDMA_SOURCE);
    EDMA_SetChannelMux(DEMO_DMA, DEMO_SAI_EDMA_CHANNEL, DEMO_SAI_EDMA_SOURCE);
#endif

    /* SAI init */
    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &s_saiTxHandle, saiCallback, NULL, &s_saiDmaHandle);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_MonoLeft, 1U << DEMO_SAI_CHANNEL);

    config.bitClock.bclkSource = DEMO_SAI_CLOCK_SOURCE;
    config.masterSlave         = DEMO_SAI_MASTER_SLAVE;
#if defined(BOARD_SAI_RXCONFIG)
    config.syncMode = DEMO_SAI_TX_SYNC_MODE;
#endif
    SAI_TransferTxSetConfigEDMA(DEMO_SAI, &s_saiTxHandle, &config);
    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
#if defined(BOARD_SAI_RXCONFIG)
    /* Configure RX to provide MCLK. */
    BOARD_SAI_RXCONFIG(&config, DEMO_SAI_RX_SYNC_MODE);
#endif
    /* master clock configurations */
    BOARD_MasterClockConfig();

    /* Setup pdm */
    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_TransferCreateHandleEDMA(DEMO_PDM, &s_pdmRxHandle, pdmCallback, NULL, &s_pdmDmaHandle);
    PDM_TransferInstallEDMATCDMemory(&s_pdmRxHandle, s_edmaTcd, 4);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
    }
    PDM_Reset(DEMO_PDM);

    while (1)
    {
        /* Wait one buffer idle to receive data */
        if (s_bufferValidBlock > 0)
        {
            pdmXfer.data         = (uint8_t *)((uint32_t)s_buffer + s_readIndex * BUFFER_SIZE);
            pdmXfer.dataSize     = BUFFER_SIZE;
            pdmXfer.linkTransfer = NULL;
            if (kStatus_Success == PDM_TransferReceiveEDMA(DEMO_PDM, &s_pdmRxHandle, &pdmXfer))
            {
                s_readIndex++;
            }
            if (s_readIndex == BUFFER_NUM)
            {
                s_readIndex = 0U;
            }
        }
        /* wait one buffer busy to send data */
        if (s_bufferValidBlock < BUFFER_NUM)
        {
            saiXfer.data     = (uint8_t *)((uint32_t)s_buffer + s_writeIndex * BUFFER_SIZE);
            saiXfer.dataSize = BUFFER_SIZE;
            if (kStatus_Success == SAI_TransferSendEDMA(DEMO_SAI, &s_saiTxHandle, &saiXfer))
            {
                s_writeIndex++;
            }
            if (s_writeIndex == BUFFER_NUM)
            {
                s_writeIndex = 0U;
            }
        }
    }
}
