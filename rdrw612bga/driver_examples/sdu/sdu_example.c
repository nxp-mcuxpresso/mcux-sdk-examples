/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_os_abstraction.h"
#include "fsl_debug_console.h"
#include "fsl_adapter_sdu.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/



/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_sdu_main_continue = 0;


/*******************************************************************************
 * Private Functions
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    status_t ret = 0;

    //while (!g_sdu_main_continue);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);

    OSA_Init();

    ret = SDU_Init();
    if (ret != kStatus_Success)
    {
        PRINTF("Failed to initialize SDIO");
        return 0;
    }

    PRINTF("OSA_Start.\r\n");
    OSA_Start();

    return 0;
}
