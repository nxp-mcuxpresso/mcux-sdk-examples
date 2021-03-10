/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_codec_common.h"
#include "fsl_cs42888.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_dmic.h"
#include "fsl_dmic_dma.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_inputmux.h"
#include <stdbool.h>
#include "fsl_gpio.h"
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_I2S_MASTER_CLOCK_FREQUENCY CLOCK_GetMclkClkFreq()
#define DEMO_I2S_TX                     (I2S4)
#define DEMO_I2S_SAMPLE_RATE            48000
#define I2S_CLOCK_DIVIDER               (24576000 / DEMO_I2S_SAMPLE_RATE / 32 / 8)
#define DEMO_SDCARD_SWITCH_VOLTAGE_FUNCTION_EXIST
#define DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
#define DEMO_DMA_MEMCPY_LEFT_CHANNEL  0U
#define DEMO_DMA_MEMCPY_RIGHT_CHANNEL 1U

#define DEMO_DMA_CHANNEL_TRIGGER_INPUT_A   kINPUTMUX_Dma0TrigOutAToDma0
#define DEMO_DMA_CHANNEL_TRIGGER_OUTPUT_A  kINPUTMUX_Dma0OtrigChannel16ToTriginChannels
#define DEMO_DMA_CHANNEL_OUT_TRIGGER_INDEX 0

#define DEMO_DMA            (DMA0)
#define DEMO_DMA1           (DMA1)
#define DEMO_I2S_TX_CHANNEL (9)

#define DEMO_DMIC_DMA_RX_CHANNEL_0 16U
#define DEMO_DMIC_DMA_RX_CHANNEL_1 17U
#define DEMO_DMIC_DMA_RX_CHANNEL_2 18U
#define DEMO_DMIC_DMA_RX_CHANNEL_3 19U
#define DEMO_DMIC_DMA_RX_CHANNEL_4 20U
#define DEMO_DMIC_DMA_RX_CHANNEL_5 21U
#define DEMO_DMIC_DMA_RX_CHANNEL_6 22U
#define DEMO_DMIC_DMA_RX_CHANNEL_7 23U
#define DEMO_DMIC_NUMS             (4U)

#define DEMO_DMA_MEMCPY_CHANNEL_0 0

#define DEMO_DMIC_CHANNEL_0       kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_1       kDMIC_Channel1
#define DEMO_DMIC_CHANNEL_2       kDMIC_Channel2
#define DEMO_DMIC_CHANNEL_3       kDMIC_Channel3
#define DEMO_DMIC_CHANNEL_4       kDMIC_Channel4
#define DEMO_DMIC_CHANNEL_5       kDMIC_Channel5
#define DEMO_DMIC_CHANNEL_6       kDMIC_Channel6
#define DEMO_DMIC_CHANNEL_7       kDMIC_Channel7
#define DEMO_CODEC_I2C_BASEADDR   I2C2
#define DEMO_CODEC_I2C_INSTANCE   2U
#define DEMO_CODEC_I2C_CLOCK_FREQ CLOCK_GetFlexCommClkFreq(2U)
#define FIFO_DEPTH           (15U)
#define PLAYBACK_BUFFER_SIZE (1024)
#define PLAYBACK_BUFFER_NUM  (2U)
#define RECORD_BUFFER_SIZE   (64)
#define RECORD_BUFFER_NUM    (DEMO_DMIC_NUMS * PLAYBACK_BUFFER_NUM)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BORAD_CodecReset(bool state);
void BOARD_PowerOnSDCARD(void);
void BOARD_PowerOffSDCARD(void);
static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData);
static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode);
static void DEMO_DMAChannelConfigurations(void);
static void DEMO_RecordDMIC(void);
static void DEMO_DMICChannelConfigurations(void);
static void ReceiverTransferConfig(dmic_transfer_t *startAddress,
                                   void *dataAddr,
                                   uint32_t xferNum,
                                   uint32_t dataWidth,
                                   uint32_t dataSizePerTransfer,
                                   uint32_t interleaveSize);
static void DEMO_InitCS42888(void);
extern void BORAD_CodecReset(bool state);
static void DEMO_DMAMemcpyChannelConfigurations(void);
static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData);
/*******************************************************************************
 * Variables
 ******************************************************************************/
cs42888_config_t cs42888Config = {
    .DACMode      = kCS42888_ModeSlave,
    .ADCMode      = kCS42888_ModeSlave,
    .reset        = BORAD_CodecReset,
    .master       = false,
    .i2cConfig    = {.codecI2CInstance = DEMO_CODEC_I2C_INSTANCE},
    .format       = {.sampleRate = 48000U, .bitWidth = 24U},
    .bus          = kCS42888_BusTDM,
    .slaveAddress = CS42888_I2C_ADDR,
};

codec_config_t boardCodecConfig = {.codecDevType = kCODEC_CS42888, .codecDevConfig = &cs42888Config};
static i2s_config_t tx_config;
static uint32_t volatile s_RecordEmptyBlock = PLAYBACK_BUFFER_NUM;
/* DMIC dma handle for 8 channel */
static dmic_dma_handle_t s_dmicDmaHandle[DEMO_DMIC_NUMS];
/* dma handle for 8 channel */
static dma_handle_t s_dmaHandle[DEMO_DMIC_NUMS];
/* i2s dma handle */
static dma_handle_t s_i2sTxDmaHandle;
static dma_handle_t s_memcpyDmaHandle;
static i2s_dma_handle_t s_i2sTxHandle;
/* ping pong descriptor */
SDK_ALIGN(dma_descriptor_t s_dmaDescriptorPingpong[RECORD_BUFFER_NUM], 16);
SDK_ALIGN(dma_descriptor_t s_memcpyDescriptor[PLAYBACK_BUFFER_NUM], 16);
/* dmic transfer configurations */
static dmic_transfer_t s_receiveXfer0[RECORD_BUFFER_NUM];
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
SDK_ALIGN(static uint8_t s_recordBuffer[RECORD_BUFFER_SIZE * RECORD_BUFFER_NUM], 4U);

static volatile bool s_m2mBufferReady    = false;
static volatile bool s_buffer1Ready      = false;
static volatile bool s_i2sTransferFinish = false;
static codec_handle_t s_codecHandle;
extern codec_config_t boardCodecConfig;
static volatile uint8_t s_dmicBufferIndex = 0U;
static volatile uint8_t s_m2mBufferIndex  = 0U;
static volatile bool s_isDMICTriggerred   = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

void BORAD_CodecReset(bool state)
{
    if (state)
    {
        GPIO_PortSet(GPIO, 1, 1 << 9);
    }
    else
    {
        GPIO_PortClear(GPIO, 1, 1 << 9);
    }
}

void DEMO_Codec_I2C_Init(void)
{
    i2c_master_config_t i2cConfig = {0};

    I2C_MasterGetDefaultConfig(&i2cConfig);
    i2cConfig.baudRate_Bps = 10000;
    I2C_MasterInit(DEMO_CODEC_I2C_BASEADDR, &i2cConfig, DEMO_CODEC_I2C_CLOCK_FREQ);
}

status_t DEMO_Codec_I2C_Send(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, const uint8_t *txBuff, uint8_t txBuffSize)
{
    return BOARD_I2C_Send(DEMO_CODEC_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, (uint8_t *)txBuff,
                          txBuffSize);
}

status_t DEMO_Codec_I2C_Receive(
    uint8_t deviceAddress, uint32_t subAddress, uint8_t subAddressSize, uint8_t *rxBuff, uint8_t rxBuffSize)
{
    return BOARD_I2C_Receive(DEMO_CODEC_I2C_BASEADDR, deviceAddress, subAddress, subAddressSize, rxBuff, rxBuffSize);
}

/*!
 * @brief Main function
 */

int main(void)
{
    i2s_transfer_t i2sTxTransfer;

    /* Board pin, clock, debug console init */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);

    /* attach AUDIO PLL clock to FLEXCOMM3 (I2S3) */
    CLOCK_AttachClk(kAUDIO_PLL_to_FLEXCOMM4);
    /* I2C */
    CLOCK_AttachClk(kFFRO_to_FLEXCOMM2);

    /* attach AUDIO PLL clock to MCLK */
    CLOCK_AttachClk(kAUDIO_PLL_to_MCLK_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivMclkClk, 2);
    SYSCTL1->MCLKPINDIR = SYSCTL1_MCLKPINDIR_MCLKPINDIR_MASK;

    CLOCK_AttachClk(kAUDIO_PLL_to_DMIC_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8);

    cs42888Config.i2cConfig.codecI2CSourceClock = CLOCK_GetFlexCommClkFreq(2);
    cs42888Config.format.mclk_HZ                = CLOCK_GetMclkClkFreq();

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

    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel1, false, 64);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel2, false, 128);
    I2S_EnableSecondaryChannel(DEMO_I2S_TX, kI2S_SecondaryChannel3, false, 192);

    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_Callback, NULL);

    DEMO_RecordDMIC();
    DEMO_DMAMemcpyChannelConfigurations();

    /* init CS42888 */
    DEMO_InitCS42888();

    PRINTF("\r\nStart play audio data\r\n");

    while (1)
    {
        if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
        {
            i2sTxTransfer.data     = s_processedAudio + (s_m2mBufferIndex == 0U ? 1U : 0U) * PLAYBACK_BUFFER_SIZE;
            i2sTxTransfer.dataSize = PLAYBACK_BUFFER_SIZE;
            I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, i2sTxTransfer);
        }

        if ((s_isDMICTriggerred) && (s_m2mBufferReady))
        {
            s_isDMICTriggerred = false;
            s_m2mBufferReady   = false;
            DMA_StartTransfer(&s_memcpyDmaHandle);
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
        DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i, kDMA_ChannelPriority1);
        DMA_CreateHandle(&s_dmaHandle[i], DEMO_DMA, DEMO_DMIC_DMA_RX_CHANNEL_0 + i);
    }

    /* configurations for I2S channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);

    /* configurations for memcpy channel channel */
    DMA_EnableChannel(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0, kDMA_ChannelPriority0);
    DMA_CreateHandle(&s_memcpyDmaHandle, DEMO_DMA, DEMO_DMA_MEMCPY_CHANNEL_0);
    DMA_SetCallback(&s_memcpyDmaHandle, memcpy_channel_callback, NULL);
}

static void DEMO_DMAMemcpyChannelConfigurations(void)
{
    DMA_SetupDescriptor(&(s_memcpyDescriptor[0]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        s_recordBuffer, &s_processedAudio[2], &(s_memcpyDescriptor[1]));

    DMA_SetupDescriptor(&(s_memcpyDescriptor[1]),
                        DMA_CHANNEL_XFER(true, true, false, true, 2U, kDMA_AddressInterleave1xWidth,
                                         kDMA_AddressInterleave4xWidth, RECORD_BUFFER_SIZE * sizeof(uint32_t)),
                        &s_recordBuffer[RECORD_BUFFER_SIZE * sizeof(uint32_t)],
                        &s_processedAudio[RECORD_BUFFER_SIZE * sizeof(uint32_t) * 4U + 2], &(s_memcpyDescriptor[0]));

    /* must make sure the memcpy buffer index is not confilct with the DMIC filling buffer index */
    if (s_dmicBufferIndex == 1U)
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle, &(s_memcpyDescriptor[0]));
    }
    else
    {
        DMA_SubmitChannelDescriptor(&s_memcpyDmaHandle, &(s_memcpyDescriptor[1]));
    }

    DMA_StartTransfer(&s_memcpyDmaHandle);
}

static void DEMO_DMICChannelConfigurations(void)
{
    uint32_t i = 0;

    /* dmic channel configurations */
    DMIC_Init(DMIC0);
    DMIC_Use2fs(DMIC0, true);
    bool rightChannel = false;

    for (i = 0U; i < DEMO_DMIC_NUMS - 1; i++)
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

        DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle[i], NULL, NULL, &s_dmaHandle[i]);
        DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle[i], &s_dmaDescriptorPingpong[i * PLAYBACK_BUFFER_NUM],
                                        PLAYBACK_BUFFER_NUM);
    }

    DMIC_ConfigChannel(DMIC0, (dmic_channel_t)i, (stereo_side_t)rightChannel, &s_dmicChannelConfig);
    DMIC_FifoChannel(DMIC0, i, FIFO_DEPTH, true, true);

    DMIC_TransferCreateHandleDMA(DMIC0, &s_dmicDmaHandle[i], dmic_Callback, NULL, &s_dmaHandle[i]);
    DMIC_InstallDMADescriptorMemory(&s_dmicDmaHandle[i], &s_dmaDescriptorPingpong[i * PLAYBACK_BUFFER_NUM],
                                    PLAYBACK_BUFFER_NUM);

    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH0(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH1(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH2(1));
    DMIC_EnableChannnel(DMIC0, DMIC_CHANEN_EN_CH3(1));
}

static void memcpy_channel_callback(struct _dma_handle *handle, void *userData, bool transferDone, uint32_t intmode)
{
    if (s_RecordEmptyBlock)
    {
        s_RecordEmptyBlock--;
    }
    s_m2mBufferIndex = s_m2mBufferIndex == 0U ? 1U : 0U;
    s_m2mBufferReady = true;
}

static void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_DMIC_Idle == status)
    {
        s_dmicBufferIndex  = s_dmicBufferIndex == 0U ? 1U : 0U;
        s_isDMICTriggerred = true;
    }
}

static void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    if (s_RecordEmptyBlock < PLAYBACK_BUFFER_NUM)
    {
        s_RecordEmptyBlock++;
    }
}

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

static void DEMO_InitCS42888(void)
{
    PRINTF("\r\nInit CS42888 codec\r\n");

    if (CODEC_Init(&s_codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("CODEC_Init failed!\r\n");
        assert(false);
    }
}

static void DEMO_RecordDMIC(void)
{
    ReceiverTransferConfig(s_receiveXfer0, s_recordBuffer, RECORD_BUFFER_NUM, sizeof(uint16_t), RECORD_BUFFER_SIZE,
                           kDMA_AddressInterleave4xWidth);

    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[0], &s_receiveXfer0[0], DEMO_DMIC_CHANNEL_0);
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[1], &s_receiveXfer0[2], DEMO_DMIC_CHANNEL_1);
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[2], &s_receiveXfer0[4], DEMO_DMIC_CHANNEL_2);
    DMIC_TransferReceiveDMA(DMIC0, &s_dmicDmaHandle[3], &s_receiveXfer0[6], DEMO_DMIC_CHANNEL_3);
}
