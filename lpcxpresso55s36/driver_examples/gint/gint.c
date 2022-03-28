/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_gint.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_GINT0_PORT kGINT_Port1
#define DEMO_GINT1_PORT kGINT_Port0

/* Select one input, active low for GINT0 */
#define DEMO_GINT0_POL_MASK ~(1U << BOARD_SW1_GPIO_PIN)
#define DEMO_GINT0_ENA_MASK (1U << BOARD_SW1_GPIO_PIN)

/* Select one input, active low for GINT1 */
#define DEMO_GINT1_POL_MASK ~(1U << BOARD_SW3_GPIO_PIN)
#define DEMO_GINT1_ENA_MASK (1U << BOARD_SW3_GPIO_PIN)

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
 * @brief Call back for GINT0 event
 */
void gint0_callback(void)
{
    PRINTF("\f\r\nGINT0 event detected\r\n");
}

/*!
 * @brief Call back for GINT1 event
 */
void gint1_callback(void)
{
    PRINTF("\f\r\nGINT1 event detected\r\n");
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Clear screen*/
    PRINTF("%c[2J", 27);
    /* Set cursor location at [0,0] */
    PRINTF("%c[0;0H", 27);
    PRINTF("\f\r\nGroup GPIO input interrupt example\r\n");

    /* Initialize GINT0 & GINT1 */
    GINT_Init(GINT0);

#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 1)
    GINT_Init(GINT1);
#endif

    /* Setup GINT0 for edge trigger, "OR" mode */
    GINT_SetCtrl(GINT0, kGINT_CombineOr, kGINT_TrigEdge, gint0_callback);

/* Setup GINT1 for edge trigger, "AND" mode */
#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 1)
    GINT_SetCtrl(GINT1, kGINT_CombineAnd, kGINT_TrigEdge, gint1_callback);
#endif
    /* Select pins & polarity for GINT0 */
    GINT_ConfigPins(GINT0, DEMO_GINT0_PORT, DEMO_GINT0_POL_MASK, DEMO_GINT0_ENA_MASK);

    /* Select pins & polarity for GINT1 */

#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 1)
    GINT_ConfigPins(GINT1, DEMO_GINT1_PORT, DEMO_GINT1_POL_MASK, DEMO_GINT1_ENA_MASK);
#endif
    /* Enable callbacks for GINT0 & GINT1 */
    GINT_EnableCallback(GINT0);

#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 1)
    GINT_EnableCallback(GINT1);
#endif

#if defined(FSL_FEATURE_SOC_GINT_COUNT) && (FSL_FEATURE_SOC_GINT_COUNT > 1)
    PRINTF("\r\nGINT0 and GINT1 events are configured\r\n");
    PRINTF("\r\nPress corresponding switches to generate events\r\n");
#else
    PRINTF("\r\nGINT0 event is configured\r\n");
    PRINTF("\r\nPress corresponding switch to generate events\r\n");
#endif

    while (1)
    {
        __WFI();
    }
}
