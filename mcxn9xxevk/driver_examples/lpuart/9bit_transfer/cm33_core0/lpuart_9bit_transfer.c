/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "fsl_lpuart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPUART          LPUART0
#define DEMO_LPUART_CLK_FREQ CLOCK_GetLPFlexCommClkFreq(0u)
#define TRANSFER_SIZE   16U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* LPUART user callback */
void LPUART_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint16_t g_txBuffer[TRANSFER_SIZE] = {0};
uint16_t g_rxBuffer[TRANSFER_SIZE] = {0};
volatile bool txComplete               = false;
volatile bool rxComplete               = false;
lpuart_handle_t g_lpuartHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
/* LPUART user callback */
void LPUART_UserCallback(LPUART_Type *base, lpuart_handle_t *handle, status_t status, void *userData)
{
    if (kStatus_LPUART_TxIdle == status)
    {
        txComplete = true;
    }

    if (kStatus_LPUART_RxIdle == status)
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
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to FLEXCOMM0 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM0);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF(
        "LPUART 9-bit mode example begins\r\nLPUART is configured with 9bit\r\n");

    /* Initialize lpuart instance. */
    /* lpuartConfig->baudRate_Bps = 115200U;
     * lpuartConfig->parityMode = kLPUART_ParityDisabled;
     * lpuartConfig->dataBitsCount = kLPUART_EightDataBits;
     * lpuartConfig->isMsb = false;
     * lpuartConfig->stopBitCount = kLPUART_OneStopBit;
     * lpuartConfig->txFifoWatermark = 0;
     * lpuartConfig->rxFifoWatermark = 1;
     * lpuartConfig->rxIdleType = kLPUART_IdleTypeStartBit;
     * lpuartConfig->rxIdleConfig = kLPUART_IdleCharacter1;
     * lpuartConfig->enableTx = false;
     * lpuartConfig->enableRx = false;
     */
    lpuart_config_t config;
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200U;
    config.enableTx     = true;
    config.enableRx     = true;
    LPUART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);
    /* Set 9bit data mode*/
    LPUART_Enable9bitMode(DEMO_LPUART, true);

    /* Set up transfer data */
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        g_txBuffer[i] = i | 0x100U ;
    }

    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        g_rxBuffer[i] = 0;
    }

    /* Create lpuart handle. */
    LPUART_TransferCreateHandle(DEMO_LPUART, &g_lpuartHandle, LPUART_UserCallback, NULL);
    LPUART_TransferEnable16Bit(&g_lpuartHandle,true);
    /* Start receiving. */
    lpuart_transfer_t g_receiveXfer;
    g_receiveXfer.data     = (uint8_t *)g_rxBuffer;
    g_receiveXfer.dataSize = TRANSFER_SIZE;
    LPUART_TransferReceiveNonBlocking(DEMO_LPUART, &g_lpuartHandle, &g_receiveXfer, NULL);

    /* First send TRANSFER_SIZE byte of data without addressing itself first, these data should be discarded. */
    PRINTF("LPUART will send data out:\n\r");
    for (i = 0U; i < TRANSFER_SIZE; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_txBuffer[i]);
    }
    PRINTF("\r\n\r\n");
    lpuart_transfer_t g_sendXfer;
    g_sendXfer.data     = (uint8_t *)g_txBuffer;
    g_sendXfer.dataSize = TRANSFER_SIZE;
    LPUART_TransferSendNonBlocking(DEMO_LPUART, &g_lpuartHandle, &g_sendXfer);
   
    /* Waiting for transfer completion */
    while ((txComplete == false) || (rxComplete == false))
    {
    }

    bool success = true;
    PRINTF("LPUART received data:\n\r");
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (i % 8U == 0U)
        {
            PRINTF("\r\n");
        }
        PRINTF("0x%2x  ", g_rxBuffer[i]);
    }
    PRINTF("\r\n\r\n");
    /* Check if the data is right. */
    for (i = 0; i < TRANSFER_SIZE; i++)
    {
        if (g_rxBuffer[i] != g_txBuffer[i])
        {
            success = false;
            PRINTF("Received data i:%d does not match!g_rxBuffer:%x,g_txBuffer:%x\n\r",i,g_rxBuffer[i],g_txBuffer[i]);
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
