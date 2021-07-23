/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "mcmgr.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS 0x20200000

#define APP_ONE_BUTTON_ONLY

#define BUTTON_1_INIT() GPIO_PinInit(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN, &sw_config)
#define IS_BUTTON_1_PRESSED() \
    ((0U == GPIO_PinRead(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN)) && (secondary_core_started == 1))
#define BUTTON_1_NAME BOARD_USER_BUTTON_NAME

#define IS_BUTTON_2_PRESSED() \
    ((0U == GPIO_PinRead(BOARD_USER_BUTTON_GPIO, BOARD_USER_BUTTON_GPIO_PIN)) && (secondary_core_started == 0))

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
#define APP_READY_EVENT_DATA (1U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
static void MCMGR_RemoteCoreUpEventHandler(uint16_t remoteData, void *context);
static void MCMGR_RemoteCoreDownEventHandler(uint16_t remoteData, void *context);
static void MCMGR_RemoteExceptionEventHandler(uint16_t remoteData, void *context);

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
static volatile uint16_t RemoteReadyEventData = 0U;

static void RemoteReadyEventHandler(uint16_t eventData, void *context)
{
    RemoteReadyEventData = eventData;
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

/*!
 * @brief Main function
 */
int main(void)
{
#ifdef APP_ONE_BUTTON_ONLY
    uint32_t secondary_core_started = 0U;
#endif
    volatile uint32_t resetDone       = 0U;
    volatile uint32_t exceptionNumber = 0U;
    volatile uint32_t startupDone     = 0U;
    /* Define the init structure for the switches*/
    gpio_pin_config_t sw_config = {kGPIO_DigitalInput, 0};

    /* Initialize MCMGR, install generic event handlers */
    (void)MCMGR_Init();

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    /* Configure BUTTONs */
    BUTTON_1_INIT();
#ifndef APP_ONE_BUTTON_ONLY
    BUTTON_2_INIT();
#endif
    /* Print the initial banner from Primary core */
    (void)PRINTF("\r\nHello World from the Primary Core!\r\n\n");

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* This section ensures the secondary core image is copied from flash location to the target RAM memory.
       It consists of several steps: image size calculation and image copying.
       These steps are not required on MCUXpresso IDE which copies the secondary core image to the target memory during
       startup automatically. */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    (void)PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS,
                 core1_image_size);

    /* Copy Secondary core application from FLASH to the target memory. */
    (void)memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    /* Install remote core up event handler */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteCoreUpEvent, MCMGR_RemoteCoreUpEventHandler, (void *)&startupDone);

    /* Install remote core down event handler */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteCoreDownEvent, MCMGR_RemoteCoreDownEventHandler, (void *)&resetDone);

    /* Install remote exception event handler */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteExceptionEvent, MCMGR_RemoteExceptionEventHandler, (void *)&exceptionNumber);

    /* Register the application event before starting the secondary core */
    (void)MCMGR_RegisterEvent(kMCMGR_RemoteApplicationEvent, RemoteReadyEventHandler, ((void *)0));

    /* Boot Secondary core application */
    (void)PRINTF("Starting Secondary core.\r\n");
    (void)MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, 2, kMCMGR_Start_Synchronous);
#ifdef APP_ONE_BUTTON_ONLY
    secondary_core_started = 1U;
#endif

    /* Wait until the secondary core application signals that it has been started. */
    while (APP_READY_EVENT_DATA != RemoteReadyEventData)
    {
    };

    (void)PRINTF("The secondary core application has been started.\r\n\r\n");
#ifdef APP_ONE_BUTTON_ONLY
    (void)PRINTF("Press the %s button to toggle Secondary core Start/Stop.\r\n", BUTTON_1_NAME);
#else
    (void)PRINTF("Press the %s button to Stop Secondary core.\r\n", BUTTON_1_NAME);
    (void)PRINTF("Press the %s button to Start Secondary core again.\r\n", BUTTON_2_NAME);
#endif
    (void)PRINTF(
        "When no action is taken the secondary core application crashes intentionally after 100 LED toggles (simulated "
        "exception), generating the RemoteExceptionEvent to this core.\r\n");
    (void)PRINTF("Use the Stop and then the Start button to get it running again.\r\n\r\n");

    for (;;)
    {
        /* Stop secondary core execution. */
        if (IS_BUTTON_1_PRESSED())
        {
            if (kStatus_MCMGR_Success == MCMGR_StopCore(kMCMGR_Core1))
            {
#ifdef APP_ONE_BUTTON_ONLY
                secondary_core_started = 0U;
#endif
                (void)PRINTF("Stopped Secondary core.\r\n");
            }
            else
            {
                (void)PRINTF("Secondary core already stopped!\r\n");
            }
            SDK_DelayAtLeastUs(1000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }
        /* Start core from reset vector */
        if (IS_BUTTON_2_PRESSED())
        {
            if (kStatus_MCMGR_Success ==
                MCMGR_StartCore(kMCMGR_Core1, (void *)(char *)CORE1_BOOT_ADDRESS, 2, kMCMGR_Start_Synchronous))
            {
#ifdef APP_ONE_BUTTON_ONLY
                secondary_core_started = 1U;
#endif
                (void)PRINTF("Started Secondary core.\r\n");
            }
            else
            {
                (void)PRINTF("Secondary core already started!\r\n");
            }
            SDK_DelayAtLeastUs(1000000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }

        if (0U != resetDone)
        {
            (void)PRINTF("Secondary core HW reset executed.\r\n");
            resetDone = 0;
        }

        if (0U != exceptionNumber)
        {
            (void)PRINTF("Secondary core is in exception number %d.\r\n", exceptionNumber);
            exceptionNumber = 0;
        }

        if (0U != startupDone)
        {
            (void)PRINTF("Secondary core is in startup code.\r\n");
            startupDone = 0;
        }
    }
}

static void MCMGR_RemoteCoreUpEventHandler(uint16_t remoteData, void *context)
{
    uint32_t *startupDone = (uint32_t *)context;
    *startupDone          = 1U;
}

static void MCMGR_RemoteCoreDownEventHandler(uint16_t remoteData, void *context)
{
    uint32_t *resetDone = (uint32_t *)context;
    *resetDone          = 1U;
}

static void MCMGR_RemoteExceptionEventHandler(uint16_t remoteData, void *context)
{
    uint32_t *exceptionNumber = (uint32_t *)context;
    *exceptionNumber          = (uint32_t)remoteData;
}
