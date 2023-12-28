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
#define APP_BOD_IRQn              BOD_IRQn
#define APP_BOD_IRQHander         BOD_IRQHandler
#define APP_BOD_THRESHOLD_VOLTAGE "2.66V"

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
    POWER_DisablePD(kPDRUNCFG_PD_BOD);
    /* software delay 30USs */
    SDK_DelayAtLeastUs(30U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    NVIC_ClearPendingIRQ(APP_BOD_IRQn);
    POWER_SetBodLevel(kBod_ResetLevelReserved, kBod_InterruptLevel2, false);
}

void APP_DeinitBod(void)
{
    POWER_SetBodLevel(kBod_ResetLevelReserved, kBod_InterruptLevelReserved, false);
}

void APP_BOD_IRQHander(void)
{
    APP_DeinitBod();
    g_BodIntFlag = true;
}

int main(void)
{
    /* Attach 12 MHz clock to USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);
    BOARD_InitPins();
    BOARD_BootClockFRO48M();
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
