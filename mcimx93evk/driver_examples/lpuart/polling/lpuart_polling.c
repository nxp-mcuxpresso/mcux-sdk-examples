/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpuart.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Get source clock for LPIT driver */
#define DEMO_LPUART          LPUART2
#define LPUART_CLOCK_ROOT    kCLOCK_Root_Lpuart2
#define LPUART_CLOCK_GATE    kCLOCK_Lpuart2
#define DEMO_LPUART_CLK_FREQ CLOCK_GetIpFreq(LPUART_CLOCK_ROOT)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t txbuff[]   = "Lpuart polling example\r\nBoard will send back received characters\r\n";
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

    /* clang-format off */

    const clock_root_config_t lpuartClkCfg = {
        .clockOff = false,
	.mux = 0,
	.div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();

    CLOCK_SetRootClock(LPUART_CLOCK_ROOT, &lpuartClkCfg);
    CLOCK_EnableClock(LPUART_CLOCK_GATE);

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
    config.baudRate_Bps = BOARD_DEBUG_UART_BAUDRATE;
    config.enableTx     = true;
    config.enableRx     = true;

    LPUART_Init(DEMO_LPUART, &config, DEMO_LPUART_CLK_FREQ);

    LPUART_WriteBlocking(DEMO_LPUART, txbuff, sizeof(txbuff) - 1);

    while (1)
    {
        LPUART_ReadBlocking(DEMO_LPUART, &ch, 1);
        LPUART_WriteBlocking(DEMO_LPUART, &ch, 1);
    }
}
