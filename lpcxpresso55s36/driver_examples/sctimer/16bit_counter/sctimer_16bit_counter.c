/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_sctimer.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SCTIMER_CLK_FREQ        CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_FIRST_SCTIMER_OUT  kSCTIMER_Out_0
#define DEMO_SECOND_SCTIMER_OUT kSCTIMER_Out_4
#define MAX_UP_COUNTER_VALUE     (0xFFFFU * 256U)
#define MAX_UPDOWN_COUNTER_VALUE (0x1FFFFU * 256U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t SCTIMER_Calculate16BitCounterConfig(uint64_t rawCountValue,
                                                    uint8_t *prescale,
                                                    uint16_t *matchValue,
                                                    sctimer_event_active_direction_t *activeDir)
{
    status_t status = kStatus_Success;

    if (rawCountValue < MAX_UP_COUNTER_VALUE)
    {
        *prescale   = (uint8_t)(rawCountValue / 0xFFFFU);
        *matchValue = (uint16_t)(rawCountValue / (*prescale + 1U));
        *activeDir  = kSCTIMER_ActiveIndependent;
    }
    else if (rawCountValue < MAX_UPDOWN_COUNTER_VALUE)
    {
        *prescale   = (uint8_t)(rawCountValue / 0x1FFFFU);
        *matchValue = (uint16_t)(0x1FFFFU - rawCountValue / (*prescale + 1));
        *activeDir  = kSCTIMER_ActiveInCountDown;
    }
    else
    {
        status = kStatus_Fail;
    }
    return status;
}
/*!
 * @brief Main function
 */
int main(void)
{
    sctimer_config_t sctimerInfo;
    uint32_t eventCounterL, eventCounterH;
    uint32_t sctimerClock;
    uint16_t matchValueL, matchValueH;
    sctimer_event_active_direction_t activeDirL, activeDirH;

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    sctimerClock = SCTIMER_CLK_FREQ;

    /* Print a note to terminal */
    PRINTF("\r\nSCTimer example to use it in 16-bit mode\r\n");
    PRINTF("\r\nThe example shows both 16-bit counters running and toggling an output periodically  ");

    SCTIMER_GetDefaultConfig(&sctimerInfo);
/* Add judgement for change colck source*/
#if defined(SCTIMER_NEED_CHANGE_CLOCK_SOURCE)
    sctimerInfo.clockMode   = DEMO_CLOCK_MODE;
    sctimerInfo.clockSelect = DEMO_CLOCK_SEL;
#endif

    /* Switch to 16-bit mode, if we want to use any 16-bit counter, no matter the Low 16-bit one or the
     * High 16-bit one, we need to disable the Unify 32-bit Counter by hardware limit.
     */
    sctimerInfo.enableCounterUnify = false;

    /* Calculate prescaler, match value and active direction for the 16-bit low counter for 100ms interval */
    if (SCTIMER_Calculate16BitCounterConfig(MSEC_TO_COUNT(100U, sctimerClock), &sctimerInfo.prescale_l, &matchValueL,
                                            &activeDirL) == kStatus_Fail)
    {
        PRINTF("\r\nSCTimer 16-bit low counter is out of range\r\n");
        return -1;
    }
    /* Calculate prescaler, match value and active direction for the 16-bit high counter for 200ms interval */
    if (SCTIMER_Calculate16BitCounterConfig(MSEC_TO_COUNT(200U, sctimerClock), &sctimerInfo.prescale_h, &matchValueH,
                                            &activeDirH) == kStatus_Fail)
    {
        PRINTF("\r\nSCTimer 16-bit high counter is out of range\r\n");
        return -1;
    }

    /* Enable bidirectional mode to extended 16-bit count range*/
    if (activeDirL != kSCTIMER_ActiveIndependent)
    {
        sctimerInfo.enableBidirection_l = true;
    }
    if (activeDirH != kSCTIMER_ActiveIndependent)
    {
        sctimerInfo.enableBidirection_h = true;
    }

    /* Initialize SCTimer module */
    SCTIMER_Init(SCT0, &sctimerInfo);

    /* Schedule a match event for the 16-bit low counter every 0.1 seconds */
    if (SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_MatchEventOnly, matchValueL, 0, kSCTIMER_Counter_L,
                                       &eventCounterL) == kStatus_Fail)
    {
        return -1;
    }

    /* Toggle first output when the 16-bit low counter event occurs */
    SCTIMER_SetupOutputToggleAction(SCT0, DEMO_FIRST_SCTIMER_OUT, eventCounterL);

    /* Reset Counter L when the 16-bit low counter event occurs */
    SCTIMER_SetupCounterLimitAction(SCT0, kSCTIMER_Counter_L, eventCounterL);

    /* Setup the 16-bit low counter event active direction */
    SCTIMER_SetupEventActiveDirection(SCT0, activeDirL, eventCounterL);

    /* Schedule a match event for the 16-bit high counter every 0.2 seconds */
    if (SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_MatchEventOnly, matchValueH, 0, kSCTIMER_Counter_H,
                                       &eventCounterH) == kStatus_Fail)
    {
        return -1;
    }

    /* Setup the 16-bit high counter event active direction */
    SCTIMER_SetupEventActiveDirection(SCT0, activeDirH, eventCounterH);

    /* Toggle second output when the 16-bit high counter event occurs */
    SCTIMER_SetupOutputToggleAction(SCT0, DEMO_SECOND_SCTIMER_OUT, eventCounterH);

    /* Reset Counter H when the 16-bit high counter event occurs */
    SCTIMER_SetupCounterLimitAction(SCT0, kSCTIMER_Counter_H, eventCounterH);

    /* Start the 16-bit low and high counter */
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_L | kSCTIMER_Counter_H);

    while (1)
    {
    }
}
