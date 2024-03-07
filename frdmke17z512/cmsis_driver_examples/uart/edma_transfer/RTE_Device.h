/*
 * Copyright 2023 NXP
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
/* LPUART0-2, UART0-1. */
#define RTE_USART4        1
#define RTE_USART4_DMA_EN 1

/* UART configuration. */
#define RTE_USART4_PIN_INIT           UART1_InitPins
#define RTE_USART4_PIN_DEINIT         UART1_DeinitPins
#define RTE_USART4_DMA_TX_CH          0
#define RTE_USART4_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART1Tx
#define RTE_USART4_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART4_DMA_TX_DMA_BASE    DMA0
#define RTE_USART4_DMA_RX_CH          1
#define RTE_USART4_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART1Rx
#define RTE_USART4_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART4_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
