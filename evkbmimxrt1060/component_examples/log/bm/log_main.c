/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_component_serial_manager.h"
#include "fsl_shell.h"

#define LOG_ENABLE 1
#include "fsl_component_log.h"
#include "fsl_component_log_backend_debugconsole.h"
#include "fsl_component_log_backend_ringbuffer.h"

LOG_MODULE_DEFINE(log_main, kLOG_LevelTrace);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define LED_NUMBERS  1U
#define LED_1_INIT() USER_LED_INIT(LOGIC_LED_OFF)
#define LED_1_ON()   USER_LED_ON()
#define LED_1_OFF()  USER_LED_OFF()
#define SHELL_Printf            PRINTF
#define APP_LOG_RINGBUFFER_SIZE 512

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Led_Init(void);

static shell_status_t logCommand(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t installbackendCommand(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t uninstallbackendCommand(shell_handle_t shellHandle, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/
SHELL_COMMAND_DEFINE(log,
                     "\r\n"
                     "\"log arg1\": show log message with entered level\r\n"
                     " Usage:\r\n"
                     "    arg1: trace|debug|info|warning|error|fatal\r\n",
                     logCommand,
                     1);
SHELL_COMMAND_DEFINE(installbackend,
                     "\r\n"
                     "\"installbackend arg1\": install backend with entered type\r\n"
                     " Usage:\r\n"
                     "    arg1: debugconsole|ringbuffer\r\n",
                     installbackendCommand,
                     1);
SHELL_COMMAND_DEFINE(uninstallbackend,
                     "\r\n"
                     "\"uninstallbackend\": uninstall backend with entered type\r\n"
                     " Usage:\r\n"
                     "    arg1: debugconsole|ringbuffer\r\n",
                     uninstallbackendCommand,
                     1);
SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;

static uint8_t s_logBackendRingbuffer[APP_LOG_RINGBUFFER_SIZE];
static uint32_t s_logBackendRingbufferTail;

static uint8_t s_logBackendString[16];

/*******************************************************************************
 * Code
 ******************************************************************************/

void Led_Init(void)
{
    LED_1_INIT();
}

static shell_status_t logCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    PRINTF("\r\n");
    if (0 == strcmp("fatal", argv[1]))
    {
        LOG_FATAL("This is \"%s\" log message", argv[1]);
    }
    else if (0 == strcmp("error", argv[1]))
    {
        LOG_ERR("This is \"%s\" log message", argv[1]);
    }
    else if (0 == strcmp("warning", argv[1]))
    {
        LOG_WRN("This is \"%s\" log message", argv[1]);
    }
    else if (0 == strcmp("info", argv[1]))
    {
        LOG_INF("This is \"%s\" log message", argv[1]);
    }
    else if (0 == strcmp("debug", argv[1]))
    {
        LOG_DBG("This is \"%s\" log message", argv[1]);
    }
    else if (0 == strcmp("trace", argv[1]))
    {
        LOG_TRACE("This is \"%s\" log message", argv[1]);
    }
    else
    {
        LOG_ERR("The input arguement \"%s\" is not valid", argv[1]);
    }
    return kStatus_SHELL_Success;
}

static shell_status_t installbackendCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if ((0 == strcmp("debugconsole", argv[1])) || (0 == strcmp("ringbuffer", argv[1])))
    {
        if (0 != strlen((char const *)s_logBackendString))
        {
            PRINTF(
                "\r\nLOG backend type \"%s\" has been installed. \r\nThe demo cannot support two types "
                "backend at the same time due to the same hardware is used. \r\nOtherwise the ouput of "
                "the two backends will be messed up.\r\nIf the new backend needs to be used, please"
                " remove the old one by using command \"uninstallbackend %s\".\r\n",
                s_logBackendString, s_logBackendString);
        }
        else
        {
            strcpy((char *)s_logBackendString, argv[1]);
            if (0 == strcmp("debugconsole", argv[1]))
            {
                LOG_InitBackendDebugconsole();
            }
            else if (0 == strcmp("ringbuffer", argv[1]))
            {
                log_backend_ring_buffer_config_t config;
                config.ringBuffer       = &s_logBackendRingbuffer[0];
                config.ringBufferLength = sizeof(s_logBackendRingbuffer);
                LOG_InitBackendRingbuffer(&config);
            }
            else
            {
            }
        }
    }
    else
    {
        PRINTF("\r\nThe input arguement \"%s\" is not valid", argv[1]);
    }

    return kStatus_SHELL_Success;
}

static shell_status_t uninstallbackendCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if (0 == strcmp((char const *)s_logBackendString, argv[1]))
    {
        if (0 == strcmp("debugconsole", argv[1]))
        {
            LOG_DeinitBackendDebugconsole();
            memset(s_logBackendString, 0, sizeof(s_logBackendString));
        }
        else if (0 == strcmp("ringbuffer", argv[1]))
        {
            LOG_DeinitBackendRingbuffer();
            memset(s_logBackendString, 0, sizeof(s_logBackendString));
        }
        else
        {
        }
    }
    else
    {
        if (0 == strlen((char const *)s_logBackendString))
        {
            PRINTF("\r\nNo log backend installed!\r\n");
        }
        else
        {
            if ((0 == strcmp("debugconsole", argv[1])) || (0 == strcmp("ringbuffer", argv[1])))
            {
                PRINTF("\r\nThe log backed \"%s\" is used.", s_logBackendString);
            }
            else
            {
                PRINTF("\r\nThe input arguement \"%s\" is not valid", argv[1]);
            }
        }
    }
    return kStatus_SHELL_Success;
}

void log_backend_ringbuffer_update(uint8_t *buffer, size_t head, size_t tail)
{
    if (s_logBackendRingbufferTail < head)
    {
        for (; s_logBackendRingbufferTail < head; s_logBackendRingbufferTail++)
        {
            PUTCHAR(buffer[s_logBackendRingbufferTail]);
        }
    }
    else
    {
        for (; s_logBackendRingbufferTail < sizeof(s_logBackendRingbuffer); s_logBackendRingbufferTail++)
        {
            PUTCHAR(buffer[s_logBackendRingbufferTail]);
        }
        for (s_logBackendRingbufferTail = 0; s_logBackendRingbufferTail < head; s_logBackendRingbufferTail++)
        {
            PUTCHAR(buffer[s_logBackendRingbufferTail]);
        }
    }
}

/*! @brief Main function */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable clock gate for GPIO1 */
    CLOCK_EnableClock(kCLOCK_Gpio1);

    /* Init LOG */
    LOG_Init();

    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];

    SHELL_Init(s_shellHandle, g_serialHandle, "LOG SHELL>> ");
    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(installbackend));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(uninstallbackend));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(log));

    while (1)
    {
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
        SHELL_Task(s_shellHandle);
#endif
    }
}
