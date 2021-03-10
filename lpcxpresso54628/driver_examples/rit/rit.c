/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_rit.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define RIT_IRQ_ID RIT_IRQn
/* Get source clock for RIT driver */
#define RIT_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_CoreSysClk)
#define LED_INIT()       LED1_INIT(LOGIC_LED_ON)
#define LED_TOGGLE()     LED1_TOGGLE()
#define APP_RIT_HANDLER  RIT_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void APP_RIT_HANDLER(void)
{
    RIT_ClearStatusFlags(RIT, kRIT_TimerFlag);
    LED_TOGGLE();
    __DSB();
}
/*!
 * @brief Main function
 */
int main(void)
{
    /* Structure of initialize RIT */
    rit_config_t ritConfig;

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    CLOCK_EnableClock(kCLOCK_Gpio3);

    BOARD_InitPins();
    BOARD_BootClockPLL220M();
    BOARD_InitDebugConsole();

    /* Init LED. */
    LED_INIT();

    /*
     * ritConfig.enableRunInDebug = false;
     */
    RIT_GetDefaultConfig(&ritConfig);

    /* Init rit module */
    RIT_Init(RIT, &ritConfig);

    /* Set timer period for Compare register. */
    RIT_SetTimerCompare(RIT, RIT_SOURCE_CLOCK);

    /* Set to enable the Counter register auto clear to zero when the counter value equals the set period. */
    RIT_SetCountAutoClear(RIT, true);

    PRINTF("RIT Example Start, You will see spcified LED blink at 1s period interval.\r\n");

    /* Start counting */
    RIT_StartTimer(RIT);

    /* Enable at the NVIC */
    EnableIRQ(RIT_IRQ_ID);

    while (1)
    {
    }
}
