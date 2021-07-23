/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS header */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"
#include "lpm.h"
#include "fsl_gpt.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Systick counter source clock frequency. Need config the Systick counter's(GPT) clock
 * source to this frequency in application.
 */
#define SYSTICK_COUNTER_SOURCE_CLK_FREQ (SYSTICK_CLOCK)
/* Define the counter clock of the systick (GPT).*/
#define SYSTICK_COUNTER_FREQ (8000000U)
/* Define the count per tick of the systick in run mode. For accuracy purpose,
 * please make SYSTICK_SOURCE_CLOCK times of configTICK_RATE_HZ.
 */
#define SYSTICK_COUNT_PER_TICK (SYSTICK_COUNTER_FREQ / configTICK_RATE_HZ)

/* If block envent exists, then do not allow M7 to enter low power mode.*/
static uint32_t s_BlockEventCnt = 0;
/* FreeRTOS implemented Systick handler. */
extern void xPortSysTickHandler(void);
/*******************************************************************************
 * Code
 ******************************************************************************/
/* Override the default definition of vPortSetupTimerInterrupt() that is weakly
 * defined in the FreeRTOS Cortex-M7F port layer with a version that configures
 * GPT to generate the tick interrupt. */
void vPortSetupTimerInterrupt(void)
{
    gpt_config_t config;
    GPT_GetDefaultConfig(&config);
    config.enableRunInDoze = true;
    config.clockSource     = kGPT_ClockSource_Osc;

    /* Initialize timer */
    GPT_Init(SYSTICK_BASE, &config);
    /* Divide GPT clock source frequency to SYSTICK_COUNTER_FREQ */
    GPT_SetOscClockDivider(SYSTICK_BASE, SYSTICK_COUNTER_SOURCE_CLK_FREQ / SYSTICK_COUNTER_FREQ);

    /* Set timer period */
    GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, SYSTICK_COUNT_PER_TICK - 1U);
    /* Enable timer interrupt */
    GPT_EnableInterrupts(SYSTICK_BASE, kGPT_OutputCompare1InterruptEnable);

    NVIC_SetPriority(SYSTICK_IRQn, configLIBRARY_LOWEST_INTERRUPT_PRIORITY);
    /* Enable NVIC interrupt */
    NVIC_EnableIRQ(SYSTICK_IRQn);
    /* Start counting */
    GPT_StartTimer(SYSTICK_BASE);
}

uint32_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec, uint64_t *pCounter)
{
    uint32_t counter, expired = 0;
    uint32_t ms, maxMS;
    uint32_t flag;
    uint32_t timeoutTicks;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;

    /* If timeout < 2 ticks, don't do tickless idle. */
    if ((uint64_t)timeoutMilliSec * configTICK_RATE_HZ < 2 * 1000)
    {
        return 0;
    }

    maxMS = 0xFFFFFFFFU / SYSTICK_COUNTER_FREQ * 1000;
    ms    = timeoutMilliSec > maxMS ? maxMS : timeoutMilliSec;

    /* Calculate the timer counter needed for timeout */
    timeoutTicks = (uint64_t)ms * configTICK_RATE_HZ / 1000;
    counter      = timeoutTicks * countPerTick;

    GPT_StopTimer(SYSTICK_BASE); /* Timer stopped. */
    flag = GPT_GetStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);
    if (flag)
    {
        GPT_ClearStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);
        NVIC_ClearPendingIRQ(SYSTICK_IRQn);
        expired = countPerTick;
    }
    expired += GPT_GetCurrentTimerCount(SYSTICK_BASE);
    /* Minus those already expired to get accurate waiting counter. */
    counter -= expired;

    /* Enable GPT free-run mode, the counter is not reset when compare events occur.
     * In this way, the counter is not overflow during tickless and the exact time
     * since tickless enter can be calculated.
     */
    SYSTICK_BASE->CR |= GPT_CR_FRR_MASK;
    /* Convert count in systick freq to tickless clock count */
    GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, counter - 1UL);
    /* Restart timer, GPT CR_ENMOD=1, counter value is reset when restart timer. */
    GPT_StartTimer(SYSTICK_BASE);

    /* return waiting counter */
    *pCounter = counter;

    return timeoutTicks;
}

void LPM_ExitTicklessIdle(uint32_t timeoutTicks, uint64_t timeoutCounter)
{
    uint32_t flag, counter, expired, expiredTicks;
    uint32_t completeTicks;
    uint32_t countPerTick = SYSTICK_COUNT_PER_TICK;

    GPT_StopTimer(SYSTICK_BASE);
    flag = GPT_GetStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);
    /* Convert tickless count to systick count. */
    expired = GPT_GetCurrentTimerCount(SYSTICK_BASE);

    if (flag)
    {
        /* If counter already exceeds 1 tick, it means wakeup takes too much time
         * and we have already lost some ticks. Calcaute and add the
         * lost ticks into completeTicks. Completed ticks minus 1 because pending interrupt will be handled immediately
         * when interrupt unmasked.
         */
        expiredTicks  = (expired - timeoutCounter) / countPerTick;
        completeTicks = expiredTicks + timeoutTicks - 1;

        /* Continue the uncompleted tick. */
        counter = countPerTick - (expired - timeoutCounter) % countPerTick;
    }
    else
    {
        /* Remaining counter. */
        counter       = timeoutCounter - expired;
        completeTicks = timeoutTicks - (counter - 1) / countPerTick - 1;
        counter       = (counter - 1) % countPerTick + 1;
    }

    /* Now reinit Systick. */
    SYSTICK_BASE->CR &= ~GPT_CR_FRR_MASK;
    GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, counter - 1UL);
    /* Restart timer */
    GPT_StartTimer(SYSTICK_BASE);

    vTaskStepTick(completeTicks);
}

/* The systick interrupt handler. */
void SYSTICK_HANDLER(void)
{
    /* Clear interrupt flag.*/
    GPT_ClearStatusFlags(SYSTICK_BASE, kGPT_OutputCompare1Flag);

    /* This is the first tick since the MCU left a low power mode the
     * compare value need to be reset to its default.
     */
    if (GPT_GetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1) != SYSTICK_COUNT_PER_TICK - 1)
    {
        /* Counter will be reset and cause minor accuracy loss */
        GPT_SetOutputCompareValue(SYSTICK_BASE, kGPT_OutputCompare_Channel1, SYSTICK_COUNT_PER_TICK - 1);
    }

    /* Call FreeRTOS tick handler. */
    xPortSysTickHandler();

    /* Add for ARM errata 838869, affects Cortex-M7, Cortex-M7F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void LPM_IncreseBlockSleepCnt(void)
{
    s_BlockEventCnt++;
}

void LPM_DecreaseBlockSleepCnt(void)
{
    s_BlockEventCnt--;
}

bool LPM_AllowSleep(void)
{
    if (s_BlockEventCnt)
    {
        return false;
    }
    else
    {
        return true;
    }
}
