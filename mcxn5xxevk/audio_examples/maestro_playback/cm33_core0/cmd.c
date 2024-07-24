/*
 * Copyright 2019-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "app_data.h"
#include "cmd.h"

#include <string.h>
#include <stdint.h>
#include "fsl_debug_console.h"
#include "fsl_shell.h"
#include "ff.h"

#include "app_definitions.h"
#include "app_streamer.h"
#include "streamer.h"

/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define STR(s)    #s
#define TO_STR(s) STR(s)
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv);

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Display component versions\r\n", shellEcho, 0);

SHELL_COMMAND_DEFINE(
    file,
    "\r\n\"file\": Perform audio file decode and playback\r\n"
    "\r\n"
    "  USAGE: file [stop|pause|volume|"
#ifndef MULTICHANNEL_EXAMPLE
    "seek|"
#endif
    "play|list|info]\r\n"
    "    stop              Stops actual playback.\r\n"
    "    pause             Pause actual track or resume if already paused.\r\n"
    "    volume=<volume>   Set volume. The volume can be set from 0 to 100.\r\n"
#ifndef MULTICHANNEL_EXAMPLE
    "    seek=<seek_time>  Seek currently paused track. Seek time is absolute time in milliseconds.\r\n"
#endif
#ifdef MULTICHANNEL_EXAMPLE
    "    play <filename> [num_channels]  Select audio track to play. Select 2 or 8 channels. \r\n"
    "                                    - If channel number not specified, default 8 is used. \r\n"
#else
    "    play=<filename>  Select audio track to play.\r\n"
#endif
    "    list              List audio files available on mounted SD card.\r\n"
    "    info              Prints playback info.\r\n"
#ifdef MULTICHANNEL_EXAMPLE
    "  NOTE: Selected audio track must always meet the following parameters:\r\n"
    "                  - Sample rate:        96 kHz\r\n"
    "                  - Width:              32 bit\r\n"
    "                  - Number of channels: Depending on the [num_channels] parameter\r\n"
#endif
    ,
    shellFile,
    SHELL_IGNORE_PARAMETER_COUNT);

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;
extern serial_handle_t g_serialHandle;
extern app_handle_t app;
streamer_handle_t streamerHandle;
OSA_SEMAPHORE_HANDLE_DEFINE(streamer_semaphore);

/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*${function:start}*/

static uint32_t isFileOnSDcard(char *filename)
{
    FRESULT error;
    DIR directory           = {0};
    FILINFO fileInformation = {0};
    uint32_t filePresent    = false;

    if (!app.sdcardInserted)
    {
        PRINTF("[CMD] Please insert an SD card with audio files and retry this command\r\n");
        return 0;
    }

    error = f_opendir(&directory, "/");
    if (error)
    {
        PRINTF("[CMD] Failed to open root directory of SD card\r\n");
        return 0;
    }

    while (1)
    {
        error = f_readdir(&directory, &fileInformation);
        /* When dir end or error detected, break out */
        if ((error != FR_OK) || (fileInformation.fname[0U] == 0U))
        {
            break;
        }
        /* Skip root directory */
        if (fileInformation.fname[0] == '.')
        {
            continue;
        }
        if (!(fileInformation.fattrib & AM_DIR))
        {
            if (strcmp(fileInformation.fname, filename) == 0)
            {
                filePresent = true;
                break;
            }
        }
    }

    if (error == FR_OK)
    {
        f_closedir(&directory);
    }

    return filePresent;
}

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    PRINTF(" Maestro version: %s\r\n", STREAMER_VERSION);
    return kStatus_SHELL_Success;
}

static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    if (!app.sdcardInserted)
    {
        PRINTF("[CMD] Please insert an SD card with audio files and retry this command\r\n");
        return kStatus_SHELL_Error;
    }

    // lock ATT mutex if needed, but based on major usecase it is not necessary
    // be sure that this routine is as short as possible without any complex logic
    if (argc >= 2)
    {
        OSA_SemaphoreWait(streamer_semaphore, osaWaitForever_c);
        if (strcmp(argv[1], "list") == 0)
        {
            cmdList();
        }
        else if (strcmp(argv[1], "play") == 0)
        {
            cmdPlay(argc, argv);
        }
        else if (strcmp(argv[1], "pause") == 0)
        {
            cmdPause();
        }
        else if (strcmp(argv[1], "volume") == 0)
        {
            cmdVolume(argc, argv);
        }
#ifndef MULTICHANNEL_EXAMPLE
        else if (strcmp(argv[1], "seek") == 0)
        {
            cmdSeek(argc, argv);
        }
#endif
        else if (strcmp(argv[1], "stop") == 0)
        {
            cmdStop();
        }
        else if (strcmp(argv[1], "info") == 0)
        {
            PRINTF("[CMD] status: %d (last error: %d), (%d/%d)[%s]\r\n", get_app_data()->status,
                   get_app_data()->lastError, get_app_data()->trackCurrent, get_app_data()->trackTotal,
                   get_app_data()->input);
        }
        else
        {
            PRINTF("[CMD] Undefined att command option. \r\n");
            OSA_SemaphorePost(streamer_semaphore);
            return kStatus_SHELL_Error;
        }

        if (get_app_data()->lastError != kAppCodeOk && get_app_data()->status != kAppError)
        {
            PRINTF("[CMD] Error occurred %d\r\n", get_app_data()->lastError);
            if (get_app_data()->status == kAppRunning)
            {
                get_app_data()->lastError = stop();
                PRINTF("[CMD] Error occurred, playback stopped\r\n");
            }
            else
            {
                get_app_data()->status = kAppError;
            }

            OSA_SemaphorePost(streamer_semaphore);
            return kStatus_SHELL_Error;
        }

        OSA_SemaphorePost(streamer_semaphore);
    }
    else
    {
        PRINTF("[CMD] Enter correct command option. \r\n");
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
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(file));

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    while (1)
    {
        SHELL_Task(s_shellHandle);
    }
#endif

    OSA_SemaphoreCreateBinary(streamer_semaphore);
    OSA_SemaphorePost(streamer_semaphore);
}

void cmdList()
{
    if (list_files(false) != kStatus_Success)
    {
        get_app_data()->lastError = kAppCodeOk;
    }
    else
    {
        get_app_data()->lastError = kAppCodeOk;
    }
}

void cmdPlay(int32_t argc, char **argv)
{
    char *dot = NULL;

#ifdef MULTICHANNEL_EXAMPLE
    uint8_t num_channels = DEMO_CHANNEL_NUM; // default number of channels is 8
#endif

    if (get_app_data()->status == kAppIdle)
    {
        if (argc >= 3)
        {
            if (isFileOnSDcard(argv[2]))
            {
                dot = strrchr(argv[2], '.');
                if (
#ifdef MULTICHANNEL_EXAMPLE
                    (dot && strncmp(dot + 1, "pcm", 4) == 0)
#else
#if (OGG_OPUS_DEC == 1)
                    (dot && strncmp(dot + 1, "opus", 4) == 0) || (dot && strncmp(dot + 1, "ogg", 3) == 0) ||
#endif
#if (AAC_DEC == 1)
                    (dot && strncmp(dot + 1, "aac", 3) == 0) ||
#endif
#if (WAV_DEC == 1)
                    (dot && strncmp(dot + 1, "wav", 3) == 0) ||
#endif
#if (FLAC_DEC == 1)
                    (dot && strncmp(dot + 1, "flac", 3) == 0) ||
#endif
                    (dot && strncmp(dot + 1, "mp3", 3) == 0)
#endif
                )
                {
                    strcpy(get_app_data()->input, argv[2]);
                }
                else
                {
                    PRINTF(
                        "[CMD] Input audio file name has to match one of the"
#ifdef MULTICHANNEL_EXAMPLE
                        " .pcm"
#else
                        " .mp3"
#if (OGG_OPUS_DEC == 1)
                        "|.opus|.ogg"
#endif
#if (AAC_DEC == 1)
                        "|.aac"
#endif
#if (WAV_DEC == 1)
                        "|.wav"
#endif
#if (FLAC_DEC == 1)
                        "|.flac"
#endif
#endif
                        " format.\r\n");

                    get_app_data()->lastError = kAppCodeOk;
                    return;
                }
#ifdef MULTICHANNEL_EXAMPLE
                if (argc == 4)
                {
                    num_channels = abs(atoi((argv[3])));
                    if (num_channels == 2 || num_channels == 8)
                    {
                        get_app_data()->num_channels = num_channels;
                    }
                    else
                    {
                        PRINTF("Number of channels not allowed (2 or 8 allowed).\r\n");
                        get_app_data()->lastError = kAppCodeOk;
                        return;
                    }
                }
                else
                {
                    get_app_data()->num_channels = num_channels;
                }
#endif
                get_app_data()->lastError = play();

                get_app_data()->status = kAppRunning;
                PRINTF("[CMD] Playback started for %s\r\n", get_app_data()->input);
            }
            else
            {
                PRINTF("[CMD] File is not on the SD card, please enter valid filename.\r\n");
                get_app_data()->lastError = kAppCodeOk;
            }
        }
        else
        {
            PRINTF("[CMD] Enter name of audio file.\r\n");
            get_app_data()->lastError = kAppCodeOk;
        }
    }
    else
    {
        PRINTF("[CMD] Use the stop command first.\r\n");
        get_app_data()->lastError = kAppCodeOk;
    }
}

void cmdPause()
{
    if (get_app_data()->status == kAppPaused)
    {
        get_app_data()->lastError = resume();
        get_app_data()->status    = kAppRunning;
        PRINTF("[CMD] Playback continued\r\n");
    }
    else if (get_app_data()->status == kAppRunning)
    {
        get_app_data()->lastError = pause();
        get_app_data()->status    = kAppPaused;
        PRINTF("[CMD] Playback paused\r\n");
    }
}

void cmdVolume(int32_t argc, char **argv)
{
    if (argc >= 3)
    {
        int value = atoi((argv[2]));
        if (value >= 0 && value <= 100)
        {
            get_app_data()->volume    = value;
            get_app_data()->lastError = set_volume();
        }
        else
        {
            PRINTF("[CMD] Ignoring wrong volume parameter.\r\n");
            get_app_data()->lastError = kAppCodeOk;
        }
    }
    else
    {
        PRINTF("[CMD] Enter volume parameter.\r\n");
        get_app_data()->lastError = kAppCodeOk;
    }
}

void cmdSeek(int32_t argc, char **argv)
{
    char *dot = NULL;
    if ((get_app_data()->status != kAppPaused) && (get_app_data()->status != kAppRunning))
    {
        PRINTF("[CMD] First select an audio track to play.\r\n");
        get_app_data()->lastError = kAppCodeOk;
    }
    else if (get_app_data()->status != kAppPaused)
    {
        PRINTF("[CMD] First pause the track.\r\n");
        get_app_data()->lastError = kAppCodeOk;
    }
    else
    {
        if (argc >= 3)
        {
            dot = strrchr(get_app_data()->input, '.');
            if ((dot && strncmp(dot + 1, "aac", 3) == 0) && (get_app_data()->status == kAppPaused))
            {
                PRINTF("[CMD] The AAC decoder does not support the seek command.\r\n");
                get_app_data()->lastError = kAppCodeOk;
            }
            else
            {
                if (atoi(argv[2]) < 0)
                {
                    PRINTF("[CMD] The seek time must be a positive value.\r\n");
                    get_app_data()->lastError = kAppCodeOk;
                }
                else
                {
                    get_app_data()->seek_time = atoi(argv[2]);
                    get_app_data()->lastError = seek();
                }
            }
        }
        else
        {
            PRINTF("[CMD] Enter a seek time value.\r\n");
            get_app_data()->lastError = kAppCodeOk;
        }
    }
}

void cmdStop()
{
    get_app_data()->lastError = stop();
}
/*${function:end}*/
