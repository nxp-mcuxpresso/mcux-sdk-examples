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

#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU MU4_MUB
/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U

/* Channel transmit and receive register */
#define CHN_MU_REG_NUM 0U

/* How many message is used to test message sending */
#define MSG_LENGTH 32U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void LED_INIT();
void LED_TOGGLE();
static uint32_t g_msgRecv[MSG_LENGTH];
/*******************************************************************************
 * Code
 ******************************************************************************/
void LED_INIT()
{
    LED_BLUE_INIT(LOGIC_LED_OFF);
}
void LED_TOGGLE()
{
    LED_BLUE_TOGGLE();
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
/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;

    /* Init board hardware. */
    CLOCK_SetXtalFreq(BOARD_XTAL_SYS_CLK_HZ); /* Note: need tell clock driver the frequency of OSC. */
    BOARD_InitBootPins();
    RESET_ClearPeripheralReset(kGPIO0_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Gpio0);

    /* Initialize LED */
    LED_INIT();

    /* MUB init */
    MU_Init(APP_MU);

    /* Send flag to Core 0 to indicate Core 1 has startup */
    MU_SetFlags(APP_MU, BOOT_FLAG);

    /* Clear the g_msgRecv array before receive */
    ClearMsgRecv();
    /* DSP receive message from CM33 core */
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgRecv[i] = MU_ReceiveMsg(APP_MU, CHN_MU_REG_NUM);
    }
    /* DSP send message back to CM33 core */
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        MU_SendMsg(APP_MU, CHN_MU_REG_NUM, g_msgRecv[i]);
    }

    while (1)
    {
        delay();
        LED_TOGGLE();
    }
}
