/*
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"

volatile static int msTicks = 0;

void SysTick_Handler(void) {
    msTicks++;
}

void init_timer (void) {
    SysTick_Config(SystemCoreClock / 1000);
}

int get_time_in_us() {
    int us = ((SystemCoreClock / 1000) - SysTick->VAL) / (SystemCoreClock / 1000000);
    us += msTicks * 1000;
    return us;
}

