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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_SLAVE_BASE    I2C11
#define I2C_SLAVE_CLOCK_FREQUENCY CLOCK_GetFlexcommClkFreq(11)
#define EXAMPLE_I2C_SLAVE ((I2C_Type *)EXAMPLE_I2C_SLAVE_BASE)

#define I2C_MASTER_SLAVE_ADDR_7BIT 0x7EU
#define I2C_DATA_LENGTH            34U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_slave_buff[I2C_DATA_LENGTH];

/*******************************************************************************
 * Code

 ******************************************************************************/

int main(void)
{
    i2c_slave_config_t slaveConfig;
    status_t reVal = kStatus_Fail;
    uint8_t subaddress;

    /* Use 48 MHz clock for the FLEXCOMM11 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM11);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\r\nI2C board2board polling example -- Slave transfer.\r\n\r\n");

    /* Set up i2c slave first*/
    I2C_SlaveGetDefaultConfig(&slaveConfig);

    /* Change the slave address */
    slaveConfig.address0.address = I2C_MASTER_SLAVE_ADDR_7BIT;

    /* Initialize the I2C slave peripheral */
    I2C_SlaveInit(EXAMPLE_I2C_SLAVE, &slaveConfig, I2C_SLAVE_CLOCK_FREQUENCY);

    memset(g_slave_buff, 0, sizeof(g_slave_buff));

    /* Start accepting I2C transfers on the I2C slave peripheral */
    reVal = I2C_SlaveReadBlocking(EXAMPLE_I2C_SLAVE, g_slave_buff, I2C_DATA_LENGTH);

    if (reVal != kStatus_Success)
    {
        return -1;
    }

    /* Start accepting I2C transfers on the I2C slave peripheral to simulate subaddress and will send ACK to master */
    reVal = I2C_SlaveReadBlocking(EXAMPLE_I2C_SLAVE, &subaddress, 1);

    if (reVal != kStatus_Success)
    {
        return -1;
    }

    reVal = I2C_SlaveWriteBlocking(EXAMPLE_I2C_SLAVE, &g_slave_buff[2], g_slave_buff[1]);

    if (reVal != kStatus_Success)
    {
        return -1;
    }

    PRINTF("Slave received data :");
    for (uint32_t i = 0U; i < g_slave_buff[1]; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[2 + i]);
    }
    PRINTF("\r\n\r\n");

    PRINTF("\r\nEnd of I2C example .\r\n");

    while (1)
    {
    }
}
