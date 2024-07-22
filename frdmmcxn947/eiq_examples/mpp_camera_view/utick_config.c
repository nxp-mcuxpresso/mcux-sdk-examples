/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*
 * Utick timer setup for supporting FreeRTOS runtime
 * task statistics
 */

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#if (configGENERATE_RUN_TIME_STATS == 1)
#include "fsl_utick.h"
#include "utick_config.h"

#define BOARD_UTICK UTICK0
/* number of us between each timer IRQ */
#define US_PER_TICK 100 /* 100 us is the max recommended for FreeRTOS (10xSpeed of systick) */

static unsigned long utickcounter = 0;

static void UTickCallback(void)
{
    utickcounter++;
}

void vConfigureTimerForRunTimeStats(void) {

    /* Enable FRO 1MHz clock for UTICK */
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_FRO1MHZ_ENA_MASK;

    /* Intiialize UTICK */
    UTICK_Init(BOARD_UTICK);

    /* Set the UTICK timer to wake up the device from reduced power mode */
    UTICK_SetTick(BOARD_UTICK, kUTICK_Repeat, US_PER_TICK, UTickCallback);
}

unsigned long vGetTimerForRunTimeStats()
{
    return utickcounter;
}
#endif

