/*
 * Copyright 2020 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "demo_config.h"
#include "board.h"

#include "task.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef DEMO_LED_VALUE
#define DEMO_LED_VALUE 0U
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern EventGroupHandle_t g_errorEvent;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void LED_Task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/

void led_blinky_init(void)
{
    if (xTaskCreate(LED_Task, "Blinky task", 1000UL / sizeof(portSTACK_TYPE), NULL, 4U, NULL) != pdPASS)
    {
        PRINTF("Blinky task creation failed!\r\n");
        while (1)
            ;
    }
}

static void LED_Task(void *pvParameters)
{
    while (xEventGroupGetBits(g_errorEvent) == 0U)
    {
        vTaskDelay(50 / portTICK_PERIOD_MS);
        if (GPIO_PinRead(BOARD_BTN_WAKE_GPIO, BOARD_BTN_WAKE_GPIO_PORT, BOARD_BTN_WAKE_GPIO_PIN) == 0)
        {
            GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, DEMO_LED_VALUE);
        }
        else
        {
            GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, !DEMO_LED_VALUE);
        }
        if (GPIO_PinRead(BOARD_BTN_ISP_GPIO, BOARD_BTN_ISP_GPIO_PORT, BOARD_BTN_ISP_GPIO_PIN) == 0)
        {
            GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, DEMO_LED_VALUE);
        }
        else
        {
            GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, !DEMO_LED_VALUE);
        }
        if (GPIO_PinRead(BOARD_BTN_USER_GPIO, BOARD_BTN_USER_GPIO_PORT, BOARD_BTN_USER_GPIO_PIN) == 0)
        {
            GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, DEMO_LED_VALUE);
        }
    }

    /*
     * Error handling loop. Will blink the BLUE LED.
     */
    GPIO_PinWrite(BOARD_LED_RED_GPIO, BOARD_LED_RED_GPIO_PORT, BOARD_LED_RED_GPIO_PIN, !DEMO_LED_VALUE);
    GPIO_PinWrite(BOARD_LED_BLUE_GPIO, BOARD_LED_BLUE_GPIO_PORT, BOARD_LED_BLUE_GPIO_PIN, !DEMO_LED_VALUE);
    GPIO_PinWrite(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, BOARD_LED_GREEN_GPIO_PIN, !DEMO_LED_VALUE);

    while (1)
    {
        vTaskDelay(300 / portTICK_PERIOD_MS);
        GPIO_PortToggle(GPIO, BOARD_LED_BLUE_GPIO_PORT, 1UL << BOARD_LED_BLUE_GPIO_PIN);
    }
}
