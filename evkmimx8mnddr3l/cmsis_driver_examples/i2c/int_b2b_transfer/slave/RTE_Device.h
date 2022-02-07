/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _RTE_DEVICE_H
#define _RTE_DEVICE_H

extern void I2C3_InitPins();
extern void I2C3_DeinitPins();

/*Driver name mapping*/
/* User needs to provide the implementation of I2CX_GetFreq/I2CX_InitPins/I2CX_DeinitPins for the enabled I2C instance.
 */
#define RTE_I2C3            1
#define RTE_I2C3_PIN_INIT   I2C3_InitPins
#define RTE_I2C3_PIN_DEINIT I2C3_DeinitPins

/*I2C configuration*/

#endif /* _RTE_DEVICE_H */
