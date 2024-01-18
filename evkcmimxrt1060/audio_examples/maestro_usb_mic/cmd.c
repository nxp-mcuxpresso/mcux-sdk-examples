/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "cmd.h"

#include <string.h>
#include <stdint.h>
#include "fsl_debug_console.h"
#include "fsl_shell.h"

#include "usb.h"
#include "audio_microphone.h"
#include "app_streamer.h"
#include "streamer.h"
/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/

/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellUsbMIC(shell_handle_t shellHandle, int32_t argc, char **argv);

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Display component versions\r\n", shellEcho, 0);

SHELL_COMMAND_DEFINE(usb_mic,
                     "\r\n\"usb_mic\": Record MIC audio and playback to the USB port as an audio 2.0\r\n"
                     "               microphone device.\r\n"
                     "    USAGE:     usb_mic <seconds>\r\n"
                     "    <seconds>  Time in seconds how long the application should run.\r\n"
                     "               When you enter a negative number the application will\r\n"
                     "               run until the board restarts.\r\n"
                     "    EXAMPLE:   The application will run for 20 seconds: usb_mic 20\r\n",
                     shellUsbMIC,
                     1);

extern usb_audio_microphone_struct_t s_audioMicrophone;

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;
extern serial_handle_t g_serialHandle;
streamer_handle_t streamerHandle;

/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*${function:start}*/

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    PRINTF(" Maestro version: %s\r\n", STREAMER_VERSION);
    return kStatus_SHELL_Success;
}

static shell_status_t shellUsbMIC(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    status_t ret;
    int time2Run = atoi(argv[1]);

    if (time2Run == 0)
    {
        PRINTF("Incorrect command parameter\r\n");
        return kStatus_SHELL_Success;
    }

    PRINTF("\r\nStarting maestro usb microphone application\r\n");
    if (time2Run > 0)
        PRINTF("The application will run for %d second(s)\r\n", time2Run);
    else
        PRINTF("The application will run until the board restarts\r\n", time2Run);

    STREAMER_Init();
    ret = STREAMER_mic_Create(&streamerHandle);
    if (ret != kStatus_Success)
    {
        PRINTF("STREAMER_Create failed\r\n");
        goto error;
    }

    PRINTF("Starting recording\r\n");
    STREAMER_Start(&streamerHandle);

    if (time2Run < 0)
    {
        while (true)
        {
            USB_AudioCodecTask();
            OSA_TimeDelay(0); // Due to switch to another task
        }
    }
    else
    {
        TickType_t milliSec = xTaskGetTickCount();
        while (xTaskGetTickCount() <= (milliSec + ((TickType_t)time2Run * 1000)))
        {
            USB_AudioCodecTask();
            OSA_TimeDelay(0); // Due to switch to another task
        }
    }

    STREAMER_Stop(&streamerHandle);

error:
    PRINTF("Cleanup\r\n");
    STREAMER_Destroy(&streamerHandle);
    return kStatus_SHELL_Success;
}

void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(usb_mic));

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    while (1)
    {
        SHELL_Task(s_shellHandle);
    }
#endif
}
/*${function:end}*/
