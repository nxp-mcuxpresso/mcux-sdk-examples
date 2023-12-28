/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_codec_common.h"

#include "app_data.h"
#include "app_streamer.h"
#include "streamer_pcm.h"
#include "app_definitions.h"
#include "main.h"
#include "maestro_logging.h"

#ifdef SSRC_PROC
#include "ssrc_proc.h"
#endif

#define APP_STREAMER_MSG_QUEUE     "app_queue"
#define STREAMER_TASK_NAME         "Streamer"
#define STREAMER_MESSAGE_TASK_NAME "StreamerMessage"

#define STREAMER_TASK_STACK_SIZE         20 * 1024
#define STREAMER_MESSAGE_TASK_STACK_SIZE 1024

static STREAMER_T *streamer;
static ringbuf_t *audioBuffer;
OSA_MUTEX_HANDLE_DEFINE(audioMutex);
OSA_TASK_HANDLE_DEFINE(msg_thread);

extern codec_handle_t codecHandle;
extern app_handle_t app;
extern OSA_SEMAPHORE_HANDLE_DEFINE(streamer_semaphore);

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
    bool exit_thread = false;
    osa_status_t ret;

    if (get_app_data()->logEnabled)
    {
        PRINTF("[STREAMER] Message Task started\r\n");
    }

    while (!streamer)
    {
        OSA_TimeDelay(1);
    }
    if (streamer->mq_out == NULL)
    {
        PRINTF("[STREAMER] osa_mq_open failed: %d\r\n");
        return;
    }

    int trackLength = 0;

    progress(0, trackLength);

    do
    {
        ret = OSA_MsgQGet(&streamer->mq_out, (void *)&msg, osaWaitForever_c);
        if (ret != KOSA_StatusSuccess)
        {
            PRINTF("[STREAMER] OSA_MsgQGet error: %d\r\n", ret);
            continue;
        }

        switch (msg.id)
        {
            case STREAM_MSG_DESTROY_PIPELINE:
                progress(0, trackLength);
                break;
            case STREAM_MSG_ERROR:
                PRINTF("[STREAMER] STREAM_MSG_ERROR\r\n");
                exit_thread = true;
                progress(0, 0);
                break;
            case STREAM_MSG_EOS:
                if (get_app_data()->logEnabled)
                {
                    PRINTF("\n[STREAMER] STREAM_MSG_EOS\r\n");
                }
                progress(0, trackLength);
                exit_thread = true;
                break;
            case STREAM_MSG_UPDATE_POSITION:
                if (get_app_data()->logEnabled)
                {
                    PRINTF("[STREAMER] STREAM_MSG_UPDATE_POSITION..");
                    PRINTF(" position: %d ms\r", msg.event_data);
                }
                progress(msg.event_data, trackLength);
                break;
            case STREAM_MSG_UPDATE_DURATION:
                if (get_app_data()->logEnabled)
                {
                    PRINTF("[STREAMER] STREAM_MSG_UPDATE_DURATION: %d\r\n", msg.event_data);
                }
                trackLength = msg.event_data;
                break;
            case STREAM_MSG_CLOSE_TASK:
                if (get_app_data()->logEnabled)
                {
                    PRINTF("[STREAMER] STREAM_MSG_CLOSE_TASK\r\n");
                }
                exit_thread = true;
                break;
            default:
                break;
        }

    } while (!exit_thread);

    OSA_MsgQDestroy(&streamer->mq_out);
    streamer->mq_out = NULL;

    /* propagate stop request to main application state machine */
    OSA_SemaphoreWait(streamer_semaphore, osaWaitForever_c);
    stop();
    OSA_SemaphorePost(streamer_semaphore);
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
    PRINTF("[STREAMER] start playback\r\n");

    handle->audioPlaying = true;
    streamer_set_state(handle->streamer, 0, STATE_PLAYING, true);
}

void STREAMER_Stop(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] stop playback\r\n");

    handle->audioPlaying = false;
    streamer_set_state(handle->streamer, 0, STATE_NULL, true);

    /* Empty input ringbuffer. */
    if (audioBuffer)
    {
        ringbuf_clear(audioBuffer);
    }
}
#ifdef MULTICHANNEL_EXAMPLE
status_t STREAMER_PCM_Create(char *filename, int volume)
{
    STREAMER_CREATE_PARAM params;
    osa_task_def_t thread_attr;
    ELEMENT_PROPERTY_T prop;
    int ret;
    ElementIndex element_ids[]        = {ELEMENT_FILE_SRC_INDEX, ELEMENT_SPEAKER_INDEX};
    int num_elements                  = sizeof(element_ids) / sizeof(ElementIndex);
    PipelineElements pipelineElements = {element_ids, num_elements};

    /* Create message process thread */
    thread_attr.tpriority = OSA_PRIORITY_HIGH;
    thread_attr.tname     = (uint8_t *)STREAMER_MESSAGE_TASK_NAME;
    thread_attr.pthread   = &STREAMER_MessageTask;
    thread_attr.stacksize = STREAMER_MESSAGE_TASK_STACK_SIZE;
    ret                   = OSA_TaskCreate(&msg_thread, &thread_attr, NULL);
    if (KOSA_StatusSuccess != ret)
    {
        return kStatus_Fail;
    }

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size   = STREAMER_TASK_STACK_SIZE;
    params.task_name    = STREAMER_TASK_NAME;
    params.in_dev_name  = "";
    params.out_dev_name = "";
    params.elements     = pipelineElements;

    streamer = streamer_create(&params);
    if (!streamer)
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_FILESRC_SET_LOCATION;
    prop.val  = (uintptr_t)filename;

    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_FILESRC_SET_SAMPLE_RATE;
    prop.val  = DEMO_SAMPLE_RATE;

    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_FILESRC_SET_NUM_CHANNELS;
    prop.val  = get_app_data()->num_channels;

    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_FILESRC_SET_BIT_WIDTH;
    prop.val  = DEMO_BIT_WIDTH;

    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_FILESRC_SET_CHUNK_SIZE;
    prop.val  = get_app_data()->num_channels * DEMO_BYTE_WIDTH * DEMO_SAMPLE_RATE / 100;

    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_SPEAKER_SET_VOLUME;
    prop.val  = volume;
    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_FILESRC_SET_FILE_TYPE;
    prop.val  = AUDIO_DATA;
    streamer_set_property(streamer, 0, prop, true);

    EXT_AUDIOELEMENT_DESC_T appFunctions = {
        .open_func      = streamer_pcm_open,
        .close_func     = streamer_pcm_close,
        .start_func     = NULL,
        .process_func   = streamer_pcm_write,
        .set_param_func = streamer_pcm_setparams,
        .get_param_func = streamer_pcm_getparams,
        .mute_func      = streamer_pcm_mute,
        .volume_func    = streamer_pcm_set_volume,
    };
    prop.prop = PROP_SPEAKER_SET_APP_FUNCTIONS;
    prop.val  = (uintptr_t)&appFunctions;
    streamer_set_property(streamer, 0, prop, true);

    return kStatus_Success;
}
#else
status_t STREAMER_file_Create(char *filename, int volume)
{
    STREAMER_CREATE_PARAM params;
    ELEMENT_PROPERTY_T prop;
    osa_task_def_t thread_attr;
    ElementIndex element_ids[]        = {ELEMENT_FILE_SRC_INDEX, ELEMENT_DECODER_INDEX,
#ifdef SSRC_PROC
                                         ELEMENT_SRC_INDEX,
#endif
                                         ELEMENT_SPEAKER_INDEX};
    int num_elements                  = sizeof(element_ids) / sizeof(ElementIndex);
    PipelineElements pipelineElements = {element_ids, num_elements};

    int ret;

    /* Create message process thread */
    thread_attr.tpriority = OSA_PRIORITY_HIGH;
    thread_attr.tname     = (uint8_t *)STREAMER_MESSAGE_TASK_NAME;
    thread_attr.pthread   = &STREAMER_MessageTask;
    thread_attr.stacksize = STREAMER_MESSAGE_TASK_STACK_SIZE;
    ret                   = OSA_TaskCreate(&msg_thread, &thread_attr, NULL);
    if (KOSA_StatusSuccess != ret)
    {
        return kStatus_Fail;
    }

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size   = STREAMER_TASK_STACK_SIZE;
    params.task_name    = STREAMER_TASK_NAME;
    params.in_dev_name  = "";
    params.out_dev_name = "";
    params.elements     = pipelineElements;

    streamer = streamer_create(&params);
    if (!streamer)
    {
        return kStatus_Fail;
    }

    streamer_set_file(streamer, 0, filename, STATE_NULL, true);

#ifdef SSRC_PROC
    SSRC_register_post_process(streamer);
#endif

    prop.prop = PROP_SPEAKER_SET_VOLUME;
    prop.val  = volume;
    streamer_set_property(streamer, 0, prop, true);

    EXT_AUDIOELEMENT_DESC_T appFunctions = {
        .open_func      = streamer_pcm_open,
        .close_func     = streamer_pcm_close,
        .start_func     = NULL,
        .process_func   = streamer_pcm_write,
        .set_param_func = streamer_pcm_setparams,
        .get_param_func = streamer_pcm_getparams,
        .mute_func      = streamer_pcm_mute,
        .volume_func    = streamer_pcm_set_volume,
    };
    prop.prop = PROP_SPEAKER_SET_APP_FUNCTIONS;
    prop.val  = (uintptr_t)&appFunctions;
    streamer_set_property(streamer, 0, prop, true);

    prop.prop = PROP_SPEAKER_TIME_UPDATE_MS;
    prop.val = 1000;
    streamer_set_property(streamer, 0, prop, true);

    return kStatus_Success;
}
#endif

void STREAMER_Init(void)
{
    return;
}

app_error_code_t play()
{
    init_logging();

    /* Uncomment below to turn on full debug logging for the streamer. */
    //     set_debug_module(0xffffffff);
    //     set_debug_level(LOGLVL_DEBUG);
    //     get_debug_state();
    streamer_pcm_init();
#ifdef MULTICHANNEL_EXAMPLE
    if (STREAMER_PCM_Create((char *)get_app_data()->input, DEMO_VOLUME) == kStatus_Success)
#else
    if (STREAMER_file_Create((char *)get_app_data()->input, (int)get_app_data()->volume) == kStatus_Success)
#endif
    {
        if (streamer_set_state(streamer, 0, STATE_PLAYING, true) == 0)
        {
            PRINTF("[CMD] Starting playback\r\n");
            return kAppCodeOk;
        }
        else
        {
            PRINTF("[CMD] Playback start failed\r\n");
            return kAppCodeError;
        }
    }
    else
    {
        PRINTF("[CMD] create_stream failed\r\n");
    }
    return kAppCodeError;
}

app_error_code_t pause()
{
    if (streamer_set_state(streamer, 0, STATE_PAUSED, true) == 0)
    {
        return kAppCodeOk;
    }
    return kAppCodeError;
}

app_error_code_t resume()
{
    if (streamer_set_state(streamer, 0, STATE_PLAYING, true) == 0)
    {
        return kAppCodeOk;
    }
    return kAppCodeError;
}

app_error_code_t volume()
{
    if (get_app_data()->status == kAppRunning || get_app_data()->status == kAppPaused)
    {
        ELEMENT_PROPERTY_T prop;
        prop.prop = PROP_SPEAKER_SET_VOLUME;
        prop.val  = get_app_data()->volume;
        if (streamer_set_property(streamer, 0, prop, true) == 0)
        {
            PRINTF("[CMD] Volume has been set to %d.\r\n", get_app_data()->volume);
            return kAppCodeOk;
        }
        return kAppCodeError;
    }

    PRINTF("[CMD] Volume has not been set. Play a track first.\r\n");
    return kAppCodeOk;
}

app_error_code_t seek()
{
    StreamData query1;

    if (streamer_query_info(streamer, 0, INFO_DURATION, &query1, true) != 0)
    {
        return kAppCodeError;
    }

    if (query1.value32u > 0U)
    {
        if ((uint32_t)get_app_data()->seek_time > query1.value32u)
        {
            PRINTF(
                "[SEEK STREAMER] No seek was performed because the seek time is longer than the duration of the audio "
                "track.\r\n");
            return kAppCodeOk;
        }

        if (streamer_seek_pipeline(streamer, 0, get_app_data()->seek_time, true) == 0)
        {
            PRINTF("[CMD] The seek audio track to %u milliseconds was performed successfully.\r\n",
                   get_app_data()->seek_time);
            return kAppCodeOk;
        }
    }

    return kAppCodeError;
}

app_error_code_t stop()
{
    if (get_app_data()->status == kAppRunning || get_app_data()->status == kAppPaused)
    {
        if (streamer_set_state(streamer, 0, STATE_NULL, true) == 0)
        {
            /* Empty input ringbuffer. */
            if (audioBuffer)
            {
                ringbuf_clear(audioBuffer);
            }
            destroy();

            get_app_data()->trackCurrent = 0;
            get_app_data()->trackTotal   = 0;
            get_app_data()->status       = kAppIdle;

            PRINTF("[CMD] Playback stopped\r\n");
        }
        else
        {
            return kAppCodeError;
        }
    }

    return kAppCodeOk;
}

void destroy()
{
    if (streamer != NULL)
    {
        streamer_destroy(streamer);
        streamer = 0;
        /* Suspend the tasks for memory cleanup */
        vTaskSuspend(app.shell_task_handle);
        vTaskDelay(100);
        vTaskResume(app.shell_task_handle);

        if (audioBuffer != NULL)
        {
            ringbuf_destroy(audioBuffer);
            audioBuffer = NULL;
        }

        deinit_logging();

        PRINTF("[CMD] destroyed\r\n");
    }
}

void progress(int current, int total)
{
    get_app_data()->trackCurrent = current;
    get_app_data()->trackTotal   = total;
}
