/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_audio.h"
#include "usb_audio_config.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "audio_generator.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "composite.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size);
/*******************************************************************************
 * Variables
 ******************************************************************************/
uint8_t g_InterfaceIsSet = 0;

extern uint8_t s_wavBuff[];

static usb_device_composite_struct_t *g_deviceComposite;

usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Audio wav data prepare function.
 *
 * This function prepare audio wav data before send.
 */
/* USB device audio ISO OUT endpoint callback */
usb_status_t USB_DeviceAudioIsoOut(usb_device_handle deviceHandle,
                                   usb_device_endpoint_callback_message_struct_t *event,
                                   void *arg)
{
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)event;
    if ((g_deviceComposite->audioGenerator.attach) &&
        (ep_cb_param->length == ((USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                     HS_ISO_IN_ENDP_PACKET_SIZE :
                                     FS_ISO_IN_ENDP_PACKET_SIZE)))
    {
        USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                                  HS_ISO_IN_ENDP_PACKET_SIZE :
                                                  FS_ISO_IN_ENDP_PACKET_SIZE);
        return USB_DeviceSendRequest(deviceHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                     (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                         HS_ISO_IN_ENDP_PACKET_SIZE :
                                         FS_ISO_IN_ENDP_PACKET_SIZE);
    }

    return kStatus_USB_Error;
}

usb_status_t USB_DeviceAudioGetControlTerminal(
    usb_device_handle handle, usb_setup_struct_t *setup, uint32_t *length, uint8_t **buffer, uint8_t entityId)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint32_t audioCommand   = 0U;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;

    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Input Terminals only support the Get Terminal Copy Protect Control request */
                    }
                    break;
                case USB_DEVICE_AUDIO_TE_CONNECTOR_CONTROL:
                    audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_CONNECTOR_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_TE_OVERLOAD_CONTROL:
                    audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_OVERLOAD_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_GET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Input Terminals only support the Get Terminal Copy Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioSetControlTerminal(
    usb_device_handle handle, usb_setup_struct_t *setup, uint32_t *length, uint8_t **buffer, uint8_t entityId)
{
    uint32_t audioCommand   = 0U;
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;

    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Output Terminals only support the Set Terminal Copy Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_TE_COPY_PROTECT_CONTROL:
                    if (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId)
                    {
                        audioCommand = USB_DEVICE_AUDIO_TE_SET_CUR_COPY_PROTECT_CONTROL;
                    }
                    else
                    {
                        /* Output Terminals only support the Set Terminal Copy Protect Control request */
                    }
                    break;
                default:
                    /*no action*/
                    break;
            }
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetCurAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_AUTOMATIC_GAIN_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_BOOST_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_BASS_BOOST_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_LOUDNESS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_CUR_LOUDNESS_CONTROL;
            break;
        default:
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
usb_status_t USB_DeviceAudioGetRangeAudioFeatureUnit(usb_device_handle handle,
                                                     usb_setup_struct_t *setup,
                                                     uint32_t *length,
                                                     uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select GET RANGE request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}
#endif

usb_status_t USB_DeviceAudioGetMinAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetMaxAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetResAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetCurAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_MUTE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_AUTOMATIC_GAIN_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_BOOST_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_BASS_BOOST_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_LOUDNESS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_CUR_LOUDNESS_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioSetMinAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetMaxAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);

    return error;
}

usb_status_t USB_DeviceAudioSetResAudioFeatureUnit(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint32_t *length,
                                                   uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (controlSelector)
    {
        case USB_DEVICE_AUDIO_FU_VOLUME_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_BASS_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_MID_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_TREBLE_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_GRAPHIC_EQUALIZER_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_GRAPHIC_EQUALIZER_CONTROL;
            break;
        case USB_DEVICE_AUDIO_FU_DELAY_CONTROL_SELECTOR:
            audioCommand = USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL;
            break;
        default:
            break;
    }
    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetFeatureUnit(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            error = USB_DeviceAudioGetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            error = USB_DeviceAudioGetRangeAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            error = USB_DeviceAudioGetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_MIN_REQUEST:
            error = USB_DeviceAudioGetMinAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_MAX_REQUEST:
            error = USB_DeviceAudioGetMaxAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_RES_REQUEST:
            error = USB_DeviceAudioGetResAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            break;
#endif
    }
    return error;
}

usb_status_t USB_DeviceAudioSetFeatureUnit(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            error = USB_DeviceAudioSetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            error = USB_DeviceAudioSetCurAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_MIN_REQUEST:
            error = USB_DeviceAudioSetMinAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_MAX_REQUEST:
            error = USB_DeviceAudioSetMaxAudioFeatureUnit(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_RES_REQUEST:
            error = USB_DeviceAudioSetResAudioFeatureUnit(handle, setup, length, buffer);
            break;
        default:
            break;
#endif
    }
    return error;
}

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
usb_status_t USB_DeviceAudioSetClockSource(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (setup->bRequest)
    {
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}

usb_status_t USB_DeviceAudioGetClockSource(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t entityId        = (uint8_t)(setup->wIndex >> 0x08);

    switch (setup->bRequest)
    {
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_CS_CLOCK_VALID_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_RANGE_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_CS_SAM_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, entityId);
    return error;
}
#endif

usb_status_t USB_DeviceAudioSetRequestInterface(usb_device_handle handle,
                                                usb_setup_struct_t *setup,
                                                uint32_t *length,
                                                uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t entityId  = (uint8_t)(setup->wIndex >> 0x08);

    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioSetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioSetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_RECORDER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
    {
        error = USB_DeviceAudioSetClockSource(handle, setup, length, buffer);
    }
#endif

    return error;
}

usb_status_t USB_DeviceAudioGetRequestInterface(usb_device_handle handle,
                                                usb_setup_struct_t *setup,
                                                uint32_t *length,
                                                uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t entityId   = (uint8_t)(setup->wIndex >> 0x08);

    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioGetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId)
    {
        error = USB_DeviceAudioGetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_RECORDER_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
    {
        error = USB_DeviceAudioGetClockSource(handle, setup, length, buffer);
    }
#endif
    return error;
}

usb_status_t USB_DeviceAudioSetRequestEndpoint(usb_device_handle handle,
                                               usb_setup_struct_t *setup,
                                               uint32_t *length,
                                               uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t endpoint        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_PITCH_CONTROL_SELECTOR_AUDIO20:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_PITCH_CONTROL_AUDIO20;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_SET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_EP_PITCH_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_CUR_PITCH_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MIN_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_MAX_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_SET_RES_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;

        default:
            break;
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, endpoint); /* endpoint is not used */
    return error;
}

usb_status_t USB_DeviceAudioGetRequestEndpoint(usb_device_handle handle,
                                               usb_setup_struct_t *setup,
                                               uint32_t *length,
                                               uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_InvalidRequest;
    uint8_t controlSelector = (setup->wValue >> 0x08) & 0xFFU;
    uint32_t audioCommand   = 0U;
    uint8_t endpoint        = (uint8_t)(setup->wIndex >> 0x08);

    /* Select SET request Control Feature Unit Module */
    switch (setup->bRequest)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_DATA_OVERRUN_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_DATA_OVERRUN_CONTROL;
                    break;
                case USB_DEVICE_AUDIO_EP_DATA_UNDERRUN_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_DATA_UNDERRUN_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
        default:
            /*no action*/
            break;
#else
        case USB_DEVICE_AUDIO_GET_CUR_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_MIN_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_MAX_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:

                    audioCommand = USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL;
                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        case USB_DEVICE_AUDIO_GET_RES_REQUEST:
            switch (controlSelector)
            {
                case USB_DEVICE_AUDIO_EP_SAMPLING_FREQ_CONTROL_SELECTOR:
                    audioCommand = USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL;

                    break;
                default:
                    /*no action*/
                    break;
            }
            break;
        default:
            /*no action*/
            break;
#endif
    }

    error = USB_DeviceAudioProcessTerminalRequest(audioCommand, length, buffer, endpoint); /* endpoint is not used */
    return error;
}

usb_status_t USB_DeviceAudioGeneratorClassRequest(usb_device_handle handle,
                                                  usb_setup_struct_t *setup,
                                                  uint32_t *length,
                                                  uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    switch (setup->bmRequestType)
    {
        case USB_DEVICE_AUDIO_SET_REQUEST_INTERFACE:
            error = USB_DeviceAudioSetRequestInterface(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_REQUEST_INTERFACE:
            error = USB_DeviceAudioGetRequestInterface(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_SET_REQUEST_ENDPOINT:
            error = USB_DeviceAudioSetRequestEndpoint(handle, setup, length, buffer);
            break;
        case USB_DEVICE_AUDIO_GET_REQUEST_ENDPOINT:
            error = USB_DeviceAudioGetRequestEndpoint(handle, setup, length, buffer);
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint)
{
    usb_status_t error = kStatus_USB_Success;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint8_t *volBuffAddr;
#else
    uint16_t volume;
#endif

    switch (audioCommand)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curMute20;
            *length = sizeof(g_deviceComposite->audioGenerator.curMute20);
#else
            *buffer = &g_deviceComposite->audioGenerator.curMute;
            *length = sizeof(g_deviceComposite->audioGenerator.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curVolume20;
            *length = sizeof(g_deviceComposite->audioGenerator.curVolume20);
#else
            *buffer = g_deviceComposite->audioGenerator.curVolume;
            *length = sizeof(g_deviceComposite->audioGenerator.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.curBass;
            *length = sizeof(g_deviceComposite->audioGenerator.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.curMid;
            *length = sizeof(g_deviceComposite->audioGenerator.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.curTreble;
            *length = sizeof(g_deviceComposite->audioGenerator.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.curAutomaticGain;
            *length = sizeof(g_deviceComposite->audioGenerator.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.curDelay;
            *length = sizeof(g_deviceComposite->audioGenerator.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.minVolume;
            *length = sizeof(g_deviceComposite->audioGenerator.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.minBass;
            *length = sizeof(g_deviceComposite->audioGenerator.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.minMid;
            *length = sizeof(g_deviceComposite->audioGenerator.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.minTreble;
            *length = sizeof(g_deviceComposite->audioGenerator.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.minDelay;
            *length = sizeof(g_deviceComposite->audioGenerator.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.maxVolume;
            *length = sizeof(g_deviceComposite->audioGenerator.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.maxBass;
            *length = sizeof(g_deviceComposite->audioGenerator.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.maxMid;
            *length = sizeof(g_deviceComposite->audioGenerator.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.maxTreble;
            *length = sizeof(g_deviceComposite->audioGenerator.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.maxDelay;
            *length = sizeof(g_deviceComposite->audioGenerator.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.resVolume;
            *length = sizeof(g_deviceComposite->audioGenerator.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.resBass;
            *length = sizeof(g_deviceComposite->audioGenerator.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.resMid;
            *length = sizeof(g_deviceComposite->audioGenerator.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioGenerator.resTreble;
            *length = sizeof(g_deviceComposite->audioGenerator.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.resDelay;
            *length = sizeof(g_deviceComposite->audioGenerator.resDelay);
            break;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curSampleFrequency;
            *length = sizeof(g_deviceComposite->audioGenerator.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioGenerator.curSampleFrequency = *(uint32_t *)(*buffer);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curClockValid;
            *length = sizeof(g_deviceComposite->audioGenerator.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            g_deviceComposite->audioGenerator.curClockValid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.volumeControlRange;
            *length = sizeof(g_deviceComposite->audioGenerator.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioGenerator.freqControlRange;
            *length = sizeof(g_deviceComposite->audioGenerator.freqControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.curSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioGenerator.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.minSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioGenerator.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.maxSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioGenerator.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioGenerator.resSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioGenerator.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioGenerator.curSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioGenerator.curSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioGenerator.minSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioGenerator.minSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioGenerator.maxSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioGenerator.maxSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioGenerator.resSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioGenerator.resSamplingFrequency[1] = *((*buffer) + 1);
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            volBuffAddr                                      = *buffer;
            g_deviceComposite->audioGenerator.curVolume20[0] = *(volBuffAddr);
            g_deviceComposite->audioGenerator.curVolume20[1] = *(volBuffAddr + 1);
#else
            g_deviceComposite->audioGenerator.curVolume[0] = **(buffer);
            g_deviceComposite->audioGenerator.curVolume[1] = *((*buffer) + 1);
            volume = (uint16_t)((uint16_t)g_deviceComposite->audioGenerator.curVolume[1] << 8U);
            volume |= (uint8_t)(g_deviceComposite->audioGenerator.curVolume[0]);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioGenerator.curMute20 = **(buffer);
#else
            g_deviceComposite->audioGenerator.curMute = **(buffer);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            g_deviceComposite->audioGenerator.curBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            g_deviceComposite->audioGenerator.curMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            g_deviceComposite->audioGenerator.curTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            g_deviceComposite->audioGenerator.curAutomaticGain = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            g_deviceComposite->audioGenerator.curDelay[0] = **(buffer);
            g_deviceComposite->audioGenerator.curDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioGenerator.curDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioGenerator.curDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            g_deviceComposite->audioGenerator.minVolume[0] = **(buffer);
            g_deviceComposite->audioGenerator.minVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            g_deviceComposite->audioGenerator.minBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            g_deviceComposite->audioGenerator.minMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            g_deviceComposite->audioGenerator.minTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            g_deviceComposite->audioGenerator.minDelay[0] = **(buffer);
            g_deviceComposite->audioGenerator.minDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioGenerator.minDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioGenerator.minDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            g_deviceComposite->audioGenerator.maxVolume[0] = **(buffer);
            g_deviceComposite->audioGenerator.maxVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            g_deviceComposite->audioGenerator.maxBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            g_deviceComposite->audioGenerator.maxMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            g_deviceComposite->audioGenerator.maxTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            g_deviceComposite->audioGenerator.maxDelay[0] = **(buffer);
            g_deviceComposite->audioGenerator.maxDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioGenerator.maxDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioGenerator.maxDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            g_deviceComposite->audioGenerator.resVolume[0] = **(buffer);
            g_deviceComposite->audioGenerator.resVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            g_deviceComposite->audioGenerator.resBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            g_deviceComposite->audioGenerator.resMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            g_deviceComposite->audioGenerator.resTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            g_deviceComposite->audioGenerator.resDelay[0] = **(buffer);
            g_deviceComposite->audioGenerator.resDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioGenerator.resDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioGenerator.resDelay[3] = *((*buffer) + 3);
#endif
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

usb_status_t USB_DeviceAudioGeneratorConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    if (status)
    {
        if ((USB_AUDIO_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    else
    {
        if ((USB_AUDIO_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    return kStatus_USB_InvalidRequest;
}

/*!
 * @brief Audio Generator device set configuration function.
 *
 * This function sets configuration for Audio class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioGeneratorSetConfigure(usb_device_handle handle, uint8_t configure)
{
    usb_status_t error = kStatus_USB_Success;
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceComposite->audioGenerator.attach = 1U;

        usb_device_endpoint_init_struct_t epInitStruct;
        usb_device_endpoint_callback_struct_t epCallback;

        epCallback.callbackFn    = USB_DeviceAudioIsoOut;
        epCallback.callbackParam = handle;

        epInitStruct.zlt          = 0U;
        epInitStruct.transferType = USB_ENDPOINT_ISOCHRONOUS;
        epInitStruct.endpointAddress =
            USB_AUDIO_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
        if (USB_SPEED_HIGH == g_deviceComposite->speed)
        {
            epInitStruct.maxPacketSize = HS_ISO_IN_ENDP_PACKET_SIZE;
            epInitStruct.interval      = HS_ISO_IN_ENDP_INTERVAL;
        }
        else
        {
            epInitStruct.maxPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
            epInitStruct.interval      = FS_ISO_IN_ENDP_INTERVAL;
        }

        USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);
    }
    return error;
}

usb_status_t USB_DeviceAudioGeneratorSetInterface(usb_device_handle handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;
    if ((alternateSetting == 1U) && (g_InterfaceIsSet == 0))
    {
        g_InterfaceIsSet = 1;
        USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                                  HS_ISO_IN_ENDP_PACKET_SIZE :
                                                  FS_ISO_IN_ENDP_PACKET_SIZE);
        error = USB_DeviceSendRequest(handle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                      (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                          HS_ISO_IN_ENDP_PACKET_SIZE :
                                          FS_ISO_IN_ENDP_PACKET_SIZE);
    }
    return error;
}

/*!
 * @brief Audio Generator device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioGeneratorInit(usb_device_composite_struct_t *deviceComposite)
{
    g_deviceComposite                                         = deviceComposite;
    g_deviceComposite->audioGenerator.copyProtect             = 0x01U;
    g_deviceComposite->audioGenerator.curMute                 = 0x01U;
    g_deviceComposite->audioGenerator.curVolume[0]            = 0x00U;
    g_deviceComposite->audioGenerator.curVolume[1]            = 0x80U;
    g_deviceComposite->audioGenerator.minVolume[0]            = 0x00U;
    g_deviceComposite->audioGenerator.minVolume[1]            = 0x80U;
    g_deviceComposite->audioGenerator.maxVolume[0]            = 0xFFU;
    g_deviceComposite->audioGenerator.maxVolume[1]            = 0X7FU;
    g_deviceComposite->audioGenerator.resVolume[0]            = 0x01U;
    g_deviceComposite->audioGenerator.resVolume[1]            = 0x00U;
    g_deviceComposite->audioGenerator.curBass                 = 0x00U;
    g_deviceComposite->audioGenerator.curBass                 = 0x00U;
    g_deviceComposite->audioGenerator.minBass                 = 0x80U;
    g_deviceComposite->audioGenerator.maxBass                 = 0x7FU;
    g_deviceComposite->audioGenerator.resBass                 = 0x01U;
    g_deviceComposite->audioGenerator.curMid                  = 0x00U;
    g_deviceComposite->audioGenerator.minMid                  = 0x80U;
    g_deviceComposite->audioGenerator.maxMid                  = 0x7FU;
    g_deviceComposite->audioGenerator.resMid                  = 0x01U;
    g_deviceComposite->audioGenerator.curTreble               = 0x01U;
    g_deviceComposite->audioGenerator.minTreble               = 0x80U;
    g_deviceComposite->audioGenerator.maxTreble               = 0x7FU;
    g_deviceComposite->audioGenerator.resTreble               = 0x01U;
    g_deviceComposite->audioGenerator.curAutomaticGain        = 0x01U;
    g_deviceComposite->audioGenerator.curDelay[0]             = 0x00U;
    g_deviceComposite->audioGenerator.curDelay[1]             = 0x40U;
    g_deviceComposite->audioGenerator.minDelay[0]             = 0x00U;
    g_deviceComposite->audioGenerator.minDelay[1]             = 0x00U;
    g_deviceComposite->audioGenerator.maxDelay[0]             = 0xFFU;
    g_deviceComposite->audioGenerator.maxDelay[1]             = 0xFFU;
    g_deviceComposite->audioGenerator.resDelay[0]             = 0x00U;
    g_deviceComposite->audioGenerator.resDelay[1]             = 0x01U;
    g_deviceComposite->audioGenerator.curLoudness             = 0x01U;
    g_deviceComposite->audioGenerator.curSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioGenerator.curSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioGenerator.curSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioGenerator.minSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioGenerator.minSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioGenerator.minSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioGenerator.maxSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioGenerator.maxSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioGenerator.maxSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioGenerator.resSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioGenerator.resSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioGenerator.resSamplingFrequency[2] = 0x01U;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
    g_deviceComposite->audioGenerator.curMute20      = 0U;
    g_deviceComposite->audioGenerator.curClockValid  = 1U;
    g_deviceComposite->audioGenerator.curVolume20[0] = 0x00U;
    g_deviceComposite->audioGenerator.curVolume20[1] = 0x1FU;
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
    g_deviceComposite->audioGenerator.curSampleFrequency             = 16000U;
    g_deviceComposite->audioGenerator.freqControlRange.wNumSubRanges = 1U;
    g_deviceComposite->audioGenerator.freqControlRange.wMIN          = 16000U;
    g_deviceComposite->audioGenerator.freqControlRange.wMAX          = 16000U;
    g_deviceComposite->audioGenerator.freqControlRange.wRES          = 0U;
#else
    g_deviceComposite->audioGenerator.curSampleFrequency             = 8000U;
    g_deviceComposite->audioGenerator.freqControlRange.wNumSubRanges = 1U;
    g_deviceComposite->audioGenerator.freqControlRange.wMIN          = 8000U;
    g_deviceComposite->audioGenerator.freqControlRange.wMAX          = 8000U;
    g_deviceComposite->audioGenerator.freqControlRange.wRES          = 0U;
#endif
    g_deviceComposite->audioGenerator.volumeControlRange.wNumSubRanges = 1U;
    g_deviceComposite->audioGenerator.volumeControlRange.wMIN          = 0x8001U;
    g_deviceComposite->audioGenerator.volumeControlRange.wMAX          = 0x7FFFU;
    g_deviceComposite->audioGenerator.volumeControlRange.wRES          = 1U;

#endif
    return kStatus_USB_Success;
}
