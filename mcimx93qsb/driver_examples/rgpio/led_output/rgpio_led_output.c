/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_rgpio.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_LED_RGPIO          GPIO2
#define BOARD_LED_RGPIO_PIN      4U
#define EXAMPLE_RGPIO_CLOCK_ROOT kCLOCK_Root_BusWakeup
#define EXAMPLE_RGPIO_CLOCK_GATE kCLOCK_Gpio2

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(void);

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
    rgpio_pin_config_t led_config = {
        kRGPIO_DigitalOutput,
        0,
    };

    /* Board pin, clock, debug console init */
    /* clang-format off */

    const clock_root_config_t rgpioClkCfg = {
        .clockOff = false,
        .mux = 0, // 24Mhz Mcore root buswake clock
        .div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(EXAMPLE_RGPIO_CLOCK_ROOT, &rgpioClkCfg);
    CLOCK_EnableClock(EXAMPLE_RGPIO_CLOCK_GATE);
    CLOCK_EnableClock(kCLOCK_Gpio2);
    /* Set PCNS register value to 0x0 to prepare the RGPIO initialization */
    BOARD_LED_RGPIO->PCNS = 0x0;

    /* Print a note to terminal. */
    PRINTF("\r\n RGPIO Driver example\r\n");
    PRINTF("\r\n The LED is taking turns to shine.\r\n");

    /* Init output LED GPIO. */
    RGPIO_PinInit(BOARD_LED_RGPIO, BOARD_LED_RGPIO_PIN, &led_config);

    while (1)
    {
        SDK_DelayAtLeastUs(1000000U, SystemCoreClock);
        RGPIO_PortToggle(BOARD_LED_RGPIO, 1u << BOARD_LED_RGPIO_PIN);
    }
}
