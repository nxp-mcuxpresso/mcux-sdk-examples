/*
 * Copyright  2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_usart.h"
#include "fsl_debug_console.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USART_SLAVE_BASEADDR USART1
#define USART_SLAVE_CLK_SRC  kCLOCK_MainClk
#define USART_SLAVE_CLK_FREQ CLOCK_GetFreq(USART_SLAVE_CLK_SRC)

#define BUFFER_SIZE         16
#define DEMO_USART_BAUDRATE 9600
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);
static void EXAMPLE_USARTSlaveInit(void);
static void EXAMPLE_USARTSlavePrepareTransfer(void);
static void EXAMPLE_USARTTransferDataCheck(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usart_handle_t g_usartSlaveHandle;
usart_transfer_t slaveXfer;

uint8_t g_txBuffer[BUFFER_SIZE] = {0};
uint8_t g_rxBuffer[BUFFER_SIZE] = {0};

volatile bool isSlaveCompleted = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
static void EXAMPLE_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    isSlaveCompleted = true;
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Select clock to USART0 (debug console) */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    /* Select the slave clock as source clock of USART1. */
    CLOCK_Select(kUART1_Clk_From_MainClk);

    /* Print information to the terminal. */
    PRINTF("USART synchronous transfer example.\r\n");
    PRINTF("Slave is working...\r\n");

    /* Initialize the slave USART. */
    EXAMPLE_USARTSlaveInit();

    /* Prepare transactional transfer for slave. */
    EXAMPLE_USARTSlavePrepareTransfer();

    isSlaveCompleted = false;

    /* Firstly, start slave receiving in non-blocking mode. */
    USART_TransferReceiveNonBlocking(USART_SLAVE_BASEADDR, &g_usartSlaveHandle, &slaveXfer, NULL);

    /* Wait transfer complete. */
    while (!isSlaveCompleted)
    {
    }

    /* Check if the received data matched. */
    EXAMPLE_USARTTransferDataCheck();

    /* Deinit the slave USART instances, */
    USART_Deinit(USART_SLAVE_BASEADDR);

    while (1)
    {
    }
}

static void EXAMPLE_USARTSlaveInit(void)
{
    usart_config_t slaveConfig;

    /* Get slave default configuration. */
    USART_GetDefaultConfig(&slaveConfig);
    slaveConfig.baudRate_Bps = 0U;
    slaveConfig.enableRx     = true;
    slaveConfig.syncMode     = kUSART_SyncModeSlave;

    /* Initialize the slave USART. */
    USART_Init(USART_SLAVE_BASEADDR, &slaveConfig, USART_SLAVE_CLK_FREQ);
}

static void EXAMPLE_USARTSlavePrepareTransfer(void)
{
    uint32_t i = 0U;

    /* Create USART handle, this API will initialize the g_usartSlaveHandle and install the callback function. */
    USART_TransferCreateHandle(USART_SLAVE_BASEADDR, &g_usartSlaveHandle, EXAMPLE_UserCallback, NULL);

    /* Initialize the g_txBuffer and g_rxBuffer. */
    for (; i < BUFFER_SIZE; i++)
    {
        g_txBuffer[i] = i % 256;
        g_rxBuffer[i] = 0U;
    }

    /* Start slave USART to receive data. */
    slaveXfer.data     = g_rxBuffer;
    slaveXfer.dataSize = BUFFER_SIZE;
}

static void EXAMPLE_USARTTransferDataCheck(void)
{
    uint8_t i        = 0U;
    uint8_t errCount = 0U;

    for (i = 0; i < BUFFER_SIZE; i++)
    {
        if (g_txBuffer[i] != g_rxBuffer[i])
        {
            errCount++;
        }
    }
    if (errCount)
    {
        PRINTF("Error occurred in transfer.\r\n");
    }
    else
    {
        PRINTF("All data matched, data transfer successfully!\r\n");
    }
}
