/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
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
#include "fsl_ctimer.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CTIMER          CTIMER1         /* Timer 1 */
#define CTIMER_MAT0_OUT kCTIMER_Match_2 /* Match output 2 */
#define CTIMER_EMT0_OUT (1u << kCTIMER_Match_2)
#define CTIMER_MAT1_OUT kCTIMER_Match_3 /* Match output 3 */
#define CTIMER_EMT1_OUT (1u << kCTIMER_Match_3)
#define CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(1U)


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void ctimer_match0_callback(uint32_t flags);
void ctimer_match1_callback(uint32_t flags);

/* Array of function pointers for callback for each channel */
ctimer_callback_t ctimer_callback_table[] = {
    NULL, NULL, ctimer_match0_callback, ctimer_match1_callback, NULL, NULL, NULL, NULL};

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Match Configuration for Channel 0 */
static ctimer_match_config_t matchConfig0;
/* Match Configuration for Channel 1 */
static ctimer_match_config_t matchConfig1;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * ctimer_match0_callback and ctimer_match1_callback are just nomial callback function.
 * They can be defined as MR1 and MR3 register match interrupt callback according to
 * CTIMER_MAT0_OUT and CTIMER_MAT1_OUT definition.
 * API CTIMER_GenericIRQHandler test every bit in IR register, each bit means MACTH0-3
 * interrupt and CAPTURE0-3 interrupt flag, then invoke corresponding callback.
 * When ctimer_match0_callback and ctimer_match1_callback are defined as MR1 and MR3
 * register match interrupt callback, they can be defined as follow:
 * 
 * ctimer_callback_t ctimer_callback_table[] = {
 *   NULL, ctimer_match0_callback, NULL, ctimer_match1_callback, NULL, NULL, NULL, NULL};
 * 
 * Each element in array relates to corresponding bit in IR register.
 */
void ctimer_match1_callback(uint32_t flags)
{
    static uint32_t count            = 0;
    static uint32_t matchUpdateCount = 8;

    if (++count > matchUpdateCount)
    {
        count = 0;
        matchConfig1.matchValue >>= 1;
        matchUpdateCount <<= 1;
        if (matchUpdateCount == (1 << 8))
        {
            matchUpdateCount        = 8;
            matchConfig1.matchValue = CTIMER_CLK_FREQ / 2;
        }
        CTIMER_SetupMatch(CTIMER, CTIMER_MAT1_OUT, &matchConfig1);
    }
#if defined(BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED)
    /* No timer match output pin connected to a LED
     * toggle LED manually according to match status
     */
    if (CTIMER_GetOutputMatchStatus(CTIMER, CTIMER_EMT1_OUT))
    {
        LED_RED2_ON();
    }
    else
    {
        LED_RED2_OFF();
    }
#endif
}

void ctimer_match0_callback(uint32_t flags)
{
    static uint32_t count            = 0;
    static uint32_t matchUpdateCount = 8;

    if (++count > matchUpdateCount)
    {
        count = 0;
        matchConfig0.matchValue >>= 1;
        matchUpdateCount <<= 1;
        if (matchUpdateCount == (1 << 8))
        {
            matchUpdateCount        = 8;
            matchConfig0.matchValue = CTIMER_CLK_FREQ / 2;
        }
        CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);
    }
#if defined(BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED)
    /* No timer match output pin connected to a LED
     * toggle LED manually according to match status
     */
    if (CTIMER_GetOutputMatchStatus(CTIMER, CTIMER_EMT0_OUT))
    {
        LED_RED1_ON();
    }
    else
    {
        LED_RED1_OFF();
    }
#endif
}

/*!
 * @brief Main function
 */
int main(void)
{
    ctimer_config_t config;

    /* Init hardware*/
    CLOCK_SetClockDiv(kCLOCK_DivCTIMER1, 1u);
    CLOCK_AttachClk(kFRO_HF_to_CTIMER1);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

#if defined(BOARD_HAS_NO_CTIMER_OUTPUT_PIN_CONNECTED_TO_LED)
    LED_RED1_INIT(LOGIC_LED_OFF);
    LED_RED2_INIT(LOGIC_LED_OFF);
#endif

    PRINTF("CTimer match example to toggle the output. \r\n");
    PRINTF("This example uses interrupt to change the match period. \r\n");

    CTIMER_GetDefaultConfig(&config);

    CTIMER_Init(CTIMER, &config);

    /* Configuration 0 */
    matchConfig0.enableCounterReset = true;
    matchConfig0.enableCounterStop  = false;
    matchConfig0.matchValue         = CTIMER_CLK_FREQ / 2;
    matchConfig0.outControl         = kCTIMER_Output_Toggle;
    matchConfig0.outPinInitState    = false;
    matchConfig0.enableInterrupt    = true;

    /* Configuration 1 */
    matchConfig1.enableCounterReset = true;
    matchConfig1.enableCounterStop  = false;
    matchConfig1.matchValue         = CTIMER_CLK_FREQ / 2;
    matchConfig1.outControl         = kCTIMER_Output_Toggle;
    matchConfig1.outPinInitState    = true;
    matchConfig1.enableInterrupt    = true;

    /* Create different ctimer_callback_table array for different CTimer instance. */
    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback_table[0], kCTIMER_MultipleCallback);

    /*
     * Macros CTIMER_MAT0_OUT and CTIMER_MAT1_OUT are nominal match output, instead of
     * hardware MR0 and MR1 register match output.
     * So CTIMER_MAT0_OUT can be defined as kCTIMER_Match_1, CTIMER_MAT1_OUT can be defined
     * as kCTIMER_Match_3, which means they are MR1 and MR3 register match output.
     */
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT0_OUT, &matchConfig0);
    CTIMER_SetupMatch(CTIMER, CTIMER_MAT1_OUT, &matchConfig1);
    CTIMER_StartTimer(CTIMER);

    while (1)
    {
    }
}
