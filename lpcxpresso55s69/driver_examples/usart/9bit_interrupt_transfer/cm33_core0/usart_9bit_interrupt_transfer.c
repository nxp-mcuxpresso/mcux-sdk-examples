/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_usart.h"

#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_USART          USART1
#define DEMO_USART_CLK_SRC  kCLOCK_Flexcomm0
#define DEMO_USART_CLK_FREQ CLOCK_GetFlexCommClkFreq(0U)

#define EXAMPLE_ADDRESS 0x7EU
#define TRANSFER_SIZE   16U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_txBuffer[TRANSFER_SIZE * 2U] = {0};
uint8_t g_rxBuffer[TRANSFER_SIZE + 1U] = {0};
volatile bool txComplete               = false;
volatile bool rxComplete               = false;
usart_handle_t g_usartHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_USART_TxIdle == status)
    {
        txComplete = true;
    }

    if (kStatus_USART_RxIdle == status)
    {
        rxComplete = true;
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM1);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF(
        "USART 9-bit mode example begins\r\nUSART is configured with address, only data sent to itself after matched "
        "address can be received\r\n");

    /* Initialize usart instance. */
    /*
     * usartConfig->baudRate_Bps = 115200U;
     * usartConfig->parityMode = kUSART_ParityDisabled;
     * usartConfig->stopBitCount = kUSART_OneStopBit;
     * usartConfig->bitCountPerChar = kUSART_8BitsPerChar;
     * usartConfig->loopback = false;
     * usartConfig->enableTx = false;
     * usartConfig->enableRx = false;
     */
    usart_config_t config;
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx     = true;
    config.enableRx     = true;
    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_Enable9bitMode(DEMO_USART, true);
    /* Configure address. */
    USART_SetMatchAddress(DEMO_USART, EXAMPLE_ADDRESS);
    /* Enable match address. */
    USART_EnableMatchAddress(DEMO_USART, true);

    /* Set up transfer data */
    for (i = 0U; i < TRANSFER_SIZE * 2U; i++)
    {
        g_txBuffer[i] = i;
    }

    for (i = 0U; i < TRANSFER_SIZE + 1U; i++)
    {
        g_rxBuffer[i] = 0;
    }

    /* Create usart handle. */
    USART_TransferCreateHandle(DEMO_USART, &g_usartHandle, USART_UserCallback, NULL);
    /* Start receiving. */
    usart_transfer_t g_receiveXfer;
    g_receiveXfer.data     = g_rxBuffer;
    g_receiveXfer.dataSize = TRANSFER_SIZE + 1U;
    USART_TransferReceiveNonBlocking(DEMO_USART, &g_usartHandle, &g_receiveXfer, NULL);

    /* First send TRANSFER_SIZE byte of data without addressing itself first, these data should be discarded. */
    PRINTF("USART will send first piece of data out:\n\r");
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_txBuffer[i]);
    }
    PRINTF("\r\n\r\n");
    usart_transfer_t g_sendXfer;
    g_sendXfer.data     = g_txBuffer;
    g_sendXfer.dataSize = TRANSFER_SIZE;
    USART_TransferSendNonBlocking(DEMO_USART, &g_usartHandle, &g_sendXfer);
    /* Waiting for transfer completion. */
    while (txComplete == false)
    {
    }
    txComplete = false;
    /* Address itself. */
    PRINTF("USART will address itself\n\r");
    USART_SendAddress(DEMO_USART, EXAMPLE_ADDRESS);
    /* Then send the other TRANSFER_SIZE byte of data, these data should be received in g_rxBuffer. */
    PRINTF("USART will send the other piece of data out:\n\r");
    for (i = TRANSFER_SIZE; i < TRANSFER_SIZE * 2U; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_txBuffer[i]);
    }
    PRINTF("\r\n\r\n");
    g_sendXfer.data     = &g_txBuffer[TRANSFER_SIZE];
    g_sendXfer.dataSize = TRANSFER_SIZE;
    USART_TransferSendNonBlocking(DEMO_USART, &g_usartHandle, &g_sendXfer);

    /* Waiting for transfer completion */
    while ((txComplete == false) || (rxComplete == false))
    {
    }

    bool success = true;
    PRINTF("USART received data:\n\r");
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_rxBuffer[i + 1U]);
    }
    PRINTF("\r\n\r\n");
    /* Check if the data is right. */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (g_rxBuffer[i + 1U] != g_txBuffer[i + TRANSFER_SIZE])
        {
            success = false;
            PRINTF("Received data does not match!\n\r");
            break;
        }
    }
    if (success)
    {
        PRINTF("All data matches!\n\r");
    }

    while (1)
    {
    }
}
