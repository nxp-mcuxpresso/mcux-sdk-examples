/*
 * Copyright  2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_spi.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_SPI_MASTER          SPI0
#define EXAMPLE_CLK_SRC             kCLOCK_MainClk
#define EXAMPLE_SPI_MASTER_CLK_FREQ CLOCK_GetFreq(EXAMPLE_CLK_SRC)
#define EXAMPLE_SPI_MASTER_BAUDRATE 500000U
#define EXAMPLE_SPI_MASTER_SSEL     kSPI_Ssel0Assert

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_MasterCallback(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData);
static void EXAMPLE_SPIMasterInit(void);
static void EXAMPLE_MasterStartTransfer(void);
static void EXAMPLE_TransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t txBuffer[BUFFER_SIZE];
static uint8_t rxBuffer[BUFFER_SIZE];
spi_master_handle_t masterHandle;
static volatile bool masterFinished = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void EXAMPLE_MasterCallback(SPI_Type *base, spi_master_handle_t *handle, status_t status, void *userData)
{
    masterFinished = true;
}

int main(void)
{
    /* Enable clock of uart0. */
    CLOCK_EnableClock(kCLOCK_Uart0);
    /* Ser DIV of uart0. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);

    /* Enable clock of spi0. */
    CLOCK_EnableClock(kCLOCK_Spi0);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("This is SPI interrupt transfer master example.\r\n");
    PRINTF("\r\nMaster start to send data to slave, please make sure the slave has been started!\r\n");

    /* Initialize the SPI master with configuration. */
    EXAMPLE_SPIMasterInit();

    /* Start transfer with slave board. */
    EXAMPLE_MasterStartTransfer();

    /* Check the received data. */
    EXAMPLE_TransferDataCheck();

    /* De-initialize the SPI. */
    SPI_Deinit(EXAMPLE_SPI_MASTER);

    while (1)
    {
    }
}

static void EXAMPLE_SPIMasterInit(void)
{
    spi_master_config_t userConfig;
    uint32_t srcFreq = 0U;

    /* configuration from using SPI_MasterGetDefaultConfig():
     * userConfig->enableLoopback = false;
     * userConfig->enableMaster = true;
     * userConfig->polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig->phase = kSPI_ClockPhaseFirstEdge;
     * userConfig->direction = kSPI_MsbFirst;
     * userConfig->baudRate_Bps = 500000U;
     * userConfig->dataWidth = kSPI_Data8Bits;
     * userConfig->sselNum = kSPI_Ssel0Assert;
     * userConfig->sselPol = kSPI_SpolActiveAllLow;
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps = EXAMPLE_SPI_MASTER_BAUDRATE;
    userConfig.sselNumber   = EXAMPLE_SPI_MASTER_SSEL;
    srcFreq                 = EXAMPLE_SPI_MASTER_CLK_FREQ;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &userConfig, srcFreq);
}

static void EXAMPLE_MasterStartTransfer(void)
{
    spi_transfer_t xfer = {0};
    uint32_t i          = 0U;

    /* Initialize txBuffer and rxBuffer. */
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        txBuffer[i] = i;
        rxBuffer[i] = 0U;
    }

    /* Set transfer parameters for master. */
    xfer.txData      = txBuffer;
    xfer.rxData      = rxBuffer;
    xfer.dataSize    = sizeof(txBuffer);
    xfer.configFlags = kSPI_EndOfTransfer;

    /* Create handle for master. */
    SPI_MasterTransferCreateHandle(EXAMPLE_SPI_MASTER, &masterHandle, EXAMPLE_MasterCallback, NULL);

    /* Start transfer data. */
    SPI_MasterTransferNonBlocking(EXAMPLE_SPI_MASTER, &masterHandle, &xfer);
}
static void EXAMPLE_TransferDataCheck(void)
{
    uint32_t i = 0U, err = 0U;
    /* Waiting for transmission complete. */
    while (masterFinished != true)
    {
    }

    PRINTF("\r\nThe received data are:");
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        /* Print 16 numbers in a line */
        if ((i & 0x0FU) == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("  0x%02X", rxBuffer[i]);
        /* Check if data matched. */
        if (txBuffer[i] != rxBuffer[i])
        {
            err++;
        }
    }

    if (err == 0)
    {
        PRINTF("\r\nMaster interrupt transfer succeed!\r\n");
    }
    else
    {
        PRINTF("\r\nMaster interrupt transfer faild!\r\n");
    }
}
