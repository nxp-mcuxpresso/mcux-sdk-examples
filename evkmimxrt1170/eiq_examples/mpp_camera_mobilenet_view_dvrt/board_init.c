/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board_init.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "display_support.h"
#include "camera_support.h"
#include "board.h"
#include "fsl_soc_src.h"

/*!
 * @brief Resets display controller.
 */
static void BOARD_ResetDisplayMix(void)
{
    /*
     * Reset the displaymix, otherwise during debugging, the
     * debugger may not reset the display, then the behavior
     * is not right.
     */
    SRC_AssertSliceSoftwareReset(SRC, kSRC_DisplaySlice);
    while (kSRC_SliceResetInProcess == SRC_GetSliceResetState(SRC, kSRC_DisplaySlice))
    {
    }
}

void BOARD_Init()
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitLpuartPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMipiCameraPins();
    BOARD_ResetDisplayMix();
    BOARD_InitMipiPanelPins();
}
