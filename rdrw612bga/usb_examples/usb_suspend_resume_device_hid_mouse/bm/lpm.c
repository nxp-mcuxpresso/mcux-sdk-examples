/*
 * Copyright 2020, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef FSL_RTOS_FREE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#endif

#include "fsl_ostimer.h"
#include "fsl_power.h"

#include "lpm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PM2_WAKEUP_TIME (2U)

static uint32_t s_curMode;
static power_sleep_config_t s_slpCfg;
static lpm_config_t s_lpmCfg;

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint64_t LPM_EnterTicklessIdle(uint32_t timeoutMilliSec)
{
    status_t status;
    uint32_t ostmrFreq = CLOCK_GetOSTimerClkFreq();
    uint64_t counter   = OSTIMER_GetCurrentTimerValue(OSTIMER);
    uint64_t match;

    /* Translate the millisecond to ostimer count value. */
    match = counter + MSEC_TO_COUNT(timeoutMilliSec, ostmrFreq);

    OSTIMER_ClearStatusFlags(OSTIMER, kOSTIMER_MatchInterruptFlag);
    /* Set the match value with unit of ticks. */
    status = OSTIMER_SetMatchValue(OSTIMER, match, NULL);
    if (status != kStatus_Success)
    {
        assert(false);
    }

    return counter;
}

static void LPM_ExitTicklessIdle(uint32_t base)
{
#ifdef FSL_RTOS_FREE_RTOS
    uint64_t counter;
    uint32_t ticks;
    uint64_t expireMs;
    uint32_t freq;
#endif

    if ((OSTIMER_GetStatusFlags(OSTIMER) & (uint32_t)kOSTIMER_MatchInterruptFlag) == 0U)
    {
        /* Not woken up from OSTMR, further alarm is not needed. */
        (void)DisableIRQ(OS_EVENT_IRQn);
    }

#ifdef FSL_RTOS_FREE_RTOS
    /* Get current OSTMR counter. */
    counter = OSTIMER_GetCurrentTimerValue(OSTIMER);
    freq    = CLOCK_GetOSTimerClkFreq();
    assert(freq != 0U);
    expireMs = (counter - base) * 1000U / freq;
    ticks    = (uint64_t)expireMs * configTICK_RATE_HZ / 1000U;

    vTaskStepTick(ticks);
#endif
}

void LPM_Init(const lpm_config_t *config)
{
    assert(config);

    s_curMode = 0U;
    memcpy(&s_lpmCfg, config, sizeof(s_lpmCfg));

    OSTIMER_Init(OSTIMER);
}

void LPM_Deinit(void)
{
}

void LPM_EnableWakeupSource(IRQn_Type source)
{
    if (source == PIN0_INT_IRQn)
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin0, kPOWER_WakeupEdgeLow);
        NVIC_ClearPendingIRQ(PIN0_INT_IRQn);
    }
    else if (source == PIN1_INT_IRQn)
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeLow);
        NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    }
    else
    {
        /* Do nothing */
    }
    EnableIRQ(source);
    POWER_ClearWakeupStatus(source);
    POWER_EnableWakeup(source);
}

void LPM_DisableWakeupSource(IRQn_Type source)
{
    if (source == PIN0_INT_IRQn)
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin0, kPOWER_WakeupEdgeHigh);
        NVIC_ClearPendingIRQ(PIN0_INT_IRQn);
    }
    else if (source == PIN1_INT_IRQn)
    {
        POWER_ConfigWakeupPin(kPOWER_WakeupPin1, kPOWER_WakeupEdgeHigh);
        NVIC_ClearPendingIRQ(PIN1_INT_IRQn);
    }
    else
    {
        /* Do nothing */
    }
    DisableIRQ(source);
    POWER_ClearWakeupStatus(source);
    POWER_DisableWakeup(source);
}

void LPM_SetPowerMode(uint32_t mode, const power_sleep_config_t *config)
{
    assert(config != NULL);

    s_curMode = mode;
    memcpy(&s_slpCfg, config, sizeof(s_slpCfg));
}

uint32_t LPM_GetPowerMode(void)
{
    return s_curMode;
}

uint32_t LPM_WaitForInterrupt(uint32_t timeoutMilliSec)
{
    uint32_t irqMask;
    uint64_t count;
    uint32_t mode      = s_curMode;
    uint32_t threshold = s_lpmCfg.threshold;

#ifdef FSL_RTOS_FREE_RTOS
    /* tickless only when timeout longer than 2 ticks and preset threshold */
    threshold = MAX(2000U / configTICK_RATE_HZ, threshold);
#endif

    irqMask = DisableGlobalIRQ();

    if (mode == 2U)
    {
        /* Only when sleep time is larger than PM2 wakeup time, PM2 will be entered. */
        threshold = MAX(PM2_WAKEUP_TIME, threshold);
        /* If PM2 sleep time is not larger than the threshold, enter PM1 mode instead. */
        if (timeoutMilliSec <= threshold)
        {
            mode = 1U;
        }
        else
        {
            /* In PM2, systick is disabled and OSTIMER acts as the wakeup source for certain period */
            LPM_EnableWakeupSource(OS_EVENT_IRQn);
        }
    }

    switch (mode)
    {
        case 1U:
            if (timeoutMilliSec <= threshold)
            {
                /* Wait short time, no tickless idle needed. */
                POWER_SetSleepMode(1U);
                __WFI();
                break;
            }
            /* Fall through */
        case 2U:
            count = LPM_EnterTicklessIdle(timeoutMilliSec - ((mode == 2U) ? PM2_WAKEUP_TIME : 0U));
            POWER_EnterPowerMode(mode, &s_slpCfg);
            LPM_ExitTicklessIdle(count);
            break;
        default:
            /* PM3/PM4 will only be waken up by application configured source. */
            POWER_EnterPowerMode(mode, &s_slpCfg);
            if (mode == 3U)
            {
                /* Need to reinitialize OSTIMER on PM3 wakeup */
                OSTIMER_Init(OSTIMER);
            }
            break;
    }

    if (mode == 2U)
    {
        LPM_DisableWakeupSource(OS_EVENT_IRQn);
    }

    EnableGlobalIRQ(irqMask);

    return mode;
}
