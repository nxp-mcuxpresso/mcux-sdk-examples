/*
 * Copyright 2019-2023 NXP
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
#include "aac.h"
#endif
/* Enable MP3 decoder*/
#if (XA_MP3_DECODER == 1)
#include "mp3.h"
#endif

/* Enable Vorbis decoder*/
#if XA_VORBIS_DECODER
#include "rawvorbis.h"
#include "oggvorbis.h"
#endif
/* Enable Opus decoder*/
#if XA_OPUS_DECODER
#include "testvector04.bit.h"
#include "opus.h"
#endif
/* Enable SBC decoder*/
#if XA_SBC_DECODER
#include "sbc.h"
#endif
/* Enable Opus encoder*/
#if XA_OPUS_ENCODER
#include "testvector11-16000-1ch_trim.out.h"
#include "testvector11-16kHz-1ch-20kbps_trim.bit.h"
#endif
/* Enable SBC encoder*/
#if XA_SBC_ENCODER
#include "hihat_trim.pcm.h"
#include "hihat_trim.sbc.h"
#endif

#if (XA_PCM_GAIN == 1 || XA_SRC_PP_FX == 1)
#include "hihat_dec_out.pcm.h"
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

#define SAMPLE_RATE_96KHZ 96000
#define SAMPLE_RATE_48KHZ 48000
#define BIT_WIDTH_32      32
#define BIT_WIDTH_24      24
#define BIT_WIDTH_16      16

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

#if (XA_AAC_DECODER || XA_MP3_DECODER || XA_VORBIS_DECODER || XA_OPUS_DECODER || XA_SBC_DECODER)
static shell_status_t shellDecoder(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if (XA_OPUS_ENCODER || XA_SBC_ENCODER)
static shell_status_t shellEncoder(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
static shell_status_t shellFile(shell_handle_t shellHandle, int32_t argc, char **argv);
#if XA_SRC_PP_FX
static shell_status_t shellSRC(shell_handle_t shellHandle, int32_t argc, char **argv);
#endif
#if XA_PCM_GAIN
static shell_status_t shellGAIN(shell_handle_t shellHandle, int32_t argc, char **argv);
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
                     "  USAGE: file [list|stop|<audio_file> "
#ifdef MULTICHANNEL
                     "[<nchannel>]"
#endif
                     "]\r\n"
                     "    list          List audio files on SD card available for playback\r\n"
                     "    <audio_file>  Select file from SD card and start playback\r\n"
#ifdef MULTICHANNEL
                     "    <nchannel>    Select the number of channels (2 or 8 can be selected).\r\n"
#endif
                     ,
                     shellFile,
                     SHELL_IGNORE_PARAMETER_COUNT);
#endif

/* Enable just aac and mp3 to save memory*/
#if (XA_AAC_DECODER || XA_MP3_DECODER || XA_OPUS_DECODER || XA_SBC_DECODER || XA_VORBIS_DECODER)
SHELL_COMMAND_DEFINE(decoder,
                     "\r\n\"decoder\": Perform decode on DSP and play to speaker.\r\n"
                     "  USAGE: decoder ["
#if XA_AAC_DECODER
                     "aac|"
#endif
#if XA_MP3_DECODER
                     "mp3|"
#endif
#if XA_OPUS_DECODER
                     "opus|"
#endif
#if XA_SBC_DECODER
                     "sbc|"
#endif
#if XA_VORBIS_DECODER
                     "vorbis_ogg|vorbis_raw"
#endif
                     "]\r\n"
#if XA_AAC_DECODER
                     "   aac:        Decode aac data\r\n"
#endif
#if XA_MP3_DECODER
                     "   mp3:        Decode mp3 data\r\n"
#endif
#if XA_OPUS_DECODER
                     "   opus:       Decode opus data\r\n"
#endif
#if XA_SBC_DECODER
                     "   sbc:        Decode sbc data\r\n"
#endif
#if XA_VORBIS_DECODER
                     "   vorbis_ogg: Decode OGG VORBIS data\r\n"
                     "   vorbis_raw: Decode raw VORBIS data\r\n"
#endif
                     , shellDecoder,
                     1);

#endif
#if (XA_OPUS_ENCODER || XA_SBC_ENCODER)
SHELL_COMMAND_DEFINE(encoder, "\r\n\"encoder\": Encode PCM data on DSP and compare with reference data.\r\n"
                     "  USAGE: encoder ["
#if XA_OPUS_ENCODER
                     "opus|"
#endif
#if XA_SBC_ENCODER
                     "sbc"
#endif
                     "]\r\n"
#if XA_OPUS_ENCODER
                     "   opus: Encode pcm data using opus encoder\r\n"
#endif
#if XA_SBC_ENCODER
                     "   sbc:  Encode pcm data using sbc encoder\r\n"
#endif
                     , shellEncoder,
                     1);
#endif
#if XA_SRC_PP_FX
SHELL_COMMAND_DEFINE(src, "\r\n\"src\" Perform sample rate conversion on DSP\r\n", shellSRC, 0);
#endif
#if XA_PCM_GAIN
SHELL_COMMAND_DEFINE(gain, "\r\n\"gain\": Perform PCM gain adjustment on DSP\r\n", shellGAIN, 0);
#endif

static bool file_playing = false;

SDK_ALIGN(static uint8_t s_shellHandleBuffer[SHELL_HANDLE_SIZE], 4);
static shell_handle_t s_shellHandle;

extern serial_handle_t g_serialHandle;
static handleShellMessageCallback_t *g_handleShellMessageCallback;
static void *g_handleShellMessageCallbackData;

#ifdef MULTICHANNEL
static uint8_t nchannel;
extern int BOARD_CodecChangeSettings(uint8_t nchannel, uint32_t sample_rate, uint32_t bit_width);
#endif
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

    if (argc < 2)
    {
        PRINTF("Incorrect command parameter(s).  Enter \"help\" to view a list of available commands.\r\n");
        return kStatus_SHELL_Error;
    }

    if (!app->sdcardInserted)
    {
        PRINTF("[CM33 CMD] Please insert an SD card with audio files and retry this command\r\n");
        return kStatus_SHELL_Success;
    }

    if (strcmp(argv[1], "list") == 0)
    {
        error = f_opendir(&directory, "/");
        if (error)
        {
            PRINTF("[CM33 CMD] Failed to open root directory of SD card\r\n");
            return kStatus_SHELL_Error;
        }

        PRINTF("[CM33 CMD] Available audio files:\r\n");

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
                    PRINTF("[CM33 CMD] %s\r\n", fileInformation.fname);
                    count++;
                }
#endif

#if XA_AAC_DECODER == 1
                if (dot && strncmp(dot + 1, "aac", 3) == 0)
                {
                    PRINTF("[CM33 CMD] %s\r\n", fileInformation.fname);
                    count++;
                }
#endif

#if XA_VORBIS_DECODER == 1
                if (dot && strncmp(dot + 1, "ogg", 3) == 0)
                {
                    PRINTF("[CM33 CMD] %s\r\n", fileInformation.fname);
                    count++;
                }
#endif
                if (dot && strncmp(dot + 1, "pcm", 3) == 0)
                {
                    PRINTF("[CM33 CMD]  %s\r\n", fileInformation.fname);
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
            PRINTF("[CM33 CMD] (none)\r\n");
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
            PRINTF("[CM33 CMD] File is not playing \r\n");
            return kStatus_SHELL_Error;
        }
    }
    else if (!file_playing)
    {
#ifdef MULTICHANNEL
      if (argc > 2)
        {
            switch (atoi(argv[2]))
            {
                case 8:
                    nchannel = 8;
                    break;
                case 2:
                    /* Intentional fall */
                default:
                    nchannel = 2;
            }
        }
        else
            nchannel = 2;
#endif
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
#ifdef MULTICHANNEL
        if (dot && strncmp(dot + 1, "pcm", 3) == 0)
        {
            msg.param[3] = DSP_COMPONENT_NONE;
            count        = 1;
            /* The PCM file needs to have 96kHz, 32bit width format */
            msg.param[4] = (uint32_t)nchannel;
            msg.param[5] = SAMPLE_RATE_96KHZ;
            msg.param[6] = BIT_WIDTH_32;
            /* The codec can play 96kHz 24bit in 32bit container */
            BOARD_CodecChangeSettings(nchannel, SAMPLE_RATE_96KHZ, BIT_WIDTH_24);
        }
        else
        {
            /* Default settings for all other decoders */
            BOARD_CodecChangeSettings(nchannel, SAMPLE_RATE_48KHZ, BIT_WIDTH_16);
        }
#endif

        if (!count)
        {
            PRINTF("[CM33 CMD] Unsupported file type %s\r\n", filename);
            return kStatus_SHELL_Error;
        }

        error = f_open(&app->fileObject, _T(filename), FA_READ);
        if (error)
        {
            PRINTF("[CM33 CMD] Cannot open file for reading: %s\r\n", filename);
            return kStatus_SHELL_Error;
        }

        error = f_read(&app->fileObject, file_ptr, FILE_PLAYBACK_INITIAL_READ_SIZE, &bytes_read);
        if (error)
        {
            PRINTF("[CM33 CMD] file read fail\r\n");
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
        PRINTF("[CM33 CMD] File is already playing\r\n");
        return kStatus_SHELL_Error;
    }

    return kStatus_SHELL_Success;
}

#if (XA_AAC_DECODER || XA_MP3_DECODER || XA_VORBIS_DECODER || XA_OPUS_DECODER || XA_SBC_DECODER)
static shell_status_t shellDecoder(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg    = {0};
    void * encoded_data = NULL;

    initMessage(&msg);
    /* Param 0 Decoder type */
    /* Param 1 Buffer address with encoded data */
    /* Param 2 Encoded data size */

    msg.head.command = SRTM_Command_Decode;
    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.param[1]      = (uint32_t)AUDIO_SHARED_BUFFER_1;

    if(false == true)
    {
      ;
    }
#if XA_AAC_DECODER
    else if (strcmp(argv[1], "aac") == 0)
    {
        msg.param[0] = SRTM_Decoder_AAC;
        msg.param[2] = sizeof(AACBUFFER);
        encoded_data = (void *)AACBUFFER;
    }
#endif
#if XA_MP3_DECODER
    else if (strcmp(argv[1], "mp3") == 0)
    {
        msg.param[0] = SRTM_Decoder_MP3;
        msg.param[2] = sizeof(MP3BUFFER);
        encoded_data = (void *)MP3BUFFER;
    }
#endif
#if XA_OPUS_DECODER
    else if(strcmp(argv[1], "opus") == 0)
    {
        msg.param[0] = SRTM_Decoder_OPUS;
        msg.param[2] = sizeof(SRTM_OPUS_INPUTBUFFER);
        encoded_data = (void *)SRTM_OPUS_INPUTBUFFER;
    }
#endif
#if XA_SBC_DECODER
    else if(strcmp(argv[1], "sbc") == 0)
    {
        msg.param[0] = SRTM_Decoder_SBC;
        msg.param[2] = sizeof(SBCBUFFER);
        encoded_data = (void *)SBCBUFFER;
    }
#endif
#if XA_VORBIS_DECODER
    else if(strcmp(argv[1], "vorbis_ogg") == 0)
    {
        msg.param[0] = SRTM_Decoder_VORBIS_OGG;
        msg.param[2] = sizeof(OGGVORBISBUFFER);
        encoded_data = (void *)OGGVORBISBUFFER;
    }
    else if(strcmp(argv[1], "vorbis_raw") == 0)
    {
        msg.param[0] = SRTM_Decoder_VORBIS_RAW;
        msg.param[2] = sizeof(RAWVORBISBUFFER);
        encoded_data = (void *)RAWVORBISBUFFER;
    }
#endif
    else
    {
        PRINTF("[CM33 CMD] Unsupported decoder\r\n");
        return kStatus_SHELL_Error;
    }

    if(encoded_data == NULL || msg.param[2] == 0)
    {
        PRINTF("[CM33 CMD] Invalid data for decoding\r\n");
        return kStatus_SHELL_Error;
    }

    if(msg.param[2] > (size_t)(AUDIO_SHARED_BUFFER_1_SIZE + AUDIO_SHARED_BUFFER_2_SIZE) )
    {
        PRINTF("[CM33 CMD] Encoded data is too large (maximum value is %d kB)\r\n", (AUDIO_SHARED_BUFFER_1_SIZE + AUDIO_SHARED_BUFFER_2_SIZE) / 1024);
        return kStatus_SHELL_Error;
    }

    /* Copy encoded audio clip into shared memory buffer */
    memcpy(s_audioInput, encoded_data, msg.param[2]);

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

#if (XA_OPUS_ENCODER || XA_SBC_ENCODER)
static shell_status_t shellEncoder(shell_handle_t shellHandle, int32_t argc, char **argv)
{
    srtm_message msg = {0};
    void * pcm_data = NULL;

    initMessage(&msg);
    /* Param 0 Encoder type */
    /* Param 1 Buffer address with PCM data */
    /* Param 2 PCM data size */
    /* Param 3 Buffer address to store encoded data */
    /* Param 4 Buffer size to store encoded data */
    /* Param 5 Return parameter, actual read size */
    /* Param 6 Return parameter, actual write size */

    msg.head.category = SRTM_MessageCategory_AUDIO;
    msg.head.command  = SRTM_Command_Encode;
    msg.param[1]      = (uint32_t)&s_audioInput[0];
    msg.param[3]      = (uint32_t)&s_audioOutput[0];
    msg.param[4]      = AUDIO_MAX_OUTPUT_BUFFER;

    if(false == true)
    {
      ;
    }
#if XA_OPUS_ENCODER
    else if (strcmp(argv[1], "opus") == 0)
    {
        msg.param[0] = SRTM_Encoder_OPUS;
        msg.param[2] = sizeof(SRTM_OPUS_ENC_INPUTBUFFER);
        pcm_data     = (void *)SRTM_OPUS_ENC_INPUTBUFFER;
    }
#endif
#if XA_SBC_ENCODER
    else if (strcmp(argv[1], "sbc") == 0)
    {
        msg.param[0] = SRTM_Encoder_SBC;
        msg.param[2] = sizeof(SRTM_SBC_ENC_INPUTBUFFER);
        pcm_data     = (void *)SRTM_SBC_ENC_INPUTBUFFER;
    }
#endif
    else
    {
        PRINTF("[CM33 CMD] Unsupported encoder\r\n");
        return kStatus_SHELL_Error;
    }

    if(pcm_data == NULL || msg.param[2] == 0)
    {
        PRINTF("[CM33 CMD] Invalid data for encoding\r\n");
        return kStatus_SHELL_Error;
    }

    if(msg.param[2] > (size_t)AUDIO_SHARED_BUFFER_1_SIZE)
    {
        PRINTF("[CM33 CMD] Input data is too large (maximum value is %d kB)\r\n", AUDIO_SHARED_BUFFER_1_SIZE / 1024);
        return kStatus_SHELL_Error;
    }

    /* Copy pcm audio clip into shared memory buffer */
    memcpy(s_audioInput, pcm_data, msg.param[2]);

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

    if(sizeof(SRTM_MP3_REFBUFFER) > (size_t)AUDIO_SHARED_BUFFER_1_SIZE)
    {
        PRINTF("[CM33 CMD] Input data is too large (maximum value is %d kB)\r\n", AUDIO_SHARED_BUFFER_1_SIZE / 1024);
        return kStatus_SHELL_Error;
    }

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

    if(sizeof(SRTM_MP3_REFBUFFER) > (size_t)AUDIO_SHARED_BUFFER_1_SIZE)
    {
        PRINTF("[CM33 CMD] Input data is too large (maximum value is %d kB)\r\n", AUDIO_SHARED_BUFFER_1_SIZE / 1024);
        return kStatus_SHELL_Error;
    }

    memcpy(s_audioInput, SRTM_MP3_REFBUFFER, sizeof(SRTM_MP3_REFBUFFER));

    g_handleShellMessageCallback(&msg, g_handleShellMessageCallbackData);
    return kStatus_SHELL_Success;
}
#endif

void shellCmd(handleShellMessageCallback_t *handleShellMessageCallback, void *arg)
{
    /* Init SHELL */
    s_shellHandle = &s_shellHandleBuffer[0];
    SHELL_Init(s_shellHandle, g_serialHandle, ">> ");

    /* Add new command to commands list */
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(version));
#if (XA_AAC_DECODER || XA_MP3_DECODER || XA_VORBIS_DECODER || XA_OPUS_DECODER || XA_SBC_DECODER)
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(decoder));
#endif
#if (XA_OPUS_ENCODER || XA_SBC_ENCODER)
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(encoder));
#endif
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(file));
#if XA_SRC_PP_FX
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(src));
#endif
#if XA_PCM_GAIN
    SHELL_RegisterCommand(s_shellHandle, SHELL_COMMAND(gain));
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
                    PRINTF("[CM33 CMD] MP3 Decoder Lib version %d.%d\r\n", msg->param[4] >> 16, msg->param[4] & 0xFF);
                    PRINTF("[CM33 CMD] AAC Decoder Lib version %d.%d\r\n", msg->param[5] >> 16, msg->param[5] & 0xFF);
#if XA_VORBIS_DECODER
                    PRINTF("[CM33 CMD] VORBIS Decoder Lib version %d.%d\r\n", msg->param[6] >> 16,
                           msg->param[6] & 0xFF);
#endif
                    PRINTF("[CM33 CMD] OPUS Codec Lib version %d.%d\r\n", msg->param[7] >> 16, msg->param[7] & 0xFF);
                    PRINTF("[CM33 CMD] SBC Decoder Lib version %d.%d\r\n", msg->param[8] >> 16, msg->param[8] & 0xFF);
                    PRINTF("[CM33 CMD] SBC Encoder Lib version %d.%d\r\n", msg->param[9] >> 16, msg->param[9] & 0xFF);
                    break;

                case SRTM_Command_SYST:
                    break;
                default:
                    PRINTF("[CM33 CMD] Incoming unknown message command %d from category %d \r\n", msg->head.command,
                           msg->head.category);
            }
            break;

        case SRTM_MessageCategory_AUDIO:
            if (file_playing && (msg->head.command < SRTM_Command_FileStart))
            {
                PRINTF(
                    "[CM33 CMD] This command is not possible to process now since a file from SD card is being "
                    "played!\r\n");
                break;
            }

            switch (msg->head.command)
            {
                case SRTM_Command_Decode:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP decoder failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("[CM33 CMD] Decode complete\r\n");

                    break;
#if (XA_OPUS_ENCODER || XA_SBC_ENCODER)
                case SRTM_Command_Encode:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP encoder failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("[CM33 CMD] Encoder read %d bytes and output %d bytes \r\n", msg->param[5], msg->param[6]);
                    PRINTF("[CM33 CMD] Checking encode results...\r\n");
                    {
                        uint8_t *reference_data    = NULL;
                        size_t reference_data_size = 0;
                        if(false == true)
                        {
                            ;
                        }
#if XA_OPUS_ENCODER
                        else if(msg->param[0] == SRTM_Encoder_OPUS)
                        {
                            reference_data = (uint8_t *)SRTM_OPUS_ENC_REFBUFFER;
                            reference_data_size = sizeof(SRTM_OPUS_ENC_REFBUFFER);
                        }
#endif
#if XA_SBC_ENCODER
                        else if (msg->param[0] == SRTM_Encoder_SBC)
                        {
                            reference_data = (uint8_t *)SRTM_SBC_ENC_REFBUFFER;
                            reference_data_size = sizeof(SRTM_SBC_ENC_REFBUFFER);
                        }
#endif
                        else
                        {
                            PRINTF("[CM33 CMD] Unsupported encoder\r\n");
                            return;
                        }

                        if((reference_data == NULL )|| (reference_data_size == 0))
                        {
                            PRINTF("[CM33 CMD] Invalid reference data\r\n");
                            return;
                        }

                        for (int i = 0; (i < msg->param[6]) && (i < reference_data_size); i++)
                        {
                            if (s_audioOutput[i] != reference_data[i])
                            {
                                PRINTF("[CM33 CMD] Output doesn't match @ %d bytes, output = %d ref = %d\r\n", i,
                                    s_audioOutput[i], reference_data[i]);
                                return;
                            }
                        }
                    }

                    PRINTF("[CM33 CMD] Encode output matches reference\r\n");
                    break;
#endif
#if XA_SRC_PP_FX
                case SRTM_Command_SRC:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP Sampling Rate Converter failed, return error = %d\r\n", msg->error);
                    }

                    PRINTF("[CM33 CMD] SRC read %d bytes and output %d bytes \r\n", msg->param[8], msg->param[9]);
#endif
#if XA_PCM_GAIN
                    break;

                case SRTM_Command_GAIN:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP Gain Control Process failed! return error = %d\r\n", msg->error);
                        break;
                    }
                    PRINTF("[CM33 CMD] PCM Gain Control read %d bytes and write %d bytes \r\n", msg->param[8],
                           msg->param[9]);
                    break;
#endif
                case SRTM_Print_String:
                    for (int i = 0; i < SRTM_CMD_PARAMS_MAX; i++)
                    {
                        string_buff[i] = (char)msg->param[i];
                    }
                    PRINTF("[CM33 CMD] %s", string_buff);
                    break;

                case SRTM_Command_FileStart:
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP file playback start failed! return error = %d\r\n", msg->error);
                        file_playing = false;
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP file playback start\r\n");
                        BOARD_MuteRightChannel(msg->param[0] == 1);
                    }

                    /* Release shell to be able to set different commands */
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
                        PRINTF("[CM33 CMD] File read failure: %d\r\n", error);
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

                    PRINTF("[CM33 CMD] DSP file playback complete\r\n");

                    file_playing = false;
                    BOARD_MuteRightChannel(false);
                    error = f_close(&app->fileObject);
                    if (error)
                    {
                        PRINTF("[CM33 CMD] Failed to close file on SD card\r\n");
                    }
                    *notify_shell = true;
                    break;
                }
                case SRTM_Command_FileStop:
                {
                    if (msg->error != SRTM_Status_Success)
                    {
                        PRINTF("[CM33 CMD] DSP file stop failed! return error = %d\r\n", msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP file stopped\r\n");
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
                        PRINTF("[CM33 CMD] DSP requested file stop due to error failed! return error = %d\r\n",
                               msg->error);
                    }
                    else
                    {
                        PRINTF("[CM33 CMD] DSP file stopped, unsupported format.\r\n");
                        file_playing = false;
                        BOARD_MuteRightChannel(false);
                        /* File has stopped playing */
                    }
                    *notify_shell = true;
                    break;
                }
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
