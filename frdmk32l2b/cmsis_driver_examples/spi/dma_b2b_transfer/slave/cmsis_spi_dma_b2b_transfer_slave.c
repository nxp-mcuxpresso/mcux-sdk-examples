/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_spi_cmsis.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_SLAVE_DMA_MUX_BASEADDR DMAMUX0
#define EXAMPLE_SPI_SLAVE_DMA_BASEADDR     DMA0
#define DRIVER_SLAVE_SPI                   Driver_SPI1
#define TRANSFER_SIZE 64U /* Transfer dataSize */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* SPI user SlaveSignalEvent */
void SPI_SlaveSignalEvent_t(uint32_t event);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t slaveRxData[TRANSFER_SIZE] = {0};
uint8_t slaveTxData[TRANSFER_SIZE] = {0};

volatile bool isTransferCompleted = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t SPI1_GetFreq(void)
{
    return CLOCK_GetBusClkFreq();
}
void SPI_SlaveSignalEvent_t(uint32_t event)
{
    /* user code */
    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("SPI CMSIS driver board to board dma example.\r\n");

    uint32_t i          = 0U;
    uint32_t errorCount = 0U;

    /* DMA Mux init and DMA init */
#if FSL_FEATURE_DMA_MODULE_CHANNEL != FSL_FEATURE_DMAMUX_MODULE_CHANNEL
    DMAMUX_Init(EXAMPLE_SPI_SLAVE_DMA_MUX_TX_BASEADDR);
    DMAMUX_Init(EXAMPLE_SPI_SLAVE_DMA_MUX_RX_BASEADDR);
#else
    DMAMUX_Init(EXAMPLE_SPI_SLAVE_DMA_MUX_BASEADDR);
#endif
    DMA_Init(EXAMPLE_SPI_SLAVE_DMA_BASEADDR);

    /*SPI slave init*/
    DRIVER_SLAVE_SPI.Initialize(SPI_SlaveSignalEvent_t);
    DRIVER_SLAVE_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_SLAVE_SPI.Control(ARM_SPI_MODE_SLAVE, false);

    PRINTF("\r\n Slave example is running...\r\n");

    /* Reset the send and receive buffer */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        slaveRxData[i] = 0U;
        slaveTxData[i] = i % 256;
    }

    isTransferCompleted = false;
    /* Set slave transfer to receive and send data */
    DRIVER_SLAVE_SPI.Transfer(slaveTxData, slaveRxData, TRANSFER_SIZE);
    /* Wait until transfer completed */
    while (!isTransferCompleted)
    {
    }

    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (slaveRxData[i] != i)
        {
            PRINTF("\r\nThe %d number is wrong! It is %d\r\n", i, slaveRxData[i]);
            errorCount++;
        }
    }

    PRINTF("\r\n");
    if (errorCount == 0)
    {
        PRINTF("\r\n Slave receive:");
        for (i = 0; i < TRANSFER_SIZE; i++)
        {
            /* Print 16 numbers in a line */
            if ((i & 0x0FU) == 0U)
            {
                PRINTF("\r\n    ");
            }
            PRINTF(" %02X", slaveRxData[i]);
        }
        PRINTF("\r\n");
        PRINTF("Succeed!\r\n");
    }
    else
    {
        PRINTF("Error occurd in SPI transfer!\r\n");
    }
    while (1)
    {
    }
}
