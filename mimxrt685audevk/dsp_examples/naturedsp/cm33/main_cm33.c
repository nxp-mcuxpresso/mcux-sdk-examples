/*
 * Copyright 2019-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_inputmux.h"
#include "dsp_config.h"
#include "dsp_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/


/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* PIMCTL clock control: Enable Clock */
    CLOCK_EnableClock(kCLOCK_InputMux);
    /* MUB interrupt signal is selected for DSP interrupt input 1 */
    INPUTMUX_AttachSignal(INPUTMUX, 1U, kINPUTMUX_MuBToDspInterrupt);

    PRINTF("\r\n[CM33 Main] NatureDSP demo start\r\n");
    PRINTF("[CM33 Main] Going to start DSP core...\r\n");

    /* Console will be used by the DSP core from now on. */
    DbgConsole_Deinit();

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    while (1)
        ;
}
