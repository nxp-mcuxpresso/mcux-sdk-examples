/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_gdet.h"

#include <string.h>

#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void GDET_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(GDET_IRQn);

    PRINTF("GDET IRQ Reached \r\n");

    NVIC_EnableIRQ(GDET_IRQn);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t status = kStatus_Fail;

    /* Init hardware */
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("GDET Peripheral Driver Example\r\n\r\n");

    NVIC_EnableIRQ(GDET_IRQn);

    GDET_Init(GDET0);

    /* 1) Set GDET to isolate */

    status = GDET_IsolateOn(GDET0);
    status = GDET_IsolateOn(GDET1);

    /*  2) Increase SoC voltage */

    /* Call code to change voltage core */

    /*  3) and 4) Call GDET change voltage*/

    status = GDET_ReconfigureVoltageMode(GDET0, kGDET_OverDriveVoltage);
    status = GDET_ReconfigureVoltageMode(GDET1, kGDET_OverDriveVoltage);

    /* 5) Turn off GDET isolate */

    status = GDET_IsolateOff(GDET0);
    status = GDET_IsolateOff(GDET1);

    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation .\r\n");
    }

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
