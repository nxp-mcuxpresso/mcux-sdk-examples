/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void I2C4_InitPins();
extern void I2C4_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of I2CX_GetFreq/I2CX_InitPins/I2CX_DeinitPins for the enabled I2C instance.
 */
#define RTE_I2C4            1
#define RTE_I2C4_PIN_INIT   I2C4_InitPins
#define RTE_I2C4_PIN_DEINIT I2C4_DeinitPins
#define RTE_I2C4_DMA_EN     1

/*I2C configuration*/
#define RTE_I2C4_Master_DMA_BASE DMA0
#define RTE_I2C4_Master_DMA_CH   9
#endif /* _RTE_DEVICE_H */
