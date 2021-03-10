/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ostimer.h"
#include "fsl_power.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_OSTIMER_FREQ           32768
#define EXAMPLE_EXCLUDE_FROM_DEEPSLEEP (kPDRUNCFG_PD_XTAL32K)
#define EXAMPLE_WAKEUP_IRQ_SOURCE      WAKEUP_OS_EVENT_TIMER
#define EXAMPLE_OSTIMER                OSTIMER
#define EXAMPLE_OSTIMER_IRQn           OS_EVENT_IRQn

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Enter deep sleep mode. */
void EXAMPLE_EnterDeepSleep(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile bool matchFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void EXAMPLE_EnterDeepSleep(void)
{
    /* Enter deep sleep mode by using power API. */
    POWER_EnterDeepSleep(EXAMPLE_EXCLUDE_FROM_DEEPSLEEP, 0x0U, EXAMPLE_WAKEUP_IRQ_SOURCE, 0x0U);
}

/* User Callback. */
void EXAMPLE_OstimerCallback(void)
{
    matchFlag = true;
    /* User code. */
}

/* Set the match value with unit of millisecond. OStimer will trigger a match interrupt after the a certain time. */
static status_t EXAMPLE_SetMatchInterruptTime(OSTIMER_Type *base, uint32_t ms, uint32_t freq, ostimer_callback_t cb)
{
    uint64_t timerTicks = OSTIMER_GetCurrentTimerValue(base);

    /* Translate the millisecond to ostimer count value. */
    timerTicks += MSEC_TO_COUNT(ms, freq);

    /* Set the match value with unit of ticks. */
    return OSTIMER_SetMatchValue(base, timerTicks, cb);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Clock setting for OS timer. choose xtal32k as the source clock of OS timer. */
    POWER_EnablePD(kPDRUNCFG_PD_XTAL32K);
    PMC->RTCOSC32K |= PMC_RTCOSC32K_SEL_MASK;
    PMC->OSTIMERr &= ~PMC_OSTIMER_OSC32KPD_MASK;

    PRINTF("Board will enter power deep sleep mode, and then wakeup by OS timer after about 5 seconds.\r\n");
    PRINTF("After Board wakeup, the OS timer will trigger the match interrupt about every 2 seconds.\r\n");

    /* Intialize the OS timer, setting clock configuration. */
    OSTIMER_Init(EXAMPLE_OSTIMER);

    matchFlag = false;

    /* Set the OS timer match value. */
    if (kStatus_Success ==
        EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 5000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
    {
        /* Enter deep sleep mode. */
        EXAMPLE_EnterDeepSleep();

        /* Wait until OS timer interrupt occurrs. */
        while (!matchFlag)
        {
        }

        /* Wakeup from deep sleep mode. */
        PRINTF("Board wakeup from deep sleep mode.\r\n\r\n");
    }
    else
    {
        PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
    }

    while (1)
    {
        matchFlag = false;

        /* Set the match value to trigger the match interrupt. */
        if (kStatus_Success ==
            EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 2000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
        {
            /* Wait for the match value to be reached. */
            while (!matchFlag)
            {
            }

            PRINTF("OS timer match value reached\r\n");
        }
        else
        {
            PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
        }
    }
}
