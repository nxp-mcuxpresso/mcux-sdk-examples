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
#include "fsl_lpi2c.h"
#include "fsl_lpi2c_cmsis.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_I2C_MASTER            Driver_I2C0
#define EXAMPLE_LPI2C_DMAMUX_BASEADDR DMAMUX
#define EXAMPLE_LPI2C_DMA_BASEADDR    DMA0

#define DMA0_IRQn DMA0_04_IRQn

#define I2C_MASTER_SLAVE_ADDR_7BIT (0x7EU)
#define I2C_DATA_LENGTH            (32) /* MAX is 256 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

AT_NONCACHEABLE_SECTION(uint8_t g_master_txBuff[I2C_DATA_LENGTH]);
AT_NONCACHEABLE_SECTION(uint8_t g_master_rxBuff[I2C_DATA_LENGTH]);
volatile bool g_MasterCompletionFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
uint32_t LPI2C0_GetFreq(void)
{
    return CLOCK_GetIpFreq(kCLOCK_Lpi2c0);
}

static void lpi2c_master_callback(uint32_t event)
{
    switch (event)
    {
        /* The master has sent a stop transition on the bus */
        case ARM_I2C_EVENT_TRANSFER_DONE:
            g_MasterCompletionFlag = true;
            break;

        default:
            break;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0;

    /*Init BOARD*/
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Lpi2c0, kCLOCK_IpSrcFircAsync);

/* DMAMux init and EDMA init */
#if (defined(FSL_FEATURE_SOC_DMAMUX_COUNT) && FSL_FEATURE_SOC_DMAMUX_COUNT)
    DMAMUX_Init(EXAMPLE_LPI2C_DMAMUX_BASEADDR);
#endif
    edma_config_t edmaConfig = {0};
    EDMA_GetDefaultConfig(&edmaConfig);
    EDMA_Init(EXAMPLE_LPI2C_DMA_BASEADDR, &edmaConfig);

#if (defined(FSL_FEATURE_EDMA_HAS_CHANNEL_MUX) && FSL_FEATURE_EDMA_HAS_CHANNEL_MUX)
    EDMA_SetChannelMux(RTE_I2C0_DMA_TX_DMA_BASE, RTE_I2C0_DMA_TX_CH, RTE_I2C0_DMA_TX_PERI_SEL);
    EDMA_SetChannelMux(RTE_I2C0_DMA_RX_DMA_BASE, RTE_I2C0_DMA_RX_CH, RTE_I2C0_DMA_RX_PERI_SEL);
#endif
    /* Initialize the LPI2C master peripheral */
    EXAMPLE_I2C_MASTER.Initialize(lpi2c_master_callback);
    EXAMPLE_I2C_MASTER.PowerControl(ARM_POWER_FULL);

    /* Change the default baudrate configuration */
    EXAMPLE_I2C_MASTER.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);

    PRINTF("\r\nCMSIS LPI2C board2board EDMA example -- Master transfer.\r\n");

    /* Set up i2c master to send data to slave*/
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        g_master_txBuff[i] = i;
    }

    /* Display the data the master will send */
    PRINTF("Master will send data :");
    for (i = 0; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_txBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Start transmitting I2C transfers on the LPI2C master peripheral */
    EXAMPLE_I2C_MASTER.MasterTransmit(I2C_MASTER_SLAVE_ADDR_7BIT, g_master_txBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    PRINTF("Receive sent data from slave :");

    /* Start accepting I2C transfers on the LPI2C master peripheral */
    EXAMPLE_I2C_MASTER.MasterReceive(I2C_MASTER_SLAVE_ADDR_7BIT, g_master_rxBuff, I2C_DATA_LENGTH, false);

    /*wait for master complete*/
    while (!g_MasterCompletionFlag)
    {
    }

    /*  Reset master completion flag to false. */
    g_MasterCompletionFlag = false;

    for (i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (i % 8 == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_master_rxBuff[i]);
    }
    PRINTF("\r\n\r\n");

    /* Transfer completed. Check the data.*/
    for (i = 0U; i < I2C_DATA_LENGTH; i++)
    {
        if (g_master_rxBuff[i] != g_master_txBuff[i])
        {
            PRINTF("\r\nError occurred in the transfer !\r\n");
            break;
        }
    }

    PRINTF("\r\nEnd of LPI2C example .\r\n");
    while (1)
    {
    }
}
