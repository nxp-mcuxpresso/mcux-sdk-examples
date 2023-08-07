/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "board_fusionf1.h"

#define BOARD_XTAL_SYS_CLK_HZ 24000000U /*!< Board xtal_sys frequency in Hz */

void BOARD_Init()
{
    BOARD_InitPins();
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);
    BOARD_InitDebugConsole();
}
