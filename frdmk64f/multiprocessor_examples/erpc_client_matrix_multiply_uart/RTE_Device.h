/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __RTE_DEVICE_H
#define __RTE_DEVICE_H

/* Driver name mapping. */
#define RTE_USART0        1
#define RTE_USART0_DMA_EN 0
#define RTE_USART1        0
#define RTE_USART1_DMA_EN 0
#define RTE_USART2        0
#define RTE_USART2_DMA_EN 0
#define RTE_USART3        0
#define RTE_USART3_DMA_EN 0
#define RTE_USART4        0
#define RTE_USART4_DMA_EN 0

/* UART configuration. */
#define RTE_USART0_DMA_TX_CH          0
#define RTE_USART0_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART0Tx
#define RTE_USART0_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART0_DMA_TX_DMA_BASE    DMA0
#define RTE_USART0_DMA_RX_CH          1
#define RTE_USART0_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART0Rx
#define RTE_USART0_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART0_DMA_RX_DMA_BASE    DMA0

#define RTE_USART1_DMA_TX_CH          0
#define RTE_USART1_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART1Tx
#define RTE_USART1_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART1_DMA_TX_DMA_BASE    DMA0
#define RTE_USART1_DMA_RX_CH          1
#define RTE_USART1_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART1Rx
#define RTE_USART1_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART1_DMA_RX_DMA_BASE    DMA0

#define RTE_USART2_DMA_TX_CH          0
#define RTE_USART2_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART2Tx
#define RTE_USART2_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART2_DMA_TX_DMA_BASE    DMA0
#define RTE_USART2_DMA_RX_CH          1
#define RTE_USART2_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART2Rx
#define RTE_USART2_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART2_DMA_RX_DMA_BASE    DMA0

#define RTE_USART3_DMA_TX_CH          0
#define RTE_USART3_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART3Tx
#define RTE_USART3_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART3_DMA_TX_DMA_BASE    DMA0
#define RTE_USART3_DMA_RX_CH          1
#define RTE_USART3_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART3Rx
#define RTE_USART3_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART3_DMA_RX_DMA_BASE    DMA0

#define RTE_USART4_DMA_TX_CH          0
#define RTE_USART4_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART4
#define RTE_USART4_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART4_DMA_TX_DMA_BASE    DMA0
#define RTE_USART4_DMA_RX_CH          1
#define RTE_USART4_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART4
#define RTE_USART4_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART4_DMA_RX_DMA_BASE    DMA0

#define RTE_USART5_DMA_TX_CH          0
#define RTE_USART5_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0UART5
#define RTE_USART5_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART5_DMA_TX_DMA_BASE    DMA0
#define RTE_USART5_DMA_RX_CH          1
#define RTE_USART5_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0UART5
#define RTE_USART5_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART5_DMA_RX_DMA_BASE    DMA0

#endif /* __RTE_DEVICE_H */
