/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef LOW_POWER_H_
#define LOW_POWER_H_

#include "bmm050.h"
#include "bma2x2.h"
#include "bmi160.h"
#include "bmp280.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define QUEUE_ELEMENT_COUNT_TRIG (50)

typedef struct
{
    int32_t temp;
    uint32_t press;

    struct bmm050_mag_s32_data_t mag;
    struct bma2x2_accel_data_temp accel;
    struct bmi160_gyro_t gyro;

} sensor_data_t;

typedef enum
{
    kGoToDeepSleep = 1,
    kProcessData,
    kTurnOffFlash
} core_cmd_t;

#endif /* LOW_POWER_H_ */
