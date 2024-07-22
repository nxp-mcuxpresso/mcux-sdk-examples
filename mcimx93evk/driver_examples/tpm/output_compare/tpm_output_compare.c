/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_tpm.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* define instance */
#define DEMO_TPM_BASEADDR     TPM2
#define BOARD_TPM_OUT_CHANNEL kTPM_Chnl_0

/* Get source clock for TPM driver */
#define LPTPM_CLOCK_ROOT kCLOCK_Root_Tpm2
#define LPTPM_CLOCK_GATE kCLOCK_Tpm2
#define TPM_SOURCE_CLOCK CLOCK_GetIpFreq(LPTPM_CLOCK_ROOT)
#ifndef DEMO_TIMER_PERIOD_MS
/* To make led toggling visible, set default counter period to 40ms, which will make output frequency (two counter
 * period) be 12.5Hz */
#define DEMO_TIMER_PERIOD_MS (40U)
#endif
#ifndef TPM_PRESCALER_VALUE
/* Calculate the clock division based on the PWM frequency to be obtained */
#define TPM_PRESCALER_VALUE \
    TPM_CalculateCounterClkDiv(DEMO_TPM_BASEADDR, 1000U / DEMO_TIMER_PERIOD_MS, TPM_SOURCE_CLOCK);
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    tpm_config_t tpmInfo;

    /* Board pin, clock, debug console init */
    /* clang-format off */

    const clock_root_config_t lptpmClkCfg = {
        .clockOff = false,
	.mux = 0,
	.div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(LPTPM_CLOCK_ROOT, &lptpmClkCfg);
    CLOCK_EnableClock(LPTPM_CLOCK_GATE);

    /* Print a note to terminal */
    PRINTF("\r\nTPM example for output compare\r\n");
    PRINTF("\r\nYou will see the output signal toggle");
    PRINTF("\r\nProbe the signal using an oscilloscope");

    TPM_GetDefaultConfig(&tpmInfo);

    tpmInfo.prescale = TPM_PRESCALER_VALUE;

    /* Initialize TPM module */
    TPM_Init(DEMO_TPM_BASEADDR, &tpmInfo);

    /* Setup the output compare mode to toggle output on a match, note the compare value must less than counter mod
     * value */
    TPM_SetupOutputCompare(DEMO_TPM_BASEADDR, BOARD_TPM_OUT_CHANNEL, kTPM_ToggleOnMatch,
                           MSEC_TO_COUNT(DEMO_TIMER_PERIOD_MS, TPM_SOURCE_CLOCK / (1U << tpmInfo.prescale)) / 2U);

    /* Set the timer period */
    TPM_SetTimerPeriod(DEMO_TPM_BASEADDR,
                       MSEC_TO_COUNT(DEMO_TIMER_PERIOD_MS, TPM_SOURCE_CLOCK / (1U << tpmInfo.prescale)));

    TPM_StartTimer(DEMO_TPM_BASEADDR, kTPM_SystemClock);
    while (1)
    {
    }
}
