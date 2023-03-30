/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPUART1_InitPins();
extern void LPUART1_DeinitPins();
extern void LPUART10_InitPins();
extern void LPUART10_DeinitPins();
extern void LPUART11_InitPins();
extern void LPUART11_DeinitPins();
extern void LPUART12_InitPins();
extern void LPUART12_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPUARTX_GetFreq/LPUARTX_InitPins/LPUARTX_DeinitPins for the enabled
 * LPUART instance. */
#define RTE_USART1         1
#define RTE_USART1_DMA_EN  1
#define RTE_USART10        0
#define RTE_USART10_DMA_EN 0
#define RTE_USART11        0
#define RTE_USART11_DMA_EN 0
#define RTE_USART12        0
#define RTE_USART12_DMA_EN 0

/* UART configuration. */
#define RTE_USART1_PIN_INIT        LPUART1_InitPins
#define RTE_USART1_PIN_DEINIT      LPUART1_DeinitPins
#define RTE_USART1_DMA_TX_CH       0
#define RTE_USART1_DMA_TX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART1Tx
#if __CORTEX_M == 7
#define RTE_USART1_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART1_DMA_TX_DMA_BASE    DMA0
#else
#define RTE_USART1_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_USART1_DMA_TX_DMA_BASE    DMA1
#endif
#define RTE_USART1_DMA_RX_CH       1
#define RTE_USART1_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART1Rx
#if __CORTEX_M == 7
#define RTE_USART1_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART1_DMA_RX_DMA_BASE    DMA0
#else
#define RTE_USART1_DMA_RX_DMAMUX_BASE DMAMUX1
#define RTE_USART1_DMA_RX_DMA_BASE    DMA1
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#if __CORTEX_M == 7
#else
#endif
#if __CORTEX_M == 7
#else
#endif

#define RTE_USART10_PIN_INIT        LPUART10_InitPins
#define RTE_USART10_PIN_DEINIT      LPUART10_DeinitPins
#define RTE_USART10_DMA_TX_CH       18
#define RTE_USART10_DMA_TX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART10Tx
#if __CORTEX_M == 7
#define RTE_USART10_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART10_DMA_TX_DMA_BASE    DMA0
#else
#define RTE_USART10_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_USART10_DMA_TX_DMA_BASE    DMA1
#endif
#define RTE_USART10_DMA_RX_CH       19
#define RTE_USART10_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART10Rx
#if __CORTEX_M == 7
#define RTE_USART10_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART10_DMA_RX_DMA_BASE    DMA0
#else
#define RTE_USART10_DMA_RX_DMAMUX_BASE DMAMUX1
#define RTE_USART10_DMA_RX_DMA_BASE    DMA1
#endif

#define RTE_USART11_PIN_INIT        LPUART11_InitPins
#define RTE_USART11_PIN_DEINIT      LPUART11_DeinitPins
#define RTE_USART11_DMA_TX_CH       20
#define RTE_USART11_DMA_TX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART11Tx
#if __CORTEX_M == 7
#define RTE_USART11_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART11_DMA_TX_DMA_BASE    DMA0
#else
#define RTE_USART11_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_USART11_DMA_TX_DMA_BASE    DMA1
#endif
#define RTE_USART11_DMA_RX_CH       21
#define RTE_USART11_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART11Rx
#if __CORTEX_M == 7
#define RTE_USART11_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART11_DMA_RX_DMA_BASE    DMA0
#else
#define RTE_USART11_DMA_RX_DMAMUX_BASE DMAMUX1
#define RTE_USART11_DMA_RX_DMA_BASE    DMA1
#endif

#define RTE_USART12_PIN_INIT        LPUART12_InitPins
#define RTE_USART12_PIN_DEINIT      LPUART12_DeinitPins
#define RTE_USART12_DMA_TX_CH       22
#define RTE_USART12_DMA_TX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART12Tx
#if __CORTEX_M == 7
#define RTE_USART12_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_USART12_DMA_TX_DMA_BASE    DMA0
#else
#define RTE_USART12_DMA_TX_DMAMUX_BASE DMAMUX1
#define RTE_USART12_DMA_TX_DMA_BASE    DMA1
#endif
#define RTE_USART12_DMA_RX_CH       23
#define RTE_USART12_DMA_RX_PERI_SEL (uint8_t) kDmaRequestMuxLPUART12Rx
#if __CORTEX_M == 7
#define RTE_USART12_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_USART12_DMA_RX_DMA_BASE    DMA0
#else
#define RTE_USART12_DMA_RX_DMAMUX_BASE DMAMUX1
#define RTE_USART12_DMA_RX_DMA_BASE    DMA1
#endif

#endif /* _RTE_DEVICE_H */
