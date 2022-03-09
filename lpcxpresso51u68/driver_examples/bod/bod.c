/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_BOD_IRQn              WDT_BOD_IRQn
#define APP_BOD_IRQHander         WDT_BOD_IRQHandler
#define APP_BOD_THRESHOLD_VOLTAGE "3.05V"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitBod(void);
void APP_DeinitBod(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool g_BodIntFlag = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_InitBod(void)
{
    power_bod_config_t config;

    /*
     *  bodConfig->enableReset = true;
     *  bodConfig->resetLevel = kBod_ResetLevel0;
     *  bodConfig->enableInterrupt = false;
     *  bodConfig->interruptLevel = kBod_InterruptLevel0;
     */
    POWER_GetDefaultBodConfig(&config);
    config.enableReset     = false;
    config.enableInterrupt = true;
    config.interruptLevel  = kBod_InterruptLevel3;

    POWER_InitBod(&config);
    POWER_DisablePD(kPDRUNCFG_PD_BOD_INTR);
}

void APP_DeinitBod(void)
{
    POWER_DeinitBod();
    NVIC_DisableIRQ(WDT_BOD_IRQn);
}

void APP_BOD_IRQHander(void)
{
    APP_DeinitBod();
    g_BodIntFlag = true;
}

int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nBOD INTERRUPT EXAMPLE.\r\n");

    APP_InitBod();
    NVIC_EnableIRQ(APP_BOD_IRQn);

    PRINTF("Please adjust input voltage low than %s to trigger BOD interrupt.\r\n", APP_BOD_THRESHOLD_VOLTAGE);
    while (!g_BodIntFlag)
    {
    }
    g_BodIntFlag = false;
    PRINTF("\r\nBOD interrupt occurred, input voltage is low than %s.\r\n", APP_BOD_THRESHOLD_VOLTAGE);
    while (1)
    {
    }
}
