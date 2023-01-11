/*
 * Copyright 2019 NXP
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

/* Enable AAC decoder*/
#if XA_AAC_DECODER
/* Demo media file includes */
#include "aac.adts.h"
#include "aac.ref.h"
#endif
/* Enable MP3 decoder*/
#if (XA_MP3_DECODER == 1 || XA_PCM_GAIN == 1 || XA_SRC_PP_FX == 1)
#include "hihat.mp3.h"
#include "hihat_48k.mp3.h"
#include "hihat_dec_out.pcm.h"
#endif
/* Enable Vorbis decoder*/
#if XA_VORBIS_DECODER
#include "hihat_vor.h"
#include "hihat_pcm.h"
#include "hihat.ogg.h"
#include "hihat_vorbis_dec_out_trim.pcm.h"
#endif
/* Enable Opus decoder*/
#if XA_OPUS_DECODER
#include "testvector04.bit.h"
#include "testvector04-48000-2ch_trim.out.h"
#endif
/* Enable Opus encoder*/
#if XA_OPUS_ENCODER
#include "testvector11-16000-1ch_trim.out.h"
#include "testvector11-16kHz-1ch-20kbps_trim.bit.h"
#endif
/* Enable SBC decoder*/
#if XA_SBC_DECODER
#include "sbc_test_02_trim.sbc.h"
#include "sbc_test_02_trim.out.h"
#endif
/* Enable SBC encoder*/
#if XA_SBC_ENCODER
#include "hihat_trim.pcm.h"
#include "hihat_trim.sbc.h"
#endif

/*${header:end}*/

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define AUDIO_MAX_INPUT_BUFFER  (AUDIO_SHARED_BUFFER_1_SIZE)
#define AUDIO_MAX_OUTPUT_BUFFER (AUDIO_SHARED_BUFFER_2_SIZE)

#define AUDIO_VORBIS_INPUT_OGG 0U
#define AUDIO_VORBIS_INPUT_RAW 1U

#define AUDIO_OUTPUT_BUFFER   0
#define AUDIO_OUTPUT_RENDERER 1

/* Audio in/out buffers for one-shot DSP handling. */
static uint8_t *s_audioInput  = (uint8_t *)AUDIO_SHARED_BUFFER_1;
static uint8_t *s_audioOutput = (uint8_t *)AUDIO_SHARED_BUFFER_2;
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*${prototype:start}*/
static void initMessage(srtm_message *msg);

static shell_status_t shellEcho(shell_handle_t shellHandle, int32_t argc, char **argv);

/* Enable just aac and mp3 to save memory*/
#if XA_AAC_DECODER
static shell_status_t shellAAC(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_MP3_DECODER
static shell_status_t shellMP3(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_OPUS_DECODER
static shell_status_t shellOpusDec(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_OPUS_ENCODER
static shell_status_t shellOpusEnc(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_SBC_DECODER
static shell_status_t shellSbcDec(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_SBC_ENCODER
static shell_status_t shellSbcEnc(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_VORBIS_DECODER
static shell_status_t shellVORBIS(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv);
#if XA_SRC_PP_FX
static shell_status_t shellSRC(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_PCM_GAIN
static shell_status_t shellGAIN(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif

/*${prototype:end}*/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*${variable:start}*/
SHELL_COMMAND_DEFINE(version, "\r\n\"version\": Query DSP for component versions\r\n", shellEcho, 0);

#if XA_MP3_DECODER == 1 || XA_AAC_DECODER || XA_VORBIS_DECODER
SHELL_COMMAND_DEFINE(file,
                     "\r\n\"file\": Perform audio file decode and playback on DSP\r\n"
                     "  USAGE: file [list|stop|<audio_file>]\r\n"
                     "    list          List audio files on SD card available for playback\r\n"
                     "    <audio_file>  Select file from SD card and start playback\r\n",
                     shellFile,
                     1);
#endif

/* Enable just aac and mp3 to save memory*/
#if XA_AAC_DECODER
SHELL_COMMAND_DEFINE(aac,
                     "\r\n\"aac\": Perform AAC decode on DSP\r\n"
                     "  USAGE: aac [buffer|codec]\r\n"
                     "  OPTIONS:\r\n"
                     "    buffer  Output decoded data to memory buffer\r\n"
                     "    codec   Output decoded data to codec for speaker playback\r\n",
                     shellAAC,
                     1);
#endif
#if XA_MP3_DECODER
SHELL_COMMAND_DEFINE(mp3,
                     "\r\n\"mp3\": Perform MP3 decode on DSP\r\n"
                     "  USAGE: mp3 [buffer|codec]\r\n"
                     "  OPTIONS:\r\n"
                     "    buffer  Output decoded data to memory buffer\r\n"
                     "    codec   Output decoded data to codec for speaker playback\r\n",
                     shellMP3,
                     1);
#endif
#if XA_OPUS_DECODER
SHELL_COMMAND_DEFINE(opusdec,
                     "\r\n\"opusdec\": Perform OPUS decode on DSP\r\n"
                     "  USAGE: opusdec [buffer|codec]\r\n"
                     "  OPTIONS:\r\n"
                     "    buffer  Output decoded data to memory buffer\r\n"
                     "    codec   Output decoded data to codec for speaker playback\r\n",
                     shellOpusDec,
                     1);
#endif
#if XA_OPUS_ENCODER
SHELL_COMMAND_DEFINE(opusenc, "\r\n\"opusenc\": Perform OPUS encode on DSP\r\n", shellOpusEnc, 0);
#endif
#if XA_SBC_DECODER
SHELL_COMMAND_DEFINE(sbcdec,
                     "\r\n\"sbcdec\": Perform SBC decode on DSP\r\n"
                     "  USAGE: sbcdec [buffer|codec]\r\n"
                     "  OPTIONS:\r\n"
                     "    buffer  Output decoded data to memory buffer\r\n"
                     "    codec   Output decoded data to codec for speaker playback\r\n",
                     shellSbcDec,
                     1);
#endif
#if XA_SBC_ENCODER
SHELL_COMMAND_DEFINE(sbcenc, "\r\n\"sbcenc\": Perform SBC encode on DSP\r\n", shellSbcEnc, 0);
#endif
#if XA_VORBIS_DECODER
SHELL_COMMAND_DEFINE(vorbis,
                     "\r\n\"vorbis\": Perform VORBIS decode on DSP\r\n"
                     "  USAGE: vorbis [buffer|codec] [ogg|raw]\r\n"
                     "  OPTIONS:\r\n"
                     "    buffer  Output decoded data to memory buffer\r\n"
                     "    codec   Output decoded data to codec for speaker playback\r\n"
                     "    ogg     Decode OGG VORBIS data\r\n"
                     "    raw     Decode raw VORBIS data\r\n",
                     shellVORBIS,
                     2);
#endif
#if XA_SRC_PP_FX
SHELL_COMMAND_DEFINE(src, "\r\n\"src\" Perform sample rate conversion on DSP\r\n", shellSRC, 0);
#endif
#if XA_PCM_GAIN
SHELL_COMMAND_DEFINE(gain, "\r\n\"gain\": Perform PCM gain adjustment on DSP\r\n", shellGAIN, 0);
#endif
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

    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;

    if (!app->sdcardInserted)
    {
        PRINTF("Please insert an SD card with audio files and retry this command\r\n");
        return kStatus_SHELL_Success;
    }

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
#if XA_MP3_DECODER == 1
                if (dot && strncmp(dot + 1, "mp3", 3) == 0)
                {
                    PRINTF("  %s\r\n", fileInformation.fname);
                    count++;
                }
#endif

#if XA_AAC_DECODER == 1
                if (dot && strncmp(dot + 1, "aac", 3) == 0)
                {
                    PRINTF("  %s\r\n", fileInformation.fname);
                    count++;
                }
#endif

#if XA_VORBIS_DECODER == 1
                if (dot && strncmp(dot + 1, "ogg", 3) == 0)
                {
                    PRINTF("  %s\r\n", fileInformation.fname);
                    count++;
                }
#endif
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
        filename         = argv[1];
        file_ptr         = (char *)AUDIO_SHARED_BUFFER_1;
        msg.head.command = SRTM_Command_FileStart;

        /* Param 0 Encoded input buffer address*/
        /* Param 1 Encoded input buffer size*/
        /* Param 2 EOF (true/false) */
        /* Param 3 Audio codec component type */
        msg.param[0] = (uint32_t)file_ptr;
        msg.param[1] = FILE_PLAYBACK_INITIAL_READ_SIZE;
        msg.param[2] = 0;

        dot = strrchr(filename, '.');
#if XA_MP3_DECODER == 1
        if (dot && strncmp(dot + 1, "mp3", 3) == 0)
        {
            msg.param[3] = DSP_COMPONENT_MP3;
            count        = 1;
        }
#endif
#if XA_AAC_DECODER == 1
        if (dot && strncmp(dot + 1, "aac", 3) == 0)
        {
            msg.param[3] = DSP_COMPONENT_AAC;
            count        = 1;
        }
#endif
#if XA_VORBIS_DECODER == 1
        if (dot && strncmp(dot + 1, "ogg", 3) == 0)
        {
            msg.param[3] = DSP_COMPONENT_VORBIS;
            count        = 1;
        }
#endif
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
            msg.param[1] = bytes_read;
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

/* Enable just aac and mp3 to save memory*/
#if XA_AAC_DECODER
static shell_status_t shellAAC(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    /* Param 0 AAC input buffer address*/
    /* Param 1 AAC input buffer size*/
    /* Param 2 PCM output buffer address*/
    /* Param 3 PCM output buffer size*/
    /* Param 4 decode output location */
    /* Param 5 return parameter, MP3 output actual size*/
    /* Param 6 return parameter, actual read size*/

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_AAC;
    /* Param 1 AAC input buffer address*/
    msg.param[0] = (unsigned int)(&s_audioInput[0]);
    /* Param 2 AAC input buffer size*/
    msg.param[1] = sizeof(AACBUFFER);
    /* Param 3 AAC output buffer address*/
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    /* Param 4 AAC output buffer size*/
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    /* Parse shell arg for output type */
    if (strcmp(argv[1], "codec") == 0)
    {
        PRINTF("Setting decode output to codec renderer\r\n");
        msg.param[4] = AUDIO_OUTPUT_RENDERER;
    }
    else
    {
        PRINTF("Setting decode output to audio buffer\r\n");
        msg.param[4] = AUDIO_OUTPUT_BUFFER;
    }

    /* Copy encoded audio clip into shared memory buffer */
    memcpy(s_audioInput, AACBUFFER, sizeof(AACBUFFER));

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_MP3_DECODER
static shell_status_t shellMP3(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_MP3;
    /* Param 0 MP3 input buffer address*/
    /* Param 1 MP3 input buffer size*/
    /* Param 2 PCM output buffer address*/
    /* Param 3 PCM output buffer size*/
    /* Param 4 decode output location */
    /* Param 5 return parameter, MP3 output actual size*/
    /* Param 6 return parameter, actual read size*/

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    /* Parse shell arg for output type */
    if (strcmp(argv[1], "codec") == 0)
    {
        PRINTF("Setting decode output to codec renderer\r\n");
        msg.param[4] = AUDIO_OUTPUT_RENDERER;

        /* Copy encoded audio clip into shared memory buffer */
        memcpy(s_audioInput, SRTM_MP3_48K_INPUTBUFFER, sizeof(SRTM_MP3_48K_INPUTBUFFER));
        msg.param[1] = sizeof(SRTM_MP3_48K_INPUTBUFFER);
    }
    else
    {
        PRINTF("Setting decode output to audio buffer\r\n");
        msg.param[4] = AUDIO_OUTPUT_BUFFER;

        /* Copy encoded audio clip into shared memory buffer */
        memcpy(s_audioInput, SRTM_MP3_INPUTBUFFER, sizeof(SRTM_MP3_INPUTBUFFER));
        msg.param[1] = sizeof(SRTM_MP3_INPUTBUFFER);
    }

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_VORBIS_DECODER
static shell_status_t shellVORBIS(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_VORBIS;
    /* Param 0 VORBIS input buffer address */
    /* Param 1 VORBIS input buffer size */
    /* Param 2 PCM output buffer address */
    /* Param 3 PCM output buffer size */
    /* Param 4 decode output location */
    /* Param 5 return parameter, actual read size */
    /* Param 6 return parameter, actual write size */
    /* Param 7 VORBIS input buffer is raw (1) or OGG encapsulated (0) */

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    /* Parse shell arg for output type */

    if ((strcmp(argv[1], "codec") == 0) || (strcmp(argv[2], "codec") == 0))
    {
        PRINTF("Setting decode output to codec renderer\r\n");
        msg.param[4] = AUDIO_OUTPUT_RENDERER;
    }
    else
    {
        PRINTF("Setting decode output to audio buffer\r\n");
        msg.param[4] = AUDIO_OUTPUT_BUFFER;
    }

    if ((strcmp(argv[1], "ogg") == 0) || (strcmp(argv[2], "ogg") == 0))
    {
        PRINTF("Setting OGG VORBIS as decode input\r\n");
        msg.param[7] = AUDIO_VORBIS_INPUT_OGG;

        /* Copy encoded audio clip into shared memory buffer */
        memcpy(s_audioInput, SRTM_OGG_VORBIS_INPUTBUFFER, sizeof(SRTM_OGG_VORBIS_INPUTBUFFER));
        msg.param[1] = sizeof(SRTM_OGG_VORBIS_INPUTBUFFER);
    }
    else
    {
        PRINTF("Setting raw VORBIS as decode input\r\n");
        msg.param[7] = AUDIO_VORBIS_INPUT_RAW;

        /* Copy encoded audio clip into shared memory buffer */
        memcpy(s_audioInput, SRTM_VORBIS_INPUTBUFFER, sizeof(SRTM_VORBIS_INPUTBUFFER));
        msg.param[1] = sizeof(SRTM_VORBIS_INPUTBUFFER);
    }

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_OPUS_DECODER
static shell_status_t shellOpusDec(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_OPUS_DEC;
    /* Param 0 OPUS input buffer address*/
    /* Param 1 OPUS input buffer size*/
    /* Param 2 PCM output buffer address*/
    /* Param 3 PCM output buffer size*/
    /* Param 4 decode output location */
    /* Param 5 return parameter, actual read size */
    /* Param 6 return parameter, actual write size */

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    if (strcmp(argv[1], "codec") == 0)
    {
        PRINTF("Setting decode output to codec renderer\r\n");
        msg.param[4] = AUDIO_OUTPUT_RENDERER;
    }
    else
    {
        PRINTF("Setting decode output to audio buffer\r\n");
        msg.param[4] = AUDIO_OUTPUT_BUFFER;
    }

    /* Copy encoded audio clip into shared memory buffer */
    memcpy(s_audioInput, SRTM_OPUS_INPUTBUFFER, sizeof(SRTM_OPUS_INPUTBUFFER));
    msg.param[1] = sizeof(SRTM_OPUS_INPUTBUFFER);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_OPUS_ENCODER
static shell_status_t shellOpusEnc(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_OPUS_ENC;
    /* Param 0 PCM input buffer address*/
    /* Param 1 PCM input buffer size*/
    /* Param 2 OPUS output buffer address*/
    /* Param 3 OPUS output buffer size*/
    /* Param 4 return parameter, actual read size */
    /* Param 5 return parameter, actual write size */

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    /* Copy pcm audio clip into shared memory buffer */
    memcpy(s_audioInput, SRTM_OPUS_ENC_INPUTBUFFER, sizeof(SRTM_OPUS_ENC_INPUTBUFFER));
    msg.param[1] = sizeof(SRTM_OPUS_ENC_INPUTBUFFER);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_SBC_DECODER
static shell_status_t shellSbcDec(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_SBC_DEC;
    /* Param 0 SBC input buffer address*/
    /* Param 1 SBC input buffer size*/
    /* Param 2 PCM output buffer address*/
    /* Param 3 PCM output buffer size*/
    /* Param 4 decode output location */
    /* Param 5 return parameter, actual read size */
    /* Param 6 return parameter, actual write size */

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    if (strcmp(argv[1], "codec") == 0)
    {
        PRINTF("Setting decode output to codec renderer\r\n");
        msg.param[4] = AUDIO_OUTPUT_RENDERER;
    }
    else
    {
        PRINTF("Setting decode output to audio buffer\r\n");
        msg.param[4] = AUDIO_OUTPUT_BUFFER;
    }

    /* Copy encoded audio clip into shared memory buffer */
    memcpy(s_audioInput, SRTM_SBC_INPUTBUFFER, sizeof(SRTM_SBC_INPUTBUFFER));
    msg.param[1] = sizeof(SRTM_SBC_INPUTBUFFER);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_SBC_ENCODER
static shell_status_t shellSbcEnc(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_SBC_ENC;
    /* Param 0 PCM input buffer address*/
    /* Param 1 PCM input buffer size*/
    /* Param 2 SBC output buffer address*/
    /* Param 3 SBC output buffer size*/
    /* Param 4 return parameter, actual read size */
    /* Param 5 return parameter, actual write size */

    msg.param[0] = (unsigned int)&s_audioInput[0];
    msg.param[2] = (unsigned int)&s_audioOutput[0];
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;

    /* Copy pcm audio clip into shared memory buffer */
    memcpy(s_audioInput, SRTM_SBC_ENC_INPUTBUFFER, sizeof(SRTM_SBC_ENC_INPUTBUFFER));
    msg.param[1] = sizeof(SRTM_SBC_ENC_INPUTBUFFER);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_SRC_PP_FX
static shell_status_t shellSRC(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    /* Param 0 SRC input buffer 1 address*/
    /* Param 1 SRC input buffer 1 size*/
    /* Param 2 SRC input sampling rate*/
    /* Param 3 SRC input channels*/
    /* Param 4 SRC input sample width*/
    /* Param 5 SRC output buffer address*/
    /* Param 6 SRC output buffer size*/
    /* Param 7 SRC output sampling rate*/
    /* Param 8 return paramter, actual read size of input*/
    /* Param 9 return paramter, actual write size of output*/

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_SRC;
    msg.param[0]      = (unsigned int)(&s_audioInput[0]);
    msg.param[1]      = sizeof(SRTM_MP3_REFBUFFER);
    msg.param[2]      = 44100;
    msg.param[3]      = 2;
    msg.param[4]      = 16;
    msg.param[5]      = (unsigned int)(&s_audioOutput[0]);
    msg.param[6]      = AUDIO_MAX_OUTPUT_BUFFER;
    msg.param[7]      = 48000;

    memcpy(s_audioInput, SRTM_MP3_REFBUFFER, sizeof(SRTM_MP3_REFBUFFER));

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_PCM_GAIN
static shell_status_t shellGAIN(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    initMessage(&msg);

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_GAIN;
    /* Param 0 PCM input buffer address*/
    /* Param 1 PCM input buffer size*/
    /* Param 2 PCM output buffer address*/
    /* Param 3 PCM output buffer size*/
    /* Param 4 PCM sampling rate, default 44100*/
    /* Param 5 PCM number of channels, only 1 or 2 supported, default 1*/
    /* Param 6 PCM sample width, default 16*/
    /* Param 7 Gain control index, default is 4, range is 0 to 6 -> {0db, -6db, -12db, -18db, 6db, 12db, 18db}*/
    /* Param 8 return parameter, actual read bytes*/
    /* Param 9 return parameter, actual written bytes*/

    /* Use MP3 Dec buffers as test, to save space */
    msg.param[0] = (unsigned int)(&s_audioInput[0]);
    msg.param[1] = sizeof(SRTM_MP3_REFBUFFER);
    msg.param[2] = (unsigned int)(&s_audioOutput[0]);
    msg.param[3] = AUDIO_MAX_OUTPUT_BUFFER;
    msg.param[4] = 44100;
    msg.param[5] = 2;
    msg.param[6] = 16;
    msg.param[7] = 4;

    memcpy(s_audioInput, SRTM_MP3_REFBUFFER, sizeof(SRTM_MP3_REFBUFFER));

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if XA_CLIENT_PROXY
static shell_status_t shellEAPeffect(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    int effectNum    = atoi(argv[1]);
    initMessage(&msg);

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
#if XA_AAC_DECODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(aac));
#endif
#if XA_MP3_DECODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(mp3));
#endif
#if XA_OPUS_DECODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(opusdec));
#endif
#if XA_OPUS_ENCODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(opusenc));
#endif
#if XA_SBC_DECODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sbcdec));
#endif
#if XA_SBC_ENCODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(sbcenc));
#endif
#if XA_VORBIS_DECODER
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(vorbis));
#endif
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(file));
#if XA_SRC_PP_FX
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(src));
#endif
#if XA_PCM_GAIN
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(gain));
#endif
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
                    PRINTF("MP3 Decoder Lib version %d.%d\r\n", msg->param[4] >> 16, msg->param[4] & 0xFF);
                    PRINTF("AAC Decoder Lib version %d.%d\r\n", msg->param[5] >> 16, msg->param[5] & 0xFF);
#if XA_VORBIS_DECODER
                    PRINTF("VORBIS Decoder Lib version %d.%d\r\n", msg->param[6] >> 16, msg->param[6] & 0xFF);
#endif
                    PRINTF("OPUS Codec Lib version %d.%d\r\n", msg->param[7] >> 16, msg->param[7] & 0xFF);
                    PRINTF("SBC Decoder Lib version %d.%d\r\n", msg->param[8] >> 16, msg->param[8] & 0xFF);
                    PRINTF("SBC Encoder Lib version %d.%d\r\n", msg->param[9] >> 16, msg->param[9] & 0xFF);
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
/* Enable just aac and mp3 to save memory*/
#if XA_AAC_DECODER
                case SRTM_Command_AAC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP AAC decoder failed, return error = %d\r\n", msg->error);
                    }

                    if (msg->param[4] == AUDIO_OUTPUT_BUFFER)
                    {
                        size_t ref_buffer_size = sizeof(SRTM_AAC_REFBUFFER);
                        PRINTF("AAC decoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                        PRINTF("  Checking decode results...\r\n");
                        for (int i = 0; (i < msg->param[6]) && (i < ref_buffer_size); i++)
                        {
                            if (s_audioOutput[i] != SRTM_AAC_REFBUFFER[i])
                            {
                                PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                       s_audioOutput[i], SRTM_AAC_REFBUFFER[i]);
                                return;
                            }
                        }
                        PRINTF("  Decode output matches reference\r\n");
                    }
                    else
                    {
                        PRINTF("AAC decode complete\r\n");
                    }
                    break;
#endif
#if XA_MP3_DECODER
                case SRTM_Command_MP3:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP MP3 decoder failed, return error = %d\r\n", msg->error);
                    }

                    if (msg->param[4] == AUDIO_OUTPUT_BUFFER)
                    {
                        size_t ref_buffer_size = sizeof(SRTM_MP3_REFBUFFER);
                        PRINTF("MP3 decoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                        PRINTF("  Checking decode results...\r\n");
                        for (int i = 0; (i < msg->param[6]) && (i < ref_buffer_size); i++)
                        {
                            if (s_audioOutput[i] != SRTM_MP3_REFBUFFER[i])
                            {
                                PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                       s_audioOutput[i], SRTM_MP3_REFBUFFER[i]);
                                return;
                            }
                        }
                        PRINTF("  Decode output matches reference\r\n");
                    }
                    else
                    {
                        PRINTF("MP3 decode complete\r\n");
                    }
                    break;
#endif
#if XA_VORBIS_DECODER
                case SRTM_Command_VORBIS:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP VORBIS decoder failed, return error = %d\r\n", msg->error);
                    }

                    if (msg->param[4] == AUDIO_OUTPUT_BUFFER)
                    {
                        const unsigned char *ref_buffer;
                        size_t ref_buffer_size;

                        if (msg->param[7] == AUDIO_VORBIS_INPUT_OGG)
                        {
                            ref_buffer      = SRTM_OGG_VORBIS_REFBUFFER;
                            ref_buffer_size = sizeof(SRTM_OGG_VORBIS_REFBUFFER);
                        }
                        else
                        {
                            ref_buffer      = SRTM_VORBIS_REFBUFFER;
                            ref_buffer_size = sizeof(SRTM_VORBIS_REFBUFFER);
                        }

                        PRINTF("VORBIS decoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                        PRINTF("  Checking decode results...\r\n");

                        for (int i = 0; (i < msg->param[6]) && (i < ref_buffer_size); i++)
                        {
                            if (s_audioOutput[i] != ref_buffer[i])
                            {
                                PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                       s_audioOutput[i], ref_buffer[i]);
                                return;
                            }
                        }
                        PRINTF("  Decode output matches reference\r\n");
                    }
                    else
                    {
                        PRINTF("VORBIS decode complete\r\n");
                    }

                    break;
#endif
#if XA_OPUS_DECODER
                case SRTM_Command_OPUS_DEC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP OPUS decoder failed, return error = %d\r\n", msg->error);
                    }

                    if (msg->param[4] == AUDIO_OUTPUT_BUFFER)
                    {
                        size_t ref_buffer_size = sizeof(SRTM_OPUS_REFBUFFER);
                        PRINTF("OPUS decoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                        PRINTF("  Checking decode results...\r\n");
                        for (int i = 0; (i < msg->param[6]) && (i < ref_buffer_size); i++)
                        {
                            if (s_audioOutput[i] != SRTM_OPUS_REFBUFFER[i])
                            {
                                PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                       s_audioOutput[i], SRTM_OPUS_REFBUFFER[i]);
                                return;
                            }
                        }
                        PRINTF("  Decode output matches reference\r\n");
                    }
                    else
                    {
                        PRINTF("OPUS decode complete\r\n");
                    }
                    break;
#endif
#if XA_OPUS_ENCODER
                case SRTM_Command_OPUS_ENC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP OPUS encoder failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("OPUS encoder read %d bytes and output %d bytes \r\n", msg->param[4], msg->param[5]);
                    PRINTF("  Checking encode results...\r\n");
                    for (int i = 0; (i < msg->param[5]) && (i < sizeof(SRTM_OPUS_ENC_REFBUFFER)); i++)
                    {
                        if (s_audioOutput[i] != SRTM_OPUS_ENC_REFBUFFER[i])
                        {
                            PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i, s_audioOutput[i],
                                   SRTM_OPUS_ENC_REFBUFFER[i]);
                            return;
                        }
                    }
                    PRINTF("  Encode output matches reference\r\n");
                    break;
#endif
#if XA_SBC_DECODER
                case SRTM_Command_SBC_DEC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP SBC decoder failed, return error = %d\r\n", msg->error);
                    }

                    if (msg->param[4] == AUDIO_OUTPUT_BUFFER)
                    {
                        size_t ref_buffer_size = sizeof(SRTM_SBC_REFBUFFER);
                        PRINTF("SBC decoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                        PRINTF("  Checking decode results...\r\n");
                        for (int i = 0; (i < msg->param[6]) && (i < ref_buffer_size); i++)
                        {
                            if (s_audioOutput[i] != SRTM_SBC_REFBUFFER[i])
                            {
                                PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                       s_audioOutput[i], SRTM_SBC_REFBUFFER[i]);
                                return;
                            }
                        }
                        PRINTF("  Decode output matches reference\r\n");
                    }
                    else
                    {
                        PRINTF("SBC decode complete\r\n");
                    }
                    break;
#endif
#if XA_SBC_ENCODER
                case SRTM_Command_SBC_ENC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP SBC encoder failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("SBC encoder read %d bytes and output %d bytes \r\n", msg->param[4], msg->param[5]);
                    PRINTF("  Checking encode results...\r\n");
                    for (int i = 0; (i < msg->param[5]) && (i < sizeof(SRTM_SBC_ENC_REFBUFFER)); i++)
                    {
                        if (s_audioOutput[i] != SRTM_SBC_ENC_REFBUFFER[i])
                        {
                            PRINTF("  Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i, s_audioOutput[i],
                                   SRTM_SBC_ENC_REFBUFFER[i]);
                            return;
                        }
                    }
                    PRINTF("  Encode output matches reference\r\n");
                    break;
#endif
#if XA_SRC_PP_FX
                case SRTM_Command_SRC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP Sampling Rate Converter failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("SRC read %d bytes and output %d bytes \r\n", msg->param[8], msg->param[9]);
#endif
#if XA_PCM_GAIN
                    break;

                case SRTM_Command_GAIN:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("DSP Gain Control Process failed! return error = %d\r\n", msg->error);
                        break;
                    }
                    PRINTF("PCM Gain Control read %d bytes and write %d bytes \r\n", msg->param[8], msg->param[9]);
                    break;
#endif
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
                        BOARD_MuteRightChannel(msg->param[0] == 1);
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
                    error    = f_read(&app->fileObject, file_ptr,
                                   msg->param[0] == 0 ? FILE_PLAYBACK_READ_SIZE : msg->param[0], &bytes_read);
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
                    BOARD_MuteRightChannel(false);
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
                        BOARD_MuteRightChannel(false);
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
                        BOARD_MuteRightChannel(false);
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
