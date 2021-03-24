/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "peripherals.h"
#include "board.h"
#include "fsl_common.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                         \
    (SYSCON_PDRUNCFG_PDEN_SRAM1_MASK | SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAMX_MASK | \
     SYSCON_PDRUNCFG_PDEN_WDT_OSC_MASK)
#define APP_LED_INIT     (LED_GREEN_INIT(1));
#define APP_LED_TOGGLE   (LED_GREEN_TOGGLE());
#define APP_INTERNAL_IRC BOARD_BootClockFRO12M

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
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable clock for GPIO*/
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitPeripherals();

    /* Running core to internal clock 12 MHz*/
    BOARD_BootToIrc();

    /* Init output LED GPIO. */
    APP_LED_INIT;

    PRINTF("Utick wakeup demo start...\r\n");
    /* Attach Main Clock as CLKOUT */
    CLOCK_AttachClk(kMAIN_CLK_to_CLKOUT);

    /* Set the clock dividor to divide by 2*/
    CLOCK_SetClkDiv(kCLOCK_DivClkOut, 2, false);

#if defined(APP_UTICK_WAKEUP_FROM_SLEEP_MODE) && APP_UTICK_WAKEUP_FROM_SLEEP_MODE
    /* Enter sleep mode. */
    POWER_EnterSleep();
#else
/* Enter Deep Sleep mode */
#if (defined(FSL_FEATURE_POWERLIB_LPC55XX_EXTEND) && FSL_FEATURE_POWERLIB_LPC55XX_EXTEND)
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, WAKEUP_GPIO_INT0_0, 0x0);
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
