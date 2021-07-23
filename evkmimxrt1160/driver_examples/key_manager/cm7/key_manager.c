/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_key_manager.h"

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
 * @brief Main function
 */
int main(void)
{
    domain_slot_config_t config;
    status_t status = kStatus_Fail;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("Key Manager driver example.\r\n");

    /* Initialize the Key manager */
    PRINTF("Key Manager Init.\r\n");
    KEYMGR_Init(KEY_MANAGER);

    /* Select PUF for Master key and lock register for writing */
    PRINTF("Select PUF for Master key and lock register for writing. \r\n");
    status = KEYMGR_MasterKeyControll(KEY_MANAGER, KEYMGR_SEL_PUF, kKEYMGR_Lock);
    if (status != kStatus_Success)
    {
        PRINTF("Error while Master Key controll. \r\n");
        return kStatus_Fail;
    }

    /* Get default slot configuration */
    status = KEYMGR_GetDefaultConfig(&config);
    if (status != kStatus_Success)
    {
        PRINTF("Error while getting default slot configuration. \r\n");
        return kStatus_Fail;
    }

    /* Set up configuration to lock and disallow access for domain 0 */
    config.allowNonSecure = kKEYMGR_Disallow;
    config.allowUser      = kKEYMGR_Disallow;
    config.lockList       = kKEYMGR_Lock;
    config.lockControl    = kKEYMGR_Lock;
    config.whiteList      = 0u;

    /* Set slot configuration */
    PRINTF("Setting slot 0 (MASTER_KEY_CTRL register) configuration. \r\n");
    status = KEYMGR_SlotControl(KEY_MANAGER, &config, kKEYMGR_Slot0);
    if (status != kStatus_Success)
    {
        PRINTF("Error while setting slot configuration. \r\n");
        return kStatus_Fail;
    }

    PRINTF("Example end. \r\n");

    while (1)
    {
    }
}
