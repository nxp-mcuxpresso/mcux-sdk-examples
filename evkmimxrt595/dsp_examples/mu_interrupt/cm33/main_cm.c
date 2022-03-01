/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "dsp_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU            MUA
#define APP_MU_IRQHandler MU_A_IRQHandler
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM 0U

/* How many message is used to test message sending */
#define MSG_LENGTH 32U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_msgSend[MSG_LENGTH];
static uint32_t g_msgRecv[MSG_LENGTH];
volatile uint32_t g_curSend = 0;
volatile uint32_t g_curRecv = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/


/*!
 * @brief Function to fill the g_msgSend array.
 * This function set the g_msgSend values 0, 1, 2, 3...
 */
static void FillMsgSend(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgSend[i] = i;
    }
}

/*!
 * @brief Function to clear the g_msgRecv array.
 * This function set g_msgRecv to be 0.
 */
static void ClearMsgRecv(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgRecv[i] = 0U;
    }
}

/*!
 * @brief Function to validate the received messages.
 * This function compares the g_msgSend and g_msgRecv, if they are the same, this
 * function returns true, otherwise returns false.
 */
static bool ValidateMsgRecv(void)
{
    uint32_t i;
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        PRINTF("Send: %d. Receive %d\r\n", g_msgSend[i], g_msgRecv[i]);

        if (g_msgRecv[i] != g_msgSend[i])
        {
            return false;
        }
    }
    return true;
}

void APP_MU_IRQHandler(void)
{
    uint32_t flag = 0;

    flag = MU_GetStatusFlags(APP_MU);
    if ((flag & kMU_Tx0EmptyFlag) == kMU_Tx0EmptyFlag)
    {
        if (g_curSend < MSG_LENGTH)
        {
            MU_SendMsgNonBlocking(APP_MU, CHN_MU_REG_NUM, g_msgSend[g_curSend++]);
        }
        else
        {
            MU_DisableInterrupts(APP_MU, kMU_Tx0EmptyInterruptEnable);
        }
    }
    if ((flag & kMU_Rx0FullFlag) == kMU_Rx0FullFlag)
    {
        if (g_curRecv < MSG_LENGTH)
        {
            g_msgRecv[g_curRecv++] = MU_ReceiveMsgNonBlocking(APP_MU, CHN_MU_REG_NUM);
        }
        else
        {
            MU_DisableInterrupts(APP_MU, kMU_Rx0FullInterruptEnable);
        }
    }
    SDK_ISR_EXIT_BARRIER;
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware.*/
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    /* Clear MUA reset */
    RESET_PeripheralReset(kMU_RST_SHIFT_RSTn);
    EnableIRQ(MU_A_IRQn);

    /* MUA init */
    MU_Init(APP_MU);

    /* Print the initial banner */
    PRINTF("\r\nMU example interrupt!\r\n");

    /* Copy DSP image to RAM and start DSP core. */
    BOARD_DSP_Init();

    /* Wait DSP core is Boot Up */
    while (BOOT_FLAG != MU_GetFlags(APP_MU))
    {
    }

    /* Fill the g_msgSend array before send */
    FillMsgSend();
    /* Clear the g_msgRecv array before receive */
    ClearMsgRecv();
    /* Enable transmit and receive interrupt */
    MU_EnableInterrupts(APP_MU, (kMU_Tx0EmptyInterruptEnable | kMU_Rx0FullInterruptEnable));
    /* Wait the data send and receive finish */
    while ((g_curSend < MSG_LENGTH) || (g_curRecv < MSG_LENGTH))
    {
    }

    /* Compare the data send and receive */
    if (true == ValidateMsgRecv())
    {
        PRINTF("MU example run succeed!");
    }
    else
    {
        PRINTF("MU example run Error!");
    }

    while (1)
    {
    }
}
