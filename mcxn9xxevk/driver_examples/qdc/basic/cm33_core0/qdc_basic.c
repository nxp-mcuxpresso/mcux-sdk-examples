/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_qdc.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_QDC_BASEADDR QDC0

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
 * @brief Main function
 */
int main(void)
{
    qdc_config_t mEncConfigStruct;
    uint32_t mCurPosValue;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nQDC Basic Example.\r\n");

    /* Initialize the QDC module. */
    QDC_GetDefaultConfig(&mEncConfigStruct);
#if (defined(FSL_FEATURE_QDC_HAS_CTRL3) && FSL_FEATURE_QDC_HAS_CTRL3)
    /*
     * If there is CTRL3, the period measurement is enabled by default,
     * with this setting, the POSD is loaded to POSDH only when POSD
     * is read (calling QDC_GetPositionDifferenceValue).
     * In this project, the POSD is desired to be loaded to POSDH when
     * UPOS is read (calling QDC_GetPositionValue), so disable the
     * period measurement here.
     */
    mEncConfigStruct.enablePeriodMeasurementFunction = false;
#endif

    QDC_Init(DEMO_QDC_BASEADDR, &mEncConfigStruct);
    QDC_DoSoftwareLoadInitialPositionValue(DEMO_QDC_BASEADDR); /* Update the position counter with initial value. */

    PRINTF("Press any key to get the encoder values ...\r\n");

    while (1)
    {
        GETCHAR();
        PRINTF("\r\n");

        /* This read operation would capture all the position counter to responding hold registers. */
        mCurPosValue = QDC_GetPositionValue(DEMO_QDC_BASEADDR);

        /* Read the position values. */
        PRINTF("Current position value: %ld\r\n", mCurPosValue);
        PRINTF("Position differential value: %d\r\n", (int16_t)QDC_GetHoldPositionDifferenceValue(DEMO_QDC_BASEADDR));
        PRINTF("Position revolution value: %d\r\n", QDC_GetHoldRevolutionValue(DEMO_QDC_BASEADDR));
    }
}
