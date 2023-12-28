/*
 * Copyright 2016, NXP
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
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_LED_GPIO_PORT BOARD_LED_GREEN_GPIO_PORT
#define BOARD_LED_GPIO_PIN  BOARD_LED_GREEN_GPIO_PIN
#define FIFO_DEPTH 15U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

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
 * @brief Call back for DMIC0 HWVAD Interrupt
 */
void DMIC0_HWVAD_Callback(void)
{
    volatile int i;

    GPIO_PortToggle(GPIO, BOARD_LED_GPIO_PORT, 1U << BOARD_LED_GPIO_PIN);
    /* reset hwvad internal interrupt */
    DMIC_CtrlClrIntrHwvad(DMIC0, true);
    /* wait for HWVAD to settle */
    PRINTF("Just woke up\r\n");
    for (i = 0; i <= 500U; i++)
    {
    }
    /*HWVAD Normal operation */
    DMIC_CtrlClrIntrHwvad(DMIC0, false);

    GPIO_PortToggle(GPIO, BOARD_LED_GPIO_PORT, 1U << BOARD_LED_GPIO_PIN);
}

/*!
 * @brief Main function
 */

int main(void)
{
    dmic_channel_config_t dmic_channel_cfg;
    /* Define the init structure for the output LED pin*/
    gpio_pin_config_t led_config = {
        kGPIO_DigitalOutput,
        0,
    };
    /* Board pin, clock, debug console init */
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivDmicClk, 8U, false);

    CLOCK_AttachClk(kEXT_CLK_to_DMIC);
    GPIO_PinInit(GPIO, BOARD_LED_GPIO_PORT, BOARD_LED_GPIO_PIN, &led_config);
    GPIO_PinWrite(GPIO, BOARD_LED_GPIO_PORT, BOARD_LED_GPIO_PIN, LOGIC_LED_OFF);

    PRINTF("Configure DMIC\r\n");

    dmic_channel_cfg.divhfclk            = kDMIC_PdmDiv1;
    dmic_channel_cfg.osr                 = 25U;
    dmic_channel_cfg.gainshft            = 3U;
    dmic_channel_cfg.preac2coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.preac4coef          = kDMIC_CompValueZero;
    dmic_channel_cfg.dc_cut_level        = kDMIC_DcCut155;
    dmic_channel_cfg.post_dc_gain_reduce = 0U;
    dmic_channel_cfg.saturate16bit       = 1U;
    dmic_channel_cfg.sample_rate         = kDMIC_PhyFullSpeed;
#if defined(FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND) && (FSL_FEATURE_DMIC_CHANNEL_HAS_SIGNEXTEND)
    dmic_channel_cfg.enableSignExtend = false;
#endif
    DMIC_Init(DMIC0);
#if !(defined(FSL_FEATURE_DMIC_HAS_NO_IOCFG) && FSL_FEATURE_DMIC_HAS_NO_IOCFG)
    DMIC_SetIOCFG(DMIC0, kDMIC_PdmStereo);
#endif
    DMIC_Use2fs(DMIC0, true);
    DMIC_EnableChannelInterrupt(DMIC0, kDMIC_Channel0, true);
    DMIC_EnableChannelInterrupt(DMIC0, kDMIC_Channel1, true);
#if defined(BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP) && (BOARD_DMIC_CHANNEL_STEREO_SIDE_SWAP)
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel0, kDMIC_Right, &dmic_channel_cfg);
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel1, kDMIC_Left, &dmic_channel_cfg);
#else
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel0, kDMIC_Left, &dmic_channel_cfg);
    DMIC_ConfigChannel(DMIC0, kDMIC_Channel1, kDMIC_Right, &dmic_channel_cfg);
#endif
    DMIC_FifoChannel(DMIC0, kDMIC_Channel0, FIFO_DEPTH, true, true);
    DMIC_FifoChannel(DMIC0, kDMIC_Channel1, FIFO_DEPTH, true, true);

    /*Gain of the noise estimator */
    DMIC_SetGainNoiseEstHwvad(DMIC0, 0x02U);

    /*Gain of the signal estimator */
    DMIC_SetGainSignalEstHwvad(DMIC0, 0x01U);

    /* 00 = first filter by-pass, 01 = hpf_shifter=1, 10 = hpf_shifter=4 */
    DMIC_SetFilterCtrlHwvad(DMIC0, 0x01U);

    /*input right-shift of (GAIN x 2 -10) bits (from -10bits (0000) to +14bits (1100)) */
    DMIC_SetInputGainHwvad(DMIC0, 0x04U);

    DisableDeepSleepIRQ(HWVAD0_IRQn);
    DisableIRQ(HWVAD0_IRQn);
    DMIC_EnableChannnel(DMIC0, (DMIC_CHANEN_EN_CH0(1) | DMIC_CHANEN_EN_CH1(1)));

    DMIC_FilterResetHwvad(DMIC0, true);
    DMIC_FilterResetHwvad(DMIC0, false);
    /* reset hwvad internal interrupt */
    DMIC_CtrlClrIntrHwvad(DMIC0, true);
    /* Delay to clear first spurious interrupt and let the filter converge */
    SDK_DelayAtLeastUs(20000, SystemCoreClock);
    /*HWVAD Normal operation */
    DMIC_CtrlClrIntrHwvad(DMIC0, false);
    DMIC_HwvadEnableIntCallback(DMIC0, DMIC0_HWVAD_Callback);
    EnableDeepSleepIRQ(HWVAD0_IRQn);

    while (1)
    {
        DisableIRQ(HWVAD0_IRQn);
        PRINTF("Going into sleep\r\n");
        EnableIRQ(HWVAD0_IRQn);
        __WFI();
    }
}
