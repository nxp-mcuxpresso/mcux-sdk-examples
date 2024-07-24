/*!
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_sctimer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* SCTimerClock: 260MHz/(25+1) = 10MHz */
#define SCTIMER_CLK_PRESCALE    (25U)
#define SCTIMER_CLK_FREQ        CLOCK_GetCoreSysClkFreq()
#define SCTIMER_INPUT_PIN       kSCTIMER_Input_1

/* Interrupt number and interrupt handler for the FTM instance used */
#define SCTIMER_INPUT_CAPTURE_HANDLER SCT0_IRQHandler


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool sctimerIsrFlag  = false;
volatile uint32_t g_capValue1 = 0;
volatile uint32_t g_capValue2 = 0;
/* Record SCTIMER Limit interrupt times */
volatile uint32_t g_timerLimitInterruptCount = 0;
volatile uint32_t g_inputRiseLimitCount      = 0;
volatile uint32_t g_inputFallLimitCount      = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SCTIMER_INPUT_CAPTURE_HANDLER(void)
{
    uint32_t statusFlags = 0;

    statusFlags = SCTIMER_GetStatusFlags(SCT0);
    /* Clear interrupt flag.*/
    SCTIMER_ClearStatusFlags(SCT0, statusFlags);

    if (statusFlags & (1 << 4U))
    {
        g_timerLimitInterruptCount++;
    }
    if (statusFlags & (1 << 2U))
    {
        g_capValue1           = SCTIMER_GetCaptureValue(SCT0, kSCTIMER_Counter_U, 2U);
        g_inputRiseLimitCount = g_timerLimitInterruptCount;
    }
    if (statusFlags & (1 << 3U))
    {
        g_capValue2           = SCTIMER_GetCaptureValue(SCT0, kSCTIMER_Counter_U, 3U);
        g_inputFallLimitCount = g_timerLimitInterruptCount;
        sctimerIsrFlag        = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    sctimer_config_t sctimerInfo;
    uint32_t currentEvent        = 0;
    uint32_t captureReg          = 0;
    uint32_t capValue1           = 0;
    uint32_t capValue2           = 0;
    uint32_t inputRiseLimitCount = 0;
    uint32_t inputFallLimitCount = 0;
    uint32_t pulseWidth          = 0;

    /* Board pin, clock, debug console init */
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nSCTimer edge capture example\r\n");
    PRINTF("\r\nOnce the input signal is received the capture values are printed");
    PRINTF("\r\nThe input signal's pulse width is calculated from the capture values & printed\r\n");

    SCTIMER_GetDefaultConfig(&sctimerInfo);

    /* SCTimerClock: 10MHz */
    sctimerInfo.prescale_l = SCTIMER_CLK_PRESCALE;

    /* Initialize SCTimer module */
    (void)SCTIMER_Init(SCT0, &sctimerInfo);

    /*  If high level input is detected on the channel when SCTimer timer starts, SCTimer will
     *  generate a rising edge capture event incorrectly although there is no rising edge.
     *  So this example ignore first pluse and capture second pluse. SCTimer capture second pluse
     *  in state 1.
     */

    /* Event 0 is triggered when first rising edge occurs in state 0. */
    /* Event 0 cause Capture 0. */
    (void)SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputRiseEvent, 0, SCTIMER_INPUT_PIN, kSCTIMER_Counter_U,
                                         &currentEvent);
    (void)SCTIMER_SetupCaptureAction(SCT0, kSCTIMER_Counter_U, &captureReg, currentEvent);
     
    /* Event 1 is triggered when first falling edge occurs in state 0. */
    /* Event 1 cause Capture 1. */
    (void)SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputFallEvent, 0, SCTIMER_INPUT_PIN, kSCTIMER_Counter_U,
                                         &currentEvent);
    (void)SCTIMER_SetupCaptureAction(SCT0, kSCTIMER_Counter_U, &captureReg, currentEvent);     
    /* After event 1 is triggered, new state is state 1. */
    SCTIMER_SetupNextStateActionwithLdMethod(SCT0, 1U, currentEvent, true);
    (void)SCTIMER_IncreaseState(SCT0);

    /* Event 2 is triggered when following rising edge occurs in state 1. */
    /* Event 2 cause Capture 2. */
    (void)SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputRiseEvent, 0, SCTIMER_INPUT_PIN, kSCTIMER_Counter_U,
                                         &currentEvent);
    (void)SCTIMER_SetupCaptureAction(SCT0, kSCTIMER_Counter_U, &captureReg, currentEvent);     
     
    /* Event 3 is triggered when following falling edge occurs in state 1. */
    /* Event 3 cause Capture 3. */
    (void)SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_InputFallEvent, 0, SCTIMER_INPUT_PIN, kSCTIMER_Counter_U,
                                         &currentEvent);
    (void)SCTIMER_SetupCaptureAction(SCT0, kSCTIMER_Counter_U, &captureReg, currentEvent);

    /* Event 4 is used as counter limit(reset). */
    /* Event 4 causes Match 4, match value is 65535. */
    (void)SCTIMER_CreateAndScheduleEvent(SCT0, kSCTIMER_MatchEventOnly, 0xFFFF, 0, kSCTIMER_Counter_U, &currentEvent);
    /* Event 4 happens in state 0 and 1, state 1 has been set. */
    SCTIMER_SetEventInState(SCT0, currentEvent, 0);
    SCTIMER_SetupCounterLimitAction(SCT0, kSCTIMER_Counter_U, currentEvent);

    /* Event 2,3,4 generate interrupt. */
    SCTIMER_EnableInterrupts(SCT0, (1 << 2U) | (1 << 3U) | (1 << 4U));

    /* Enable SCTimer interrupt */
    (void)EnableIRQ(SCT0_IRQn);

    /* Start timer */
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);

    while (sctimerIsrFlag != true)
    {
    }

    /* Disable capture interrupt to prevent capture and limit value be changed in interrupt */
    SCTIMER_DisableInterrupts(SCT0, (1 << 2U) | (1 << 3U));

    capValue1           = g_capValue1;
    capValue2           = g_capValue2;
    inputRiseLimitCount = g_inputRiseLimitCount;
    inputFallLimitCount = g_inputFallLimitCount;

    SCTIMER_EnableInterrupts(SCT0, (1 << 2U) | (1 << 3U));

    pulseWidth = (((inputFallLimitCount - inputRiseLimitCount) * 65536U + capValue2 - capValue1) + 1U) / 10U;

    PRINTF("\r\nCapture2: %d Capture3: %d Pluse Width: %d us\r\n", capValue1, capValue2, pulseWidth);

    while (1)
    {
    }
}
