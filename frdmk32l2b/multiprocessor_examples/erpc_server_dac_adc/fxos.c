/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_fxos.h"
#include "fsl_debug_console.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
fxos_handle_t g_fxosHandle;
uint8_t g_sensor_address[] = {0x1CU, 0x1EU, 0x1DU, 0x1FU};
uint8_t g_sensorRange      = 0;
uint8_t g_dataScale        = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

void Sensor_ReadData(int16_t *Ax, int16_t *Ay, int16_t *Az, int16_t *Mx, int16_t *My, int16_t *Mz)
{
    fxos_data_t fxos_data;

    if (FXOS_ReadSensorData(&g_fxosHandle, &fxos_data) != kStatus_Success)
    {
        PRINTF("Failed to read acceleration data!\r\n");
    }
    /* Get the accel data from the sensor data structure in 14 bit left format data*/
    *Ax = (int16_t)((uint16_t)((uint16_t)fxos_data.accelXMSB << 8) | (uint16_t)fxos_data.accelXLSB) / 4U;
    *Ay = (int16_t)((uint16_t)((uint16_t)fxos_data.accelYMSB << 8) | (uint16_t)fxos_data.accelYLSB) / 4U;
    *Az = (int16_t)((uint16_t)((uint16_t)fxos_data.accelZMSB << 8) | (uint16_t)fxos_data.accelZLSB) / 4U;
    *Ax *= g_dataScale;
    *Ay *= g_dataScale;
    *Az *= g_dataScale;
    *Mx = (int16_t)((uint16_t)((uint16_t)fxos_data.magXMSB << 8) | (uint16_t)fxos_data.magXLSB);
    *My = (int16_t)((uint16_t)((uint16_t)fxos_data.magYMSB << 8) | (uint16_t)fxos_data.magYLSB);
    *Mz = (int16_t)((uint16_t)((uint16_t)fxos_data.magZMSB << 8) | (uint16_t)fxos_data.magZLSB);
}

int32_t init_mag_accel(void)
{
    fxos_config_t config     = {0};
    status_t result          = kStatus_Fail;
    uint16_t i               = 0;
    uint16_t array_addr_size = 0;

    /* Configure the I2C function */
    config.I2C_SendFunc    = BOARD_Accel_I2C_Send;
    config.I2C_ReceiveFunc = BOARD_Accel_I2C_Receive;

    /* Initialize sensor devices */
    array_addr_size = sizeof(g_sensor_address) / sizeof(g_sensor_address[0]);
    for (i = 0; i < array_addr_size; i++)
    {
        config.slaveAddress = g_sensor_address[i];
        /* Initialize accelerometer sensor */
        result = FXOS_Init(&g_fxosHandle, &config);
        if (result == kStatus_Success)
        {
            break;
        }
    }

    if (result != kStatus_Success)
    {
        return -1;
    }
    /* Get sensor range */
    if (FXOS_ReadReg(&g_fxosHandle, XYZ_DATA_CFG_REG, &g_sensorRange, 1) != kStatus_Success)
    {
        return -1;
    }
    if (g_sensorRange == 0x00)
    {
        g_dataScale = 2U;
    }
    else if (g_sensorRange == 0x01)
    {
        g_dataScale = 4U;
    }
    else if (g_sensorRange == 0x10)
    {
        g_dataScale = 8U;
    }

    return 0;
}
