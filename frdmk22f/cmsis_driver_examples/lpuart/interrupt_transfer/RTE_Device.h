/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART0_InitPins();
extern void LPUART0_DeinitPins();

/* USART Select. */
/* Use UART0 - UART2. */
/*Use LPUART0. */
/* User needs to provide the implementation of UARTX_GetFreq/UARTX_InitPins/UARTX_DeinitPins for the enabled UART
 * instance. */
#define RTE_USART3        1
#define RTE_USART3_DMA_EN 0

/* UART RX Buffer configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART0_RX_BUFFER_ENABLE 0
#define USART1_RX_BUFFER_ENABLE 0
#define USART2_RX_BUFFER_ENABLE 0
#define USART3_RX_BUFFER_ENABLE 1

/* UART configuration. */

#define RTE_USART3_PIN_INIT           LPUART0_InitPins
#define RTE_USART3_PIN_DEINIT         LPUART0_DeinitPins
#define RTE_USART3_DMA_TX_CH          0
#define RTE_USART3_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0LPUART0Tx
#define RTE_USART3_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART3_DMA_TX_DMA_BASE    DMA0
#define RTE_USART3_DMA_RX_CH          1
#define RTE_USART3_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0LPUART0Rx
#define RTE_USART3_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART3_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
