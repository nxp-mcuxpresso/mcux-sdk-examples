/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void I2C2_InitPins();
extern void I2C2_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of I2CX_GetFreq/I2CX_InitPins/I2CX_DeinitPins for the enabled I2C instance.
 */
#define RTE_I2C2            1
#define RTE_I2C2_PIN_INIT   I2C2_InitPins
#define RTE_I2C2_PIN_DEINIT I2C2_DeinitPins
#define RTE_I2C2_DMA_EN     0

/*I2C configuration*/
#define RTE_I2C2_Master_DMA_BASE DMA0
#define RTE_I2C2_Master_DMA_CH   5

#endif /* _RTE_DEVICE_H */
