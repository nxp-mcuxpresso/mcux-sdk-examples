/*
 * Copyright 2020 NXP
 * All rights reserved.
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
#define DEMO_USART          USART2
#define DEMO_USART_CLK_FREQ CLOCK_GetFreq(kCLOCK_MainClk)
#define DELAY_TIME          100000U
#define TRANSFER_SIZE     256U    /*! Transfer dataSize */
#define TRANSFER_BAUDRATE 115200U /*! Transfer baudrate - 115200 */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t transferRxData[TRANSFER_SIZE] = {0U};
uint8_t transferTxData[TRANSFER_SIZE] = {0U};
usart_handle_t g_usartHandle;
volatile bool isTransferCompleted = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* USART user callback */
void USART_UserCallback(USART_Type *base, usart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_USART_TxIdle == status)
    {
        isTransferCompleted = true;
    }
}
/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i = 0U, errCount = 0U;
    status_t status = 0;
    usart_config_t config;
    usart_transfer_t sendXfer;

    /* Select the main clock as source clock of USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);
    /* Select the main clock as source clock of USART2 (demo instance) */
    CLOCK_Select(kUART2_Clk_From_MainClk);

    BOARD_InitBootPins();
    BOARD_BootClockFRO48M();
    BOARD_InitDebugConsole();
    PRINTF("This is USART hardware flow control example on one board.\r\n");
    PRINTF("This example will send data to itself and will use hardware flow control to avoid the overflow.\r\n");
    PRINTF("Please make sure you make the correct line connection. Basically, the connection is: \r\n");
    PRINTF("      USART_TX    --     USART_RX    \r\n");
    PRINTF("      USART_RTS   --     USART_CTS   \r\n");

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kUSART_ParityDisabled;
     * config.stopBitCount = kUSART_OneStopBit;
     * config.txWatermark = kUSART_TxFifo0;
     * config.rxWatermark = kUSART_RxFifo1;
     * config.enableTx = false;
     * config.enableRx = false;
     * config.enableHardwareFlowControl = false;
     */
    USART_GetDefaultConfig(&config);
    config.baudRate_Bps              = BOARD_DEBUG_USART_BAUDRATE;
    config.enableTx                  = true;
    config.enableRx                  = true;
    config.enableHardwareFlowControl = true;

    USART_Init(DEMO_USART, &config, DEMO_USART_CLK_FREQ);
    USART_TransferCreateHandle(DEMO_USART, &g_usartHandle, USART_UserCallback, NULL);

    /* Set up the transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        transferTxData[i] = i % 256U;
        transferRxData[i] = 0U;
    }

    sendXfer.data     = (uint8_t *)transferTxData;
    sendXfer.dataSize = TRANSFER_SIZE;
    USART_TransferSendNonBlocking(DEMO_USART, &g_usartHandle, &sendXfer);

    /* Delay for some time to let the RTS pin dessart. */
    for (i = 0U; i < DELAY_TIME; i++)
    {
        __NOP();
    }

    status = USART_ReadBlocking(DEMO_USART, transferRxData, TRANSFER_SIZE);
    if (kStatus_Success != status)
    {
        PRINTF(" Error occurred when USART receiving data.\r\n");
    }
    /* Wait for the transmit complete. */
    while (!isTransferCompleted)
    {
    }

    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (transferTxData[i] != transferRxData[i])
        {
            errCount++;
        }
    }
    if (errCount)
    {
        PRINTF("Data not matched! Transfer error.\r\n");
    }
    else
    {
        PRINTF("Data matched! Transfer successfully.\r\n");
    }

    /* Deinit the USART. */
    USART_Deinit(DEMO_USART);

    while (1)
    {
    }
}
