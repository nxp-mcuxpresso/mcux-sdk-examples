/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
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
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if defined(FSL_FEATURE_SOC_PORT_COUNT) && (FSL_FEATURE_SOC_PORT_COUNT > 0)
#include "fsl_port.h"
#endif
#include "fsl_lpuart.h"
#if configUSE_TICKLESS_IDLE == 2
#include "fsl_lpit.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
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
extern void vPortLpitIsr(void);
IRQn_Type vPortGetLpitIrqn(void);
LPIT_Type *vPortGetLpitBase(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static void Tickless_task(void *pvParameters);
static void SW_task(void *pvParameters);

SemaphoreHandle_t xSWSemaphore = NULL;
/*******************************************************************************
 * Code
 ******************************************************************************/

#if configUSE_TICKLESS_IDLE == 2
/*!
 * @brief Interrupt service fuction of LPT timer.
 *
 * This function to call low power timer ISR
 */
void M4_LPIT_IRQHandler(void)
{
    vPortLpitIsr();
}

/*!
 * @brief Fuction of LPT timer.
 *
 * This function to return LPT timer base address
 */

LPIT_Type *vPortGetLpitBase(void)
{
    return CM4__LPIT;
}

/*!
 * @brief Fuction of LPT timer.
 *
 * This function to return LPT timer interrupt number
 */

IRQn_Type vPortGetLpitIrqn(void)
{
    return M4_LPIT_IRQn;
}
#endif
/*!
 * @brief Main function
 */
int main(void)
{
/* Define the init structure for the input switch pin */
#ifdef BOARD_SW_NAME
    gpio_pin_config_t sw_config = {
        kGPIO_DigitalInput,
        0,
#if defined(FSL_FEATURE_SOC_IGPIO_COUNT) && (FSL_FEATURE_SOC_IGPIO_COUNT > 0)
        kGPIO_IntRisingEdge,
#endif
    };
#endif
    sc_ipc_t ipc;
#if configUSE_TICKLESS_IDLE == 2
    lpit_config_t lpitConfig;
    lpit_chnl_params_t lpitChannelConfig;
#endif

    ipc = BOARD_InitRpc();
    BOARD_InitPins(ipc);
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitMemory();

#if configUSE_TICKLESS_IDLE == 2
    /* Power on Peripherals. */
    if (sc_pm_set_resource_power_mode(ipc, SC_R_M4_0_PIT, SC_PM_PW_MODE_ON) != SC_ERR_NONE)
    {
        PRINTF("Error: Failed to power on LPIT\r\n");
    }

    /* Set peripheral's clock. */
    if (CLOCK_SetIpFreq(kCLOCK_M4_0_Lpit, SC_66MHZ) == 0)
    {
        PRINTF("Error: Failed to set LPIT frequency\r\n");
    }

    /* Init lpit module */
    lpitConfig.enableRunInDebug = false;
    lpitConfig.enableRunInDoze  = true;
    LPIT_Init(CM4__LPIT, &lpitConfig);

    lpitChannelConfig.chainChannel          = false;
    lpitChannelConfig.enableReloadOnTrigger = false;
    lpitChannelConfig.enableStartOnTrigger  = false;
    lpitChannelConfig.enableStopOnTimeout   = false;
    lpitChannelConfig.timerMode             = kLPIT_PeriodicCounter;
    /* Set default values for the trigger source */
    lpitChannelConfig.triggerSelect = kLPIT_Trigger_TimerChn0;
    lpitChannelConfig.triggerSource = kLPIT_TriggerSource_External;

    /* Init lpit channel 0 */
    LPIT_SetupChannel(CM4__LPIT, kLPIT_Chnl_0, &lpitChannelConfig);
    /* Enable timer interrupts for channel 0 */
    LPIT_EnableInterrupts(CM4__LPIT, kLPIT_Channel0TimerInterruptEnable);
#endif

    PRINTF("Press any key to start the example\r\n");
    GETCHAR();

    /* Print a note to terminal. */
    PRINTF("Tickless Demo example\r\n");
#ifdef BOARD_SW_NAME
    PRINTF("Press or turn on %s to wake up the CPU\r\n", BOARD_SW_NAME);

/* Init input switch GPIO. */
#if defined(FSL_FEATURE_SOC_PORT_COUNT) && (FSL_FEATURE_SOC_PORT_COUNT > 0)
    PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
#endif

#if defined(__CORTEX_M)
    NVIC_SetPriority(BOARD_SW_IRQ, SW_NVIC_PRIO);
#else
    GIC_SetPriority(BOARD_SW_IRQ, BOARD_SW_GIC_PRIO);
#endif

    EnableIRQ(BOARD_SW_IRQ);
    GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);
#if defined(FSL_FEATURE_SOC_IGPIO_COUNT) && (FSL_FEATURE_SOC_IGPIO_COUNT > 0)
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    GPIO_PortEnableInterrupts(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
#endif
#endif

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
    /*Task Scheduler*/
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
#ifdef BOARD_SW_NAME
void BOARD_SW_IRQ_HANDLER(void)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

/* Clear external interrupt flag. */
#ifdef BOARD_SW_DELAY
    volatile uint32_t i = 0;
    for (i = 0; i < 10000000; ++i)
    {
        __NOP(); /* delay */
    }
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    if (1 == GPIO_PinRead(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN))
    {
        xSemaphoreGiveFromISR(xSWSemaphore, &xHigherPriorityTaskWoken);
    }
#else
    GPIO_PortClearInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
    xSemaphoreGiveFromISR(xSWSemaphore, &xHigherPriorityTaskWoken);
#endif
}
#endif
