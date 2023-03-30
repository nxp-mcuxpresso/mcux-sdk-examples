/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_ssarc.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define EXAMPLE_LED_GPIO     BOARD_USER_LED_GPIO
#define EXAMPLE_LED_GPIO_PIN BOARD_USER_LED_GPIO_PIN

#define EXAMPLE_SSARC_GROUP_ID         0U
#define EXAMPLE_SSARC_DESCRIPTOR_ID    0U
#define EXAMPLE_SSARC_GROUP_CPU_DOMAIN kSSARC_CM4Core


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/


int main(void)
{
    ssarc_descriptor_config_t descriptorConfig;
    ssarc_group_config_t groupConfig;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    PRINTF("SSARC Software Trigger Example Start.\r\n");

    PRINTF("Open the LED.\r\n");
    USER_LED_ON();

    groupConfig.startIndex      = EXAMPLE_SSARC_DESCRIPTOR_ID;
    groupConfig.endIndex        = EXAMPLE_SSARC_DESCRIPTOR_ID;
    groupConfig.highestAddress  = 0xFFFFFFFFU;
    groupConfig.lowestAddress   = 0x00000000U;
    groupConfig.saveOrder       = kSSARC_ProcessFromStartToEnd;
    groupConfig.savePriority    = 0U;
    groupConfig.restoreOrder    = kSSARC_ProcessFromStartToEnd;
    groupConfig.restorePriority = 0U;
    groupConfig.powerDomain     = kSSARC_LPSRMIXPowerDomain;
    groupConfig.cpuDomain       = EXAMPLE_SSARC_GROUP_CPU_DOMAIN;
    SSARC_GroupInit(SSARC_LP, EXAMPLE_SSARC_GROUP_ID, &groupConfig);

    descriptorConfig.size      = kSSARC_DescriptorRegister32bitWidth;
    descriptorConfig.address   = (uint32_t)(&EXAMPLE_LED_GPIO->DR);
    descriptorConfig.data      = 0UL;
    descriptorConfig.operation = kSSARC_SaveEnableRestoreEnable;
    descriptorConfig.type      = kSSARC_ReadValueWriteBack;
    SSARC_SetDescriptorConfig(SSARC_HP, EXAMPLE_SSARC_DESCRIPTOR_ID, &descriptorConfig);

    PRINTF("Please press any key to trigger save operation.\r\n");
    GETCHAR();
    PRINTF("Saving. Please Wait.\r\n");
    SSARC_TriggerSoftwareRequest(SSARC_LP, EXAMPLE_SSARC_GROUP_ID, kSSARC_TriggerSaveRequest);

    PRINTF("Closed the LED.\r\n");
    USER_LED_OFF();

    PRINTF("Please press any key to trigger restore operation. After restoring, the LED will be re-opened.\r\n");
    GETCHAR();
    PRINTF("Restoring. Please Wait.\r\n");
    SSARC_TriggerSoftwareRequest(SSARC_LP, EXAMPLE_SSARC_GROUP_ID, kSSARC_TriggerRestoreRequest);

    PRINTF("SSARC Software Trigger Example Finish.\r\n");
    while (1)
    {
    }
}
