/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void LPI2C1_InitPins();
extern void LPI2C1_DeinitPins();

/* Driver name mapping. */
/* User needs to provide the implementation of LPI2CX_GetFreq/LPI2CX_InitPins/LPI2CX_DeinitPins for the enabled
 * LPI2C instance. */
#define RTE_I2C1        1
#define RTE_I2C1_DMA_EN 0

/* LPI2C configuration. */

#define RTE_I2C1_PIN_INIT   LPI2C1_InitPins
#define RTE_I2C1_PIN_DEINIT LPI2C1_DeinitPins

#endif /* _RTE_DEVICE_H */
