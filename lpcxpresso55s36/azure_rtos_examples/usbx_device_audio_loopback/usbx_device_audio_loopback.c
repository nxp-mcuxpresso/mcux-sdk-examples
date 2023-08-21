/* This example illustrates USBX device of audio */

#include "ux_api.h"

#include "ux_device_class_audio.h"
#include "ux_device_class_audio20.h"
#include "ux_device_descriptor.h"

#include "fsl_debug_console.h"
#include "board_setup.h"

#ifndef USBX_MEMORY_SIZE
#define USBX_MEMORY_SIZE                    (1024 * 64)
#endif

#define USB_THREAD_STACK_SIZE               (1024 * 2)
#define UX_DEMO_SAMPLING_FREQUENCY          48000
#define UX_DEMO_MAX_FRAME_SIZE              256
#define UX_DEMO_FRAME_BUFFER_NB             3
#define UX_DEMO_TRANSMIT_START_THRESHOLD    1

/* Define local variables.  */
UX_DEVICE_CLASS_AUDIO                  *audio;
UX_DEVICE_CLASS_AUDIO_STREAM           *stream_write;
UX_DEVICE_CLASS_AUDIO_STREAM           *stream_read;
UX_DEVICE_CLASS_AUDIO_PARAMETER         audio_parameter;
UX_DEVICE_CLASS_AUDIO_STREAM_PARAMETER  audio_stream_parameter[2];
UX_DEVICE_CLASS_AUDIO20_CONTROL         audio_control[2];
ULONG                                   audio_frame_write_start;

static TX_THREAD                        thread_usb;

static ULONG                            thread_usb_stack[USB_THREAD_STACK_SIZE / sizeof(ULONG)];

AT_NONCACHEABLE_SECTION_ALIGN(static ULONG usb_memory[USBX_MEMORY_SIZE / sizeof(ULONG)], 64);

UCHAR                                   audio_buffer[UX_DEMO_MAX_FRAME_SIZE];
INT                                     audio_frame_length = 192;

static void demo_audio_instance_activate(void *audio_instance)
{
    /* Save the Audio instance.  */
    audio = (UX_DEVICE_CLASS_AUDIO *)audio_instance;

    /* Get the streams instances.  */
    ux_device_class_audio_stream_get(audio, 0, &stream_write);
    ux_device_class_audio_stream_get(audio, 1, &stream_read);

    PRINTF("Audio device activate\r\n");
}

static void demo_audio_instance_deactivate(void *audio_instance)
{
    /* Reset the Audio instance.  */
    audio = UX_NULL;

    /* Reset the Audio streams.  */
    stream_write = UX_NULL;
    stream_read = UX_NULL;

    PRINTF("Audio device deactivate\r\n");
}

static void demo_audio_read_change(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG alternate_setting)
{
    /* Do nothing if alternate setting is 0 (stream closed).  */
    if (alternate_setting == 0)
    {
        memset(audio_buffer, 0, sizeof(audio_buffer));
        return;
    }
    /* Start reception (stream opened).  */
    ux_device_class_audio_reception_start(stream);
}

static void demo_audio_write_change(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG alternate_setting)
{
    /* Stop audio frame loop back if stream closed.  */
    if (alternate_setting == 0)
    {
        audio_frame_write_start = 0xFFFFFFFF;
        return;
    }

    ux_device_class_audio_frame_write(stream, audio_buffer, audio_frame_length);
    ux_device_class_audio_transmission_start(stream);

    /* Set start delay - at least 1 frame, recommend half frame buffer size.  */
    audio_frame_write_start = UX_DEMO_TRANSMIT_START_THRESHOLD;
}

static void demo_audio_read_done(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG length)
{
    UCHAR *frame_buffer;
    ULONG frame_length;

    /* Get access to first audio input frame.  */
    ux_device_class_audio_read_frame_get(stream, &frame_buffer, &frame_length);

    /* Write audio data to the audio buffer.  */
    audio_frame_length = frame_length;
    memcpy(audio_buffer, frame_buffer, frame_length);

    /* Re-free the first audio input frame for transfer.  */
    ux_device_class_audio_read_frame_free(stream);
}

static void demo_audio_write_done(UX_DEVICE_CLASS_AUDIO_STREAM *stream, ULONG length)
{
    /* Write frame data to write stream.  */
    ux_device_class_audio_frame_write(stream, audio_buffer, audio_frame_length);
}

static UINT demo_audio20_request_process(UX_DEVICE_CLASS_AUDIO *audio, UX_SLAVE_TRANSFER *transfer)
{
    UINT status;
    UX_DEVICE_CLASS_AUDIO20_CONTROL_GROUP  group = {2, audio_control};

    /* Handle request and update control values.
       Note here only mute and volume for master channel is supported.
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

static void thread_usb_entry(ULONG thread_input)
{
    UX_PARAMETER_NOT_USED(thread_input);
}

int main(void)
{
    /* Initialize the board. */
    board_setup();

    usb_device_hw_setup();

    PRINTF("USBX device audio loopback example...\r\n");

    /* Enter the ThreadX kernel. */
    tx_kernel_enter();

    return 0;
}

void tx_application_define(void *first_unused_memory)
{
    UINT    status;

    /* Initialize USBX Memory */
    status = ux_system_initialize((VOID *)usb_memory, USBX_MEMORY_SIZE, UX_NULL, 0);
    if (status != UX_SUCCESS)
        goto err;

    usb_device_setup();

    /* The code below is required for installing the device portion of USBX.
       In this demo, DFU is possible and we have a call back for state change. */
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
    audio_control[0].ux_device_class_audio20_control_fu_id                = 2;
    audio_control[0].ux_device_class_audio20_control_mute[0]              = 0;
    audio_control[0].ux_device_class_audio20_control_volume_min[0]        = 0;
    audio_control[0].ux_device_class_audio20_control_volume_max[0]        = 100;
    audio_control[0].ux_device_class_audio20_control_volume_res[0]        = 1;
    audio_control[0].ux_device_class_audio20_control_volume[0]            = 50;
    audio_control[1].ux_device_class_audio20_control_cs_id                = 0x10;
    audio_control[1].ux_device_class_audio20_control_sampling_frequency   = UX_DEMO_SAMPLING_FREQUENCY;
    audio_control[1].ux_device_class_audio20_control_fu_id                = 5;
    audio_control[1].ux_device_class_audio20_control_mute[0]              = 0;
    audio_control[1].ux_device_class_audio20_control_volume_min[0]        = 0;
    audio_control[1].ux_device_class_audio20_control_volume_max[0]        = 100;
    audio_control[1].ux_device_class_audio20_control_volume_res[0]        = 1;
    audio_control[1].ux_device_class_audio20_control_volume[0]            = 50;

    /* Set the parameters for Audio streams.  */
    audio_stream_parameter[0].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_change     = demo_audio_write_change;
    audio_stream_parameter[0].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_frame_done = demo_audio_write_done;
    audio_stream_parameter[0].ux_device_class_audio_stream_parameter_max_frame_buffer_nb   = UX_DEMO_FRAME_BUFFER_NB;
    audio_stream_parameter[0].ux_device_class_audio_stream_parameter_max_frame_buffer_size = UX_DEMO_MAX_FRAME_SIZE;
    audio_stream_parameter[0].ux_device_class_audio_stream_parameter_thread_entry = ux_device_class_audio_write_thread_entry;
    audio_stream_parameter[1].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_change     = demo_audio_read_change;
    audio_stream_parameter[1].ux_device_class_audio_stream_parameter_callbacks.ux_device_class_audio_stream_frame_done = demo_audio_read_done;
    audio_stream_parameter[1].ux_device_class_audio_stream_parameter_max_frame_buffer_nb   = UX_DEMO_FRAME_BUFFER_NB;
    audio_stream_parameter[1].ux_device_class_audio_stream_parameter_max_frame_buffer_size = UX_DEMO_MAX_FRAME_SIZE;
    audio_stream_parameter[1].ux_device_class_audio_stream_parameter_thread_entry = ux_device_class_audio_read_thread_entry;

    /* Set the parameters for Audio device.  */
    audio_parameter.ux_device_class_audio_parameter_streams_nb  = 2;
    audio_parameter.ux_device_class_audio_parameter_streams     = audio_stream_parameter;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_activate   = demo_audio_instance_activate;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_slave_class_audio_instance_deactivate = demo_audio_instance_deactivate;
    audio_parameter.ux_device_class_audio_parameter_callbacks.ux_device_class_audio_control_process    = demo_audio20_request_process;

    /* Initialize the device Audio class. This class owns interfaces starting with 0. */
     status =  ux_device_stack_class_register(_ux_system_slave_class_audio_name,
                                              ux_device_class_audio_entry,
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
