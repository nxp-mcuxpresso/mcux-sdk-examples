/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "app.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "fsl_component_led.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"
#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
#include "fsl_component_button.h"
#endif
#include "fsl_component_timer_manager.h"

#include "app.h"
#include "fsl_os_abstraction.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define SHELL_APP_BUFFER_LENGTH (64U)

#define SHELL_APP_WRITE_TSET_HANDLE_COUNT (10U)

#define SHELL_APP_TEST_MEMORY_SIZE (256U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static shell_status_t SHELL_LedCommand(shell_handle_t shellHandle, int32_t argc, char **argv);
#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
static shell_status_t SHELL_DimCommand(shell_handle_t shellHandle, int32_t argc, char **argv);
static int StringToInt(char *buffer, int length, uint32_t *num);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern serial_handle_t g_serialHandle;

static SHELL_HANDLE_DEFINE(s_shellHandle);

SHELL_COMMAND_DEFINE(led,
                     "\r\n\"led arg1\": Set the LED status\r\n"
                     "  Usage:\r\n    arg1: on/off\r\n",
                     SHELL_LedCommand,
                     1);
#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
SHELL_COMMAND_DEFINE(dim,
                     "\r\n\"dim arg1 arg2\": Set the brightness to incerase or decrease and its duration in ms \r\n"
                     "  Usage:\r\n    arg1:brighten/darken\r\n    arg2:set dimming duration in ms,eg:5000 \r\n",
                     SHELL_DimCommand,
                     2);
#endif

extern led_config_t g_ledMonochrome[LED_TYPE_MONOCHROME_COUNT];
#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
extern led_config_t g_ledMonochromeDim[LED_TYPE_MONOCHROME_COUNT];
#endif
static LED_HANDLE_ARRAY_DEFINE(s_ledMonochromeHandle, LED_TYPE_MONOCHROME_COUNT);

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
extern button_config_t g_buttonConfig[BUTTON_COUNT];
static BUTTON_HANDLE_ARRAY_DEFINE(buttonHandle, BUTTON_COUNT);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

static shell_status_t SHELL_LedCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if (0 == strcmp(argv[1], "on"))
    {
        LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 1);
    }
    else if (0 == strcmp(argv[1], "off"))
    {
        LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 0);
    }
    else
    {
        SHELL_Printf(shellHandle, "Invalid command!\r\n");
    }
    return kStatus_SHELL_Success;
}

#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
static shell_status_t SHELL_DimCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    uint32_t duration = 5000;
    if (0 != StringToInt(argv[2], strlen(argv[2]), &duration))
    {
        SHELL_Printf(shellHandle, "Invalid command!\r\n");
    }
    else
    {
        if (0 == strcmp(argv[1], "brighten"))
        {
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 0);
            LED_Dimming((led_handle_t)s_ledMonochromeHandle[0], duration, 1);
        }
        else if (0 == strcmp(argv[1], "darken"))
        {
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 1);
            LED_Dimming((led_handle_t)s_ledMonochromeHandle[0], duration, 0);
        }
        else
        {
            SHELL_Printf(shellHandle, "Invalid command!\r\n");
        }
    }
    return kStatus_SHELL_Success;
}

static int StringToInt(char *buffer, int length, uint32_t *num)
{
    uint8_t isHex      = 0;
    uint8_t startIndex = 0;

    *num = 0;
    if ((length > 2) && ('0' == buffer[0]) && (('X' == buffer[1]) || ('x' == buffer[1])))
    {
        isHex      = 1;
        startIndex = 2;
    }

    for (int i = startIndex; i < length; i++)
    {
        if ((buffer[i] >= '0') && (buffer[i] <= '9'))
        {
            *num *= (isHex) ? 16 : 10;
            *num += buffer[i] - '0';
        }
        else if (((buffer[i] >= 'a') && (buffer[i] <= 'f')) || ((buffer[i] >= 'A') && (buffer[i] <= 'F')))
        {
            if (isHex)
            {
                *num *= 16;
                if ((buffer[i] >= 'a') && (buffer[i] <= 'f'))
                {
                    *num += buffer[i] - 'a' + 10;
                }
                else
                {
                    *num += buffer[i] - 'A' + 10;
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return -1;
        }
    }
    return 0;
}
#endif

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
button_status_t button_callback(void *buttonHandle, button_callback_message_t *message, void *callbackParam)
{
    button_status_t status = kStatus_BUTTON_Success;

    switch (message->event)
    {
        case kBUTTON_EventOneClick:
            PRINTF("kBUTTON_EventOneClick\r\n");
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 0);
            break;
        case kBUTTON_EventShortPress:
            PRINTF("kBUTTON_EventShortPress\r\n");
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 0);
            break;
        case kBUTTON_EventDoubleClick:
            PRINTF("kBUTTON_EventDoubleClick\r\n");
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 1);
            break;
        case kBUTTON_EventLongPress:
            PRINTF("kBUTTON_EventLongPress\r\n");
            LED_TurnOnOff((led_handle_t)s_ledMonochromeHandle[0], 1);
            break;
        case kBUTTON_EventError:
            PRINTF("kBUTTON_EventError\r\n");
            break;
        default:
            status = kStatus_BUTTON_Error;
            break;
    }

    return status;
}
#endif

/*! @brief Main function */
void main_task(osa_task_param_t arg)
{
    static uint8_t initialized = 0;

    if (!initialized)
    {
        timer_config_t timerConfig;

        initialized = 1;

        timerConfig.instance       = 0U;
        timerConfig.srcClock_Hz    = TIMER_SOURCE_CLOCK;
        timerConfig.clockSrcSelect = 0U;
        TM_Init(&timerConfig);

        PRINTF("The demo is used to demonstrate how to use new components.\r\n");
        PRINTF("The main function of the demo is to control the led by using the shell or button.\r\n");
        PRINTF(
            "For shell, please enter \"help\" to get the help information firstly. Turn on LED by"
            " using command \"led on\". And turn off LED by using command \"led off\".\r\n");
#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
        PRINTF(
            "Enhance led brightness by using command \"dim brighten 5000\"."
            "And dim the brightness by using command \"dim darken 5000\".\r\n");
#endif

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
        PRINTF(
            "For button, please press the button %s to control LED. Turn on LED when the"
            " button is pressed with long press or double click event. And turn off LED"
            " when the button is pressed with short press or one click event.\r\n",
            BUTTON_NAME);
#endif

        if (kStatus_SHELL_Success != SHELL_Init((shell_handle_t)s_shellHandle, g_serialHandle, "led_control@SHELL>"))
        {
            PRINTF("Shell initialization failed!\r\n");
        }

        if (kStatus_SHELL_Success != SHELL_RegisterCommand((shell_handle_t)s_shellHandle, SHELL_COMMAND(led)))
        {
            PRINTF("Shell register led command failed!\r\n");
        }

#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
        if (kStatus_SHELL_Success != SHELL_RegisterCommand((shell_handle_t)s_shellHandle, SHELL_COMMAND(dim)))
        {
            PRINTF("Shell register dim command failed!\r\n");
        }
#endif

        for (int i = 0; i < LED_TYPE_MONOCHROME_COUNT; i++)
        {
#if (defined(LED_DIMMING_ENABLEMENT) && (LED_DIMMING_ENABLEMENT > 0U))
            if (kStatus_LED_Success != LED_Init((led_handle_t)s_ledMonochromeHandle[i], &g_ledMonochromeDim[i]))
#else
            if (kStatus_LED_Success != LED_Init((led_handle_t)s_ledMonochromeHandle[i], &g_ledMonochrome[i]))
#endif
            {
                PRINTF("LED %d initialization failed\r\n", i);
            }
        }

#if (defined(BUTTON_COUNT) && (BUTTON_COUNT > 0U))
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            BUTTON_Init((button_handle_t)buttonHandle[i], &g_buttonConfig[i]);
            BUTTON_InstallCallback((button_handle_t)buttonHandle[i], button_callback, NULL);
        }
#endif
    }

#if USE_RTOS
    while (1)
#endif
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task((shell_handle_t)s_shellHandle);
#endif
    }
}
