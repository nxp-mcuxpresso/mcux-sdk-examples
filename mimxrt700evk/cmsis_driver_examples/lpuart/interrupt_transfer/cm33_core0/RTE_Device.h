/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART0_InitPins();
extern void LPUART0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART0        1
#define RTE_USART0_DMA_EN 0

/* UART configuration. */
#define RTE_USART0_PIN_INIT        LPUART0_InitPins
#define RTE_USART0_PIN_DEINIT      LPUART0_DeinitPins
#define RTE_USART0_DMA_TX_CH       0
#define RTE_USART0_DMA_TX_PERI_SEL (uint16_t) kDmaRequestMuxLpFlexcomm0Tx
#define RTE_USART0_DMA_TX_DMA_BASE DMA0
#define RTE_USART0_DMA_RX_CH       1
#define RTE_USART0_DMA_RX_PERI_SEL (uint16_t) kDmaRequestMuxLpFlexcomm0Rx
#define RTE_USART0_DMA_RX_DMA_BASE DMA0

#endif /* _RTE_DEVICE_H */
