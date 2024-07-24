/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_ele_base_api.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_systickCounter;

/*******************************************************************************
 * Code
 ******************************************************************************/
void SysTick_Handler(void)
{
    g_systickCounter++;

    /*
     *  RT118x ELE requires ping every 24 hours, which is mandatory,
     *  otherwise soc may reset.
     *
     *  note:
     *    1. This is generic rule for all RT118x demos.
     *    2. Most of RT118x demos don't ping ELE every 24 hours, that
     *       is because those demos focus on the function demonstrate only.
     *       It is still MUST to ping ELE every 24 hours if demo run
     *       duration > 24 hours.
     *    3. Below is an example to ping the ELE every 23(but not 24)
     *       hours, in case of any clock inaccuracy.
     */
    if (g_systickCounter >= (23 * 60 * 60 * 1000UL))
    {
        g_systickCounter = 0;
        ELE_BaseAPI_Ping(MU_RT_S3MUA);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    char ch;

    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set systick reload value to generate 1ms interrupt */
    if (SysTick_Config(SystemCoreClock / 1000U))
    {
        /* 1ms interrupt configure error, stick here */
        while (1)
        {
        }
    }

    PRINTF("hello world.\r\n");

    while (1)
    {
        ch = GETCHAR();
        PUTCHAR(ch);
    }
}
