/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART2_InitPins();
extern void LPUART2_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART2        1
#define RTE_USART2_DMA_EN 1

/* UART configuration. */

#define RTE_USART2_PIN_INIT        LPUART2_InitPins
#define RTE_USART2_PIN_DEINIT      LPUART2_DeinitPins
#define RTE_USART2_DMA_RX_CH       (uint8_t) kDma3RequestMuxLPUART2Rx
#define RTE_USART2_DMA_RX_PERI_SEL (uint8_t) kDma3RequestMuxLPUART2Rx
#define RTE_USART2_DMA_RX_DMA_BASE DMA3
#define RTE_USART2_DMA_TX_CH       (uint8_t) kDma3RequestMuxLPUART2Tx
#define RTE_USART2_DMA_TX_PERI_SEL (uint8_t) kDma3RequestMuxLPUART2Tx
#define RTE_USART2_DMA_TX_DMA_BASE DMA3

#endif /* _RTE_DEVICE_H */
