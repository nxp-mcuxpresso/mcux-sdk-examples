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
static void EXAMPLE_SPIMasterInit(void);
static void EXAMPLE_MasterStartTransfer(void);
static void EXAMPLE_TransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t txBuffer[BUFFER_SIZE];
static uint8_t rxBuffer[BUFFER_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    /* Initizlize the hardware(clock/pins configuration/debug console). */
    /* Attach main clock to USART0 (debug console) */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    BOARD_InitBootPins();
    BOARD_BootClockFRO30M();
    BOARD_InitDebugConsole();

    /* Attach main clock to SPI0. */
    CLOCK_Select(kSPI0_Clk_From_MainClk);

    PRINTF("This is SPI polling transfer master example.\r\n");
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
    spi_master_config_t userConfig = {0};
    uint32_t srcFreq               = 0U;
    /* Note: The slave board using interrupt way, slave will spend more time to write data
     *       to TX register, to prevent TX data missing in slave, we will add some delay between
     *       frames and capture data at the second edge, this operation will make the slave
     *       has more time to prapare the data.
     */
    SPI_MasterGetDefaultConfig(&userConfig);
    userConfig.baudRate_Bps           = EXAMPLE_SPI_MASTER_BAUDRATE;
    userConfig.sselNumber             = EXAMPLE_SPI_MASTER_SSEL;
    userConfig.clockPhase             = kSPI_ClockPhaseSecondEdge;
    userConfig.delayConfig.frameDelay = 0xFU;
    srcFreq                           = EXAMPLE_SPI_MASTER_CLK_FREQ;
    SPI_MasterInit(EXAMPLE_SPI_MASTER, &userConfig, srcFreq);
}

static void EXAMPLE_MasterStartTransfer(void)
{
    uint32_t i          = 0U;
    spi_transfer_t xfer = {0};

    /* Init Buffer*/
    for (i = 0; i < BUFFER_SIZE; i++)
    {
        txBuffer[i] = i % 256;
        rxBuffer[i] = 0U;
    }

    /*Start Transfer*/
    xfer.txData      = txBuffer;
    xfer.rxData      = rxBuffer;
    xfer.dataSize    = sizeof(txBuffer);
    xfer.configFlags = kSPI_EndOfTransfer | kSPI_EndOfFrame;
    /* Transfer data in polling mode. */
    SPI_MasterTransferBlocking(EXAMPLE_SPI_MASTER, &xfer);
}

static void EXAMPLE_TransferDataCheck(void)
{
    uint32_t i = 0U, err = 0U;
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
        PRINTF("\r\nMaster polling transfer succeed!\r\n");
    }
    else
    {
        PRINTF("\r\nMaster polling transfer faild!\r\n");
    }
}
