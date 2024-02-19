/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "board.h"
#include "fsl_mailbox.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define PRIMARY_CORE_MAILBOX_CPU_ID   kMAILBOX_CM33_Core0
#define SECONDARY_CORE_MAILBOX_CPU_ID kMAILBOX_CM33_Core1

#define START_EVENT 1234

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_msg;

/*******************************************************************************
 * Code
 ******************************************************************************/
void MAILBOX_IRQHandler()
{
    g_msg = MAILBOX_GetValue(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID);
    MAILBOX_ClearValueBits(MAILBOX, SECONDARY_CORE_MAILBOX_CPU_ID, 0xffffffff);
    g_msg++;
    MAILBOX_SetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, g_msg);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    /* set BOD VBAT level to 1.65V */
    BOARD_InitBootPins();

    /* Initialize Mailbox */
    MAILBOX_Init(MAILBOX);

    /* Enable mailbox interrupt */
    NVIC_EnableIRQ(MAILBOX_IRQn);

    /* Let the other side know the application is up and running */
    MAILBOX_SetValue(MAILBOX, PRIMARY_CORE_MAILBOX_CPU_ID, (uint32_t)START_EVENT);

    while (1)
    {
        __WFI();
    }
}
