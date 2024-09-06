/*
 * Copyright 2018-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <xtensa/config/core.h>
#include <xtensa/xos.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
#include "fsl_gpio.h"
#include "fsl_sai_edma.h"
#include "fsl_pdm_edma.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_inputmux.h"
#include "dsp_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PDM                      PDM
#define DEMO_PDM_CLK_FREQ             CLOCK_GetMicfilClkFreq()
#define DEMO_PDM_FIFO_WATERMARK       (FSL_FEATURE_PDM_FIFO_DEPTH / 2U)
#define DEMO_PDM_QUALITY_MODE         kPDM_QualityModeHigh
#define DEMO_PDM_CHANNEL_GAIN         (kPDM_DfOutputGain5)
#define DEMO_PDM_CIC_OVERSAMPLE_RATE  (16U)
#define DEMO_PDM_ENABLE_CHANNEL_LEFT  (0U)
#define DEMO_PDM_ENABLE_CHANNEL_RIGHT (1U)

/* SAI instance and clock */
#define DEMO_SAI                SAI0
#define DEMO_SAI_CHANNEL        (0)
#define DEMO_SAI_TX_SYNC_MODE   kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE   kSAI_ModeSync
#define DEMO_SAI_MASTER_SLAVE   kSAI_Master
#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth32bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#define DEMO_SAI_CLOCK_SOURCE   kSAI_BclkSourceMclkDiv

#define DEMO_DMA              DMA0
#define DEMO_PDM_EDMA_CHANNEL 0
#define DEMO_SAI_EDMA_CHANNEL 1
#define DEMO_PDM_EDMA_SOURCE  kDmaRequestMuxMicfil
#define DEMO_SAI_EDMA_SOURCE  kDmaRequestMuxSai0Tx

/* Get frequency of sai clock */
#define DEMO_SAI_CLK_FREQ CLOCK_GetSaiClkFreq()

/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ 24000000U /* CLOCK_GetLPI2cClkFreq(2) */
#define BOARD_SAI_RXCONFIG(config, mode)

#define DEMO_QUICKACCESS_SECTION_CACHEABLE 1U
#define BUFFER_SIZE                        (1024)
#define BUFFER_NUM                         (4U)

/*******************************************************************************
 * Variables
 ******************************************************************************/
// extern int NonCacheable_start, NonCacheable_end;
// extern int NonCacheable_init_start, NonCacheable_init_end;

static sai_config_t tx_config;
AT_NONCACHEABLE_SECTION_ALIGN(pdm_edma_handle_t s_pdmRxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_pdmDmaHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_handle_t s_saiDmaHandle, 4);
#if defined(DEMO_QUICKACCESS_SECTION_CACHEABLE) && DEMO_QUICKACCESS_SECTION_CACHEABLE
AT_NONCACHEABLE_SECTION_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
AT_NONCACHEABLE_SECTION_ALIGN(edma_tcd_t s_edmaTcd[BUFFER_NUM], 32U);
#else
AT_QUICKACCESS_SECTION_DATA_ALIGN(sai_edma_handle_t s_saiTxHandle, 4);
AT_QUICKACCESS_SECTION_DATA_ALIGN(edma_tcd_t s_edmaTcd[4], 32U);
#endif
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t s_buffer[BUFFER_SIZE * BUFFER_NUM], 32);

static volatile uint32_t s_bufferValidBlock = BUFFER_NUM;
static volatile uint32_t s_readIndex        = 0U;
static volatile uint32_t s_writeIndex       = 0U;

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
    .gain       = kPDM_DfOutputGain7,
#endif
};


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_LoopbackFunc(void);


/*******************************************************************************
 * Code
 ******************************************************************************/
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

static void XOS_Init(void)
{
    xos_set_clock_freq(XOS_CLOCK_FREQ);
    xos_start_system_timer(-1, 0);
    /* Map DMA IRQ handler to INPUTMUX selection DSP_INT0_SEL18
     * EXTINT18 = DSP INT 23 */
    xos_register_interrupt_handler(XCHAL_EXTINT18_NUM, (XosIntFunc *)EDMA_HandleIRQ, &s_saiDmaHandle);
    xos_interrupt_enable(XCHAL_EXTINT18_NUM);
    xos_register_interrupt_handler(XCHAL_EXTINT17_NUM, (XosIntFunc *)EDMA_HandleIRQ, &s_pdmDmaHandle);
    xos_interrupt_enable(XCHAL_EXTINT17_NUM);
}

static void BOARD_Init_DMA(void)
{
    edma_config_t dmaConfig = {0};
    RESET_ClearPeripheralReset(kDMA0_RST_SHIFT_RSTn);
    EDMA_EnableRequest(DEMO_DMA, DEMO_PDM_EDMA_SOURCE);
    EDMA_EnableRequest(DEMO_DMA, DEMO_SAI_EDMA_SOURCE);

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
}

static void BOARD_Init_PDM(void)
{
    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_MICFIL0);
    CLOCK_SetClkDiv(kCLOCK_DivMicfil0Clk, 15U);

    RESET_ClearPeripheralReset(kPDM_RST_SHIFT_RSTn);

    PDM_Init(DEMO_PDM, &pdmConfig);
    PDM_TransferCreateHandleEDMA(DEMO_PDM, &s_pdmRxHandle, pdmCallback, NULL, &s_pdmDmaHandle);
    PDM_TransferInstallEDMATCDMemory(&s_pdmRxHandle, s_edmaTcd, 4);
    PDM_TransferSetChannelConfigEDMA(DEMO_PDM, &s_pdmRxHandle, DEMO_PDM_ENABLE_CHANNEL_LEFT, &channelConfig);
    if (PDM_SetSampleRateConfig(DEMO_PDM, DEMO_PDM_CLK_FREQ, DEMO_AUDIO_SAMPLE_RATE) != kStatus_Success)
    {
        PRINTF("PDM configure sample rate failed.\r\n");
        return;
    }
    PDM_Reset(DEMO_PDM);
}

static void BOARD_Init_SAI(void)
{
    sai_transceiver_t config;

    /* SAI clock 368.64 / 15 = 24.576MHz */
    CLOCK_AttachClk(kAUDIO_PLL_PFD3_to_AUDIO_VDD2);
    CLOCK_AttachClk(kAUDIO_VDD2_to_SAI012);
    CLOCK_SetClkDiv(kCLOCK_DivSai012Clk, 15U);

    RESET_ClearPeripheralReset(kSAI0_RST_SHIFT_RSTn);

    SYSCON0->SAI0_MCLK_CTRL |= SYSCON0_SAI0_MCLK_CTRL_SAIMCLKDIR_MASK;

    SAI_Init(DEMO_SAI);
    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &s_saiTxHandle, saiCallback, NULL, &s_saiDmaHandle);
    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&config, DEMO_AUDIO_BIT_WIDTH, kSAI_MonoLeft, 1U << DEMO_SAI_CHANNEL);

    config.bitClock.bclkSource = DEMO_SAI_CLOCK_SOURCE;
    config.masterSlave         = DEMO_SAI_MASTER_SLAVE;
#if defined BOARD_SAI_RXCONFIG
    config.syncMode = DEMO_SAI_TX_SYNC_MODE;
#endif

    SAI_TransferTxSetConfigEDMA(DEMO_SAI, &s_saiTxHandle, &config);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
}

void BOARD_MasterClockConfig(void)
{
    sai_master_clock_t mclkConfig;
    mclkConfig.mclkOutputEnable = true;
    mclkConfig.mclkHz           = 24576000U;
    mclkConfig.mclkSourceClkHz  = 24576000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}


void BOARD_LoopbackFunc()
{
    pdm_edma_transfer_t pdmXfer;
    sai_transfer_t saiXfer;
    PRINTF("[DSP Main] PDM->DMA->SAI->CODEC running \r\n\r\n");
    while (1)
    {
        /* wait one buffer idle to recieve data */
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

/*!
 * @brief Main function
 */
int main(void)
{
    xos_start_main("main", 7, 0);

    /* Disable DSP cache for noncacheable sections. */
    xthal_set_region_attribute((uint32_t *)0x20400000, 30000, XCHAL_CA_BYPASS, 0);
    //    xthal_set_region_attribute((uint32_t *)&NonCacheable_init_start,
    //                               (uint32_t)&NonCacheable_init_end - (uint32_t)&NonCacheable_init_start,
    //                               XCHAL_CA_BYPASS, 0);
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ); /* Note: need tell clock driver the frequency of OSC. */
    INPUTMUX_Init(INPUTMUX0);
    INPUTMUX_AttachSignal(INPUTMUX0, 17, kINPUTMUX_Dma0Irq0ToDspInterrupt);
    INPUTMUX_AttachSignal(INPUTMUX0, 18, kINPUTMUX_Dma0Irq1ToDspInterrupt);
    BOARD_InitBootPins();
    BOARD_InitDebugConsole();
    XOS_Init();

    /* Initialize DMA. */
    BOARD_Init_DMA();

    /* Initialize PDM */
    BOARD_Init_PDM();

    /* Initialize SAI */
    BOARD_Init_SAI();

    BOARD_MasterClockConfig();

    PRINTF("[DSP Main] DSP starts on core '%s'\r\n", XCHAL_CORE_ID);

    BOARD_LoopbackFunc();
}
