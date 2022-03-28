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
#include "fsl_mrt.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_LED_INIT   LED_RED_INIT(LOGIC_LED_OFF);
#define APP_LED_ON     (LED_RED_ON());
#define APP_LED_TOGGLE (LED_RED_TOGGLE());
#define MRT_CLK_FREQ   CLOCK_GetFreq(kCLOCK_BusClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static volatile bool mrtIsrFlag          = false;
static volatile bool mrtEnableCount      = false;
static volatile uint32_t mrtCountValue   = 0;
static volatile uint32_t mrtDividerValue = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
void MRT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
    if (mrtEnableCount == true)
    {
        mrtCountValue++;
        if (mrtCountValue == (1 << mrtDividerValue))
        {
            mrtIsrFlag = true;
        }
    }
    else
    {
        mrtIsrFlag = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t mrt_clock;

    /* Structure of initialize MRT */
    mrt_config_t mrtConfig;

    /* Board pin, clock, debug console init */
    /* enable clock for GPIO; used to toggle the LED's */
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Initialize and enable LED */
    APP_LED_INIT;

    mrt_clock = MRT_CLK_FREQ;

    /* mrtConfig.enableMultiTask = false; */
    MRT_GetDefaultConfig(&mrtConfig);

    /* Init mrt module */
    MRT_Init(MRT0, &mrtConfig);

    /* Setup Channel 0 to be repeated */
    MRT_SetupChannelMode(MRT0, kMRT_Channel_0, kMRT_RepeatMode);

    /* Enable timer interrupts for channel 0 */
    MRT_EnableInterrupts(MRT0, kMRT_Channel_0, kMRT_TimerInterruptEnable);

    /* Enable at the NVIC */
    EnableIRQ(MRT0_IRQn);

    /* Start channel 0 */
    PRINTF("\r\nStarting channel No.0 ...");
    if (USEC_TO_COUNT(250000U, mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK)
    {
        mrtDividerValue = 0;
        mrtEnableCount  = true;
        while (USEC_TO_COUNT((250000U >> (++mrtDividerValue)), mrt_clock) > MRT_CHANNEL_INTVAL_IVALUE_MASK)
        {
        }
        MRT_StartTimer(MRT0, kMRT_Channel_0, USEC_TO_COUNT((250000U >> mrtDividerValue), mrt_clock));
    }
    else
    {
        MRT_StartTimer(MRT0, kMRT_Channel_0, USEC_TO_COUNT(250000U, mrt_clock));
    }

    while (true)
    {
        /* Check whether occur interupt and toggle LED */
        if (true == mrtIsrFlag)
        {
            PRINTF("\r\n Channel No.0 interrupt is occurred !");
            APP_LED_TOGGLE;
            mrtCountValue = 0;
            mrtIsrFlag    = false;
        }
    }
}
