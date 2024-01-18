/*
 * Copyright 2023 NXP
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

#include "app_streamer.h"
#include "fsl_sd_disk.h"
#include "portable.h"
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
static shell_status_t shellStart(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellStop(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellDebug(shell_handle_t shellHandle, int32_t argc, char **argv);

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Display component versions\r\n", shellEcho, 0);

SHELL_COMMAND_DEFINE(start,
                     "\r\n\"start [nosdcard]\": Starts a streamer task.\r\n"
                     "                  - Initializes the streamer with the Memory->Speaker pipeline and with\r\n"
                     "                    the Microphone->VoiceSeeker->VIT->SDcard pipeline.\r\n"
                     "                  - Runs repeatedly until stop command.\r\n"
                     "         nosdcard - Doesn't use SD card to store data.\r\n",
                     shellStart,
                     SHELL_IGNORE_PARAMETER_COUNT);

SHELL_COMMAND_DEFINE(
    debug,
    "\r\n\"debug [on|off]\": Starts / stops debugging.\r\n"
    "                - Starts / stops saving VoiceSeeker input data (reference and microphone data) to SDRAM.\r\n"
    "                - After the stop command, this data is overwritten to the SD card.\r\n",
    shellDebug,
    SHELL_IGNORE_PARAMETER_COUNT);

SHELL_COMMAND_DEFINE(stop, "\r\n\"stop\": Stops a running streamer.\r\n", shellStop, SHELL_IGNORE_PARAMETER_COUNT);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;
extern serial_handle_t g_serialHandle;
streamer_handle_t streamerHandle;

bool noSDCard  = false;
bool isPlaying = false;

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

static shell_status_t shellStart(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if ((argc != 1) && (argc != 2))
    {
        PRINTF("Too many parameters\r\n");
        return kStatus_SHELL_Error;
    }

    if (isPlaying == true)
    {
        PRINTF("The streamer task is already running\r\n");
        return kStatus_SHELL_Error;
    }

    if (argc == 2)
    {
        if (strcmp(argv[1], "nosdcard") == 0)
        {
            noSDCard = true;
        }
        else
        {
            PRINTF("Bad parameter\r\n");
            return kStatus_SHELL_Error;
        }
    }

    if (noSDCard == false)
    {
        /* Check if a SD card is inserted */
        if (!SDCARD_inserted())
        {
            PRINTF("Insert the SD card first\r\n");
            return kStatus_SHELL_Success;
        }
    }

    STREAMER_Init();

    if (STREAMER_Create(&streamerHandle, noSDCard) != kStatus_Success)
    {
        PRINTF("STREAMER_Create failed\r\n");
        goto error;
    }

    PRINTF("Starting playing and recording\r\n");

    STREAMER_Start(&streamerHandle);

    isPlaying = true;

    return kStatus_SHELL_Success;

error:
    PRINTF("Error, Cleanup\r\n");
    STREAMER_Destroy(&streamerHandle);
    /* Delay for cleanup */
    OSA_TimeDelay(100);
    return kStatus_SHELL_Success;
}

static shell_status_t shellStop(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    STREAMER_Stop(&streamerHandle);

    PRINTF("Cleanup\r\n");
    STREAMER_Destroy(&streamerHandle);
    /* Delay for cleanup */
    OSA_TimeDelay(100);

    noSDCard  = false;
    isPlaying = false;

    return kStatus_SHELL_Success;
}

static shell_status_t shellDebug(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if ((argc != 2) || ((strcmp(argv[1], "on") != 0) && (strcmp(argv[1], "off") != 0)))
    {
        PRINTF("Please use the parameter either on or off\r\n");
        return kStatus_SHELL_Error;
    }

    if (streamerHandle.streamer == NULL)
    {
        PRINTF("First, start the streamer task with the start command.\r\n");
        return kStatus_SHELL_Error;
    }

    if (noSDCard == true)
    {
        PRINTF("Debugging cannot be used when the SD card is disabled\r\n");
        return kStatus_SHELL_Error;
    }

    if (STREAMER_SetDebug(&streamerHandle, strcmp(argv[1], "on") == 0 ? true : false) != kStatus_Success)
    {
        PRINTF("Error: Debugging was not set correctly.\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

void shellCmd(void)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(start));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(stop));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(debug));

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    while (1)
    {
        SHELL_Task(s_shellHandle);
    }
#endif
}
/*${function:end}*/
