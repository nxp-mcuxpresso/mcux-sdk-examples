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

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "board.h"
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#include "fsl_ctimer.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* audio 2.0 and high speed, use low latency, but IP3511HS controller do not have micro frame count */
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
volatile static uint8_t s_microFrameCountIp3511HS = 0;
#endif
#endif

#define USB_AUDIO_ENTER_CRITICAL() \
                                   \
    OSA_SR_ALLOC();                \
                                   \
    OSA_ENTER_CRITICAL()

#define USB_AUDIO_EXIT_CRITICAL() OSA_EXIT_CRITICAL()
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

extern void BOARD_SetCodecMuteUnmute(bool);
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
extern void CTIMER_CaptureInit(void);
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
extern void audio_fro_trim_up(void);
extern void audio_fro_trim_down(void);
#endif
extern void USB_AudioPllChange(void);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayDataBuff[AUDIO_SPEAKER_DATA_WHOLE_BUFFER_COUNT_NORMAL * AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME];
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[FS_ISO_OUT_ENDP_PACKET_SIZE];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioPlayPacket[(FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)];
#endif

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecDataBuff[AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE];
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecPacket[(FS_ISO_IN_ENDP_PACKET_SIZE)];
#else
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t audioRecPacket[(FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t usbAudioFeedBackBuffer[4];
USB_RAM_ADDRESS_ALIGNMENT(4) uint8_t audioFeedBackBuffer[4];
volatile uint8_t feedbackValueUpdating;
#endif

usb_device_composite_struct_t *g_deviceAudioComposite;
volatile bool g_CodecSpeakerMuteUnmute    = false;
volatile bool g_CodecMicrophoneMuteUnmute = false;
extern usb_device_composite_struct_t g_composite;

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
    uint8_t entity_id                            = (uint8_t)(request->setup->wIndex >> 0x08);
#if (!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint16_t volume;
#endif

    switch (event)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSpeakerMute20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerMute20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curMicrophoneMute20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneMute20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSpeakerMute;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerMute);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curMicrophoneMute;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneMute);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSpeakerVolume20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerVolume20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curMicrophoneVolume20;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneVolume20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curSpeakerVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerVolume);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curMicrophoneVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneVolume);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curAutomaticGain;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.curDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.minTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.maxTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resVolume;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resBass;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resMid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.resTreble;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resDelay;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resDelay);
            break;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSampleFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.curSampleFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSampleFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            request->buffer = &g_deviceAudioComposite->audioUnified.curClockValid;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curClockValid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curClockValid);
            }
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.freqControlRange;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.freqControlRange);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            request->buffer = (uint8_t *)&g_deviceAudioComposite->audioUnified.volumeControlRange;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.volumeControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.curSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.minSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.maxSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            request->buffer = g_deviceAudioComposite->audioUnified.resSamplingFrequency;
            request->length = sizeof(g_deviceAudioComposite->audioUnified.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxSamplingFrequency);
            }
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minSamplingFrequency;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minSamplingFrequency);
            }
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_deviceAudioComposite->audioUnified.curSpeakerVolume20;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerVolume20);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_deviceAudioComposite->audioUnified.curMicrophoneVolume20;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneVolume20);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    g_deviceAudioComposite->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
                }
                else
                {
                    /* no action */
                }
            }
#else
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_deviceAudioComposite->audioUnified.curSpeakerVolume;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerVolume);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = g_deviceAudioComposite->audioUnified.curMicrophoneVolume;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneVolume);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    volume = (uint16_t)((uint16_t)g_deviceAudioComposite->audioUnified.curSpeakerVolume[1] << 8U);
                    volume |= (uint8_t)(g_deviceAudioComposite->audioUnified.curSpeakerVolume[0]);
                    g_deviceAudioComposite->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    volume = (uint16_t)((uint16_t)g_deviceAudioComposite->audioUnified.curMicrophoneVolume[1] << 8U);
                    volume |= (uint8_t)(g_deviceAudioComposite->audioUnified.curMicrophoneVolume[0]);
                    g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
                }
                else
                {
                    /* no action */
                }
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_deviceAudioComposite->audioUnified.curSpeakerMute20;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerMute20);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_deviceAudioComposite->audioUnified.curMicrophoneMute20;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneMute20);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                /* trigger task, only adjust speaker mute/unmute practically */
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_deviceAudioComposite->audioUnified.curSpeakerMute20)
                    {
                        g_deviceAudioComposite->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_deviceAudioComposite->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_deviceAudioComposite->audioUnified.curSpeakerMute20)
                    {
                        g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else
                {
                    /* no action */
                }
            }
#else
            if (request->isSetup == 1U)
            {
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_deviceAudioComposite->audioUnified.curSpeakerMute;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curSpeakerMute);
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    request->buffer = &g_deviceAudioComposite->audioUnified.curMicrophoneMute;
                    request->length = sizeof(g_deviceAudioComposite->audioUnified.curMicrophoneMute);
                }
                else
                {
                    /* no action */
                }
            }
            else
            {
                /* trigger task, only adjust speaker mute/unmute practically */
                if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_deviceAudioComposite->audioUnified.curSpeakerMute)
                    {
                        g_deviceAudioComposite->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_deviceAudioComposite->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entity_id)
                {
                    if (g_deviceAudioComposite->audioUnified.curMicrophoneMute)
                    {
                        g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= MUTE_CODEC_TASK;
                    }
                    else
                    {
                        g_deviceAudioComposite->audioUnified.codecMicrophoneTask |= UNMUTE_CODEC_TASK;
                    }
                }
                else
                {
                    /* no action */
                }
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.curAutomaticGain;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curAutomaticGain);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.curDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.curDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.minTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.minDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.minDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.maxTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.maxDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.maxDelay);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resVolume;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resVolume);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resBass;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resBass);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resMid;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resMid);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = &g_deviceAudioComposite->audioUnified.resTreble;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resTreble);
            }
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            if (request->isSetup == 1U)
            {
                request->buffer = g_deviceAudioComposite->audioUnified.resDelay;
                request->length = sizeof(g_deviceAudioComposite->audioUnified.resDelay);
            }
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

/* The USB_AudioSpeakerBufferSpaceUsed() function gets the used speaker ringbuffer size */
uint32_t USB_AudioSpeakerBufferSpaceUsed(void)
{
    uint64_t write_count = 0U;
    uint64_t read_count  = 0U;

    write_count = (uint64_t)((((uint64_t)g_deviceAudioComposite->audioUnified.audioSpeakerWriteDataCount[1]) << 32U) |
                             g_deviceAudioComposite->audioUnified.audioSpeakerWriteDataCount[0]);
    read_count  = (uint64_t)((((uint64_t)g_deviceAudioComposite->audioUnified.audioSpeakerReadDataCount[1]) << 32U) |
                            g_deviceAudioComposite->audioUnified.audioSpeakerReadDataCount[0]);

    if (write_count >= read_count)
    {
        return (uint32_t)(write_count - read_count);
    }
    else
    {
        return 0;
    }
}

/* The USB_AudioRecorderBufferSpaceUsed() function gets the reserved recorder ringbuffer size */
uint32_t USB_AudioRecorderBufferSpaceUsed(void)
{
    if (g_deviceAudioComposite->audioUnified.tdWriteNumberRec > g_deviceAudioComposite->audioUnified.tdReadNumberRec)
    {
        g_deviceAudioComposite->audioUnified.recorderReservedSpace =
            g_deviceAudioComposite->audioUnified.tdWriteNumberRec -
            g_deviceAudioComposite->audioUnified.tdReadNumberRec;
    }
    else
    {
        g_deviceAudioComposite->audioUnified.recorderReservedSpace =
            g_deviceAudioComposite->audioUnified.tdWriteNumberRec +
            AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE -
            g_deviceAudioComposite->audioUnified.tdReadNumberRec;
    }
    return g_deviceAudioComposite->audioUnified.recorderReservedSpace;
}

/* The USB_AudioRecorderGetBuffer() function gets audioRecPacket from the audioRecDataBuff in every callback*/
void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size)
{
    while (size)
    {
        *buffer = audioRecDataBuff[g_deviceAudioComposite->audioUnified.tdReadNumberRec];
        g_deviceAudioComposite->audioUnified.tdReadNumberRec++;
        buffer++;
        size--;

        if (g_deviceAudioComposite->audioUnified.tdReadNumberRec >=
            AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            g_deviceAudioComposite->audioUnified.tdReadNumberRec = 0;
        }
    }
}

/* The USB_AudioSpeakerPutBuffer() function fills the audioRecDataBuff with audioPlayPacket in every callback*/
void USB_AudioSpeakerPutBuffer(uint8_t *buffer, uint32_t size)
{
    uint32_t audioSpeakerPreWriteDataCount = 0U;
    uint32_t remainBufferSpace;
    uint32_t buffer_length = 0;

    remainBufferSpace = g_deviceAudioComposite->audioUnified.audioPlayBufferSize - USB_AudioSpeakerBufferSpaceUsed();
    if (size > remainBufferSpace) /* discard the overflow data */
    {
        if (remainBufferSpace > (AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE))
        {
            size = (remainBufferSpace - (AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE));
        }
        else
        {
            size = 0;
        }
    }
    if (size > 0)
    {
        buffer_length = g_deviceAudioComposite->audioUnified.tdWriteNumberPlay + size;
        if (buffer_length < g_deviceAudioComposite->audioUnified.audioPlayBufferSize)
        {
            memcpy((void *)(&audioPlayDataBuff[g_deviceAudioComposite->audioUnified.tdWriteNumberPlay]),
                   (void *)(&buffer[0]), size);
            g_deviceAudioComposite->audioUnified.tdWriteNumberPlay += size;
        }
        else
        {
            uint32_t firstLength = g_deviceAudioComposite->audioUnified.audioPlayBufferSize -
                                   g_deviceAudioComposite->audioUnified.tdWriteNumberPlay;
            memcpy((void *)(&audioPlayDataBuff[g_deviceAudioComposite->audioUnified.tdWriteNumberPlay]),
                   (void *)(&buffer[0]), firstLength);
            buffer_length = size - firstLength; /* the remain data length */
            if ((buffer_length) > 0U)
            {
                memcpy((void *)(&audioPlayDataBuff[0]), (void *)((uint8_t *)(&buffer[0]) + firstLength), buffer_length);
                g_deviceAudioComposite->audioUnified.tdWriteNumberPlay = buffer_length;
            }
            else
            {
                g_deviceAudioComposite->audioUnified.tdWriteNumberPlay = 0;
            }
        }
        audioSpeakerPreWriteDataCount = g_composite.audioUnified.audioSpeakerWriteDataCount[0];
        g_composite.audioUnified.audioSpeakerWriteDataCount[0] += size;
        if (audioSpeakerPreWriteDataCount > g_composite.audioUnified.audioSpeakerWriteDataCount[0])
        {
            g_composite.audioUnified.audioSpeakerWriteDataCount[1] += 1U;
        }
    }
}

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
void USB_DeviceCalculateFeedback(void)
{
    volatile static uint64_t totalFrameValue = 0U;
    volatile static uint32_t frameDistance   = 0U;
    volatile static uint32_t feedbackValue   = 0U;

    uint32_t audioSpeakerUsedSpace = 0U;

    /* feedback interval is AUDIO_CALCULATE_Ff_INTERVAL */
    if (USB_SPEED_HIGH == g_deviceAudioComposite->speed)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) /* high speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
        if (g_deviceAudioComposite->audioUnified.speakerIntervalCount !=
            AUDIO_CALCULATE_Ff_INTERVAL *
                (AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME / g_deviceAudioComposite->audioUnified.audioPlayTransferSize))
#else
        if (g_deviceAudioComposite->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
#endif
        {
            g_deviceAudioComposite->audioUnified.speakerIntervalCount++;
            return;
        }
    }
    else /* full speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
    {
        if (g_deviceAudioComposite->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
        {
            g_deviceAudioComposite->audioUnified.speakerIntervalCount++;
            return;
        }
    }

    if (0U == g_deviceAudioComposite->audioUnified.firstCalculateFeedback)
    {
        g_deviceAudioComposite->audioUnified.speakerIntervalCount = 0;
        g_deviceAudioComposite->audioUnified.currentFrameCount    = 0;
        g_deviceAudioComposite->audioUnified.audioSendCount[0]    = 0;
        g_deviceAudioComposite->audioUnified.audioSendCount[1]    = 0;
        totalFrameValue                                           = 0;
        frameDistance                                             = 0;
        feedbackValue                                             = 0;
        USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID,
                                            (uint32_t *)&g_deviceAudioComposite->audioUnified.lastFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
        if (USB_SPEED_HIGH == g_deviceAudioComposite->speed)
        {
            g_deviceAudioComposite->audioUnified.lastFrameCount += s_microFrameCountIp3511HS;
        }
#endif
#endif
        g_deviceAudioComposite->audioUnified.firstCalculateFeedback = 1U;
        return;
    }
    g_deviceAudioComposite->audioUnified.speakerIntervalCount = 0;
    USB_DeviceClassGetCurrentFrameCount(CONTROLLER_ID,
                                        (uint32_t *)&g_deviceAudioComposite->audioUnified.currentFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    if (USB_SPEED_HIGH == g_deviceAudioComposite->speed)
    {
        g_deviceAudioComposite->audioUnified.currentFrameCount += s_microFrameCountIp3511HS;
    }
#endif
#endif
    frameDistance = ((g_deviceAudioComposite->audioUnified.currentFrameCount + USB_DEVICE_MAX_FRAME_COUNT + 1U -
                      g_deviceAudioComposite->audioUnified.lastFrameCount) &
                     USB_DEVICE_MAX_FRAME_COUNT);
    g_deviceAudioComposite->audioUnified.lastFrameCount = g_deviceAudioComposite->audioUnified.currentFrameCount;

    totalFrameValue += frameDistance;

    if (1U == g_deviceAudioComposite->audioUnified.stopFeedbackUpdate)
    {
        return;
    }

    if (1U == g_deviceAudioComposite->audioUnified.feedbackDiscardFlag)
    {
        if (0 != g_deviceAudioComposite->audioUnified.feedbackDiscardTimes)
        {
            g_deviceAudioComposite->audioUnified.feedbackDiscardTimes--;
            if (0 != g_deviceAudioComposite->audioUnified.lastFeedbackValue)
            {
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_deviceAudioComposite->audioUnified.lastFeedbackValue);
            }
            return;
        }
        else
        {
            g_deviceAudioComposite->audioUnified.feedbackDiscardFlag = 0;
        }
    }

    if (USB_SPEED_HIGH == g_deviceAudioComposite->speed)
    {
        feedbackValue = (uint32_t)((((((uint64_t)g_deviceAudioComposite->audioUnified.audioSendCount[1]) << 32U) |
                                     g_deviceAudioComposite->audioUnified.audioSendCount[0])) *
                                   1024UL * 8UL / totalFrameValue /
                                   ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }
    else
    {
        feedbackValue =
            (uint32_t)((((((uint64_t)g_deviceAudioComposite->audioUnified.audioSendCount[1]) << 32U) |
                         g_deviceAudioComposite->audioUnified.audioSendCount[0])) *
                       1024UL / totalFrameValue / ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }

    audioSpeakerUsedSpace = USB_AudioSpeakerBufferSpaceUsed();
    if (audioSpeakerUsedSpace <= (g_deviceAudioComposite->audioUnified.audioPlayTransferSize *
                                  USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD))
    {
        feedbackValue += AUDIO_ADJUST_MIN_STEP;
    }

    if ((audioSpeakerUsedSpace + (g_deviceAudioComposite->audioUnified.audioPlayTransferSize *
                                  USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD)) >=
        g_deviceAudioComposite->audioUnified.audioPlayBufferSize)
    {
        feedbackValue -= AUDIO_ADJUST_MIN_STEP;
    }

    g_deviceAudioComposite->audioUnified.lastFeedbackValue = feedbackValue;
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, feedbackValue);
}

/* The USB_RecorderDataMatch() function increase/decrease the adjusted packet interval according to the reserved
 * ringbuffer size */
uint32_t USB_RecorderDataMatch(uint32_t reservedspace)
{
    uint32_t epPacketSize = 0;
    if (reservedspace >=
        AUDIO_BUFFER_UPPER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_SIZE * AUDIO_IN_FORMAT_CHANNELS;
    }
    else if ((reservedspace >=
              AUDIO_BUFFER_LOWER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)) &&
             (reservedspace <
              AUDIO_BUFFER_UPPER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
    }
    else if (reservedspace <
             AUDIO_BUFFER_LOWER_LIMIT(AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE))
    {
        epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE - AUDIO_IN_FORMAT_SIZE * AUDIO_IN_FORMAT_CHANNELS;
    }
    else
    {
    }
    return epPacketSize;
}
#endif

/*!
 * @brief device Audio callback function.
 *
 * This function handle the Audio class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
usb_status_t USB_DeviceAudioCompositeCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param           = (usb_device_endpoint_callback_message_struct_t *)param;
    uint32_t epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;

    switch (event)
    {
        case kUSB_DeviceAudioEventStreamSendResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((g_deviceAudioComposite->audioUnified.attach) &&
                (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                if (0)
                {
                }
#else
                if (ep_cb_param->length == ((USB_SPEED_HIGH == g_composite.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                                                                    FS_ISO_FEEDBACK_ENDP_PACKET_SIZE))
                {
                    if (!feedbackValueUpdating)
                    {
                        *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
                    }
                    error =
                        USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioSpeakerHandle,
                                            USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                            (USB_SPEED_HIGH == g_composite.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                                                                    FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
                }
#endif
                else
                {
                    if (g_deviceAudioComposite->audioUnified.startRec == 0)
                    {
                        g_deviceAudioComposite->audioUnified.startRec = 1;
                    }
                    if ((g_deviceAudioComposite->audioUnified.tdWriteNumberRec >=
                         AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE / 2) &&
                        (g_deviceAudioComposite->audioUnified.startRecHalfFull == 0))
                    {
                        g_deviceAudioComposite->audioUnified.startRecHalfFull = 1;
                    }
                    if (g_deviceAudioComposite->audioUnified.startRecHalfFull)
                    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
                        USB_AudioRecorderGetBuffer(audioRecPacket, epPacketSize);
                        error = USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioRecorderHandle,
                                                    USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecPacket[0],
                                                    FS_ISO_IN_ENDP_PACKET_SIZE);
#else
                        USB_AUDIO_ENTER_CRITICAL();
                        epPacketSize = USB_RecorderDataMatch(USB_AudioRecorderBufferSpaceUsed());
                        USB_AUDIO_EXIT_CRITICAL();

                        USB_AudioRecorderGetBuffer(audioRecPacket, epPacketSize);

                        error =
                            USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioRecorderHandle,
                                                USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecPacket[0], epPacketSize);
#endif

                        g_deviceAudioComposite->audioUnified.usbSendTimes++;
                    }
                    else
                    {
                        error = USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioRecorderHandle,
                                                    USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecDataBuff[0],
                                                    FS_ISO_IN_ENDP_PACKET_SIZE);
                    }
                }
            }
            break;
        case kUSB_DeviceAudioEventStreamRecvResponse:
            /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
            if ((g_deviceAudioComposite->audioUnified.attach) &&
                (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
            {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
                if (USB_SPEED_HIGH ==
                    g_deviceAudioComposite->speed) /* high speed and audio 2.0, use low latency solution  */
                {
                    if (g_deviceAudioComposite->audioUnified.tdWriteNumberPlay >=
                        (g_deviceAudioComposite->audioUnified.audioPlayTransferSize *
                         AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT))
                    {
                        g_deviceAudioComposite->audioUnified.startPlayFlag = 1;
                    }
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if (1U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 7U)
                    {
                        s_microFrameCountIp3511HS++;
                    }
                    else
                    {
                        s_microFrameCountIp3511HS = 0U;
                    }
#elif (2U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 6U)
                    {
                        s_microFrameCountIp3511HS += 2U;
                    }
                    else
                    {
                        s_microFrameCountIp3511HS = 0U;
                    }
#elif (3U == HS_ISO_OUT_ENDP_INTERVAL)
                    if (s_microFrameCountIp3511HS < 4U)
                    {
                        s_microFrameCountIp3511HS += 4U;
                    }
                    else
                    {
                        s_microFrameCountIp3511HS = 0U;
                    }
#else
                    s_microFrameCountIp3511HS = 0;
#endif
#endif
                }
                else
                {
                    if ((g_deviceAudioComposite->audioUnified.tdWriteNumberPlay >=
                         (g_deviceAudioComposite->audioUnified.audioPlayBufferSize / 2U)) &&
                        (g_deviceAudioComposite->audioUnified.startPlayFlag == 0))
                    {
                        g_deviceAudioComposite->audioUnified.startPlayFlag = 1;
                    }
                }
#else
                if ((g_deviceAudioComposite->audioUnified.tdWriteNumberPlay >=
                     (g_deviceAudioComposite->audioUnified.audioPlayBufferSize / 2U)) &&
                    (g_deviceAudioComposite->audioUnified.startPlayFlag == 0))
                {
                    g_deviceAudioComposite->audioUnified.startPlayFlag = 1;
                }
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
#else
                if ((g_deviceAudioComposite->audioUnified.tdWriteNumberPlay >=
                     (g_deviceAudioComposite->audioUnified.audioPlayBufferSize / 2U)) &&
                    (g_deviceAudioComposite->audioUnified.startPlayFlag == 0))
                {
                    g_deviceAudioComposite->audioUnified.startPlayFlag = 1;
                }
#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */
                USB_AudioSpeakerPutBuffer(audioPlayPacket, ep_cb_param->length);
                g_deviceAudioComposite->audioUnified.usbRecvCount += ep_cb_param->length;
                g_deviceAudioComposite->audioUnified.usbRecvTimes++;
                error = USB_DeviceAudioRecv(handle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0],
                                            g_deviceAudioComposite->audioUnified.currentStreamOutMaxPacketSize);
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

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
void CTIMER_SOF_TOGGLE_HANDLER_FRO(uint32_t i)
{
    uint32_t currentCtCap = 0, pllCountPeriod = 0;
    uint32_t usedSpace      = 0;
    static int32_t pllCount = 0, pllDiff = 0;
    static int32_t err, abs_err;
    uint32_t up_change                 = 0;
    uint32_t down_change               = 0;
    static uint32_t delay_adj_up       = 0;
    static uint32_t delay_adj_down     = 0;
    static uint32_t FroPreUsbRecvCount = 0U;

    if (CTIMER_GetStatusFlags(CTIMER1) & (1 << 4U))
    {
        /* Clear interrupt flag.*/
        CTIMER_ClearStatusFlags(CTIMER1, (1 << 4U));
    }

    if (g_composite.audioUnified.froTrimIntervalCount != AUDIO_FRO_ADJUST_INTERVAL)
    {
        g_composite.audioUnified.froTrimIntervalCount++;
        return;
    }

    g_composite.audioUnified.froTrimIntervalCount = 1;
    currentCtCap                                  = CTIMER1->CR[0];
    pllCountPeriod                                = currentCtCap - g_composite.audioUnified.usbFroTicksPrev;
    g_composite.audioUnified.usbFroTicksPrev      = currentCtCap;
    pllCount                                      = pllCountPeriod;

    if (g_composite.audioUnified.attach)
    {
        if (abs(pllCount - AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT) < AUDIO_FRO_USB_SOF_INTERVAL_VALID_DEVIATION)
        {
            pllDiff = pllCount - g_composite.audioUnified.usbFroTicksEma;
            g_composite.audioUnified.usbFroTickEmaFrac += (pllDiff % 8);
            g_composite.audioUnified.usbFroTicksEma += (pllDiff / 8) + g_composite.audioUnified.usbFroTickEmaFrac / 8;
            g_composite.audioUnified.usbFroTickEmaFrac = (g_composite.audioUnified.usbFroTickEmaFrac % 8);

            err     = g_composite.audioUnified.usbFroTicksEma - AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
            abs_err = abs(err);
            if (abs_err > g_composite.audioUnified.usbFroTickBasedPrecision)
            {
                if (err > 0)
                {
                    down_change = 1;
                }
                else
                {
                    up_change = 1;
                }
            }

            if (g_composite.audioUnified.startPlayFlag)
            {
                /* if USB transfer stops, can not use data length to do adjustment */
                if (0U == g_composite.audioUnified.stopDataLengthAudioAdjust)
                {
                    /* USB is transferring */
                    if (FroPreUsbRecvCount != g_composite.audioUnified.usbRecvCount)
                    {
                        FroPreUsbRecvCount = g_composite.audioUnified.usbRecvCount;
                        usedSpace          = USB_AudioSpeakerBufferSpaceUsed();
                        if ((usedSpace + (g_composite.audioUnified.audioPlayTransferSize *
                                          AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD)) >=
                            g_composite.audioUnified.audioPlayBufferSize)
                        {
                            if (delay_adj_up == 0)
                            {
                                delay_adj_down = 0;
                                delay_adj_up   = AUDIO_FRO_TRIM_DATA_BASED_INTERVAL;
                                up_change      = 1;
                            }
                            else
                            {
                                delay_adj_up--;
                            }
                        }
                        else if (usedSpace <= (g_composite.audioUnified.audioPlayTransferSize *
                                               AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD))
                        {
                            if (delay_adj_down == 0)
                            {
                                delay_adj_up   = 0;
                                delay_adj_down = AUDIO_FRO_TRIM_DATA_BASED_INTERVAL;
                                down_change    = 1;
                            }
                            else
                            {
                                delay_adj_down--;
                            }
                        }
                        else
                        {
                            /* no action */
                        }
                    }
                }
            }
        }

        if (down_change)
        {
            audio_fro_trim_down();
        }
        if (up_change)
        {
            audio_fro_trim_up();
        }
    }
}
#endif /* USB_DEVICE_CONFIG_LPCIP3511FS */

void CTIMER_SOF_TOGGLE_HANDLER_PLL(uint32_t i)
{
    uint32_t currentCtCap = 0, pllCountPeriod = 0, pll_change = 0;
    uint32_t usedSpace      = 0;
    static int32_t pllCount = 0, pllDiff = 0;
    static int32_t err, abs_err;
    static uint32_t delay_adj_up       = 0;
    static uint32_t delay_adj_down     = 0;
    static uint32_t PllPreUsbRecvCount = 0U;

    if (CTIMER_GetStatusFlags(CTIMER0) & (1 << 4U))
    {
        /* Clear interrupt flag.*/
        CTIMER_ClearStatusFlags(CTIMER0, (1 << 4U));
    }

    if (g_composite.audioUnified.speakerIntervalCount != AUDIO_PLL_ADJUST_INTERVAL)
    {
        g_composite.audioUnified.speakerIntervalCount++;
        return;
    }

    g_composite.audioUnified.speakerIntervalCount = 1;
    currentCtCap                                  = CTIMER0->CR[0];
    pllCountPeriod                                = currentCtCap - g_composite.audioUnified.audioPllTicksPrev;
    g_composite.audioUnified.audioPllTicksPrev    = currentCtCap;
    pllCount                                      = pllCountPeriod;
    if (g_composite.audioUnified.attach)
    {
        if (abs(pllCount - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT) < AUDIO_PLL_USB_SOF_INTERVAL_VALID_DEVIATION)
        {
            pllDiff = pllCount - g_composite.audioUnified.audioPllTicksEma;
            g_composite.audioUnified.audioPllTickEmaFrac += (pllDiff % 8);
            g_composite.audioUnified.audioPllTicksEma +=
                (pllDiff / 8) + g_composite.audioUnified.audioPllTickEmaFrac / 8;
            g_composite.audioUnified.audioPllTickEmaFrac = (g_composite.audioUnified.audioPllTickEmaFrac % 8);

            err     = g_composite.audioUnified.audioPllTicksEma - AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
            abs_err = abs(err);
            if (abs_err > g_composite.audioUnified.audioPllTickBasedPrecision)
            {
                if (err > 0)
                {
                    g_composite.audioUnified.curAudioPllFrac -=
                        abs_err / g_composite.audioUnified.audioPllTickBasedPrecision;
                }
                else
                {
                    g_composite.audioUnified.curAudioPllFrac +=
                        abs_err / g_composite.audioUnified.audioPllTickBasedPrecision;
                }
                pll_change = 1;
            }

            if (0U != g_composite.audioUnified.startPlayFlag)
            {
                /* if USB transfer stops, can not use data length to do adjustment */
                if (0U == g_composite.audioUnified.stopDataLengthAudioAdjust)
                {
                    /* USB is transferring */
                    if (PllPreUsbRecvCount != g_composite.audioUnified.usbRecvCount)
                    {
                        PllPreUsbRecvCount = g_composite.audioUnified.usbRecvCount;
                        usedSpace          = USB_AudioSpeakerBufferSpaceUsed();
                        if (usedSpace <=
                            (g_composite.audioUnified.audioPlayTransferSize * AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD))
                        {
                            if (delay_adj_down == 0)
                            {
                                delay_adj_up   = 0;
                                delay_adj_down = AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL;
                                g_composite.audioUnified.curAudioPllFrac -= AUDIO_PLL_ADJUST_DATA_BASED_STEP;
                                pll_change = 1;
                            }
                            else
                            {
                                delay_adj_down--;
                            }
                        }
                        else if ((usedSpace + (g_composite.audioUnified.audioPlayTransferSize *
                                               AUDIO_SYNC_DATA_BASED_ADJUST_THRESHOLD)) >=
                                 g_composite.audioUnified.audioPlayBufferSize)
                        {
                            if (delay_adj_up == 0)
                            {
                                delay_adj_down = 0;
                                delay_adj_up   = AUDIO_PLL_ADJUST_DATA_BASED_INTERVAL;
                                g_composite.audioUnified.curAudioPllFrac += AUDIO_PLL_ADJUST_DATA_BASED_STEP;
                                pll_change = 1;
                            }
                            else
                            {
                                delay_adj_up--;
                            }
                        }
                    }
                }
            }
        }

        if (pll_change)
        {
            USB_AudioPllChange();
        }
    }
}
#endif /* USB_DEVICE_AUDIO_USE_SYNC_MODE */

/* The USB_DeviceAudioRecorderStatusReset() function resets the audio recorder status to the initialized status */
void USB_DeviceAudioRecorderStatusReset(void)
{
    g_deviceAudioComposite->audioUnified.startRec              = 0;
    g_deviceAudioComposite->audioUnified.startRecHalfFull      = 0;
    g_deviceAudioComposite->audioUnified.audioRecvCount        = 0;
    g_deviceAudioComposite->audioUnified.usbSendTimes          = 0;
    g_deviceAudioComposite->audioUnified.tdWriteNumberRec      = 0;
    g_deviceAudioComposite->audioUnified.tdReadNumberRec       = 0;
    g_deviceAudioComposite->audioUnified.recorderReservedSpace = 0;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void)
{
    g_deviceAudioComposite->audioUnified.startPlayFlag                 = 0;
    g_deviceAudioComposite->audioUnified.startPlayHalfFull             = 0;
    g_deviceAudioComposite->audioUnified.tdReadNumberPlay              = 0;
    g_deviceAudioComposite->audioUnified.tdWriteNumberPlay             = 0;
    g_deviceAudioComposite->audioUnified.audioSpeakerReadDataCount[0]  = 0;
    g_deviceAudioComposite->audioUnified.audioSpeakerReadDataCount[1]  = 0;
    g_deviceAudioComposite->audioUnified.audioSpeakerWriteDataCount[0] = 0;
    g_deviceAudioComposite->audioUnified.audioSpeakerWriteDataCount[1] = 0;
    g_deviceAudioComposite->audioUnified.audioSendCount[0]             = 0;
    g_deviceAudioComposite->audioUnified.audioSendCount[1]             = 0;
    g_deviceAudioComposite->audioUnified.usbRecvCount                  = 0;
    g_deviceAudioComposite->audioUnified.lastAudioSendCount            = 0;
    g_deviceAudioComposite->audioUnified.audioSendTimes                = 0;
    g_deviceAudioComposite->audioUnified.usbRecvTimes                  = 0;
    g_deviceAudioComposite->audioUnified.speakerIntervalCount          = 0;
    g_deviceAudioComposite->audioUnified.speakerReservedSpace          = 0;
    g_deviceAudioComposite->audioUnified.timesFeedbackCalculate        = 0;
    g_deviceAudioComposite->audioUnified.speakerDetachOrNoInput        = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    g_deviceAudioComposite->audioUnified.audioPllTicksPrev = 0U;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_deviceAudioComposite->audioUnified.usbFroTicksPrev          = 0U;
    g_deviceAudioComposite->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceAudioComposite->audioUnified.usbFroTickEmaFrac        = 0U;
    g_deviceAudioComposite->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_deviceAudioComposite->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceAudioComposite->audioUnified.audioPllTickEmaFrac        = 0U;
    g_deviceAudioComposite->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_deviceAudioComposite->audioUnified.stopDataLengthAudioAdjust  = 0U;
#else
    g_deviceAudioComposite->audioUnified.firstCalculateFeedback = 0;
    g_deviceAudioComposite->audioUnified.lastFrameCount         = 0;
    g_deviceAudioComposite->audioUnified.currentFrameCount      = 0;
    g_deviceAudioComposite->audioUnified.feedbackDiscardFlag    = 0U;
    g_deviceAudioComposite->audioUnified.feedbackDiscardTimes   = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;

    /* use the last saved feedback value */
    if (g_deviceAudioComposite->audioUnified.lastFeedbackValue)
    {
        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_deviceAudioComposite->audioUnified.lastFeedbackValue);
    }
#endif
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
usb_status_t USB_DeviceAudioCompositeSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceAudioComposite->audioUnified.attach = 1U;
    }
    return kStatus_USB_Success;
}

usb_status_t USB_DeviceAudioRecorderSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;

    if (alternateSetting == USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_1)
    {
        USB_DeviceAudioRecorderStatusReset();
        error =
            USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioRecorderHandle,
                                USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecDataBuff[0], FS_ISO_IN_ENDP_PACKET_SIZE);
    }
    return error;
}

usb_status_t USB_DeviceAudioSpeakerSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_status_t error = kStatus_USB_Success;

    if (USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 == alternateSetting)
    {
        error =
            USB_DeviceAudioRecv(g_deviceAudioComposite->audioUnified.audioSpeakerHandle,
                                USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayDataBuff[0], FS_ISO_OUT_ENDP_PACKET_SIZE);
        if (error != kStatus_USB_Success)
        {
            return error;
        }
        else
        {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
            if (!feedbackValueUpdating)
            {
                *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
            }
            error = USB_DeviceAudioSend(g_deviceAudioComposite->audioUnified.audioSpeakerHandle,
                                        USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                        (USB_SPEED_HIGH == g_composite.speed) ? HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                                                                FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
#endif
        }
    }
    return error;
}
/*!
 * @brief Audio init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceAudioCompositeInit(usb_device_composite_struct_t *device_composite)
{
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    CTIMER_CaptureInit();
#else
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif
    g_deviceAudioComposite                                       = device_composite;
    g_deviceAudioComposite->audioUnified.copyProtect             = 0x01U;
    g_deviceAudioComposite->audioUnified.curSpeakerMute          = 0x00U;
    g_deviceAudioComposite->audioUnified.curMicrophoneMute       = 0x00U;
    g_deviceAudioComposite->audioUnified.curSpeakerVolume[0]     = 0x00U;
    g_deviceAudioComposite->audioUnified.curSpeakerVolume[1]     = 0x1fU;
    g_deviceAudioComposite->audioUnified.curMicrophoneVolume[0]  = 0x00U;
    g_deviceAudioComposite->audioUnified.curMicrophoneVolume[1]  = 0x1fU;
    g_deviceAudioComposite->audioUnified.minVolume[0]            = 0x00U;
    g_deviceAudioComposite->audioUnified.minVolume[1]            = 0x00U;
    g_deviceAudioComposite->audioUnified.maxVolume[0]            = 0x00U;
    g_deviceAudioComposite->audioUnified.maxVolume[1]            = 0X43U;
    g_deviceAudioComposite->audioUnified.resVolume[0]            = 0x01U;
    g_deviceAudioComposite->audioUnified.resVolume[1]            = 0x00U;
    g_deviceAudioComposite->audioUnified.curBass                 = 0x00U;
    g_deviceAudioComposite->audioUnified.curBass                 = 0x00U;
    g_deviceAudioComposite->audioUnified.minBass                 = 0x80U;
    g_deviceAudioComposite->audioUnified.maxBass                 = 0x7FU;
    g_deviceAudioComposite->audioUnified.resBass                 = 0x01U;
    g_deviceAudioComposite->audioUnified.curMid                  = 0x00U;
    g_deviceAudioComposite->audioUnified.minMid                  = 0x80U;
    g_deviceAudioComposite->audioUnified.maxMid                  = 0x7FU;
    g_deviceAudioComposite->audioUnified.resMid                  = 0x01U;
    g_deviceAudioComposite->audioUnified.curTreble               = 0x01U;
    g_deviceAudioComposite->audioUnified.minTreble               = 0x80U;
    g_deviceAudioComposite->audioUnified.maxTreble               = 0x7FU;
    g_deviceAudioComposite->audioUnified.resTreble               = 0x01U;
    g_deviceAudioComposite->audioUnified.curAutomaticGain        = 0x01U;
    g_deviceAudioComposite->audioUnified.curDelay[0]             = 0x00U;
    g_deviceAudioComposite->audioUnified.curDelay[1]             = 0x40U;
    g_deviceAudioComposite->audioUnified.minDelay[0]             = 0x00U;
    g_deviceAudioComposite->audioUnified.minDelay[1]             = 0x00U;
    g_deviceAudioComposite->audioUnified.maxDelay[0]             = 0xFFU;
    g_deviceAudioComposite->audioUnified.maxDelay[1]             = 0xFFU;
    g_deviceAudioComposite->audioUnified.resDelay[0]             = 0x00U;
    g_deviceAudioComposite->audioUnified.resDelay[1]             = 0x01U;
    g_deviceAudioComposite->audioUnified.curLoudness             = 0x01U;
    g_deviceAudioComposite->audioUnified.curSamplingFrequency[0] = 0x00U;
    g_deviceAudioComposite->audioUnified.curSamplingFrequency[1] = 0x00U;
    g_deviceAudioComposite->audioUnified.curSamplingFrequency[2] = 0x01U;
    g_deviceAudioComposite->audioUnified.minSamplingFrequency[0] = 0x00U;
    g_deviceAudioComposite->audioUnified.minSamplingFrequency[1] = 0x00U;
    g_deviceAudioComposite->audioUnified.minSamplingFrequency[2] = 0x01U;
    g_deviceAudioComposite->audioUnified.maxSamplingFrequency[0] = 0x00U;
    g_deviceAudioComposite->audioUnified.maxSamplingFrequency[1] = 0x00U;
    g_deviceAudioComposite->audioUnified.maxSamplingFrequency[2] = 0x01U;
    g_deviceAudioComposite->audioUnified.resSamplingFrequency[0] = 0x00U;
    g_deviceAudioComposite->audioUnified.resSamplingFrequency[1] = 0x00U;
    g_deviceAudioComposite->audioUnified.resSamplingFrequency[2] = 0x01U;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
    g_deviceAudioComposite->audioUnified.curSpeakerMute20         = 0U;
    g_deviceAudioComposite->audioUnified.curMicrophoneMute20      = 0U;
    g_deviceAudioComposite->audioUnified.curClockValid            = 1U;
    g_deviceAudioComposite->audioUnified.curSpeakerVolume20[0]    = 0x00U;
    g_deviceAudioComposite->audioUnified.curSpeakerVolume20[1]    = 0x1FU;
    g_deviceAudioComposite->audioUnified.curMicrophoneVolume20[0] = 0x00U;
    g_deviceAudioComposite->audioUnified.curMicrophoneVolume20[1] = 0x1FU;
    g_deviceAudioComposite->audioUnified.curSampleFrequency       = 48000U;

    g_deviceAudioComposite->audioUnified.freqControlRange.wNumSubRanges = 1U;
    g_deviceAudioComposite->audioUnified.freqControlRange.wMIN          = 48000U;
    g_deviceAudioComposite->audioUnified.freqControlRange.wMAX          = 48000U;
    g_deviceAudioComposite->audioUnified.freqControlRange.wRES          = 0U;

    g_deviceAudioComposite->audioUnified.volumeControlRange.wNumSubRanges = 1U;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wMIN          = 0x8001U;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wMAX          = 0x7FFFU;
    g_deviceAudioComposite->audioUnified.volumeControlRange.wRES          = 1U;

#endif
    g_deviceAudioComposite->audioUnified.tdReadNumberPlay       = 0;
    g_deviceAudioComposite->audioUnified.tdWriteNumberPlay      = 0;
    g_deviceAudioComposite->audioUnified.tdWriteNumberRec       = 0;
    g_deviceAudioComposite->audioUnified.tdReadNumberRec        = 0;
    g_deviceAudioComposite->audioUnified.audioSendCount[0]      = 0;
    g_deviceAudioComposite->audioUnified.audioSendCount[1]      = 0;
    g_deviceAudioComposite->audioUnified.lastAudioSendCount     = 0;
    g_deviceAudioComposite->audioUnified.usbRecvCount           = 0;
    g_deviceAudioComposite->audioUnified.audioSendTimes         = 0;
    g_deviceAudioComposite->audioUnified.usbRecvTimes           = 0;
    g_deviceAudioComposite->audioUnified.audioRecvCount         = 0;
    g_deviceAudioComposite->audioUnified.usbSendTimes           = 0;
    g_deviceAudioComposite->audioUnified.startPlayFlag          = 0;
    g_deviceAudioComposite->audioUnified.startPlayHalfFull      = 0;
    g_deviceAudioComposite->audioUnified.startRec               = 0;
    g_deviceAudioComposite->audioUnified.startRecHalfFull       = 0;
    g_deviceAudioComposite->audioUnified.speakerIntervalCount   = 0;
    g_deviceAudioComposite->audioUnified.speakerReservedSpace   = 0;
    g_deviceAudioComposite->audioUnified.recorderReservedSpace  = 0;
    g_deviceAudioComposite->audioUnified.timesFeedbackCalculate = 0;
    g_deviceAudioComposite->audioUnified.speakerDetachOrNoInput = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_deviceAudioComposite->audioUnified.froTrimIntervalCount     = 0;
    g_deviceAudioComposite->audioUnified.usbFroTicksPrev          = 0;
    g_deviceAudioComposite->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceAudioComposite->audioUnified.usbFroTickEmaFrac        = 0;
    g_deviceAudioComposite->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_deviceAudioComposite->audioUnified.curAudioPllFrac            = AUDIO_PLL_FRACTIONAL_DIVIDER;
    g_deviceAudioComposite->audioUnified.audioPllTicksPrev          = 0;
    g_deviceAudioComposite->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceAudioComposite->audioUnified.audioPllTickEmaFrac        = 0;
    g_deviceAudioComposite->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_deviceAudioComposite->audioUnified.stopDataLengthAudioAdjust  = 0U;
#endif
    return kStatus_USB_Success;
}

void USB_AudioCodecTask(void)
{
    if (g_deviceAudioComposite->audioUnified.codecSpeakerTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curSpeakerMute20);
#else
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curSpeakerMute);
#endif
        BOARD_SetCodecMuteUnmute(true);
        g_deviceAudioComposite->audioUnified.codecSpeakerTask &= ~MUTE_CODEC_TASK;
        g_CodecSpeakerMuteUnmute = true;
    }
    if (g_deviceAudioComposite->audioUnified.codecMicrophoneTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMicrophoneMute20);
#else
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMicrophoneMute);
#endif
        /* here add code to set mute practically */
        g_deviceAudioComposite->audioUnified.codecMicrophoneTask &= ~MUTE_CODEC_TASK;
        g_CodecMicrophoneMuteUnmute = true;
    }
    if (g_deviceAudioComposite->audioUnified.codecSpeakerTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curSpeakerMute20);
#else
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curSpeakerMute);
#endif
        BOARD_SetCodecMuteUnmute(false);
        g_deviceAudioComposite->audioUnified.codecSpeakerTask &= ~UNMUTE_CODEC_TASK;
        g_CodecSpeakerMuteUnmute = false;
    }
    if (g_deviceAudioComposite->audioUnified.codecMicrophoneTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMicrophoneMute20);
#else
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceAudioComposite->audioUnified.curMicrophoneMute);
#endif
        /* here add code to set unmute practically */
        g_deviceAudioComposite->audioUnified.codecMicrophoneTask &= ~UNMUTE_CODEC_TASK;
        g_CodecMicrophoneMuteUnmute = false;
    }
    if (g_deviceAudioComposite->audioUnified.codecSpeakerTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceAudioComposite->audioUnified.curSpeakerVolume20[1] << 8U) |
                     g_deviceAudioComposite->audioUnified.curSpeakerVolume20[0]);
#else
        usb_echo("Set Speaker Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceAudioComposite->audioUnified.curSpeakerVolume[1] << 8U) |
                     g_deviceAudioComposite->audioUnified.curSpeakerVolume[0]);
#endif
        g_deviceAudioComposite->audioUnified.codecSpeakerTask &= ~VOLUME_CHANGE_TASK;
    }
    if (g_deviceAudioComposite->audioUnified.codecMicrophoneTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceAudioComposite->audioUnified.curMicrophoneVolume20[1] << 8U) |
                     g_deviceAudioComposite->audioUnified.curMicrophoneVolume20[0]);
#else
        usb_echo("Set Microphone Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceAudioComposite->audioUnified.curMicrophoneVolume[1] << 8U) |
                     g_deviceAudioComposite->audioUnified.curMicrophoneVolume[0]);
#endif
        g_deviceAudioComposite->audioUnified.codecMicrophoneTask &= ~VOLUME_CHANGE_TASK;
    }
}

void USB_AudioSpeakerResetTask(void)
{
    if (g_deviceAudioComposite->audioUnified.speakerDetachOrNoInput)
    {
        USB_DeviceAudioSpeakerStatusReset();
    }
}
