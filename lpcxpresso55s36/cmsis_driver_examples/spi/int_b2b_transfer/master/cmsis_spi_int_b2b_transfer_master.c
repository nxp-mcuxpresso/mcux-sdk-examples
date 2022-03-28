/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_spi_cmsis.h"
#include "pin_mux.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DRIVER_MASTER_SPI Driver_SPI2

#define TRANSFER_SIZE     64U     /* Transfer dataSize */
#define TRANSFER_BAUDRATE 500000U /* Transfer baudrate - 500k */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* SPI user SignalEvent */
void SPI_MasterSignalEvent_t(uint32_t event);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t masterRxData[TRANSFER_SIZE] = {0};
uint8_t masterTxData[TRANSFER_SIZE] = {0};

volatile bool isTransferCompleted = false;
volatile bool isMasterOnTransmit  = false;
volatile bool isMasterOnReceive   = false;
/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t SPI2_GetFreq(void)
{
    return CLOCK_GetFlexCommClkFreq(2U);
}
void SPI_MasterSignalEvent_t(uint32_t event)
{
    if (true == isMasterOnReceive)
    {
        PRINTF("This is SPI_MasterSignalEvent_t\r\n");
        PRINTF("Master receive data from slave has completed!\r\n");
        isMasterOnReceive = false;
    }
    if (true == isMasterOnTransmit)
    {
        PRINTF("This is SPI_MasterSignalEvent_t\r\n");
        PRINTF("Master transmit data to slave has completed!\r\n");
        isMasterOnTransmit = false;
    }
    isTransferCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to FLEXCOMM2 (SPI2 master) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom2Clk, 1u, true);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC2_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("SPI CMSIS driver board to board interrupt example.\r\n");
    PRINTF("This example use one board as master and another as slave.\r\n");
    PRINTF("Master and slave uses interrupt way. Slave should start first. \r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
    PRINTF("SPI_master -- SPI_slave   \r\n");
    PRINTF("   CLK      --    CLK  \r\n");
    PRINTF("   PCS      --    PCS \r\n");
    PRINTF("   MOSI     --    MOSI \r\n");
    PRINTF("   MISO     --    MISO \r\n");
    PRINTF("   GND      --    GND \r\n");

    uint32_t errorCount;
    uint32_t i;

    /*SPI master init*/
    DRIVER_MASTER_SPI.Initialize(SPI_MasterSignalEvent_t);
    DRIVER_MASTER_SPI.PowerControl(ARM_POWER_FULL);
    DRIVER_MASTER_SPI.Control(ARM_SPI_MODE_MASTER, TRANSFER_BAUDRATE);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        masterTxData[i] = i % 256U;
        masterRxData[i] = 0U;
    }

    /* Print out transmit buffer */
    PRINTF("\r\n Master transmit:\r\n");
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        /* Print 16 numbers in a line */
        if ((i & 0x0FU) == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF(" %02X", masterTxData[i]);
    }
    PRINTF("\r\n");

    isTransferCompleted = false;
    /* Start master transfer, send data to slave */
    DRIVER_MASTER_SPI.Transfer(masterTxData, masterRxData, TRANSFER_SIZE);
    /* Wait until transfer completed */
    while (!isTransferCompleted)
    {
    }

    errorCount = 0;
    /* Check if the data is right */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (masterRxData[i] != i)
        {
            errorCount++;
        }
    }

    if (errorCount == 0)
    {
        PRINTF(" \r\nSPI transfer all data matched! \r\n");
        /* Print out receive buffer */
        PRINTF("\r\n Master received:\r\n");
        for (i = 0; i < TRANSFER_SIZE; i++)
        {
            /* Print 16 numbers in a line */
            if ((i & 0x0FU) == 0U)
            {
                PRINTF("\r\n");
            }
            PRINTF(" %02X", masterRxData[i]);
        }
        PRINTF("\r\n");
    }
    else
    {
        PRINTF(" \r\nError occurred in SPI transfer ! \r\n");
    }
    while (1)
    {
    }
}
