/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void SPI1_InitPins();
extern void SPI1_DeinitPins();

/*Driver name mapping.*/
/* User needs to provide the implementation of SPIX_GetFreq/SPIX_InitPins/SPIX_DeinitPins for the enabled SPI instance.
 */
#define RTE_SPI1        1
#define RTE_SPI1_DMA_EN 1

/* SPI configuration. */
#define RTE_SPI1_PIN_INIT           SPI1_InitPins
#define RTE_SPI1_PIN_DEINIT         SPI1_DeinitPins
#define RTE_SPI1_DMA_TX_CH          3U
#define RTE_SPI1_DMAMUX_TX_CH       0U
#define RTE_SPI1_DMA_TX_PERI_SEL    (uint8_t) kDmaRequestMux2SPI1Tx
#define RTE_SPI1_DMA_TX_DMAMUX_BASE DMAMUX3
#define RTE_SPI1_DMA_TX_DMA_BASE    DMA0
#define RTE_SPI1_DMA_RX_CH          2U
#define RTE_SPI1_DMAMUX_RX_CH       0U
#define RTE_SPI1_DMA_RX_PERI_SEL    (uint8_t) kDmaRequestMux2SPI1Rx
#define RTE_SPI1_DMA_RX_DMAMUX_BASE DMAMUX2
#define RTE_SPI1_DMA_RX_DMA_BASE    DMA0

#endif /* _RTE_DEVICE_H */
