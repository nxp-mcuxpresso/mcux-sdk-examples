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
#define DEMO_DAC_BASE DAC0

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
    uint8_t index;
    uint32_t dacValue;
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

    PRINTF("\r\nDAC basic Example.\r\n");

    /* Configure the DAC. */
    DAC_GetDefaultConfig(&dacConfigStruct);
    DAC_Init(DEMO_DAC_BASE, &dacConfigStruct);

    while (1)
    {
        index    = 0;
        dacValue = 0;
        PRINTF("\r\nPlease input a value (0 - 1023) to output with DAC: ");
        while (index != 0x0D)
        {
            index = GETCHAR();
            if ((index >= '0') && (index <= '9'))
            {
                PUTCHAR(index);
                dacValue = dacValue * 10 + (index - 0x30U);
            }
        }
        PRINTF("\r\nInput value is %d\r\n", dacValue);
        if (dacValue > 1023U)
        {
            PRINTF("Your value is output of range.\r\n");
            continue;
        }
        DAC_SetBufferValue(DEMO_DAC_BASE, dacValue);
        PRINTF("DAC out: %d\r\n", dacValue);
    }
}
