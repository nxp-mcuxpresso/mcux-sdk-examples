/*
 * Copyright 2016-2022 NXP
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include "fsl_debug_console.h"
#include "fsl_rgpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if configUSE_TICKLESS_IDLE == 2
#include "fsl_gpt.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BOARD_SW_GPIO        BOARD_USER_BUTTON_GPIO
#define BOARD_SW_GPIO_PIN    BOARD_USER_BUTTON_GPIO_PIN
#define BOARD_SW_IRQ         BOARD_USER_BUTTON_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_USER_BUTTON_IRQ_HANDLER
#define BOARD_SW_NAME        BOARD_USER_BUTTON_NAME
#define BOARD_SW_INT_OUTPUT  kRGPIO_InterruptOutput0
/* Task priorities. */
/* clang-format off */
#define tickless_task_PRIORITY   ( configMAX_PRIORITIES - 2 )
#define SW_task_PRIORITY   ( configMAX_PRIORITIES - 1 )
#define TIME_DELAY_SLEEP      5000

/* Interrupt priorities. */
#define SW_NVIC_PRIO 2

/* clang-format on */
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void vPortGptIsr(void);
IRQn_Type vPortGetGptIrqn(void);
GPT_Type *vPortGetGptBase(void);
static void Tickless_task(void *pvParameters);
static void SW_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
SemaphoreHandle_t xSWSemaphore = NULL;

/*******************************************************************************
 * Code
 ******************************************************************************/

#if configUSE_TICKLESS_IDLE == 2
/*!
 * @brief Interrupt service fuction of GPT timer.
 *
 * This function to call low power timer ISR
 */
void GPT1_IRQHandler(void)
{
    vPortGptIsr();
}

/*!
 * @brief Fuction of GPT timer.
 *
 * This function to return GPT timer base address
 */

GPT_Type *vPortGetGptBase(void)
{
    return GPT1;
}

/*!
 * @brief Fuction of GPT timer.
 *
 * This function to return GPT timer interrupt number
 */

IRQn_Type vPortGetGptIrqn(void)
{
    return GPT1_IRQn;
}
#endif
/*!
 * @brief Main function
 */
int main(void)
{
    /* Define the init structure for the input switch pin */
    rgpio_pin_config_t sw_config = {
        kRGPIO_DigitalInput,
        0,
    };

#if configUSE_TICKLESS_IDLE == 2
    gpt_config_t gptConfig;
#endif
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Workaround: Disable interrupt which might be enabled by ROM. */
    RGPIO_SetPinInterruptConfig(RGPIO1, 9U, kRGPIO_InterruptOutput0, kRGPIO_InterruptOrDMADisabled);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    NVIC_ClearPendingIRQ(GPIO1_0_IRQn);

#if configUSE_TICKLESS_IDLE == 2
    GPT_GetDefaultConfig(&gptConfig);

    /* Initialize GPT module */
    GPT_Init(GPT1, &gptConfig);

    /* Divide GPT clock source frequency by 1 inside GPT module */
    GPT_SetClockDivider(GPT1, 1);

    /* Enable GPT Output Compare1 interrupt */
    GPT_EnableInterrupts(GPT1, kGPT_OutputCompare1InterruptEnable);

    /* Enable at the Interrupt */
    EnableIRQ(GPT1_IRQn);
#endif

    PRINTF("Press any key to start the example\r\n");
    GETCHAR();

    /* Print a note to terminal. */
    PRINTF("Tickless Demo example\r\n");
#ifdef BOARD_SW_NAME
    PRINTF("Press or turn on %s to wake up the CPU\r\n", BOARD_SW_NAME);
#endif
    /* Init input switch GPIO. */
    RGPIO_SetPinInterruptConfig(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, BOARD_SW_INT_OUTPUT, kRGPIO_InterruptFallingEdge);
    NVIC_SetPriority(BOARD_SW_IRQ, SW_NVIC_PRIO);
    EnableIRQ(BOARD_SW_IRQ);
    RGPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);

    /*Create tickless task*/
    if (xTaskCreate(Tickless_task, "Tickless_task", configMINIMAL_STACK_SIZE + 100, NULL, tickless_task_PRIORITY,
                    NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    if (xTaskCreate(SW_task, "Switch_task", configMINIMAL_STACK_SIZE + 100, NULL, SW_task_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    PRINTF("\r\nTick count :\r\n");
    vTaskStartScheduler();
    for (;;)
        ;
}

/* Tickless Task */
static void Tickless_task(void *pvParameters)
{
    for (;;)
    {
        PRINTF("%d\r\n", xTaskGetTickCount());
        vTaskDelay(TIME_DELAY_SLEEP);
    }
}

/* Switch Task */
static void SW_task(void *pvParameters)
{
    xSWSemaphore = xSemaphoreCreateBinary();
    for (;;)
    {
        if (xSemaphoreTake(xSWSemaphore, portMAX_DELAY) == pdTRUE)
        {
            PRINTF("CPU woken up by external interrupt\r\n");
        }
    }
}

/*!
 * @brief Interrupt service fuction of switch.
 *
 * This function to wake up CPU
 */
void BOARD_SW_IRQ_HANDLER(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    /* Clear external interrupt flag. */
    RGPIO_ClearPinsInterruptFlags(BOARD_SW_GPIO, BOARD_SW_INT_OUTPUT, 1U << BOARD_SW_GPIO_PIN);
    xSemaphoreGiveFromISR(xSWSemaphore, &xHigherPriorityTaskWoken);
}
