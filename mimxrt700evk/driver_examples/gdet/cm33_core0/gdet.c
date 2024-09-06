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
#include "board.h"

#include "fsl_gdet.h"

#include <string.h>

#include "fsl_power.h"
#include "fsl_clock.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_GDET GDET0

#define APP_GLIKEY GLIKEY3

#define GDET_APP_IRQ GDET0_IRQn
#define APP_GDET_DriverIRQHandler GDET0_DriverIRQHandler

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_GDET_DriverIRQHandler(void)
{
    NVIC_DisableIRQ(GDET_APP_IRQ);

    PRINTF("GDET IRQ Reached \r\n");

    NVIC_EnableIRQ(GDET_APP_IRQ);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    status_t status = kStatus_Fail;

    /* Init hardware */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Move Glikey FSM to write enable */
    GlikeyWriteEnable(APP_GLIKEY, 0u);


    PRINTF("GDET Peripheral Driver Example\r\n\r\n");

    PRINTF("GDET Init\r\n");
    status = GDET_Init(APP_GDET);
    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation!\r\n");
    }

    /* Loading the configuration into the GDET SFR should happen before enabling */

    PRINTF("GDET Enable\r\n");
    status = GDET_Enable(APP_GDET);
    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation!\r\n");
    }

    /* 1) Set GDET to isolate */
    PRINTF("GDET Isolate\r\n");
    status = GDET_IsolateOn(APP_GDET);
    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation!\r\n");
    }

    /*  2) Change SoC voltage */

    /* Call code to change voltage core */
    PRINTF("* Now VDD voltage can be adjusted *\r\n");

    /*  3) and 4) Call GDET change voltage*/
    PRINTF("GDET Change voltage mode\r\n");
    status = GDET_ReconfigureVoltageMode(APP_GDET, kGDET_1_0v);
    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation!\r\n");
    }

    /* 5) Turn off GDET isolate */
    PRINTF("GDET Isolation off\r\n\r\n");
    status = GDET_IsolateOff(APP_GDET);
    if (status != kStatus_Success)
    {
        PRINTF("Error while GDET operation!\r\n");
    }

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
