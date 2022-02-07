/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void DSPI0_InitPins();
extern void DSPI0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of DSPIX_GetFreq/DSPIX_InitPins/DSPIX_DeinitPins for the enabled DSPI
 * instance. */
#define RTE_SPI0        1
#define RTE_SPI0_DMA_EN 1

/* DSPI configuration. */
#define RTE_SPI0_PCS_TO_SCK_DELAY       1000
#define RTE_SPI0_SCK_TO_PSC_DELAY       1000
#define RTE_SPI0_BETWEEN_TRANSFER_DELAY 1000
#define RTE_SPI0_MASTER_PCS_PIN_SEL     kDSPI_MasterPcs0
#define RTE_SPI0_PIN_INIT               DSPI0_InitPins
#define RTE_SPI0_PIN_DEINIT             DSPI0_DeinitPins
#define RTE_SPI0_DMA_TX_CH              0
#define RTE_SPI0_DMA_TX_PERI_SEL        (uint8_t) kDmaRequestMux0SPI0Tx
#define RTE_SPI0_DMA_TX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI0_DMA_TX_DMA_BASE        DMA0
#define RTE_SPI0_DMA_RX_CH              1
#define RTE_SPI0_DMA_RX_PERI_SEL        (uint8_t) kDmaRequestMux0SPI0Rx
#define RTE_SPI0_DMA_RX_DMAMUX_BASE     DMAMUX0
#define RTE_SPI0_DMA_RX_DMA_BASE        DMA0
#define RTE_SPI0_DMA_LINK_DMA_BASE      DMA0
#define RTE_SPI0_DMA_LINK_CH            2

#endif /* _RTE_DEVICE_H */
