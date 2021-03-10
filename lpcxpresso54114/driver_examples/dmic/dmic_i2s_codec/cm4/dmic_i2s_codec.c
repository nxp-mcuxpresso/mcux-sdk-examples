/*
 * Copyright 2016-2020, NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_dmic.h"
#include "fsl_i2c.h"
#include "fsl_i2s.h"
#include "fsl_wm8904.h"
#include <stdlib.h>
#include <string.h>
#include "fsl_codec_common.h"
#include <stdbool.h>
#include "fsl_codec_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_I2C                        (I2C4)
#define DEMO_I2S_MASTER_CLOCK_FREQUENCY (24576000)
#define DEMO_I2S_TX                     (I2S1)
#define DEMO_I2S_RX                     (I2S0)
#define DEMO_I2S_CLOCK_DIVIDER          16
#define DEMO_I2S_TX_MODE                kI2S_MasterSlaveNormalMaster
#define FIFO_DEPTH 0U

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
codec_handle_t codecHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/


/*!
 * @brief Call back for DMIC0 Interrupt
 */
void DMIC0_Callback(void)
{
    /* In this example this interrupt is disabled */
}

/*!
 * @brief Main function
 */

int main(void)
{
    dmic_channel_config_t dmic_channel_cfg;

    /* Board pin, clock, debug console init */
    const pll_setup_t pllSetup = {
        .syspllctrl   = SYSCON_SYSPLLCTRL_BANDSEL_MASK | SYSCON_SYSPLLCTRL_SELP(0x1FU) | SYSCON_SYSPLLCTRL_SELI(0x8U),
        .syspllndec   = SYSCON_SYSPLLNDEC_NDEC(0x2DU),
        .syspllpdec   = SYSCON_SYSPLLPDEC_PDEC(0x42U),
        .syspllssctrl = {SYSCON_SYSPLLSSCTRL0_MDEC(0x34D3U) | SYSCON_SYSPLLSSCTRL0_SEL_EXT_MASK, 0x00000000U},
        .pllRate      = 24576000U, /* 16 bits * 2 channels * 48 kHz * 16 */
        .flags        = PLL_SETUPFLAG_WAITLOCK};

    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Iocon);
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* USART0 clock */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* I2C clock */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);

    /* Initialize PLL clock */
    CLOCK_AttachClk(kFRO12M_to_SYS_PLL);
    CLOCK_SetPLLFreq(&pllSetup);

    /* I2S clocks */
    CLOCK_AttachClk(kSYS_PLL_to_FLEXCOMM6);
    CLOCK_AttachClk(kSYS_PLL_to_FLEXCOMM7);

    /* DMIC uses system pll as clock source, divider 8, 24576000/8=3.072MHZ */
    CLOCK_AttachClk(kSYS_PLL_to_DMIC);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8, false);

    /* Attach PLL clock to MCLK for I2S, no divider */
    CLOCK_AttachClk(kSYS_PLL_to_MCLK);
    SYSCON->MCLKDIV = SYSCON_MCLKDIV_DIV(0U);
    SYSCON->MCLKIO  = 1U;

    /* reset FLEXCOMM for I2C */
    RESET_PeripheralReset(kFC4_RST_SHIFT_RSTn);

    /* reset FLEXCOMM for I2S */
    RESET_PeripheralReset(kFC6_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kFC7_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockFROHF48M();
    BOARD_InitDebugConsole();

    PRINTF("Configure WM8904 codec\r\n");

    if (CODEC_Init(&codecHandle, &boardCodecConfig) != kStatus_Success)
    {
        PRINTF("WM8904_Init failed!\r\n");
        assert(false);
    }

    /* Initial volume kept low for hearing safety.
     * Adjust it to your needs, 0-100, 0 for mute, 100 for maximum volume.
     */
    if (CODEC_SetVolume(&codecHandle, kCODEC_PlayChannelHeadphoneLeft | kCODEC_PlayChannelHeadphoneRight, 32U) !=
        kStatus_Success)
    {
        assert(false);
    }

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = 32U;
    dmic_channel_cfg.gainshft            = 3U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 1U;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
#if defined(FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND) && (FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND)
    dmic_channel_cfg.enableSignExtend = true;
#endif

    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmDual);
#endif
    DMIC_Use2fs(DMIC0, true);
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel0, kDMIC_Left, &dmic_channel_cfg);
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel1, kDMIC_Right, &dmic_channel_cfg);

    /* FIFO disabled */
    DMIC_FifoChannel(DMIC0, kDMIC_Channel0, FIFO_DEPTH, false, true);
    DMIC_FifoChannel(DMIC0, kDMIC_Channel1, FIFO_DEPTH, false, true);
    DMIC_EnableChannnel(DMIC0, (DMIC_CHANEN_EN_CH0(1) | DMIC_CHANEN_EN_CH1(1)));
    DMIC_DisableIntCallback(DMIC0, DMIC0_Callback);
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
    tx_config.divider     = DEMO_I2S_CLOCK_DIVIDER; /* divide by 16*/
    tx_config.pdmData     = true;
    tx_config.masterSlave = DEMO_I2S_TX_MODE;

    /*
     * masterSlave = kI2S_MasterSlaveNormalSlave;
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
    I2S_TxInit(DEMO_I2S_TX, &tx_config);
    I2S_Enable(DEMO_I2S_TX);

    while (1)
    {
    }
}
