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

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief XIP function.
 * Define the function which runs in external memory(XIP).
 * The XIP code can be put into ".xip_section" section.
 */
__attribute__((section(".xip_section"))) void Demo_FuncXip(void)
{
    /* Do something. Run the code in external flash(XIP). */
    PRINTF("Run in flash.\r\n");
}
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
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("hello world.\r\n");
    Demo_FuncXip();

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
