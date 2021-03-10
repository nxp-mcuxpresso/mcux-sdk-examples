/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_spi.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER_BASEADDR SPI3
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFlexCommClkFreq(3)
#define EXAMPLE_SPI_MASTER_SSEL     kSPI_Ssel2
#define EXAMPLE_SPI_MASTER_SPOL     kSPI_SpolActiveAllLow
/* Callback function for interrupt transfer. */
static void SPI_MasterInterruptCallback(SPI_Type *base,
                                        spi_master_handle_t *masterHandle,
                                        status_t status,
                                        void *userData);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t txData[BUFFER_SIZE];
static uint8_t rxData[BUFFER_SIZE];

spi_master_handle_t handle;
static volatile bool masterFinished = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void SPI_MasterInterruptCallback(SPI_Type *base,
                                        spi_master_handle_t *masterHandle,
                                        status_t status,
                                        void *userData)
{
    masterFinished = true;
}

int main(void)
{
    uint32_t srcFreq = 0;
    uint32_t i       = 0;
    uint32_t err     = 0U;
    /* Transfer structure for half-duplex. */
    spi_half_duplex_transfer_t xfer = {0};
    spi_master_config_t masterConfig;

    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to SPI3 */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM3);

    /* reset FLEXCOMM for SPI */
    RESET_PeripheralReset(kFC3_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("This is SPI half-duplex interrupt transfer example!\r\n");
    PRINTF("SPI master board will transmit data to slave board first, then receive data from slave board.\r\n");
    PRINTF("To make sure the transfer work successfully, please start the slave board first!\r\n");
    PRINTF("Master will use interrupt way, and slave will use interrupt way, too.\r\n");
    PRINTF("\r\nMaster start to tansfer data...\r\n");

    /* Master config */
    SPI_MasterGetDefaultConfig(&masterConfig);
    masterConfig.sselNum = EXAMPLE_SPI_MASTER_SSEL;
    masterConfig.sselPol = EXAMPLE_SPI_MASTER_SPOL;
    srcFreq              = EXAMPLE_SPI_MASTER_CLK_FREQ;
    SPI_MasterInit(EXAMPLE_SPI_MASTER_BASEADDR, &masterConfig, srcFreq);

    /* Init Buffer*/
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        txData[i] = i;
    }

    /*Start Transfer by polling mode. */
    xfer.txData                = txData;
    xfer.rxData                = rxData;
    xfer.txDataSize            = sizeof(txData);
    xfer.rxDataSize            = sizeof(rxData);
    xfer.isTransmitFirst       = true;
    xfer.isPcsAssertInTransfer = true;
    xfer.configFlags           = kSPI_FrameAssert;

    /* Half-duplex transfer in interrupt way. */
    SPI_MasterTransferCreateHandle(EXAMPLE_SPI_MASTER_BASEADDR, &handle, SPI_MasterInterruptCallback, NULL);
    SPI_MasterHalfDuplexTransferNonBlocking(EXAMPLE_SPI_MASTER_BASEADDR, &handle, &xfer);

    while (!masterFinished)
    {
    }
    PRINTF("The half-duplex transfer in interrupt way is end!\r\n");
    PRINTF("\r\nThe received data are:");
    /*Check if the data is right*/
    err = 0U;
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        /* Print 16 numbers in a line */
        if ((i & 0x0FU) == 0U)
        {
            PRINTF("\r\n    ");
        }
        PRINTF("  0x%02X", rxData[i]);
        /* Check if data matched. */
        if (txData[i] != rxData[i])
        {
            err++;
        }
    }

    if (err == 0)
    {
        PRINTF("\r\nMaster half-duplex interrupt transfer succeed!\r\n");
    }
    else
    {
        PRINTF("\r\nMaster half-duplex interrupt transfer faild!\r\n");
    }

    SPI_Deinit(EXAMPLE_SPI_MASTER_BASEADDR);

    while (1)
    {
    }
}
