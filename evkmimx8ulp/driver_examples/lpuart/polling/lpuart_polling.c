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

#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPUART          LPUART1
#define DEMO_LPUART_CLK_FREQ CLOCK_GetIpFreq(kCLOCK_Lpuart1)

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

    BOARD_InitPins();
    BOARD_BootClockRUN();
    CLOCK_SetIpSrc(kCLOCK_Lpuart1, kCLOCK_Pcc1BusIpSrcCm33Bus);
    RESET_PeripheralReset(kRESET_Lpuart1);

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

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
