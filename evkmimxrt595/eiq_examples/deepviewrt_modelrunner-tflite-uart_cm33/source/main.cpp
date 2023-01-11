/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/


#include "modelrunner.h"
#include "timer.h"

//#include "pin_mux.h"
//#include "clock_config.h"
//#include "board.h"
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
 * Code
 ******************************************************************************/
int64_t os_clock_now()
{
    return TIMER_GetTimeInUS();
}

/*!
 * @brief Main function.
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    TIMER_Init();

    PRINTF("\r\n*************************************************");
    PRINTF("\r\n               TFLite Modelrunner UART");
    PRINTF("\r\n*************************************************\r\n");

    modelrunner();

}
