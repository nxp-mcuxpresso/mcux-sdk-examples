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

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_USART          USART0
#define EXAMPLE_USART_CLK_SRC  kCLOCK_MainClk
#define EXAMPLE_USART_CLK_FREQ CLOCK_GetFreq(EXAMPLE_USART_CLK_SRC)

#define ECHO_BUFFER_LENGTH 8

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_USARTUserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);
static void EXAMPLE_USARTInit(void);
static void EXAMPLE_USARTPrepareTransfer(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usart_handle_t g_usartHandle;
uint8_t g_tipString[] =
    "Usart interrupt transfer example.\r\nBoard receives 8 characters then sends them out.\r\nNow please input:\r\n";

uint8_t g_txBuffer[ECHO_BUFFER_LENGTH] = {0};
uint8_t g_rxBuffer[ECHO_BUFFER_LENGTH] = {0};
volatile bool rxBufferEmpty            = true;
volatile bool txBufferFull             = false;
volatile bool txOnGoing                = false;
volatile bool rxOnGoing                = false;

usart_transfer_t xfer;
usart_transfer_t sendXfer;
usart_transfer_t receiveXfer;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* USART user callback */
static void EXAMPLE_USARTUserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status)
    {
        txBufferFull = false;
        txOnGoing    = false;
    }

    if (kStatus_USART_RxIdle == status)
    {
        rxBufferEmpty = false;
        rxOnGoing     = false;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize the pins. */
    BOARD_InitBootPins();

    /* Enable clock to 60MHz. */
    BOARD_BootClockFRO48M();

    /* Select the main clock as source clock of USART0. */
    CLOCK_Select(kUART0_Clk_From_MainClk);

    /* Initialize USART with configuration. */
    EXAMPLE_USARTInit();

    /* Prepare for nonblocking transfer. */
    EXAMPLE_USARTPrepareTransfer();

    /* Set flag for waiting the transfer complete. */
    txOnGoing = true;

    /* Send g_tipString out. */
    USART_TransferSendNonBlocking(EXAMPLE_USART, &g_usartHandle, &xfer);

    /* Waiting for data sending complete. */
    while (txOnGoing)
    {
    }

    while (1)
    {
        /* If RX is idle and g_rxBuffer is empty, start to read data to g_rxBuffer. */
        if ((!rxOnGoing) && rxBufferEmpty)
        {
            rxOnGoing = true;
            USART_TransferReceiveNonBlocking(EXAMPLE_USART, &g_usartHandle, &receiveXfer, NULL);
        }

        /* If TX is idle and g_txBuffer is full, start to send data. */
        if ((!txOnGoing) && txBufferFull)
        {
            txOnGoing = true;
            USART_TransferSendNonBlocking(EXAMPLE_USART, &g_usartHandle, &sendXfer);
        }

        /* If g_txBuffer is empty and g_rxBuffer is full, copy g_rxBuffer to g_txBuffer. */
        if ((!rxBufferEmpty) && (!txBufferFull))
        {
            memcpy(g_txBuffer, g_rxBuffer, ECHO_BUFFER_LENGTH);
            rxBufferEmpty = true;
            txBufferFull  = true;
        }
    }
}

static void EXAMPLE_USARTInit(void)
{
    usart_config_t config;

    /* Default config by using USART_GetDefaultConfig():
     * config.baudRate_Bps = 9600U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.bitCountPerChar = kUSART_8BitsPerChar;
     * config.loopback = false;
     * config.enableRx = false;
     * config.enableTx = false;
     * config.syncMode = kUSART_SyncModeDisabled;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_USART_BAUDRATE;
    config.enableRx     = true;
    config.enableTx     = true;

    /* Initialize the USART with configuration. */
    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);
}

static void EXAMPLE_USARTPrepareTransfer(void)
{
    /* Create USART handle, this API will initialize the g_usartHandle and install the callback function. */
    USART_TransferCreateHandle(EXAMPLE_USART, &g_usartHandle, EXAMPLE_USARTUserCallback, NULL);

    /* Set the xfer parameter to send tip info to the terminal. */
    xfer.data     = g_tipString;
    xfer.dataSize = sizeof(g_tipString) - 1;

    /* Set xfer parameters for sending data. */
    sendXfer.data     = g_txBuffer;
    sendXfer.dataSize = sizeof(g_txBuffer);

    /* Set xfers parameters for receiving data. */
    receiveXfer.data     = g_rxBuffer;
    receiveXfer.dataSize = sizeof(g_rxBuffer);
}
