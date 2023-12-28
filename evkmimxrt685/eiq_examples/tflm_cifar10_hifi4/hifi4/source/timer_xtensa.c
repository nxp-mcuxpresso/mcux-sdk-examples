/*
 * Copyright 2018-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "timer.h"

#include <xtensa/xos.h>

/*******************************************************************************
 * Code
 ******************************************************************************/

void TIMER_Init()
{
    xos_set_clock_freq(XOS_CLOCK_FREQ);
    xos_start_system_timer(-1, 0);
}

int TIMER_GetTimeInUS()
{
    return xos_get_system_cycles() / (xos_get_clock_freq() / 1000000);
}
