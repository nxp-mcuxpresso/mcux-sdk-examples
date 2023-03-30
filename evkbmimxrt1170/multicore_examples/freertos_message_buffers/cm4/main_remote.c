/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "FreeRTOS.h"
#include "message_buffer.h"
#include "task.h"

#include "mcmgr.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_TASK_STACK_SIZE                                (configMINIMAL_STACK_SIZE + 100)
#define APP_READY_EVENT_DATA                               (1U)
#define APP_MESSAGE_BUFFER_EVENT_DATA                      (1U)
#define APP_MESSAGE_BUFFER_SIZE                            (24U)
#define APP_SH_MEM_PRIMARY_TO_SECONDARY_MB_OFFSET          (0x0u)
#define APP_SH_MEM_SECONDARY_TO_PRIMARY_MB_OFFSET          (0x4u)
#define APP_SH_MEM_PRIMARY_TO_SECONDARY_MB_STRUCT_OFFSET   (0x50u)
#define APP_SH_MEM_SECONDARY_TO_PRIMARY_MB_STRUCT_OFFSET   (0xA0u)
#define APP_SH_MEM_PRIMARY_TO_SECONDARY_BUF_STORAGE_OFFSET (0x100u)
#define APP_SH_MEM_SECONDARY_TO_PRIMARY_BUF_STORAGE_OFFSET (0x200u)

typedef struct the_message
{
    uint32_t DATA;
} THE_MESSAGE, *THE_MESSAGE_PTR;

volatile THE_MESSAGE msg = {0};

#define SH_MEM_TOTAL_SIZE (6144U)
#if defined(__ICCARM__) /* IAR Workbench */
extern unsigned char rpmsg_sh_mem_start[];
extern unsigned char rpmsg_sh_mem_end[];
#define APP_SH_MEM_BASE rpmsg_sh_mem_start
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION) /* Keil MDK */
extern unsigned char Image$$RPMSG_SH_MEM$$Base;
extern unsigned char Image$$RPMSG_SH_MEM$$Length;
#define APP_SH_MEM_BASE &Image$$RPMSG_SH_MEM$$Base
#elif defined(__GNUC__) /* GCC */
unsigned char rpmsg_sh_mem[SH_MEM_TOTAL_SIZE] __attribute__((section(".noinit.$rpmsg_sh_mem")));
#define APP_SH_MEM_BASE (uint32_t) & rpmsg_sh_mem
#else
#error "Message Buffers are not placed in shared memory!"
#endif

#define xPrimaryToSecondaryMessageBuffer \
    (*(MessageBufferHandle_t *)(APP_SH_MEM_BASE + APP_SH_MEM_PRIMARY_TO_SECONDARY_MB_OFFSET))
#define xSecondaryToPrimaryMessageBuffer \
    (*(MessageBufferHandle_t *)(APP_SH_MEM_BASE + APP_SH_MEM_SECONDARY_TO_PRIMARY_MB_OFFSET))
#define xPrimaryToSecondaryMessageBufferStruct \
    (*(StaticStreamBuffer_t *)(APP_SH_MEM_BASE + APP_SH_MEM_PRIMARY_TO_SECONDARY_MB_STRUCT_OFFSET))
#define xSecondaryToPrimaryMessageBufferStruct \
    (*(StaticStreamBuffer_t *)(APP_SH_MEM_BASE + APP_SH_MEM_SECONDARY_TO_PRIMARY_MB_STRUCT_OFFSET))
#define ucPrimaryToSecondaryBufferStorage \
    (*(uint8_t *)(APP_SH_MEM_BASE + APP_SH_MEM_PRIMARY_TO_SECONDARY_BUF_STORAGE_OFFSET))
#define ucSecondaryToPrimaryBufferStorage \
    (*(uint8_t *)(APP_SH_MEM_BASE + APP_SH_MEM_SECONDARY_TO_PRIMARY_BUF_STORAGE_OFFSET))
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*
 * configSUPPORT_STATIC_ALLOCATION is set to 1, requiring this callback to
 * provide statically allocated data for use by the idle task, which is a task
 * created by the scheduler when it starts.
 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/*******************************************************************************
 * Code
 ******************************************************************************/
static volatile uint16_t RemoteAppReadyEventData = 0U;
static StaticTask_t xTaskBuffer;
static StackType_t xStack[APP_TASK_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be allocated on
    the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide
    an implementation of vApplicationGetIdleTaskMemory() to provide the memory
    that is used by the Idle task.
    www.freertos.org/a00110.html#configSUPPORT_STATIC_ALLOCATION */

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
void vGenerateSecondaryToPrimaryInterrupt(void *xUpdatedMessageBuffer)
{
    /* Trigger the inter-core interrupt using the MCMGR component.
       Pass the APP_MESSAGE_BUFFER_EVENT_DATA as data that accompany
       the kMCMGR_FreeRtosMessageBuffersEvent event. */
    (void)MCMGR_TriggerEventForce(kMCMGR_FreeRtosMessageBuffersEvent, APP_MESSAGE_BUFFER_EVENT_DATA);
}

static void FreeRtosMessageBuffersEventHandler(uint16_t eventData, void *context)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Make sure the message has been addressed to us. Using eventData that accompany
       the event of the kMCMGR_FreeRtosMessageBuffersEvent type, we can distinguish
       different consumers. */
    if (APP_MESSAGE_BUFFER_EVENT_DATA == eventData)
    {
        /* Call the API function that sends a notification to any task that is
    blocked on the xUpdatedMessageBuffer message buffer waiting for data to
    arrive. */
        (void)xMessageBufferSendCompletedFromISR(xPrimaryToSecondaryMessageBuffer, &xHigherPriorityTaskWoken);
    }

    /* Normal FreeRTOS "yield from interrupt" semantics, where
    HigherPriorityTaskWoken is initialzed to pdFALSE and will then get set to
    pdTRUE if the interrupt unblocks a task that has a priority above that of
    the currently executing task. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    /* No need to clear the interrupt flag here, it is handled by the mcmgr. */
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    /* Initialize MCMGR - low level multicore management library. Call this
       function as close to the reset entry as possible to allow CoreUp event
       triggering. The SystemInitHook() weak function overloading is used in this
       application. */
    (void)MCMGR_EarlyInit();
}

static void app_task(void *param)
{
    /* Create the Secondary-To-Primary message buffer, statically allocated at a known location
       as both cores need to know where they are. */
    xSecondaryToPrimaryMessageBuffer = xMessageBufferCreateStatic(
        /* The buffer size in bytes. */
        APP_MESSAGE_BUFFER_SIZE,
        /* Statically allocated buffer storage area. */
        &ucSecondaryToPrimaryBufferStorage,
        /* Message buffer handle. */
        &xSecondaryToPrimaryMessageBufferStruct);

    /* Print the initial banner */
    (void)PRINTF("\r\nFreeRTOS Message Buffers Ping-Pong Demo...\r\n");

    uint32_t startupData;
    mcmgr_status_t status;

    /* Get the startup data */
    do
    {
        status = MCMGR_GetStartupData(&startupData);
    } while (status != kStatus_MCMGR_Success);

    (void)MCMGR_RegisterEvent(kMCMGR_FreeRtosMessageBuffersEvent, FreeRtosMessageBuffersEventHandler, ((void *)0));

    /* Signal the other core we are ready by triggering the event and passing the APP_READY_EVENT_DATA */
    (void)MCMGR_TriggerEvent(kMCMGR_RemoteApplicationEvent, APP_READY_EVENT_DATA);

    while (msg.DATA <= 100U)
    {
        (void)PRINTF("Waiting for ping...\r\n");

        (void)xMessageBufferReceive(xPrimaryToSecondaryMessageBuffer, (void *)&msg, sizeof(THE_MESSAGE), portMAX_DELAY);

        msg.DATA++;
        (void)PRINTF("Sending pong...\r\n");

        (void)xMessageBufferSend(xSecondaryToPrimaryMessageBuffer, (void *)&msg, sizeof(THE_MESSAGE), 0);
    }

    (void)PRINTF("Ping pong done, deinitializing...\r\n");

    /* Wait until the last message is read by the opposite side */
    while (pdFALSE == xMessageBufferIsEmpty(xSecondaryToPrimaryMessageBuffer))
    {
    }
    vMessageBufferDelete(xSecondaryToPrimaryMessageBuffer);

    (void)PRINTF("Looping forever...\r\n");

    /* End of the example */
    for (;;)
    {
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_ConfigMPU();
    BOARD_InitPins();

    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();

    xTaskCreateStatic(app_task, "APP_TASK", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1U, xStack, &xTaskBuffer);

    vTaskStartScheduler();

    (void)PRINTF("Failed to start FreeRTOS on core1.\r\n");
    for (;;)
    {
    }
}
