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
#include "board.h"

#include <stdbool.h>
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
    uint32_t uartClkSrcFreq;

    /* SIM_SOPT2[27:26]:
     *  00: Clock Disabled
     *  01: MCGFLLCLK, or MCGPLLCLK, or IRC48M
     *  10: OSCERCLK
     *  11: MCGIRCCLK
     */
    CLOCK_SetLpuartClock(1);

    uartClkSrcFreq = CLOCK_GetFreq(kCLOCK_PllFllSelClk);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);

    PRINTF("hello world.\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
