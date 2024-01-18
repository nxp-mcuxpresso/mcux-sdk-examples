/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifdef SSRC_PROC

#include "fsl_debug_console.h"

#include "streamer_api.h"
#include "streamer_element_properties.h"
#include "streamer.h"

#include "app_streamer.h"
#include "ssrc_proc.h"
#include "app_data.h"

LVM_Fs_en SSRC_get_sample_rate_enum(int sample_rate);
int SSRC_get_sample_rate_num(LVM_Fs_en sample_rate);
LVM_Format_en SSRC_get_number_of_channels_enum(int number_of_channels);
int SSRC_get_number_of_channels_num(LVM_Format_en number_of_channels);

SSRC_Instance_t SSRC_Instance;                  /* Allocate memory for the SSRC instance */
SSRC_Params_t SSRC_Params;                      /* Memory for init parameters */

static SSRC_Scratch_t *pScratch    = NULL;      /* Pointer to scratch memory */
static LVM_INT16 *pInputInScratch  = NULL;      /* Pointer to the input in the scratch buffer */
static LVM_INT16 *pOutputInScratch = NULL;      /* Pointer to the output in the scratch buffer */

static int8_t *outBuff      = NULL;             /* Pointer to a buffer for storing the converted audio data */
static uint16_t outBuffSize = 0U;               /* Allocated size for the outBuffer */

static int8_t *resInBuff               = NULL;  /* Pointer to a buffer to hold the rest of the unconverted audio data */
static uint16_t resInBuffSize          = 0U;    /* Allocated size for the resInBuff */
static uint16_t resInBuffDataAvailable = 0U;    /* Number of bytes available in the resInBuff */

static int8_t *resOutBuff               = NULL; /* Pointer to a buffer to hold the rest of the converted audio data */
static uint16_t resOutBuffDataAvailable = 0U;   /* Number of bytes available in the resOutBuff */

static uint8_t OutputByteMultiple = 0U;         /* Ensure output data size is a multiple of 32/64 (due to EDMA). */

LVM_Fs_en SSRC_get_sample_rate_enum(int sample_rate)
{
    switch (sample_rate)
    {
        case 8000:
            return LVM_FS_8000;
        case 11025:
            return LVM_FS_11025;
        case 12000:
            return LVM_FS_12000;
        case 16000:
            return LVM_FS_16000;
        case 22050:
            return LVM_FS_22050;
        case 24000:
            return LVM_FS_24000;
        case 32000:
            return LVM_FS_32000;
        case 44100:
            return LVM_FS_44100;
        case 48000:
            return LVM_FS_48000;
        default:
            break;
    }

    return LVM_FS_INVALID;
}

int SSRC_get_sample_rate_num(LVM_Fs_en sample_rate)
{
    switch (sample_rate)
    {
        case LVM_FS_8000:
            return 8000;
        case LVM_FS_11025:
            return 11025;
        case LVM_FS_12000:
            return 12000;
        case LVM_FS_16000:
            return 16000;
        case LVM_FS_22050:
            return 22050;
        case LVM_FS_24000:
            return 24000;
        case LVM_FS_32000:
            return 32000;
        case LVM_FS_44100:
            return 44100;
        case LVM_FS_48000:
            return 48000;
        default:
            break;
    }

    return -1;
}

LVM_Format_en SSRC_get_number_of_channels_enum(int number_of_channels)
{
    switch (number_of_channels)
    {
        case 1:
            return LVM_MONO;
        case 2:
            return LVM_STEREO;
        case 6:
            return LVM_5DOT1;
        case 8:
            return LVM_7DOT1;
        default:
            break;
    }

    return LVM_SOURCE_DUMMY;
}

int SSRC_get_number_of_channels_num(LVM_Format_en number_of_channels)
{
    switch (number_of_channels)
    {
        case LVM_MONO:
            return 1;
        case LVM_STEREO:
            return 2;
        case LVM_5DOT1:
            return 6;
        case LVM_7DOT1:
            return 8;
        default:
            break;
    }

    return -1;
}

int SSRC_Proc_Init(void *arg)
{
    ext_proc_args *args               = NULL;    /* Variable to store the input arguments */
    int16_t NrSamplesMin              = 0;       /* Minimal number of samples on the input or on the output */
    int32_t ScratchSize               = 0;       /* The size of the scratch memory */
    SSRC_ReturnStatus_en ReturnStatus = SSRC_OK; /* Function return status */

    if (arg == NULL)
    {
        return (int)SSRC_NULL_POINTER;
    }

    /* Init global variables */
    pScratch                = NULL;
    pInputInScratch         = NULL;
    pOutputInScratch        = NULL;
    outBuff                 = NULL;
    outBuffSize             = 0U;
    resInBuff               = NULL;
    resInBuffSize           = 0U;
    resInBuffDataAvailable  = 0U;
    resOutBuff              = NULL;
    resOutBuffDataAvailable = 0U;
    OutputByteMultiple      = 0U;

    /* Set SSRC params */
    args                          = arg;
    SSRC_Params.SSRC_Fs_In        = SSRC_get_sample_rate_enum(args->sample_rate);
    SSRC_Params.SSRC_NrOfChannels = SSRC_get_number_of_channels_enum(args->num_channels);
    SSRC_Params.SSRC_Fs_Out       = LVM_FS_48000;
    SSRC_Params.Quality           = (args->num_channels == 8) ? SSRC_QUALITY_VERY_HIGH : SSRC_QUALITY_HIGH;
    OutputByteMultiple            = (args->num_channels == 8) ? 64 : 32;

    if ((SSRC_Params.SSRC_Fs_In == LVM_FS_INVALID) || (SSRC_Params.SSRC_Fs_Out == LVM_FS_INVALID) ||
        (SSRC_Params.SSRC_NrOfChannels == LVM_SOURCE_DUMMY) || (SSRC_Params.Quality == SSRC_QUALITY_DUMMY))
    {
        return (int)SSRC_INVALID_VALUE;
    }

    ReturnStatus = SSRC_GetNrSamples(SSRC_NR_SAMPLES_MIN, &SSRC_Params);
    if (ReturnStatus != SSRC_OK)
    {
        PRINTF("[SSRC_PROC] Error code %d returned by the SSRC_GetNrSamples function\n", (int)ReturnStatus);
        return (int)ReturnStatus;
    }

    NrSamplesMin = (int16_t)((SSRC_Params.NrSamplesIn > SSRC_Params.NrSamplesOut) ? SSRC_Params.NrSamplesOut :
                                                                                    SSRC_Params.NrSamplesIn);

    /* Don't take blocks smaller that the minimal block size */
    while (NrSamplesMin < MINBLOCKSIZE)
    {
        SSRC_Params.NrSamplesIn  = SSRC_Params.NrSamplesIn << 1;
        SSRC_Params.NrSamplesOut = SSRC_Params.NrSamplesOut << 1;
        NrSamplesMin             = NrSamplesMin << 1;
    }

    ReturnStatus = SSRC_GetScratchSize(&SSRC_Params, (LVM_INT32 *)&ScratchSize);
    if (ReturnStatus != SSRC_OK)
    {
        PRINTF("[SSRC_PROC] Error code %d returned by the SSRC_GetNrSamples function\n", (int)ReturnStatus);
        return (int)ReturnStatus;
    }

    pScratch = (SSRC_Scratch_t *)OSA_MemoryAllocate((uint32_t)ScratchSize);
    if (pScratch == NULL)
    {
        PRINTF("[SSRC_PROC] Error malloc: not enough memory available\n");
        return (int)SSRC_NULL_POINTER;
    }

    ReturnStatus = SSRC_Init(&SSRC_Instance, pScratch, &SSRC_Params, &pInputInScratch, &pOutputInScratch);
    if (ReturnStatus != SSRC_OK)
    {
        PRINTF("[SSRC_PROC] Error code %d returned by the SSRC_Init function\n", (int)ReturnStatus);
        return (int)ReturnStatus;
    }

    /* Allocate resOutBuff */
    resOutBuff = (int8_t *)OSA_MemoryAllocate((uint32_t)OutputByteMultiple);
    if (resOutBuff == NULL)
    {
        PRINTF("[SSRC_PROC] Error malloc: not enough memory available\n");
        return (int)SSRC_NULL_POINTER;
    }

    return (int)ReturnStatus;
}

int SSRC_Proc_Execute(void *arg, void *inputBuffer, int size)
{
    StreamBuffer *buf    = (StreamBuffer *)inputBuffer; /* Buffer with the input audio data and audio packet header */
    int8_t *pkt_hdr_size = arg;                         /* Audio packet header size */
    AudioPacketHeader *data_packet = NULL;              /* Pointer to audio packet header */
    int8_t *data_ptr               = NULL;              /* Pointer to input audio data */
    int8_t num_channel             = 0;                 /* Number of channels */
    int8_t byte_width              = 0;                 /* Byte width */
    uint32_t sample_rate           = 0;
    uint32_t dataInAvailable       = 0;                 /* Number of bytes available for conversion */
    uint32_t dataOutAvailable      = 0;                 /* Number of bytes available in the output audio buffer */
    uint32_t dataResCount = 0; /* Number of bytes converted that exceed integer multiples of OutputByteMultiple in the
                             output audio buffer */
    int16_t NrInBytes                 = 0;       /* The number of bytes in each input block for SSRC */
    int16_t NrOutBytes                = 0;       /* The number of bytes in each output block for SSRC */
    int8_t *pInputData                = NULL;    /* Pointer to input block of audio data to convert */
    int8_t *pOutputData               = NULL;    /* Pointer to store the converted block of audio data */
    uint16_t outBuffSizeLocal         = 0U;      /* Required size for the outBuffer */
    SSRC_ReturnStatus_en ReturnStatus = SSRC_OK; /* Function return status */

    data_packet = (AudioPacketHeader *)buf->buffer;
    data_ptr    = buf->buffer + *pkt_hdr_size;
    num_channel = data_packet->num_channels;
    byte_width  = data_packet->bits_per_sample >> 3;
    sample_rate = data_packet->sample_rate;

    if ((num_channel != SSRC_get_number_of_channels_num(SSRC_Params.SSRC_NrOfChannels)) ||
        (sample_rate != SSRC_get_sample_rate_num(SSRC_Params.SSRC_Fs_In)))
    {
        ext_proc_args args = {.num_channels = num_channel, .sample_rate = sample_rate};
        (void)SSRC_Proc_Deinit();
        if (SSRC_Proc_Init(&args) != 0)
        {
            return -1;
        }
    }

    dataInAvailable = (uint32_t)size + (uint32_t)resInBuffDataAvailable;

    NrInBytes  = (SSRC_Params.NrSamplesIn * num_channel) * byte_width;
    NrOutBytes = (SSRC_Params.NrSamplesOut * num_channel) * byte_width;

    /* Allocate outBuf */
    outBuffSizeLocal = ((uint16_t)(size + (int)NrInBytes) * SSRC_Params.NrSamplesOut) / SSRC_Params.NrSamplesIn +
                       *pkt_hdr_size + OutputByteMultiple;
    if (outBuffSizeLocal > outBuffSize)
    {
        if (outBuff != NULL)
        {
            OSA_MemoryFree(outBuff);
            outBuff = NULL;
        }

        outBuffSize = outBuffSizeLocal;
        outBuff     = (int8_t *)OSA_MemoryAllocate((uint32_t)outBuffSize);
        if (outBuff == NULL)
        {
            return (int)SSRC_NULL_POINTER;
        }
    }

    pOutputData = outBuff + *pkt_hdr_size;
    pInputData  = data_ptr;

    /* Copy the converted bytes from the previous cycle to the output buffer */
    if (resOutBuffDataAvailable > 0U)
    {
        memcpy(pOutputData, resOutBuff, resOutBuffDataAvailable);
        dataOutAvailable += (uint32_t)resOutBuffDataAvailable;
        pOutputData += (int8_t)resOutBuffDataAvailable;
        resOutBuffDataAvailable = 0U;
    }

    /* Process input audio data */
    while (dataInAvailable >= (uint32_t)NrInBytes)
    {
        /* Prepare input block of audio data for conversion */
        if (resInBuffDataAvailable > 0)
        {
            /* If unconverted audio data from the previous cycle is available, use it first */
            LVM_INT16 diff = NrInBytes - resInBuffDataAvailable;
            int8_t *data   = (int8_t *)pInputInScratch;
            memcpy(data, resInBuff, resInBuffDataAvailable);
            data += resInBuffDataAvailable;
            resInBuffDataAvailable = 0U;

            /* Fill the remaining space of the input block with the audio data stored in the input buffer */
            if (diff > 0)
            {
                memcpy(data, pInputData, diff);
                pInputData += diff;
            }
        }
        else
        {
            /* If there is no unconverted audio data from the previous cycle, fill the input block with only the audio
             * data stored in the input buffer */
            memcpy(pInputInScratch, pInputData, NrInBytes);
            pInputData += NrInBytes;
        }

        /* Convert the input block of audio data */
        if (OutputByteMultiple == 32)
        {
            ReturnStatus = SSRC_Process(&SSRC_Instance, pInputInScratch, pOutputInScratch);
        }
        else
        {
            ReturnStatus =
                SSRC_Process_D32(&SSRC_Instance, (LVM_INT32 *)pInputInScratch, (LVM_INT32 *)pOutputInScratch);
        }

        if (ReturnStatus != SSRC_OK)
        {
            PRINTF("[SSRC_PROC] Error code %d returned by the SSRC_Process function\n", (LVM_INT16)ReturnStatus);
            return (int)ReturnStatus;
        }

        /* Copy the converted audio data to the output buffer */
        memcpy(pOutputData, pOutputInScratch, NrOutBytes);

        /* Update pointers and sizes */
        dataInAvailable -= (uint32_t)NrInBytes;
        dataOutAvailable += (uint32_t)NrOutBytes;
        pOutputData += NrOutBytes;
    }

    /* Save the remaining unconverted audio data for the next processing cycle */
    if (dataInAvailable > 0)
    {
        if (NrInBytes > resInBuffSize)
        {
            if (resInBuff != NULL)
            {
                OSA_MemoryFree(resInBuff);
                resInBuff = NULL;
            }

            resInBuffSize = (uint16_t)NrInBytes;
            resInBuff     = (int8_t *)OSA_MemoryAllocate(resInBuffSize);
            if (resInBuff == NULL)
            {
                return (int)SSRC_NULL_POINTER;
            }
        }

        memcpy(resInBuff, pInputData, dataInAvailable);
        resInBuffDataAvailable = (uint16_t)dataInAvailable;
    }

    /* Save the excess converted audio data to be sent in the next processing cycle (due to compliance with an integer
     * multiple of OutputByteMultiple in the output buffer) */
    dataResCount = (dataOutAvailable % (uint32_t)OutputByteMultiple);
    if (dataResCount > 0)
    {
        memcpy(resOutBuff, pOutputData - dataResCount, dataResCount);
        resOutBuffDataAvailable = (uint16_t)dataResCount;
        dataOutAvailable -= dataResCount;
    }

    /* Set the current valid values for the audio packet header */
    data_packet->chunk_size  = dataOutAvailable;
    data_packet->sample_rate = SSRC_get_sample_rate_num(SSRC_Params.SSRC_Fs_Out);

    /* Copy valid audio packet header to the outBuff */
    memcpy(outBuff, data_packet, *pkt_hdr_size);

    /* Update buf variable */
    buf->buffer = outBuff;
    buf->size   = dataOutAvailable + (uint32_t)*pkt_hdr_size;

    return (int)SSRC_OK;
}

int SSRC_Proc_Deinit(void)
{
    if (pScratch != NULL)
    {
        OSA_MemoryFree(pScratch);
        pScratch = NULL;
    }

    if (outBuff != NULL)
    {
        OSA_MemoryFree(outBuff);
        outBuff     = NULL;
        outBuffSize = 0U;
    }

    if (resInBuff != NULL)
    {
        OSA_MemoryFree(resInBuff);
        resInBuff              = NULL;
        resInBuffSize          = 0U;
        resInBuffDataAvailable = 0U;
    }

    if (resOutBuff != NULL)
    {
        OSA_MemoryFree(resOutBuff);
        resOutBuff              = NULL;
        resOutBuffDataAvailable = 0U;
    }

    return (int)SSRC_OK;
}

int SSRC_register_post_process(void *streamer)
{
    ELEMENT_PROPERTY_T prop;

    PRINTF("[SSRC_PROC] registering post process SSRC\r\n");

    EXT_PROCESS_DESC_T ssrc_proc = {SSRC_Proc_Init, SSRC_Proc_Execute, SSRC_Proc_Deinit, &get_app_data()->proc_args};

    prop.prop = PROP_SRC_FUNCPTR;
    prop.val  = (uintptr_t)&ssrc_proc;

    if (streamer_set_property(streamer, 0, prop, true) != 0)
    {
        PRINTF("[SSRC_PROC] SSRC post processor registration failed\r\n");
        return -1;
    }

    return 0;
}

#endif /* SSRC_PROC */
