/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "board_fusionf1.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Initialize debug console. */
void BOARD_InitDebugConsole(void)
{
    uint32_t uartClkSrcFreq;

    CLOCK_SetIpSrc(BOARD_DEBUG_UART_IP_NAME, BOARD_DEBUG_UART_CLKSRC);
    uartClkSrcFreq = BOARD_DEBUG_UART_CLK_FREQ;
    RESET_PeripheralReset(BOARD_DEBUG_UART_RESET);

    DbgConsole_Init(BOARD_DEBUG_UART_INSTANCE, BOARD_DEBUG_UART_BAUDRATE, BOARD_DEBUG_UART_TYPE, uartClkSrcFreq);
}
