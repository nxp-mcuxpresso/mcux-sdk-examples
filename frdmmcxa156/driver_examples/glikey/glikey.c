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

#include "fsl_glikey.h"

#include <string.h>

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define app_glikey_base GLIKEY0
#define EXAMPLE_CRITICAL_VALUE 0x42
#define GLIKEY_CODEWORD_STEP3_PROTECTED (GLIKEY_CODEWORD_STEP3 ^ EXAMPLE_CRITICAL_VALUE)
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function.
 */
int main(void)
{
    /* Init hardware */
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("GLIKEY Peripheral Driver Example\r\n\r\n");

    uint32_t index = 2u;
    uint32_t example_check_value =
        EXAMPLE_CRITICAL_VALUE; /* Should depend on some calculation relevant to unlocked index*/

    status_t status = GLIKEY_IsLocked(app_glikey_base);
    if (kStatus_GLIKEY_NotLocked != status)
    {
        PRINTF("ERROR: GLIKEY locked\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY is not locked\r\n\r\n");
    }

    status = GLIKEY_SyncReset(app_glikey_base);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Sync reset fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Sync Reset done\r\n\r\n");
    }

    status = GLIKEY_StartEnable(app_glikey_base, index);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Start Enable fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Start enable done\r\n\r\n");
    }

    /* Perform tests to assure enabling of the indexed SFR can continue */
    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP1);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 1 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable Step 1 pass\r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP2);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 2 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable Step 2 pass\r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP3_PROTECTED ^ example_check_value);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 3 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable Step 3 pass\r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP4);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 4 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable  Step 4 pass \r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP5);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 5 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable  Step 5 pass \r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP6);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 6 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable  Step 6 pass \r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP7);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step 7 fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable  Step 7 pass \r\n\r\n");
    }

    status = GLIKEY_ContinueEnable(app_glikey_base, GLIKEY_CODEWORD_STEP_EN);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY Continue step enable fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY Continue enable Step Enable pass \r\n\r\n");
    }

    PRINTF("SUCCESS: Register on index %d can be now written! \r\n\r\n", index);

    status = GLIKEY_EndOperation(app_glikey_base);
    if (kStatus_Success != status)
    {
        PRINTF("ERROR: GLIKEY End operation fail\r\n\r\n");
        return kStatus_Fail;
    }
    else
    {
        PRINTF("*Success* GLIKEY End of operation done\r\n\r\n");
    }

    PRINTF("End of example\r\n");
    /* End of example */
    while (1)
    {
    }
}
