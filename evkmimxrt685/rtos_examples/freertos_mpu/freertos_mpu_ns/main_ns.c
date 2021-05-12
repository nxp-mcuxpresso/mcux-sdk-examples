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

#include "fsl_common.h"
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

/* Memory shared between two tasks. */
static uint8_t ucSharedMemory[32] __attribute__((aligned(32)));

/* Counter incremented in the callback which is
 * called from the secure side. */
static uint32_t ulNonSecureCounter __attribute__((aligned(32))) = 0;
/*-----------------------------------------------------------*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/**
 * @brief Called from the kernel if a stack overflow is detected.
 *
 * @param xTask[in] Task handle of the task for which stack overflow
 *                  is detected.
 * @param pcTaskName[in] The name of the task for which stack overflow
 *                       is detected.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);

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
 * @brief Implements the task which has Read Only access to the
 * memory area ucSharedMemory.
 *
 * @param pvParameters[in] Parameters as passed during task creation.
 */
static void prvROAccessTask(void *pvParameters);

/**
 * @brief Implements the task which has Read Write access to the
 * memory area ucSharedMemory.
 *
 * @param pvParameters[in] Parameters as passed during task creation.
 */
static void prvRWAccessTask(void *pvParameters);

/**
 * @brief Implements the task which calls the secure side functions.
 *
 * @param pvParameters[in] Parameters as passed during task creation.
 */
static void prvSecureCallingTask(void *pvParameters);
/*-----------------------------------------------------------*/

/*******************************************************************************
 * Code
 ******************************************************************************/

void SystemInit(void)
{
}


/* Non-secure main() */
/*!
 * @brief Main function
 */
int main(void)
{
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

static void prvCreateTasks(void)
{
    static StackType_t xROAccessTaskStack[configMINIMAL_STACK_SIZE + 100] __attribute__((aligned(32)));
    static StackType_t xRWAccessTaskStack[configMINIMAL_STACK_SIZE + 100] __attribute__((aligned(32)));
    static StackType_t xSecureCallingTaskStack[configMINIMAL_STACK_SIZE + 100] __attribute__((aligned(32)));
    TaskParameters_t xROAccessTaskParameters = {
        .pvTaskCode     = prvROAccessTask,
        .pcName         = "ROAccess",
        .usStackDepth   = configMINIMAL_STACK_SIZE + 100,
        .pvParameters   = NULL,
        .uxPriority     = tskIDLE_PRIORITY,
        .puxStackBuffer = xROAccessTaskStack,
        .xRegions       = {
            {ucSharedMemory, 32, tskMPU_REGION_READ_ONLY | tskMPU_REGION_EXECUTE_NEVER},
            {0, 0, 0},
            {0, 0, 0},
        }};
    TaskParameters_t xRWAccessTaskParameters = {
        .pvTaskCode     = prvRWAccessTask,
        .pcName         = "RWAccess",
        .usStackDepth   = configMINIMAL_STACK_SIZE + 100,
        .pvParameters   = NULL,
        .uxPriority     = tskIDLE_PRIORITY,
        .puxStackBuffer = xRWAccessTaskStack,
        .xRegions       = {
            {ucSharedMemory, 32, tskMPU_REGION_READ_WRITE | tskMPU_REGION_EXECUTE_NEVER},
            {0, 0, 0},
            {0, 0, 0},
        }};
    TaskParameters_t xSecureCallingTaskParameters = {
        .pvTaskCode     = prvSecureCallingTask,
        .pcName         = "SecCalling",
        .usStackDepth   = configMINIMAL_STACK_SIZE + 100,
        .pvParameters   = NULL,
        .uxPriority     = tskIDLE_PRIORITY,
        .puxStackBuffer = xSecureCallingTaskStack,
        .xRegions       = {
            {&(ulNonSecureCounter), 4, tskMPU_REGION_READ_WRITE | tskMPU_REGION_EXECUTE_NEVER},
            {0, 0, 0},
            {0, 0, 0},
        }};
    /* Create an unpriviledged task with RO access to ucSharedMemory. */
    xTaskCreateRestricted(&(xROAccessTaskParameters), NULL);

    /* Create an unpriviledged task with RW access to ucSharedMemory. */
    xTaskCreateRestricted(&(xRWAccessTaskParameters), NULL);

    /* Create an unpriviledged task which calls secure functions. */
    xTaskCreateRestricted(&(xSecureCallingTaskParameters), NULL);
}
/*-----------------------------------------------------------*/

static void prvCallback(void)
{
    /* This function is called from the secure side.
     * Just increment the counter here. The check that
     * this counter keeps incrementing is performed in
     * the prvSecureCallingTask. */
    ulNonSecureCounter += 1;
}
/*-----------------------------------------------------------*/

static void prvROAccessTask(void *pvParameters)
{
    uint8_t ucVal;

    /* Unused parameters. */
    (void)pvParameters;

    for (;;)
    {
        /* This task has RO access to ucSharedMemory
         * and therefore it can read it but cannot
         * modify it. */
        ucVal = ucSharedMemory[0];

        /* Trying to write to ucSharedMemory results in
         * fault. Therefore uncommenting the following line
         * will result in fault. */
        /* ucSharedMemory[ 0 ] = 0; */

        /* Wait for a second. */
        vTaskDelay(pdMS_TO_TICKS(1000));

        (void)ucVal;
    }
}
/*-----------------------------------------------------------*/

static void prvRWAccessTask(void *pvParameters)
{
    /* Unused parameters. */
    (void)pvParameters;

    for (;;)
    {
        /* This task has RW access to ucSharedMemory
         * and therefore can write to it. */
        ucSharedMemory[0] = 0;

        /* Wait for a second. */
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
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
        /* Call the secure side funtion. It does two things:
         * - It calls the supplied function (prvCallback)
         *   which in turn increments the non-secure counter.
         * - It increments the secure counter and returns
         *   the incremented value.
         * Therefore at the end of this function call both
         * the secure and non-secure counters must have been
         * incremented.
         */
        ulCurrentSecureCounter = NSCFunction(prvCallback);

        /* Make sure that both the counters are incremented. */
        configASSERT(ulCurrentSecureCounter == ulLastSecureCounter + 1);
        configASSERT(ulNonSecureCounter == ulLastNonSecureCounter + 1);

        /* Update the last values for both the counters. */
        ulLastSecureCounter    = ulCurrentSecureCounter;
        ulLastNonSecureCounter = ulNonSecureCounter;

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

void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    /* Silence warning about unused parameters. */
    (void)xTask;

    /* Force an assert. */
    configASSERT(pcTaskName == 0);
}
/*-----------------------------------------------------------*/

void prvGetRegistersFromStack(uint32_t *pulFaultStackAddress)
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

#if defined(__GNUC)
/**
 * @brief The fault handler implementation calls a function called
 * prvGetRegistersFromStack().
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
        " handler2_address_const: .word prvGetRegistersFromStack    \n");
}
/*-----------------------------------------------------------*/
#endif
