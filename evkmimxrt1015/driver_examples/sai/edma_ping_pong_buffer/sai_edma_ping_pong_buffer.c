/*
 * Copyright 2020 NXP
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
#include "fsl_wm8960.h"
#include "fsl_codec_adapter.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SAI instance and clock */
#define DEMO_CODEC_WM8960
#define DEMO_SAI              SAI1
#define DEMO_SAI_CHANNEL      (0)
#define DEMO_SAI_IRQ          SAI1_IRQn
#define DEMO_SAITxIRQHandler  SAI1_IRQHandler
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI_MCLK_OUTPUT  true
#define DEMO_SAI_MASTER_SLAVE kSAI_Master

#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth16bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ

/* IRQ */
#define DEMO_SAI_TX_IRQ SAI1_IRQn
#define DEMO_SAI_RX_IRQ SAI1_IRQn

/* DMA */
#define DEMO_DMA             DMA0
#define DEMO_DMAMUX          DMAMUX
#define DEMO_TX_EDMA_CHANNEL (0U)
#define DEMO_RX_EDMA_CHANNEL (1U)
#define DEMO_SAI_TX_SOURCE   kDmaRequestMuxSai1Tx
#define DEMO_SAI_RX_SOURCE   kDmaRequestMuxSai1Rx

/* Select Audio/Video PLL (786.48 MHz) as sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_SELECT (2U)
/* Clock pre divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER (0U)
/* Clock divider for sai1 clock source */
#define DEMO_SAI1_CLOCK_SOURCE_DIVIDER (63U)
/* Get frequency of sai1 clock */
#define DEMO_SAI_CLK_FREQ                                                        \
    (CLOCK_GetFreq(kCLOCK_AudioPllClk) / (DEMO_SAI1_CLOCK_SOURCE_DIVIDER + 1U) / \
     (DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER + 1U))

/* I2C instance and clock */
#define DEMO_I2C LPI2C1

/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define DEMO_LPI2C_CLOCK_SOURCE_DIVIDER (5U)
/* Get frequency of lpi2c clock */
#define DEMO_I2C_CLK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (DEMO_LPI2C_CLOCK_SOURCE_DIVIDER + 1U))

#define BOARD_MASTER_CLOCK_CONFIG()
#define BUFFER_SIZE (256U)
#define BUFFER_NUM  (2)
#define PLAY_COUNT  (100)
#ifndef DEMO_CODEC_INIT_DELAY_MS
#define DEMO_CODEC_INIT_DELAY_MS (1000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_StartTXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize);
static void DEMO_StartRXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize);
extern void BOARD_SAI_RXConfig(sai_transceiver_t *config, sai_sync_mode_t sync);
/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8960_config_t wm8960Config = {
    .i2cConfig = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .route     = kWM8960_RoutePlaybackandRecord,
    .leftInputSource  = kWM8960_InputDifferentialMicInput3,
    .rightInputSource = kWM8960_InputDifferentialMicInput2,
    .playSource       = kWM8960_PlaySourceDAC,
    .slaveAddress     = WM8960_I2C_ADDR,
    .bus              = kWM8960_BusI2S,
    .format = {.mclk_HZ = 6144000U, .sampleRate = kWM8960_AudioSampleRate16KHz, .bitWidth = kWM8960_AudioBitWidth16bit},
    .master_slave = false,
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
static edma_handle_t s_dmaTxHandle = {0};
static edma_handle_t s_dmaRxHandle = {0};
extern codec_config_t boardCodecConfig;
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_buffer[BUFFER_NUM][BUFFER_SIZE], 4);
volatile bool isFinished      = false;
volatile uint32_t finishIndex = 0U;
volatile uint32_t emptyBlock  = BUFFER_NUM;
AT_NONCACHEABLE_SECTION_ALIGN(static edma_tcd_t s_emdaTcd[4], 32);
static volatile bool s_Transfer_Done = false;
static volatile uint32_t s_playIndex = 0U;
static volatile uint32_t s_playCount = 0U;
static sai_transceiver_t s_saiConfig;
static codec_handle_t s_codecHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_EnableSaiMclkOutput(bool enable)
{
    if (enable)
    {
        IOMUXC_GPR->GPR1 |= IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK;
    }
    else
    {
        IOMUXC_GPR->GPR1 &= (~IOMUXC_GPR_GPR1_SAI1_MCLK_DIR_MASK);
    }
}

void DelayMS(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++)
    {
        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}

void EDMA_RX_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    __NOP();
}

void EDMA_TX_Callback(edma_handle_t *handle, void *param, bool transferDone, uint32_t tcds)
{
    __NOP();
}

/*!
 * @brief Main function
 */
int main(void)
{
    edma_config_t dmaConfig = {0};

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    CLOCK_InitAudioPll(&audioPllConfig);
    BOARD_InitDebugConsole();

    /*Clock setting for LPI2C*/
    CLOCK_SetMux(kCLOCK_Lpi2cMux, DEMO_LPI2C_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, DEMO_LPI2C_CLOCK_SOURCE_DIVIDER);

    /*Clock setting for SAI1*/
    CLOCK_SetMux(kCLOCK_Sai1Mux, DEMO_SAI1_CLOCK_SOURCE_SELECT);
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, DEMO_SAI1_CLOCK_SOURCE_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Sai1Div, DEMO_SAI1_CLOCK_SOURCE_DIVIDER);

    /*Enable MCLK clock*/
    BOARD_EnableSaiMclkOutput(true);

    /* Init DMAMUX */
    DMAMUX_Init(DEMO_DMAMUX);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_TX_EDMA_CHANNEL, (uint8_t)DEMO_SAI_TX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_TX_EDMA_CHANNEL);
    DMAMUX_SetSource(DEMO_DMAMUX, DEMO_RX_EDMA_CHANNEL, (uint8_t)DEMO_SAI_RX_SOURCE);
    DMAMUX_EnableChannel(DEMO_DMAMUX, DEMO_RX_EDMA_CHANNEL);

    PRINTF("SAI EDMA ping pong buffer example started!\n\r");

    memset(s_buffer, 0, BUFFER_NUM * BUFFER_SIZE);

    /* Create EDMA handle */
    /*
     * dmaConfig.enableRoundRobinArbitration = false;
     * dmaConfig.enableHaltOnError = true;
     * dmaConfig.enableContinuousLinkMode = false;
     * dmaConfig.enableDebugMode = false;
     */
    EDMA_GetDefaultConfig(&dmaConfig);
    EDMA_Init(DEMO_DMA, &dmaConfig);
    EDMA_CreateHandle(&s_dmaTxHandle, DEMO_DMA, DEMO_TX_EDMA_CHANNEL);
    EDMA_CreateHandle(&s_dmaRxHandle, DEMO_DMA, DEMO_RX_EDMA_CHANNEL);
    EDMA_SetCallback(&s_dmaTxHandle, EDMA_TX_Callback, NULL);
    EDMA_SetCallback(&s_dmaRxHandle, EDMA_RX_Callback, NULL);
    EDMA_ResetChannel(DEMO_DMA, DEMO_TX_EDMA_CHANNEL);
    EDMA_ResetChannel(DEMO_DMA, DEMO_RX_EDMA_CHANNEL);

    /* SAI init */
    SAI_Init(DEMO_SAI);

    /* I2S mode configurations */
    SAI_GetClassicI2SConfig(&s_saiConfig, DEMO_AUDIO_BIT_WIDTH, kSAI_Stereo, 1U << DEMO_SAI_CHANNEL);
    s_saiConfig.syncMode    = DEMO_SAI_TX_SYNC_MODE;
    s_saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;
    SAI_TxSetConfig(DEMO_SAI, &s_saiConfig);
    s_saiConfig.masterSlave = DEMO_SAI_MASTER_SLAVE;
    s_saiConfig.syncMode    = DEMO_SAI_RX_SYNC_MODE;
    SAI_RxSetConfig(DEMO_SAI, &s_saiConfig);

    /* set bit clock divider */
    SAI_TxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);
    SAI_RxSetBitClockRate(DEMO_SAI, DEMO_AUDIO_MASTER_CLOCK, DEMO_AUDIO_SAMPLE_RATE, DEMO_AUDIO_BIT_WIDTH,
                          DEMO_AUDIO_DATA_CHANNEL);

    /* master clock configurations */
    BOARD_MASTER_CLOCK_CONFIG();

    /* Use default setting to init codec */
    if (CODEC_Init(&s_codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        assert(false);
    }

    /* delay for codec output stable */
    DelayMS(DEMO_CODEC_INIT_DELAY_MS);

    DEMO_StartRXPingPongBuffer(&s_buffer[0], &s_buffer[1], BUFFER_SIZE);
    DEMO_StartTXPingPongBuffer(&s_buffer[1], &s_buffer[0], BUFFER_SIZE);

    while (1)
    {
    }
}

static void DEMO_StartRXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize)
{
    edma_transfer_config_t transferConfig = {0};
    uint32_t srcAddr                      = SAI_RxGetDataRegisterAddress(DEMO_SAI, DEMO_SAI_CHANNEL);

    /* Configure and submit transfer structure 1 */
    EDMA_PrepareTransfer(&transferConfig, (void *)srcAddr, DEMO_AUDIO_BIT_WIDTH / 8U, buffer1,
                         DEMO_AUDIO_BIT_WIDTH / 8U,
                         (FSL_FEATURE_SAI_FIFO_COUNT - s_saiConfig.fifo.fifoWatermark) * (DEMO_AUDIO_BIT_WIDTH / 8U),
                         bufferSize, kEDMA_PeripheralToMemory);

    EDMA_TcdSetTransferConfig(&s_emdaTcd[2], &transferConfig, &s_emdaTcd[3]);
    EDMA_PrepareTransfer(&transferConfig, (void *)srcAddr, DEMO_AUDIO_BIT_WIDTH / 8U, buffer2,
                         DEMO_AUDIO_BIT_WIDTH / 8U,
                         (FSL_FEATURE_SAI_FIFO_COUNT - s_saiConfig.fifo.fifoWatermark) * (DEMO_AUDIO_BIT_WIDTH / 8U),
                         bufferSize, kEDMA_PeripheralToMemory);
    EDMA_TcdSetTransferConfig(&s_emdaTcd[3], &transferConfig, &s_emdaTcd[2]);
    EDMA_TcdEnableInterrupts(&s_emdaTcd[2], kEDMA_MajorInterruptEnable);
    EDMA_TcdEnableInterrupts(&s_emdaTcd[3], kEDMA_MajorInterruptEnable);
    EDMA_InstallTCD(DEMO_DMA, s_dmaRxHandle.channel, &s_emdaTcd[2]);
    EDMA_StartTransfer(&s_dmaRxHandle);
    /* Enable DMA enable bit */
    SAI_RxEnableDMA(DEMO_SAI, kSAI_FIFORequestDMAEnable, true);
    /* Enable SAI Tx clock */
    SAI_RxEnable(DEMO_SAI, true);
    /* Enable the channel FIFO */
    SAI_RxSetChannelFIFOMask(DEMO_SAI, 1U << DEMO_SAI_CHANNEL);
}

static void DEMO_StartTXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize)
{
    edma_transfer_config_t transferConfig = {0};
    uint32_t destAddr                     = SAI_TxGetDataRegisterAddress(DEMO_SAI, DEMO_SAI_CHANNEL);

    /* Configure and submit transfer structure 1 */
    EDMA_PrepareTransfer(&transferConfig, buffer1, DEMO_AUDIO_BIT_WIDTH / 8U, (void *)destAddr,
                         DEMO_AUDIO_BIT_WIDTH / 8U,
                         (FSL_FEATURE_SAI_FIFO_COUNT - s_saiConfig.fifo.fifoWatermark) * (DEMO_AUDIO_BIT_WIDTH / 8U),
                         bufferSize, kEDMA_MemoryToPeripheral);

    EDMA_TcdSetTransferConfig(&s_emdaTcd[0], &transferConfig, &s_emdaTcd[1]);
    EDMA_PrepareTransfer(&transferConfig, buffer2, DEMO_AUDIO_BIT_WIDTH / 8U, (void *)destAddr,
                         DEMO_AUDIO_BIT_WIDTH / 8U,
                         (FSL_FEATURE_SAI_FIFO_COUNT - s_saiConfig.fifo.fifoWatermark) * (DEMO_AUDIO_BIT_WIDTH / 8U),
                         bufferSize, kEDMA_MemoryToPeripheral);
    EDMA_TcdSetTransferConfig(&s_emdaTcd[1], &transferConfig, &s_emdaTcd[0]);
    EDMA_TcdEnableInterrupts(&s_emdaTcd[0], kEDMA_MajorInterruptEnable);
    EDMA_TcdEnableInterrupts(&s_emdaTcd[1], kEDMA_MajorInterruptEnable);
    EDMA_InstallTCD(DEMO_DMA, s_dmaTxHandle.channel, &s_emdaTcd[0]);
    EDMA_StartTransfer(&s_dmaTxHandle);
    /* Enable DMA enable bit */
    SAI_TxEnableDMA(DEMO_SAI, kSAI_FIFORequestDMAEnable, true);
    /* Enable SAI Tx clock */
    SAI_TxEnable(DEMO_SAI, true);
    /* Enable the channel FIFO */
    SAI_TxSetChannelFIFOMask(DEMO_SAI, 1U << DEMO_SAI_CHANNEL);
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
