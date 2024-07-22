/*
 * Copyright 2020, 2023-2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <string.h>
/*  SDK Included Files */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_component_i3c.h"

#include "fsl_component_i3c_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1A
#ifndef EXAMPLE_I2C_BAUDRATE
#define EXAMPLE_I2C_BAUDRATE    400000U
#endif
#ifndef EXAMPLE_I3C_OD_BAUDRATE
#define EXAMPLE_I3C_OD_BAUDRATE 2000000U
#endif
#ifndef EXAMPLE_I3C_PP_BAUDRATE
#define EXAMPLE_I3C_PP_BAUDRATE 4000000U
#endif
#ifndef I3C_MASTER_SLAVE_ADDR_7BIT
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1A
#endif

#define I3C_NXP_VENDOR_ID 0x11B

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
static i3c_bus_t demo_i3cBus;
static i3c_device_t demo_masterDev;

/*******************************************************************************
 * Code
 ******************************************************************************/
i3c_master_adapter_resource_t demo_masterResource = {.base      = EXAMPLE_MASTER,
                                                     .transMode = kI3C_MasterTransferInterruptMode};
i3c_device_control_info_t i3cMasterCtlInfo        = {
           .funcs = (i3c_device_hw_ops_t *)&master_ops, .resource = &demo_masterResource, .isSecondary = false};


/*!
 * @brief Main function
 */
int main(void)
{
    i3c_device_information_t masterInfo;
    i3c_device_information_t devInfo;
    i3c_bus_config_t busConfig;
    uint8_t slaveAddr;
    status_t result;

    /* Attach main clock to I3C, 500MHz / 10 = 50MHz. */
    CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 10);

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    demo_masterResource.clockInHz = I3C_MASTER_CLOCK_FREQUENCY;

    PRINTF("\r\nI3C bus master example.\r\n");

    /* Create I3C bus. */
    I3C_BusGetDefaultBusConfig(&busConfig);
    busConfig.i2cBaudRate = EXAMPLE_I2C_BAUDRATE;
    busConfig.i3cOpenDrainBaudRate = EXAMPLE_I3C_OD_BAUDRATE;
    busConfig.i3cPushPullBaudRate = EXAMPLE_I3C_PP_BAUDRATE;
    I3C_BusCreate(&demo_i3cBus, &busConfig);

    memset(&masterInfo, 0, sizeof(masterInfo));
    masterInfo.staticAddr  = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterInfo.bcr         = 0x60U;
#if defined(FSL_FEATURE_I3C_HAS_IBI_PAYLOAD_SIZE_OPTIONAL_BYTE) && FSL_FEATURE_I3C_HAS_IBI_PAYLOAD_SIZE_OPTIONAL_BYTE
    masterInfo.bcr |= I3C_BUS_DEV_BCR_IBI_PAYLOAD_MASK;
#endif
    masterInfo.dcr         = 0x00U;
    masterInfo.dynamicAddr = I3C_BusGetValidAddrSlot(&demo_i3cBus, 0);
    masterInfo.vendorID    = I3C_NXP_VENDOR_ID;

    extern i3c_device_control_info_t i3cMasterCtlInfo;
    result = I3C_BusMasterCreate(&demo_masterDev, &demo_i3cBus, &masterInfo, &i3cMasterCtlInfo);
    if (result != kStatus_Success)
    {
        return result;
    }

    PRINTF("I3C bus master creates.\r\n");

    slaveAddr = 0;
    for (list_element_handle_t listItem = demo_i3cBus.i3cDevList.head; listItem != NULL; listItem = listItem->next)
    {
        if ((i3c_device_t *)listItem == &demo_masterDev)
        {
            continue;
        }
        else
        {
            /* Find the connected I3C slave on the other board */
            if (((i3c_device_t *)listItem)->info.vendorID == I3C_NXP_VENDOR_ID)
            {
                slaveAddr = ((i3c_device_t *)listItem)->info.dynamicAddr;
                break;
            }
        }
    }
    if (slaveAddr == 0U)
    {
        PRINTF("I3C dynamic address assignment fails.\r\n");
        return kStatus_Fail;
    }

    result = I3C_BusMasterGetDeviceInfo(&demo_masterDev, slaveAddr, &devInfo);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait for mastership handoff. */
    while (demo_i3cBus.currentMaster == &demo_masterDev)
    {
    }

    PRINTF("I3C bus mastership handoff.\r\n");

    while (1)
    {
    }
}
