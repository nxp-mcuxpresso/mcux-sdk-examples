/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
float DEMO_MeasureTemperature(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
float DEMO_MeasureTemperature(void)
{
    uint16_t tempRawValue = 0U;

    if (0UL == ((SENSOR_CTRL->MISC_CTRL_REG & SENSOR_CTRL_MISC_CTRL_REG_TIMER_1_ENABLE_MASK) >>
                SENSOR_CTRL_MISC_CTRL_REG_TIMER_1_ENABLE_SHIFT))
    {
        SENSOR_CTRL->MISC_CTRL_REG |= SENSOR_CTRL_MISC_CTRL_REG_TIMER_1_ENABLE_MASK;
    }

    tempRawValue = (((SENSOR_CTRL->TSEN_CTRL_1_REG_2) & SENSOR_CTRL_TSEN_CTRL_1_REG_2_TSEN_TEMP_VALUE_MASK) >>
                    SENSOR_CTRL_TSEN_CTRL_1_REG_2_TSEN_TEMP_VALUE_SHIFT);

    return (tempRawValue * 0.480561F - 220.7074F);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\n Temperature measurement example.");
    PRINTF("\r\n Please press any key to get the temperature.");
    while (1)
    {
        GETCHAR();
        PRINTF("\r\n Current temperature is: %.3f degrees Celsius.", ((double)DEMO_MeasureTemperature()));
    }
}
