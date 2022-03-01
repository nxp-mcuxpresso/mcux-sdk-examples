/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void UART2_InitPins();
extern void UART2_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of UARTX_GetFreq/UARTX_InitPins/UARTX_DeinitPins for the enabled UART
 * instance. */
#define RTE_USART2            1
#define RTE_USART2_PIN_INIT   UART2_InitPins
#define RTE_USART2_PIN_DEINIT UART2_DeinitPins
#define RTE_USART2_DMA_EN     0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART2_RX_BUFFER_ENABLE 1

#endif /* _RTE_DEVICE_H */
