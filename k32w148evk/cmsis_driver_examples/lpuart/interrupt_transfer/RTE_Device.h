/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART1_InitPins(void);
extern void LPUART1_DeinitPins(void);

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART1        1
#define RTE_USART1_DMA_EN 0

/* UART configuration. */
#define RTE_USART1_PIN_INIT   LPUART1_InitPins
#define RTE_USART1_PIN_DEINIT LPUART1_DeinitPins
#endif /* _RTE_DEVICE_H */
