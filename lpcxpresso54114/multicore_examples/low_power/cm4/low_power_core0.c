/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_mailbox.h"
#include "static_queue.h"
#include "low_power.h"
#include "fsl_power.h"
#include <stdio.h>

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_EXCLUDE_FROM_DEEPSLEEP                                                                         \
    (SYSCON_PDRUNCFG_PDEN_SRAM0_MASK | SYSCON_PDRUNCFG_PDEN_SRAM1_MASK | SYSCON_PDRUNCFG_PDEN_SRAM2_MASK | \
     SYSCON_PDRUNCFG_PDEN_SRAMX_MASK | SYSCON_PDRUNCFG_PDEN_WDT_OSC_MASK)

#define APP_DISABLE_PINS  \
    IOCON->PIO[0][3] = 0; \
    IOCON->PIO[1][6] = 0;

/* Address of RAM, where the image for core1 should be copied */
#define CORE1_BOOT_ADDRESS 0x20010000

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
extern uint32_t core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
static static_queue_t *volatile g_dataBuff = NULL;
static bool volatile g_goToDeepSleep       = false;

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

static void processData(void)
{
    if (static_queue_size((static_queue_t *)g_dataBuff) < QUEUE_ELEMENT_COUNT_TRIG)
    {
        return;
    }

    (void)PRINTF("Processing...\n");

    /* Compute average values of temperature and pressure */
    int32_t i;
    sensor_data_t *data;
    int32_t avgTemp   = 0;
    uint32_t avgPress = 0;
    for (i = 0; i < QUEUE_ELEMENT_COUNT_TRIG; i++)
    {
        data = (sensor_data_t *)static_queue_get((static_queue_t *)g_dataBuff);
        if (data != NULL)
        {
            avgTemp += data->temp;
            avgPress += data->press;
        }
        else
        {
            (void)PRINTF("Error: data corrupted.\n");
            return;
        }
    }
    avgTemp /= QUEUE_ELEMENT_COUNT_TRIG;
    avgPress /= QUEUE_ELEMENT_COUNT_TRIG;

    /* Format temperature - add decimal point */
    char sbuff[10] = {'\0'};
    int32_t len    = 0;
    len            = (int32_t)sprintf(sbuff, "%d", (int)avgTemp);
    if (len > 2)
    {
        sbuff[len]     = sbuff[len - 1];
        sbuff[len - 1] = sbuff[len - 2];
        sbuff[len - 2] = '.';
    }
    else
    {
        sbuff[0] = '-';
    }

    (void)PRINTF("Average values:\r\ntemperature: %s C, pressure: %dPa\r\n", sbuff, (int)avgPress);
    (void)PRINTF("mag: x: %d, y: %d, z: %d, R: %d\r\n", data->mag.datax, data->mag.datay, data->mag.dataz,
                 data->mag.resistance);
    (void)PRINTF("accel: x: %d, y: %d, z: %d\r\n", data->accel.x, data->accel.y, data->accel.z);
    (void)PRINTF("gyro: x: %d, y: %d, z: %d\r\n", data->gyro.x, data->gyro.y, data->gyro.z);
    (void)PRINTF("\r\n");
}

void MAILBOX_IRQHandler(void)
{
    if (g_dataBuff == NULL)
    {
        g_dataBuff = (static_queue_t *)MAILBOX_GetValue(MAILBOX, kMAILBOX_CM4);
    }
    else
    {
        core_cmd_t cmd = (core_cmd_t)MAILBOX_GetValue(MAILBOX, kMAILBOX_CM4);
        if (cmd == kGoToDeepSleep)
        {
            g_goToDeepSleep = true;
        }
    }
    MAILBOX_ClearValueBits(MAILBOX, kMAILBOX_CM4, 0xffffffffu);
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockFRO12M();
    BOARD_InitDebugConsole();

    /* TURN OFF FLASH Clock (only needed for FLASH programming, will be turned on by ROM API) */
    CLOCK_DisableClock(kCLOCK_Flash);

    (void)PRINTF("CORE0 is running\n");

    /* Minimize power consumption by switching several pins to analog mode */
    APP_DISABLE_PINS;

    /* Init Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image */
    uint32_t core1_image_size;
    core1_image_size = get_core1_image_size();
    (void)PRINTF("Copy CORE1 image to address: 0x%x, size: %d\r\n", (void *)(char *)CORE1_BOOT_ADDRESS,
                 core1_image_size);

    /* Copy application from FLASH to RAM */
    (void)memcpy((void *)(char *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#endif

    (void)PRINTF("Starting CORE1\n");

    RESET_SlaveCoreReset(*((uint32_t *)CORE1_BOOT_ADDRESS + 1u), *(uint32_t *)(CORE1_BOOT_ADDRESS));

    /* Wait for initializing data buffer from core1 */
    while (g_dataBuff == NULL)
    {
    }

    (void)PRINTF("CORE1 is running, CORE0 is waiting for interrupt from CORE1.\n");

    for (;;)
    {
        if (g_goToDeepSleep)
        {
            /* Enter to deep-sleep mode */
            POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP);
            g_goToDeepSleep = false;
        }

        /* Process data from data buffer */
        processData();

        /* Send interrupt to CM0 to turn off flash to decrease consumption when CM4 will be in sleep by WFI */
        MAILBOX_SetValue(MAILBOX, kMAILBOX_CM0Plus, (uint32_t)kTurnOffFlash);

        __WFI();
    }
}
