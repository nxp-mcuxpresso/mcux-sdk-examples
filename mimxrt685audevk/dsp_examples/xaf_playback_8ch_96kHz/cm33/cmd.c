/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*${header:start}*/
#include "cmd.h"
#include "dsp_ipc.h"

#include <string.h>
#include <stdint.h>
#include "fsl_debug_console.h"
#include "fsl_shell.h"

/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define AUDIO_MAX_INPUT_BUFFER  (AUDIO_SHARED_BUFFER_1_SIZE)
#define AUDIO_MAX_OUTPUT_BUFFER (AUDIO_SHARED_BUFFER_2_SIZE)

#define AUDIO_OUTPUT_BUFFER   0
#define AUDIO_OUTPUT_RENDERER 1
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
static void initMessage(srtm_message *msg);

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv);
#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Query DSP for component versions\r\n", shellEcho, 0);

SHELL_COMMAND_DEFINE(file,
                     "\r\n\"file\": Perform audio file decode and playback on DSP\r\n"
                     "  USAGE: file [list|stop|<audio_file> [<nchannel>]]\r\n"
                     "    list          List audio files on SD card available for playback\r\n"
                     "    <audio_file>  Select file from SD card and start playback\r\n"
                     "    <nchannel>    Select the number of channels (2 or 8 can be selected).\r\n"
                     "  NOTE: Selected audio file must always meet the following parameters:\r\n"
                     "                  - Sample rate: 96 kHz\r\n"
                     "                  - Width:       32 bit\r\n"
                     "                  - Number of channels: Depending on the [num_channels] parameter\r\n"
#if XA_CLIENT_PROXY
                     "  NOTE: Only when 2 channels are selected EAP can be applied to the audio file.\r\n"
#endif
                     ,
                     shellFile,
                     SHELL_IGNORE_PARAMETER_COUNT);

#if XA_CLIENT_PROXY
SHELL_COMMAND_DEFINE(eap,
                     "\r\n\"eap\": Set EAP parameters\r\n"
                     "  USAGE: eap [1|2|3|4|5|6|7|+|-|l|r]\r\n"
                     "  OPTIONS:\r\n"
                     "    1:	All effect Off \r\n"
                     "    2:	Voice enhancer \r\n"
                     "    3:	Music enhancer \r\n"
                     "    4:	Auto volume leveler \r\n"
                     "    5:	Loudness maximiser  \r\n"
                     "    6:	3D Concert sound  \r\n"
                     "    7:	Custom\r\n"
                     "    8:	Tone Generator\r\n"
                     "    9:	Crossover 2 way speaker\r\n"
                     "   10:	Crossover for subwoofer\r\n"
                     "    +:	Volume up\r\n"
                     "    -:	Volume down\r\n"
                     "    l:	Balance left\r\n"
                     "    r:	Balance right\r\n",
                     shellEAPeffect,
                     1);
#endif

static bool file_playing = false;
static uint8_t nchannel;

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;
static handleShellMessageCallback_t *g_handleShellMessageCallback;
static void *g_handleShellMessageCallbackData;

extern int BOARD_CodecChangeSettings(uint8_t nchannel);
/*${variable:end}*/

/*******************************************************************************
 * Code
 ******************************************************************************/

/*${function:start}*/
static void initMessage(srtm_message *msg)
{
    /* Common field for command */
    /* For single command, command list not used at the moment */
    msg->head.type = SRTM_MessageTypeRequest;

    msg->head.majorVersion = SRTM_VERSION_MAJOR;
    msg->head.minorVersion = SRTM_VERSION_MINOR;
}

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_GENERAL;
    msg.head.command  = SRTM_Command_ECHO;

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}

static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    app_handle_t *app = (app_handle_t *)g_handleShellMessageCallbackData;
    srtm_message msg  = {0};
    DIR directory;
    FILINFO fileInformation;
    const char *filename, *dot;
    char *file_ptr;
    uint32_t count = 0;
    FRESULT error;
    UINT bytes_read;

    if (argc < 2)
    {
        PRINTF("Incorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n");
        return kStatus_SHELL_Error;
    }

    if (!app->sdcardInserted)
    {
        PRINTF("Please insert an SD card with audio files and retry this command\r\n");
        return kStatus_SHELL_Success;
    }

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (strcmp(argv[1], "list") == 0)
    {
        error = f_opendir(&directory, "/");
        if (error)
        {
            PRINTF("Failed to open root directory of SD card\r\n");
            return kStatus_SHELL_Error;
        }

        PRINTF("Available audio files:\r\n");

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
                /* Check file for supported audio extension */
                dot = strrchr(fileInformation.fname, '.');
                if (dot && strncmp(dot + 1, "pcm", 3) == 0)
                {
                    PRINTF("  %s\r\n", fileInformation.fname);
                    count++;
                }
            }
        }

        if (error == FR_OK)
        {
            f_closedir(&directory);
        }

        if (!count)
        {
            PRINTF("  (none)\r\n");
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (file_playing)
        {
            msg.head.command = SRTM_Command_FileStop;
            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
            return kStatus_SHELL_Success;
        }
        else
        {
            PRINTF("File is not playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (!file_playing)
    {
        if (argc > 2)
        {
            switch (atoi(argv[2]))
            {
                case 2:
                    nchannel = 2;
                    break;
                case 8:
                    /* Intentional fall */
                default:
                    nchannel = 8;
            }
        }
        else
            nchannel = 8;

        filename         = argv[1];
        file_ptr         = (char *)AUDIO_SHARED_BUFFER_1;
        msg.head.command = SRTM_Command_FileStart;

        /* Param 0 Encoded input buffer address*/
        /* Param 1 Encoded input buffer size*/
        /* Param 2 EOF (true/false) */
        /* Param 3 Audio codec component type */
        /* Param 4 Number of channels */
        /* Param 5 Sample rate */
        /* Param 6 Pcm width */
        msg.param[0] = (uint32_t)file_ptr;
        msg.param[1] = FILE_PLAYBACK_INITIAL_READ_SIZE;
        msg.param[2] = 0;
        msg.param[4] = (uint32_t)nchannel;
        msg.param[5] = 96000;
        msg.param[6] = 32;

        BOARD_CodecChangeSettings(nchannel);

        dot = strrchr(filename, '.');
        if (dot && strncmp(dot + 1, "pcm", 3) == 0)
        {
            msg.param[3] = DSP_COMPONENT_NONE;
            count        = 1;
        }
        if (!count)
        {
            PRINTF("Unsupported file type %s\r\n", filename);
            return kStatus_SHELL_Error;
        }

        error = f_open(&app->fileObject, _T(filename), FA_READ);
        if (error)
        {
            PRINTF("Cannot open file for reading: %s\r\n", filename);
            return kStatus_SHELL_Error;
        }

        error = f_read(&app->fileObject, file_ptr, FILE_PLAYBACK_INITIAL_READ_SIZE, &bytes_read);
        if (error)
        {
            PRINTF("file read fail\r\n");
            return kStatus_SHELL_Error;
        }

        /* Set EOF if file smaller than initial read block size */
        if (bytes_read < FILE_PLAYBACK_INITIAL_READ_SIZE)
        {
            msg.param[2] = 1;
        }
        file_playing = true;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    }
    else
    {
        PRINTF("File is already playing\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    int effectNum    = atoi(argv[1]);
    initMessage(&msg);

    if (file_playing && (nchannel != 2))
    {
        PRINTF("EAP can only be applied to a 2-channel audio file! Please see help.\r\n");
        return kStatus_SHELL_Error;
    }

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_FilterCfg;
    /* Param 0 Number of EAP config*/

    if (effectNum > 0 && effectNum < 11)
    {
        msg.param[0] = effectNum;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "+") == 0)
    {
        msg.param[0] = 11;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "-") == 0)
    {
        msg.param[0] = 12;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "l") == 0)
    {
        msg.param[0] = 13;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else if (strcmp(argv[1], "r") == 0)
    {
        msg.param[0] = 14;
        g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        return kStatus_SHELL_Success;
    }
    else
    {
        PRINTF("Effect parameter is out of range! Please see help. \r\n");
        return kStatus_SHELL_Error;
    }
}
#endif

void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));

    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(file));

#if XA_CLIENT_PROXY
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(eap));
#endif

    g_handleShellMessageCallback     = handleShellMessageCallback;
    g_handleShellMessageCallbackData = arg;

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    SHELL_Task(s_shellHandle);
#endif
}

static void handleDSPMessageInner(app_handle_t *app, srtm_message *msg, bool *notify_shell)
{
    *notify_shell = true;

    char string_buff[SRTM_CMD_PARAMS_MAX];

    if (msg->head.type == SRTM_MessageTypeResponse)
    {
        PRINTF("[APP_DSP_IPC_Task] response from DSP, cmd: %d, error: %d\r\n", msg->head.command, msg->error);
    }

    /* Processing returned data*/
    switch (msg->head.category)
    {
        case SRTM_MessageCategory_GENERAL:
            switch (msg->head.command)
            {
                /* echo returns version info of key components*/
                case SRTM_Command_ECHO:
                    PRINTF("Component versions from DSP:\r\n");
                    PRINTF("Audio Framework version %d.%d \r\n", msg->param[0] >> 16, msg->param[0] & 0xFF);
                    PRINTF("Audio Framework API version %d.%d\r\n", msg->param[1] >> 16, msg->param[1] & 0xFF);
                    PRINTF("NatureDSP Lib version %d.%d\r\n", msg->param[2] >> 16, msg->param[2] & 0xFF);
                    PRINTF("NatureDSP API version %d.%d\r\n", msg->param[3] >> 16, msg->param[3] & 0xFF);
                    break;

                case SRTM_Command_SYST:
                    break;
                default:
                    PRINTF("Incoming unknown message command %d from category %d \r\n", msg->head.command,
                           msg->head.category);
            }
            break;

        case SRTM_MessageCategory_AUDIO:
            if (file_playing &&
                (msg->head.command < SRTM_Command_FileStart || msg->head.command > SRTM_Command_FilterCfg))
            {
                PRINTF("This command is not possible to process now since a file from SD card is being played!\r\n");
                break;
            }
            else if (!file_playing && msg->head.command == SRTM_Command_FilterCfg)
            {
                PRINTF("Please play a file first, then apply an EAP preset.\r\n");
                break;
            }
            switch (msg->head.command)
            {
                case SRTM_Print_String:
                    for (int i = 0; i < SRTM_CMD_PARAMS_MAX; i++)
                    {
                        string_buff[i] = (char)msg->param[i];
                    }
                    PRINTF("%s", string_buff);
                    break;

                case SRTM_Command_FileStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP file playback start failed! return error = %d\r\n", msg->error);
                        file_playing = false;
                    }
                    else
                    {
                        PRINTF("DSP file playback start\r\n");
                    }

                    /* Release shell to be able to set different EAP presets*/
                    *notify_shell = true;
                    break;

                case SRTM_Command_FileData:
                {
                    FRESULT error;
                    UINT bytes_read;
                    char *file_ptr;

                    file_ptr = (char *)AUDIO_SHARED_BUFFER_1;
                    error    = f_read(&app->fileObject, file_ptr, FILE_PLAYBACK_READ_SIZE, &bytes_read);
                    if (error)
                    {
                        PRINTF("File read failure: %d\r\n", error);
                        *notify_shell = false;
                        break;
                    }

                    msg->head.type = SRTM_MessageTypeResponse;
                    msg->param[0]  = (uint32_t)file_ptr;
                    msg->param[1]  = bytes_read;
                    /* Set EOF param if final segment of file is sent. */
                    msg->param[2] = f_eof(&app->fileObject);

                    /* Send response message to DSP with new data */
                    dsp_ipc_send_sync(msg);

                    /* Don't release shell until receive notification of file end */
                    *notify_shell = false;
                    break;
                }

                case SRTM_Command_FileEnd:
                {
                    FRESULT error;

                    PRINTF("DSP file playback complete\r\n");

                    file_playing = false;

                    error = f_close(&app->fileObject);
                    if (error)
                    {
                        PRINTF("Failed to close file on SD card\r\n");
                    }
                    *notify_shell = true;
                    break;
                }
                case SRTM_Command_FileStop:
                {
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP file stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP file stopped\r\n");
                        file_playing = false;
                        /* File has stopped playing */
                    }
                    *notify_shell = true;
                    break;
                }
                case SRTM_Command_FileError:
                {
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP requested file stop due to error failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP file stopped, unsupported format.\r\n");
                        file_playing = false;
                        /* File has stopped playing */
                    }
                    *notify_shell = true;
                    break;
                }
#if XA_CLIENT_PROXY
                case SRTM_Command_FilterCfg:
                {
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP Filter cfg failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("DSP Filter cfg success!\r\n");
                    }
                    *notify_shell = true;
                    break;
                }
#endif
                default:
                    PRINTF("Incoming unknown message category %d \r\n", msg->head.category);
                    break;
            }
            break;
    }
}

void handleDSPMessage(app_handle_t *app, srtm_message *msg)
{
    bool notify_shell = false;

    handleDSPMessageInner(app, msg, &notify_shell);

    if (notify_shell)
    {
        /* Signal to shell that response has been processed. */
        if (app->shell_task_handle != NULL)
        {
            xTaskNotifyGive(app->shell_task_handle);
        }
    }
}
/*${function:end}*/
