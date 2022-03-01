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

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_audio_config.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"

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

extern uint8_t s_wavBuff[];

static usb_device_composite_struct_t *g_deviceComposite;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle           The USB device handle.
 * @param event            The USB device event type.
 * @param param            The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param)
{
    usb_device_control_request_struct_t *request = (usb_device_control_request_struct_t *)param;
    usb_status_t error                           = kStatus_USB_Success;
    uint16_t volume;

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curMute20;
            request->length = sizeof(g_deviceComposite->audioGenerator.curMute20);
#else
            request->buffer = &g_deviceComposite->audioGenerator.curMute;
            request->length = sizeof(g_deviceComposite->audioGenerator.curMute);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curVolume20;
            request->length = sizeof(g_deviceComposite->audioGenerator.curVolume20);
#else
            request->buffer = g_deviceComposite->audioGenerator.curVolume;
            request->length = sizeof(g_deviceComposite->audioGenerator.curVolume);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.curBass;
            request->length = sizeof(g_deviceComposite->audioGenerator.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.curMid;
            request->length = sizeof(g_deviceComposite->audioGenerator.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.curTreble;
            request->length = sizeof(g_deviceComposite->audioGenerator.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.curAutomaticGain;
            request->length = sizeof(g_deviceComposite->audioGenerator.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.curDelay;
            request->length = sizeof(g_deviceComposite->audioGenerator.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.minVolume;
            request->length = sizeof(g_deviceComposite->audioGenerator.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.minBass;
            request->length = sizeof(g_deviceComposite->audioGenerator.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.minMid;
            request->length = sizeof(g_deviceComposite->audioGenerator.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.minTreble;
            request->length = sizeof(g_deviceComposite->audioGenerator.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.minDelay;
            request->length = sizeof(g_deviceComposite->audioGenerator.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.maxVolume;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.maxBass;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.maxMid;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.maxTreble;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.maxDelay;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.resVolume;
            request->length = sizeof(g_deviceComposite->audioGenerator.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.resBass;
            request->length = sizeof(g_deviceComposite->audioGenerator.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.resMid;
            request->length = sizeof(g_deviceComposite->audioGenerator.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.resTreble;
            request->length = sizeof(g_deviceComposite->audioGenerator.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.resDelay;
            request->length = sizeof(g_deviceComposite->audioGenerator.resDelay);
            break;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curSampleFrequency;
            request->length = sizeof(g_deviceComposite->audioGenerator.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.curSampleFrequency;
                request->length = sizeof(g_deviceComposite->audioGenerator.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &g_deviceComposite->audioGenerator.curClockValid;
            request->length = sizeof(g_deviceComposite->audioGenerator.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curClockValid;
                request->length = sizeof(g_deviceComposite->audioGenerator.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.freqControlRange;
            request->length = sizeof(g_deviceComposite->audioGenerator.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_deviceComposite->audioGenerator.volumeControlRange;
            request->length = sizeof(g_deviceComposite->audioGenerator.volumeControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.curSamplingFrequency;
            request->length = sizeof(g_deviceComposite->audioGenerator.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.minSamplingFrequency;
            request->length = sizeof(g_deviceComposite->audioGenerator.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.maxSamplingFrequency;
            request->length = sizeof(g_deviceComposite->audioGenerator.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceComposite->audioGenerator.resSamplingFrequency;
            request->length = sizeof(g_deviceComposite->audioGenerator.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.curSamplingFrequency;
                request->length = sizeof(g_deviceComposite->audioGenerator.curSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.resSamplingFrequency;
                request->length = sizeof(g_deviceComposite->audioGenerator.resSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.maxSamplingFrequency;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.minSamplingFrequency;
                request->length = sizeof(g_deviceComposite->audioGenerator.minSamplingFrequency);
            }
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.curVolume20;
                request->length = sizeof(g_deviceComposite->audioGenerator.curVolume20);
            }
            else
            {
                volume = (uint16_t)((uint16_t)g_deviceComposite->audioGenerator.curVolume20[1] << 8U);
                volume |= (uint8_t)(g_deviceComposite->audioGenerator.curVolume20[0]);
                usb_echo("Set Cur Volume : %x\r\n", volume);
            }
#else
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.curVolume;
                request->length = sizeof(g_deviceComposite->audioGenerator.curVolume);
            }
            else
            {
                volume = (uint16_t)((uint16_t)g_deviceComposite->audioGenerator.curVolume[1] << 8U);
                volume |= (uint8_t)(g_deviceComposite->audioGenerator.curVolume[0]);
                usb_echo("Set Cur Volume : %x\r\n", volume);
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curMute20;
                request->length = sizeof(g_deviceComposite->audioGenerator.curMute20);
            }
            else
            {
                usb_echo("Set Cur Mute : %x\r\n", g_deviceComposite->audioGenerator.curMute20);
            }
#else
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curMute;
                request->length = sizeof(g_deviceComposite->audioGenerator.curMute);
            }
            else
            {
                usb_echo("Set Cur Mute : %x\r\n", g_deviceComposite->audioGenerator.curMute);
            }
            break;
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curBass;
                request->length = sizeof(g_deviceComposite->audioGenerator.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curMid;
                request->length = sizeof(g_deviceComposite->audioGenerator.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curTreble;
                request->length = sizeof(g_deviceComposite->audioGenerator.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.curAutomaticGain;
                request->length = sizeof(g_deviceComposite->audioGenerator.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.curDelay;
                request->length = sizeof(g_deviceComposite->audioGenerator.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.minVolume;
                request->length = sizeof(g_deviceComposite->audioGenerator.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.minBass;
                request->length = sizeof(g_deviceComposite->audioGenerator.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.minMid;
                request->length = sizeof(g_deviceComposite->audioGenerator.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.minTreble;
                request->length = sizeof(g_deviceComposite->audioGenerator.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.minDelay;
                request->length = sizeof(g_deviceComposite->audioGenerator.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.maxVolume;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.maxBass;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.maxMid;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.maxTreble;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.maxDelay;
                request->length = sizeof(g_deviceComposite->audioGenerator.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.resVolume;
                request->length = sizeof(g_deviceComposite->audioGenerator.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.resBass;
                request->length = sizeof(g_deviceComposite->audioGenerator.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.resMid;
                request->length = sizeof(g_deviceComposite->audioGenerator.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceComposite->audioGenerator.resTreble;
                request->length = sizeof(g_deviceComposite->audioGenerator.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceComposite->audioGenerator.resDelay;
                request->length = sizeof(g_deviceComposite->audioGenerator.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/*!
 * @brief device Audio callback function.
 *
 * This function handle the Audio class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
usb_status_t USB_DeviceAudioGeneratorCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)param;

    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
            if ((g_deviceComposite->audioGenerator.attach) &&
                (ep_cb_param->length == ((USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                             HS_ISO_IN_ENDP_PACKET_SIZE :
                                             FS_ISO_IN_ENDP_PACKET_SIZE)))
            {
                USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                                          HS_ISO_IN_ENDP_PACKET_SIZE :
                                                          FS_ISO_IN_ENDP_PACKET_SIZE);
                error = USB_DeviceAudioSend(handle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                                            (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                                HS_ISO_IN_ENDP_PACKET_SIZE :
                                                FS_ISO_IN_ENDP_PACKET_SIZE);
            }
            break;

        default:
            if (param && (event > 0xFF))
            {
                error = USB_DeviceAudioRequest(handle, event, param);
            }
            break;
    }

    return error;
}
/*!
 * @brief Audio set configuration function.
 *
 * This function sets configuration for msc class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioGeneratorSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceComposite->audioGenerator.attach = 1U;
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceAudioGeneratorSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (alternateSetting == 1U)
    {
        USB_AudioRecorderGetBuffer(s_wavBuff, (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ?
                                                  HS_ISO_IN_ENDP_PACKET_SIZE :
                                                  FS_ISO_IN_ENDP_PACKET_SIZE);
        USB_DeviceAudioSend(g_deviceComposite->audioGenerator.audioHandle, USB_AUDIO_STREAM_ENDPOINT, s_wavBuff,
                            (USB_SPEED_HIGH == g_deviceComposite->audioGenerator.speed) ? HS_ISO_IN_ENDP_PACKET_SIZE :
                                                                                          FS_ISO_IN_ENDP_PACKET_SIZE);
    }
    return kStatus_USB_Error;
}
/*!
 * @brief Audio init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceAudioGeneratorInit(usb_device_composite_struct_t *device_composite)
{
    g_deviceComposite                                         = device_composite;
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
