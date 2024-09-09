/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pint.h"

#include "fsl_common.h"
#include "fsl_inputmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_PINT_BASE      PINT0
#define DEMO_PINT_BSLICE0_SRC  kPINT_PatternMatchInp0Src
#define DEMO_PINT_BSLICE1_SRC  kPINT_PatternMatchInp0Src
#define DEMO_PINT_BSLICE2_SRC  kPINT_PatternMatchInp1Src
#define DEMO_PINT_PIN_INT0_SRC kINPUTMUX_GpioPort0Pin9ToPintsel
#define DEMO_PINT_PIN_INT1_SRC kINPUTMUX_GpioPort1Pin3ToPintsel
#ifndef EXAMPLE_PINT_BASE
#define EXAMPLE_PINT_BASE PINT
#endif

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
 * @brief Call back for PINT Pin interrupt 0-7.
 */
void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    PRINTF("\f\r\nPINT Pin Interrupt %d event detected. PatternMatch status = %8b\r\n", pintr, pmatch_status);
}

/*!
 * @brief Main function
 */
int main(void)
{
    pint_pmatch_cfg_t pmcfg;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Connect trigger sources to PINT, PINT should be enabled before configuring INPUTMUX. */
    RESET_ClearPeripheralReset(kPINT_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Pint);
    INPUTMUX_Init(INPUTMUX0);
    INPUTMUX_AttachSignal(INPUTMUX0, kPINT_PinInt0, DEMO_PINT_PIN_INT0_SRC);
    INPUTMUX_AttachSignal(INPUTMUX0, kPINT_PinInt1, DEMO_PINT_PIN_INT1_SRC);
    INPUTMUX_Deinit(INPUTMUX0);

    /* Clear screen*/
    PRINTF("%c[2J", 27);
    /* Set cursor location at [0,0] */
    PRINTF("%c[0;0H", 27);
    PRINTF("\f\r\nPINT Pattern Match example\r\n");

    /* Initialize PINT */
    PINT_Init(EXAMPLE_PINT_BASE);

    /* configure kPINT_PatternMatchBSlice0 to show the single inputsrc */
    /* Setup Pattern Match Bit Slice 0 */
    pmcfg.bs_src    = DEMO_PINT_BSLICE0_SRC;
    pmcfg.bs_cfg    = kPINT_PatternMatchStickyFall;
    pmcfg.callback  = pint_intr_callback;
    pmcfg.end_point = true;
    PINT_PatternMatchConfig(EXAMPLE_PINT_BASE, kPINT_PatternMatchBSlice0, &pmcfg);

    /* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(EXAMPLE_PINT_BASE, kPINT_PinInt0);

    /* configure kPINT_PatternMatchBSlice1 and kPINT_PatternMatchBSlice2 to show the combined inputsrc */
#if (FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS > 1U)
    /* Setup Pattern Match Bit Slice 1 */
    pmcfg.bs_src    = DEMO_PINT_BSLICE1_SRC;
    pmcfg.bs_cfg    = kPINT_PatternMatchStickyRise;
    pmcfg.callback  = pint_intr_callback;
    pmcfg.end_point = false;
    PINT_PatternMatchConfig(EXAMPLE_PINT_BASE, kPINT_PatternMatchBSlice1, &pmcfg);
#endif

#if (FSL_FEATURE_PINT_NUMBER_OF_CONNECTED_OUTPUTS > 2U)
    /* Setup Pattern Match Bit Slice 2 for falling edge detection */
    pmcfg.bs_src    = DEMO_PINT_BSLICE2_SRC;
    pmcfg.bs_cfg    = kPINT_PatternMatchStickyRise;
    pmcfg.callback  = pint_intr_callback;
    pmcfg.end_point = true;
    PINT_PatternMatchConfig(EXAMPLE_PINT_BASE, kPINT_PatternMatchBSlice2, &pmcfg);

    /* Enable callbacks for PINT2 by Index */
    PINT_EnableCallbackByIndex(EXAMPLE_PINT_BASE, kPINT_PinInt2);
#endif

    /* Enable PatternMatch */
    PINT_PatternMatchEnable(EXAMPLE_PINT_BASE);

    PRINTF("\r\nPINT Pattern match events are configured\r\n");
    PRINTF("\r\nPress corresponding switches to generate events\r\n");
    while (1)
    {
        __WFI();
    }
}
