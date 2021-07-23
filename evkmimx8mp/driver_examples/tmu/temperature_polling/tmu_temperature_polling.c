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
#define DEMO_TMU_BASE TMU

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

    PRINTF("TMU temperature polling example.\r\n");

    /* Get default configuration. */
    TMU_GetDefaultConfig(&config);

    /* Set TMU configuration. */
    config.averageLPF  = kTMU_AverageLowPassFilter0_25;
    config.probeSelect = kTMU_ProbeSelectMainProbe;

    /* Initialize the TMU mode. */
    TMU_Init(DEMO_TMU_BASE, &config);

    /* The temperature at the initial power-on has a temperature peak that causes the average to be high, waiting for
       the average to stabilized, delay 200us to make sure the average temperature value tends to be stable. */
    SDK_DelayAtLeastUs(200, SystemCoreClock);

    while (true)
    {
        /* Get current average temperature. */
        status = TMU_GetAverageTemperature(DEMO_TMU_BASE, config.probeSelect, &temp);

        if (kStatus_Success == status)
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
                    PRINTF("Average temperature is nagetive %d celsius degree\r\n", temp);
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
                    PRINTF("Average temperature is positive %d celsius degree\r\n", temp);
                }
            }
        }
        else
        {
            PRINTF("Average temperature failed\r\n");
        }

        /* Delay. */
        SDK_DelayAtLeastUs(1000000, SystemCoreClock);
    }
}
