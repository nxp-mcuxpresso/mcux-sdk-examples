/*
 * Copyright 2017, 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_acomp.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_power.h"
#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ACOMP                ACOMP
#define DEMO_ACOMP_POSITIVE_INPUT 0U /* Voltage ladder output. */
#define DEMO_ACOMP_NEGATIVE_INPUT 3U /* ACMP_I3. */
#define BOARD_LED_PORT            0U
#define BOARD_LED_PIN             12U

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
    acomp_config_t acompConfigStruct;
    acomp_ladder_config_t ladderConfigStruct;

    /* Enable clock of uart0. */
    CLOCK_EnableClock(kCLOCK_Uart0);
    /* Ser DIV of uart0. */
    CLOCK_SetClkDivider(kCLOCK_DivUsartClk, 1U);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    /* Power on ACMP. */
    POWER_DisablePD(kPDRUNCFG_PD_ACMP);

    PRINTF("\r\nLPC_ACOMP Basic Example.\r\n");

    /* Init output LED GPIO. */
    LED_RED_INIT(LOGIC_LED_ON);

    /* Initialize ACOMP module. */
    ACOMP_GetDefaultConfig(&acompConfigStruct);
    ACOMP_Init(DEMO_ACOMP, &acompConfigStruct);

    /* Configure internal voltage ladder. */
    ladderConfigStruct.ladderValue      = 15U;                           /* Half of reference voltage. */
    ladderConfigStruct.referenceVoltage = kACOMP_LadderRefVoltagePinVDD; /* VDDA as the reference voltage. */
    ACOMP_SetLadderConfig(DEMO_ACOMP, &ladderConfigStruct);

#if defined(FSL_FEATURE_ACOMP_HAS_CTRL_INTENA) && FSL_FEATURE_ACOMP_HAS_CTRL_INTENA
    /* Disable interrupt. */
    ACOMP_EnableInterrupts(DEMO_ACOMP, kACOMP_InterruptsDisable);
#endif /*FSL_FEATURE_ACOMP_HAS_CTRL_INTENA*/
    /* Configure ACOMP negative and positive input channels. */
    ACOMP_SetInputChannel(DEMO_ACOMP, DEMO_ACOMP_POSITIVE_INPUT, DEMO_ACOMP_NEGATIVE_INPUT);

    PRINTF("The example compares analog input to the voltage ladder output(ACOMP negative port).\r\n");
    PRINTF("The LED will be turned ON/OFF when the analog input is LOWER/HIGHER than the ladder's output.\r\n");
    PRINTF("Change the analog input voltage to see the LED status.\r\n");

    while (true)
    {
        /* Check the comparison result and sets the LED state according to the result.*/
        if (ACOMP_GetOutputStatusFlags(DEMO_ACOMP))
        {
            LED_RED_ON();
        }
        else
        {
            LED_RED_OFF();
        }
    }
}
