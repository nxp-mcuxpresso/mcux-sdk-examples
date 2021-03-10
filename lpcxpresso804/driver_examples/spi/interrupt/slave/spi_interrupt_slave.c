/*
 * Copyright  2017 NXP
 * All rights reserved.
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
#define EXAMPLE_SPI_SLAVE     SPI0
#define EXAMPLE_SPI_SLAVE_IRQ SPI0_IRQn

#define SPI_SLAVE_IRQHandler SPI0_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_SlaveInit(void);
static void EXAMPLE_SlaveStartTransfer(void);
static void EXAMPLE_TransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define BUFFER_SIZE (64)
static uint8_t txBuffer[BUFFER_SIZE];
static uint8_t rxBuffer[BUFFER_SIZE];
static uint32_t txIndex            = BUFFER_SIZE;
static uint32_t rxIndex            = BUFFER_SIZE;
static volatile bool slaveFinished = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void SPI_SLAVE_IRQHandler(void)
{
    if ((SPI_GetStatusFlags(EXAMPLE_SPI_SLAVE) & kSPI_RxReadyFlag))
    {
        rxBuffer[BUFFER_SIZE - rxIndex] = SPI_ReadData(EXAMPLE_SPI_SLAVE);
        rxIndex--;
    }
    if ((SPI_GetStatusFlags(EXAMPLE_SPI_SLAVE) & kSPI_TxReadyFlag) && (txIndex != 0U))
    {
        SPI_WriteData(EXAMPLE_SPI_SLAVE, (uint16_t)(txBuffer[BUFFER_SIZE - txIndex]));
        txIndex--;
    }
    if ((txIndex == 0U) && (rxIndex == 0U))
    {
        slaveFinished = true;
        SPI_DisableInterrupts(EXAMPLE_SPI_SLAVE, kSPI_RxReadyInterruptEnable);
    }
    __DSB();
}

int main(void)
{
    /* Initialize the boards */
    /* Attach main clock to USART0 (debug console) */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Attach main clock to SPI0. */
    CLOCK_Select(kSPI0_Clk_From_MainClk);

    PRINTF("This is SPI interrupt functional slave example.\n\r");
    PRINTF("\n\rSlave is working....\n\r");

    /* Initialize the slave SPI with configuration. */
    EXAMPLE_SlaveInit();

    /* Slave start transfer with master. */
    EXAMPLE_SlaveStartTransfer();

    /* Check transfer data. */
    EXAMPLE_TransferDataCheck();

    /* De-initialize the slave SPI. */
    SPI_Deinit(EXAMPLE_SPI_SLAVE);

    while (1)
    {
    }
}

static void EXAMPLE_SlaveInit(void)
{
    spi_slave_config_t slaveConfig = {0};

    /* Default configuration for slave:
     * userConfig.enableSlave = true;
     * userConfig.polarity = kSPI_ClockPolarityActiveHigh;
     * userConfig.phase = kSPI_ClockPhaseFirstEdge;
     * userConfig.direction = kSPI_MsbFirst;
     * userConfig.dataWidth = kSPI_Data8Bits;
     * userConfig.sselPol = kSPI_SpolActiveAllLow;
     */
    SPI_SlaveGetDefaultConfig(&slaveConfig);
    SPI_SlaveInit(EXAMPLE_SPI_SLAVE, &slaveConfig);
}

static void EXAMPLE_SlaveStartTransfer(void)
{
    uint32_t i = 0U;

    /* Init source buffer */
    for (i = 0U; i < BUFFER_SIZE; i++)
    {
        txBuffer[i] = i;
        rxBuffer[i] = 0U;
    }

    /* Write data to TXDAT register to trigger receive interrupt. */
    SPI_WriteData(EXAMPLE_SPI_SLAVE, txBuffer[BUFFER_SIZE - (txIndex--)]);
    /* Enable SPI RX ready interrupt. */
    EnableIRQ(EXAMPLE_SPI_SLAVE_IRQ);
    SPI_EnableInterrupts(EXAMPLE_SPI_SLAVE, kSPI_RxReadyInterruptEnable);
}

static void EXAMPLE_TransferDataCheck(void)
{
    uint32_t i = 0U, err = 0U;
    /* Waiting for the transmission complete. */
    while (slaveFinished != true)
    {
    }

    PRINTF("\n\rThe received data are:");
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
        PRINTF("\n\rSlave transfer succeed!\n\r");
    }
    else
    {
        PRINTF("\n\rSlave transfer faild!\n\r");
    }
}
