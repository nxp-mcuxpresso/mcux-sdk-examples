/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
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

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define TASK_PRIO (configMAX_PRIORITIES - 1)

SemaphoreHandle_t xSemaphore_producer;
SemaphoreHandle_t xSemaphore_consumer;

/* Static memory size definitions */
#define PRODUCER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 166)
#define CONSUMER_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE + 166)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void producer_task(void *pvParameters);
static void consumer_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Statically allocated memory for producer_task stack */
static StackType_t ProducerTaskStack[PRODUCER_TASK_STACK_SIZE];
/* Statically allocated memory for producer_task task control block */
static StaticTask_t ProducerTaskTCB;
/* Statically allocated memory for consumer task stacks */
static StackType_t ConsumerTaskStack0[PRODUCER_TASK_STACK_SIZE];
static StackType_t ConsumerTaskStack1[PRODUCER_TASK_STACK_SIZE];
static StackType_t ConsumerTaskStack2[PRODUCER_TASK_STACK_SIZE];
/* Statically allocated memory for consumer_task task control block */
static StaticTask_t ConsumerTaskTCB0;
static StaticTask_t ConsumerTaskTCB1;
static StaticTask_t ConsumerTaskTCB2;
/* Statically allocated memory for producer semaphore structure */
static StaticSemaphore_t ProducerSemaphoreBuffer;
/* Statically allocated memory for consumer semaphore structure */
static StaticSemaphore_t ConsumerSemaphoreBuffer;

TaskHandle_t TaskHandle = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    TaskHandle = xTaskCreateStatic(producer_task, "PRODUCER_TASK", PRODUCER_TASK_STACK_SIZE, NULL, TASK_PRIO,
                                   &(ProducerTaskStack[0]), &ProducerTaskTCB);
    if (TaskHandle != NULL)
    {
        PRINTF("Producer_task created.\r\n");
    }
    else
    {
        PRINTF("Failed to create producer task");
        while (1)
            ;
    }
    /* Start scheduling. */
    vTaskStartScheduler();
    for (;;)
        ;
}

/*!
 * @brief Task producer_task.
 */
static void producer_task(void *pvParameters)
{
    xSemaphore_producer = xSemaphoreCreateBinaryStatic(&ProducerSemaphoreBuffer);
    if (xSemaphore_producer == NULL)
    {
        PRINTF("xSemaphore_producer creation failed.\r\n");
        vTaskSuspend(NULL);
    }

    xSemaphore_consumer = xSemaphoreCreateBinaryStatic(&ConsumerSemaphoreBuffer);
    if (xSemaphore_consumer == NULL)
    {
        PRINTF("xSemaphore_consumer creation failed.\r\n");
        vTaskSuspend(NULL);
    }

    TaskHandle = xTaskCreateStatic(consumer_task, "CONSUMER_TASK_0", CONSUMER_TASK_STACK_SIZE, (void *)0, TASK_PRIO,
                                   &(ConsumerTaskStack0[0]), &ConsumerTaskTCB0);
    if (TaskHandle != NULL)
    {
        PRINTF("Consumer_task 0 created.\r\n");
    }
    else
    {
        PRINTF("Failed to create consumer_task 0.\r\n");
        vTaskSuspend(NULL);
    }

    TaskHandle = xTaskCreateStatic(consumer_task, "CONSUMER_TASK_1", CONSUMER_TASK_STACK_SIZE, (void *)1, TASK_PRIO,
                                   &(ConsumerTaskStack1[0]), &ConsumerTaskTCB1);
    if (TaskHandle != NULL)
    {
        PRINTF("Consumer_task 1 created.\r\n");
    }
    else
    {
        PRINTF("Failed to create consumer_task 1.\r\n");
        vTaskSuspend(NULL);
    }

    TaskHandle = xTaskCreateStatic(consumer_task, "CONSUMER_TASK_2", CONSUMER_TASK_STACK_SIZE, (void *)2, TASK_PRIO,
                                   &(ConsumerTaskStack2[0]), &ConsumerTaskTCB2);
    if (TaskHandle != NULL)
    {
        PRINTF("Consumer_task 2 created.\r\n");
    }
    else
    {
        PRINTF("Failed to create consumer_task 2.\r\n");
        vTaskSuspend(NULL);
    }

    while (1)
    {
        /* Producer is ready to provide item. */
        xSemaphoreGive(xSemaphore_consumer);
        /* Producer is waiting when consumer will be ready to accept item. */
        if (xSemaphoreTake(xSemaphore_producer, portMAX_DELAY) == pdTRUE)
        {
            PRINTF("Producer released item.\r\n");
        }
        else
        {
            PRINTF("Producer is waiting for customer.\r\n");
        }
    }
}

/*!
 * @brief Task consumer_task.
 */
static void consumer_task(void *pvParameters)
{
    PRINTF("Consumer number: %d\r\n", pvParameters);
    while (1)
    {
        /* Consumer is ready to accept. */
        xSemaphoreGive(xSemaphore_producer);
        /* Consumer is waiting when producer will be ready to produce item. */
        if (xSemaphoreTake(xSemaphore_consumer, portMAX_DELAY) == pdTRUE)
        {
            PRINTF("Consumer %d accepted item.\r\n", pvParameters);
        }
        else
        {
            PRINTF("Consumer %d is waiting for producer.\r\n", pvParameters);
        }
    }
}

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
