/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART1_InitPins();
extern void LPUART1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART1         1
#define RTE_USART1_DMA_EN  1

/* UART configuration. */
#define RTE_USART1_PIN_INIT        LPUART1_InitPins
#define RTE_USART1_PIN_DEINIT      LPUART1_DeinitPins
#define RTE_USART1_DMA_TX_CH       0
#define RTE_USART1_DMA_TX_PERI_SEL kDma3RequestMuxLPUART1Tx
#define RTE_USART1_DMA_TX_DMA_BASE DMA3
#define RTE_USART1_DMA_RX_CH       1
#define RTE_USART1_DMA_RX_PERI_SEL kDma3RequestMuxLPUART1Rx
#define RTE_USART1_DMA_RX_DMA_BASE DMA3

#endif /* _RTE_DEVICE_H */
