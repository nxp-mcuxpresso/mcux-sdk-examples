/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART4_InitPins();
extern void LPUART4_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART4        1
#define RTE_USART4_DMA_EN 0

/* UART configuration. */
#define RTE_USART4_PIN_INIT        LPUART4_InitPins
#define RTE_USART4_PIN_DEINIT      LPUART4_DeinitPins
#define RTE_USART4_DMA_TX_CH       0
#define RTE_USART4_DMA_TX_PERI_SEL (uint16_t) kDma0RequestMuxLpFlexcomm4Tx
#define RTE_USART4_DMA_TX_DMA_BASE DMA0
#define RTE_USART4_DMA_RX_CH       1
#define RTE_USART4_DMA_RX_PERI_SEL (uint16_t) kDma0RequestMuxLpFlexcomm4Rx
#define RTE_USART4_DMA_RX_DMA_BASE DMA0

#endif /* _RTE_DEVICE_H */
