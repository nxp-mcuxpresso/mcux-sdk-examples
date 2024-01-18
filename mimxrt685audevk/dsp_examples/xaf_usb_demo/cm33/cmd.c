/*
 * Copyright 2021-2023 NXP
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

#include "composite.h"
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
static void initMessage(srtm_message *msg);

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellUsbSpeaker(shell_handle_t shellHandle, int32_t argc, char **argv);
static shell_status_t shellUsbMic(shell_handle_t shellHandle, int32_t argc, char **argv);
/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_composite_struct_t g_composite;
extern uint8_t *audioPlayDataBuff;
extern uint8_t *audioPlayDMATempBuff;
extern uint8_t *audioRecDataBuff;
extern uint8_t *audioRecDMATempBuff;

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Query DSP for component versions\r\n", shellEcho, 0);
SHELL_COMMAND_DEFINE(usb_speaker,
                     "\r\n\"usb_speaker\": Perform usb speaker device and playback on DSP\r\n"
                     "  USAGE: usb_speaker [start|stop]\r\n"
                     "    start          Start usb speaker device and playback on DSP\r\n"
                     "    stop           Stop usb speaker device and playback on DSP\r\n",
                     shellUsbSpeaker,
                     1);

SHELL_COMMAND_DEFINE(usb_mic,
                     "\r\n\"usb_mic\": Record DMIC audio and playback on usb microphone audio device\r\n"
                     "  USAGE: usb_mic [start|stop]\r\n"
                     "    start          Start record and playback on usb microphone audio device\r\n"
                     "    stop           Stop record and playback on usb microphone audio device\r\n",
                     shellUsbMic,
                     1);

static bool usb_playing   = false;
static bool usb_recording = false;

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;
static handleShellMessageCallback_t *g_handleShellMessageCallback;
static void *g_handleShellMessageCallbackData;
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

static shell_status_t shellUsbSpeaker(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (strcmp(argv[1], "start") == 0)
    {
        if (usb_recording)
        {
            PRINTF("[CM33 CMD] Stop the USB microphone first, then start the USB speaker again\r\n");
            return kStatus_SHELL_Error;
        }
        if (!usb_playing)
        {
            USB_DeviceAudioSpeakerStatusReset();
            USB_DeviceRun(g_composite.deviceHandle);
            usb_playing = true;

            /* Small delay for USB starting */
            TickType_t timeTMP = xTaskGetTickCount();
            while (xTaskGetTickCount() < (timeTMP + 500))
            {
                ;
            }

            /* Clear buffers */
            memset(audioPlayDataBuff, 0, AUDIO_SHARED_BUFFER_1_SIZE);

            msg.head.command = SRTM_Command_UsbSpeakerStart;

            /* Param 0 input buffer address */
            /* Param 1 input buffer size */
            /* Param 2 number of channels */
            /* Param 3 sampling_rate */
            /* Param 4 pcm_width */
            msg.param[0] = (uint32_t)(&audioPlayDataBuff[0]);
            msg.param[1] = 2 * ((0U != g_composite.audioUnified.audioPlayTransferSize) ?
                                    g_composite.audioUnified.audioPlayTransferSize :
                                    HS_ISO_OUT_ENDP_PACKET_SIZE);
            msg.param[2] = AUDIO_IN_FORMAT_CHANNELS;
            msg.param[3] = AUDIO_IN_SAMPLING_RATE;
            msg.param[4] = AUDIO_IN_FORMAT_BITS;

            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        }
        else
        {
            PRINTF("[CM33 CMD] USB is already playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (usb_playing)
        {
            msg.head.command = SRTM_Command_UsbSpeakerStop;
            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
            return kStatus_SHELL_Success;
        }
        else
        {
            PRINTF("[CM33 CMD] USB is not playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else
    {
        PRINTF("[CM33 CMD] Unknown parameter\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

static shell_status_t shellUsbMic(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (strcmp(argv[1], "start") == 0)
    {
        if (usb_playing)
        {
            PRINTF("[CM33 CMD] Stop the USB speaker first, then start the USB microphone again\r\n");
            return kStatus_SHELL_Error;
        }
        if (!usb_recording)
        {
            USB_DeviceAudioRecorderStatusReset();
            USB_DeviceRun(g_composite.deviceHandle);
            usb_recording = true;

            /* Small delay for USB starting */
            TickType_t timeTMP = xTaskGetTickCount();
            while (xTaskGetTickCount() < (timeTMP + 500))
            {
                ;
            }

            /* Clear buffers */
            memset(audioRecDataBuff, 0, AUDIO_SHARED_BUFFER_2_SIZE);

            msg.head.command = SRTM_Command_UsbMicStart;

            /* Param 0 output buffer address */
            /* Param 1 output buffer size */
            /* Param 2 number of channels */
            /* Param 3 sampling_rate */
            /* Param 4 pcm_width */
            msg.param[0] = (uint32_t)(&audioRecDataBuff[0]);
            msg.param[1] = AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE;
            msg.param[2] = AUDIO_IN_FORMAT_CHANNELS;
            msg.param[3] = AUDIO_IN_SAMPLING_RATE;
            msg.param[4] = AUDIO_IN_FORMAT_BITS;

            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
        }
        else
        {
            PRINTF("[CM33 CMD] USB is already recording\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (strcmp(argv[1], "stop") == 0)
    {
        if (usb_recording)
        {
            msg.head.command = SRTM_Command_UsbMicStop;
            g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
            return kStatus_SHELL_Success;
        }
        else
        {
            PRINTF("[CM33 CMD] USB is not recording\r\n");
            return kStatus_SHELL_Error;
        }
    }
    else
    {
        PRINTF("[CM33 CMD] Unknown parameter\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(usb_speaker));
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(usb_mic));

    g_handleShellMessageCallback     = handleShellMessageCallback;
    g_handleShellMessageCallbackData = arg;

#if !(defined(SHELL_NON_BLOCKING_MODE) && (SHELL_NON_BLOCKING_MODE > 0U))
    SHELL_Task(s_shellHandle);
#endif
}

static void handleDSPMessageInner(app_handle_t *app, srtm_message *msg, bool *notify_shell)
{
    uint32_t audioSpeakerPreReadDataCount = 0U;
    uint32_t preAudioSendCount            = 0U;
    *notify_shell                         = true;

    char string_buff[SRTM_CMD_PARAMS_MAX];

    if (msg->head.type == SRTM_MessageTypeResponse)
    {
        PRINTF("[CM33 CMD] [APP_DSP_IPC_Task] response from DSP, cmd: %d, error: %d\r\n", msg->head.command,
               msg->error);
    }

    /* Processing returned data*/
    switch (msg->head.category)
    {
        case SRTM_MessageCategory_GENERAL:
            switch (msg->head.command)
            {
                /* echo returns version info of key components*/
                case SRTM_Command_ECHO:
                    PRINTF("[CM33 CMD] Component versions from DSP:\r\n");
                    PRINTF("[CM33 CMD] Audio Framework version %d.%d \r\n", msg->param[0] >> 16, msg->param[0] & 0xFF);
                    PRINTF("[CM33 CMD] Audio Framework API version %d.%d\r\n", msg->param[1] >> 16,
                           msg->param[1] & 0xFF);
                    PRINTF("[CM33 CMD] NatureDSP Lib version %d.%d\r\n", msg->param[2] >> 16, msg->param[2] & 0xFF);
                    PRINTF("[CM33 CMD] NatureDSP API version %d.%d\r\n", msg->param[3] >> 16, msg->param[3] & 0xFF);
                    break;

                case SRTM_Command_SYST:
                    break;
                default:
                    PRINTF("[CM33 CMD] Incoming unknown message command %d from category %d \r\n", msg->head.command,
                           msg->head.category);
            }
            break;

        case SRTM_MessageCategory_AUDIO:
            if (usb_playing && (msg->head.command < SRTM_Command_UsbSpeakerStart || msg->head.command > SRTM_Command_UsbSpeakerError))
            {
                PRINTF("[CM33 CMD] This command cannot be processed at this time because USB is being played!\r\n");
                break;
            }

            switch (msg->head.command)
            {
                case SRTM_Print_String:
                    for (int i = 0; i < SRTM_CMD_PARAMS_MAX; i++)
                    {
                        string_buff[i] = (char)msg->param[i];
                    }
                    PRINTF("[CM33 CMD] %s", string_buff);
                    break;

                case SRTM_Command_UsbSpeakerStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP USB playback start failed! return error = %d\r\n", msg->error);
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB playback start\r\n");
                    }

                    /* Release shell to be able to set different commands */
                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerData:
                    msg->head.type = SRTM_MessageTypeResponse;

                    if ((USB_AudioSpeakerBufferSpaceUsed() < (g_composite.audioUnified.audioPlayTransferSize)) &&
                        (g_composite.audioUnified.startPlayFlag == 1U))
                    {
                        g_composite.audioUnified.startPlayFlag          = 0;
                        g_composite.audioUnified.speakerDetachOrNoInput = 1;
                    }
                    if (0U != g_composite.audioUnified.startPlayFlag)
                    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
                        USB_DeviceCalculateFeedback();
#endif

                        msg->param[0]     = (uint32_t)(&audioPlayDataBuff[g_composite.audioUnified.tdReadNumberPlay]);
                        msg->param[1]     = g_composite.audioUnified.audioPlayTransferSize;
                        preAudioSendCount = g_composite.audioUnified.audioSendCount[0];
                        g_composite.audioUnified.audioSendCount[0] += g_composite.audioUnified.audioPlayTransferSize;
                        if (preAudioSendCount > g_composite.audioUnified.audioSendCount[0])
                        {
                            g_composite.audioUnified.audioSendCount[1] += 1U;
                        }
                        g_composite.audioUnified.audioSendTimes++;
                        g_composite.audioUnified.tdReadNumberPlay += g_composite.audioUnified.audioPlayTransferSize;
                        if (g_composite.audioUnified.tdReadNumberPlay >= g_composite.audioUnified.audioPlayBufferSize)
                        {
                            g_composite.audioUnified.tdReadNumberPlay = 0;
                        }
                        audioSpeakerPreReadDataCount = g_composite.audioUnified.audioSpeakerReadDataCount[0];
                        g_composite.audioUnified.audioSpeakerReadDataCount[0] +=
                            g_composite.audioUnified.audioPlayTransferSize;
                        if (audioSpeakerPreReadDataCount > g_composite.audioUnified.audioSpeakerReadDataCount[0])
                        {
                            g_composite.audioUnified.audioSpeakerReadDataCount[1] += 1U;
                        }
                    }
                    else
                    {
                        msg->param[0] = (uint32_t)(&audioPlayDMATempBuff[0]);
                        msg->param[1] = (0U != g_composite.audioUnified.audioPlayTransferSize) ?
                                            g_composite.audioUnified.audioPlayTransferSize :
                                            HS_ISO_OUT_ENDP_PACKET_SIZE;
                    }

                    /* Send response message to DSP with new data */
                    dsp_ipc_send_sync(msg);

                    /* Don't release shell until receive notification of usb end */
                    *notify_shell = false;
                    break;

                case SRTM_Command_UsbSpeakerEnd:
                    PRINTF("[CM33 CMD] DSP USB playback complete\r\n");

                    USB_DeviceStop(g_composite.deviceHandle);
                    USB_DeviceAudioSpeakerStatusReset();
                    usb_playing = false;

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerStop:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP USB stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbSpeakerError:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP requested USB stop due to error failed! return error = %d\r\n",
                               msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB stopped, unsupported format!!!.\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioSpeakerStatusReset();
                        usb_playing = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP USB record start failed! return error = %d\r\n", msg->error);
                        USB_DeviceStop(g_composite.deviceHandle);
                        usb_recording = false;
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB record start\r\n");
                    }

                    /* Release shell to be able to send different command */
                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicData:
                    g_composite.audioUnified.tdWriteNumberRec = msg->param[0];
                    *notify_shell                             = true;
                    break;

                case SRTM_Command_UsbMicEnd:
                    PRINTF("[CM33 CMD] DSP USB recording complete\r\n");

                    USB_DeviceStop(g_composite.deviceHandle);
                    USB_DeviceAudioRecorderStatusReset();
                    usb_recording = false;

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicStop:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP USB stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioRecorderStatusReset();
                        usb_recording = false;
                    }

                    *notify_shell = true;
                    break;

                case SRTM_Command_UsbMicError:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP requested USB stop due to error failed! return error = %d\r\n",
                               msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP USB stopped\r\n");
                        USB_DeviceStop(g_composite.deviceHandle);
                        USB_DeviceAudioRecorderStatusReset();
                        usb_recording = false;
                    }

                    *notify_shell = true;
                    break;

                default:
                    PRINTF("[CM33 CMD] Incoming unknown message category %d \r\n", msg->head.category);
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
