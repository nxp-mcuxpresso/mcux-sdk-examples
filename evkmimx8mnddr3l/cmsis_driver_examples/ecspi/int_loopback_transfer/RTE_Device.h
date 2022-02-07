/*
 * Copyright 2018,2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void ECSPI2_InitPins();
extern void ECSPI2_DeinitPins();

/* Driver name mapping. */
#define RTE_SPI2        1
#define RTE_SPI2_DMA_EN 0

/* ECSPI configuration. */
#define RTE_SPI2_PIN_INIT         ECSPI2_InitPins
#define RTE_SPI2_PIN_DEINIT       ECSPI2_DeinitPins
#define RTE_SPI2_TRANSFER_CHANNEL kECSPI_Channel0

#endif /* _RTE_DEVICE_H */
