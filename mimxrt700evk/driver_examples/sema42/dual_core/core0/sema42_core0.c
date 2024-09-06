/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_sema42.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

#include "core1_support.h"
#include "fsl_cache.h"
#include "fsl_mu.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE0_BOOT_CORE1              0U
#define APP_BOARD_HAS_LED             0U
#define CORE0_BOOT_CORE1_SPECIFIC_WAY 1U

#define APP_MU     MU1_MUA
#define APP_SEMA42 SEMA42_0

#define CORE1_IMAGE_FLUSH_CACHE XCACHE_CleanInvalidateCacheByRange
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U
/* Flag indicates Core 0 has locked the sema42 gate. */
#define SEMA42_LOCK_FLAG 0x02U
/* Flag indicates Core 1 has locked the sema42 gate. */
#define SEMA42_CORE1_LOCK_FLAG 0x03U
/* The SEMA42 gate */
#define SEMA42_GATE 0U

/*
 * Use core 0 to boot core 1.
 * When set to 1, the core 0 assign domain ID for core 0 and core 1, then boot core 1.
 * When set to 0, core 0 and core 1 are boot up by uboot or other component, and
 * they assign their own domain ID seperately.
 */
#ifndef CORE0_BOOT_CORE1
#define CORE0_BOOT_CORE1 1
#endif

/*
 * Use device specific way to boot core 1.
 * When set to 0, Core 0 uses MU to boot Core 1.
 * When set to 1, Core 0 uses a device specific way to boot Core 1.
 */
#ifndef CORE0_BOOT_CORE1_SPECIFIC_WAY
#define CORE0_BOOT_CORE1_SPECIFIC_WAY 0
#endif

/*
 * Use static domain ID or dynamic domain ID.
 */
#ifndef USE_STATIC_DOMAIN_ID
#define USE_STATIC_DOMAIN_ID 1
#endif

/*
 * Use MU peripheral for inter-core notifications.
 */
#ifndef USE_MU_NOTIFICATIONS
#define USE_MU_NOTIFICATIONS 1
#endif

/*
 * The static domain ID used.
 */
#ifndef APP_STATIC_DOMAIN_ID
#define APP_STATIC_DOMAIN_ID 0
#endif

/*
 * The board has LED to show the status.
 */
#ifndef APP_BOARD_HAS_LED
#define APP_BOARD_HAS_LED 1
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void APP_InitCore0Domain(void);
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
#if USE_STATIC_DOMAIN_ID
uint8_t APP_GetCore0DomainID(void)
{
    return APP_STATIC_DOMAIN_ID;
}
#else
uint8_t APP_GetCore0DomainID(void);
#endif

#if CORE0_BOOT_CORE1_SPECIFIC_WAY
void APP_BootCore1(void);
#else
void APP_BootCore1(void)
{
    MU_BootOtherCore(APP_MU, APP_CORE1_BOOT_MODE);
}
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size()
{
    return CORE1_IMAGE_SIZE;
}
#endif

void APP_BootCore1(void)
{
    BOARD_InitAHBSC();
    BOARD_ReleaseCore1Power();
    BOARD_BootCore1(CORE1_BOOT_ADDRESS, CORE1_BOOT_ADDRESS);
}

void APP_InitCore0Domain(void)
{
    /* BOARD_CopyCore1Image(CORE1_BOOT_ADDRESS); */
    APP_BootCore1();
}

/*!
 * @brief Function to copy core1 image to execution address.
 */
static void APP_CopyCore1Image(void)
{
#ifdef CORE1_IMAGE_COPY_TO_RAM
    /* Calculate size of the image  - not required on MCUXpresso IDE. MCUXpresso copies the secondary core
       image to the target memory during startup automatically */
    uint32_t core1_image_size = get_core1_image_size();

    PRINTF("Copy Secondary core image to address: 0x%x, size: %d\r\n", CORE1_BOOT_ADDRESS, core1_image_size);

    /* Copy Secondary core application from FLASH to the target memory. */
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#ifdef CORE1_IMAGE_FLUSH_CACHE
    CORE1_IMAGE_FLUSH_CACHE(CORE1_BOOT_ADDRESS, core1_image_size);
#endif
    memcpy((void *)CORE1_BOOT_ADDRESS, (void *)CORE1_IMAGE_START, core1_image_size);
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    SCB_CleanInvalidateDCache_by_Addr((void *)CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#ifdef CORE1_IMAGE_FLUSH_CACHE
    CORE1_IMAGE_FLUSH_CACHE(CORE1_BOOT_ADDRESS, core1_image_size);
#endif
#endif
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t domainId;
    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();

    BOARD_InitDebugConsole();

    RESET_PeripheralReset(kMU1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kSEMA420_RST_SHIFT_RSTn);

    APP_CopyCore1Image();

#if APP_BOARD_HAS_LED
    /* Initialize LED */
    LED_INIT();
#endif

#if USE_MU_NOTIFICATIONS
    /* MUA init */
    MU_Init(APP_MU);
#else
    APP_InitInterCoreNotifications();
#endif

    /* Print the initial banner */
    PRINTF("\r\nSema42 example!\r\n");

    /* SEMA42 init */
    SEMA42_Init(APP_SEMA42);
    /* Reset the sema42 gate */
    SEMA42_ResetAllGates(APP_SEMA42);

#if CORE0_BOOT_CORE1
    APP_InitDomain();
    /* Boot Core 1. */
    APP_BootCore1();
#else
    APP_InitCore0Domain();
#endif

#if USE_MU_NOTIFICATIONS
    MU_SetFlags(APP_MU, BOOT_FLAG);
#else
    APP_SetInterCoreNotificationsData((uint32_t)BOOT_FLAG);
#endif

    /* Wait Core 1 is Boot Up */
#if USE_MU_NOTIFICATIONS
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
#else
    while (BOOT_FLAG != APP_GetInterCoreNotificationsData())
#endif
    {
    }

    domainId = APP_GetCore0DomainID();

    /* Lock the sema42 gate. */
    SEMA42_Lock(APP_SEMA42, SEMA42_GATE, domainId);

#if USE_MU_NOTIFICATIONS
    MU_SetFlags(APP_MU, SEMA42_LOCK_FLAG);
#else
    APP_SetInterCoreNotificationsData((uint32_t)SEMA42_LOCK_FLAG);
#endif

    /* Wait until user press any key */
#if APP_BOARD_HAS_LED
    PRINTF("Press any key to unlock semaphore and Core 1 will turn off the LED\r\n");
#else
    PRINTF("Press any key to unlock semaphore and Core 1 will lock it\r\n");
#endif
    GETCHAR();

    /* Unlock the sema42 gate. */
    SEMA42_Unlock(APP_SEMA42, SEMA42_GATE);

#if APP_BOARD_HAS_LED
    PRINTF("Now the LED should be turned off\r\n");
#else
    PRINTF("Wait for core 1 lock the semaphore\r\n");
#endif
    /* Wait for core 1 lock the sema */
#if USE_MU_NOTIFICATIONS
    while (SEMA42_CORE1_LOCK_FLAG != MU_GetFlags(APP_MU))
#else
    while (SEMA42_CORE1_LOCK_FLAG != APP_GetInterCoreNotificationsData())
#endif
    {
    }

#if CORE0_BOOT_CORE1
    APP_DeinitDomain();
#endif

    PRINTF("\r\nSema42 example succeed!\r\n");

    while (1)
    {
    }
}
