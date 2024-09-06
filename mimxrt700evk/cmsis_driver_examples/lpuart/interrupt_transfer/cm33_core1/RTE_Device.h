/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART19_InitPins();
extern void LPUART19_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART19        1
#define RTE_USART19_DMA_EN 0

/* UART configuration. */
#define RTE_USART19_PIN_INIT        LPUART19_InitPins
#define RTE_USART19_PIN_DEINIT      LPUART19_DeinitPins
#define RTE_USART19_DMA_TX_CH       0
#define RTE_USART19_DMA_TX_PERI_SEL (uint16_t) kDmaRequestMuxLpFlexcomm19Tx
#define RTE_USART19_DMA_TX_DMA_BASE DMA2
#define RTE_USART19_DMA_RX_CH       1
#define RTE_USART19_DMA_RX_PERI_SEL (uint16_t) kDmaRequestMuxLpFlexcomm19Rx
#define RTE_USART19_DMA_RX_DMA_BASE DMA2

#endif /* _RTE_DEVICE_H */
