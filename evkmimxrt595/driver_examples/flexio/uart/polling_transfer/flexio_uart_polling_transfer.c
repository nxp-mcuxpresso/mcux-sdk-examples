/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_flexio_uart.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_FLEXIO_BASE  FLEXIO0
#define FLEXIO_UART_TX_PIN 10U
#define FLEXIO_UART_RX_PIN 11U

/* Select flexio clock source */
#define FLEXIO_CLOCK_SRC (kFRG_to_FLEXIO)
/* Clock divider for flexio clock source */
#define FLEXIO_CLOCK_DIVIDER   (33U)
#define FLEXIO_CLOCK_FREQUENCY CLOCK_GetFlexioClkFreq()

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
FLEXIO_UART_Type uartDev;
uint8_t txbuff[]   = "Flexio uart polling example\r\nBoard will send back received characters\r\n";
uint8_t rxbuff[20] = {0};
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t ch;
    flexio_uart_config_t config;
    status_t result = kStatus_Success;

    const clock_frg_clk_config_t flexio_frg = {17U, kCLOCK_FrgMainClk, 255U, 0U};
    BOARD_InitPins();
    BOARD_BootClockRUN();

    /* Clock setting for Flexio */
    CLOCK_SetFRGClock(&flexio_frg);
    CLOCK_AttachClk(FLEXIO_CLOCK_SRC);
    CLOCK_SetClkDiv(kCLOCK_DivFlexioClk, FLEXIO_CLOCK_DIVIDER);

    RESET_ClearPeripheralReset(kFLEXIO_RST_SHIFT_RSTn);

    /*
     * config.enableUart = true;
     * config.enableInDoze = false;
     * config.enableInDebug = true;
     * config.enableFastAccess = false;
     * config.baudRate_Bps = 115200U;
     * config.bitCountPerChar = kFLEXIO_UART_8BitsPerChar;
     */
    FLEXIO_UART_GetDefaultConfig(&config);
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableUart   = true;

    uartDev.flexioBase      = BOARD_FLEXIO_BASE;
    uartDev.TxPinIndex      = FLEXIO_UART_TX_PIN;
    uartDev.RxPinIndex      = FLEXIO_UART_RX_PIN;
    uartDev.shifterIndex[0] = 0U;
    uartDev.shifterIndex[1] = 1U;
    uartDev.timerIndex[0]   = 0U;
    uartDev.timerIndex[1]   = 1U;

    result = FLEXIO_UART_Init(&uartDev, &config, FLEXIO_CLOCK_FREQUENCY);
    if (result != kStatus_Success)
    {
        return -1;
    }

    FLEXIO_UART_WriteBlocking(&uartDev, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
        FLEXIO_UART_ReadBlocking(&uartDev, &ch, 1);
        FLEXIO_UART_WriteBlocking(&uartDev, &ch, 1);
    }
}
