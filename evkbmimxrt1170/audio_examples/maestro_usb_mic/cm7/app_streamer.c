/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "fsl_debug_console.h"

#include "app_streamer.h"
#include "streamer_pcm_app.h"
#include "maestro_logging.h"
#include "audio_microphone.h"

#define APP_STREAMER_MSG_QUEUE     "app_queue"
#define STREAMER_TASK_NAME         "Streamer"
#define STREAMER_MESSAGE_TASK_NAME "StreamerMessage"

#define STREAMER_TASK_STACK_SIZE         16 * 1024
#define STREAMER_MESSAGE_TASK_STACK_SIZE 1024

OSA_TASK_HANDLE_DEFINE(msg_thread);
extern usb_audio_microphone_struct_t s_audioMicrophone;

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

bool STREAMER_IsPlaying(streamer_handle_t *handle)
{
    return handle->audioPlaying;
}

void STREAMER_Start(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] start usb microphone\r\n");

    USB_DeviceRun(s_audioMicrophone.deviceHandle);
    handle->audioPlaying = true;
    streamer_set_state(handle->streamer, 0, STATE_PLAYING, true);
}

void STREAMER_Stop(streamer_handle_t *handle)
{
    PRINTF("[STREAMER] stop usb microphone\r\n");

    USB_DeviceStop(s_audioMicrophone.deviceHandle);
    USB_DeviceAudioMicrophoneStatusReset();

    handle->audioPlaying = false;
    streamer_set_state(handle->streamer, 0, STATE_NULL, true);
}

status_t STREAMER_mic_Create(streamer_handle_t *handle)
{
    STREAMER_CREATE_PARAM params;
    osa_task_def_t thread_attr;
    ELEMENT_PROPERTY_T prop;
    int ret;

    /* Create streamer */
    strcpy(params.out_mq_name, APP_STREAMER_MSG_QUEUE);
    params.stack_size    = STREAMER_TASK_STACK_SIZE;
    params.pipeline_type = STREAM_PIPELINE_PCM;
    params.task_name     = STREAMER_TASK_NAME;
    params.in_dev_name   = "microphone";
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

    /* Set the dummy tx */
    prop.prop = PROP_AUDIOSRC_SET_DUMMY_TX_ENABLE;
    prop.val  = true;

    streamer_set_property(handle->streamer, prop, true);

    prop.prop = PROP_AUDIOSRC_SET_SAMPLE_RATE;
    prop.val  = AUDIO_SAMPLING_RATE;

    streamer_set_property(handle->streamer, prop, true);

    prop.prop = PROP_AUDIOSRC_SET_NUM_CHANNELS;
    prop.val  = 1;

    streamer_set_property(handle->streamer, prop, true);

    prop.prop = PROP_AUDIOSRC_SET_FRAME_MS;
    prop.val  = 1;

    streamer_set_property(handle->streamer, prop, true);

    return kStatus_Success;
}

void STREAMER_Destroy(streamer_handle_t *handle)
{
    streamer_destroy(handle->streamer);
    deinit_logging();
}

void STREAMER_Init(void)
{
    /* Initialize logging */
    init_logging();

    /* Uncomment below to turn on full debug logging for the streamer. */
    //    set_debug_module(0xffffffff);
    //    set_debug_level(LOGLVL_DEBUG);
    //    get_debug_state();

    /* Initialize streamer PCM management library. */
    streamer_pcm_init();
}
