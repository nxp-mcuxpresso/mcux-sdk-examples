/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_gpio.h"
#include "veneer_table.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SECURE_SW1_GPIO      SECGPIO
#define DEMO_SECURE_SW1_GPIO_PORT 0U
#define DEMO_SECURE_SW1_GPIO_PIN  5U

#define DEMO_SW1_GPIO      BOARD_SW1_GPIO
#define DEMO_SW1_GPIO_PORT BOARD_SW1_GPIO_PORT
#define DEMO_SW1_GPIO_PIN  BOARD_SW1_GPIO_PIN


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/


/*******************************************************************************
 * Code
 ******************************************************************************/

void SystemInit(void)
{
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);

    while (1)
    {
        if (GPIO_PinRead(DEMO_SW1_GPIO, DEMO_SW1_GPIO_PORT, DEMO_SW1_GPIO_PIN))
        {
            LED_BLUE_OFF();
        }
        else
        {
            LED_BLUE_ON();
        }
    }
}
