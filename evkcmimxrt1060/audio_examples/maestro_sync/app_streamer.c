/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"

#include "app_streamer.h"
#include "streamer_pcm.h"
#include "maestro_logging.h"
#include "app_definitions.h"
#include "test_audio.h"
#include "vit_proc.h"
#include "voice_seeker.h"

#define APP_STREAMER_MSG_QUEUE     "app_queue"
#define STREAMER_TASK_NAME         "Streamer"
#define STREAMER_MESSAGE_TASK_NAME "StreamerMessage"

#define STREAMER_TASK_STACK_SIZE         20 * 1024
#define STREAMER_MESSAGE_TASK_STACK_SIZE 1024
#define MAX_FILE_NAME_LENGTH             100

#if defined(PLATFORM_RT1060)
#define DEMO_NUM_CHANNELS (1)
#define DEMO_BYTE_WIDTH   (2)
#elif defined(PLATFORM_RT1170_EVKB)
#define DEMO_NUM_CHANNELS (2)
#define DEMO_BYTE_WIDTH   (4)
#endif

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
                /* Print information about current position - disabled by default */
                // PRINTF("STREAM_MSG_UPDATE_POSITION..");
                // PRINTF(" position: %d ms\r", msg.event_data);
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
    return true;
}

void STREAMER_Start(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] start \r\n");

    for (size_t i = 0; i < MAX_PIPELINES; i++)
    {
        if (handle->streamer->pipes[i] != (uintptr_t)NULL)
        {
            if (streamer_set_state(handle->streamer, i, STATE_PLAYING, false) != 0)
            {
                PRINTF("[STREAMER] Playback start failed\r\n");
            }
        }
    }
}

void STREAMER_Stop(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] stop \r\n");

    for (size_t i = 0; i < MAX_PIPELINES; i++)
    {
        if (handle->streamer->pipes[i] != (uintptr_t)NULL)
        {
            streamer_set_state(handle->streamer, i, STATE_NULL, true);
        }
    }

    /* Empty input ringbuffer. */
    if (audioBuffer)
    {
        ringbuf_clear(audioBuffer);
    }
}

status_t STREAMER_Create(streamer_handle_t *handle, bool noSDCard)
{
    STREAMER_CREATE_PARAM params;
    osa_task_def_t thread_attr;
    ELEMENT_PROPERTY_T prop;
    int num_elements                  = 2;
    ElementIndex element_ids[4]       = {ELEMENT_MEM_SRC_INDEX, ELEMENT_SPEAKER_INDEX};
    PipelineElements pipelineElements = {element_ids, num_elements};

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size   = STREAMER_TASK_STACK_SIZE;
    params.task_name    = STREAMER_TASK_NAME;
    params.in_dev_name  = "memory";
    params.out_dev_name = "speaker";
    params.elements     = pipelineElements;

    handle->streamer = streamer_create(&params);
    if (handle->streamer == NULL)
    {
        return kStatus_Fail;
    }

    /* Add another pipeline */
    params.out_dev_name = "SD card";
    params.in_dev_name  = "microphone";
    element_ids[0]      = ELEMENT_MICROPHONE_INDEX;
    if (noSDCard == true)
    {
#ifdef VOICE_SEEKER_PROC
        element_ids[1] = ELEMENT_VOICESEEKER_INDEX;
#endif
        element_ids[2]      = ELEMENT_VIT_INDEX;
        num_elements        = 3;
        params.out_dev_name = "VIT";
    }
    else
    {
#ifdef VOICE_SEEKER_PROC
        element_ids[1] = ELEMENT_VOICESEEKER_INDEX;
#endif
        element_ids[2] = ELEMENT_VIT_PROC_INDEX;
        element_ids[3] = ELEMENT_FILE_SINK_INDEX;
        num_elements   = 4;
    }
    pipelineElements.element_ids  = element_ids;
    pipelineElements.num_elements = num_elements;

    if (streamer_create_pipeline(handle->streamer, 1, params.in_dev_name, params.out_dev_name, pipelineElements, false))
    {
        return kStatus_Fail;
    }

    /* Create message process thread */
    thread_attr.tpriority = OSA_PRIORITY_HIGH;
    thread_attr.tname     = (uint8_t *)STREAMER_MESSAGE_TASK_NAME;
    thread_attr.pthread   = &STREAMER_MessageTask;
    thread_attr.stacksize = STREAMER_MESSAGE_TASK_STACK_SIZE;
    if (KOSA_StatusSuccess != OSA_TaskCreate(&msg_thread, &thread_attr, (void *)handle))
    {
        return kStatus_Fail;
    }

    /* PCM_AUDIO_MEM pipeline */
    MEMSRC_SET_BUFFER_T buf;
    buf.location = (int8_t *)TESTAUDIO_DATA;
    buf.size     = TESTAUDIO_LEN;

    prop.prop = PROP_MEMSRC_SET_BUFF;
    prop.val  = (uintptr_t)&buf;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_MEM_TYPE;
    prop.val  = AUDIO_DATA;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_SAMPLE_RATE;
    prop.val  = DEMO_SAMPLE_RATE;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_NUM_CHANNELS;
    prop.val  = DEMO_NUM_CHANNELS;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_BIT_WIDTH;
    prop.val  = DEMO_BYTE_WIDTH * 8;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MEMSRC_SET_CHUNK_SIZE;
#ifdef VOICE_SEEKER_PROC
    // (num_channels * DEMO_BYTE_WIDTH * DEMO_SAMPLE_RATE / 100) * frame_ms/10;
    prop.val = (DEMO_NUM_CHANNELS * DEMO_BYTE_WIDTH * 160) * 3;
#else
    prop.val = (DEMO_NUM_CHANNELS * DEMO_BYTE_WIDTH * 160);
#endif
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    // Set volume
    prop.prop = PROP_SPEAKER_SET_VOLUME;
    prop.val  = 75;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    EXT_AUDIOELEMENT_DESC_T appFunctions = {0};
    appFunctions.open_func               = streamer_pcm_tx_open;
    appFunctions.close_func              = streamer_pcm_tx_close;
    appFunctions.start_func              = NULL;
    appFunctions.process_func            = streamer_pcm_write;
    appFunctions.set_param_func          = streamer_pcm_setparams;
    appFunctions.get_param_func          = streamer_pcm_getparams;
    appFunctions.mute_func               = streamer_pcm_mute;
    appFunctions.volume_func             = streamer_pcm_set_volume;

    prop.prop = PROP_SPEAKER_SET_APP_FUNCTIONS;
    prop.val  = (uintptr_t)&appFunctions;
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    if (STREAM_OK != streamer_set_repeat(handle->streamer, 0, true, true))
    {
        return kStatus_Fail;
    }

    /* VIT pipeline */
    appFunctions.open_func      = streamer_pcm_rx_open;
    appFunctions.close_func     = streamer_pcm_rx_close;
    appFunctions.start_func     = NULL;
    appFunctions.process_func   = streamer_pcm_read;
    appFunctions.set_param_func = streamer_pcm_setparams;
    appFunctions.get_param_func = streamer_pcm_getparams;
    appFunctions.mute_func      = streamer_pcm_mute;
    appFunctions.volume_func    = streamer_pcm_set_volume;

    prop.prop = PROP_MICROPHONE_SET_APP_FUNCTIONS;
    prop.val  = (uintptr_t)&appFunctions;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MICROPHONE_SET_SAMPLE_RATE;
    prop.val  = DEMO_SAMPLE_RATE;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

#if defined(PLATFORM_RT1170_EVKB)
    prop.prop = PROP_MICROPHONE_SET_NUM_CHANNELS;
    prop.val  = 2;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }
    prop.prop = PROP_MICROPHONE_SET_BITS_PER_SAMPLE;
    prop.val  = 32;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }
#endif

    /* Clear the dummy tx for microphone */
    prop.prop = PROP_MICROPHONE_SET_DUMMY_TX_ENABLE;
    prop.val  = false;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

#ifdef VOICE_SEEKER_PROC
    prop.prop = PROP_SPEAKER_SET_REFDATA_ELEMENT;
    prop.val  = (uintptr_t)handle->streamer->elems[1][ELEMENT_VOICESEEKER_INDEX];
    if (STREAM_OK != streamer_set_property(handle->streamer, 0, prop, true))
    {
        return kStatus_Fail;
    }

    prop.prop = PROP_MICROPHONE_SET_FRAME_MS;
    prop.val  = 30;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

    {
        EXT_PROCESS_DESC_T voice_seeker_proc = {VoiceSeeker_Initialize_func, VoiceSeeker_Execute_func,
                                                VoiceSeeker_Deinit_func, NULL};

        prop.prop = PROP_VOICESEEKER_FUNCPTR;
        prop.val  = (uintptr_t)&voice_seeker_proc;
        if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
        {
            return kStatus_Fail;
        }
    }
    {
        EXT_PROCESS_REFDAT_DESC_T voice_seeker_proc = {VoiceSeeker_RefData_Set_NumBuff_func,
                                                       VoiceSeeker_RefData_Push_func, VoiceSeeker_Set_Debugging_func};

        prop.prop = PROP_VOICESEEKER_REFDATA_FUNCPTR;
        prop.val  = (uintptr_t)&voice_seeker_proc;
        if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
        {
            return kStatus_Fail;
        }

        prop.prop = PROP_VOICESEEKER_SET_DEBUGGING;
        prop.val  = (uintptr_t) false;
        if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
        {
            return kStatus_Fail;
        }
    }
#endif // VOICE_SEEKER_PROC

    EXT_PROCESS_DESC_T vit_proc = {VIT_Initialize_func, VIT_Execute_func, VIT_Deinit_func, NULL};

    if (num_elements == 3 && pipelineElements.element_ids[2] == ELEMENT_VIT_INDEX)
    {
        prop.prop = PROP_VITSINK_FUNCPTR;
    }
    else
    {
        prop.prop = PROP_VIT_PROC_FUNCPTR;
    }
    prop.val = (uintptr_t)&vit_proc;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

    if (num_elements == 4 && pipelineElements.element_ids[3] == ELEMENT_FILE_SINK_INDEX)
    {
        prop.prop = PROP_FILESINK_LOCATION;
        prop.val  = (uintptr_t) "voiceseeker_output.pcm";
        if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
        {
            return kStatus_Fail;
        }
    }

    return kStatus_Success;
}

status_t STREAMER_SetDebug(streamer_handle_t *handle, bool set_debug)
{
    ELEMENT_PROPERTY_T prop;

    prop.prop = PROP_VOICESEEKER_SET_DEBUGGING;
    prop.val  = (uintptr_t)set_debug;
    if (STREAM_OK != streamer_set_property(handle->streamer, 1, prop, true))
    {
        return kStatus_Fail;
    }

    PRINTF("Debugging has been turned %s.\r\n", (set_debug == true) ? "on" : "off");

    return kStatus_Success;
}

void STREAMER_Destroy(streamer_handle_t *handle)
{
    if (handle->streamer != NULL)
    {
        streamer_destroy(handle->streamer);
        handle->streamer = NULL;
    }

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
