/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_rtc.h"
#include "demo_config.h"
#include "board.h"

#include "task.h"
#include "event_groups.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern EventGroupHandle_t g_errorEvent;

rtc_datetime_t g_startDateTime = {
    .year   = 2020U,
    .month  = 9U,
    .day    = 24U,
    .hour   = 19U,
    .minute = 15U,
    .second = 0U,
};
rtc_datetime_t g_stopDateTime;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void rtc_task(void *pvParameters);

/*******************************************************************************
 * Code
 ******************************************************************************/

void rtc_init(void)
{
    PRINTF("\r\nStart datetime: %04d-%02d-%02d %02d:%02d:%02d\r\n", g_startDateTime.year, g_startDateTime.month,
           g_startDateTime.day, g_startDateTime.hour, g_startDateTime.minute, g_startDateTime.second);
    RTC_Init(DEMO_RTC_BASE);
    RTC_StopTimer(DEMO_RTC_BASE);
    RTC_SetDatetime(DEMO_RTC_BASE, &g_startDateTime);
    RTC_StartTimer(DEMO_RTC_BASE);

    if (xTaskCreate(rtc_task, "RTC task", 1000UL / sizeof(portSTACK_TYPE), NULL, 4U, NULL) != pdPASS)
    {
        PRINTF("RTC task creation failed!\r\n");
        while (1)
            ;
    }
}

static void rtc_task(void *pvParameters)
{
    while (xEventGroupGetBits(g_errorEvent) == 0U)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        GPIO_PortToggle(BOARD_LED_GREEN_GPIO, BOARD_LED_GREEN_GPIO_PORT, 1U << BOARD_LED_GREEN_GPIO_PIN);
    }
    RTC_GetDatetime(DEMO_RTC_BASE, &g_stopDateTime);
    PRINTF("\r\nStop datetime : %04d-%02d-%02d %02d:%02d:%02d\r\n", g_stopDateTime.year, g_stopDateTime.month,
           g_stopDateTime.day, g_stopDateTime.hour, g_stopDateTime.minute, g_stopDateTime.second);
    vTaskSuspend(NULL);
}
