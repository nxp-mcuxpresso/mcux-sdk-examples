/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "music.h"
#if defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT
#include "fsl_dmamux.h"
#endif
#include "fsl_debug_console.h"
#include "fsl_sai_edma.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_dialog7212.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SAI              SAI1
#define DEMO_SAI_CHANNEL      0
#define DEMO_SAI_CLK_FREQ     CLOCK_GetSaiClkFreq(1U)
#define DEMO_SAI_IRQ          SAI1_IRQn
#define DEMO_SAITxIRQHandler  SAI1_IRQHandler
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI_MCLK_OUTPUT  true
#define DEMO_SAI_MASTER_SLAVE kSAI_Master

#define DEMO_DMA              DMA0
#define DEMO_EDMA_CHANNEL     0
#define DEMO_SAI_EDMA_CHANNEL kDma0RequestMuxSai1Tx

#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth16bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)

#define BOARD_SAI_RXCONFIG(config, mode)

#define DEMO_CODEC_VOLUME 60U
#define BUFFER_NUM (2U)
#ifndef DEMO_XFER_BUFFER_SIZE
#define DEMO_XFER_BUFFER_SIZE (1600U)
#endif
#ifndef DEMO_CODEC_INIT_DELAY_MS
#define DEMO_CODEC_INIT_DELAY_MS (1000U)
#endif
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 100U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_MASTER_CLOCK_CONFIG(void);
static void callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData);
extern void BOARD_SAI_RXConfig(sai_transceiver_t *config, sai_sync_mode_t sync);

/*******************************************************************************
 * Variables
 ******************************************************************************/
da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 12000000},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
    .format       = {.mclk_HZ = 12288000, .sampleRate = DEMO_AUDIO_SAMPLE_RATE, .bitWidth = DEMO_AUDIO_BIT_WIDTH},
    .sysClkSource = kDA7212_SysClkSourceMCLK,
    .isMaster     = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

sai_master_clock_t mclkConfig;

#if defined(DEMO_QUICKACCESS_SECTION_CACHEABLE) && DEMO_QUICKACCESS_SECTION_CACHEABLE
AT_NONCACHEABLE_SECTION_INIT(sai_edma_handle_t txHandle);
#else
AT_QUICKACCESS_SECTION_DATA(sai_edma_handle_t txHandle);
#endif
edma_handle_t g_dmaHandle = {0};
extern codec_config_t boardCodecConfig;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t buffer[BUFFER_NUM * DEMO_XFER_BUFFER_SIZE], 4);
volatile bool isFinished      = false;
volatile uint32_t finishIndex = 0U;
volatile uint32_t emptyBlock  = BUFFER_NUM;
#if (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && DEMO_EDMA_HAS_CHANNEL_CONFIG)
extern edma_config_t dmaConfig;
#else
edma_config_t dmaConfig = {0};
#endif

codec_handle_t codecHandle;
/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_MASTER_CLOCK_CONFIG(void)
{
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = 12288000U;
    mclkConfig.mclkSourceClkHz = 12288000U;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
}

static void callback(I2S_Type *base, sai_edma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_SAI_RxError == status)
    {
    }
    else
    {
        finishIndex++;
        emptyBlock++;
        /* Judge whether the music array is completely transfered. */
        if (MUSIC_LEN / DEMO_XFER_BUFFER_SIZE == finishIndex)
        {
            isFinished = true;
        }
    }
}

void DelayMS(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    sai_transfer_t xfer;
    uint32_t cpy_index = 0U, tx_index = 0U;
    sai_transceiver_t saiConfig;

    /* attach FRO 12M to LPFLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to LPFLEXCOMM2 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    CLOCK_EnableClock(kCLOCK_Scg);
    CLOCK_SetupFROHFClocking(48000000U);
    /* < Set up PLL1 */
    const pll_setup_t pll1Setup = {.pllctrl = SCG_SPLLCTRL_SOURCE(1U) | SCG_SPLLCTRL_SELI(3U) | SCG_SPLLCTRL_SELP(1U),
                                   .pllndiv = SCG_SPLLNDIV_NDIV(25U),
                                   .pllpdiv = SCG_SPLLPDIV_PDIV(10U),
                                   .pllmdiv = SCG_SPLLMDIV_MDIV(256U),
                                   .pllRate = 24576000U};
    CLOCK_SetPLL1Freq(&pll1Setup);
    CLOCK_SetClkDiv(kCLOCK_DivPLL1Clk0, 1U);

    /* attach PLL1 to SAI1 */
    CLOCK_SetClkDiv(kCLOCK_DivSai1Clk, 1u);
    CLOCK_AttachClk(kPLL1_CLK0_to_SAI1);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("SAI EDMA example started!\n\r");

#if (!defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) || (defined(DEMO_EDMA_HAS_CHANNEL_CONFIG) && !DEMO_EDMA_HAS_CHANNEL_CONFIG))
    /* Create EDMA handle */
    /*
     * dmaConfig.enableRoundRobinArbitration = false;
     * dmaConfig.enableHaltOnError = true;
     * dmaConfig.enableContinuousLinkMode = false;
     * dmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&dmaConfig);
#endif
    EDMA_Init(DEMO_DMA, &dmaConfig);
    EDMA_CreateHandle(&g_dmaHandle, DEMO_DMA, DEMO_EDMA_CHANNEL);
#if defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX
    EDMA_SetChannelMux(DEMO_DMA, DEMO_EDMA_CHANNEL, DEMO_SAI_EDMA_CHANNEL);
#endif

    /* SAI init */
    SAI_Init(DEMO_SAI);

    SAI_TransferTxCreateHandleEDMA(DEMO_SAI, &txHandle, callback, NULL, &g_dmaHandle);

    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, 1U << DEMO_SAI_CHANNEL);
    saiConfig.syncMode    = DEMO_SAI_TX_SYNC_MODE;
    saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;
    SAI_TransferTxSetConfigEDMA(DEMO_SAI, &txHandle, &saiConfig);
    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* sai rx configurations */
    BOARD_SAI_RXCONFIG(&saiConfig, DEMO_SAI_RX_SYNC_MODE);
    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();

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
    /* delay for codec output stable */
    DelayMS(DEMO_CODEC_INIT_DELAY_MS);

/* If need to handle audio error, enable sai interrupt */
#if defined(DEMO_SAI_IRQ)
    EnableIRQ(DEMO_SAI_IRQ);
    SAI_TxEnableInterrupts(DEMO_SAI, kSAI_FIFOErrorInterruptEnable);
#endif

    /* Waiting until finished. */
    while (!isFinished)
    {
        if ((emptyBlock > 0U) && (cpy_index < MUSIC_LEN / DEMO_XFER_BUFFER_SIZE))
        {
            /* Fill in the buffers. */
            memcpy((uint8_t *)&buffer[DEMO_XFER_BUFFER_SIZE * (cpy_index % BUFFER_NUM)],
                   (uint8_t *)&music[cpy_index * DEMO_XFER_BUFFER_SIZE], sizeof(uint8_t) * DEMO_XFER_BUFFER_SIZE);
            emptyBlock--;
            cpy_index++;
        }
        if (emptyBlock < BUFFER_NUM)
        {
            /*  xfer structure */
            xfer.data     = (uint8_t *)&buffer[DEMO_XFER_BUFFER_SIZE * (tx_index % BUFFER_NUM)];
            xfer.dataSize = DEMO_XFER_BUFFER_SIZE;
            /* Wait for available queue. */
            if (kStatus_Success == SAI_TransferSendEDMA(DEMO_SAI, &txHandle, &xfer))
            {
                tx_index++;
            }
        }
    }

    /* Once transfer finish, disable SAI instance. */
    SAI_TransferAbortSendEDMA(DEMO_SAI, &txHandle);
    SAI_Deinit(DEMO_SAI);
    PRINTF("\n\r SAI EDMA example finished!\n\r ");
    while (1)
    {
    }
}

#if defined(DEMO_SAITxIRQHandler)
void DEMO_SAITxIRQHandler(void)
{
    /* Clear the FIFO error flag */
    SAI_TxClearStatusFlags(DEMO_SAI, kSAI_FIFOErrorFlag);

    /* Reset FIFO */
    SAI_TxSoftwareReset(DEMO_SAI, kSAI_ResetTypeFIFO);
    SDK_ISR_EXIT_BARRIER;
}
#endif
