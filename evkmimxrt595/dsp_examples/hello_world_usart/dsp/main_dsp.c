/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <xtensa/config/core.h>

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board_fusionf1.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_XTAL_SYS_CLK_HZ 24000000U /*!< Board xtal_sys frequency in Hz */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);
    BOARD_InitDebugConsole();

    PRINTF("\r\nHello World running on DSP core '%s'\r\n", XCHAL_CORE_ID);

    while (1)
    {
    }
}
