/*
 * Copyright  2019 NXP
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
#define EXAMPLE_USART          USART5
#define EXAMPLE_USART_CLK_FREQ CLOCK_GetFlexCommClkFreq(5)

#define EXAMPLE_TRANSFER_SIZE  (16U)
#define EXAMPLE_USART_BAUDRATE (500000U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

/* Initialize the USART as master node in synchronous mode. */
static void EXAMPLE_InitUSART(void);

/* Start synchronous transfer with slave board. */
static void EXAMPLE_StartSyncTransfer(void);

/* Check the data from slave board and print the result. */
static void EXAMPLE_CheckResult(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usart_handle_t g_usartHandle;
uint8_t txBuff[EXAMPLE_TRANSFER_SIZE] = {0};
uint8_t rxBuff[EXAMPLE_TRANSFER_SIZE] = {0};
volatile bool txComplete              = false;
volatile bool rxComplete              = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach 12 MHz clock to FLEXCOMM5. */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM5);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    PRINTF("This is USART synchronous transfer master example.\r\n");
    PRINTF("Master will send data to slave firstly, and then receive data from slave.\r\n");
    PRINTF("Please connect the pins as below shows:\r\n");
    PRINTF("   Master Board          Slave Board    \r\n");
    PRINTF("   USART_TX      -------   USART_RX     \r\n");
    PRINTF("   USART_RX      -------   USART_TX     \r\n");
    PRINTF("   USART_SCLK    -------   USART_SCLK   \r\n");
    PRINTF("   GND           -------   GND          \r\n");

    /* Initialize the USART as master node. */
    EXAMPLE_InitUSART();

    /* Start transfer with slave board. */
    EXAMPLE_StartSyncTransfer();

    /* Check the data and print the result. */
    EXAMPLE_CheckResult();

    /* De-init the USART instance. */
    USART_Deinit(EXAMPLE_USART);

    while (1)
    {
    }
}

/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_USART_TxIdle == status)
    {
        txComplete = true;
    }

    if (kStatus_USART_RxIdle == status)
    {
        rxComplete = true;
    }
}

/* Initialize the USART as master node in synchronous mode. */
static void EXAMPLE_InitUSART(void)
{
    usart_config_t config;

    /*
     * config.baudRate_Bps = 500000U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.enableRx = true;
     * config.enableTx = true;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps         = EXAMPLE_USART_BAUDRATE;
    config.syncMode             = kUSART_SyncModeMaster;
    config.clockPolarity        = kUSART_RxSampleOnFallingEdge;
    config.enableContinuousSCLK = true;
    config.enableRx             = true;
    config.enableTx             = true;

    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);
}

/*
 * Start synchronous transfer with slave board. master will send to slave firstly, and
 * then receive data from slave board.
 */
static void EXAMPLE_StartSyncTransfer(void)
{
    usart_transfer_t xfer;
    uint32_t i = 0U;

    for (i = 0U; i < EXAMPLE_TRANSFER_SIZE; i++)
    {
        txBuff[i] = i % 256;
        rxBuff[i] = 0U;
    }

    USART_TransferCreateHandle(EXAMPLE_USART, &g_usartHandle, USART_UserCallback, NULL);

    /* Send data to slave board. */
    xfer.data     = txBuff;
    xfer.dataSize = EXAMPLE_TRANSFER_SIZE;
    txComplete    = false;
    USART_TransferSendNonBlocking(EXAMPLE_USART, &g_usartHandle, &xfer);

    /* Waitting for the data transfer is completed. */
    while (!txComplete)
    {
    }

    /* Receive data from slave. */
    xfer.data     = rxBuff;
    xfer.dataSize = EXAMPLE_TRANSFER_SIZE;
    rxComplete    = false;
    USART_TransferReceiveNonBlocking(EXAMPLE_USART, &g_usartHandle, &xfer, NULL);

    /* Waitting for the data transfer is completed. */
    while (!rxComplete)
    {
    }

    PRINTF("Transfer Complete. ");
}

/* Check the data from slave board and print the result. */
static void EXAMPLE_CheckResult(void)
{
    uint32_t i        = 0U;
    uint32_t errCount = 0U;

    PRINTF("The data received from slave are: \r\n");

    for (i = 0U; i < EXAMPLE_TRANSFER_SIZE; i++)
    {
        if (rxBuff[i] != txBuff[i])
        {
            errCount++;
        }
        if ((i & 0xF) == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF(" 0x%02X", rxBuff[i]);
    }

    if (errCount == 0U)
    {
        PRINTF("\r\n\r\nData transfer matched!");
    }
    else
    {
        PRINTF("\r\n\r\nData transfer failed!");
    }
}
