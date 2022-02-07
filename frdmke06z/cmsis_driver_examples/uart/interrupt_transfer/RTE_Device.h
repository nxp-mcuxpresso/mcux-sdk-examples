/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void UART1_InitPins();
extern void UART1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of UARTX_GetFreq/UARTX_InitPins/UARTX_DeinitPins for the enabled UART
 * instance. */
#define RTE_USART1            1
#define RTE_USART1_PIN_INIT   UART1_InitPins
#define RTE_USART1_PIN_DEINIT UART1_DeinitPins

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART1_RX_BUFFER_ENABLE 1

#endif /* _RTE_DEVICE_H */
