/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void SPI2_InitPins();
extern void SPI2_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of SPIX_GetFreq/SPIX_InitPins/SPIX_DeinitPins for the enabled SPI instance.
 */
#define RTE_SPI2        1
#define RTE_SPI2_DMA_EN 1

/* SPI configuration. */

#define RTE_SPI2_SSEL_NUM        kSPI_Ssel0
#define RTE_SPI2_PIN_INIT        SPI2_InitPins
#define RTE_SPI2_PIN_DEINIT      SPI2_DeinitPins
#define RTE_SPI2_DMA_TX_CH       11
#define RTE_SPI2_DMA_TX_DMA_BASE DMA0
#define RTE_SPI2_DMA_RX_CH       10
#define RTE_SPI2_DMA_RX_DMA_BASE DMA0
#endif /* _RTE_DEVICE_H */
