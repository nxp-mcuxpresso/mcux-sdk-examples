/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


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
    char ch;

    /* Init board hardware. */
    BOARD_InitPins();

    /* Clock was initialized in mpi_loader_extram_loader, just need to set global variables here. */
    POWER_UpdateOscSettlingTime(BOARD_SYSOSC_SETTLING_US); /* Updated XTAL oscillator settling time */
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);              /* Sets external XTAL OSC freq */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;

    BOARD_InitDebugConsole();

    PRINTF("hello world.\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
