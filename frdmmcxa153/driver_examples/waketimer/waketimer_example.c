/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_waketimer.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_WAKETIMER     WAKETIMER0
#define EXAMPLE_WAKETIMER_IRQ WAKETIMER0_IRQn


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool timeoutFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* User Callback. */
void EXAMPLE_WaketimerCallback(void)
{
    timeoutFlag = true;
    /* User code. */
}

/*!
 * @brief Main function
 */
int main(void)
{
    waketimer_config_t config;
    /* Board pin, clock, debug console init */
    CLOCK_SetupFRO16KClocking(kCLKE_16K_SYSTEM | kCLKE_16K_COREMAIN);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("WAKETIMER will timeout every second\r\n");

    /* Enable OSCDivide and interrupt by default*/
    WAKETIMER_GetDefaultConfig(&config);
    /* Intialize the WAKETIMER. */
    WAKETIMER_Init(EXAMPLE_WAKETIMER, &config);
    /* Set user callback function */
    WAKETIMER_SetCallback(EXAMPLE_WAKETIMER, EXAMPLE_WaketimerCallback);
    /* Enable interrupt */
    WAKETIMER_EnableInterrupts(EXAMPLE_WAKETIMER, kWAKETIMER_WakeInterruptEnable);
    EnableIRQ(EXAMPLE_WAKETIMER_IRQ);

    timeoutFlag = false;
    WAKETIMER_StartTimer(EXAMPLE_WAKETIMER, 1000);

    while (1)
    {
        if (timeoutFlag)
        {
            PRINTF("Timeout\r\n");

            timeoutFlag = false;
            WAKETIMER_StartTimer(EXAMPLE_WAKETIMER, 1000);
        }
    }
}
