/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "board.h"
#include "fsl_anactrl.h"
#include "fsl_debug_console.h"
#include "fsl_inputmux.h"
#include "fsl_inputmux_connections.h"

#include <stdbool.h>
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_INPUTMUX                  INPUTMUX
#define DEMO_ANACTRL                   ANACTRL
#define DEMO_REFERENCE_CLOCK           kINPUTMUX_WdtOscToFreqmeasRef
#define DEMO_REFERENCE_CLOCK_FREQUENCY 1000000U
#define DEMO_TARGET_CLOCK              kINPUTMUX_32KhzOscToFreqmeasTarget
#define DEMO_SCALE                     11U
#define DEMO_REFERENCE_CLOCK_SOURCE    "\r\nReference clock source: wdt oscillator.\r\n"
#define DEMO_TARGET_CLOCK_SOURCE       "\r\nTarget clock source: 32kHz oscillator.\r\n"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function.
 */
int main(void)
{
    uint32_t targetClkFreq = 0U;

    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);
    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 2U, true);

    CLOCK_EnableClock(kCLOCK_Freqme);
    POWER_DisablePD(kPDRUNCFG_PD_FRO32K);
    /* Enable FRO 1 MHz clock for Frequency Measure module. */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_UTICK_ENA_MASK;

    /* Init inputmux. */
    INPUTMUX_Init(DEMO_INPUTMUX);

    /* Init analog control. */
    ANACTRL_Init(DEMO_ANACTRL);

    /* Set the reference and target clock source. */
    INPUTMUX_AttachSignal(DEMO_INPUTMUX, 0U, DEMO_REFERENCE_CLOCK);
    INPUTMUX_AttachSignal(DEMO_INPUTMUX, 0U, DEMO_TARGET_CLOCK);

    PRINTF("Analog control measure frequency example.\r\n");

    /* Start Measurement. */
    targetClkFreq = ANACTRL_MeasureFrequency(DEMO_ANACTRL, DEMO_SCALE, DEMO_REFERENCE_CLOCK_FREQUENCY);
    PRINTF(DEMO_REFERENCE_CLOCK_SOURCE);
    PRINTF(DEMO_TARGET_CLOCK_SOURCE);
    PRINTF("\r\nTarget clock frequency: %d Hz.\r\n", targetClkFreq);

    while (1)
    {
    }
}
