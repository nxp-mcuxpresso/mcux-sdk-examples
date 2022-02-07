/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void UART4_InitPins();
extern void UART4_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of UARTX_GetFreq/UARTX_InitPins/UARTX_DeinitPins for the enabled UART
 * instance. */
#define RTE_USART4            1
#define RTE_USART4_PIN_INIT   UART4_InitPins
#define RTE_USART4_PIN_DEINIT UART4_DeinitPins
#define RTE_USART4_DMA_EN     0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART4_RX_BUFFER_ENABLE 1

#endif /* _RTE_DEVICE_H */
