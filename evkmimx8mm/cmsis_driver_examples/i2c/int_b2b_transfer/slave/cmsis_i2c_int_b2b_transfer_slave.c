/*
 * Copyright 2017 NXP
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
#include "fsl_i2c.h"
#include "Driver_I2C.h"
#include "fsl_i2c_cmsis.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_SLAVE Driver_I2C3
#define I2C_MASTER_SLAVE_ADDR 0x7EU
#define I2C_DATA_LENGTH       32U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_slave_buff[I2C_DATA_LENGTH];
volatile bool g_SlaveCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t I2C3_GetFreq(void)
{
    return (CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootI2c3)) /
            (CLOCK_GetRootPostDivider(kCLOCK_RootI2c3)) / 5); /* SYSTEM PLL1 DIV5 */
}


void I2C_SlaveSignalEvent_t(uint32_t event)
{
    if (event == ARM_I2C_EVENT_TRANSFER_DONE)
    {
        g_SlaveCompletionFlag = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();

    CLOCK_SetRootMux(kCLOCK_RootI2c3, kCLOCK_I2cRootmuxSysPll1Div5); /* Set I2C source to SysPLL1 Div5 160MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootI2c3, 1U, 10U);                  /* Set root clock to 160MHZ / 10 = 16MHZ */

    PRINTF("\r\nCMSIS I2C board2board interrupt example -- Slave transfer.\r\n\r\n");

    /*Init I2C*/
    EXAMPLE_I2C_SLAVE.Initialize(I2C_SlaveSignalEvent_t);

    /* Enable i2c moduole and i2c clock */
    EXAMPLE_I2C_SLAVE.PowerControl(ARM_POWER_FULL);

    /*config slave addr*/
    EXAMPLE_I2C_SLAVE.Control(ARM_I2C_OWN_ADDRESS, I2C_MASTER_SLAVE_ADDR);

    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        g_slave_buff[i] = 0;
    }

    /*start transfer*/
    EXAMPLE_I2C_SLAVE.SlaveReceive(g_slave_buff, I2C_DATA_LENGTH);

    /*  Wait for transfer completed. */
    while (!g_SlaveCompletionFlag)
    {
    }

    /*  Reset slave completion flag to false. */
    g_SlaveCompletionFlag = false;

    EXAMPLE_I2C_SLAVE.SlaveTransmit(g_slave_buff, I2C_DATA_LENGTH);

    /* Wait for master receive completed.*/

    PRINTF("Slave received data :");
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_slave_buff[i]);
    }
    PRINTF("\r\n\r\n");

    while (!g_SlaveCompletionFlag)
    {
    }
    g_SlaveCompletionFlag = false;

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
