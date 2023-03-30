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
/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS (void *)0x20200000

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
extern unsigned char core1_image_start[];
#define CORE1_IMAGE_START core1_image_start
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
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

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
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

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)&core1_image_start;
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif
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
void vGeneratePrimaryToSecondaryInterrupt(void *xUpdatedMessageBuffer)
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
        (void)xMessageBufferSendCompletedFromISR(xSecondaryToPrimaryMessageBuffer, &xHigherPriorityTaskWoken);
    }

    /* Normal FreeRTOS "yield from interrupt" semantics, where
    HigherPriorityTaskWoken is initialzed to pdFALSE and will then get set to
    pdTRUE if the interrupt unblocks a task that has a priority above that of
    the currently executing task. */
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    /* No need to clear the interrupt flag here, it is handled by the mcmgr. */
}

static void RemoteAppReadyEventHandler(uint16_t eventData, void *context)
{
    uint16_t *data = (uint16_t *)context;

    *data = eventData;
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
    size_t xReceivedBytes;

    (void)memset((void *)(char *)APP_SH_MEM_BASE, 0, SH_MEM_TOTAL_SIZE);
    /* Create the Primary-To-Secondary message buffer, statically allocated at a known location
       as both cores need to know where they are. */
    xPrimaryToSecondaryMessageBuffer = xMessageBufferCreateStatic(
        /* The buffer size in bytes. */
        APP_MESSAGE_BUFFER_SIZE,
        /* Statically allocated buffer storage area. */
        &ucPrimaryToSecondaryBufferStorage,
        /* Message buffer handle. */
        &xPrimaryToSecondaryMessageBufferStruct);

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* This section ensures the secondary core image is copied from flash location to the target RAM memory.
       It consists of several steps: image size calculation and image copying.
       These steps are not required on MCUXpresso IDE which copies the secondary core image to the target memory during
       startup automatically. */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    (void)PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS,
                 core1_image_size);

    /* Copy application from FLASH to RAM */
    (void)memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Initialize MCMGR before calling its API */
    (void)MCMGR_Init();

    /* Register the application event before starting the secondary core */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RemoteAppReadyEventHandler,
                              (void *)&RemoteAppReadyEventData);

    /* Boot Secondary core application */
    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, 0, kMCMGR_Start_Synchronous);

    /* Wait until the secondary core application signals it is ready to communicate. */
    while (APP_READY_EVENT_DATA != RemoteAppReadyEventData)
    {
    };

    (void)MCMGR_RegisterEvent(kMCMGR_FreeRtosMessageBuffersEvent, FreeRtosMessageBuffersEventHandler, ((void *)0));

    /* Send the first message to the secondary core app. */
    msg.DATA = 0U;
    (void)xMessageBufferSend(xPrimaryToSecondaryMessageBuffer, (void *)&msg, sizeof(THE_MESSAGE), 0);

    while (msg.DATA <= 100U)
    {
        xReceivedBytes =
            xMessageBufferReceive(xSecondaryToPrimaryMessageBuffer, (void *)&msg, sizeof(THE_MESSAGE), portMAX_DELAY);

        (void)PRINTF("Primary core received a msg\r\n");
        (void)PRINTF("Message: Size=%x, DATA = %i\r\n", xReceivedBytes, msg.DATA);
        msg.DATA++;

        (void)xMessageBufferSend(xPrimaryToSecondaryMessageBuffer, (void *)&msg, sizeof(THE_MESSAGE), 0);
    }

    vMessageBufferDelete(xPrimaryToSecondaryMessageBuffer);

    /* Print the ending banner */
    (void)PRINTF("\r\nFreeRTOS Message Buffers demo ends\r\n");
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
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Print the initial banner */
    (void)PRINTF("\r\nFreeRTOS Message Buffers demo starts\r\n");

    xTaskCreateStatic(app_task, "APP_TASK", APP_TASK_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1U, xStack, &xTaskBuffer);

    vTaskStartScheduler();

    (void)PRINTF("Failed to start FreeRTOS on core0.\r\n");
    for (;;)
    {
    }
}
