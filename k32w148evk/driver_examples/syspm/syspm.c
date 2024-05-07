/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_syspm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SYSPM_EXAMPLE_EVENT_CODE 0x7F
uint64_t eventcounter = 0;

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
void main(void)
{
    uint8_t count = 3;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("START DEMO SYSPM\r\n");

    /* enable clock for this block*/
    SYSPM_Init(SYSPM);

    /* Reset event counter*/
    SYSPM_ResetEvent(SYSPM, kSYSPM_Monitor0, kSYSPM_Event1);

    SYSPM_SelectEvent(SYSPM, kSYSPM_Monitor0, kSYSPM_Event1, SYSPM_EXAMPLE_EVENT_CODE);
    /* Set count mode */
    SYSPM_SetCountMode(SYSPM, kSYSPM_Monitor0, kSYSPM_UserMode);
    /* Start event counter */
    SYSPM_SetStartStopControl(SYSPM, kSYSPM_Monitor0, kSYSPM_LocalStart);

    /* Get event counter */
    while (count--)
    {
        eventcounter = SYSPM_GetEventCounter(SYSPM, kSYSPM_Monitor0, kSYSPM_Event1);
        PRINTF("eventcounter = %llu\r\n", eventcounter);
    }

    /* Stop event counter */
    SYSPM_SetStartStopControl(SYSPM, kSYSPM_Monitor0, kSYSPM_LocalStop);
#if !((defined(FSL_FEATURE_SYSPM_HAS_PMCR_DCIFSH)) && (FSL_FEATURE_SYSPM_HAS_PMCR_DCIFSH == 0U))
    SYSPM_DisableCounter(SYSPM, kSYSPM_Monitor0);
#endif

    PRINTF("END DEMO SYSPM\r\n");
    {
    }

    while (1)
    {
    }
}
