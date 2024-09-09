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

#include "core1_support.h"
#include "dsp_support.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_MU MU4_MUA
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
/*******************************************************************************
 * Code
 ******************************************************************************/
void APP_BootCore1(void)
{
    BOARD_ReleaseCore1Power();
    BOARD_BootCore1(CORE1_BOOT_ADDRESS, CORE1_BOOT_ADDRESS);
}


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

/*!
 * @brief Main function
 */
int main(void)
{
    uint32_t i;

    /* Init board hardware.*/
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();

    BOARD_InitDebugConsole();
    /* Clear MUA reset */
    RESET_PeripheralReset(kMU4_RST_SHIFT_RSTn);

    BOARD_InitAHBSC();

    /* Enable GPIO access from DSP side */
    RESET_ClearPeripheralReset(kGPIO0_RST_SHIFT_RSTn);
    CLOCK_EnableClock(kCLOCK_Gpio0);

    GPIO_EnablePinControlNonSecure(BOARD_LED_BLUE_GPIO, 1 << BOARD_LED_BLUE_GPIO_PIN);
    GPIO_EnablePinControlNonPrivilege(BOARD_LED_BLUE_GPIO, 1 << BOARD_LED_BLUE_GPIO_PIN);

#ifdef CORE1_IMAGE_COPY_TO_RAM
    BOARD_CopyCore1Image(CORE1_BOOT_ADDRESS);
#endif
    APP_BootCore1();

    /* MUA init */
    MU_Init(APP_MU);

    /* Print the initial banner */
    PRINTF("\r\nMU example polling!\r\n");

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
    /* Core 0 send message to Core 1 */
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        MU_SendMsg(APP_MU, CHN_MU_REG_NUM, g_msgSend[i]);
    }
    /* Core 0 receive message from Core 1 */
    for (i = 0U; i < MSG_LENGTH; i++)
    {
        g_msgRecv[i] = MU_ReceiveMsg(APP_MU, CHN_MU_REG_NUM);
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
