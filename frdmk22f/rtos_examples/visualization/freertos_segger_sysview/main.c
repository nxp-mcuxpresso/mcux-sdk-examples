/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "SEGGER_SYSVIEW.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

/* Freescale includes. */
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/* Include internal header to get SEGGER_RTT_CB */
#include "SEGGER_RTT.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SYSVIEW_DEVICE_NAME "FRDMK22F Cortex-M4"
#define SYSVIEW_RAM_BASE    (0x1FFF0000)

extern SEGGER_RTT_CB _SEGGER_RTT;
extern const SEGGER_SYSVIEW_OS_API SYSVIEW_X_OS_TraceAPI;

/* The application name to be displayed in SystemViewer */
#ifndef SYSVIEW_APP_NAME
#define SYSVIEW_APP_NAME "SDK System view example"
#endif

/* The target device name */
#ifndef SYSVIEW_DEVICE_NAME
#define SYSVIEW_DEVICE_NAME "Generic Cortex device"
#endif

/* Frequency of the timestamp. Must match SEGGER_SYSVIEW_GET_TIMESTAMP in SEGGER_SYSVIEW_Conf.h */
#define SYSVIEW_TIMESTAMP_FREQ (configCPU_CLOCK_HZ)

/* System Frequency. SystemcoreClock is used in most CMSIS compatible projects. */
#define SYSVIEW_CPU_FREQ configCPU_CLOCK_HZ

/* The lowest RAM address used for IDs (pointers) */
#ifndef SYSVIEW_RAM_BASE
#define SYSVIEW_RAM_BASE 0x20000000
#endif

/* Task priorities. */
#define FIRST_TASK_PRIORITY  (configMAX_PRIORITIES - 1)
#define SECOND_TASK_PRIORITY (configMAX_PRIORITIES - 2)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void first_task(void *pvParameters);
static void second_task(void *pvParameters);

#define APP_DELAY 100

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief System View callback
 */
static void _cbSendSystemDesc(void)
{
    SEGGER_SYSVIEW_SendSysDesc("N=" SYSVIEW_APP_NAME ",D=" SYSVIEW_DEVICE_NAME ",O=FreeRTOS");
    SEGGER_SYSVIEW_SendSysDesc("I#15=SysTick");
}

/*!
 * @brief System View configuration
 */
void SEGGER_SYSVIEW_Conf(void)
{
    SEGGER_SYSVIEW_Init(SYSVIEW_TIMESTAMP_FREQ, SYSVIEW_CPU_FREQ, &SYSVIEW_X_OS_TraceAPI, _cbSendSystemDesc);
    SEGGER_SYSVIEW_SetRAMBase(SYSVIEW_RAM_BASE);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    SEGGER_SYSVIEW_Conf();

    if (xTaskCreate(first_task, "first_task", configMINIMAL_STACK_SIZE + 100, NULL, FIRST_TASK_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    while (1)
        ;
}

/*!
 * @brief First task, higher priority.
 */
static void first_task(void *pvParameters)
{
    if (xTaskCreate(second_task, "second_task", configMINIMAL_STACK_SIZE + 100, NULL, SECOND_TASK_PRIORITY, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        vTaskSuspend(NULL);
    }

    PRINTF("RTT block address is: 0x%x \r\n", &_SEGGER_RTT);

    /* dummy code, print counter and delay */
    for (int counter = 0;; counter++)
    {
        SEGGER_SYSVIEW_PrintfTarget("first task counter: %d ", counter++);
        vTaskDelay(APP_DELAY);
    }
}

/*!
 * @brief Second task, lower priority.
 */
static void second_task(void *pvParameters)
{
    while (1)
    {
        /* dummy code, notify HOST when int16 underflow */
        for (uint16_t j = (uint16_t)-1;; j--)
        {
            if (j == 0)
            {
                SEGGER_SYSVIEW_Warn("second task int underflow");
            }
            __NOP();
        }
    }
}

/*!
 * @brief Apply only for M0plus core. M4/M3 uses cycle counter.
 */
U32 SEGGER_SYSVIEW_X_GetTimestamp(void)
{
    return __get_IPSR() ? xTaskGetTickCountFromISR() : xTaskGetTickCount();
}
