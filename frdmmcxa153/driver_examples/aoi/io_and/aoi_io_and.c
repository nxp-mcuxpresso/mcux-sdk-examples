/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_aoi.h"
#include "fsl_debug_console.h"
/*
 * This example demos the AOI uses two IO. The AOI_OUT= (IO0 & IO1).
 */

#include <stdbool.h>
#include "fsl_inputmux.h"
#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_AOI_BASEADDR AOI0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void IO_Configuration(void);

static void AOI_Configuration(void);
extern void IO_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*
 * Function description:
 * This function is initialize the TRIG_OUT
 */
void IO_Configuration(void)
{
    /* Two IOs are used for AOI input */
    INPUTMUX_AttachSignal(INPUTMUX0, kINPUTMUX_INDEX_AOI0_TRIGSEL0, kINPUTMUX_TrigIn0ToAoi0Mux);
    INPUTMUX_AttachSignal(INPUTMUX0, kINPUTMUX_INDEX_AOI0_TRIGSEL1, kINPUTMUX_TrigIn1ToAoi0Mux);
}



int main(void)
{
    /* Initializes board hardware */
    RESET_PeripheralReset(kAOI0_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    INPUTMUX_Init(INPUTMUX0);
    /* Set the AOI_OUT to TRIG_OUT0 */
    INPUTMUX_AttachSignal(INPUTMUX0, kINPUTMUX_INDEX_EXT_TRIGSEL0, kINPUTMUX_Aoi0Out0ToExtTrigger);

    IO_Configuration();
    AOI_Configuration();

    PRINTF("aoi_io_and project.\r\n");

    while (1)
    {
    }
}

/*
 * Function description:
 * This function initializes the AOI module
 */
static void AOI_Configuration(void)
{
    aoi_event_config_t aoiEventLogicStruct;

    /* Configure the AOI event */
    aoiEventLogicStruct.PT0AC = kAOI_InputSignal; /* IO0 input */
    aoiEventLogicStruct.PT0BC = kAOI_InputSignal; /* IO1 input */
    aoiEventLogicStruct.PT0CC = kAOI_LogicOne;
    aoiEventLogicStruct.PT0DC = kAOI_LogicOne;

    aoiEventLogicStruct.PT1AC = kAOI_LogicZero;
    aoiEventLogicStruct.PT1BC = kAOI_LogicOne;
    aoiEventLogicStruct.PT1CC = kAOI_LogicOne;
    aoiEventLogicStruct.PT1DC = kAOI_LogicOne;

    aoiEventLogicStruct.PT2AC = kAOI_LogicZero;
    aoiEventLogicStruct.PT2BC = kAOI_LogicOne;
    aoiEventLogicStruct.PT2CC = kAOI_LogicOne;
    aoiEventLogicStruct.PT2DC = kAOI_LogicOne;

    aoiEventLogicStruct.PT3AC = kAOI_LogicZero;
    aoiEventLogicStruct.PT3BC = kAOI_LogicOne;
    aoiEventLogicStruct.PT3CC = kAOI_LogicOne;
    aoiEventLogicStruct.PT3DC = kAOI_LogicOne;

    /* Initializes AOI module. */
    AOI_Init(DEMO_AOI_BASEADDR);

    /* AOI_OUT = IO0 & IO1 */
    AOI_SetEventLogicConfig(DEMO_AOI_BASEADDR, kAOI_Event0, &aoiEventLogicStruct);
}
