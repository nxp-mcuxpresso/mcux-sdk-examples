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
#include "board.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
#include "fsl_ele_base_api.h"
#include "fsl_mu.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define ELE_TRDC_AON_ID    0x74
#define ELE_TRDC_WAKEUP_ID 0x78
#define ELE_CORE_CM33_ID   0x1
#define ELE_CORE_CM7_ID    0x2

/*
 * Set ELE_STICK_FAILED_STS to 0 when ELE status check is not required,
 * which is useful when debug reset, where the core has already get the
 * TRDC ownership at first time and ELE is not able to release TRDC
 * ownership again for the following TRDC ownership request.
 */
#define ELE_STICK_FAILED_STS 1

#if ELE_STICK_FAILED_STS
#define ELE_IS_FAILED(x) (x != kStatus_Success)
#else
#define ELE_IS_FAILED(x) false
#endif
#define LED_INIT()                    USER_LED_INIT(LOGIC_LED_ON)
#define APP_MU                        MU1_MUA
#define APP_SEMA42                    SEMA1
#define APP_STATIC_DOMAIN_ID          8
#define BOOT_CORE1_BY_MU              0
#define CORE0_BOOT_CORE1_SPECIFIC_WAY (!BOOT_CORE1_BY_MU)
/* Address of memory, from which the secondary core will boot */
#define CORE1_BOOT_ADDRESS    (void *)0x303C0000
#define CORE1_KICKOFF_ADDRESS 0x0

#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
extern uint32_t Image$$CORE1_REGION$$Base;
extern uint32_t Image$$CORE1_REGION$$Length;
#define CORE1_IMAGE_START &Image$$CORE1_REGION$$Base
#elif defined(__ICCARM__)
#pragma section = "__core1_image"
#define CORE1_IMAGE_START __section_begin("__core1_image")
#elif (defined(__GNUC__)) && (!defined(__MCUXPRESSO))
extern const char core1_image_start[];
extern const char *core1_image_end;
extern int core1_image_size;
#define CORE1_IMAGE_START ((void *)core1_image_start)
#define CORE1_IMAGE_SIZE  ((void *)core1_image_size)
#endif
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
#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void);
#endif
#if !(defined(BOOT_CORE1_BY_MU) && BOOT_CORE1_BY_MU)
void APP_BootCore1(void);
#endif

void APP_InitDomain(void);
void APP_DeinitDomain(void);
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

static void BOARD_InitLedPin(void)
{
    const rgpio_pin_config_t config = {
        .pinDirection = kRGPIO_DigitalOutput,
        .outputLogic  = 1,
    };

    RGPIO_PinInit(BOARD_USER_LED_GPIO, BOARD_USER_LED_GPIO_PIN, &config);
}

/*!
 * @brief Application-specific implementation of the SystemInitHook() weak function.
 */
void SystemInitHook(void)
{
    Prepare_CM7(CORE1_KICKOFF_ADDRESS);
}

void APP_InitDomain(void)
{
    /* Use static domain ID, don't need to do anything here. */
}

void APP_DeinitDomain(void)
{
    /* Use static domain ID, don't need to do anything here. */
}

#if !(defined(BOOT_CORE1_BY_MU) && BOOT_CORE1_BY_MU)
void APP_BootCore1(void)
{
    status_t sts;

    /* Enble CM7 */
    do
    {
        sts = ELE_BaseAPI_EnableAPC(MU_RT_S3MUA);
    } while (ELE_IS_FAILED(sts));

    /* Deassert Wait */
    BLK_CTRL_S_AONMIX->M7_CFG =
        (BLK_CTRL_S_AONMIX->M7_CFG & (~BLK_CTRL_S_AONMIX_M7_CFG_WAIT_MASK)) | BLK_CTRL_S_AONMIX_M7_CFG_WAIT(0);
}
#endif

#ifdef CORE1_IMAGE_COPY_TO_RAM
uint32_t get_core1_image_size(void)
{
    uint32_t image_size;
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
    image_size = (uint32_t)&Image$$CORE1_REGION$$Length;
#elif defined(__ICCARM__)
    image_size = (uint32_t)__section_end("__core1_image") - (uint32_t)__section_begin("__core1_image");
#elif defined(__GNUC__)
    image_size = (uint32_t)core1_image_size;
#endif
    return image_size;
}
#endif

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
    BOARD_InitLedPin();

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
