/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"
#include "fsl_sx1502.h"

#include <stdbool.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SX1502_INIT_REGDATA (0xFFU)
#define SX1502_INIT_REGDIR  (0xFFU)

#define BOARD_SX1502_I2C_INSTANCE 1
#define BOARD_SX1502_I2C_BASEADDR I2C1

#define LED_NUMBERS     7U
#define PATTERN_NUMBERS 4U
#define SHELL_Printf PRINTF
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t LedControl(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t PatternControl(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
sx1502_config_t sx1502Config = {
    .initRegDataValue  = SX1502_INIT_REGDATA,
    .initRegDirValue   = SX1502_INIT_REGDIR,
    .sx1502I2CInstance = BOARD_SX1502_I2C_INSTANCE,
};

sx1502_handle_t sx1502Handle;
SHELL_COMMAND_DEFINE(led,
                     "\r\n\"led arg1 arg2\":\r\n Usage:\r\n    arg1: 1|2|3|4...         "
                     "   Led index\r\n    arg2: on|off                Led status\r\n",
                     LedControl,
                     2);

SHELL_COMMAND_DEFINE(pattern,
                     "\r\n\"pattern arg1 arg2\":\r\n Usage:\r\n    arg1: 1|2|3|4...         "
                     "   Pattern index\r\n    arg2: on|off                Pattern status\r\n",
                     PatternControl,
                     2);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;
extern sx1502_config_t sx1502Config;
extern sx1502_handle_t sx1502Handle;
/*******************************************************************************
 * Code
 ******************************************************************************/

static shell_status_t LedControl(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int32_t kLedIndex = ((int32_t)atoi(argv[1]));
    char *kLedCommand = argv[2];

    /* Check second argument to control led */
    switch (kLedIndex)
    {
#if defined(LED_NUMBERS) && LED_NUMBERS > 0
        case 1:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO0);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO0);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 1
        case 2:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO1);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO1);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 2
        case 3:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO2);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO2);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 3
        case 4:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO3);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO3);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 4
        case 5:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO4);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO4);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 5
        case 6:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO5);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO5);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(LED_NUMBERS) && LED_NUMBERS > 6
        case 7:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_Clear(&sx1502Handle, kSX1502_IO6);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_Set(&sx1502Handle, kSX1502_IO6);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
        default:
            SHELL_Printf("LED index is wrong!\r\n");
            break;
    }
    return kStatus_SHELL_Success;
}

static shell_status_t PatternControl(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    int32_t kPatternIndex = ((int32_t)atoi(argv[1]));
    char *kLedCommand     = argv[2];

    /* Check second argument to control led */
    switch (kPatternIndex)
    {
#if defined(PATTERN_NUMBERS) && PATTERN_NUMBERS > 0
        case 1:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0x40U);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, SX1502_INIT_REGDATA);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(PATTERN_NUMBERS) && PATTERN_NUMBERS > 1
        case 2:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0x52U);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, SX1502_INIT_REGDATA);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(PATTERN_NUMBERS) && PATTERN_NUMBERS > 2
        case 3:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0x2DU);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, SX1502_INIT_REGDATA);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
#if defined(PATTERN_NUMBERS) && PATTERN_NUMBERS > 3
        case 4:
            if (strcmp(kLedCommand, "on") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, 0x2AU);
            }
            else if (strcmp(kLedCommand, "off") == 0)
            {
                SX1502_IO_OutputControl(&sx1502Handle, kSX1502_IO_All, SX1502_INIT_REGDATA);
            }
            else
            {
                SHELL_Printf("Control conmmand is wrong!\r\n");
            }
            break;
#endif
        default:
            SHELL_Printf("Pattern index is wrong!\r\n");
            break;
    }
    return kStatus_SHELL_Success;
}

/*! @brief Main function */
int main(void)
{
    CLOCK_AttachClk(kSFRO_to_FLEXCOMM1);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    RESET_PeripheralReset(kHSGPIO0_RST_SHIFT_RSTn);

    sx1502Config.sx1502I2CSourceClock = CLOCK_GetFlexCommClkFreq(1U);

    /* Init SX1502 */
    PRINTF("Configure SX1502 IO expander driver.\r\n");

    if (SX1502_Init(&sx1502Handle, &sx1502Config) != kStatus_Success)
    {
        PRINTF("SX1502_Init failed!\r\n");
        assert(false);
    }

    PRINTF("The demo is used to demonstrate how to use new component.\r\n");
    PRINTF(
        "The main function of the demo is to control the led on external dmic board by using the shell."
        "Please enter \"help\" to get the help information firstly.\r\n");
    PRINTF("\r\nFormat of input command:\r\n");
    PRINTF(
        "To control single LED, Turn on LED by using command \"led index on\". And turn off LED by using command "
        "\"led index off\".\r\n");
    PRINTF("Such as:\r\n    led 1 on  : LED1 on\r\n    led 1 off : LED1 off\r\n");
    PRINTF(
        "To control multiple LED, Turn on using command \"pattern index on\". And turn off by using command \"pattern "
        "index off\". When the pattern is off, the LEDs return to the initial state.\r\n");
    PRINTF(
        "Such as:\r\n    pattern 1 on : LED1-LED6 on, LED7 off\r\n    pattern 2 on : LED1/LED3/LED4/LED6 on, "
        "LED2/LED5/LED7 off\r\n    pattern 3 on : LED2/LED5/LED7 on, LED1/LED3/LED4/LED6 off\r\n    pattern 4 on : "
        "LED1/LED3/LED5/LED7 on, LED2/LED4/LED6 off\r\n    pattern index off : All LEDs return to the initial "
        "state.\r\n");
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];

    SHELL_Init(s_shellHandle, g_serialHandle, "SHELL>> ");
    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(led));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(pattern));

    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }
}
