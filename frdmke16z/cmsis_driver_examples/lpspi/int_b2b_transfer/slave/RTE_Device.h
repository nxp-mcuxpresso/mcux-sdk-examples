/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPSPI0_InitPins();
extern void LPSPI0_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPSPIX_GetFreq/LPSPIX_InitPins/LPSPIX_DeinitPins for the enabled LPSPI
 * instance. */
#define RTE_SPI0            1
#define RTE_SPI0_PIN_INIT   LPSPI0_InitPins
#define RTE_SPI0_PIN_DEINIT LPSPI0_DeinitPins
#define RTE_SPI0_DMA_EN     0

/* DSPI configuration. */
#define RTE_SPI0_PCS_TO_SCK_DELAY       1000
#define RTE_SPI0_SCK_TO_PSC_DELAY       1000
#define RTE_SPI0_BETWEEN_TRANSFER_DELAY 1000
#define RTE_SPI0_SLAVE_PCS_PIN_SEL      (kLPSPI_SlavePcs3)

#endif /* _RTE_DEVICE_H */
