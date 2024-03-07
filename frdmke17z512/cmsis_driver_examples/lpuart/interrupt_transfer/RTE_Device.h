/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART2_InitPins(void);
extern void LPUART2_DeinitPins(void);

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART2        1
#define RTE_USART2_DMA_EN 0

/* UART configuration. */
#define USART_RX_BUFFER_LEN     64
#define USART2_RX_BUFFER_ENABLE 1

#define RTE_USART2_PIN_INIT           LPUART2_InitPins
#define RTE_USART2_PIN_DEINIT         LPUART2_DeinitPins
#define RTE_USART2_DMA_TX_CH          0
#define RTE_USART2_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0LPUART2Tx
#define RTE_USART2_DMA_TX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_TX_DMA_BASE    DMA0
#define RTE_USART2_DMA_RX_CH          1
#define RTE_USART2_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0LPUART2Rx
#define RTE_USART2_DMA_RX_DMAMUX_BASE DMAMUX
#define RTE_USART2_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
