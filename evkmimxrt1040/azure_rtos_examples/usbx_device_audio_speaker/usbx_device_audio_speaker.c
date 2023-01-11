/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* This example illustrates USBX Device as an audio speaker  */

#include "ux_api.h"
#include "ux_device_class_audio.h"
#include "ux_device_class_audio20.h"
#include "ux_device_stack.h"
#include "ux_device_descriptor.h"

#include "usb.h"
#include "board_setup.h"
#include "audio_speaker.h"
#include "fsl_debug_console.h"

#define USB_THREAD_STACK_SIZE               (1024 * 2)
#define USBX_MEMORY_SIZE                     (1024 * 64)

/* Definition check.  */
#if !defined(UX_DEVICE_CLASS_AUDIO_FEEDBACK_SUPPORT) || !defined(UX_DEVICE_BIDIRECTIONAL_ENDPOINT_SUPPORT)
#error Must enable audio feedback and bidirectional endpoint support
#endif
#if UX_SLAVE_REQUEST_CONTROL_MAX_LENGTH < 261
#error Must have enough control request buffer
#endif
#if defined(UX_DEVICE_INITIALIZE_FRAMEWORK_SCAN_DISABLE) && (UX_MAX_DEVICE_ENDPOINTS < 2)
#error Must support at least 2 endpoints, please change compile option (UX_DEVICE_INITIALIZE_FRAMEWORK_SCAN_DISABLE, UX_MAX_DEVICE_ENDPOINTS)
#endif
#if UX_MAX_SLAVE_INTERFACES < 2
#error Must support at least 2 interfaces, please change compile option (UX_MAX_SLAVE_INTERFACES)
#endif
#ifdef UX_DEVICE_ALTERNATE_SETTING_SUPPORT_DISABLE
#error Must support alternate settings, please undefine compile option(UX_DEVICE_ALTERNATE_SETTING_SUPPORT_DISABLE)
#endif

#define UX_DEMO_SAMPLING_FREQUENCY          48000
#define UX_DEMO_MAX_FRAME_SIZE              256
#define UX_DEMO_FRAME_BUFFER_NB             3
#define UX_DEMO_TRANSMIT_START_THRESHOLD    1

#define UX_DEMO_FEEDBACK_BP_SHIFT_HS        16
#define UX_DEMO_FEEDBACK_BP_SHIFT_FS        14
#define UX_DEMO_FEEDBACK_INTERVAL           8192
#define UX_DEMO_BYTES_PER_SAMPLE            4
#define UX_DEMO_BYTES_SYNC_THRESHOLD        64

/* Define local variables.  */
UX_DEVICE_CLASS_AUDIO                  *audio;
UX_DEVICE_CLASS_AUDIO_STREAM           *stream_write;
UX_DEVICE_CLASS_AUDIO_STREAM           *stream_read;
UX_DEVICE_CLASS_AUDIO_PARAMETER         audio_parameter;
UX_DEVICE_CLASS_AUDIO_STREAM_PARAMETER  audio_stream_parameter;
UX_DEVICE_CLASS_AUDIO20_CONTROL         audio_control[1];

static TX_THREAD                        thread_usb;
static ULONG                            thread_usb_stack[USB_THREAD_STACK_SIZE / sizeof(ULONG)];

ULONG                                   usb_transfer_packet_count;
INT                                     audio_resync_flag;
INT                                     feedback_bp_shift;
ULONG                                   audio_frame_read_feedback;

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t usb_memory[USBX_MEMORY_SIZE];

void demo_audio_instance_activate(VOID *audio_instance)
{
    /* Save the Audio instance.  */
    audio = (UX_DEVICE_CLASS_AUDIO *)audio_instance;

    PRINTF("USB Audio Speaker device activate\r\n");

    /* Get the streams instances.  */
    ux_device_class_audio_stream_get(audio, 1, &stream_read);
}

void demo_audio_instance_deactivate(VOID *audio_instance)
{
    /* Reset the Audio instance.  */
    audio = UX_NULL;

    /* Reset the Audio streams.  */
    stream_read = UX_NULL;

    audio_clean_tx_buff();

    PRINTF("USB Audio Speaker device deactivate\r\n");
}

void demo_audio_read_change(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG alternate_setting)
{
    /* Do nothing if alternate setting is 0 (stream closed).  */
    if (alternate_setting == 0)
    {
        audio_clean_tx_buff();
        return;
    }

    audio_set_tx_resync();

    usb_transfer_packet_count = 0;

    /* Initialize data rate.  */
    if (ux_device_class_audio_speed_get(stream) == UX_HIGH_SPEED_DEVICE)
    {
        feedback_bp_shift = UX_DEMO_FEEDBACK_BP_SHIFT_HS;
        audio_frame_read_feedback = (UX_DEMO_SAMPLING_FREQUENCY / 8000) << feedback_bp_shift;
    }
    else
    {
        feedback_bp_shift = UX_DEMO_FEEDBACK_BP_SHIFT_FS;
        audio_frame_read_feedback = (UX_DEMO_SAMPLING_FREQUENCY / 1000) << feedback_bp_shift;
    }

    ux_device_class_audio_feedback_set(stream, (UCHAR *)&audio_frame_read_feedback);

    /* Start reception (stream opened).  */
    ux_device_class_audio_reception_start(stream);
}

void demo_audio_read_done(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG length)
{
    UCHAR               *frame_buffer;
    ULONG               frame_length;
    size_t              transferred_byte_count;

    usb_transfer_packet_count++;
    if ((usb_transfer_packet_count % UX_DEMO_FEEDBACK_INTERVAL) == 0)
    {
        transferred_byte_count = audio_get_tx_cout();
        audio_frame_read_feedback = (transferred_byte_count / UX_DEMO_BYTES_PER_SAMPLE) * ((1 << feedback_bp_shift) / UX_DEMO_FEEDBACK_INTERVAL);
        ux_device_class_audio_feedback_set(stream, (UCHAR*)&audio_frame_read_feedback);
    }

    /* Get access to first audio input frame.  */
    if(ux_device_class_audio_read_frame_get(stream, &frame_buffer, &frame_length) == UX_SUCCESS)
    {
        audio_send(frame_buffer, frame_length);
    }

    /* Re-free the first audio input frame for transfer.  */
    ux_device_class_audio_read_frame_free(stream);
}

UINT demo_audio20_request_process(UX_DEVICE_CLASS_AUDIO *audio, UX_SLAVE_TRANSFER *transfer)
{
    UINT                                    status;
    UX_DEVICE_CLASS_AUDIO20_CONTROL_GROUP   group = {1, audio_control};

    /*
     * Handle request and update control values.
     * Note here only mute and volume for master channel is supported.
     */
    status = ux_device_class_audio20_control_process(audio, transfer, &group);
    if (status == UX_SUCCESS)
    {
        /* Request handled, check changes */
        switch(audio_control[0].ux_device_class_audio20_control_changed)
        {
        case UX_DEVICE_CLASS_AUDIO20_CONTROL_MUTE_CHANGED:
        case UX_DEVICE_CLASS_AUDIO20_CONTROL_VOLUME_CHANGED:
        default:
            break;
        }
    }
    return(status);
}

/* Define the USB thread.  */
void thread_usb_entry(ULONG thread_input)
{

    audio_speaker_start();

    usb_device_setup();
}

int main(void)
{
    /* Initialize the board. */
    audio_speaker_board_setup();

    usb_device_hw_setup();

    PRINTF("USBX Device Audio Speaker example\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

/* Define what the initial system looks like.  */

void tx_application_define(void *first_unused_memory)
{
    UINT    status;

    UX_PARAMETER_NOT_USED(first_unused_memory);

    /* Initialize USBX Memory */
    status = ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (status != UX_SUCCESS)
        goto err;

    /* The code below is required for installing the device portion of USBX. No call back for
       device status change in this example. */
    status = ux_device_stack_initialize(ux_get_hs_framework(),
                                        ux_get_hs_framework_length(),
                                        ux_get_fs_framework(),
                                        ux_get_fs_framework_length(),
                                        ux_get_string_framework(),
                                        ux_get_string_framework_length(),
                                        ux_get_language_framework(),
                                        ux_get_language_framework_length(),
                                        UX_NULL);
    if (status != UX_SUCCESS)
        goto err;

    /* Initialize audio 2.0 control values.  */
    audio_control[0].ux_device_class_audio20_control_cs_id                = 0x10;
    audio_control[0].ux_device_class_audio20_control_sampling_frequency   = UX_DEMO_SAMPLING_FREQUENCY;
    audio_control[0].ux_device_class_audio20_control_fu_id                = 5;
    audio_control[0].ux_device_class_audio20_control_mute[0]              = 0;
    audio_control[0].ux_device_class_audio20_control_volume_min[0]        = 0;
    audio_control[0].ux_device_class_audio20_control_volume_max[0]        = 100;
    audio_control[0].ux_device_class_audio20_control_volume_res[0]        = 1;
    audio_control[0].ux_device_class_audio20_control_volume[0]            = 50;

    /* Set the parameters for Audio streams.  */
    audio_stream_parameter.ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_change     = demo_audio_read_change;
    audio_stream_parameter.ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_frame_done = demo_audio_read_done;
    audio_stream_parameter.ux_device_class_audio_stream_parameter_max_frame_buffer_nb   = UX_DEMO_FRAME_BUFFER_NB;
    audio_stream_parameter.ux_device_class_audio_stream_parameter_max_frame_buffer_size = UX_DEMO_MAX_FRAME_SIZE;
    audio_stream_parameter.ux_device_class_audio_stream_parameter_thread_entry = ux_device_class_audio_read_thread_entry;
    audio_stream_parameter.ux_device_class_audio_stream_parameter_feedback_thread_entry = ux_device_class_audio_feedback_thread_entry;

    /* Set the parameters for Audio device.  */
    audio_parameter.ux_device_class_audio_parameter_streams_nb  = 1;
    audio_parameter.ux_device_class_audio_parameter_streams     = &audio_stream_parameter;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_activate   = demo_audio_instance_activate;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_deactivate = demo_audio_instance_deactivate;

    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_device_class_audio_control_process    = demo_audio20_request_process;

    /* Initialize the device Audio class. This class owns interfaces starting with 0. */
     status =  ux_device_stack_class_register(_ux_system_slave_class_audio_name, ux_device_class_audio_entry,
                                              1, 0, &audio_parameter);
    if (status != UX_SUCCESS)
        goto err;

    /* Create the main demo thread.  */
    tx_thread_create(&thread_usb, "USB thread", thread_usb_entry, 0,
            thread_usb_stack, USB_THREAD_STACK_SIZE,
            20, 20, 1, TX_AUTO_START);

    return;

err:
    PRINTF("tx_application_define: ERROR 0x%x\r\n", status);
    while (1)
    {
    }
}
