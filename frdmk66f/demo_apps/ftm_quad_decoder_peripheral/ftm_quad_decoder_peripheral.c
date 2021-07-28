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
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t encoder_count         = 0U;
volatile uint8_t dir_when_overflow      = 0U;
volatile uint8_t counter_overflow_flag  = 0U;
volatile uint8_t counter_overflow_count = 0U;
volatile uint32_t loop_counter          = 0U;
volatile bool encoder_direction         = false;
volatile bool gQdFreshReady             = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_PIT_0_IRQHANDLER(void)
{
    /* Clear pit interrupt flag */
    PIT_ClearStatusFlags(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0, kPIT_TimerFlag);

    loop_counter++;
    if (loop_counter > 1000U) /* 1s. */
    {
        loop_counter  = 0U;
        gQdFreshReady = true;
        /* Read counter value */
        encoder_count = FTM_GetQuadDecoderCounterValue(DEMO_FTM_PERIPHERAL);
        /* Clear counter */
        FTM_ClearQuadDecoderCounterValue(DEMO_FTM_PERIPHERAL);
        /* Read direction */
        if (FTM_GetQuadDecoderFlags(DEMO_FTM_PERIPHERAL) & kFTM_QuadDecoderCountingIncreaseFlag)
        {
            encoder_direction = true;
        }
        else
        {
            encoder_direction = false;
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitBootPeripherals();

    /* Print a note to terminal */
    PRINTF("\r\nFTM quad decoder peripheral example\r\n");

    PIT_StartTimer(DEMO_PIT_PERIPHERAL, kPIT_Chnl_0);

    while (1)
    {
        /* Wati for the counter value is ready. */
        while (!gQdFreshReady)
        {
        }
        gQdFreshReady = false;

        /* Print the value. */
        if (encoder_direction)
        {
            PRINTF("Encoder direction: +\r\n");
        }
        else
        {
            PRINTF("Encoder direction: -\r\n");
        }

        PRINTF("Get current counter: %d\r\n", encoder_count);
    }
}
