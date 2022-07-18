/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "board_hifi4.h"

void BOARD_Init()
{
    BOARD_InitPins();
    BOARD_InitDebugConsole();
}
