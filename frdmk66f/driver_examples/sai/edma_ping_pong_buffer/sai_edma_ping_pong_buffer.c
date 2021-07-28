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
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "fsl_codec_adapter.h"
#include "fsl_dialog7212.h"
#include "fsl_sai.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SAI and I2C instance and clock */
#define DEMO_CODEC_DA7212
#define DEMO_SAI              I2S0
#define DEMO_SAI_CHANNEL      (0)
#define DEMO_SAI_CLKSRC       kCLOCK_CoreSysClk
#define DEMO_SAI_CLK_FREQ     CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define DEMO_SAI_IRQ          I2S0_Tx_IRQn
#define DEMO_SAITxIRQHandler  I2S0_Tx_IRQHandler
#define DEMO_SAI_TX_SYNC_MODE kSAI_ModeAsync
#define DEMO_SAI_RX_SYNC_MODE kSAI_ModeSync
#define DEMO_SAI_MCLK_OUTPUT  true
#define DEMO_SAI_MASTER_SLAVE kSAI_Master

#define DEMO_DMA             DMA0
#define DEMO_EDMA_CHANNEL    (0)
#define DEMO_DMAMUX          DMAMUX
#define DEMO_TX_EDMA_CHANNEL (0U)
#define DEMO_RX_EDMA_CHANNEL (1U)
#define DEMO_SAI_TX_SOURCE   kDmaRequestMux0I2S0Tx
#define DEMO_SAI_RX_SOURCE   kDmaRequestMux0I2S0Rx

#define DEMO_AUDIO_DATA_CHANNEL (2U)
#define DEMO_AUDIO_BIT_WIDTH    kSAI_WordWidth16bits
#define DEMO_AUDIO_SAMPLE_RATE  (kSAI_SampleRate16KHz)
#define DEMO_AUDIO_MASTER_CLOCK 12288000U

#define DEMO_I2C              I2C1
#define DEMO_I2C_CLKSRC       kCLOCK_BusClk
#define DEMO_I2C_CLK_FREQ     CLOCK_GetFreq(kCLOCK_BusClk)
#define I2C_RELEASE_SDA_PORT  PORTC
#define I2C_RELEASE_SCL_PORT  PORTC
#define I2C_RELEASE_SDA_GPIO  GPIOC
#define I2C_RELEASE_SDA_PIN   11U
#define I2C_RELEASE_SCL_GPIO  GPIOC
#define I2C_RELEASE_SCL_PIN   10U
#define I2C_RELEASE_BUS_COUNT 100U

#define BOARD_MASTER_CLOCK_CONFIG BOARD_MasterClockConfig
#define BUFFER_SIZE (256U)
#define BUFFER_NUM  (2)
#define PLAY_COUNT  (100)
#ifndef DEMO_CODEC_INIT_DELAY_MS
#define DEMO_CODEC_INIT_DELAY_MS (1000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_I2C_ReleaseBus(void);
void BOARD_MasterClockConfig(void);
static void DEMO_StartTXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize);
static void DEMO_StartRXPingPongBuffer(void *buffer1, void *buffer2, uint32_t bufferSize);
extern void BOARD_SAI_RXConfig(sai_transceiver_t *config, sai_sync_mode_t sync);
/*******************************************************************************
 * Variables
 ******************************************************************************/
da7212_config_t da7212Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = 60000000},
    .dacSource    = kDA7212_DACSourceInputStream,
    .slaveAddress = DA7212_ADDRESS,
    .protocol     = kDA7212_BusI2S,
    .format       = {.mclk_HZ = DEMO_AUDIO_MASTER_CLOCK, .sampleRate = 16000, .bitWidth = 16},
    .isMaster     = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_DA7212, .codecDevConfig = &da7212Config};

sai_master_clock_t mclkConfig;

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

void BOARD_MasterClockConfig(void)
{
    mclkConfig.mclkOutputEnable = true, mclkConfig.mclkHz = DEMO_AUDIO_MASTER_CLOCK;
    mclkConfig.mclkSourceClkHz = DEMO_SAI_CLK_FREQ;
    SAI_SetMasterClockConfig(DEMO_SAI, &mclkConfig);
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

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

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
