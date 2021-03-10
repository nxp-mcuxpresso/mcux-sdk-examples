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

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t txbuff[] =
    "Usart polling example.\r\nBoard will send back received characters.\r\nNow, please input any character:\r\n";

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t ch;
    usart_config_t config;

    BOARD_InitPins();
    BOARD_BootClockIRC12M();

    /* Config USART clock for generating a precise baud rate.
     * In this example we need to set the baud rate to 9600.
     * For asynchronous mode (UART mode) the formula is:
     * mainClock_Hz = (BRG + 1) * (1 + (UARTFRGMULT/256)) * (UARTCLKDIV) * (16 * baudrate_Hz.)
     * For this example, The mainClock_Hz is 12,000,000 Hz, we set UARTCLKDIV = 1,
     * We proceed in 2 steps.
     * Step 1: Let UARTFRGMULT = 0, and round (down) to the nearest integer value of BRG for the desired baudrate.
     * Step 2: Plug in the BRG from step 1, and find the nearest integer value of m, (for the FRG fractional part).
     *
     * Step 1 (with UARTFRGMULT = 0)
     * BRG = ((mainClock_Hz) / (16 * baudrate Hz.)) - 1
     *     = (12,000,000/(16 * 9600)) - 1
     *     = 77
     * So let BRG = 77
     * Note: the BRG will be caculated and set by API USART_Init() automatically.
     *
     * Step 2.
     * UARTFRGMULT = 256 * [-1 + {(mainClock Hz.) / (1 * 16 * baudrate_Hz) * (BRG + 1)}]
     *             = 256 * [-1 + {(12,000,000) / (1*16*9600)*(78)}]
     *             = 0.4
     * So, the UARTFRGMULT is 0.
     * Note: Due to all the USART share the same clock source, please using the higheset baud rate
     *       as the main baud rate to set the clock configurture.
     */

    /* Set Div value for USART. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);
    /* Set the UARTFRGMULT register. */
    CLOCK_SetFRGClkMul(0);

    /* Default config by using USART_GetDefaultConfig():
     * config->baudRate_Bps = 9600U;
     * config->parityMode = kUSART_ParityDisabled;
     * config->stopBitCount = kUSART_OneStopBit;
     * config->bitCountPerChar = kUSART_8BitsPerChar;
     * config->loopback = false;
     * config->enableRx = false;
     * config->enableTx = false;
     * config->syncMode = kUSART_SyncModeDisabled;
     */
    USART_GetDefaultConfig(&config);
    config.enableRx     = true;
    config.enableTx     = true;
    config.baudRate_Bps = BOARD_DEBUG_USART_BAUDRATE;

    /* Initialize the USART with configuration. */
    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);

    /* Send data in polling way. */
    USART_WriteBlocking(EXAMPLE_USART, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
        /* Receive a character from USART, this API will wait until one character has been received. */
        USART_ReadBlocking(EXAMPLE_USART, &ch, 1);
        /* Send the received character to the terminal. */
        USART_WriteBlocking(EXAMPLE_USART, &ch, 1);
    }
}
