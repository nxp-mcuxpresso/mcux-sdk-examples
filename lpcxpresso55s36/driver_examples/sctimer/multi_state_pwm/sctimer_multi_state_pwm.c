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

#include "fsl_inputmux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SCTIMER_CLK_FREQ        CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_FIRST_SCTIMER_OUT  kSCTIMER_Out_0
#define DEMO_SECOND_SCTIMER_OUT kSCTIMER_Out_4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    sctimer_config_t sctimerInfo;
    sctimer_pwm_signal_param_t pwmParam;
    uint32_t stateNumber;
    uint32_t eventFirstNumberOutput, eventSecondNumberOutput, eventNumberInput;
    uint32_t sctimerClock;

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* enable clock for GPIO*/
    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* configure the input mux for the sct timer input1 from external pin*/
    INPUTMUX_Init(INPUTMUX);
    INPUTMUX_AttachSignal(INPUTMUX, 1U, kINPUTMUX_SctGpioInHToSct0);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    sctimerClock = SCTIMER_CLK_FREQ;

    /* Print a note to terminal */
    PRINTF("\r\nSCTimer example to output edge-aligned PWM signal\r\n");
    PRINTF("\r\nWhen user presses a switch the PWM signal will be seen from Out %d  ",
           (uint32_t)DEMO_SECOND_SCTIMER_OUT);
    PRINTF("\r\nWhen user presses the switch again PWM signal on Out %d will turn off ",
           (uint32_t)DEMO_SECOND_SCTIMER_OUT);
    PRINTF("\r\nThe PWM signal from Out %d will remain active all the time ", (uint32_t)DEMO_FIRST_SCTIMER_OUT);

    /* Default configuration operates the counter in 32-bit mode */
    SCTIMER_GetDefaultConfig(&sctimerInfo);

    /* Initialize SCTimer module */
    SCTIMER_Init(SCT0, &sctimerInfo);

    stateNumber = SCTIMER_GetCurrentState(SCT0);

    /* Configure PWM params with frequency 24kHZ from first output */
    pwmParam.output           = DEMO_FIRST_SCTIMER_OUT;
    pwmParam.level            = kSCTIMER_HighTrue;
    pwmParam.dutyCyclePercent = 10;

    /* Schedule events in current state; State 0 */
    /* Schedule events for generating a 24KHz PWM with 10% duty cycle from first Out in the current state */
    if (SCTIMER_SetupPwm(SCT0, &pwmParam, kSCTIMER_EdgeAlignedPwm, 24000U, sctimerClock, &eventFirstNumberOutput) ==
        kStatus_Fail)
    {
        return -1;
    }

    /* Schedule an event to look for a rising edge on input 1 in this state */
    if (SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputRiseEvent, 0, kSCTIMER_Input_1, kSCTIMER_Counter_U,
                                       &eventNumberInput) == kStatus_Fail)
    {
        return -1;
    }

    /* Transition to next state when a rising edge is detected on input 1 */
    SCTIMER_SetupNextStateActionwithLdMethod(SCT0, stateNumber + 1, eventNumberInput, true);

    /* Go to next state; State 1 */
    SCTIMER_IncreaseState(SCT0);

    /* Schedule events in State 1 */
    /* Schedule events for generating a 24KHz PWM with 50% duty cycle from second Out in this new state */
    pwmParam.output           = DEMO_SECOND_SCTIMER_OUT;
    pwmParam.dutyCyclePercent = 50;
    if (SCTIMER_SetupPwm(SCT0, &pwmParam, kSCTIMER_EdgeAlignedPwm, 24000U, sctimerClock, &eventSecondNumberOutput) ==
        kStatus_Fail)
    {
        return -1;
    }

    /* Re-enable PWM coming out from Out 4 by scheduling the PWM events in this new state */
    /* To get a PWM, the SCTIMER_SetupPwm() function creates 2 events; 1 for the pulse period and
     * and 1 for the pulse, we need to schedule both events in this new state
     */
    /* Schedule the period event for the PWM */
    SCTIMER_ScheduleEvent(SCT0, eventFirstNumberOutput);
    /* Schedule the pulse event for the PWM */
    SCTIMER_ScheduleEvent(SCT0, eventFirstNumberOutput + 1);

    /* Schedule an event to look for a rising edge on input 1 in this state */
    if (SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputRiseEvent, 0, kSCTIMER_Input_1, kSCTIMER_Counter_U,
                                       &eventNumberInput) == kStatus_Fail)
    {
        return -1;
    }

    /* Transition back to State 0 when a rising edge is detected on input 1 */
    /* State 0 has only 1 PWM active, there will be no PWM from Out 2 */
    SCTIMER_SetupNextStateActionwithLdMethod(SCT0, stateNumber, eventNumberInput, true);

    /* Start the 32-bit unify timer */
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);

    while (1)
    {
    }
}
