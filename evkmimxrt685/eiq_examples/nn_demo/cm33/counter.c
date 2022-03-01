/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "counter.h"

unsigned long get_ccount(void)
{
    return KIN1_DWT_CYCCNT;
}
