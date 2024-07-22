/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_flexio.h"

#include "fsl_iomuxc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_LED_FLEXIO     FLEXIO1
#define BOARD_LED_FLEXIO_PIN 4U

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
    /* Define the init structure for the output LED pin*/
    flexio_gpio_config_t led_config = {
        kFLEXIO_DigitalOutput,
        0U,
        0U,
    };
    flexio_config_t fxioUserConfig;
    /* Board pin, clock, debug console init */
    /* clang-format off */
    const clock_root_config_t flexioClkCfg = {
        .clockOff = false,
	.mux = 0, /* 24MHz oscillator source */
	.div = 1
    };
    /* clang-format on */

    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(kCLOCK_Root_Flexio1, &flexioClkCfg);

    /* Print a note to terminal. */
    PRINTF("\r\n FLEXIO PIN Driver example\r\n");
    PRINTF("\r\n The LED is taking turns to shine.\r\n");

    /* Init flexio, use default configure
     * Disable doze and fast access mode
     * Enable in debug mode
     */
    FLEXIO_GetDefaultConfig(&fxioUserConfig);
    /* Reset the flexio to clear register*/
    FLEXIO_Init(BOARD_LED_FLEXIO, &fxioUserConfig);

    /* Init output LED FLEXIO. */
    FLEXIO_SetPinConfig(BOARD_LED_FLEXIO, BOARD_LED_FLEXIO_PIN, &led_config);

    while (1)
    {
        SDK_DelayAtLeastUs(1000000U, SystemCoreClock);
        FLEXIO_TogglePortOutput(BOARD_LED_FLEXIO, 1U << BOARD_LED_FLEXIO_PIN);
    }
}
