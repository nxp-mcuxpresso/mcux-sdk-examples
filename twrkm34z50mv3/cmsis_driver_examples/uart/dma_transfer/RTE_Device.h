/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
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
#define RTE_USART1        1
#define RTE_USART1_DMA_EN 1

/* UART configuration. */
#define RTE_USART1_PIN_INIT           UART1_InitPins
#define RTE_USART1_PIN_DEINIT         UART1_DeinitPins
#define RTE_USART1_DMA_TX_CH          1U
#define RTE_USART1_DMAMUX_TX_CH       0U
#define RTE_USART1_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux2UART1Tx
#define RTE_USART1_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_USART1_DMA_TX_DMA_BASE    DMA0
#define RTE_USART1_DMA_RX_CH          2U
#define RTE_USART1_DMAMUX_RX_CH       0U
#define RTE_USART1_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux2UART1Rx
#define RTE_USART1_DMA_RX_DMAMUX_BASE DMAMUX2
#define RTE_USART1_DMA_RX_DMA_BASE    DMA0
#endif /* _RTE_DEVICE_H */
