/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART2_InitPins();
extern void LPUART2_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART2        1
#define RTE_USART2_DMA_EN 0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART1_RX_BUFFER_ENABLE 1

#define RTE_USART2_PIN_INIT   LPUART2_InitPins
#define RTE_USART2_PIN_DEINIT LPUART2_DeinitPins

#endif /* _RTE_DEVICE_H */
