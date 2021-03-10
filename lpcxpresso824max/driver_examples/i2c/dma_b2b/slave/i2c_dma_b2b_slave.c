/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  SDK Included Files */
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_i2c.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_SLAVE                 I2C0
#define EXAMPLE_I2C_SLAVE_CLOCK_FREQUENCY (12000000UL)
#define EXAMPLE_I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define EXAMPLE_I2C_DATA_LENGTH            (32) /* MAX is 256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_slave_buff[EXAMPLE_I2C_DATA_LENGTH];
i2c_slave_handle_t g_s_handle;
volatile bool g_SlaveCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void i2c_slave_callback(I2C_Type *base, volatile i2c_slave_transfer_t *xfer, void *userData)
{
    switch (xfer->event)
    {
        /*  Address match event */
        case kI2C_SlaveAddressMatchEvent:
            xfer->rxData = NULL;
            xfer->rxSize = 0;
            break;
        /*  Receive request */
        case kI2C_SlaveReceiveEvent:
            xfer->rxData = g_slave_buff;
            xfer->rxSize = EXAMPLE_I2C_DATA_LENGTH;
            break;

        /*  Transfer done */
        case kI2C_SlaveCompletionEvent:
            /*  Transmit request */
            g_SlaveCompletionFlag = true;
            break;

        /*  Transmit request */
        case kI2C_SlaveTransmitEvent:
            xfer->txData = g_slave_buff;
            xfer->txSize = EXAMPLE_I2C_DATA_LENGTH;
            break;

        default:
            g_SlaveCompletionFlag = false;
            break;
    }
}

int main(void)
{
    i2c_slave_config_t slaveConfig;

    /* Enable clock of uart0. */
    CLOCK_EnableClock(kCLOCK_Uart0);
    /* Ser DIV of uart0. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
    /* Enable clock of i2c0. */
    CLOCK_EnableClock(kCLOCK_I2c0);

    BOARD_InitPins();
    BOARD_BootClockIRC12M();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI2C board2board DMA example -- Slave transfer.\r\n\r\n");

    /*Set up i2c slave first*/
    /*
     * slaveConfig.addressingMode = kI2C_Address7bit;
     * slaveConfig.enableGeneralCall = false;
     * slaveConfig.enableWakeUp = false;
     * slaveConfig.enableBaudRateCtl = false;
     * slaveConfig.enableSlave = true;
     */
    I2C_SlaveGetDefaultConfig(&slaveConfig);

    /* Change the slave address */
    slaveConfig.address0.address = EXAMPLE_I2C_MASTER_SLAVE_ADDR_7BIT;

    /* Initialize the I2C slave peripheral */
    I2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, EXAMPLE_I2C_SLAVE_CLOCK_FREQUENCY);

    memset(g_slave_buff, 0, sizeof(g_slave_buff));
    memset(&g_s_handle, 0, sizeof(g_s_handle));

    I2C_SlaveTransferCreateHandle(EXAMPLE_I2C_SLAVE, &g_s_handle, i2c_slave_callback, NULL);
    I2C_SlaveTransferNonBlocking(EXAMPLE_I2C_SLAVE, &g_s_handle,
                                 kI2C_SlaveCompletionEvent | kI2C_SlaveAddressMatchEvent);

    /*  wait for transfer completed. */
    while (!g_SlaveCompletionFlag)
    {
    }
    g_SlaveCompletionFlag = false;

    PRINTF("Slave received data :");
    for (uint32_t i = 0U; i < EXAMPLE_I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Wait for master receive completed.*/
    while (!g_SlaveCompletionFlag)
    {
    }
    g_SlaveCompletionFlag = false;

    PRINTF("\r\nEnd of I2C example .\r\n");

    while (1)
    {
    }
}
