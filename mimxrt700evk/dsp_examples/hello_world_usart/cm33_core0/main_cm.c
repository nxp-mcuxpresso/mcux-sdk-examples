/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "core1_support.h"
#include "dsp_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_BootCore1(void)
{
    BOARD_ReleaseCore1Power();
    BOARD_BootCore1(CORE1_BOOT_ADDRESS, CORE1_BOOT_ADDRESS);
}


/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    BOARD_InitAHBSC();

#ifdef CORE1_IMAGE_COPY_TO_RAM
    BOARD_CopyCore1Image(CORE1_BOOT_ADDRESS);
#endif

    APP_BootCore1();

    /* Print the initial banner */
    PRINTF("\r\nHello World running on core 'Cortex-M33'\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    while (1)
        ;
}
