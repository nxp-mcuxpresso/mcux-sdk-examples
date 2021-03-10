/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_dac.h"

#include "fsl_power.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASE             DAC0
#define DEMO_DAC_IRQ              DAC0_IRQn
#define DEMO_DAC_COUNTER_VALUE    1000U
#define DEMO_DAC_IRQ_HANDLER_FUNC DAC0_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
const uint32_t g_waveform[] = {0U,    100U, 200U, 300U, 400U, 500U, 600U, 700U, 800U, 900U,
                               1000U, 900U, 800U, 700U, 600U, 500U, 400U, 300U, 200U, 100U};
/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_DAC_IRQ_HANDLER_FUNC(void)
{
    static uint32_t windex = 0;
    DAC_SetBufferValue(DEMO_DAC_BASE, g_waveform[windex++]);
    if (windex == sizeof(g_waveform) / sizeof(g_waveform[0]))
    {
        windex = 0;
    }
    __DSB();
}

/*!
 * @brief Main function
 */
int main(void)
{
    dac_config_t dacConfigStruct;

    /* Attach 12 MHz clock to USART0 (debug console) */
    CLOCK_Select(BOARD_DEBUG_USART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockFRO30M();
    BOARD_InitDebugConsole();

    /* Power on the DAC0.*/
    POWER_DisablePD(kPDRUNCFG_PD_DAC0);

    /* Turn on LED RED */
    LED_RED_INIT(LOGIC_LED_ON);

    PRINTF("\r\nDAC interrupt Example.\r\n");

    /* Configure the DAC. */
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_BASE, &dacConfigStruct);
    /* Configure the frequency of DAC interrupts. */
    DAC_SetCounterValue(DEMO_DAC_BASE, DEMO_DAC_COUNTER_VALUE);
    DAC_EnableDoubleBuffering(DEMO_DAC_BASE, true);
    /* Enable the DAC interrupt. */
    NVIC_EnableIRQ(DEMO_DAC_IRQ);

    PRINTF("Please probe the signal using an oscilloscope.\r\n");

    while (1)
    {
    }
}
