/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_uart.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_UART          UART3
#define DEMO_UART_CLK_FREQ                                                                  \
    CLOCK_GetPllFreq(kCLOCK_SystemPll1Ctrl) / (CLOCK_GetRootPreDivider(kCLOCK_RootUart3)) / \
        (CLOCK_GetRootPostDivider(kCLOCK_RootUart3)) / 10


#define EXAMPLE_ADDRESS 0xFEU
#define TRANSFER_SIZE   8U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* UART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uart_handle_t g_uartHandle;
uint8_t g_txBuffer0[TRANSFER_SIZE] = {0};
uint8_t g_txBuffer1[TRANSFER_SIZE] = {0};
uint8_t g_rxBuffer[TRANSFER_SIZE + 1U] = {0};
volatile bool txOnGoing                = false;
volatile bool rxOnGoing                = false;
volatile bool addressDetected     = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* UART user callback */
void UART_UserCallback(UART_Type *base, uart_handle_t *handle, status_t status, void *userData)
{
    userData = userData;

    if (kStatus_UART_TxIdle == status)
    {
        txOnGoing    = false;
    }

    if (kStatus_UART_RxIdle == status)
    {
        rxOnGoing     = false;
    }
    if (kStatus_UART_RS485SlaveAddressDetected == status)
    {
        addressDetected     = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    status_t status;
    uart_config_t config;
    uint32_t i;

    /* Board specific RDC settings */
    BOARD_RdcInit();

    /* Set UART3 source to SysPLL1 Div10 80MHZ */
    CLOCK_SetRootMux(kCLOCK_RootUart3, kCLOCK_UartRootmuxSysPll1Div10);
    /* Set root clock to 80MHZ/ 1= 80MHZ */
    CLOCK_SetRootDivider(kCLOCK_RootUart3, 1U, 1U);

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();
    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUART_ParityDisabled;
     * config.dataBitsCount = kUART_EightDataBits;
     * config.stopBitCount = kUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 1;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    UART_GetDefaultConfig(&config);
    config.baudRate_Bps    = BOARD_DEBUG_UART_BAUDRATE;
    config.txFifoWatermark = 2;
    config.rxFifoWatermark = 8;
    config.enableTx        = true;
    config.enableRx        = true;

    status = UART_Init(DEMO_UART, &config, DEMO_UART_CLK_FREQ);
    if (kStatus_Success != status)
    {
        return kStatus_Fail;
    }

    /* Configure RS485 slave address. */
    UART_SetMatchAddress(DEMO_UART, EXAMPLE_ADDRESS);
    /* Select Automatic Address Detect mode. */
    UART_SetRS485AddressDetectMode(DEMO_UART, RS485_ADDRESS_DETECT_MODE_AUTOMATIC);
    /* Enable RS-485 Slave Address Detected Interrupt. */
    UART_SetRS485AddressDetectInterrput(DEMO_UART, true);
    /* Set up transfer data */
    UART_Enable9bitRS485Mode(DEMO_UART, true);

    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        g_txBuffer0[i] = 0x10 + i;
    }

    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        g_txBuffer1[i] = 0x80 + i;
    }

    for (i = 0U; i < TRANSFER_SIZE + 1U; i++)
    {
        g_rxBuffer[i] = 0;
    }

    UART_TransferCreateHandle(DEMO_UART, &g_uartHandle, UART_UserCallback, NULL);

    /* Start receiving. */
    uart_transfer_t g_receiveXfer;
    g_receiveXfer.data     = g_rxBuffer;
    g_receiveXfer.dataSize = TRANSFER_SIZE + 1U;
    UART_TransferReceiveNonBlocking(DEMO_UART, &g_uartHandle, &g_receiveXfer, NULL);

    /* First send TRANSFER_SIZE byte of data without addressing itself first, these data should be discarded. */
    PRINTF("  UART will send first piece of data out without addressing itself:\n\r");
    PRINTF("    ");
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        PRINTF("0x%2x  ", g_txBuffer0[i]);
    }
    PRINTF("\r\n\r\n");

    uart_transfer_t g_sendXfer;
    g_sendXfer.data     = g_txBuffer0;
    g_sendXfer.dataSize = TRANSFER_SIZE;
    txOnGoing     = true;
    UART_TransferSendNonBlocking(DEMO_UART, &g_uartHandle, &g_sendXfer);
    /* Waiting for transfer completion. */
    while (txOnGoing)
    {
    }

    /* Address itself. */
    PRINTF("  UART will send second piece of data out with addressing itself:\n\r");
    UART_Send9bitRS485Address(DEMO_UART, EXAMPLE_ADDRESS);

    g_sendXfer.data     = g_txBuffer1;
    g_sendXfer.dataSize = TRANSFER_SIZE;
    txOnGoing = true;
    rxOnGoing = true;
    PRINTF("    Address: 0x%2x : ", EXAMPLE_ADDRESS);

    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        PRINTF("0x%2x  ", g_txBuffer1[i]);
    }
    PRINTF("\r\n\r\n");
    UART_TransferSendNonBlocking(DEMO_UART, &g_uartHandle, &g_sendXfer);

    /* Waiting for RS-485 Slave Address has been detected.*/
    while (!addressDetected)
    {
    }
    PRINTF("  RS-485 Slave Address has been detected.\r\n");

    /* Waiting for transfer completion */
    while (txOnGoing || rxOnGoing)
    {
    }

    bool success = true;
    PRINTF("  UART received data:\n\r");
    PRINTF("  Address: 0x%2x : ", g_rxBuffer[0]);
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        PRINTF("0x%2x  ", g_rxBuffer[i + 1U]);
    }
    PRINTF("\r\n\r\n");

    /* Check if the data is right. */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (g_rxBuffer[i + 1U] != g_txBuffer1[i])
        {
            success = false;
            PRINTF("  Received data does not match!\n\r");
            break;
        }
    }
    if (success)
    {
        PRINTF("  All data matches!\n\r");
    }

    while (1)
    {
    }
}
