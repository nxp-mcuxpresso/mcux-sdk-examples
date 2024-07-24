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
#define EXAMPLE_MASTER             I3C0
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
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
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
#endif

#define I3C_NXP_VENDOR_ID 0x11B

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static i3c_bus_t demo_i3cBus;
static i3c_device_t demo_slaveDev;

extern i3c_device_control_info_t i3cMasterCtlInfo;

/*******************************************************************************
 * Code
 ******************************************************************************/
i3c_master_adapter_resource_t demo_masterResource = {.base      = EXAMPLE_MASTER,
                                                     .transMode = kI3C_MasterTransferInterruptMode};
i3c_device_control_info_t i3cMasterCtlInfo        = {
           .funcs = (i3c_device_hw_ops_t *)&master_ops, .resource = &demo_masterResource, .isSecondary = true};

/*!
 * @brief Main function
 */
int main(void)
{
    i3c_device_information_t masterInfo;
    i3c_device_information_t devInfo;
    i3c_bus_config_t busConfig;
    status_t result;

    /* Attach main clock to I3C, 396MHz / 4 = 99MHz. */
    CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 4);

    /* Attach lposc_1m clock to I3C time control, clear halt for slow clock. */
    CLOCK_AttachClk(kLPOSC_to_I3C_TC_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cTcClk, 1);
    CLOCK_SetClkDiv(kCLOCK_DivI3cSlowClk, 1);

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    demo_masterResource.clockInHz = I3C_MASTER_CLOCK_FREQUENCY;

    PRINTF("\r\nI3C bus slave example.\r\n");

    /* Create I3C bus, work as secondary master. */
    I3C_BusGetDefaultBusConfig(&busConfig);
    busConfig.i2cBaudRate = EXAMPLE_I2C_BAUDRATE;
    busConfig.i3cOpenDrainBaudRate = EXAMPLE_I3C_OD_BAUDRATE;
    busConfig.i3cPushPullBaudRate = EXAMPLE_I3C_PP_BAUDRATE;
    I3C_BusCreate(&demo_i3cBus, &busConfig);

    memset(&masterInfo, 0, sizeof(masterInfo));
    masterInfo.staticAddr = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterInfo.bcr        = 0x60U;
#if defined(FSL_FEATURE_I3C_HAS_IBI_PAYLOAD_SIZE_OPTIONAL_BYTE) && FSL_FEATURE_I3C_HAS_IBI_PAYLOAD_SIZE_OPTIONAL_BYTE
    masterInfo.bcr |= I3C_BUS_DEV_BCR_IBI_PAYLOAD_MASK;
#endif
    masterInfo.dcr        = 0x00U;
    masterInfo.vendorID   = I3C_NXP_VENDOR_ID;
    I3C_BusMasterCreate(&demo_slaveDev, &demo_i3cBus, &masterInfo, &i3cMasterCtlInfo);

    PRINTF("I3C bus secondary master creates.\r\n");

    /* Wait for address assignment finish. */
    while (EXAMPLE_MASTER->SDYNADDR == 0U)
    {
    }
    PRINTF("I3C bus second master requires to be primary master.\r\n");

    /* Request mastership. */
    result = I3C_BusSlaveRequestMasterShip(&demo_slaveDev);
    if (result != kStatus_Success)
    {
        return result;
    }

    /* Wait for mastership takeover. */
    while (demo_i3cBus.currentMaster != &demo_slaveDev)
    {
    }

    PRINTF("I3C bus mastership takeover.\r\n");

    uint8_t slaveAddr = 0xFF;
    for (list_element_handle_t listItem = demo_i3cBus.i3cDevList.head; listItem != NULL; listItem = listItem->next)
    {
        if ((i3c_device_t *)listItem == &demo_slaveDev)
        {
            continue;
        }
        else
        {
            slaveAddr = ((i3c_device_t *)listItem)->info.dynamicAddr;
            break;
        }
    }
    if (slaveAddr == 0xFFU)
    {
        PRINTF("I3C dynamic address is wrong.\r\n");
        return kStatus_Fail;
    }

    result = I3C_BusMasterGetDeviceInfo(&demo_slaveDev, slaveAddr, &devInfo);
    if (result != kStatus_Success)
    {
        PRINTF("I3C bus master get device info fails with error code %u.\r\n", result);
        return result;
    }

    PRINTF("I3C bus get device info:\r\nDynamicAddr - 0x%X, VendorID - 0x%X.\r\n", devInfo.dynamicAddr, devInfo.vendorID);

    while (1)
    {
    }
}
