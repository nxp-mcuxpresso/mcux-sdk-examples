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
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_dmic.h"
#include "fsl_dmic_dma.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_i2s_dma.h"
#include "fsl_codec_common.h"
#include <stdbool.h>
#include "fsl_wm8904.h"
#include "fsl_codec_adapter.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DMAREQ_DMIC0                    23U
#define DMAREQ_DMIC1                    24U
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY (24576000)
#define DEMO_I2S_TX                     (I2S0)
#define DEMO_I2S_CLOCK_DIVIDER                                                                                 \
    (24576000U / 48000U / 16U / 2) /* I2S source clock 24.576MHZ, sample rate 48KHZ, bits width 16, 2 channel, \
                                  so bitclock should be 48KHZ * 16 = 768KHZ, divider should be 24.576MHZ / 768KHZ */

#define DEMO_DMA             (DMA0)
#define DEMO_DMIC_RX_CHANNEL DMAREQ_DMIC0
#define DEMO_I2S_TX_CHANNEL  (5)
#define DEMO_I2S_TX_MODE     kI2S_MasterSlaveNormalMaster

#define DEMO_DMIC_CHANNEL        kDMIC_Channel0
#define DEMO_DMIC_CHANNEL_ENABLE DMIC_CHANEN_EN_CH0(1) | DMIC_CHANEN_EN_CH1(1)
#define DEMO_AUDIO_BIT_WIDTH     (16)
#define DEMO_AUDIO_SAMPLE_RATE   (48000)
#define DEMO_AUDIO_PROTOCOL      kCODEC_BusI2S

#ifndef DEMO_DMIC_NUMS
#define DEMO_DMIC_NUMS 1U
#endif

#define FIFO_DEPTH           (15U)
#define RECORD_BUFFER_SIZE   (128)
#define PLAYBACK_BUFFER_SIZE (128 * 2U)
#define BUFFER_NUM           (2U)
#ifndef DEMO_CODEC_VOLUME
#define DEMO_CODEC_VOLUME 30U
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
wm8904_config_t wm8904Config = {
    .i2cConfig    = {.codecI2CInstance = BOARD_CODEC_I2C_INSTANCE, .codecI2CSourceClock = BOARD_CODEC_I2C_CLOCK_FREQ},
    .recordSource = kWM8904_RecordSourceLineInput,
    .recordChannelLeft  = kWM8904_RecordChannelLeft2,
    .recordChannelRight = kWM8904_RecordChannelRight2,
    .playSource         = kWM8904_PlaySourceDAC,
    .slaveAddress       = WM8904_I2C_ADDRESS,
    .protocol           = kWM8904_ProtocolI2S,
    .format             = {.sampleRate = kWM8904_SampleRate48kHz, .bitWidth = kWM8904_BitWidth16},
    .mclk_HZ            = DEMO_I2S_MASTER_CLOCK_FREQUENCY,
    .master             = false,
};
codec_config_t boardCodecConfig = {.codecDevType = kCODEC_WM8904, .codecDevConfig = &wm8904Config};
static i2s_config_t tx_config;
extern codec_config_t boardCodecConfig;
static uint8_t s_buffer[PLAYBACK_BUFFER_SIZE * BUFFER_NUM];
static uint32_t volatile s_writeIndex = 0U;
static uint32_t volatile s_emptyBlock = BUFFER_NUM;

static dmic_dma_handle_t s_leftDmicDmaHandle;
static dma_handle_t s_leftDmicRxDmaHandle;

#if DEMO_DMIC_NUMS == 2U
static dmic_dma_handle_t s_rightDmicDmaHandle;
static dma_handle_t s_rightDmicRxDmaHandle;
#endif

static dma_handle_t s_i2sTxDmaHandle;
static i2s_dma_handle_t s_i2sTxHandle;

SDK_ALIGN(dma_descriptor_t s_leftDmaDescriptorPingpong[2], 16);

static dmic_transfer_t s_leftReceiveXfer[2U] = {
    /* transfer configurations for channel0 */
    {
        .data                   = s_buffer,
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = RECORD_BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave2xWidth,
        .linkTransfer           = &s_leftReceiveXfer[1],
    },

    {
        .data                   = &s_buffer[PLAYBACK_BUFFER_SIZE],
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = RECORD_BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave2xWidth,
        .linkTransfer           = &s_leftReceiveXfer[0],
    },
};

#if DEMO_DMIC_NUMS == 2U
SDK_ALIGN(dma_descriptor_t s_rightDmaDescriptorPingpong[2], 16);

static dmic_transfer_t s_rightReceiveXfer[2U] = {
    /* transfer configurations for channel0 */
    {
        .data                   = &s_buffer[2],
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = RECORD_BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave2xWidth,
        .linkTransfer           = &s_rightReceiveXfer[1],
    },

    {
        .data                   = &s_buffer[PLAYBACK_BUFFER_SIZE + 2U],
        .dataWidth              = sizeof(uint16_t),
        .dataSize               = RECORD_BUFFER_SIZE,
        .dataAddrInterleaveSize = kDMA_AddressInterleave2xWidth,
        .linkTransfer           = &s_rightReceiveXfer[0],
    },
};
#endif

codec_handle_t codecHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_InitDebugConsole(void)
{
    /* attach 12 MHz clock to FLEXCOMM6 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);

    RESET_ClearPeripheralReset(kFC6_RST_SHIFT_RSTn);

    uint32_t uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;

    DbgConsole_Init(6U, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}



void dmic_Callback(DMIC_Type *base, dmic_dma_handle_t *handle, status_t status, void *userData)
{
    if (s_emptyBlock)
    {
        s_emptyBlock--;
    }
}

void i2s_Callback(I2S_Type *base, i2s_dma_handle_t *handle, status_t completionStatus, void *userData)
{
    if (s_emptyBlock < BUFFER_NUM)
    {
        s_emptyBlock++;
    }
}

/*!
 * @brief Main function
 */

int main(void)
{
    dmic_channel_config_t dmic_channel_cfg;
    i2s_transfer_t i2sTxTransfer;

    /* Board pin, clock, debug console init */
    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* USART clock */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom6Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM6);

    /* I2C clock */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);

    POWER_DisablePD(kPDRUNCFG_PD_XTALHF);                   /* Ensure XTAL32M is powered */
    POWER_DisablePD(kPDRUNCFG_PD_LDOXTALHF);                /* Ensure XTAL32M is powered */
    CLOCK_SetupExtClocking(16000000U);                      /* Enable clk_in clock */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK; /*!< Ensure CLK_IN is on  */
    ANACTRL->XO32M_CTRL |= ANACTRL_XO32M_CTRL_ENABLE_SYSTEM_CLK_OUT_MASK;

    /*!< Set up PLL */
    CLOCK_AttachClk(kEXT_CLK_to_PLL0);  /*!< Switch PLL0CLKSEL to EXT_CLK */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0); /* Ensure PLL is on  */
    POWER_DisablePD(kPDRUNCFG_PD_PLL0_SSCG);
    const pll_setup_t pll0Setup = {
        .pllctrl = SYSCON_PLL0CTRL_CLKEN_MASK | SYSCON_PLL0CTRL_SELI(2U) | SYSCON_PLL0CTRL_SELP(31U),
        .pllndec = SYSCON_PLL0NDEC_NDIV(125U),
        .pllpdec = SYSCON_PLL0PDEC_PDIV(8U),
        .pllsscg = {0x0U, (SYSCON_PLL0SSCG1_MDIV_EXT(3072U) | SYSCON_PLL0SSCG1_SEL_EXT_MASK)},
        .pllRate = 24576000U,
        .flags   = PLL_SETUPFLAG_WAITLOCK};
    CLOCK_SetPLL0Freq(&pll0Setup); /*!< Configure PLL0 to the desired values */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivPllClk, 1U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexFrg0, 0U, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1U, false);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8U, false);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_SetClkDiv(kCLOCK_DivMclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivMclk, 1U, false);
    CLOCK_AttachClk(kPLL0_to_MCLK);
    SYSCON->MCLKIO = 1U;

    /*!< Switch PLL0 clock source selector to XTAL16M */
    /* I2S clocks */
    CLOCK_AttachClk(kPLL0_to_PLLCLKDIV);
    CLOCK_AttachClk(kPLL_CLK_DIV_FRG0_to_FLEXCOMM0);

    /* DMIC clocks */
    CLOCK_AttachClk(kPLL0_to_DMIC);

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC0_RST_SHIFT_RSTn);
    NVIC_ClearPendingIRQ(FLEXCOMM0_IRQn);

    /* Enable interrupts for I2S */
    EnableIRQ(FLEXCOMM0_IRQn);

    /* Initialize the rest */
    BOARD_InitPins();
    BOARD_BootClockFROHF96M();
    APP_InitDebugConsole();

    PRINTF("Configure codec\r\n");

    /* protocol: i2s
     * sampleRate: 48K
     * bitwidth:16
     */
    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("codec_Init failed!\r\n");
        assert(false);
    }

    /* Initial volume kept low for hearing safety.
     * Adjust it to your needs, 0-100, 0 for mute, 100 for maximum volume.
     */
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight,
                        DEMO_CODEC_VOLUME) != kStatus_Success)
    {
        assert(false);
    }

    DMA_Init(DEMO_DMA);

    DMA_EnableChannel(DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_EnableChannel(DEMO_DMA, DEMO_DMIC_RX_CHANNEL);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_I2S_TX_CHANNEL, kDMA_ChannelPriority3);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_RX_CHANNEL, kDMA_ChannelPriority2);
    DMA_CreateHandle(&s_i2sTxDmaHandle, DEMO_DMA, DEMO_I2S_TX_CHANNEL);
    DMA_CreateHandle(&s_leftDmicRxDmaHandle, DEMO_DMA, DEMO_DMIC_RX_CHANNEL);

#if DEMO_DMIC_NUMS == 2U
    DMA_EnableChannel(DEMO_DMA, DEMO_DMIC_RX_CHANNEL_1);
    DMA_SetChannelPriority(DEMO_DMA, DEMO_DMIC_RX_CHANNEL_1, kDMA_ChannelPriority2);
    DMA_CreateHandle(&s_rightDmicRxDmaHandle, DEMO_DMA, DEMO_DMIC_RX_CHANNEL_1);
#endif

    memset(&dmic_channel_cfg, 0U, sizeof(dmic_channel_config_t));

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = 32U;
    dmic_channel_cfg.gainshft            = 3U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1U;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmDual);
#endif
    DMIC_Use2fs(DMIC0, true);
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL, true);
#if defined(BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP) && (BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP)
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL, kDMIC_Right, &dmic_channel_cfg);
#else
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL, kDMIC_Left, &dmic_channel_cfg);
#endif
    DMIC_FifoChannel(DMIC0, DEMO_DMIC_CHANNEL, FIFO_DEPTH, true, true);

#if DEMO_DMIC_NUMS == 2U
    DMIC_EnableChannelDma(DMIC0, DEMO_DMIC_CHANNEL_1, true);
#if defined(BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP) && (BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP)
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL_1, kDMIC_Left, &dmic_channel_cfg);
#else
    DMIC_ConfigChannel(DMIC0, DEMO_DMIC_CHANNEL_1, kDMIC_Right, &dmic_channel_cfg);
#endif
    DMIC_FifoChannel(DMIC0, DEMO_DMIC_CHANNEL_1, FIFO_DEPTH, true, true);
#endif

    DMIC_EnableChannnel(DMIC0, DEMO_DMIC_CHANNEL_ENABLE);

#if DEMO_DMIC_NUMS == 2U
    DMIC_EnableChannnel(DMIC0, DEMO_DMIC_CHANNEL_1_ENABLE);
#endif

    PRINTF("Configure I2S\r\n");

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
    tx_config.divider     = DEMO_I2S_CLOCK_DIVIDER;
    tx_config.masterSlave = DEMO_I2S_TX_MODE;
    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_TxTransferCreateHandleDMA(DEMO_I2S_TX, &s_i2sTxHandle, &s_i2sTxDmaHandle, i2s_Callback, NULL);

#if DEMO_DMIC_NUMS == 2U
    DMIC_TransferCreateHandleDMA(DMIC0, &s_leftDmicDmaHandle, NULL, NULL, &s_leftDmicRxDmaHandle);
    DMIC_InstallDMADescriptorMemory(&s_leftDmicDmaHandle, s_leftDmaDescriptorPingpong, 2U);

    DMIC_TransferCreateHandleDMA(DMIC0, &s_rightDmicDmaHandle, dmic_Callback, NULL, &s_rightDmicRxDmaHandle);
    DMIC_InstallDMADescriptorMemory(&s_rightDmicDmaHandle, s_rightDmaDescriptorPingpong, 2U);

    DMIC_TransferReceiveDMA(DMIC0, &s_leftDmicDmaHandle, s_leftReceiveXfer, DEMO_DMIC_CHANNEL);
    DMIC_TransferReceiveDMA(DMIC0, &s_rightDmicDmaHandle, s_rightReceiveXfer, DEMO_DMIC_CHANNEL_1);
#else
    DMIC_TransferCreateHandleDMA(DMIC0, &s_leftDmicDmaHandle, dmic_Callback, NULL, &s_leftDmicRxDmaHandle);
    DMIC_InstallDMADescriptorMemory(&s_leftDmicDmaHandle, s_leftDmaDescriptorPingpong, 2U);
    DMIC_TransferReceiveDMA(DMIC0, &s_leftDmicDmaHandle, s_leftReceiveXfer, DEMO_DMIC_CHANNEL);
#endif
    while (1)
    {
        if (s_emptyBlock < BUFFER_NUM)
        {
            i2sTxTransfer.data     = s_buffer + s_writeIndex * PLAYBACK_BUFFER_SIZE;
            i2sTxTransfer.dataSize = PLAYBACK_BUFFER_SIZE;
            if (I2S_TxTransferSendDMA(DEMO_I2S_TX, &s_i2sTxHandle, i2sTxTransfer) == kStatus_Success)
            {
                if (++s_writeIndex >= BUFFER_NUM)
                {
                    s_writeIndex = 0U;
                }
            }
        }
    }
}
