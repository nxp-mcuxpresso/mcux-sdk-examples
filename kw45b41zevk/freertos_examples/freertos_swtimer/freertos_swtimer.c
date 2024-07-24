/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* Standard includes. */
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* The software timer period. */
#define SW_TIMER_PERIOD_MS (1000 / portTICK_PERIOD_MS)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* The callback function. */
static void SwTimerCallback(TimerHandle_t xTimer);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    TimerHandle_t SwTimerHandle = NULL;

    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    SystemCoreClockUpdate();
    /* Create the software timer. */
    SwTimerHandle = xTimerCreate("SwTimer",          /* Text name. */
                                 SW_TIMER_PERIOD_MS, /* Timer period. */
                                 pdTRUE,             /* Enable auto reload. */
                                 0,                  /* ID is not used. */
                                 SwTimerCallback);   /* The callback function. */
    /* Start timer. */
    xTimerStart(SwTimerHandle, 0);
    /* Start scheduling. */
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Software timer callback.
 */
static void SwTimerCallback(TimerHandle_t xTimer)
{
    PRINTF("Tick.\r\n");
}
