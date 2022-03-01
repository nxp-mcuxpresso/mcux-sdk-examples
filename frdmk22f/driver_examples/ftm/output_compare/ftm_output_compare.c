/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ftm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer base address/channel used for board */
#define BOARD_FTM_BASEADDR    FTM0
#define BOARD_FTM_OUT_CHANNEL kFTM_Chnl_6

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#ifndef DEMO_TIMER_PERIOD_MS
/* To make led toggling visible, set counter period to 100ms */
#define DEMO_TIMER_PERIOD_MS (100U)
#endif
#ifndef FTM_PRESCALER_VALUE
/* Calculate the clock division based on the PWM frequency to be obtained */
#define FTM_PRESCALER_VALUE \
    FTM_CalculateCounterClkDiv(BOARD_FTM_BASEADDR, 1000U / DEMO_TIMER_PERIOD_MS, FTM_SOURCE_CLOCK);
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
 * @brief Main function
 */
int main(void)
{
    ftm_config_t ftmInfo;

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nFTM example for output compare\r\n");
    PRINTF("\r\nYou will see the output signal toggle");
    PRINTF("\r\nProbe the signal using an oscilloscope");

    /* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo);
    /* Calculate the clock division based on the timer period frequency to be obtained */
    ftmInfo.prescale = FTM_PRESCALER_VALUE;

    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Setup the output compare mode to toggle output on a match, note the compare value must less than counter mod
     * value */
    FTM_SetupOutputCompare(BOARD_FTM_BASEADDR, BOARD_FTM_OUT_CHANNEL, kFTM_ToggleOnMatch,
                           MSEC_TO_COUNT(DEMO_TIMER_PERIOD_MS, FTM_SOURCE_CLOCK / (1U << ftmInfo.prescale)) / 2U);

    /* Set the timer period */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR,
                       MSEC_TO_COUNT(DEMO_TIMER_PERIOD_MS, FTM_SOURCE_CLOCK / (1U << ftmInfo.prescale)));

    /* Update the buffered registers */
    FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
    while (1)
    {
    };
}
