/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "app.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
#include "fsl_component_button.h"
#endif
#include "fsl_component_led.h"

/*${header:end}*/

/*${variable:start}*/
button_config_t g_buttonConfig[BUTTON_COUNT] = {
    {
        .gpio =
            {
                .direction       = kHAL_GpioDirectionIn,
                .pinStateDefault = 1,
                .port            = 0,
                .pin             = BOARD_SW3_GPIO_PIN,
            },
    },
};

led_config_t g_ledMonochrome[LED_TYPE_MONOCHROME_COUNT] = {
    {
        .type = kLED_TypeMonochrome,
        .ledMonochrome =
            {
                .monochromePin =
                    {
                        .dimmingEnable = 0,
                        .gpio =
                            {
#if (defined(LED_USE_CONFIGURE_STRUCTURE) && (LED_USE_CONFIGURE_STRUCTURE > 0U))
                                .direction = kHAL_GpioDirectionOut,
#endif
                                .level = 1,
                                .port  = 2,
                                .pin   = BOARD_LED3_GPIO_PIN,
                            },
                    },
            },
    },
};
/*${variable:end}*/

/*${function:start}*/
void BOARD_InitHardware(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Enable the asynchronous bridge */
    SYSCON->ASYNCAPBCTRL = 1;

    /* Use 12 MHz clock for some of the Ctimers */
    CLOCK_AttachClk(kFRO12M_to_ASYNC_APB);
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
}
/*${function:end}*/
