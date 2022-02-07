/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "board_init.h"
#include "dsp_support.h"

/*!
 * @brief Main function
 */
int main(void)
{
    /* Initialize standard SDK demo application pins */
    BOARD_Init();

    /* Print the initial banner */
    printf("\r\nStarting FusionF1 example from Cortex-M33 core\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    for (;;)
        ;
}
