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
#include "pin_mux.h"
//#include "clock_config.h"
//#include "board.h"
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
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ);
    BOARD_InitDebugConsole();
    TIMER_Init();

    PRINTF("\r\n*************************************************");
    PRINTF("\r\n               TFLite Modelrunner UART");
    PRINTF("\r\n*************************************************\r\n");

    modelrunner();

}
