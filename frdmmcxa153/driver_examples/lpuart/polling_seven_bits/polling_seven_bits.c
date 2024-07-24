/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_clock.h"
#include "fsl_reset.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPUART          LPUART2
#define DEMO_LPUART_CLK_FREQ CLOCK_GetLpuartClkFreq(2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t txbuff[]   = "Lpuart polling example with seven data bits\r\nBoard will send back received characters\r\n";
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
    lpuart_config_t config;

    /* Attach 12 MHz clock to LPUART (debug console) */
    CLOCK_SetClockDiv(kCLOCK_DivLPUART2, 1u);
    CLOCK_AttachClk(kFRO12M_to_LPUART2);

    BOARD_InitPins();
    BOARD_InitBootClocks();

    /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps  = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx      = true;
    config.enableRx      = true;
    config.dataBitsCount = kLPUART_SevenDataBits;
    config.isMsb         = false;

    LPUART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);

    LPUART_WriteBlocking(DEMO_LPUART, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
        LPUART_ReadBlocking(DEMO_LPUART, &ch, 1);
        LPUART_WriteBlocking(DEMO_LPUART, &ch, 1);
    }
}
