/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_inputmux.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_fmeas.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FMEAS_SYSCON FREQME
/* FMEASURE_CH0_SEL is reference clock selector, FMEASURE_CH1_SEL is target clock selector. */
#define EXAMPLE_REFERENCE_CLOCK_REGISTRY_INDEX 0
#define EXAMPLE_REFERENCE_CLOCK_NAME           "main system clock"
#define EXAMPLE_REFERENCE_CLOCK                kINPUTMUX_MainSysClkToFreqmeas
#define EXAMPLE_REFERENCE_CLOCK_FREQUENCY      CLOCK_GetFreq(kCLOCK_BusClk)
#define EXAMPLE_TARGET_CLOCK_REGISTRY_INDEX    1
#define EXAMPLE_TARGET_CLOCK_1_NAME            "External clock (clk_in)"
#define EXAMPLE_TARGET_CLOCK_2_NAME            "LPOSC clock"
#define EXAMPLE_TARGET_CLOCK_3_NAME            "FRO192M clock"
#define EXAMPLE_TARGET_CLOCK_1                 kINPUTMUX_XtalinToFreqmeas
#define EXAMPLE_TARGET_CLOCK_2                 kINPUTMUX_LposcToFreqmeas
#define EXAMPLE_TARGET_CLOCK_3                 kINPUTMUX_Fro192mToFreqmeas
#define EXAMPLE_TARGET_CLOCK_1_FREQUENCY       CLOCK_GetXtalInClkFreq()
#define EXAMPLE_TARGET_CLOCK_2_FREQUENCY       CLOCK_GetLpOscFreq()
#define EXAMPLE_TARGET_CLOCK_3_FREQUENCY       CLK_FRO_CLK

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief    Measurement cycle with value display
 *
 * @param    srcName : Name of the measured target clock
 * @param    target  : Target (measured) clock
 * @param    freqRef : Reference frequency [Hz]
 * @param    freqExp : Expected frequency [Hz]
 *
 * @return   Measured frequency [Hz]
 */
static uint32_t MeasureDisplay(char *srcName, inputmux_connection_t target, uint32_t freqRef, uint32_t freqExp)
{
    uint32_t freqComp;

    /* Setup to measure the selected target */
    INPUTMUX_AttachSignal(INPUTMUX, EXAMPLE_TARGET_CLOCK_REGISTRY_INDEX, target);

    /* Start a measurement cycle and wait for it to complete. If the target
       clock is not running, the measurement cycle will remain active
       forever, so a timeout may be necessary if the target clock can stop */
    FMEAS_StartMeasure(FMEAS_SYSCON);
    while (!FMEAS_IsMeasureComplete(FMEAS_SYSCON))
    {
    }

    /* Get computed frequency */
    freqComp = FMEAS_GetFrequency(FMEAS_SYSCON, freqRef);

    /* Show the raw capture value and the compute frequency */
    PRINTF("Capture source: %s, reference frequency = %u Hz\r\n", srcName, freqRef);
    PRINTF("Computed frequency value = %u Hz\r\n", freqComp);
    PRINTF("Expected frequency value = %u Hz\r\n\r\n", freqExp);

    return freqComp;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_EnableClock(kCLOCK_InputMux);
    CLOCK_EnableClock(kCLOCK_Freqme);
    RESET_PeripheralReset(kFREQME_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kINPUTMUX_RST_SHIFT_RSTn);

    uint32_t freqRef = EXAMPLE_REFERENCE_CLOCK_FREQUENCY;

    INPUTMUX_AttachSignal(INPUTMUX, EXAMPLE_REFERENCE_CLOCK_REGISTRY_INDEX, EXAMPLE_REFERENCE_CLOCK);

    /* Start frequency measurement 1 and display results */
    MeasureDisplay(EXAMPLE_TARGET_CLOCK_1_NAME " (" EXAMPLE_REFERENCE_CLOCK_NAME " reference)", EXAMPLE_TARGET_CLOCK_1,
                   freqRef, EXAMPLE_TARGET_CLOCK_1_FREQUENCY);

    /* Start frequency measurement 2 and display results */
    MeasureDisplay(EXAMPLE_TARGET_CLOCK_2_NAME " (" EXAMPLE_REFERENCE_CLOCK_NAME " reference)", EXAMPLE_TARGET_CLOCK_2,
                   freqRef, EXAMPLE_TARGET_CLOCK_2_FREQUENCY);

    /* Start frequency measurement 3 and display results */
    MeasureDisplay(EXAMPLE_TARGET_CLOCK_3_NAME " (" EXAMPLE_REFERENCE_CLOCK_NAME " reference)", EXAMPLE_TARGET_CLOCK_3,
                   freqRef, EXAMPLE_TARGET_CLOCK_3_FREQUENCY);

    while (1)
    {
    }
}
