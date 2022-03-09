/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void USART0_InitPins();
extern void USART0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of USARTX_GetFreq/USARTX_InitPins/USARTX_DeinitPins for the enabled USART
 * instance. */
#define RTE_USART0        1
#define RTE_USART0_DMA_EN 1

/* USART configuration. */
#define RTE_USART0_PIN_INIT        USART0_InitPins
#define RTE_USART0_PIN_DEINIT      USART0_DeinitPins
#define RTE_USART0_DMA_TX_CH       1
#define RTE_USART0_DMA_TX_DMA_BASE DMA0
#define RTE_USART0_DMA_RX_CH       0
#define RTE_USART0_DMA_RX_DMA_BASE DMA0

#endif /* _RTE_DEVICE_H */
