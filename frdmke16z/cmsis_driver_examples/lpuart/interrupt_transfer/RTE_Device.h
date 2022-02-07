/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART0_InitPins();
extern void LPUART0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART0            1
#define RTE_USART0_PIN_INIT   LPUART0_InitPins
#define RTE_USART0_PIN_DEINIT LPUART0_DeinitPins
#define RTE_USART0_DMA_EN     0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART0_RX_BUFFER_ENABLE 1

#endif /* _RTE_DEVICE_H */
