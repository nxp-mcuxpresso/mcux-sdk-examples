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

    /* Initialize the pins. */
    BOARD_InitPins();
    /* Enable clock to 30MHz. */
    BOARD_BootClockFRO30M();
    /* Select the main clock as source clock of USART0. */
    CLOCK_Select(kUART0_Clk_From_MainClk);

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
