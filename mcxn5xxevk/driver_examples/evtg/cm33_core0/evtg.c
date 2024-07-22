/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_evtg.h"

#include "fsl_inputmux.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_EVTG       EVTG0
#define DEMO_EVTG_INDEX kEVTG_Index0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void IO_Config(void);

static void EVTG_Config(void);
extern void IO_Config(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void IO_Config(void)
{
    /* Two IOs are used for EVTG input */
    INPUTMUX_AttachSignal(INPUTMUX0, 0U, kINPUTMUX_TrigIn0ToEvtgTrigger);
    INPUTMUX_AttachSignal(INPUTMUX0, 1U, kINPUTMUX_TrigIn1ToEvtgTrigger);
}


/*!
 * @brief Main function
 */
int main(void)
{
    /* Initializes board hardware */
    /* Enables the clock for INPUTMUX: Enables clock */
    CLOCK_EnableClock(kCLOCK_InputMux0);
    /* Enables the clock for INPUTMUX: Enables clock */
    CLOCK_EnableClock(kCLOCK_Evtg);

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    INPUTMUX_Init(INPUTMUX0);

    /* Set the EVTG_OUT to TRIG_OUT0 */
    INPUTMUX_AttachSignal(INPUTMUX0, 0U, kINPUTMUX_EvtgOut0AToExtTrigger);

    IO_Config();
    EVTG_Config();

    PRINTF("\r\nEvtg project \r\n");

    while (1)
    {
    }
}

/*
 * Function description:
 * This function initializes the EVTG module
 */
static void EVTG_Config(void)
{
    evtg_config_t evtgConfig;

    /* Initialize EVTG module */
    EVTG_GetDefaultConfig(&evtgConfig, kEVTG_FFModeBypass);
    evtgConfig.aoi0Config.productTerm0.aInput = kEVTG_InputDirectPass;
    evtgConfig.aoi0Config.productTerm0.bInput = kEVTG_InputDirectPass;
    evtgConfig.aoi0Config.productTerm0.cInput = kEVTG_InputLogicOne;
    evtgConfig.aoi0Config.productTerm0.dInput = kEVTG_InputLogicOne;
    EVTG_Init(DEMO_EVTG, DEMO_EVTG_INDEX, &evtgConfig);
}
