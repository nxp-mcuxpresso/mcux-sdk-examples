/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_dac.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_DAC_BASEADDR DAC0
#define DEMO_DAC_VREF     kDAC_ReferenceVoltageSourceAlt1

/*******************************************************************************
 * Prototypes
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
    dac_config_t dacConfigStruct;
    uint32_t dacValue;

    /* Attach peripheral clock */
    CLOCK_SetClockDiv(kCLOCK_DivDAC0, 1u);
    CLOCK_AttachClk(kFRO12M_to_DAC0);

    /* Release peripheral reset */
    RESET_ReleasePeripheralReset(kDAC0_RST_SHIFT_RSTn);

    /* Enable DAC0 */
    SPC0->ACTIVE_CFG1 |= 0x10;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nDAC Basic Example.\r\n");

    /* Configure the DAC. */
    DAC_GetDefaultConfig(&dacConfigStruct);
    dacConfigStruct.referenceVoltageSource = DEMO_DAC_VREF;
    DAC_Init(DEMO_DAC_BASEADDR, &dacConfigStruct);
    DAC_Enable(DEMO_DAC_BASEADDR, true); /* Enable the logic and output. */

    while (1)
    {
        index    = 0;
        dacValue = 0;
        PRINTF("\r\nPlease input a value (0 - 4095) to output with DAC: ");
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
        if (dacValue > 0xFFFU)
        {
            PRINTF("Your value is output of range.\r\n");
            continue;
        }
        DAC_SetData(DEMO_DAC_BASEADDR, dacValue);
        PRINTF("DAC out: %d\r\n", dacValue);
        /*
         * The value in the first item would be converted. User can measure the output voltage from DACx_OUT pin.
         */
    }
}
