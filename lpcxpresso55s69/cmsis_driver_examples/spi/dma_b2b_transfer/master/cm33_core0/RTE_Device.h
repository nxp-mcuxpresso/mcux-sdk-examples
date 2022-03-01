/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void SPI7_InitPins();
extern void SPI7_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of SPIX_GetFreq/SPIX_InitPins/SPIX_DeinitPins for the enabled SPI instance.
 */
#define RTE_SPI7        1
#define RTE_SPI7_DMA_EN 1

/* SPI configuration. */

#define RTE_SPI7_SSEL_NUM        kSPI_Ssel1
#define RTE_SPI7_PIN_INIT        SPI7_InitPins
#define RTE_SPI7_PIN_DEINIT      SPI7_DeinitPins
#define RTE_SPI7_DMA_TX_CH       19
#define RTE_SPI7_DMA_TX_DMA_BASE DMA0
#define RTE_SPI7_DMA_RX_CH       18
#define RTE_SPI7_DMA_RX_DMA_BASE DMA0
#endif /* _RTE_DEVICE_H */
