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
#define EXAMPLE_USART_IRQn     USART0_IRQn

/*! @brief Buffer size (Unit: Byte). */
#define RX_BUFFER_SIZE 32
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void EXAMPLE_USARTInit(void);
static void EXAMPLE_USARTSendToTerminal(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_demoInfo[] = "Usart functional API interrupt example.\r\nBoard receives characters then sends them out.\r\n";
uint8_t g_promptInfo[]   = "Please input characters and press the Enter key to finish input:\r\n";
uint8_t g_logInfo[]      = "The received characters are:";
uint8_t g_overFlowInfo[] = "\r\nYou have input too many characters, Please try again!\r\n";

uint8_t rxBuffer[RX_BUFFER_SIZE];
volatile uint8_t rxDataCounter   = 0U;
volatile uint8_t txDataCounter   = 0U;
volatile bool rxNewLineDetected  = false;
volatile bool rxDataOverFlowFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void USART0_IRQHandler(void)
{
    uint8_t data;

    /* If TX is ready to send, and the `Enter` key is detected, send the received characters. */
    if (kUSART_TxReady & USART_GetStatusFlags(EXAMPLE_USART) && rxNewLineDetected)
    {
        USART_WriteByte(EXAMPLE_USART, rxBuffer[txDataCounter++]);

        if (txDataCounter == rxDataCounter)
        {
            /* Disable TX ready interrupt. */
            USART_DisableInterrupts(EXAMPLE_USART, kUSART_TxReadyInterruptEnable);
        }
    }

    /* If this Rx read flag is set, read data to buffer. */
    if (kUSART_RxReady & USART_GetStatusFlags(EXAMPLE_USART))
    {
        data                      = USART_ReadByte(EXAMPLE_USART);
        rxBuffer[rxDataCounter++] = data;
        /* Wait for TX is ready, send back the character. */
        while (!(kUSART_TxReady & USART_GetStatusFlags(EXAMPLE_USART)))
        {
        }
        USART_WriteByte(EXAMPLE_USART, data);
        /* Store the received character to rx buffer. */
        if (0x0D == data)
        {
            rxBuffer[rxDataCounter++] = 0x0A;
            /* Wait for TX is ready, send the character. */
            while (!(kUSART_TxReady & USART_GetStatusFlags(EXAMPLE_USART)))
            {
            }
            USART_WriteByte(EXAMPLE_USART, 0x0A);
            /* Set the rxNewLineDetected to true, and start a new line. */
            rxNewLineDetected = true;
        }
        else
        {
            /* If the data length is larger then the buffer size, set the rxDataOverFlowFlag.
             * The last 2 bytes of rx buffer is 0x0D and 0x0A.
             */
            if (rxDataCounter == RX_BUFFER_SIZE - 1)
            {
                rxDataOverFlowFlag = true;
            }
        }
    }
    __DSB();
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize the pins. */
    BOARD_InitPins();

    /* Enable clock to 12MHz. */
    BOARD_BootClockIRC12M();

    /* Select the main clock as source clock of USART0. */
    CLOCK_Select(kMAINCLK_From_Irc);

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
     * UARTFRGMULT = 256 * [-1 + {(MainClock Hz.) / (1 * 16 * baudrate_Hz) * (BRG + 1)}]
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

    /* Initialize the USART instance with configuration, and enable receive ready interrupt. */
    EXAMPLE_USARTInit();

    while (1)
    {
        /* If the Enter key was pressed, send the received characters out to the terminal, and start a new line.*/
        if (true == rxNewLineDetected)
        {
            EXAMPLE_USARTSendToTerminal();
        }
        /* If too many characters were received, send prompt info to terminal. */
        if (true == rxDataOverFlowFlag)
        {
            /* Send the overflow info to the PC terminal. */
            USART_WriteBlocking(EXAMPLE_USART, g_overFlowInfo,
                                (sizeof(g_overFlowInfo) / sizeof(g_overFlowInfo[0])) - 1);

            /* Reset rxDataOverFlowFlag and rxDataCounter for next loop. */
            rxDataOverFlowFlag = false;
            rxDataCounter      = 0U;
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
    config.enableRx     = true;
    config.enableTx     = true;
    config.baudRate_Bps = BOARD_DEBUG_USART_BAUDRATE;

    /* Initialize the USART with configuration. */
    USART_Init(EXAMPLE_USART, &config, EXAMPLE_USART_CLK_FREQ);

    /* Send demo info and prompt info out to terminal in blocking way. */
    USART_WriteBlocking(EXAMPLE_USART, g_demoInfo, (sizeof(g_demoInfo) / sizeof(g_demoInfo[0])) - 1);
    USART_WriteBlocking(EXAMPLE_USART, g_promptInfo, (sizeof(g_promptInfo) / sizeof(g_promptInfo[0])) - 1);

    /* Enable USART RX ready interrupt. */
    USART_EnableInterrupts(EXAMPLE_USART, kUSART_RxReadyInterruptEnable);
    EnableIRQ(EXAMPLE_USART_IRQn);
}

static void EXAMPLE_USARTSendToTerminal(void)
{
    /* Send log info to terminal in polling-mode. */
    USART_WriteBlocking(EXAMPLE_USART, g_logInfo, (sizeof(g_logInfo) / sizeof(g_logInfo[0])) - 1);

    /* Send back the received characters to terminal in interrupt-mode. */
    USART_EnableInterrupts(EXAMPLE_USART, kUSART_TxReadyInterruptEnable);

    /* Waitting for the Tx complete. */
    while (txDataCounter != rxDataCounter)
    {
    }
    /* Reset the txDataCounter, rxDataCounter and rxNewLineDetected for next loop. */
    txDataCounter     = 0U;
    rxDataCounter     = 0U;
    rxNewLineDetected = false;

    /* Send prompt info to terminal in polling-mode. */
    USART_WriteBlocking(EXAMPLE_USART, g_promptInfo, (sizeof(g_promptInfo) / sizeof(g_promptInfo[0])) - 1);
}
