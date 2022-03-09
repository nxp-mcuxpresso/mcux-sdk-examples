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
#include "fsl_icm42688p.h"
#include "fsl_component_i3c.h"

#include "fsl_component_i3c_adapter.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x01A
#define SENSOR_STATIC_ADDRESS      0x69U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void icm42688p_ibi_callback(i3c_device_t *dev, const void *ibiData, uint32_t ibiLen);

/*******************************************************************************
 * Variables
 ******************************************************************************/
i3c_bus_t demo_i3cBus;
i3c_device_t demo_masterDev;
i3c_device_t *demo_icm42688pDev;
icm42688p_handle_t icmp42688p_handle;
volatile bool icm42688p_ibiFlag = false;
uint8_t icm42688p_ibiData[16];
uint8_t icm42688p_ibiLen;
static i3c_device_ibi_info_t dev_icm42688pIbi = {
    .maxPayloadLength = 1, .enabled = true, .ibiHandler = icm42688p_ibi_callback};

/*******************************************************************************
 * Code
 ******************************************************************************/
i3c_master_adapter_resource_t demo_masterResource = {.base      = EXAMPLE_MASTER,
                                                     .transMode = kI3C_MasterTransferInterruptMode};
i3c_device_control_info_t i3cMasterCtlInfo        = {
    .funcs = (i3c_device_hw_ops_t *)&master_ops, .resource = &demo_masterResource, .isSecondary = false};

void icm42688p_ibi_callback(i3c_device_t *dev, const void *ibiData, uint32_t ibiLen)
{
    icm42688p_ibiFlag = true;
    icm42688p_ibiLen  = ibiLen;
    memcpy(icm42688p_ibiData, ibiData, icm42688p_ibiLen);
}

status_t I3C_WriteSensor(uint8_t deviceAddress, uint8_t regAddress, uint8_t *regData, size_t dataSize)
{
    status_t result = kStatus_Success;
    i3c_bus_transfer_t busXfer;
    memset(&busXfer, 0, sizeof(busXfer));

    busXfer.regAddr     = regAddress;
    busXfer.regAddrSize = 1;
    busXfer.isRead      = false;
    busXfer.data        = regData;
    busXfer.dataSize    = dataSize;

    result = I3C_BusMasterDoTransferToI3CDev(&demo_masterDev, demo_icm42688pDev, &busXfer);

    return result;
}

status_t I3C_ReadSensor(uint8_t deviceAddress, uint8_t regAddress, uint8_t *regData, size_t dataSize)
{
    status_t result = kStatus_Success;
    i3c_bus_transfer_t busXfer;
    memset(&busXfer, 0, sizeof(busXfer));

    busXfer.regAddr     = regAddress;
    busXfer.regAddrSize = 1;
    busXfer.isRead      = true;
    busXfer.data        = regData;
    busXfer.dataSize    = dataSize;

    result = I3C_BusMasterDoTransferToI3CDev(&demo_masterDev, demo_icm42688pDev, &busXfer);

    return result;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Attach main clock to I3C, 500MHz / 20 = 25MHZ. */
    CLOCK_AttachClk(kMAIN_CLK_to_I3C_CLK);
    CLOCK_SetClkDiv(kCLOCK_DivI3cClk, 20);

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    demo_masterResource.clockInHz = I3C_MASTER_CLOCK_FREQUENCY;

    PRINTF("\r\nI3C bus master read sensor example.\r\n");

    /* Create I3C bus. */
    i3c_bus_config_t busConfig;
    I3C_BusGetDefaultBusConfig(&busConfig);
    busConfig.i3cPushPullBaudRate  = 12500000ULL;
    busConfig.i3cOpenDrainBaudRate = 4000000ULL;
    I3C_BusCreate(&demo_i3cBus, &busConfig);

    i3c_device_information_t masterInfo;
    memset(&masterInfo, 0, sizeof(masterInfo));
    masterInfo.staticAddr  = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterInfo.bcr         = 0x60U;
    masterInfo.dcr         = 0x00U;
    masterInfo.dynamicAddr = I3C_BusGetValidAddrSlot(&demo_i3cBus, 0);
    masterInfo.vendorID    = 0x345U;

    extern i3c_device_control_info_t i3cMasterCtlInfo;

    /* If you need do pre-assign init dynamic address to device, need to create device structure and
    add device into bus before master create. Master create will firstly do setdasa to the device list on
    bus which have init dynamic address configured, then it will dynamically probe device on
    bus and create device structure for the newly probed device. */
    I3C_BusMasterCreate(&demo_masterDev, &demo_i3cBus, &masterInfo, &i3cMasterCtlInfo);

    PRINTF("\r\nI3C bus master creates.\r\n");

    i3c_device_t *eachDev = NULL;

    for (list_element_handle_t listItem = demo_i3cBus.i3cDevList.head; listItem != NULL; listItem = listItem->next)
    {
        eachDev = (i3c_device_t *)listItem;
        if (eachDev == &demo_masterDev)
        {
            continue;
        }
        else
        {
            if (eachDev->info.vendorID == 0x235)
            {
                demo_icm42688pDev = eachDev;
                break;
            }
        }
    }

    I3C_BusMasterRegisterDevIBI(&demo_masterDev, demo_icm42688pDev, &dev_icm42688pIbi);

    status_t result    = kStatus_Success;
    uint8_t sensorAddr = demo_icm42688pDev->info.dynamicAddr;

    icm42688p_config_t sensorConfig;
    sensorConfig.Sensor_WriteTransfer = (sensor_write_transfer_func_t)I3C_WriteSensor;
    sensorConfig.Sensor_ReadTransfer  = (sensor_read_transfer_func_t)I3C_ReadSensor;
    sensorConfig.sensorAddress        = sensorAddr;
    sensorConfig.isReset              = true;

    result = ICM42688P_Init(&icmp42688p_handle, &sensorConfig);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSensor reset failed.\r\n");
        return -1;
    }

    /* sensor is reset, use SETDASA to set the sensor static address as dynamic address. */
    result = I3C_BusMasterSetDynamicAddrFromStaticAddr(&demo_masterDev, SENSOR_STATIC_ADDRESS, sensorAddr);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSet sensor address from static address failure.\r\n");
        return -1;
    }

    result = ICM42688P_EnableSensors(&icmp42688p_handle);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSensor enable failed.\r\n");
        return -1;
    }

    result = ICM42688P_ConfigureTapDetectIBI(&icmp42688p_handle);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nEnable TAP detect IBI failed.\r\n");
        return -1;
    }

    uint8_t bankSel = 0;
    /* Select bank 0 to read sensor data.*/
    result = ICM42688P_WriteReg(&icmp42688p_handle, BANK_SEL, &bankSel, 1);
    if (result != kStatus_Success)
    {
        PRINTF("\r\nSelect sensor bank 0 failure.\r\n");
        return -1;
    }

    icm42688p_sensor_data_t sensorData = {0};
    while (!icm42688p_ibiFlag)
    {
        result = ICM42688P_ReadSensorData(&icmp42688p_handle, &sensorData);
        if (result != kStatus_Success)
        {
            PRINTF("\r\nRead sensor data failed.\r\n");
            return -1;
        }
        PRINTF("\r\nSensor Data: ACCEL X %d, Y %d, Z %d; GYRO X %d, Y %d, Z %d.", sensorData.accelDataX,
               sensorData.accelDataY, sensorData.accelDataZ, sensorData.gyroDataX, sensorData.gyroDataY,
               sensorData.gyroDataZ);
        SDK_DelayAtLeastUs(50000, SystemCoreClock);
    }

    while (1)
    {
        if (icm42688p_ibiFlag)
        {
            PRINTF("\r\nReceived slave IBI request.");
            for (uint8_t count = 0; count < icm42688p_ibiLen; count++)
            {
                PRINTF(" Data 0x%x.", icm42688p_ibiData[count]);
            }
            icm42688p_ibiFlag = false;
        }
    }
}
