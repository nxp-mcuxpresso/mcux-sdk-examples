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

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

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
#define LED_NUMBERS  3U
#define LED_1_INIT() LED_RED_INIT(LOGIC_LED_OFF)
#define LED_2_INIT() LED_GREEN_INIT(LOGIC_LED_OFF)
#define LED_3_INIT() LED_BLUE_INIT(LOGIC_LED_OFF)
#define LED_1_ON()   LED_RED_ON()
#define LED_1_OFF()  LED_RED_OFF()
#define LED_2_ON()   LED_GREEN_ON()
#define LED_2_OFF()  LED_GREEN_OFF()
#define LED_3_ON()   LED_BLUE_ON()
#define LED_3_OFF()  LED_BLUE_OFF()
#define SHELL_Printf            PRINTF
#define APP_LOG_RINGBUFFER_SIZE 512
/* Task priorities. */
#define APP_LOG_PRIORITY   (configMAX_PRIORITIES - 1)
#define APP_LOG_STACK_SIZE (configMINIMAL_STACK_SIZE * 4)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Led_Init(void);

static void APP_LogTask(void *pvParameters);
static unsigned int APP_LogTimestamp(void);

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
#if (defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
static SemaphoreHandle_t s_logNew;
char s_logLevelString[16];
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

void Led_Init(void)
{
    LED_1_INIT();
    LED_2_INIT();
    LED_3_INIT();
}

static void APP_LogOutput(char *arg)
{
    PRINTF("\r\n");
    if (0 == strcmp("fatal", arg))
    {
        LOG_FATAL("This is \"%s\" log message", arg);
    }
    else if (0 == strcmp("error", arg))
    {
        LOG_ERR("This is \"%s\" log message", arg);
    }
    else if (0 == strcmp("warning", arg))
    {
        LOG_WRN("This is \"%s\" log message", arg);
    }
    else if (0 == strcmp("info", arg))
    {
        LOG_INF("This is \"%s\" log message", arg);
    }
    else if (0 == strcmp("debug", arg))
    {
        LOG_DBG("This is \"%s\" log message", arg);
    }
    else if (0 == strcmp("trace", arg))
    {
        LOG_TRACE("This is \"%s\" log message", arg);
    }
    else
    {
        LOG_ERR("The input arguement \"%s\" is not valid", arg);
    }
}

static shell_status_t logCommand(shell_handle_t shellHandle, int32_t argc, char **argv)
{
#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    APP_LogOutput(argv[1]);
#else
    strncpy(s_logLevelString, argv[1], sizeof(s_logLevelString) - 1);
#if defined(__GIC_PRIO_BITS)
    if ((__get_CPSR() & CPSR_M_Msk) == 0x13)
#else
    if (__get_IPSR())
#endif
    {
        portBASE_TYPE taskToWake = pdFALSE;
        if (pdTRUE == xSemaphoreGiveFromISR(s_logNew, &taskToWake))
        {
            portYIELD_FROM_ISR((taskToWake));
        }
    }
    else
    {
        xSemaphoreGive(s_logNew);
    }
#endif
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
    /* Init board hardware. */
    CLOCK_EnableClock(kCLOCK_HsGpio0);
    CLOCK_EnableClock(kCLOCK_HsGpio1);
    CLOCK_EnableClock(kCLOCK_HsGpio3);
    RESET_PeripheralReset(kHSGPIO0_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kHSGPIO1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kHSGPIO3_RST_SHIFT_RSTn);

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    if (xTaskCreate(APP_LogTask, "APP_LogTask", APP_LOG_STACK_SIZE, NULL, APP_LOG_PRIORITY, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
    vTaskStartScheduler();
    for (;;)
        ;
}

static unsigned int APP_LogTimestamp(void)
{
    TickType_t ticks;

    if (0U != __get_IPSR())
    {
        ticks = xTaskGetTickCountFromISR();
    }
    else
    {
        ticks = xTaskGetTickCount();
    }
    return ((uint32_t)((uint64_t)(ticks)*1000uL / (uint64_t)configTICK_RATE_HZ));
}

/*!
 * @brief Task responsible for printing of "Hello world." message.
 */
static void APP_LogTask(void *pvParameters)
{
#if (defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    s_logNew = xSemaphoreCreateBinary();
    if (NULL == s_logNew)
    {
        PRINTF("s_logNew creation failed.\r\n");
        vTaskSuspend(NULL);
    }
#endif
    /* Init LOG */
    LOG_Init();
#if LOG_ENABLE_TIMESTAMP
    LOG_SetTimestamp(APP_LogTimestamp);
#endif

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
#else
        /* Consumer is waiting when producer will be ready to produce item. */
        if (xSemaphoreTake(s_logNew, portMAX_DELAY) == pdTRUE)
        {
            char tempBuffer[sizeof(s_logLevelString)];

            uint32_t regMask = DisableGlobalIRQ();
            memcpy(tempBuffer, s_logLevelString, sizeof(tempBuffer));
            memset(s_logLevelString, 0, sizeof(s_logLevelString));
            EnableGlobalIRQ(regMask);

            APP_LogOutput(tempBuffer);
        }
#endif
    }
}
