/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"
#include "fsl_p3t1755.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER_BASE    I2C2
#define I2C_MASTER_CLOCK_FREQUENCY CLOCK_GetFlexCommClkFreq(2U)
#define I2C_MASTER_SLAVE_ADDR_7BIT (0x48U)

#define EXAMPLE_I2C_MASTER ((I2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define I2C_BAUDRATE               (100000) /* 100K */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t I2C_WriteSensor(uint8_t deviceAddress, uint32_t regAddress, uint8_t *regData, size_t dataSize);
status_t I2C_ReadSensor(uint8_t deviceAddress, uint32_t regAddress, uint8_t *regData, size_t dataSize);

/*******************************************************************************
 * Variables
 ******************************************************************************/
i2c_master_handle_t g_m_handle;
volatile bool g_MasterCompletionFlag;
p3t1755_handle_t p3t1755Handle;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void i2c_master_callback(I2C_Type *base, i2c_master_handle_t *handle, status_t status, void *userData)
{
    /* Signal transfer success when received success status. */
    if (status == kStatus_Success)
    {
        g_MasterCompletionFlag = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t count = 0;
    status_t result = kStatus_Success;
    i2c_master_config_t masterConfig;
    double temperature;
    p3t1755_config_t p3t1755Config;

    /* Use 16 MHz clock for the FLEXCOMM2 */
    CLOCK_AttachClk(kSFRO_to_FLEXCOMM2);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI2C master read sensor data example.\r\n\r\n");

    /*
     * masterConfig.debugEnable = false;
     * masterConfig.ignoreAck = false;
     * masterConfig.pinConfig = kI2C_2PinOpenDrain;
     * masterConfig.baudRate_Bps = 100000U;
     * masterConfig.busIdleTimeout_ns = 0;
     * masterConfig.pinLowTimeout_ns = 0;
     * masterConfig.sdaGlitchFilterWidth_ns = 0;
     * masterConfig.sclGlitchFilterWidth_ns = 0;
     */
    I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, I2C_MASTER_CLOCK_FREQUENCY);

    /* Create the I2C handle for the non-blocking transfer */
    I2C_MasterTransferCreateHandle(EXAMPLE_I2C_MASTER, &g_m_handle, i2c_master_callback, NULL);

    p3t1755Config.writeTransfer = I2C_WriteSensor;
    p3t1755Config.readTransfer  = I2C_ReadSensor;
    p3t1755Config.sensorAddress = I2C_MASTER_SLAVE_ADDR_7BIT;
    P3T1755_Init(&p3t1755Handle, &p3t1755Config);

    // measure temperature 10 times
    do
    {
        if (10U == count)
        {
            break;
        }
        
        result = P3T1755_ReadTemperature(&p3t1755Handle, &temperature);
        if (result != kStatus_Success)
        {
            PRINTF("\r\nP3T1755 read temperature failed.\r\n");
        }
        else
        {
            PRINTF("Temperature: %f \r\n", temperature);
        }

        SDK_DelayAtLeastUs(1000000, CLOCK_GetCoreSysClkFreq());

        count++;
    } while (1);

    PRINTF("\r\nEnd of demo.\r\n");

    while (1)
    {
    }
}

status_t I2C_WriteSensor(uint8_t deviceAddress, uint32_t regAddress, uint8_t *regData, size_t dataSize)
{
    status_t result                  = kStatus_Success;
    i2c_master_transfer_t masterXfer = {0};

    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kI2C_Write;
    masterXfer.subaddress     = regAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = regData;
    masterXfer.dataSize       = dataSize;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    g_MasterCompletionFlag = false;
    result = I2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &g_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    while (!g_MasterCompletionFlag)
    {
    }

    return result;
}

status_t I2C_ReadSensor(uint8_t deviceAddress, uint32_t regAddress, uint8_t *regData, size_t dataSize)
{
    status_t result                  = kStatus_Success;
    i2c_master_transfer_t masterXfer = {0};

    masterXfer.slaveAddress   = deviceAddress;
    masterXfer.direction      = kI2C_Read;
    masterXfer.subaddress     = regAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = regData;
    masterXfer.dataSize       = dataSize;
    masterXfer.flags          = kI2C_TransferDefaultFlag;

    g_MasterCompletionFlag = false;
    result                 = I2C_MasterTransferNonBlocking(EXAMPLE_I2C_MASTER, &g_m_handle, &masterXfer);
    if (kStatus_Success != result)
    {
        return result;
    }

    while (!g_MasterCompletionFlag)
    {
    }

    return result;
}
