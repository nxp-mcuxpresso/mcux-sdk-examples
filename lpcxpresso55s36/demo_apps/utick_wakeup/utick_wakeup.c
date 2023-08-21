/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"
#include "fsl_utick.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_UTICK_WAKEUP_FROM_SLEEP_MODE 1
#define APP_LED_INIT                     LED_RED_INIT(1);
#define APP_LED_TOGGLE                   (LED_RED_TOGGLE());
#define APP_INTERNAL_IRC                 BOARD_BootClockFRO12M

#define UTICK_TIME 3000000
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(void);

void BOARD_BootToIrc()
{
    APP_INTERNAL_IRC();
}

/*******************************************************************************
 * Code
 ******************************************************************************/
void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 100000; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();
#if !defined(DONOT_ENABLE_FLASH_PREFETCH)
    /* enable flash prefetch for better performance */
    SYSCON->FMCCR |= SYSCON_FMCCR_PREFEN_MASK;
#endif

    /* Enable FRO 1M clock for UTICK module.
     * Note: the FRO1MHZ_FREQM bit must be set.
     */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK | SYSCON_CLOCK_CTRL_FRO1MHZ_UTICK_ENA_MASK;
    /* Init output LED GPIO. */
    LED_RED_INIT(LOGIC_LED_OFF);

    /* Running core to internal clock 12 MHz*/
    BOARD_BootToIrc();

    /* Init output LED GPIO. */
    APP_LED_INIT;

    PRINTF("Utick wakeup demo start...\r\n");
    /* Attach Main Clock as CLKOUT */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);

    /* Set the clock dividor to divide by 2*/
    CLOCK_SetClkDiv(kCLOCK_DivClkOut, 2, false);

    /* Intiialize UTICK */
    UTICK_Init(UTICK0);

    /* Set the UTICK timer to wake up the device from reduced power mode */
    UTICK_SetTick(UTICK0, kUTICK_Repeat, UTICK_TIME, NULL);

#if defined(APP_UTICK_WAKEUP_FROM_SLEEP_MODE) && APP_UTICK_WAKEUP_FROM_SLEEP_MODE
    /* Enter sleep mode. */
    POWER_EnterSleep();
#else
/* Enter Deep Sleep mode */
#if (defined(FSL_FEATURE_POWERLIB_EXTEND) && FSL_FEATURE_POWERLIB_EXTEND)
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, WAKEUP_UTICK, 0x0);
#else
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
#endif
#endif

    /* Set the clock dividor to divide by 1*/
    CLOCK_SetClkDiv(kCLOCK_DivClkOut, 1, false);

#if defined(APP_UTICK_WAKEUP_FROM_SLEEP_MODE) && APP_UTICK_WAKEUP_FROM_SLEEP_MODE
    PRINTF("Wakeup from sleep mode...\r\n");
#else
    PRINTF("Wakeup from deep sleep mode...\r\n");
#endif

    while (1)
    {
        /* Toggle LED */
        APP_LED_TOGGLE;
        delay();
    }
}
