/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void ECSPI1_InitPins();
extern void ECSPI1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of ECSPIX_GetFreq/ECSPIX_InitPins/ECSPIX_DeinitPins for the enabled ECSPI
 * instance. */
#define RTE_SPI1            1
#define RTE_SPI1_PIN_INIT   ECSPI1_InitPins
#define RTE_SPI1_PIN_DEINIT ECSPI1_DeinitPins
#define RTE_SPI1_DMA_EN     0

/* ECSPI configuration. */
#define RTE_SPI1_TRANSFER_CHANNEL kECSPI_Channel0

#endif /* _RTE_DEVICE_H */
