/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_rgpio.h"
#include "veneer_table.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define TEST_SW7_GPIO GPIOB
#define TEST_SW8_GPIO GPIOB
#define TEST_NS_GPIOA GPIOA

#define TEST_SW8_GPIO_PIN 12U
#define TEST_SW7_GPIO_PIN 13U
#define TEST_GPIOA_PIN15  15U
#define TEST_GPIOA_PIN18  18U

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

    while (1)
    {
        if (RGPIO_PinRead(TEST_SW7_GPIO, TEST_SW7_GPIO_PIN))
        {
            RGPIO_PinWrite(TEST_NS_GPIOA, TEST_GPIOA_PIN18, 1);
        }
        else
        {
            RGPIO_PinWrite(TEST_NS_GPIOA, TEST_GPIOA_PIN18, 0);
        }

        SDK_DelayAtLeastUs(1000, SystemCoreClock);
    }
}
