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
#include "clock_config.h"
#include "board.h"
#include "fsl_ctimer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CTIMER          CTIMER5
#define CTIMER_MAT_OUT  kCTIMER_Match_2
#define CTIMER_CLK_FREQ CLOCK_GetCTimerClkFreq(5U)
#ifndef CTIMER_MAT_PWM_PERIOD_CHANNEL
#define CTIMER_MAT_PWM_PERIOD_CHANNEL kCTIMER_Match_3
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void pwm_match_callback(uint32_t flags);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t dutyCycle        = 99;
volatile uint32_t g_pwmPeriod   = 0U;
volatile uint32_t g_pulsePeriod = 0U;

static ctimer_callback_t ctimer_callback[] = {pwm_match_callback};

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t CTIMER_GetPwmPeriodValue(uint32_t pwmFreqHz, uint8_t dutyCyclePercent, uint32_t timerClock_Hz)
{
    /* Calculate PWM period match value */
    g_pwmPeriod = (timerClock_Hz / pwmFreqHz) - 1U;

    /* Calculate pulse width match value */
    g_pulsePeriod = (g_pwmPeriod + 1U) * (100 - dutyCyclePercent) / 100;

    return kStatus_Success;
}
status_t CTIMER_UpdatePwmPulsePeriodValue(uint8_t dutyCyclePercent)
{
    /* Calculate pulse width match value */
    g_pulsePeriod = (g_pwmPeriod + 1U) * (100 - dutyCyclePercent) / 100;

    return kStatus_Success;
}

void pwm_match_callback(uint32_t flags)
{
    static uint32_t count    = 0;
    static uint8_t decrement = 1;

    if ((flags & (1 << CTIMER_MAT_OUT)) && (++count > 100))
    {
        count = 0;
        /* Update pulse width match value after the PWM duty cycle is changed */
        CTIMER_UpdatePwmPulsePeriodValue(dutyCycle);
        CTIMER_UpdatePwmPulsePeriod(CTIMER, CTIMER_MAT_OUT, g_pulsePeriod);
        dutyCycle = dutyCycle + 1 - decrement * 2;
        if (dutyCycle == 69)
        {
            decrement = 0;
        }
        else if (dutyCycle == 99)
        {
            decrement = 1;
        }
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    ctimer_config_t config;
    uint32_t srcClock_Hz;
    uint32_t timerClock;

    /* Init hardware*/
    BOARD_InitAHBSC();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_AttachClk(kFRO2_DIV1_to_CTIMER5);
    CLOCK_SetClkDiv(kCLOCK_DivCtimer5Clk, 10U);

    /* CTimer0 counter uses the AHB clock, some CTimer1 modules use the Aysnc clock */
    srcClock_Hz = CTIMER_CLK_FREQ;

    PRINTF("CTimer example to generate a PWM signal\r\n");
    PRINTF("This example uses interrupts to update the PWM duty cycle\r\n");

    CTIMER_GetDefaultConfig(&config);
    timerClock = srcClock_Hz / (config.prescale + 1);

    CTIMER_Init(CTIMER, &config);

    CTIMER_RegisterCallBack(CTIMER, &ctimer_callback[0], kCTIMER_SingleCallback);
    /* Get the PWM period match value and pulse width match value of 2Khz PWM signal */
    CTIMER_GetPwmPeriodValue(2000, dutyCycle, timerClock);
    CTIMER_SetupPwmPeriod(CTIMER, CTIMER_MAT_PWM_PERIOD_CHANNEL, CTIMER_MAT_OUT, g_pwmPeriod, g_pulsePeriod, true);
    CTIMER_StartTimer(CTIMER);

    while (1)
    {
    }
}
