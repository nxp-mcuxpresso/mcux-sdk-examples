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

#include "fsl_mpi_loader.h"
#include "fsl_dsp.h"
#include "fsl_power.h"
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

/* Enable multicore packed image loader for DSP context copy. */
void SystemInitHook(void)
{
    /* Configure FFRO clock */
    if ((SYSCTL0->PDRUNCFG0 & SYSCTL0_PDRUNCFG0_FFRO_PD_MASK) != 0)
    {
        POWER_DisablePD(kPDRUNCFG_PD_FFRO);  /* Power on FFRO (48/60MHz) */
        CLOCK_EnableFfroClk(kCLOCK_Ffro48M); /* Enable FFRO clock*/
    }
    /* DSP RAM need to be powered up and applied clock */
    CLOCK_AttachClk(kFFRO_to_DSP_MAIN_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivDspCpuClk, 1);
    CLOCK_SetClkDiv(kCLOCK_DivDspRamClk, 1);
    DSP_Init();
    MPI_LoadMultiImages();
}


/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print the initial banner */
    PRINTF("\r\nHello World running on core 'Cortex-M33'\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    while (1)
        ;
}
