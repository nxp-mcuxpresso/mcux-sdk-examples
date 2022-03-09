/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __I2S_IF2_H__
#define __I2S_IF2_H__

void I2S_Start(void);
void I2S_Stop(void);
void I2S_Init(void);
void I2S_SetSampleRate(uint32_t rate);

bool I2S_SampleChk(void);
int32_t I2S_SampleDiff(void);

#endif
