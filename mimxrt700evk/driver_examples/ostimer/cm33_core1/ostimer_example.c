/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ostimer.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_OSTIMER_FREQ         CLOCK_GetLpOscFreq()
#define EXAMPLE_OSTIMER              OSTIMER_CPU1
#define EXAMPLE_OSTIMER_IRQn         OS_EVENT_IRQn
#define EXAMPLE_EnableDeepSleepIRQ() EnableDeepSleepIRQ(OS_EVENT_IRQn)

#define APP_DEEPSLEEP_SLEEPCFG    (0U)
#define APP_DEEPSLEEP_PDSLEEPCFG0 (0U)
#define APP_DEEPSLEEP_RAM_APD     0x3FFC0000U /* 0x580000 - 0x67FFFF([PT18-PT26]) keep powered */
#define APP_DEEPSLEEP_RAM_PPD     0x0U
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                     \
    (((const uint32_t[]){APP_DEEPSLEEP_SLEEPCFG, APP_DEEPSLEEP_PDSLEEPCFG0, 0U, APP_DEEPSLEEP_RAM_APD, \
                         APP_DEEPSLEEP_RAM_PPD, 0U, 0U}))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Enter deep sleep mode. */
void EXAMPLE_EnterDeepSleep(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool matchFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void EXAMPLE_EnterDeepSleep(void)
{
    POWER_SelectSleepSetpoint(kRegulator_Vdd1LDO, 0U); /* Select lowest voltage when DS. */
    POWER_EnableSleepRBB(kPower_BodyBiasVdd1 | kPower_BodyBiasVdd1Sram);
    /* Enter deep sleep mode by using power API. */
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
}

/* User Callback. */
void EXAMPLE_OstimerCallback(void)
{
    matchFlag = true;
    /* User code. */
}

/* Set the match value with unit of millisecond. OStimer will trigger a match interrupt after the a certain time. */
static status_t EXAMPLE_SetMatchInterruptTime(OSTIMER_Type *base, uint32_t ms, uint32_t freq, ostimer_callback_t cb)
{
    uint64_t timerTicks = OSTIMER_GetCurrentTimerValue(base);

    /* Translate the millisecond to ostimer count value. */
    timerTicks += MSEC_TO_COUNT(ms, freq);

    /* Set the match value with unit of ticks. */
    return OSTIMER_SetMatchValue(base, timerTicks, cb);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    BOARD_InitAHBSC();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    /* Enable the used modules in sense side. */
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSEP_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_SHUT_SENSES_MAINCLK);
    POWER_DisablePD(kPDRUNCFG_GATE_FRO2);
    POWER_DisablePD(kPDRUNCFG_PD_FRO2);
    POWER_DisablePD(kPDRUNCFG_SHUT_RAM1_CLK);

    PMC1->PDRUNCFG2 &= ~(0x3FFC0000U); /* Power up all the SRAM partitions in Sense domain. */
    PMC1->PDRUNCFG3 &= ~(0x3FFC0000U);

    POWER_ApplyPD();

    CLOCK_AttachClk(kLPOSC_to_OSTIMER);
    CLOCK_SetClkDiv(kCLOCK_DivOstimerClk, 1U);

    PRINTF("Press any key to start example.\r\n\r\n");
    GETCHAR();
    PRINTF("Board will enter power deep sleep mode, and then wakeup by OS timer after about 5 seconds.\r\n");
    PRINTF("After Board wakeup, the OS timer will trigger the match interrupt about every 2 seconds.\r\n");

    /* Intialize the OS timer, setting clock configuration. */
    OSTIMER_Init(EXAMPLE_OSTIMER);

    matchFlag = false;

    /* Set the OS timer match value. */
    if (kStatus_Success ==
        EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 5000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
    {
        /* Enable OSTIMER IRQ under deep sleep mode. */
        EXAMPLE_EnableDeepSleepIRQ();
        /* Enter deep sleep mode. */
        EXAMPLE_EnterDeepSleep();

        /* Wait until OS timer interrupt occurrs. */
        while (!matchFlag)
        {
        }

        /* Wakeup from deep sleep mode. */
        PRINTF("Board wakeup from deep sleep mode.\r\n\r\n");
    }
    else
    {
        PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
    }

    while (1)
    {
        matchFlag = false;

        /* Set the match value to trigger the match interrupt. */
        if (kStatus_Success ==
            EXAMPLE_SetMatchInterruptTime(EXAMPLE_OSTIMER, 2000U, EXAMPLE_OSTIMER_FREQ, EXAMPLE_OstimerCallback))
        {
            /* Wait for the match value to be reached. */
            while (!matchFlag)
            {
            }

            PRINTF("OS timer match value reached\r\n");
        }
        else
        {
            PRINTF("SetMatchInterruptTime failed: set time has already expired! \r\n");
        }
    }
}
