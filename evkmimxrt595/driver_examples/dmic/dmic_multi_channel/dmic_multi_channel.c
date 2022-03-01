/*
 * Copyright 2018 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_codec_common.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_dmic.h"
#include "fsl_dmic_dma.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_inputmux.h"
#include <stdbool.h>
#include "fsl_codec_adapter.h"
#include "fsl_cs42448.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_I2S_TX          (I2S5)
#define DEMO_I2S_SAMPLE_RATE 48000
#define I2S_CLOCK_DIVIDER    (24576000 / DEMO_I2S_SAMPLE_RATE / 32 / 8)

#define DEMO_ENABLE_DMIC_0 1
#define DEMO_ENABLE_DMIC_1 1
#define DEMO_ENABLE_DMIC_2 1
#define DEMO_ENABLE_DMIC_3 1
#define DEMO_ENABLE_DMIC_4 1
#define DEMO_ENABLE_DMIC_5 1
#define DEMO_ENABLE_DMIC_6 1
#define DEMO_ENABLE_DMIC_7 1

#define DEMO_DMA_MEMCPY_CHANNEL_0 0
#define DEMO_DMA_MEMCPY_CHANNEL_1 1

#define DEMO_DMIC_NUMS 8U

#define DEMO_DMA_CHANNEL_TRIGGER_INPUT_A  kINPUTMUX_Dma0TrigOutAToDma0
#define DEMO_DMA_CHANNEL_TRIGGER_INPUT_B  kINPUTMUX_Dma0TrigOutBToDma0
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT_A 0
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT_B 1
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT0  kINPUTMUX_Dma0OtrigChannel0ToTriginChannels
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT1  kINPUTMUX_Dma0OtrigChannel1ToTriginChannels

#define DEMO_DMA            (DMA0)
#define DEMO_DMA1           (DMA1)
#define DEMO_I2S_TX_CHANNEL (11)

#define DEMO_DMIC_DMA_RX_CHANNEL_0 16U
#define DEMO_DMIC_DMA_RX_CHANNEL_1 17U
#define DEMO_DMIC_DMA_RX_CHANNEL_2 18U
#define DEMO_DMIC_DMA_RX_CHANNEL_3 19U
#define DEMO_DMIC_DMA_RX_CHANNEL_4 20U
#define DEMO_DMIC_DMA_RX_CHANNEL_5 21U
#define DEMO_DMIC_DMA_RX_CHANNEL_6 22U
#define DEMO_DMIC_DMA_RX_CHANNEL_7 23U

#define DEMO_DMIC_CHANNEL_0          kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_1          kDMIC_Channel1
#define DEMO_DMIC_CHANNEL_2          kDMIC_Channel2
#define DEMO_DMIC_CHANNEL_3          kDMIC_Channel3
#define DEMO_DMIC_CHANNEL_4          kDMIC_Channel4
#define DEMO_DMIC_CHANNEL_5          kDMIC_Channel5
#define DEMO_DMIC_CHANNEL_6          kDMIC_Channel6
#define DEMO_DMIC_CHANNEL_7          kDMIC_Channel7
#define DEMO_TDM_DATA_START_POSITION 1U

#define DEMO_CODEC_I2C_INSTANCE 2U
#define FIFO_DEPTH           (15U)
#define PLAYBACK_BUFFER_SIZE (1024)
#define PLAYBACK_BUFFER_NUM  (2U)
#define RECORD_BUFFER_SIZE   (64)
#define RECORD_BUFFER_NUM    (DEMO_DMIC_NUMS * PLAYBACK_BUFFER_NUM)
#if DEMO_ENABLE_DMIC_0
#define DEMO_DMIC_MAX_ID 0
#endif
#if DEMO_ENABLE_DMIC_1
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 1
#endif
#if DEMO_ENABLE_DMIC_2
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 2
#endif
#if DEMO_ENABLE_DMIC_3
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 3
#endif
#if DEMO_ENABLE_DMIC_4
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 4
#endif
#if DEMO_ENABLE_DMIC_5
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 5
#endif
#if DEMO_ENABLE_DMIC_6
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 6
#endif
#if DEMO_ENABLE_DMIC_7
#undef DEMO_DMIC_MAX_ID
#define DEMO_DMIC_MAX_ID 7
#endif
#ifndef DEMO_DMIC_MAX_ID
#error "Please enable at least one DMIC"
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BORAD_CodecReset(bool state);

static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode);
static void DEMO_DMAChannelConfigurations(void);
static void DEMO_RecordDMIC(void);
static void DEMO_DMICChannelConfigurations(void);
#ifdef DMIC_FLASH_TARGET
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize,
                                   uint32_t startChannel);
#else
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize);
#endif
static void DEMO_InitCodec(void);
extern void BORAD_CodecReset(bool state);
static void DEMO_DMAMemcpyChannelConfigurations(void);
#ifdef DMIC_FLASH_TARGET
static void dmic_Callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode);
#else
static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42448_config_t cs42448Config = {
    .DACMode      = kCS42448_ModeSlave,
    .ADCMode      = kCS42448_ModeSlave,
    .reset        = NULL,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42448_BusTDM,
    .slaveAddress = CS42448_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42448, .codecDevConfig = &cs42448Config};
static i2s_config_t tx_config;
static uint32_t volatile s_RecordEmptyBlock = PLAYBACK_BUFFER_NUM;
/* DMIC dma handle for 8 channel */
#ifndef DMIC_FLASH_TARGET
static dmic_dma_handle_t s_dmicDmaHandle[DEMO_DMIC_NUMS];
#endif
/* dma handle for 8 channel */
static dma_handle_t s_dmaHandle[DEMO_DMIC_NUMS];
/* i2s dma handle */
static dma_handle_t s_i2sTxDmaHandle;
static dma_handle_t s_memcpyDmaHandle0;
static dma_handle_t s_memcpyDmaHandle1;
static i2s_dma_handle_t s_i2sTxHandle;
/* ping pong descriptor */
SDK_ALIGN(dma_descriptor_t s_dmaDescriptorPingpong[RECORD_BUFFER_NUM], 16);
SDK_ALIGN(dma_descriptor_t s_memcpyDescriptor[PLAYBACK_BUFFER_NUM * 2U], 16);
/* dmic transfer configurations */
static dmic_transfer_t s_receiveXfer0[RECORD_BUFFER_NUM / 2];
static dmic_transfer_t s_receiveXfer1[RECORD_BUFFER_NUM / 2];
/* dmic channel configurations */
static dmic_channel_config_t s_dmicChannelConfig = {
    .divhfclk            = kDMIC_PdmDiv1,
    .osr                 = 32U,
    .gainshft            = 3U,
    .preac2coef          = kDMIC_CompValueZero,
    .preac4coef          = kDMIC_CompValueZero,
    .dc_cut_level        = kDMIC_DcCut155,
    .post_dc_gain_reduce = 1U,
    .saturate16bit       = 1U,
    .sample_rate         = kDMIC_PhyFullSpeed,
};

/*!@breif data buffer */
SDK_ALIGN(static uint8_t s_processedAudio[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM * 4U], 4U);
SDK_ALIGN(static uint8_t s_recordBuffer0[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM / 2], 4U);
SDK_ALIGN(static uint8_t s_recordBuffer1[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM / 2], 4U);

static volatile bool s_buffer1Ready      = false;
static volatile bool s_i2sTransferFinish = false;
static codec_handle_t s_codecHandle;
extern codec_config_t boardCodecConfig;
static volatile uint8_t s_dmicBufferIndex = 0U;
static volatile uint8_t s_m2mBufferIndex  = 0U;
static volatile uint8_t s_m2mBufferIndex1 = 0U;
static volatile bool s_isDMICTriggerred   = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */

int main(void)
{
    i2s_transfer_t i2sTxTransfer;

    /* Board pin, clock, debug console init */
    gpio_pin_config_t gpio_config = {
        kGPIO_DigitalOutput,
        1,
    };
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch0ToDmac0Ch16RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch1ToDmac0Ch17RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch2ToDmac0Ch18RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch3ToDmac0Ch19RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch4ToDmac0Ch20RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch5ToDmac0Ch21RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch6ToDmac0Ch22RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Dmic0Ch7ToDmac0Ch23RequestEna, true);
    INPUTMUX_EnableSignal(INPUTMUX, kINPUTMUX_Flexcomm5TxToDmac0Ch11RequestEna, true);
    INPUTMUX_Deinit(INPUTMUX);

    /* attach main clock to I2C */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to FLEXCOMM5 (I2S5) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM5);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 2);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;
    /* DMIC source from audio pll, divider 8, 24.576M/8=3.072MHZ */
    CLOCK_AttachClk(kAUDIO_PLL_to_DMIC);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8);

    cs42448Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexcommClkFreq(2);
    cs42448Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

    RESET_ClearPeripheralReset(kHSGPIO1_RST_SHIFT_RSTn);
    GPIO_PortInit(GPIO, 1);
    GPIO_PinInit(GPIO, 1, 6, &gpio_config);

    PRINTF("DMIC multi channel example.\r\n");

    DEMO_DMAChannelConfigurations();
    DEMO_DMICChannelConfigurations();

    /* i2s configurations */
    /*
     * masterSlave = kI2S_MasterSlaveNormalMaster;
     * mode = kI2S_ModeI2sClassic;
     * rightLow = false;
     * leftJust = false;
     * pdmData = false;
     * sckPol = false;
     * wsPol = false;
     * divider = 1;
     * oneChannel = false;
     * dataLength = 16;
     * frameLength = 32;
     * position = 0;
     * fifoLevel = 4;
     */
    I2S_TxGetDefaultConfig(&tx_config);
    tx_config.divider     = I2S_CLOCK_DIVIDER;
    tx_config.mode        = kI2S_ModeDspWsShort;
    tx_config.wsPol       = true;
    tx_config.dataLength  = 32U;
    tx_config.frameLength = 32 * 8U;
    tx_config.position    = DEMO_TDM_DATA_START_POSITION;

    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel1, false, 64 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel2, false, 128 + DEMO_TDM_DATA_START_POSITION);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel3, false, 192 + DEMO_TDM_DATA_START_POSITION);

    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_Callback, NULL);
    DEMO_RecordDMIC();
    DEMO_DMAMemcpyChannelConfigurations();
    /* init CS42888 */
    DEMO_InitCodec();
    PRINTF("\r\nStart play audio data\r\n");
    while (1)
    {
        if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
        {
            i2sTxTransfer.data     = s_processedAudio + (s_m2mBufferIndex == 0U ? 0U : 1U) * PLAYBACK_BUFFER_SIZE;
            i2sTxTransfer.dataSize = PLAYBACK_BUFFER_SIZE;
            I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, i2sTxTransfer);
        }

        if ((s_isDMICTriggerred))
        {
            s_isDMICTriggerred = false;
            DMA_StartTransfer(&s_memcpyDmaHandle0);
            DMA_StartTransfer(&s_memcpyDmaHandle1);
        }
    }
}

static void DEMO_DMAChannelConfigurations(void)
{
    uint32_t i = 0;

    /* DMA configurations */
    DMA_Init(DEMO_DMA);

    for (i = 0; i < DEMO_DMIC_NUMS; i++)
    {
        /* configurations for DMIC channel */
        DMA_EnableChannel(DEMO_DMA, i + DEMO_DMIC_DMA_RX_CHANNEL_0);
        DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i, kDMA_ChannelPriority0);
        DMA_CreateHandle(&s_dmaHandle[i], DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i);
    }
#ifdef DMIC_FLASH_TARGET
    DMA_SetCallback(&s_dmaHandle[DEMO_DMIC_NUMS - 1U], dmic_Callback, NULL);
#endif
    /* configurations for I2S channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    /* configurations for memcpy channel channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0, kDMA_ChannelPriority2);
    DMA_CreateHandle(&s_memcpyDmaHandle0, DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetCallback(&s_memcpyDmaHandle0, NULL, NULL);

    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_memcpyDmaHandle1, DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_1);
    DMA_SetCallback(&s_memcpyDmaHandle1, memcpy_channel_callback, NULL);

    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_1);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_2);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_3);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_4);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_5);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_6);
    DMA_DisableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_7);

    /* Only enable the maximum dmic channel's dma interrupt */
#if DEMO_DMIC_MAX_ID == 0
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0);
#endif
#if DEMO_DMIC_MAX_ID == 1
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_1);
#endif
#if DEMO_DMIC_MAX_ID == 2
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_2);
#endif
#if DEMO_DMIC_MAX_ID == 3
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_3);
#endif
#if DEMO_DMIC_MAX_ID == 4
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_4);
#endif
#if DEMO_DMIC_MAX_ID == 5
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_5);
#endif
#if DEMO_DMIC_MAX_ID == 6
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_6);
#endif
#if DEMO_DMIC_MAX_ID == 7
    DMA_EnableChannelInterrupts(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_7);
#endif
}

static void DEMO_DMAMemcpyChannelConfigurations(void)
{
    DMA_SetupDescriptor(&(s_memcpyDescriptor[0]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        s_recordBuffer0, &s_processedAudio[2], &(s_memcpyDescriptor[1]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[1]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        &s_recordBuffer0[RECORD_BUFFER_SIZE * sizeof(uint32_t)],
                        &s_processedAudio[RECORD_BUFFER_SIZE * sizeof(uint32_t) * 4U + 2], &(s_memcpyDescriptor[0]));

    /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
    DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle0, &(s_memcpyDescriptor[0]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[2]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        s_recordBuffer1, &s_processedAudio[6], &(s_memcpyDescriptor[3]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[3]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        &s_recordBuffer1[RECORD_BUFFER_SIZE * sizeof(uint32_t)],
                        &s_processedAudio[RECORD_BUFFER_SIZE * sizeof(uint32_t) * 4U + 6], &(s_memcpyDescriptor[2]));

    /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
    DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle1, &(s_memcpyDescriptor[2]));
}

static void DEMO_DMICChannelConfigurations(void)
{
    uint32_t i = 0;

    /* dmic channel configurations */
    DMIC_Init(DMIC0);
    DMIC_Use2fs(DMIC0, true);
    bool rightChannel = false;

    for (i = 0U; i < DEMO_DMIC_NUMS; i++)
    {
        if (i % 2)
        {
            rightChannel = true;
        }
        else
        {
            rightChannel = false;
        }

        DMIC_ConfigChannel(DMIC0, (dmic_channel_t)i, (stereo_side_t)rightChannel, &s_dmicChannelConfig);
        DMIC_FifoChannel(DMIC0, i, FIFO_DEPTH, true, true);
#ifndef DMIC_FLASH_TARGET
        DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle[i], dmic_Callback, NULL, &s_dmaHandle[i]);
        DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle[i], &s_dmaDescriptorPingpong[i * PLAYBACK_BUFFER_NUM],
                                        PLAYBACK_BUFFER_NUM);
#endif
    }
#ifdef DMIC_FLASH_TARGET
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_1);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_2);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_3);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_4);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_5);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_6);
    DMA_EnableChannelPeriphRq(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_7);

    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_0, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_1, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_2, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_3, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_4, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_5, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_6, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_7, true);
#else
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH0(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH1(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH2(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH3(1));

    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH4(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH5(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH6(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH7(1));
#endif
}

static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode)
{
    if (s_RecordEmptyBlock)
    {
        s_RecordEmptyBlock--;
    }
    s_m2mBufferIndex = s_m2mBufferIndex == 0U ? 1U : 0U;
}

#ifdef DMIC_FLASH_TARGET
static void dmic_Callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode)
{
    s_dmicBufferIndex  = s_dmicBufferIndex == 0U ? 1U : 0U;
    s_isDMICTriggerred = true;
}
#else
static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_DMIC_Idle == status)
    {
        s_dmicBufferIndex  = s_dmicBufferIndex == 0U ? 1U : 0U;
        s_isDMICTriggerred = true;
    }
}
#endif

static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
    {
        s_RecordEmptyBlock++;
    }
}

#ifdef DMIC_FLASH_TARGET
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize,
                                   uint32_t startChannel)
{
    uint32_t i = 0, offset = 0U;

    for (i = 0; i < xferNum; i += 2)
    {
        DMA_SetupDescriptor(&(s_dmaDescriptorPingpong[i + startChannel * 2U]),
                            DMA_CHANNEL_XFER(true, false, false, true, 2U, kDMA_AddressInterleave0xWidth,
                                             interleaveSize, dataSizePerTransfer),
                            (uint32_t *)DMIC_FifoGetAddress(DMIC0, i / 2U + startChannel),
                            (uint8_t *)((uint32_t)dataAddr + offset),
                            &(s_dmaDescriptorPingpong[i + 1 + startChannel * 2U]));

        DMA_SetupDescriptor(&(s_dmaDescriptorPingpong[i + 1 + startChannel * 2U]),
                            DMA_CHANNEL_XFER(true, false, false, true, 2U, kDMA_AddressInterleave0xWidth,
                                             interleaveSize, dataSizePerTransfer),
                            (uint32_t *)DMIC_FifoGetAddress(DMIC0, i / 2U + startChannel),
                            (uint8_t *)((uint32_t)dataAddr + dataSizePerTransfer * interleaveSize + offset),
                            &(s_dmaDescriptorPingpong[i + startChannel * 2U]));

        offset += dataWidth;
        /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
        DMA_SubmitChannelDescriptor(&s_dmaHandle[i / 2U + startChannel],
                                    &(s_dmaDescriptorPingpong[i + startChannel * 2U]));
        DMA_StartTransfer(&s_dmaHandle[i / 2U + startChannel]);
    }
}
#else
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize)
{
    uint32_t i = 0, offset = 0U;

    for (i = 0; i < xferNum; i += 2)
    {
        startAddress[i].data                   = (uint8_t *)((uint32_t)dataAddr + offset);
        startAddress[i].dataWidth              = dataWidth;
        startAddress[i].dataSize               = dataSizePerTransfer;
        startAddress[i].dataAddrInterleaveSize = interleaveSize;
        startAddress[i].linkTransfer           = &startAddress[i + 1];

        startAddress[i + 1].data      = (uint8_t *)((uint32_t)dataAddr + dataSizePerTransfer * interleaveSize + offset);
        startAddress[i + 1].dataWidth = dataWidth;
        startAddress[i + 1].dataSize  = dataSizePerTransfer;
        startAddress[i + 1].dataAddrInterleaveSize = interleaveSize;
        startAddress[i + 1].linkTransfer           = &startAddress[i];

        offset += dataWidth;
    }
}
#endif

static void DEMO_InitCodec(void)
{
    PRINTF("\r\nInit codec\r\n");

    if (CODEC_Init(&s_codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }
}

#ifdef DMIC_FLASH_TARGET
static void DEMO_RecordDMIC(void)
{
    ReceiverTransferConfig(s_receiveXfer0, s_recordBuffer0, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth, 0U);
    ReceiverTransferConfig(s_receiveXfer1, s_recordBuffer1, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth, 4U);

    DMIC_EnableChannnel(DMIC0,
#if DEMO_ENABLE_DMIC_0
                        DMIC_CHANEN_EN_CH0(1) |
#endif
#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH1(1) |
#endif
#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH2(1) |
#endif
#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH3(1) |
#endif

#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH4(1) |
#endif

#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH5(1) |
#endif

#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH6(1) |
#endif

#if DEMO_ENABLE_DMIC_0
                            DMIC_CHANEN_EN_CH7(1)
#endif
    );
}
#else
static void DEMO_RecordDMIC(void)
{
    ReceiverTransferConfig(s_receiveXfer0, s_recordBuffer0, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth);
    ReceiverTransferConfig(s_receiveXfer1, s_recordBuffer1, RECORD_BUFFER_NUM / 2, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth);

#if DEMO_ENABLE_DMIC_0
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[0], &s_receiveXfer0[0], DEMO_DMIC_CHANNEL_0);
#endif

#if DEMO_ENABLE_DMIC_1
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[1], &s_receiveXfer0[2], DEMO_DMIC_CHANNEL_1);
#endif

#if DEMO_ENABLE_DMIC_2
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[2], &s_receiveXfer0[4], DEMO_DMIC_CHANNEL_2);
#endif

#if DEMO_ENABLE_DMIC_3
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[3], &s_receiveXfer0[6], DEMO_DMIC_CHANNEL_3);
#endif

#if DEMO_ENABLE_DMIC_4
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[4], &s_receiveXfer1[0], DEMO_DMIC_CHANNEL_4);
#endif

#if DEMO_ENABLE_DMIC_5
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[5], &s_receiveXfer1[2], DEMO_DMIC_CHANNEL_5);
#endif

#if DEMO_ENABLE_DMIC_6
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[6], &s_receiveXfer1[4], DEMO_DMIC_CHANNEL_6);
#endif

#if DEMO_ENABLE_DMIC_7
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[7], &s_receiveXfer1[6], DEMO_DMIC_CHANNEL_7);
#endif
}
#endif
