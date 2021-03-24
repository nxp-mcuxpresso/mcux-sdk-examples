/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
#if defined(FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER) && FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER
#include "fsl_syscon.h"
#else
#include "fsl_inputmux.h"
#endif /* FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER */
#include "fsl_pint.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DEBUG_CONSOLE_SWO_PORT     (0U)
#define DEMO_DEBUG_CONSOLE_SWO_BAUDRATE (4000000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsoleSWO(uint32_t port, uint32_t baudrate);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_userPress = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
extern volatile bool g_userPress;

void pint_intr_callback(pint_pin_int_t pintr, uint32_t pmatch_status)
{
    g_userPress = true;
}

void BOARD_InitKey(void)
{
#if defined(FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER) && FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER
    /* Connect trigger sources to PINT */
    SYSCON_AttachSignal(SYSCON, kPINT_PinInt0, kINPUTMUX_GpioPort0Pin5ToPintsel);
#else
    /* Connect trigger sources to PINT */
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, kPINT_PinInt0, kINPUTMUX_GpioPort0Pin5ToPintsel);
    /* Turnoff clock to inputmux to save power. Clock is only needed to make changes */
    INPUTMUX_Deinit(INPUTMUX);
#endif /* FSL_FEATURE_SYSCON_HAS_PINT_SEL_REGISTER */

    /* Initialize PINT */
    PINT_Init(PINT);

    /* Setup Pin Interrupt 0 for rising edge */
    PINT_PinInterruptConfig(PINT, kPINT_PinInt0, kPINT_PinIntEnableRiseEdge, pint_intr_callback);

    /* Enable callbacks for PINT */
    PINT_EnableCallback(PINT);
}


void BOARD_InitDebugConsoleSWO(uint32_t port, uint32_t baudrate)
{
    uint32_t clkSrcFreq = BOARD_BOOTCLOCKFRO12M_CORE_CLOCK;

    DbgConsole_Init(port, baudrate, kSerialPort_Swo, clkSrcFreq);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    /*!< Switch TRACE to TRACE_DIV */
    CLOCK_AttachClk(kTRACE_DIV_to_TRACE);

    BOARD_InitPins();
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();
    BOARD_InitKey();
    BOARD_InitDebugConsoleSWO(DEMO_DEBUG_CONSOLE_SWO_PORT, DEMO_DEBUG_CONSOLE_SWO_BAUDRATE);

    while (1)
    {
        if (g_userPress)
        {
            PRINTF("SWO: hello_world\r\n");
            g_userPress = false;
        }
    }
}
