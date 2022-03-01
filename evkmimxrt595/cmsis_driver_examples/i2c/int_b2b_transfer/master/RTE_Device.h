/*
 * Copyright 2019 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void I2C11_InitPins();
extern void I2C11_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of I2CX_GetFreq/I2CX_InitPins/I2CX_DeinitPins for the enabled I2C instance.
 */
#define RTE_I2C11            1
#define RTE_I2C11_PIN_INIT   I2C11_InitPins
#define RTE_I2C11_PIN_DEINIT I2C11_DeinitPins
#define RTE_I2C11_DMA_EN     0

/*I2C configuration*/
#define RTE_I2C11_Master_DMA_BASE DMA0
#define RTE_I2C11_Master_DMA_CH   33
#endif /* _RTE_DEVICE_H */
