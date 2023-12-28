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
#define BOARD_LED_RGPIO     GPIOE
#define BOARD_LED_RGPIO_PIN 6U

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
    BOARD_BootClockRUN();
    BOARD_InitDebugConsolePins();
    BOARD_InitDebugConsole();

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot();
    }
    else /* low power boot type */
    {
        PRINTF("Pls run the demo with A Core(the demo depend on resource of A Core)\r\n");
        assert(false);
    }

    SDK_DelayAtLeastUs(4000000U, SystemCoreClock); /* wait 4 seconds to make sure setup gpio by mcore */

    BOARD_InitBootPins();
    CLOCK_EnableClock(kCLOCK_RgpioE);

    /* Make m33(in secure world, private mode) can write RGPIOE/F(TF-A running on A Core changed gpio's PCNS, ICNS,
     * PCNP, ICNP, this make master(m33) in nonsecure world, user mode can write gpio) */
    if (BOARD_LED_RGPIO->PCNS & (1 << BOARD_LED_RGPIO_PIN))
    {
        BOARD_LED_RGPIO->PCNS &=
            ~(1 << BOARD_LED_RGPIO_PIN); /* make master in secure world, private mode can access the pin */
    }
    if (BOARD_LED_RGPIO->ICNS & (1 << BOARD_LED_RGPIO_PIN))
    {
        BOARD_LED_RGPIO->ICNS &=
            ~(1 << BOARD_LED_RGPIO_PIN); /* make master in secure world, private mode can access the pin */
    }
    if (BOARD_LED_RGPIO->PCNP & (1 << BOARD_LED_RGPIO_PIN))
    {
        BOARD_LED_RGPIO->PCNP &=
            ~(1 << BOARD_LED_RGPIO_PIN); /* make master in secure world, private mode can access the pin */
    }
    if (BOARD_LED_RGPIO->ICNP & (1 << BOARD_LED_RGPIO_PIN))
    {
        BOARD_LED_RGPIO->ICNP &=
            ~(1 << BOARD_LED_RGPIO_PIN); /* make master in secure world, private mode can access the pin */
    }

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
