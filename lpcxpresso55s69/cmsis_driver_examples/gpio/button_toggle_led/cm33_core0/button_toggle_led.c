/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_pint.h"

#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_inputmux.h"
#include "fsl_power.h"
#include "fsl_gpio_cmsis.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define EXAMPLE_BUTTON_GPIO_INTERFACE (Driver_GPIO_PORT1)
#define EXAMPLE_BUTTON_PIN            BOARD_SW2_GPIO_PIN
#define EXAMPLE_BUTTON_INTERRUPT_TYPE ARM_GPIO_INTERRUPT_FALLING_EDGE
#define EXAMPLE_LED_GPIO_INTERFACE    (Driver_GPIO_PORT1)
#define EXAMPLE_LED_PIN               BOARD_LED_BLUE_GPIO_PIN
#define EXAMPLE_LED_LOGIC_DARK        ARM_GPIO_LOGIC_ZERO

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/
void BUTTON_InterrupHandler(uint32_t pin)
{
    if (pin == EXAMPLE_BUTTON_PIN)
    {
        EXAMPLE_LED_GPIO_INTERFACE.PinToggle(EXAMPLE_LED_PIN);
        PRINTF("\r\nBUTTON Pressed! \r\n");
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach main clock divide to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Initialize PINT */
    PINT_Init(PINT);

    PRINTF("\r\nCMSIS GPIO Example! \r\n");
    PRINTF("\r\nUse Button to toggle LED! \r\n");

    EXAMPLE_BUTTON_GPIO_INTERFACE.Initialize();
    EXAMPLE_LED_GPIO_INTERFACE.Initialize();

    EXAMPLE_BUTTON_GPIO_INTERFACE.InitPinAsInput(EXAMPLE_BUTTON_PIN, EXAMPLE_BUTTON_INTERRUPT_TYPE,
                                                 BUTTON_InterrupHandler);
    EXAMPLE_LED_GPIO_INTERFACE.InitPinAsOutput(EXAMPLE_LED_PIN, EXAMPLE_LED_LOGIC_DARK);

    while (1)
    {
        __WFI();
    }
}
