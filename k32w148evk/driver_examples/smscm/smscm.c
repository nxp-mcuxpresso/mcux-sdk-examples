/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_smscm.h"
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

/*!
 * @brief Example for Security Counter.
 */
void EXAMPLE_SecurityCounter()
{
    SMSCM_SetSecurityCount(SMSCM, 0xff);
    PRINTF("The value of Security Counter Register (SCTR) is: 0x%x\r\n", SMSCM_GetSecurityCount(SMSCM));
    SMSCM_IncreaseSecurityCount(SMSCM, 0x3);
    PRINTF("Write the value x to Plus Register: x = 3.\r\n");
    if (SMSCM_GetSecurityCount(SMSCM) == 0x102)
    {
        PRINTF("The value in security counter is plused to 0x102 successfully!\r\n");
    }
    else
    {
        PRINTF("Failed to increase the security counter ...\r\n");
    }
    SMSCM_DecreaseSecurityCount(SMSCM, 0x2);
    PRINTF("Write the value x to Minus Register: x = 2.\r\n");
    if (SMSCM_GetSecurityCount(SMSCM) == 0x100)
    {
        PRINTF("The value in security counter is minused to 0x100 successfully!\r\n");
    }
    else
    {
        PRINTF("Failed to decrease the security counter ...\r\n");
    }
}

int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("SMSCM example start\r\n");

    EXAMPLE_SecurityCounter();

    PRINTF("SMSCM example finished\r\n");
    while (1)
    {
    }
}
