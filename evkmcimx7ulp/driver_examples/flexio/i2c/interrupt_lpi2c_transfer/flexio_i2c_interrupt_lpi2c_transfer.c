/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
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
#include "fsl_lpi2c.h"
#include "fsl_flexio_i2c_master.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_LPI2C_SLAVE_BASE LPI2C3
#define BOARD_LPI2C_SLAVE_IRQn LPI2C3_IRQn
#define BOARD_FLEXIO_BASE      FLEXIO0
#define LPI2C_CLOCK_FREQUENCY  CLOCK_GetIpFreq(kCLOCK_Lpi2c3)
#define FLEXIO_CLOCK_FREQUENCY CLOCK_GetIpFreq(kCLOCK_Flexio0)
#define FLEXIO_I2C_SDA_PIN     1U
#define FLEXIO_I2C_SCL_PIN     0U

/* I2C Baudrate 400K */
#define I2C_BAUDRATE (400000)
/* I2C Slave Address */
#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
/* The length of data */
#define I2C_DATA_LENGTH (32U)

/*******************************************************************************
 * Prototypes
 *******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_slave_buff[I2C_DATA_LENGTH];
uint8_t g_master_buff[I2C_DATA_LENGTH];

flexio_i2c_master_handle_t g_m_handle;
FLEXIO_I2C_Type i2cDev;
lpi2c_slave_handle_t g_s_handle;
volatile bool completionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void lpi2c_slave_callback(LPI2C_Type *base, lpi2c_slave_transfer_t *xfer, void *param)
{
    if (xfer->event == kLPI2C_SlaveReceiveEvent)
    {
        xfer->data     = g_slave_buff;
        xfer->dataSize = I2C_DATA_LENGTH;
    }

    if (xfer->event == kLPI2C_SlaveCompletionEvent)
    {
        completionFlag = true;
    }
}

int main(void)
{
    lpi2c_slave_config_t slaveConfig;

    flexio_i2c_master_transfer_t masterXfer;
    uint32_t timeout = UINT32_MAX;

    BOARD_InitPins();
    BOARD_I2C_ConfigurePins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Flexio0, kCLOCK_IpSrcFircAsync);
    CLOCK_SetIpSrc(kCLOCK_Lpi2c3, kCLOCK_IpSrcFircAsync);

    PRINTF("\r\nFlexIO I2C interrupt - LPI2C interrupt\r\n");

    /*1.Set up i2c slave first*/
    /*
     * slaveConfig.address0 = 0U;
     * slaveConfig.address1 = 0U;
     * slaveConfig.addressMatchMode = kLPI2C_MatchAddress0;
     * slaveConfig.filterDozeEnable = true;
     * slaveConfig.filterEnable = true;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.ignoreAck = false;
     * slaveConfig.enableReceivedAddressRead = false;
     * slaveConfig.sdaGlitchFilterWidth_ns = 0;
     * slaveConfig.sclGlitchFilterWidth_ns = 0;
     * slaveConfig.dataValidDelay_ns = 0;
     * slaveConfig.clockHoldTime_ns = 0;
     */
    LPI2C_SlaveGetDefaultConfig(&slaveConfig);

    slaveConfig.address0 = I2C_MASTER_SLAVE_ADDR_7BIT;

    LPI2C_SlaveInit(BOARD_LPI2C_SLAVE_BASE, &slaveConfig, LPI2C_CLOCK_FREQUENCY);

    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        g_slave_buff[i] = 0;
    }

    memset(&g_s_handle, 0, sizeof(g_s_handle));

    LPI2C_SlaveTransferCreateHandle(BOARD_LPI2C_SLAVE_BASE, &g_s_handle, lpi2c_slave_callback, NULL);
    LPI2C_SlaveTransferNonBlocking(BOARD_LPI2C_SLAVE_BASE, &g_s_handle, kLPI2C_SlaveCompletionEvent);

    /*2.Set up i2c master to send data to slave*/
    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        g_master_buff[i] = i;
    }

    PRINTF("Master will send data :");
    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%02X  ", g_master_buff[i]);
    }
    PRINTF("\r\n\r\n");

    flexio_i2c_master_config_t masterConfig;

    /*do hardware configuration*/
    i2cDev.flexioBase      = BOARD_FLEXIO_BASE;
    i2cDev.SDAPinIndex     = FLEXIO_I2C_SDA_PIN;
    i2cDev.SCLPinIndex     = FLEXIO_I2C_SCL_PIN;
    i2cDev.shifterIndex[0] = 0U;
    i2cDev.shifterIndex[1] = 1U;
    i2cDev.timerIndex[0]   = 0U;
    i2cDev.timerIndex[1]   = 1U;
    i2cDev.timerIndex[2]   = 2U;

    /*
     * masterConfig.enableMaster = true;
     * masterConfig.enableInDoze = false;
     * masterConfig.enableInDebug = true;
     * masterConfig.enableFastAccess = false;
     * masterConfig.baudRate_Bps = 100000U;
     */
    FLEXIO_I2C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Bps = I2C_BAUDRATE;
    FLEXIO_I2C_MasterInit(&i2cDev, &masterConfig, FLEXIO_CLOCK_FREQUENCY);

    memset(&g_m_handle, 0, sizeof(g_m_handle));
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress   = I2C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kFLEXIO_I2C_Write;
    masterXfer.subaddress     = 0;
    masterXfer.subaddressSize = 0;
    masterXfer.data           = g_master_buff;
    masterXfer.dataSize       = I2C_DATA_LENGTH;

    FLEXIO_I2C_MasterTransferCreateHandle(&i2cDev, &g_m_handle, NULL, NULL);
    FLEXIO_I2C_MasterTransferNonBlocking(&i2cDev, &g_m_handle, &masterXfer);

    /*  wait for transfer completed. */
    while ((!completionFlag) && (--timeout))
    {
    }
    completionFlag = false;

    /* Failed to complete the transfer. */
    if (timeout == 0)
    {
        PRINTF("\r\nTime out !\r\n");
    }

    /*3.Transfer completed. Check the data.*/
    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (g_slave_buff[i] != g_master_buff[i])
        {
            PRINTF("\r\nError occurred in this transfer !\r\n");
            break;
        }
    }

    PRINTF("Slave received data :");
    for (uint32_t i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%02X  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");

    while (1)
    {
    }
}
