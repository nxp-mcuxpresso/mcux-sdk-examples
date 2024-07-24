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

#include "fsl_gpio.h"
#include "fsl_mailbox.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CORE0_BOOT_CORE1     0U
#define APP_BOARD_HAS_LED    1U
#define USE_STATIC_DOMAIN_ID 0U
#define USE_MU_NOTIFICATIONS 0U

#define APP_SEMA42   SEMA42_0
#define LED_INIT()   LED_RED_INIT(LOGIC_LED_ON)
#define LED_ON()     LED_RED_ON()
#define LED_OFF()    LED_RED_OFF()
#define LED_TOGGLE() LED_RED_TOGGLE()

#define PRIMARY_CORE_MAILBOX_CPU_ID   kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1
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
#define APP_STATIC_DOMAIN_ID 1
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
void APP_InitCore1Domain(void);
uint8_t APP_GetCore1DomainID(void);
void APP_InitInterCoreNotifications(void);
uint32_t APP_GetInterCoreNotificationsData(void);
void APP_SetInterCoreNotificationsData(uint32_t data);
#if USE_STATIC_DOMAIN_ID
uint8_t APP_GetCore1DomainID(void)
{
    return APP_STATIC_DOMAIN_ID;
}
#else
uint8_t APP_GetCore1DomainID(void);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_InitCore1Domain(void)
{
}

uint8_t APP_GetCore1DomainID(void)
{
    return 4U;
}
void APP_InitInterCoreNotifications(void)
{
    /* Init Mailbox */
    MAILBOX_Init(MAILBOX);
}
uint32_t APP_GetInterCoreNotificationsData(void)
{
    return MAILBOX_GetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID);
}
void APP_SetInterCoreNotificationsData(uint32_t data)
{
    MAILBOX_SetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, data);
}

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t domainId;

    /* enable clock for GPIO */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    BOARD_InitBootPins();

#if !CORE0_BOOT_CORE1
    APP_InitCore1Domain();
#endif

#if USE_MU_NOTIFICATIONS
    /* MUB init */
    MU_Init(APP_MU);
#else
    APP_InitInterCoreNotifications();
#endif

    /* Synchronize with core 0, make sure all resources are ready. */
#if USE_MU_NOTIFICATIONS
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
#else
    while (BOOT_FLAG != APP_GetInterCoreNotificationsData())
#endif
    {
    }

    /* Send flag to Core 0 to indicate Core 1 has startup */
#if USE_MU_NOTIFICATIONS
    MU_SetFlags(APP_MU, BOOT_FLAG);
#else
    APP_SetInterCoreNotificationsData((uint32_t)BOOT_FLAG);
#endif

    /* Wait for core 1 lock the sema42 gate. */
#if USE_MU_NOTIFICATIONS
    while (SEMA42_LOCK_FLAG != MU_GetFlags(APP_MU))
#else
    while (SEMA42_LOCK_FLAG != APP_GetInterCoreNotificationsData())
#endif
    {
    }

    /* SEMA42 init */
    SEMA42_Init(APP_SEMA42);

    domainId = APP_GetCore1DomainID();

    /* Lock the sema42 gate. */
    SEMA42_Lock(APP_SEMA42, SEMA42_GATE, domainId);

#if APP_BOARD_HAS_LED
    /* Turn off led */
    LED_OFF();
#endif

    /* Send flag to Core 0 to indicate Core 1 has locked the semaphore. */
#if USE_MU_NOTIFICATIONS
    MU_SetFlags(APP_MU, SEMA42_CORE1_LOCK_FLAG);
#else
    APP_SetInterCoreNotificationsData((uint32_t)SEMA42_CORE1_LOCK_FLAG);
#endif

    while (1)
    {
    }
}
