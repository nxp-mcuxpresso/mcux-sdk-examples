/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "fsl_debug_console.h"
#include "board_fusionf1.h"

#define BOARD_XTAL_SYS_CLK_HZ 24000000U /*!< Board xtal_sys frequency in Hz */

void BOARD_Init()
{
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);
    BOARD_InitDebugConsole();
}
