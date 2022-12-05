/*
 * Copyright 2019 NXP
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
#include "fsl_i3c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_MASTER             I3C0
#define EXAMPLE_I2C_BAUDRATE       400000
#define I3C_MASTER_CLOCK_FREQUENCY CLOCK_GetI3cClkFreq()
#define I3C_MASTER_SLAVE_ADDR_7BIT 0x1E
#define WAIT_TIME                  1000
#define I3C_DATA_LENGTH            33

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_master_txBuff[I3C_DATA_LENGTH];
uint8_t g_master_rxBuff[I3C_DATA_LENGTH];

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    i3c_master_config_t masterConfig;
    i3c_master_transfer_t masterXfer;
    status_t result = kStatus_Success;

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

    PRINTF("\r\nI3C board2board polling example -- Master transfer.\r\n");

    /* I3C mode: Set up i3c master to work in I3C mode, send data to slave*/
    /* First data in txBuff is data length of the transmitting data. */
    g_master_txBuff[0] = I3C_DATA_LENGTH - 1U;
    for (uint32_t i = 1U; i < I3C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i - 1;
    }

    PRINTF("\r\nStart to do I3C master transfer in I2C mode.\r\n");

    PRINTF("Master will send data :");
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i + 1]);
    }
    PRINTF("\r\n\r\n");

    I3C_MasterGetDefaultConfig(&masterConfig);
    masterConfig.baudRate_Hz.i2cBaud          = EXAMPLE_I2C_BAUDRATE;
    masterConfig.baudRate_Hz.i3cPushPullBaud  = 4000000U;
    masterConfig.baudRate_Hz.i3cOpenDrainBaud = 1500000U;
    masterConfig.enableOpenDrainStop          = false;
    I3C_MasterInit(EXAMPLE_MASTER, &masterConfig, I3C_MASTER_CLOCK_FREQUENCY);

    memset(&masterXfer, 0, sizeof(masterXfer));

    /* subAddress = 0x01, data = g_master_txBuff - write to slave.
      start + slaveaddress(w) + subAddress + length of data buffer + data buffer + stop*/
    uint8_t deviceAddress     = 0x01U;
    masterXfer.slaveAddress   = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI3C_Write;
    masterXfer.busType        = kI3C_TypeI2C;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_txBuff;
    masterXfer.dataSize       = I3C_DATA_LENGTH;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        return -1;
    }

    PRINTF("Receive sent data from slave :");

    /* Wait until the slave is ready for transmit, wait time depend on user's case.
       Slave devices that need some time to process received byte or are not ready yet to
       send the next byte, can pull the clock low to signal to the master that it should wait.*/
    for (uint32_t i = 0U; i < WAIT_TIME; i++)
    {
        __NOP();
    }

    /* subAddress = 0x01, data = g_master_rxBuff - read from slave.
      start + slaveaddress(w) + subAddress + repeated start + slaveaddress(r) + rx data buffer + stop */
    masterXfer.slaveAddress   = I3C_MASTER_SLAVE_ADDR_7BIT;
    masterXfer.direction      = kI3C_Read;
    masterXfer.busType        = kI3C_TypeI2C;
    masterXfer.subaddress     = (uint32_t)deviceAddress;
    masterXfer.subaddressSize = 1;
    masterXfer.data           = g_master_rxBuff;
    masterXfer.dataSize       = I3C_DATA_LENGTH - 1U;
    masterXfer.flags          = kI3C_TransferDefaultFlag;

    result = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        return -1;
    }

    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Transfer completed. Check the data.*/
    for (uint32_t i = 0U; i < I3C_DATA_LENGTH - 1U; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer !\r\n");
            return -1;
        }
    }

    PRINTF("\r\nI3C master transfer successful in I2C mode .\r\n");

    PRINTF("\r\nI3C master do dynamic address assignment to the I3C slaves on bus.\r\n");

    uint8_t addressList[8] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37};
    result                 = I3C_MasterProcessDAA(EXAMPLE_MASTER, addressList, 8);
    if (result != kStatus_Success)
    {
        return -1;
    }

    PRINTF("\r\nI3C master dynamic address assignment done.\r\n");

    PRINTF("\r\nStart to do I3C master transfer in I3C SDR mode.\r\n");
    memset(&masterXfer, 0, sizeof(masterXfer));

    masterXfer.slaveAddress = 0x30U;
    masterXfer.data         = g_master_txBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH;
    masterXfer.direction    = kI3C_Write;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        return -1;
    }

    /* Wait until the slave is ready for transmit, wait time depend on user's case.*/
    for (uint32_t i = 0U; i < WAIT_TIME; i++)
    {
        __NOP();
    }

    memset(g_master_rxBuff, 0, I3C_DATA_LENGTH);

    masterXfer.slaveAddress = 0x30U;
    masterXfer.data         = g_master_rxBuff;
    masterXfer.dataSize     = I3C_DATA_LENGTH - 1U;
    masterXfer.direction    = kI3C_Read;
    masterXfer.busType      = kI3C_TypeI3CSdr;
    masterXfer.flags        = kI3C_TransferDefaultFlag;
    masterXfer.ibiResponse  = kI3C_IbiRespAckMandatory;
    result                  = I3C_MasterTransferBlocking(EXAMPLE_MASTER, &masterXfer);
    if (result != kStatus_Success)
    {
        return -1;
    }

    for (uint32_t i = 0U; i < g_master_txBuff[0]; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i + 1])
        {
            PRINTF("\r\nError occurred in the transfer !\r\n");
            return -1;
        }
    }

    PRINTF("\r\nI3C master transfer successful in I3C SDR mode .\r\n");

    while (1)
    {
    }
}
