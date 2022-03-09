/*
 * Copyright 2018-2021 NXP
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
#if defined(OSTIMER0)
#define EXAMPLE_OSTIMER OSTIMER0
#else
#define EXAMPLE_OSTIMER OSTIMER
#endif
#define EXAMPLE_OSTIMER_FREQ CLOCK_GetLpOscFreq()
#define EXAMPLE_DEEPSLEEP_RUNCFG0 \
    (SYSCTL0_PDRUNCFG0_LPOSC_PD_MASK)         /*!< Power down all unnecessary blocks during deep sleep*/
#define EXAMPLE_DEEPSLEEP_RAM_APD 0xFFC00000U /* 0x280000 - 0x4FFFFF keep powered */
#define EXAMPLE_DEEPSLEEP_RAM_PPD 0x0U
#define EXAMPLE_EXCLUDE_FROM_DEEPSLEEP                                                                              \
    (((const uint32_t[]){EXAMPLE_DEEPSLEEP_RUNCFG0,                                                                 \
                         (SYSCTL0_PDSLEEPCFG1_FLEXSPI0_SRAM_APD_MASK | SYSCTL0_PDSLEEPCFG1_FLEXSPI1_SRAM_APD_MASK | \
                          SYSCTL0_PDSLEEPCFG1_FLEXSPI0_SRAM_PPD_MASK | SYSCTL0_PDSLEEPCFG1_FLEXSPI1_SRAM_PPD_MASK), \
                         EXAMPLE_DEEPSLEEP_RAM_APD, EXAMPLE_DEEPSLEEP_RAM_PPD}))
#define EXAMPLE_EnableDeepSleepIRQ() EnableDeepSleepIRQ(OS_EVENT_IRQn)

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
    POWER_EnterDeepSleep(EXAMPLE_EXCLUDE_FROM_DEEPSLEEP);
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
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kLPOSC_to_OSTIMER_CLK);

    PRINTF("Press any key to start example.\r\n\r\n");
    GETCHAR();
    PRINTF("Board will enter power deep sleep mode, and then wakeup by OS timer after about 5 seconds.\r\n");
    PRINTF("After Board wakeup, the OS timer will trigger the match interrupt about every 2 seconds.\r\n");

    /* Intialize the OS timer, setting clock configuration. */
    OSTIMER_Init(EXAMPLE_OSTIMER);

    matchFlag = false;

    /* Set the OS timer match value. */
    if (kStatus_Success ==
        EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 5000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
    {
        /* Enable OSTIMER IRQ under deep sleep mode. */
        EXAMPLE_EnableDeepSleepIRQ();
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
