/*
 * FreeRTOS Pre-Release V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
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
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Non-Secure callable functions. */
#include "nsc_functions.h"

#include "clock_config.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


#if defined(__ARMCC_VERSION)
/* Externs needed by MPU setup code.
 * Must match the memory map as specified
 * in scatter file. */
/* Privileged flash. */
extern unsigned int Image$$ER_priv_func$$Base[];
extern unsigned int Image$$ER_priv_func$$Limit[];

extern unsigned int Image$$ER_sys_calls$$Base[];
extern unsigned int Image$$ER_sys_calls$$Limit[];

extern unsigned int Image$$ER_m_text$$Base[];
extern unsigned int Image$$ER_m_text$$Limit[];

extern unsigned int Image$$RW_priv_data$$Base[];
extern unsigned int Image$$RW_priv_data$$Limit[];

const uint32_t *__privileged_functions_start__ = (uint32_t *)Image$$ER_priv_func$$Base;
const uint32_t *__privileged_functions_end__ =
    (uint32_t *)Image$$ER_priv_func$$Limit; /* Last address in privileged Flash region. */

/* Flash containing system calls. */
const uint32_t *__syscalls_flash_start__ = (uint32_t *)Image$$ER_sys_calls$$Base;
const uint32_t *__syscalls_flash_end__ =
    (uint32_t *)Image$$ER_sys_calls$$Limit; /* Last address in Flash region containing system calls. */

/* Unprivileged flash. Note that the section containing
 * system calls is unprivilged so that unprivleged tasks
 * can make system calls. */
const uint32_t *__unprivileged_flash_start__ = (uint32_t *)Image$$ER_sys_calls$$Limit;
const uint32_t *__unprivileged_flash_end__ =
    (uint32_t *)Image$$ER_m_text$$Limit; /* Last address in un-privileged Flash region. */

/* 512 bytes (0x200) of RAM starting at 0x30008000 is
 * priviledged access only. This contains kernel data. */
const uint32_t *__privileged_sram_start__ = (uint32_t *)Image$$RW_priv_data$$Base;
const uint32_t *__privileged_sram_end__ = (uint32_t *)Image$$RW_priv_data$$Limit; /* Last address in privileged RAM. */

#endif
/*-----------------------------------------------------------*/

/**
 * @brief The counter incremented in the callback which is called from the
 * secure side.
 */
static uint32_t ulNonSecureCounter = 0;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Creates all the tasks for this demo.
 */
static void prvCreateTasks(void);

/**
 * @brief Increments the ulNonSecureCounter.
 *
 * This function is called from the secure side.
 */
static void prvCallback(void);

/**
 * @brief Implements the task which calls a secure side function.
 *
 * It passes a callback function to the secure side function which is in-turn
 * called from the secure side. It also toggles the on-board green LED to
 * provide visual feedback that everything is working as expected.
 *
 * @param pvParameters[in] Parameters as passed during task creation.
 */
static void prvSecureCallingTask(void *pvParameters);

/**
 * @brief Implements the task which calls a secure side function to toggle the
 * on-board blue LED.
 *
 * @param pvParameters[in] Parameters as passed during task creation.
 */
static void prvLEDTogglingTask(void *pvParameters);

/**
 * @brief Stack overflow hook.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
/*-----------------------------------------------------------*/

/*******************************************************************************
 * Code
 ******************************************************************************/

void SystemInit(void)
{
#if ((__FPU_PRESENT == 1) && (__FPU_USED == 1))
    SCB->CPACR |= ((3UL << 10 * 2) | (3UL << 11 * 2)); /* set CP10, CP11 Full Access */
#endif                                                 /* ((__FPU_PRESENT == 1) && (__FPU_USED == 1)) */

    SCB->CPACR |= ((3UL << 0 * 2) | (3UL << 1 * 2)); /* set CP0, CP1 Full Access (enable PowerQuad) */

    SCB->NSACR |= ((3UL << 0) | (3UL << 10)); /* enable CP0, CP1, CP10, CP11 Non-secure Access */
}


static void prvCreateTasks(void)
{
    /* Create the secure calling task. */
    (void)xTaskCreate(prvSecureCallingTask,           /* The function that implements the demo task. */
                      "ScCall",                       /* The name to assign to the task being created. */
                      configMINIMAL_STACK_SIZE + 100, /* The size, in WORDS (not bytes), of the stack to allocate for
                                                   the task being created. */
                      NULL,                           /* The task parameter is not being used. */
                      portPRIVILEGE_BIT | tskIDLE_PRIORITY, /* The priority at which the task being created will run. */
                      NULL);

    /* Create the LED toggling task. */
    (void)xTaskCreate(prvLEDTogglingTask,             /* The function that implements the demo task. */
                      "LedToggle",                    /* The name to assign to the task being created. */
                      configMINIMAL_STACK_SIZE + 100, /* The size, in WORDS (not bytes), of the stack to allocate for
                                                   the task being created. */
                      NULL,                           /* The task parameter is not being used. */
                      portPRIVILEGE_BIT | tskIDLE_PRIORITY, /* The priority at which the task being created will run. */
                      NULL);
}
/*-----------------------------------------------------------*/

static void prvCallback(void)
{
    /* This function is called from the secure side. Just increment the counter
     * here. The check that this counter keeps incrementing is performed in the
     * prvSecureCallingTask. */
    ulNonSecureCounter += 1;
}
/*-----------------------------------------------------------*/

static void prvSecureCallingTask(void *pvParameters)
{
    uint32_t ulLastSecureCounter = 0, ulLastNonSecureCounter = 0;
    uint32_t ulCurrentSecureCounter = 0;

    /* This task calls secure side functions. So allocate a
     * secure context for it. */
    portALLOCATE_SECURE_CONTEXT(configMINIMAL_SECURE_STACK_SIZE);

    for (;;)
    {
        /* Call the secure side function which does two things:
         * - It calls the supplied function (prvCallback) which in turn
         *   increments the non-secure counter.
         * - It increments the secure counter and returns the incremented value.
         * Therefore at the end of this function call both the secure and
         * the non-secure counters must have been incremented.
         */
        ulCurrentSecureCounter = NSCFunction(prvCallback);

        /* Make sure that both the counters are incremented. */
        configASSERT(ulCurrentSecureCounter == ulLastSecureCounter + 1);
        configASSERT(ulNonSecureCounter == ulLastNonSecureCounter + 1);

        /* Call the secure side function to toggle green LED to indicate that
         * everything is working as expected. The configASSERT above is defined
         * to an infinite loop and hence the LED will stop blinking if any of
         * the assert fails. */
        vToggleGreenLED();

        /* Update the last values for both the counters. */
        ulLastSecureCounter    = ulCurrentSecureCounter;
        ulLastNonSecureCounter = ulNonSecureCounter;

        /* Wait for a second. */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*-----------------------------------------------------------*/

static void prvLEDTogglingTask(void *pvParameters)
{
    /* This task calls secure side functions. So allocate a
     * secure context for it. */
    portALLOCATE_SECURE_CONTEXT(configMINIMAL_SECURE_STACK_SIZE);

    for (;;)
    {
        /* Call the secure side function to toggle the on-board blue LED. */
        vToggleBlueLED();

        /* Wait for a second. */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE + 100];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle
     * task's state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE + 100;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    /* If the buffers to be provided to the Timer task are declared inside this
     * function then they must be declared static - otherwise they will be allocated on
     * the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;
    static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
     * task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
     * Note that, as the array is necessarily of type StackType_t,
     * configTIMER_TASK_STACK_DEPTH is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
/*-----------------------------------------------------------*/

/* Stack overflow hook. */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    /* Silence warning about unused parameters. */
    (void)xTask;

    /*Force an assert. */
    configASSERT(pcTaskName == 0);
}
/*-----------------------------------------------------------*/

void vGetRegistersFromStack(uint32_t *pulFaultStackAddress)
{
    /* These are volatile to try and prevent the compiler/linker optimising them
     * away as the variables never actually get used.  If the debugger won't show the
     * values of the variables, make them global my moving their declaration outside
     * of this function. */
    volatile uint32_t r0;
    volatile uint32_t r1;
    volatile uint32_t r2;
    volatile uint32_t r3;
    volatile uint32_t r12;
    volatile uint32_t lr;  /* Link register. */
    volatile uint32_t pc;  /* Program counter. */
    volatile uint32_t psr; /* Program status register. */

    r0 = pulFaultStackAddress[0];
    r1 = pulFaultStackAddress[1];
    r2 = pulFaultStackAddress[2];
    r3 = pulFaultStackAddress[3];

    r12 = pulFaultStackAddress[4];
    lr  = pulFaultStackAddress[5];
    pc  = pulFaultStackAddress[6];
    psr = pulFaultStackAddress[7];

    /* Remove compiler warnings about the variables not being used. */
    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;
    (void)r12;
    (void)lr;  /* Link register. */
    (void)pc;  /* Program counter. */
    (void)psr; /* Program status register. */

    /* When the following line is hit, the variables contain the register values. */
    for (;;)
    {
    }
}
/*-----------------------------------------------------------*/

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
/**
 * @brief The fault handler implementation calls a function called
 * vGetRegistersFromStack().
 */
void MemManage_Handler(void)
{
    __asm volatile(
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, handler2_address_const                            \n"
        " bx r1                                                     \n"
        "                                                           \n"
        " handler2_address_const: .word vGetRegistersFromStack      \n");
}
#endif
/*-----------------------------------------------------------*/

/* Non-secure main(). */
/*!
 * @brief Main function
 */
int main(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* Get the updated SystemCoreClock from the secure side */
    SystemCoreClock = getSystemCoreClock();
    /* Create tasks. */
    prvCreateTasks();

    /* Start the schedular. */
    vTaskStartScheduler();

    /* Should not reach here as the schedular is
     * already started. */
    for (;;)
    {
    }
}
/*-----------------------------------------------------------*/
