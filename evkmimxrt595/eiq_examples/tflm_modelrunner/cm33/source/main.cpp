/*
 * Copyright 2021-2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/


#include "modelrunner.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "timer.h"
#include "app.h"
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
int64_t os_clock_now(){
    return TIMER_GetTimeInUS();
}


#ifdef __cplusplus
extern "C" {
#endif

int64_t getTime() { return os_clock_now(); }

int gethostname(char *name)
{
    const char* host = "evkmimxrt595";
    memcpy(name, host, strlen(host));
    return 0;
}

#ifdef __cplusplus
}
#endif

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
    PRINTF("\r\n               TFLite Modelrunner");
    PRINTF("\r\n*************************************************\r\n");

    modelrunner();

}
