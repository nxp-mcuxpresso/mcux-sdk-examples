/*
 * Copyright 2019 NXP
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
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF(
        "The Hardfault Handling demo is used to demonstrate the functionality of the exception handling component. "
        "\r\n");
    PRINTF(
        "This main function is used to generate a hardfault event. The program enters hardfault when you input the "
        "character \"1\"."
        " Then you can see the \"Stack frame\" information in the terminal. \r\n");

    while (1)
    {
        PRINTF("Please input the character \"1\" to enter hardfault case. \r\n");
        ch = GETCHAR();
        PUTCHAR(ch);
        if (ch == '1')
        {
            *(uint32_t *)(0x3fffffff - 0x4) = 0x123;
        }
        else
        {
            PRINTF("\r\nInvalid character entered.\r\n");
        }
    }
}
