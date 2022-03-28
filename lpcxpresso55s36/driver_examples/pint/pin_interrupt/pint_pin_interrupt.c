/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2020 NXP
 * All rights reserved.
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
#define DEMO_PINT_PIN_INT0_SRC     kINPUTMUX_GpioPort0Pin17ToPintsel
#define DEMO_PINT_PIN_INT1_SRC     kINPUTMUX_GpioPort1Pin18ToPintsel
#define DEMO_SEC_PINT_PIN_INT0_SRC kINPUTMUX_GpioPort0Pin17ToPintSecsel
#define DEMO_PIN_NUM               2

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
    PRINTF("\f\r\nPINT Pin Interrupt %d event detected.", pintr);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, DEMO_PINT_PIN_INT0_SRC);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt1, DEMO_PINT_PIN_INT1_SRC);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, DEMO_SEC_PINT_PIN_INT0_SRC);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);

    /* Clear screen*/
    PRINTF("%c[2J", 27);
    /* Set cursor location at [0,0] */
    PRINTF("%c[0;0H", 27);
    PRINTF("\f\r\nPINT Pin interrupt example\r\n");

    /* Initialize PINT */
    PINT_Init(PINT);

    /* Setup Pin Interrupt 0 for rising edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, pint_intr_callback);
    /* Enable callbacks for PINT0 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt0);

#if (DEMO_PIN_NUM > 1U)
    /* Setup Pin Interrupt 1 for falling edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt1, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    /* Enable callbacks for PINT1 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt1);
#endif

#if (DEMO_PIN_NUM > 2U)
    /* Setup Pin Interrupt 2 for falling edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt2, kPINT_PinIntEnableFallEdge, pint_intr_callback);
    /* Enable callbacks for PINT2 by Index */
    PINT_EnableCallbackByIndex(PINT, kPINT_PinInt2);
#endif

    PRINTF("\r\nPINT Pin Interrupt events are configured\r\n");
    PRINTF("\r\nPress corresponding switches to generate events\r\n");
    while (1)
    {
        __WFI();
    }
}
