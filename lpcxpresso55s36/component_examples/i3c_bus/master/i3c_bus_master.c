/*
 * Copyright 2020 NXP
 * All rights reserved.
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
#include "fsl_i3c.h"

#include "fsl_component_i3c_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C0
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1A

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/

i3c_bus_t demo_i3cBus;
i3c_device_t demo_masterDev;
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
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, true);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, false);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Attach main clock to I3C, 150MHz / 4 = 37.5MHz. */
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 0U, true);
    CLOCK_SetClkDiv(kCLOCK_DivI3cFclk, 4U, false);
    CLOCK_AttachClk(kMAIN_CLK_to_I3CFCLK);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    demo_masterResource.clockInHz = I3C_MASTER_CLOCK_FREQUENCY;

    PRINTF("\r\nI3C bus master example.\r\n");

    /* Create I3C bus. */
    i3c_bus_config_t busConfig;
    I3C_BusGetDefaultBusConfig(&busConfig);
    I3C_BusCreate(&demo_i3cBus, &busConfig);
    i3c_device_information_t masterInfo;
    memset(&masterInfo, 0, sizeof(masterInfo));
    masterInfo.staticAddr  = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterInfo.bcr         = 0x60U;
    masterInfo.dcr         = 0x00U;
    masterInfo.dynamicAddr = I3C_BusGetValidAddrSlot(&demo_i3cBus, 0);
    masterInfo.vendorID    = 0x345U;

    extern i3c_device_control_info_t i3cMasterCtlInfo;
    I3C_BusMasterCreate(&demo_masterDev, &demo_i3cBus, &masterInfo, &i3cMasterCtlInfo);

    PRINTF("\r\nI3C bus master creates.\r\n");

    i3c_device_information_t devInfo;
    uint8_t slaveAddr = 0x0U;

    for (list_element_handle_t listItem = demo_i3cBus.i3cDevList.head; listItem != NULL; listItem = listItem->next)
    {
        if ((i3c_device_t *)listItem == &demo_masterDev)
        {
            continue;
        }
        else
        {
            /* Find the connected I3C slave on the other board */
            if (((i3c_device_t *)listItem)->info.vendorID == 0x346U)
            {
                slaveAddr = ((i3c_device_t *)listItem)->info.dynamicAddr;
                break;
            }
        }
    }
    I3C_BusMasterGetDeviceInfo(&demo_masterDev, slaveAddr, &devInfo);

    /* Wait for mastership handoff. */
    while (demo_i3cBus.currentMaster == &demo_masterDev)
        ;

    PRINTF("\r\nI3C bus mastership handoff.\r\n");

    while (1)
    {
    }
}
