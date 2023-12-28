/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _BOARD_FUSIONF1_H_
#define _BOARD_FUSIONF1_H_

#include "fsl_common.h"
#include "fsl_reset.h"
#include "fsl_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief The board name */
#define BOARD_NAME "MIMX8ULP-EVK"

/*! @brief The UART to use for debug messages. */
#define BOARD_DEBUG_UART_TYPE     kSerialPort_Uart
#define BOARD_DEBUG_UART_BASEADDR (uint32_t) LPUART1_BASE
#define BOARD_DEBUG_UART_INSTANCE 1U
#define BOARD_DEBUG_UART_CLK_FREQ CLOCK_GetLpuartClkFreq(BOARD_DEBUG_UART_INSTANCE)
#define BOARD_DEBUG_UART_IP_NAME  kCLOCK_Lpuart1
#define BOARD_DEBUG_UART_CLKSRC   kCLOCK_Pcc1BusIpSrcCm33Bus
#define BOARD_DEBUG_UART_RESET    kRESET_Lpuart1
#define BOARD_UART_IRQ            LPUART1_IRQn

#ifndef BOARD_DEBUG_UART_BAUDRATE
#define BOARD_DEBUG_UART_BAUDRATE 115200
#endif /* BOARD_DEBUG_UART_BAUDRATE */

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*******************************************************************************
 * API
 ******************************************************************************/

void BOARD_InitDebugConsole(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _BOARD_FUSIONF1_H_ */
