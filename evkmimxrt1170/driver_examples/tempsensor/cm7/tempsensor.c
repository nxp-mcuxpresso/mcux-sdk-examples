/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_tempsensor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_TEMP_SENSOR              TMPSNS
#define DEMO_TEMP_LOW_HIGH_IRQn       TMPSNS_LOW_HIGH_IRQn
#define DEMO_TEMP_PANIC_IRQn          TMPSNS_PANIC_IRQn
#define DEMO_TEMP_LOW_HIGH_IRQHandler TMPSNS_LOW_HIGH_IRQHandler
#define DEMO_TEMP_PANIC_IRQHandler    TMPSNS_PANIC_IRQHandler
#define DEMO_HIGH_ALARM_TEMP          27U


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

float temperature = 0U;

volatile bool temperatureReach = false;

/*******************************************************************************
 * Code
 ******************************************************************************/

void DEMO_TEMP_LOW_HIGH_IRQHandler(void)
{
    temperatureReach = true;

    /* Disable high temp interrupt to jump out dead loop */
    TMPSNS_DisableInterrupt(DEMO_TEMP_SENSOR, kTEMPSENSOR_HighTempInterruptStatusEnable);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    tmpsns_config_t config;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("TMPSNS driver example. \r\n");

    TMPSNS_GetDefaultConfig(&config);
    config.measureMode   = kTEMPSENSOR_ContinuousMode;
    config.frequency     = 0x03U;
    config.highAlarmTemp = DEMO_HIGH_ALARM_TEMP;

    TMPSNS_Init(DEMO_TEMP_SENSOR, &config);
    TMPSNS_StartMeasure(DEMO_TEMP_SENSOR);

    EnableIRQ(DEMO_TEMP_LOW_HIGH_IRQn);

    /* Get temperature */
    temperature = TMPSNS_GetCurrentTemperature(DEMO_TEMP_SENSOR);

    PRINTF("The chip initial temperature is %.1f celsius degree. \r\n", temperature);

    while (1)
    {
        /* Get current temperature */
        temperature = TMPSNS_GetCurrentTemperature(DEMO_TEMP_SENSOR);

        if (temperatureReach && (temperature - DEMO_HIGH_ALARM_TEMP > 0))
        {
            temperatureReach = false;

            /* Re-enable high temperature interrupt */
            TMPSNS_EnableInterrupt(DEMO_TEMP_SENSOR, kTEMPSENSOR_HighTempInterruptStatusEnable);

            PRINTF("The chip temperature has reached high temperature that is %.1f celsius degree. \r\n", temperature);
            break;
        }
    }

    while (1)
    {
    }
}
