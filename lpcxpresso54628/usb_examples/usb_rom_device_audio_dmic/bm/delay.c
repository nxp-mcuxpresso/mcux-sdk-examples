/*
 * @brief delay (implementation file)
 *
 * @note
 * Copyright  2015, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include <stdint.h>
#include <stdbool.h>

#include "board.h"
#include "delay.h"

#define TICKRATE_HZ 1000

static volatile uint32_t tick_ct    = 0;
static volatile uint32_t tick_block = 0;
static volatile uint32_t tick_nobl  = 0;

static void TimerTick(void)
{
    if (tick_block > 0)
        tick_block -= 1;

    if (tick_nobl > 0)
        tick_nobl -= 1;
}

void Delay_block(uint32_t ms_delay)
{
    tick_block = ms_delay;
    while (tick_block != 0)
    {
        __WFI();
    }
}

void Delay_nobl(uint32_t ms_delay)
{
    tick_nobl = ms_delay;
}

bool Delay_timeout(void)
{
    return (tick_nobl == 0) ? true : false;
}

void SysTick_Init(void)
{
    tick_ct = tick_block = 0;
    SysTick_Config(SystemCoreClock / TICKRATE_HZ);
}

void SysTick_Handler(void)
{
    tick_ct += 1;
    TimerTick();
}
