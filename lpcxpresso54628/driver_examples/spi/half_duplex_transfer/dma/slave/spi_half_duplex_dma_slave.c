/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_spi_dma.h"
#include "fsl_spi.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_SLAVE_BASEADDR SPI9
#define EXAMPLE_SPI_SLAVE_SPOL     kSPI_SpolActiveAllLow

#define EXAMPLE_SPI_SLAVE_DMA_BASEADDR DMA0
#define EXAMPLE_SPI_SLAVE_RX_CHANNEL   22
#define EXAMPLE_SPI_SLAVE_TX_CHANNEL   23
/* The slave will receive 64 bytes from master, and then send them back.
 * slave will do this operation in one transimission, so the transfer data size is 128,
 * and slave will store the received data from the address of dataBuff[64] in the
 * first half transimission. In the second half transimission, slave will send them
 * back to the master board.
 */
#define BUFFER_SIZE        (192)
#define RX_BUFFER_INDEX    (64)
#define TRANSFER_DATA_SIZE (128)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void SPI_SlaveUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t dataBuff[BUFFER_SIZE] = {0};

dma_handle_t slaveTxHandle;
dma_handle_t slaveRxHandle;
spi_dma_handle_t slaveHandle;

static volatile bool slaveFinished = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
void SPI_SlaveUserCallback(SPI_Type *base, spi_dma_handle_t *handle, status_t status, void *userData)
{
    slaveFinished = true;
}

int main(void)
{
    uint32_t i          = 0;
    spi_transfer_t xfer = {0};
    spi_slave_config_t slaveConfig;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI9 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM9);

    BOARD_InitPins();
    BOARD_BootClockPLL220M();
    BOARD_InitDebugConsole();
    PRINTF("\n\rThis is slave example for SPI half-duplex DMA transfer.");
    PRINTF("\n\rSlave is working....\r\n");

    /* DMA init */
    DMA_Init(EXAMPLE_SPI_SLAVE_DMA_BASEADDR);

    /* Slave config */
    SPI_SlaveGetDefaultConfig(&slaveConfig);
    slaveConfig.sselPol = EXAMPLE_SPI_SLAVE_SPOL;
    SPI_SlaveInit(EXAMPLE_SPI_SLAVE_BASEADDR, &slaveConfig);

    DMA_EnableChannel(EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_TX_CHANNEL);
    DMA_EnableChannel(EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_RX_CHANNEL);
    DMA_SetChannelPriority(EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_TX_CHANNEL, kDMA_ChannelPriority0);
    DMA_SetChannelPriority(EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_RX_CHANNEL, kDMA_ChannelPriority1);
    DMA_CreateHandle(&slaveTxHandle, EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_TX_CHANNEL);
    DMA_CreateHandle(&slaveRxHandle, EXAMPLE_SPI_SLAVE_DMA_BASEADDR, EXAMPLE_SPI_SLAVE_RX_CHANNEL);

    SPI_SlaveTransferCreateHandleDMA(EXAMPLE_SPI_SLAVE_BASEADDR, &slaveHandle, SPI_SlaveUserCallback, NULL,
                                     &slaveTxHandle, &slaveRxHandle);

    xfer.txData   = dataBuff;
    xfer.rxData   = &dataBuff[RX_BUFFER_INDEX];
    xfer.dataSize = TRANSFER_DATA_SIZE;
    SPI_SlaveTransferDMA(EXAMPLE_SPI_SLAVE_BASEADDR, &slaveHandle, &xfer);

    /* Wait for the recieve complete. */
    while (slaveFinished != true)
    {
    }

    PRINTF("\r\nThe received data are:");
    for (i = 0; i < BUFFER_SIZE / 3; i++)
    {
        /* Print 16 data in a line. */
        if ((i & 0x0F) == 0)
        {
            PRINTF("\r\n");
        }
        PRINTF("  0x%02X", dataBuff[i + RX_BUFFER_INDEX]);
    }

    PRINTF("\r\n\r\nSlave transfer completed.");

    /* Stop the transfer. */
    DMA_Deinit(EXAMPLE_SPI_SLAVE_DMA_BASEADDR);
    SPI_Deinit(EXAMPLE_SPI_SLAVE_BASEADDR);

    while (1)
    {
    }
}
