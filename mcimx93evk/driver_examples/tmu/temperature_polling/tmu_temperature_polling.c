/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_tmu.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* @TEST_ANCHOR */

#define DEMO_TMU_BASE TMU
#ifndef DEMO_TMU_IMMEDIATE_THRESOLD
#define DEMO_TMU_IMMEDIATE_THRESOLD 0U
#endif
#ifndef DEMO_TMU_RISING_FALLING_THRESOLD
#define DEMO_TMU_RISING_FALLING_THRESOLD 7U
#endif
#ifndef DEMO_TMU_INTERVAL_VALUE
#define DEMO_TMU_INTERVAL_VALUE 5U
#endif
#define DEMO_TMU_IRQ              TEMPMON_IRQn
#define DEMO_TMU_IRQ_HANDLER_FUNC TEMPMON_IRQHandler


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
    tmu_config_t config;
    tmu_thresold_config_t k_tmuThresoldConfig;

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    CLOCK_EnableClock(kCLOCK_Tmc);

    PRINTF("TMU temperature polling example.\r\n");

    /* Get default configuration. */
    TMU_GetDefaultConfig(&config);

    /* Set TMU configuration. */
    config.averageLPF  = kTMU_AverageLowPassFilter1_0;

    /* Initialize the TMU mode. */
    TMU_Init(DEMO_TMU_BASE, &config);

     /* Set the temperature threshold. */
    k_tmuThresoldConfig.immediateThresoldEnable       = true;
    k_tmuThresoldConfig.averageThresoldEnable         = false;
    k_tmuThresoldConfig.averageCriticalThresoldEnable = false;
    k_tmuThresoldConfig.risingCriticalThresoldEnable  = true;
    k_tmuThresoldConfig.fallingCriticalThresoldEnable = true;
    k_tmuThresoldConfig.immediateThresoldValue        = DEMO_TMU_IMMEDIATE_THRESOLD;
    k_tmuThresoldConfig.averageThresoldValue          = 0U;
    k_tmuThresoldConfig.averageCriticalThresoldValue  = 0U;
    k_tmuThresoldConfig.risingfallingCriticalThresoldValue   = DEMO_TMU_RISING_FALLING_THRESOLD;
    TMU_SetHighTemperatureThresold(DEMO_TMU_BASE, &k_tmuThresoldConfig);

    /* Enable TMU */
    TMU_Enable(DEMO_TMU_BASE, true);

    /* Delay 1s to avoid temp value 0 degree. */
    SDK_DelayAtLeastUs(1000000, SystemCoreClock);

    while (true)
    {
        float temp = 0.0f;
        if (0U != (TMU->TIDR & TMU_TIDR_RTRCT_MASK) || 0U != (TMU->TIDR & TMU_TIDR_FTRCT_MASK))
        {
            TMU->TIDR |= (TMU_TIDR_RTRCT_MASK | TMU_TIDR_FTRCT_MASK);
        }
        else
        {
            TMU_GetImmediateTemperature(DEMO_TMU_BASE, &temp);
            PRINTF("Board immediate temperature is %6.2f celsius degree.\r\n", (double)temp);
        }
        /* Delay. */
        SDK_DelayAtLeastUs(1000000, SystemCoreClock);
    }
}
