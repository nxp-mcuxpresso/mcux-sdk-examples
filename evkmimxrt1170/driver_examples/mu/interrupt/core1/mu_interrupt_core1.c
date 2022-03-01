/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_mu.h"
#include "fsl_gpio.h"
#include "pin_mux.h"
#include "board.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_INIT()        USER_LED_INIT(LOGIC_LED_OFF)
#define LED_TOGGLE()      USER_LED_TOGGLE()
#define APP_MU            MUB
#define APP_MU_IRQn       MUB_IRQn
#define APP_MU_IRQHandler MUB_IRQHandler
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM kMU_MsgReg0

/* How many message is used to test message sending */
#define MSG_LENGTH 32U
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t g_msgRecv[MSG_LENGTH];
volatile uint32_t g_curSend = 0;
volatile uint32_t g_curRecv = 0;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*
 * For the dual core project, generally primary core starts first, initializes
 * the system, then starts the secondary core to run.
 * In the case that debugging dual-core at the same time (for example, using IAR+DAPLink),
 * the secondary core is started by debugger. Then the secondary core might
 * run when the primary core initialization not finished. The SRC->GPR is used
 * here to indicate whether secondary core could go. When started, the secondary core
 * should check and wait the flag in SRC->GPR, the primary core sets the
 * flag in SRC->GPR when its initialization work finished.
 */
#define BOARD_SECONDARY_CORE_GO_FLAG 0xa5a5a5a5u
#define BOARD_SECONDARY_CORE_SRC_GPR kSRC_GeneralPurposeRegister20

void BOARD_WaitAndClearSecondaryCoreGoFlag(void)
{
    while (BOARD_SECONDARY_CORE_GO_FLAG != SRC_GetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR))
    {
    }

    SRC_SetGeneralPurposeRegister(SRC, BOARD_SECONDARY_CORE_SRC_GPR, 0x0);
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
 * @brief Function to create delay for Led blink.
 */
void delay(void)
{
    volatile uint32_t i = 0;
    for (i = 0; i < 5000000; ++i)
    {
        __NOP();
    }
}

void APP_MU_IRQHandler(void)
{
    uint32_t flag = 0;

    flag = MU_GetStatusFlags(APP_MU);
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
    if (((flag & kMU_Tx0EmptyFlag) == kMU_Tx0EmptyFlag) && (g_curRecv == MSG_LENGTH))
    {
        if (g_curSend < MSG_LENGTH)
        {
            MU_SendMsgNonBlocking(APP_MU, CHN_MU_REG_NUM, g_msgRecv[g_curSend++]);
        }
        else
        {
            MU_DisableInterrupts(APP_MU, kMU_Tx0EmptyInterruptEnable);
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
    BOARD_WaitAndClearSecondaryCoreGoFlag();
    BOARD_InitPins();
    NVIC_EnableIRQ(APP_MU_IRQn);
    /* Initialize LED */
    LED_INIT();

    /* MUB init */
    MU_Init(APP_MU);

    /* Send flag to Core 0 to indicate Core 1 has startup */
    MU_SetFlags(APP_MU, BOOT_FLAG);

    /* Clear the g_msgRecv array before receive */
    ClearMsgRecv();
    /* Enable transmit and receive interrupt */
    MU_EnableInterrupts(APP_MU, (kMU_Tx0EmptyInterruptEnable | kMU_Rx0FullInterruptEnable));
    /* Wait the data send and receive finish */
    while ((g_curSend < MSG_LENGTH) || (g_curRecv < MSG_LENGTH))
    {
    }

    while (1)
    {
        delay();
        LED_TOGGLE();
    }
}
