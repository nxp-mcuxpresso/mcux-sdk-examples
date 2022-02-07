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
#include "fsl_i2c_edma.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_DMAMUX_BASEADDR DMAMUX
#define EXAMPLE_I2C_DMA_BASEADDR    DMA0
#define EXAMPLE_I2C_MASTER          Driver_I2C0

#define I2C_DATA_LENGTH       32U
#define I2C_MASTER_SLAVE_ADDR 0x7E

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t g_master_txBuff[I2C_DATA_LENGTH];
uint8_t g_master_rxBuff[I2C_DATA_LENGTH];
volatile bool g_MasterCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t I2C0_GetFreq(void)
{
    return CLOCK_GetFreq(I2C0_CLK_SRC);
}


void I2C_MasterSignalEvent_t(uint32_t event)
{
    /*  Transfer done */
    if (event == ARM_I2C_EVENT_TRANSFER_DONE)
    {
        g_MasterCompletionFlag = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* DMAMux init and EDMA init */
    DMAMUX_Init(EXAMPLE_I2C_DMAMUX_BASEADDR);
    edma_config_t edmaConfig = {0};
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(EXAMPLE_I2C_DMA_BASEADDR, &edmaConfig);

    /*enable DMA0 IRQ*/
    NVIC_EnableIRQ(DMA0_IRQn);

    /*Init I2C*/
    EXAMPLE_I2C_MASTER.Initialize(I2C_MasterSignalEvent_t);

    EXAMPLE_I2C_MASTER.PowerControl(ARM_POWER_FULL);

    /*config transmit speed*/
    EXAMPLE_I2C_MASTER.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

    PRINTF("\r\nCMSIS I2C board2board EDMA example -- Master transfer.\r\n");

    /* Set up i2c master to send data to slave*/
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i;
    }

    PRINTF("Master will send data :");
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /*start transfer*/
    EXAMPLE_I2C_MASTER.MasterTransmit(I2C_MASTER_SLAVE_ADDR, g_master_txBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");

    EXAMPLE_I2C_MASTER.MasterReceive(I2C_MASTER_SLAVE_ADDR, g_master_rxBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Transfer completed. Check the data.*/
    for (uint32_t i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i])
        {
            PRINTF("\r\nError occurred in the transfer ! \r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of I2C example .\r\n");
    while (1)
    {
    }
}
