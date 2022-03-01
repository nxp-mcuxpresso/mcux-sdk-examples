/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_tmu.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#define DEMO_TMU_BASE TMU
/* For real temperature value, bit-8 is sign bit: 1 means nagetive and 0 means positive. */
#ifndef DEMO_TMU_TEST
#define DEMO_TMU_IMMEDIATE_THRESOLD        (1U << 7 | 10U)
#define DEMO_TMU_AVERAGE_THRESOLD          (0U << 7 | 50U)
#define DEMO_TMU_AVERAGE_CRITICAL_THRESOLD (0U << 7 | 88U)
#endif
#define DEMO_TMU_IRQ                       ANAMIX_IRQn
#define DEMO_TMU_IRQ_HANDLER_FUNC          ANAMIX_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void TMU_LogCurrentTemperature(int8_t temp, bool isfirst);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool temperatureReach = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief TMU ISR.
 */
void DEMO_TMU_IRQ_HANDLER_FUNC(void)
{
    uint32_t intStatus;

    temperatureReach = true;

    intStatus = TMU_GetInterruptStatusFlags(DEMO_TMU_BASE);
    /* Clear interrupt status flag has one condition that real temperature is lower than threshold. */
    TMU_ClearInterruptStatusFlags(DEMO_TMU_BASE, intStatus);

    /* Disable TMU IRQ to avoid dead loop. */
    (void)DisableIRQ(DEMO_TMU_IRQ);
}

/*!
 * @brief Main function
 */
int main(void)
{
    int8_t temp;
    status_t status;
    tmu_config_t config;

    /* M7 has its local cache and enabled by default,
     * need to set smart subsystems (0x28000000 ~ 0x3FFFFFFF)
     * non-cacheable before accessing this address region */
    BOARD_InitMemory();

    /* Board specific RDC settings */
    BOARD_RdcInit();

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("TMU monitor threshold example.\r\n");

    /* Get default configuration. */
    TMU_GetDefaultConfig(&config);

    /* Set TMU configuration. */
    config.averageLPF                                               = kTMU_AverageLowPassFilter0_5;
    config.probeSelect                                              = kTMU_ProbeSelectMainProbe;
    config.thresholdConfig.immediateThresholdEnable                 = false;
    config.thresholdConfig.immediateThresholdValueOfMainProbe       = DEMO_TMU_IMMEDIATE_THRESOLD;
    config.thresholdConfig.AverageThresholdEnable                   = true;
    config.thresholdConfig.averageThresholdValueOfMainProbe         = DEMO_TMU_AVERAGE_THRESOLD;
    config.thresholdConfig.AverageCriticalThresholdEnable           = false;
    config.thresholdConfig.averageCriticalThresholdValueOfMainProbe = DEMO_TMU_AVERAGE_CRITICAL_THRESOLD;

    /* Initialize the TMU mode. */
    TMU_Init(DEMO_TMU_BASE, &config);

    /* The temperature at the initial power-on has a temperature peak that causes the average to be high, waiting for
       the average to stabilized, delay 200us to make sure that the average temperature value tends to be stable. */
    SDK_DelayAtLeastUs(200, SystemCoreClock);

    /* Enable the Average temperature threshold exceeded interrupt of main probe. */
    TMU_EnableInterrupts(DEMO_TMU_BASE, kTMU_AverageTemperature0InterruptEnable);
    (void)EnableIRQ(DEMO_TMU_IRQ);

    /* Get current average temperature. */
    status = TMU_GetAverageTemperature(DEMO_TMU_BASE, config.probeSelect, &temp);

    if (kStatus_Success == status)
    {
        TMU_LogCurrentTemperature(temp, true);
    }
    else
    {
        PRINTF("Initialization average temperature failed\r\n");
    }

    /* Waiting for average threshold to be reached. */
    while (temperatureReach != true)
    {
    }

    /* Get current average temperature. */
    status = TMU_GetAverageTemperature(DEMO_TMU_BASE, config.probeSelect, &temp);

    if (kStatus_Success == status)
    {
        TMU_LogCurrentTemperature(temp, false);
    }
    else
    {
        PRINTF("Average temperature failed\r\n");
    }

    PRINTF("High temperature average threshold to be reached.\r\n");

    while (true)
    {
    }
}

void TMU_LogCurrentTemperature(int8_t temp, bool isfirst)
{
    /* Bit-8 is sign bit: 1 means nagetive and 0 means positive. */
    if ((temp >> 0x07U) == 0x01U)
    {
        temp = temp & ~(0x01U << 7);

        if (temp > 40)
        {
            PRINTF("Temperature out of scope!\r\n");
        }
        else
        {
            if (isfirst)
            {
                PRINTF("Initialization average temperature is nagetive %d celsius degree\r\n", temp);
            }
            else
            {
                PRINTF("Average temperature is nagetive %d celsius degree\r\n", temp);
            }
        }
    }
    else
    {
        if (temp > 125)
        {
            PRINTF("Temperature out of scope!\r\n");
        }
        else
        {
            if (isfirst)
            {
                PRINTF("Initialization average temperature is positive %d celsius degree\r\n", temp);
            }
            else
            {
                PRINTF("Average temperature is positive %d celsius degree\r\n", temp);
            }
        }
    }
}
