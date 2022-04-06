/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <xtensa/config/core.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"
#include "fsl_sema42.h"

#include "board_fusionf1.h"
#include "pin_mux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU               MUB
#define APP_SEMA42           SEMA42
#define LED_OFF              LED_RED_OFF
#define APP_BOARD_HAS_LED    1
#define USE_STATIC_DOMAIN_ID 0
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U
/* Flag indicates CM33 core has locked the sema42 gate. */
#define SEMA42_LOCK_FLAG 0x02U
/* Flag indicates DSP core has locked the sema42 gate. */
#define SEMA42_DSP_LOCK_FLAG 0x03U
/* The SEMA42 gate */
#define SEMA42_GATE 0U

/*
 * Use static domain ID or dynamic domain ID.
 */
#ifndef USE_STATIC_DOMAIN_ID
#define USE_STATIC_DOMAIN_ID 1
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
#if USE_STATIC_DOMAIN_ID
uint8_t APP_GetDSPCoreDomainID(void)
{
    return 1U;
}
#else
uint8_t APP_GetDSPCoreDomainID(void);
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/
uint8_t APP_GetDSPCoreDomainID(void)
{
    return 3U;
}
void LED_INIT()
{
    CLOCK_EnableClock(kCLOCK_HsGpio0);
    RESET_PeripheralReset(kHSGPIO0_RST_SHIFT_RSTn);
    LED_RED_INIT(LOGIC_LED_OFF);
}
/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t domainId;

    BOARD_InitPins();

    /* MUB init */
    MU_Init(APP_MU);

    /* Synchronize with CM33 Core, make sure all resources are ready. */
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
    {
    }

    /* Send flag to CM33 Core to indicate DSP Core has startup */
    MU_SetFlags(APP_MU, BOOT_FLAG);

    /* Wait for CM33 core lock the sema42 gate. */
    while (SEMA42_LOCK_FLAG != MU_GetFlags(APP_MU))
    {
    }

    /* SEMA42 init */
    SEMA42_Init(APP_SEMA42);

    domainId = APP_GetDSPCoreDomainID();

    /* Lock the sema42 gate. */
    SEMA42_Lock(APP_SEMA42, SEMA42_GATE, domainId);

#if APP_BOARD_HAS_LED
    /* Turn off led */
    LED_OFF();
#endif

    /* Send flag to CM33 Core to indicate DSP Core has locked the semaphore. */
    MU_SetFlags(APP_MU, SEMA42_DSP_LOCK_FLAG);

    while (1)
    {
    }
}
