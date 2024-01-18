/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_wwdt.h"

#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_LED_INIT        LED_RED_INIT(LOGIC_LED_OFF);
#define APP_LED_ON          (LED_RED_ON());
#define APP_LED_TOGGLE      (LED_RED_TOGGLE());
#define APP_WDT_IRQn        WDT_BOD_IRQn
#define APP_WDT_IRQ_HANDLER WDT_BOD_IRQHandler

#define WDT_CLK_FREQ        CLOCK_GetWdtClkFreq()
#define LPC55S36_WORKAROUND (1)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_WDT_IRQ_HANDLER(void)
{
    uint32_t wdtStatus = WWDT_GetStatusFlags(WWDT);

    APP_LED_TOGGLE;

    /* The chip will reset before this happens */
    if (wdtStatus & kWWDT_TimeoutFlag)
    {
        WWDT_ClearStatusFlags(WWDT, kWWDT_TimeoutFlag);
    }

    /* Handle warning interrupt */
    if (wdtStatus & kWWDT_WarningFlag)
    {
        /* A watchdog feed didn't occur prior to warning timeout */
        WWDT_ClearStatusFlags(WWDT, kWWDT_WarningFlag);
        /* User code. User can do urgent case before timeout reset.
         * IE. user can backup the ram data or ram log to flash.
         * the period is set by config.warningValue, user need to
         * check the period between warning interrupt and timeout.
         */
    }

#if (defined(LPC55S36_WORKAROUND) && LPC55S36_WORKAROUND)
    /* Set PMC register value that could run with GDET enable */
    PMC->LDOPMU   = 0x0109CF18;
    PMC->DCDC0    = 0x010A767E;
    PMC->LDOCORE0 = 0x2801006B;
#endif
    SDK_ISR_EXIT_BARRIER;
}

void delayWwdtWindow(void)
{
    /* For the TV counter register value will decrease after feed watch dog,
     * we can use it to as delay. But in user scene, user need feed watch dog
     * in the time period after enter Window but before warning intterupt.
     */
    while (WWDT->TV > WWDT->WINDOW)
    {
        __NOP();
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    wwdt_config_t config;
    uint32_t wdtFreq;
    bool timeOutResetEnable;

    /* Init hardware*/
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Enable FRO 1M clock for WWDT module. */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_CLK_ENA_MASK;
    /* Set clock divider for WWDT clock source. */
    CLOCK_SetClkDiv(kCLOCK_DivWdtClk, 1U, true);

    /* Initialize the LED port. */
    LED_RED_INIT(LOGIC_LED_OFF);

#if !defined(FSL_FEATURE_WWDT_HAS_NO_PDCFG) || (!FSL_FEATURE_WWDT_HAS_NO_PDCFG)
    POWER_DisablePD(kPDRUNCFG_PD_WDT_OSC);
#endif

    /* Set Red LED to initially be high */
    APP_LED_INIT;

    /* Enable the WWDT time out to reset the CPU. */
    timeOutResetEnable = true;

    /* Check if reset is due to Watchdog */
#ifdef IS_WWDT_RESET
    if (IS_WWDT_RESET)
#else
    if (WWDT_GetStatusFlags(WWDT) & kWWDT_TimeoutFlag)
#endif
    {
        APP_LED_ON;
        PRINTF("Watchdog reset occurred\r\n");
        timeOutResetEnable = false;
        /* The timeout flag can only clear when and after wwdt intial. */
    }

    /* wdog refresh test in window mode/timeout reset */
    PRINTF("\r\n--- %s test start ---\r\n", (timeOutResetEnable) ? "Time out reset" : "Window mode refresh");

    /* The WDT divides the input frequency into it by 4 */
    wdtFreq = WDT_CLK_FREQ / 4;

    WWDT_GetDefaultConfig(&config);

    /*
     * Set watchdog feed time constant to approximately 4s
     * Set watchdog warning time to 512 ticks after feed time constant
     * Set watchdog window time to 1s
     */
    config.timeoutValue = wdtFreq * 4;
    config.warningValue = 512;
    config.windowValue  = wdtFreq * 1;
    /* Configure WWDT to reset on timeout */
    config.enableWatchdogReset = true;
    /* Setup watchdog clock frequency(Hz). */
    config.clockFreq_Hz = WDT_CLK_FREQ;
    WWDT_Init(WWDT, &config);

    NVIC_EnableIRQ(APP_WDT_IRQn);

    while (1)
    {
        if (timeOutResetEnable)
        {
            /* SDK_DelayAtLeastUs can be replaced by Detail User code*/
            SDK_DelayAtLeastUs(1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }
        else
        {
            /* The WINDOW register determines the highest TV value allowed when a watchdog feed is
             * performed. If a feed sequence occurs when TV is greater than the value in WINDOW, a
             * watchdog event will occur. User can set window same as timeout value if required. */
            delayWwdtWindow();
            WWDT_Refresh(WWDT);
            PRINTF(" WDOG has been refreshed!\r\n");
            /* SDK_DelayAtLeastUs can be replaced by Detail User code*/
            SDK_DelayAtLeastUs(1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }
    }
}
