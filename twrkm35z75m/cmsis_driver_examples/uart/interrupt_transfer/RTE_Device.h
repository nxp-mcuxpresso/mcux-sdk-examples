/*
 * Copyright 2019 NXP
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
#define RTE_USART2        1
#define RTE_USART2_DMA_EN 0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART2_RX_BUFFER_ENABLE 1

#define RTE_USART2_PIN_INIT           UART2_InitPins
#define RTE_USART2_PIN_DEINIT         UART2_DeinitPins
#define RTE_USART2_DMA_TX_CH          0
#define RTE_USART2_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART2Tx
#define RTE_USART2_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_TX_DMA_BASE    DMA0
#define RTE_USART2_DMA_RX_CH          1
#define RTE_USART2_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART2Rx
#define RTE_USART2_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
