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
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_INPUT_PIN_FLEXIO     FLEXIO2
#define BOARD_INPUT_PIN_FLEXIO_PIN 0U

#define BOARD_RGPIO_OUPUT_PORT     RGPIO4
#define BOARD_RGPIO_OUPUT_PORT_PIN 1U

#define FLEXIO_PIN_UserCallback FLEXIO2_IRQHandler

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool flexio_interrupt_flag = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Interrupt service fuction of FLEXIO.
 *
 * This function clear the interrupt flag.
 */
void FLEXIO_PIN_UserCallback(void)
{
    /* Clear FELXIO pin interrupt flag. */
    FLEXIO_Type *base = (FLEXIO_Type *)BOARD_INPUT_PIN_FLEXIO;
    if (FLEXIO_GetPinStatus(base, BOARD_INPUT_PIN_FLEXIO_PIN))
    {
        FLEXIO_ClearPortStatus(base, 1U << BOARD_INPUT_PIN_FLEXIO_PIN);
        flexio_interrupt_flag = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Define the init structure for the input pin*/
    flexio_gpio_config_t input_config = {
        kFLEXIO_DigitalInput,
        0U,
        kFLEXIO_FlagRisingEdgeEnable | kFLEXIO_InputInterruptEnable,
    };
    flexio_config_t fxioUserConfig;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print a note to terminal. */
    PRINTF("\r\n FLEXIO PIN Driver example\r\n");
    PRINTF("\r\n The output pin is taking turns to occurr rising and falling edge.\r\n");

    /* Init flexio, use default configure
     * Disable doze and fast access mode
     * Enable in debug mode
     */
    FLEXIO_GetDefaultConfig(&fxioUserConfig);
    /* Reset the flexio to clear register*/
    FLEXIO_Init(BOARD_INPUT_PIN_FLEXIO, &fxioUserConfig);

    /* Init input FLEXIO. */
    FLEXIO_SetPinConfig(BOARD_INPUT_PIN_FLEXIO, BOARD_INPUT_PIN_FLEXIO_PIN, &input_config);

    while (1)
    {
        /* Toggle GPIO output pin. */
        RGPIO_PortToggle(BOARD_RGPIO_OUPUT_PORT, 1U << BOARD_RGPIO_OUPUT_PORT_PIN);
        SDK_DelayAtLeastUs(1000000U, SystemCoreClock);

        /* reset the interrupt flag . */
        if (flexio_interrupt_flag == true)
        {
            PRINTF("Toggle output pin , FLEXIO PIN interrupt rising interrupt occurred \r\n");
            flexio_interrupt_flag = false;
        }
        else
        {
            PRINTF("Toggle output pin , FLEXIO PIN interrupt didn't occurr\r\n");
        }
    }
}
