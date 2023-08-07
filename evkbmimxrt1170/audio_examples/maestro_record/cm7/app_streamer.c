/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"

#include "app_streamer.h"
#include "streamer_pcm_app.h"
#include "maestro_logging.h"
#ifdef VIT_PROC
#include "vit_proc.h"
#endif
#ifdef VOICE_SEEKER_PROC
#include "voice_seeker.h"
#endif
#include "app_definitions.h"

#define APP_STREAMER_MSG_QUEUE     "app_queue"
#define STREAMER_TASK_NAME         "Streamer"
#define STREAMER_MESSAGE_TASK_NAME "StreamerMessage"

#ifdef OPUS_ENCODE
/* The STREAMER_OPUS_TASK_STACK_SIZE value is different for the IAR because the IAR allocates memory for VLA (variable
 * length array - used in the opus encoder) in the application heap and the opus task stack would be largely unused.
 */
#if defined(__ICCARM__)
#define STREAMER_OPUS_TASK_STACK_SIZE 5 * 1024
#else
#define STREAMER_OPUS_TASK_STACK_SIZE 30 * 1024
#endif
#endif

#define STREAMER_TASK_STACK_SIZE         16 * 1024
#define STREAMER_MESSAGE_TASK_STACK_SIZE 1024
#define MAX_FILE_NAME_LENGTH             100

ringbuf_t *audioBuffer;
OSA_MUTEX_HANDLE_DEFINE(audioMutex);
OSA_TASK_HANDLE_DEFINE(msg_thread);

/*!
 * @brief Streamer task for communicating messages
 *
 * This function is the entry point of a task that is manually created by
 * STREAMER_Create.  It listens on a message queue and receives status updates
 * about errors, audio playback state and position.  The application can make
 * use of this data.
 *
 * @param arg Data to be passed to the task
 */
static void STREAMER_MessageTask(void *arg)
{
    STREAMER_MSG_T msg;
    streamer_handle_t *handle;
    bool exit_thread = false;
    osa_status_t ret;

    handle = (streamer_handle_t *)arg;

    while (!handle->streamer->mq_out)
    {
        OSA_TimeDelay(1);
    }
    if (handle->streamer->mq_out == NULL)
    {
        PRINTF("[STREAMER] osa_mq_open failed: %d\r\n");
        return;
    }

    PRINTF("[STREAMER] Message Task started\r\n");

    do
    {
        ret = OSA_MsgQGet(&handle->streamer->mq_out, (void *)&msg, osaWaitForever_c);
        if (ret != KOSA_StatusSuccess)
        {
            PRINTF("OSA_MsgQGet error: %d\r\n", ret);
            continue;
        }

        switch (msg.id)
        {
            case STREAM_MSG_ERROR:
                PRINTF("STREAM_MSG_ERROR\r\n");
                exit_thread = true;
                STREAMER_Stop(handle);
                break;
            case STREAM_MSG_EOS:
                PRINTF("\nSTREAM_MSG_EOS\r\n");
                exit_thread = true;
                /* Indicate to other software layers that playing has ended. */
                STREAMER_Stop(handle);
                break;
            case STREAM_MSG_UPDATE_POSITION:
                PRINTF("STREAM_MSG_UPDATE_POSITION..");
                PRINTF(" position: %d ms\r", msg.event_data);
                break;
            case STREAM_MSG_CLOSE_TASK:
                PRINTF("STREAM_MSG_CLOSE_TASK\r\n");
                exit_thread = true;
                break;
            default:
                break;
        }

    } while (!exit_thread);

    OSA_MsgQDestroy(&handle->streamer->mq_out);
    handle->streamer->mq_out = NULL;

    OSA_TaskDestroy(msg_thread);
}

int STREAMER_Read(uint8_t *data, uint32_t size)
{
    uint32_t bytes_read;

    OSA_MutexLock(&audioMutex, osaWaitForever_c);
    bytes_read = ringbuf_read(audioBuffer, data, size);
    OSA_MutexUnlock(&audioMutex);

    if (bytes_read != size)
    {
        PRINTF("[STREAMER WARN] read underrun: size: %d, read: %d\r\n", size, bytes_read);
    }

    return bytes_read;
}

int STREAMER_Write(uint8_t *data, uint32_t size)
{
    uint32_t written;

    OSA_MutexLock(&audioMutex, osaWaitForever_c);
    written = ringbuf_write(audioBuffer, data, size);
    OSA_MutexUnlock(&audioMutex);

    if (written != size)
    {
        PRINTF("[STREAMER ERR] write overflow: size %d, written %d\r\n", size, written);
    }

    return written;
}

bool STREAMER_IsPlaying(streamer_handle_t *handle)
{
    return handle->audioPlaying;
}

void STREAMER_Start(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] start \r\n");

    handle->audioPlaying = true;
    streamer_set_state(handle->streamer, 0, STATE_PLAYING, true);
}

void STREAMER_Stop(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] stop \r\n");

    handle->audioPlaying = false;
    streamer_set_state(handle->streamer, 0, STATE_NULL, true);

    /* Empty input ringbuffer. */
    if (audioBuffer)
    {
        ringbuf_clear(audioBuffer);
    }
}

status_t STREAMER_mic_Create(streamer_handle_t *handle, out_sink_t out_sink, char *file_name)
{
    STREAMER_CREATE_PARAM params;
    osa_task_def_t thread_attr;
    ELEMENT_PROPERTY_T prop;
    int ret;

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size = STREAMER_TASK_STACK_SIZE;

    switch (out_sink)
    {
        case AUDIO_SINK:
            params.pipeline_type = STREAM_PIPELINE_PCM;
            params.out_dev_name  = "";
            break;

        case FILE_SINK:
            params.pipeline_type = STREAM_PIPELINE_MIC2FILE;
            params.out_dev_name  = "file";
            break;

        case VIT_SINK:
            params.pipeline_type = STREAM_PIPELINE_VIT;
            params.out_dev_name  = "";
            break;

        default:
            PRINTF("[STREAMER ERR] wrong type of sink\r\n");
            return kStatus_InvalidArgument;
    }

    params.task_name   = STREAMER_TASK_NAME;
    params.in_dev_name = "microphone";

    handle->streamer = streamer_create(&params);
    if (!handle->streamer)
    {
        return kStatus_Fail;
    }

    /* Create message process thread */
    thread_attr.tpriority = OSA_PRIORITY_HIGH;
    thread_attr.tname     = (uint8_t *)STREAMER_MESSAGE_TASK_NAME;
    thread_attr.pthread   = &STREAMER_MessageTask;
    thread_attr.stacksize = STREAMER_MESSAGE_TASK_STACK_SIZE;
    ret                   = OSA_TaskCreate(&msg_thread, &thread_attr, (void *)handle);
    if (KOSA_StatusSuccess != ret)
    {
        return kStatus_Fail;
    }

#ifdef VIT_PROC
    if (params.pipeline_type == STREAM_PIPELINE_VIT)
    {
#ifdef VOICE_SEEKER_PROC
        EXT_PROCESS_DESC_T voice_seeker_proc = {VoiceSeeker_Initialize_func, VoiceSeeker_Execute_func,
                                                VoiceSeeker_Deinit_func, NULL};

        prop.prop = PROP_VOICESEEKER_PROC_FUNCPTR;
        prop.val  = (uintptr_t)&voice_seeker_proc;
        streamer_set_property(handle->streamer, 0, prop, true);

#endif // VOICE_SEEKER_PROC
        prop.prop = PROP_AUDIOSRC_SET_FRAME_MS;
        prop.val  = 30;
        streamer_set_property(handle->streamer, 0, prop, true);

        EXT_PROCESS_DESC_T vit_proc = {VIT_Initialize_func, VIT_Execute_func, VIT_Deinit_func, NULL};

        prop.prop = PROP_VITSINK_PROC_FUNCPTR;
        prop.val  = (uintptr_t)&vit_proc;
        streamer_set_property(handle->streamer, 0, prop, true);
    } // STREAM_PIPELINE_VIT
#else
    if (params.pipeline_type == STREAM_PIPELINE_VIT)
    {
        PRINTF("[STREAMER] VIT pipeline not available for this config\r\n switching to audio sink");
        params.pipeline_type = STREAM_PIPELINE_PCM;
    }
#endif // VIT_PROC

#if (defined(PLATFORM_RT1170) || defined(PLATFORM_RT1160) || DEMO_CODEC_CS42448)
#ifndef VOICE_SEEKER_PROC
    if (params.pipeline_type == STREAM_PIPELINE_VIT)
    {
        PRINTF(
            "[STREAMER] Please enable VoiceSeeker, it must be used if more than one channel is used and VIT is "
            "enabled.\r\n");
        return kStatus_Fail;
    }
#endif

#if (defined(PLATFORM_RT1170) || defined(PLATFORM_RT1160))
    prop.prop = PROP_AUDIOSRC_SET_FRAME_MS;
    prop.val  = 30;
    streamer_set_property(handle->streamer, 0, prop, true);

    prop.prop = PROP_AUDIOSRC_SET_NUM_CHANNELS;
    prop.val  = 2;
    streamer_set_property(handle->streamer, 0, prop, true);
#endif

#if DEMO_CODEC_CS42448
    prop.prop = PROP_AUDIOSRC_SET_NUM_CHANNELS;
    prop.val  = 8;
    streamer_set_property(handle->streamer, 0, prop, true);
#endif

#if (defined(PLATFORM_RT1170) || defined(PLATFORM_RT1160) || defined(MCXN548_cm33_core0_SERIES) || DEMO_CODEC_CS42448)
    prop.prop = PROP_AUDIOSRC_SET_BITS_PER_SAMPLE;
    prop.val  = 32;
    streamer_set_property(handle->streamer, 0, prop, true);
#endif
#endif //(defined(PLATFORM_RT1170) || defined(PLATFORM_RT1160) || DEMO_CODEC_CS42448)
    prop.prop = PROP_AUDIOSRC_SET_SAMPLE_RATE;
    prop.val  = 16000;
    streamer_set_property(handle->streamer, 0, prop, true);

    if (out_sink == FILE_SINK)
    {
        char file_name_val[MAX_FILE_NAME_LENGTH];
        memcpy(file_name_val, file_name == NULL ? "tmp" : file_name, MAX_FILE_NAME_LENGTH);
        strcat(file_name_val, ".pcm");

        prop.prop = PROP_FILESINK_LOCATION;
        prop.val  = (uintptr_t)file_name_val;
        streamer_set_property(handle->streamer, 0, prop, true);
    }

    return kStatus_Success;
}

#ifdef OPUS_ENCODE
status_t STREAMER_opusmem2mem_Create(streamer_handle_t *handle,
                                     CeiBitstreamInfo *info,
                                     MEMSRC_SET_BUFFER_T *inBuf,
                                     SET_BUFFER_DESC_T *outBuf)
{
    STREAMER_CREATE_PARAM params;
    osa_task_def_t thread_attr;
    ELEMENT_PROPERTY_T prop;
    int ret;

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size    = STREAMER_OPUS_TASK_STACK_SIZE;
    params.pipeline_type = STREAM_PIPELINE_OPUS_MEM2MEM;
    params.task_name     = STREAMER_TASK_NAME;
    params.in_dev_name   = "";
    params.out_dev_name  = "";

    handle->streamer = streamer_create(&params);
    if (!handle->streamer)
    {
        return kStatus_Fail;
    }

    /* Create message process thread */
    thread_attr.tpriority = OSA_PRIORITY_HIGH;
    thread_attr.tname     = (uint8_t *)STREAMER_MESSAGE_TASK_NAME;
    thread_attr.pthread   = &STREAMER_MessageTask;
    thread_attr.stacksize = STREAMER_MESSAGE_TASK_STACK_SIZE;
    ret                   = OSA_TaskCreate(&msg_thread, &thread_attr, (void *)handle);
    if (KOSA_StatusSuccess != ret)
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_BUFF;
    prop.val  = (uintptr_t)inBuf;
    ret       = streamer_set_property(handle->streamer, 0, prop, true);
    if (ret != STREAM_OK)
    {
        streamer_destroy(handle->streamer);
        handle->streamer = NULL;

        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSINK_BUFFER_DESC;
    prop.val  = (uintptr_t)outBuf;
    ret       = streamer_set_property(handle->streamer, 0, prop, true);
    if (ret != STREAM_OK)
    {
        streamer_destroy(handle->streamer);
        handle->streamer = NULL;

        return kStatus_Fail;
    }

    prop.prop = PROP_ENCODER_TYPE;
    prop.val  = (uintptr_t)CEIENC_OPUS;
    ret       = streamer_set_property(handle->streamer, 0, prop, true);
    if (ret != STREAM_OK)
    {
        streamer_destroy(handle->streamer);
        handle->streamer = NULL;

        return kStatus_Fail;
    }

    prop.prop = PROP_ENCODER_BITSTREAMINFO;
    prop.val  = (uintptr_t)info;
    ret       = streamer_set_property(handle->streamer, 0, prop, true);
    if (ret != STREAM_OK)
    {
        streamer_destroy(handle->streamer);
        handle->streamer = NULL;

        return kStatus_Fail;
    }

    return kStatus_Success;
}
#endif

void STREAMER_Destroy(streamer_handle_t *handle)
{
    streamer_destroy(handle->streamer);
    handle->streamer = NULL;

    if (audioBuffer != NULL)
    {
        ringbuf_destroy(audioBuffer);
        audioBuffer = NULL;
    }
    deinit_logging();
}

void STREAMER_Init(void)
{
    /* Initialize logging */
    init_logging();

    /* Uncomment below to turn on full debug logging for the streamer. */
    // set_debug_module(0xffffffff);
    // set_debug_level(LOGLVL_DEBUG);
    // get_debug_state();

    /* Initialize streamer PCM management library. */
    streamer_pcm_init();
}
