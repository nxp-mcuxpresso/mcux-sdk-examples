/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
 
#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void SPI0_InitPins();
extern void SPI0_DeinitPins();

/*Driver name mapping.*/
/* User needs to provide the implementation of SPIX_GetFreq/SPIX_InitPins/SPIX_DeinitPins for the enabled SPI instance.
 */
#define RTE_SPI0        1
#define RTE_SPI0_DMA_EN 1

/* SPI configuration. */
#define RTE_SPI0_PIN_INIT           SPI0_InitPins
#define RTE_SPI0_PIN_DEINIT         SPI0_DeinitPins
#define RTE_SPI0_DMA_TX_CH          0
#define RTE_SPI0_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux0SPI0Tx
#define RTE_SPI0_DMA_TX_DMAMUX_BASE DMAMUX0
#define RTE_SPI0_DMA_TX_DMA_BASE    DMA0
#define RTE_SPI0_DMA_RX_CH          1
#define RTE_SPI0_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux0SPI0Rx
#define RTE_SPI0_DMA_RX_DMAMUX_BASE DMAMUX0
#define RTE_SPI0_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
