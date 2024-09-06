/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPSPI14_InitPins();
extern void LPSPI14_DeinitPins();

/*Driver name mapping.*/
/* User needs to provide the implementation of LPSPIX_GetFreq/LPSPIX_InitPins/LPSPIX_DeinitPins for the enabled LPSPI
 * instance. */
#define RTE_SPI14        1
#define RTE_SPI14_DMA_EN 0

/* SPI configuration. */
#define RTE_SPI14_PIN_INIT               LPSPI14_InitPins
#define RTE_SPI14_PIN_DEINIT             LPSPI14_DeinitPins
#define RTE_SPI14_DMA_TX_CH              0
#define RTE_SPI14_DMA_TX_PERI_SEL        kDmaRequestMuxLpspi14Tx
#define RTE_SPI14_DMA_TX_DMA_BASE        DMA0
#define RTE_SPI14_DMA_RX_CH              1
#define RTE_SPI14_DMA_RX_PERI_SEL        kDmaRequestMuxLpspi14Rx
#define RTE_SPI14_DMA_RX_DMA_BASE        DMA0

#endif /* _RTE_DEVICE_H */
