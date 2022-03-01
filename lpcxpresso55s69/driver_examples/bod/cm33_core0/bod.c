/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
#include "fsl_anactrl.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_BOD_IRQn              WDT_BOD_IRQn
#define APP_BOD_IRQHander         WDT_BOD_IRQHandler
#define APP_BOD_THRESHOLD_VOLTAGE "2.0V"

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
    /* set BOD VBAT level to 2.0V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel2000mv, kPOWER_BodHystLevel25mv, false);

    ANACTRL_Init(ANACTRL);
    ANACTRL_ClearInterrupts(ANACTRL, kANACTRL_BodVbatInterruptEnable);
    ANACTRL_EnableInterrupts(ANACTRL, kANACTRL_BodVbatInterruptEnable);
}

void APP_DeinitBod(void)
{
    ANACTRL_ClearInterrupts(ANACTRL, kANACTRL_BodVbatInterruptEnable);
    ANACTRL_DisableInterrupts(ANACTRL, kANACTRL_BodVbatInterruptEnable);
    POWER_DisablePD(kPDRUNCFG_PD_BODVBAT);
}


void APP_BOD_IRQHander(void)
{
    APP_DeinitBod();
    g_BodIntFlag = true;
}

int main(void)
{
    /* attach main clock divide to FLEXCOMM0 (debug console) */
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
