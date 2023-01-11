/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2019 NXP
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
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "audio_unified.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "composite.h"
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
volatile bool g_CodecSpeakerMuteUnmute    = false;
volatile bool g_CodecMicrophoneMuteUnmute = false;

uint8_t g_InterfaceIsSet = 0;

static usb_device_composite_struct_t *g_deviceComposite;
extern usb_device_composite_struct_t g_composite;

usb_status_t USB_DeviceAudioProcessTerminalRequest(uint32_t audioCommand,
                                                   uint32_t *length,
                                                   uint8_t **buffer,
                                                   uint8_t entityOrEndpoint);

/*******************************************************************************
 * Code
 ******************************************************************************/

/* The USB_AudioSpeakerBufferSpaceUsed() function gets the used speaker ringbuffer size */
uint32_t USB_AudioSpeakerBufferSpaceUsed(void)
{
    uint64_t write_count = 0U;
    uint64_t read_count  = 0U;

    write_count = (uint64_t)((((uint64_t)g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[1]) << 32U) |
                             g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[0]);
    read_count  = (uint64_t)((((uint64_t)g_deviceComposite->audioUnified.audioSpeakerReadDataCount[1]) << 32U) |
                            g_deviceComposite->audioUnified.audioSpeakerReadDataCount[0]);

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
    if (g_deviceComposite->audioUnified.tdWriteNumberRec > g_deviceComposite->audioUnified.tdReadNumberRec)
    {
        g_deviceComposite->audioUnified.recorderReservedSpace =
            g_deviceComposite->audioUnified.tdWriteNumberRec - g_deviceComposite->audioUnified.tdReadNumberRec;
    }
    else
    {
        g_deviceComposite->audioUnified.recorderReservedSpace =
            g_deviceComposite->audioUnified.tdWriteNumberRec +
            AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE -
            g_deviceComposite->audioUnified.tdReadNumberRec;
    }
    return g_deviceComposite->audioUnified.recorderReservedSpace;
}

/* The USB_AudioRecorderGetBuffer() function gets audioRecPacket from the audioRecDataBuff in every callback*/
void USB_AudioRecorderGetBuffer(uint8_t *buffer, uint32_t size)
{
    while (size)
    {
        *buffer = audioRecDataBuff[g_deviceComposite->audioUnified.tdReadNumberRec];
        g_deviceComposite->audioUnified.tdReadNumberRec++;
        buffer++;
        size--;

        if (g_deviceComposite->audioUnified.tdReadNumberRec >=
            AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE)
        {
            g_deviceComposite->audioUnified.tdReadNumberRec = 0;
        }
    }
}

/* The USB_AudioSpeakerPutBuffer() function fills the audioRecDataBuff with audioPlayPacket in every callback*/
void USB_AudioSpeakerPutBuffer(uint8_t *buffer, uint32_t size)
{
    uint32_t audioSpeakerPreWriteDataCount = 0U;
    uint32_t remainBufferSpace;
    uint32_t buffer_length = 0;

    remainBufferSpace = g_deviceComposite->audioUnified.audioPlayBufferSize - USB_AudioSpeakerBufferSpaceUsed();
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
        buffer_length = g_deviceComposite->audioUnified.tdWriteNumberPlay + size;
        if (buffer_length < g_deviceComposite->audioUnified.audioPlayBufferSize)
        {
            memcpy((void *)(&audioPlayDataBuff[g_deviceComposite->audioUnified.tdWriteNumberPlay]),
                   (void *)(&buffer[0]), size);
            g_deviceComposite->audioUnified.tdWriteNumberPlay += size;
        }
        else
        {
            uint32_t firstLength =
                g_deviceComposite->audioUnified.audioPlayBufferSize - g_deviceComposite->audioUnified.tdWriteNumberPlay;
            memcpy((void *)(&audioPlayDataBuff[g_deviceComposite->audioUnified.tdWriteNumberPlay]),
                   (void *)(&buffer[0]), firstLength);
            buffer_length = size - firstLength; /* the remain data length */
            if ((buffer_length) > 0U)
            {
                memcpy((void *)(&audioPlayDataBuff[0]), (void *)((uint8_t *)(&buffer[0]) + firstLength), buffer_length);
                g_deviceComposite->audioUnified.tdWriteNumberPlay = buffer_length;
            }
            else
            {
                g_deviceComposite->audioUnified.tdWriteNumberPlay = 0;
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
    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) /* high speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
        if (g_deviceComposite->audioUnified.speakerIntervalCount !=
            AUDIO_CALCULATE_Ff_INTERVAL *
                (AUDIO_PLAY_BUFFER_SIZE_ONE_FRAME / g_deviceComposite->audioUnified.audioPlayTransferSize))
#else
        if (g_deviceComposite->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
#endif
        {
            g_deviceComposite->audioUnified.speakerIntervalCount++;
            return;
        }
    }
    else /* full speed, feedback interval is AUDIO_CALCULATE_Ff_INTERVAL ms */
    {
        if (g_deviceComposite->audioUnified.speakerIntervalCount != AUDIO_CALCULATE_Ff_INTERVAL)
        {
            g_deviceComposite->audioUnified.speakerIntervalCount++;
            return;
        }
    }

    if (0U == g_deviceComposite->audioUnified.firstCalculateFeedback)
    {
        g_deviceComposite->audioUnified.speakerIntervalCount = 0;
        g_deviceComposite->audioUnified.currentFrameCount    = 0;
        g_deviceComposite->audioUnified.audioSendCount[0]    = 0;
        g_deviceComposite->audioUnified.audioSendCount[1]    = 0;
        totalFrameValue                                      = 0;
        frameDistance                                        = 0;
        feedbackValue                                        = 0;
        USB_DeviceGetStatus(g_deviceComposite->deviceHandle, kUSB_DeviceStatusGetCurrentFrameCount,
                            (uint32_t *)&g_deviceComposite->audioUnified.lastFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
        if (USB_SPEED_HIGH == g_deviceComposite->speed)
        {
            g_deviceComposite->audioUnified.lastFrameCount += s_microFrameCountIp3511HS;
        }
#endif
#endif
        g_deviceComposite->audioUnified.firstCalculateFeedback = 1U;
        return;
    }
    g_deviceComposite->audioUnified.speakerIntervalCount = 0;
    USB_DeviceGetStatus(g_deviceComposite->deviceHandle, kUSB_DeviceStatusGetCurrentFrameCount,
                        (uint32_t *)&g_deviceComposite->audioUnified.currentFrameCount);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
        g_deviceComposite->audioUnified.currentFrameCount += s_microFrameCountIp3511HS;
    }
#endif
#endif
    frameDistance = ((g_deviceComposite->audioUnified.currentFrameCount + USB_DEVICE_MAX_FRAME_COUNT + 1U -
                      g_deviceComposite->audioUnified.lastFrameCount) &
                     USB_DEVICE_MAX_FRAME_COUNT);
    g_deviceComposite->audioUnified.lastFrameCount = g_deviceComposite->audioUnified.currentFrameCount;

    totalFrameValue += frameDistance;

    if (1U == g_deviceComposite->audioUnified.stopFeedbackUpdate)
    {
        return;
    }

    if (1U == g_deviceComposite->audioUnified.feedbackDiscardFlag)
    {
        if (0 != g_deviceComposite->audioUnified.feedbackDiscardTimes)
        {
            g_deviceComposite->audioUnified.feedbackDiscardTimes--;
            if (0 != g_deviceComposite->audioUnified.lastFeedbackValue)
            {
                AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_deviceComposite->audioUnified.lastFeedbackValue);
            }
            return;
        }
        else
        {
            g_deviceComposite->audioUnified.feedbackDiscardFlag = 0;
        }
    }

    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
        feedbackValue = (uint32_t)((((((uint64_t)g_deviceComposite->audioUnified.audioSendCount[1]) << 32U) |
                                     g_deviceComposite->audioUnified.audioSendCount[0])) *
                                   1024UL * 8UL / totalFrameValue /
                                   ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }
    else
    {
        feedbackValue =
            (uint32_t)((((((uint64_t)g_deviceComposite->audioUnified.audioSendCount[1]) << 32U) |
                         g_deviceComposite->audioUnified.audioSendCount[0])) *
                       1024UL / totalFrameValue / ((uint64_t)(AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)));
    }

    audioSpeakerUsedSpace = USB_AudioSpeakerBufferSpaceUsed();
    if (audioSpeakerUsedSpace <=
        (g_deviceComposite->audioUnified.audioPlayTransferSize * USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD))
    {
        feedbackValue += AUDIO_ADJUST_MIN_STEP;
    }

    if ((audioSpeakerUsedSpace + (g_deviceComposite->audioUnified.audioPlayTransferSize *
                                  USB_AUDIO_PLAY_BUFFER_FEEDBACK_TOLERANCE_THRESHOLD)) >=
        g_deviceComposite->audioUnified.audioPlayBufferSize)
    {
        feedbackValue -= AUDIO_ADJUST_MIN_STEP;
    }

    g_deviceComposite->audioUnified.lastFeedbackValue = feedbackValue;
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
 * @brief Audio wav data prepare function.
 *
 * This function prepare audio wav data before send.
 */
/* USB device audio ISO OUT endpoint callback */
usb_status_t USB_DeviceAudioIsoIN(usb_device_handle deviceHandle,
                                  usb_device_endpoint_callback_message_struct_t *event,
                                  void *arg)
{
    usb_status_t error = kStatus_USB_Error;

    uint32_t epPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    if (0)
    {
    }
#else
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)event;
    if ((g_deviceComposite->audioUnified.attach) &&
        (ep_cb_param->length == ((USB_SPEED_HIGH == g_deviceComposite->audioUnified.speed) ?
                                     HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                     FS_ISO_FEEDBACK_ENDP_PACKET_SIZE)))
    {
        if (!feedbackValueUpdating)
        {
            *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
        }
        error = USB_DeviceSendRequest(deviceHandle, USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                      (USB_SPEED_HIGH == g_deviceComposite->audioUnified.speed) ?
                                          HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                          FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
    }
#endif
    else
    {
        if (g_deviceComposite->audioUnified.startRec == 0)
        {
            g_deviceComposite->audioUnified.startRec = 1;
        }
        if ((g_deviceComposite->audioUnified.tdWriteNumberRec >=
             AUDIO_RECORDER_DATA_WHOLE_BUFFER_COUNT_NORMAL * FS_ISO_IN_ENDP_PACKET_SIZE / 2) &&
            (g_deviceComposite->audioUnified.startRecHalfFull == 0))
        {
            g_deviceComposite->audioUnified.startRecHalfFull = 1;
        }
        if (g_deviceComposite->audioUnified.startRecHalfFull)
        {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
            USB_AudioRecorderGetBuffer(audioRecPacket, epPacketSize);
            error = USB_DeviceSendRequest(deviceHandle, USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecPacket[0],
                                          epPacketSize);
#else
            USB_AUDIO_ENTER_CRITICAL();
            epPacketSize = USB_RecorderDataMatch(USB_AudioRecorderBufferSpaceUsed());
            USB_AUDIO_EXIT_CRITICAL();

            USB_AudioRecorderGetBuffer(audioRecPacket, epPacketSize);

            error = USB_DeviceSendRequest(deviceHandle, USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecPacket[0],
                                          epPacketSize);
#endif

            g_deviceComposite->audioUnified.usbSendTimes++;
        }
        else
        {
            error = USB_DeviceSendRequest(deviceHandle, USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecDataBuff[0],
                                          FS_ISO_IN_ENDP_PACKET_SIZE);
        }
    }

    return error;
}

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
    usb_status_t error = kStatus_USB_InvalidRequest;
    usb_device_endpoint_callback_message_struct_t *ep_cb_param;
    ep_cb_param = (usb_device_endpoint_callback_message_struct_t *)event;

    /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
    if ((g_deviceComposite->audioUnified.attach) && (ep_cb_param->length != (USB_CANCELLED_TRANSFER_LENGTH)))
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
        if (USB_SPEED_HIGH ==
            g_deviceComposite->audioUnified.speed) /* high speed and audio 2.0, use low latency solution  */
        {
            if (g_deviceComposite->audioUnified.tdWriteNumberPlay >=
                (g_deviceComposite->audioUnified.audioPlayTransferSize * AUDIO_CLASS_2_0_HS_LOW_LATENCY_TRANSFER_COUNT))
            {
                g_deviceComposite->audioUnified.startPlayFlag = 1;
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
            if ((g_deviceComposite->audioUnified.tdWriteNumberPlay >=
                 (g_deviceComposite->audioUnified.audioPlayBufferSize / 2U)) &&
                (g_deviceComposite->audioUnified.startPlayFlag == 0))
            {
                g_deviceComposite->audioUnified.startPlayFlag = 1;
            }
        }
#else
        if ((g_deviceComposite->audioUnified.tdWriteNumberPlay >=
             (g_deviceComposite->audioUnified.audioPlayBufferSize / 2U)) &&
            (g_deviceComposite->audioUnified.startPlayFlag == 0))
        {
            g_deviceComposite->audioUnified.startPlayFlag = 1;
        }
#endif /* USB_DEVICE_CONFIG_EHCI, USB_DEVICE_CONFIG_LPCIP3511HS */
#else
        if ((g_deviceComposite->audioUnified.tdWriteNumberPlay >=
             (g_deviceComposite->audioUnified.audioPlayBufferSize / 2U)) &&
            (g_deviceComposite->audioUnified.startPlayFlag == 0))
        {
            g_deviceComposite->audioUnified.startPlayFlag = 1;
        }
#endif /* USB_DEVICE_CONFIG_AUDIO_CLASS_2_0 */
        USB_AudioSpeakerPutBuffer(audioPlayPacket, ep_cb_param->length);
        g_deviceComposite->audioUnified.usbRecvCount += ep_cb_param->length;
        error = USB_DeviceRecvRequest(deviceHandle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayPacket[0],
                                      g_deviceComposite->audioUnified.currentStreamOutMaxPacketSize);
    }
    return error;
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
                    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
                        (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId))
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
                    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
                        (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId))
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
                    if ((USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId) ||
                        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
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
                    if ((USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId) ||
                        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
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
    uint8_t entityId   = (uint8_t)(setup->wIndex >> 0x08);

    if ((USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioSetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if ((USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId) ||
             (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityId))
    {
        error = USB_DeviceAudioSetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
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
        (USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID == entityId) ||
        (USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID == entityId))
    {
        error = USB_DeviceAudioGetControlTerminal(handle, setup, length, buffer, entityId);
    }
    else if ((USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityId) ||
             (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityId))
    {
        error = USB_DeviceAudioGetFeatureUnit(handle, setup, length, buffer);
    }
    else
    {
    }
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    if (USB_AUDIO_CONTROL_CLOCK_SOURCE_ENTITY_ID == entityId)
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

usb_status_t USB_DeviceAudioUnifiedClassRequest(usb_device_handle handle,
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
    uint8_t *volBuffAddr;
#if (!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
    uint16_t volume;
#endif

    switch (audioCommand)
    {
        case USB_DEVICE_AUDIO_FU_GET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curSpeakerMute20;
                *length = sizeof(g_deviceComposite->audioUnified.curSpeakerMute20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curMicrophoneMute20;
                *length = sizeof(g_deviceComposite->audioUnified.curMicrophoneMute20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = &g_deviceComposite->audioUnified.curSpeakerMute;
                *length = sizeof(g_deviceComposite->audioUnified.curSpeakerMute);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = &g_deviceComposite->audioUnified.curMicrophoneMute;
                *length = sizeof(g_deviceComposite->audioUnified.curMicrophoneMute);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curSpeakerVolume20;
                *length = sizeof(g_deviceComposite->audioUnified.curSpeakerVolume20);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curMicrophoneVolume20;
                *length = sizeof(g_deviceComposite->audioUnified.curMicrophoneVolume20);
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = g_deviceComposite->audioUnified.curSpeakerVolume;
                *length = sizeof(g_deviceComposite->audioUnified.curSpeakerVolume);
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                *buffer = g_deviceComposite->audioUnified.curMicrophoneVolume;
                *length = sizeof(g_deviceComposite->audioUnified.curMicrophoneVolume);
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.curBass;
            *length = sizeof(g_deviceComposite->audioUnified.curBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_MID_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.curMid;
            *length = sizeof(g_deviceComposite->audioUnified.curMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.curTreble;
            *length = sizeof(g_deviceComposite->audioUnified.curTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_AUTOMATIC_GAIN_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.curAutomaticGain;
            *length = sizeof(g_deviceComposite->audioUnified.curAutomaticGain);
            break;
        case USB_DEVICE_AUDIO_FU_GET_CUR_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioUnified.curDelay;
            *length = sizeof(g_deviceComposite->audioUnified.curDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioUnified.minVolume;
            *length = sizeof(g_deviceComposite->audioUnified.minVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.minBass;
            *length = sizeof(g_deviceComposite->audioUnified.minBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_MID_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.minMid;
            *length = sizeof(g_deviceComposite->audioUnified.minMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.minTreble;
            *length = sizeof(g_deviceComposite->audioUnified.minTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MIN_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioUnified.minDelay;
            *length = sizeof(g_deviceComposite->audioUnified.minDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioUnified.maxVolume;
            *length = sizeof(g_deviceComposite->audioUnified.maxVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.maxBass;
            *length = sizeof(g_deviceComposite->audioUnified.maxBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_MID_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.maxMid;
            *length = sizeof(g_deviceComposite->audioUnified.maxMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.maxTreble;
            *length = sizeof(g_deviceComposite->audioUnified.maxTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_MAX_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioUnified.maxDelay;
            *length = sizeof(g_deviceComposite->audioUnified.maxDelay);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_VOLUME_CONTROL:
            *buffer = g_deviceComposite->audioUnified.resVolume;
            *length = sizeof(g_deviceComposite->audioUnified.resVolume);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_BASS_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.resBass;
            *length = sizeof(g_deviceComposite->audioUnified.resBass);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_MID_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.resMid;
            *length = sizeof(g_deviceComposite->audioUnified.resMid);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_TREBLE_CONTROL:
            *buffer = &g_deviceComposite->audioUnified.resTreble;
            *length = sizeof(g_deviceComposite->audioUnified.resTreble);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RES_DELAY_CONTROL:
            *buffer = g_deviceComposite->audioUnified.resDelay;
            *length = sizeof(g_deviceComposite->audioUnified.resDelay);
            break;
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        case USB_DEVICE_AUDIO_CS_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curSampleFrequency;
            *length = sizeof(g_deviceComposite->audioUnified.curSampleFrequency);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioUnified.curSampleFrequency = *(uint32_t *)(*buffer);
            break;
        case USB_DEVICE_AUDIO_CS_GET_CUR_CLOCK_VALID_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioUnified.curClockValid;
            *length = sizeof(g_deviceComposite->audioUnified.curClockValid);
            break;
        case USB_DEVICE_AUDIO_CS_SET_CUR_CLOCK_VALID_CONTROL:
            g_deviceComposite->audioUnified.curClockValid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_GET_RANGE_VOLUME_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioUnified.volumeControlRange;
            *length = sizeof(g_deviceComposite->audioUnified.volumeControlRange);
            break;
        case USB_DEVICE_AUDIO_CS_GET_RANGE_SAMPLING_FREQ_CONTROL:
            *buffer = (uint8_t *)&g_deviceComposite->audioUnified.freqControlRange;
            *length = sizeof(g_deviceComposite->audioUnified.freqControlRange);
            break;
#else
        case USB_DEVICE_AUDIO_EP_GET_CUR_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioUnified.curSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioUnified.curSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MIN_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioUnified.minSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioUnified.minSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_MAX_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioUnified.maxSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioUnified.maxSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_GET_RES_SAMPLING_FREQ_CONTROL:
            *buffer = g_deviceComposite->audioUnified.resSamplingFrequency;
            *length = sizeof(g_deviceComposite->audioUnified.resSamplingFrequency);
            break;
        case USB_DEVICE_AUDIO_EP_SET_CUR_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioUnified.curSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioUnified.curSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MIN_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioUnified.minSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioUnified.minSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_MAX_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioUnified.maxSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioUnified.maxSamplingFrequency[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_EP_SET_RES_SAMPLING_FREQ_CONTROL:
            g_deviceComposite->audioUnified.resSamplingFrequency[0] = **(buffer);
            g_deviceComposite->audioUnified.resSamplingFrequency[1] = *((*buffer) + 1);
            break;
#endif
        case USB_DEVICE_AUDIO_FU_SET_CUR_VOLUME_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                volBuffAddr                                           = *buffer;
                g_deviceComposite->audioUnified.curSpeakerVolume20[0] = *(volBuffAddr);
                g_deviceComposite->audioUnified.curSpeakerVolume20[1] = *(volBuffAddr + 1);
                g_deviceComposite->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                volBuffAddr                                              = *buffer;
                g_deviceComposite->audioUnified.curMicrophoneVolume20[0] = *(volBuffAddr);
                g_deviceComposite->audioUnified.curMicrophoneVolume20[1] = *(volBuffAddr + 1);
                g_deviceComposite->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                volBuffAddr                                         = *buffer;
                g_deviceComposite->audioUnified.curSpeakerVolume[0] = *volBuffAddr;
                g_deviceComposite->audioUnified.curSpeakerVolume[1] = *(volBuffAddr + 1);
                volume = (uint16_t)((uint16_t)g_deviceComposite->audioUnified.curSpeakerVolume[1] << 8U);
                volume |= (uint8_t)(g_deviceComposite->audioUnified.curSpeakerVolume[0]);
                g_deviceComposite->audioUnified.codecSpeakerTask |= VOLUME_CHANGE_TASK;
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                volBuffAddr                                            = *buffer;
                g_deviceComposite->audioUnified.curMicrophoneVolume[0] = *volBuffAddr;
                g_deviceComposite->audioUnified.curMicrophoneVolume[1] = *(volBuffAddr + 1);
                volume = (uint16_t)((uint16_t)g_deviceComposite->audioUnified.curMicrophoneVolume[1] << 8U);
                volume |= (uint8_t)(g_deviceComposite->audioUnified.curMicrophoneVolume[0]);
                g_deviceComposite->audioUnified.codecMicrophoneTask |= VOLUME_CHANGE_TASK;
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MUTE_CONTROL:
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                g_deviceComposite->audioUnified.curSpeakerMute20 = **(buffer);
                if (g_deviceComposite->audioUnified.curSpeakerMute20)
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                }
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                g_deviceComposite->audioUnified.curMicrophoneMute20 = **(buffer);
                if (g_deviceComposite->audioUnified.curMicrophoneMute20)
                {
                    g_deviceComposite->audioUnified.codecMicrophoneTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceComposite->audioUnified.codecMicrophoneTask |= UNMUTE_CODEC_TASK;
                }
            }
            else
            {
                /* no action */
            }
#else
            if (USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                g_deviceComposite->audioUnified.curSpeakerMute = **(buffer);
                if (g_deviceComposite->audioUnified.curSpeakerMute)
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                }
            }
            else if (USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID == entityOrEndpoint)
            {
                g_deviceComposite->audioUnified.curSpeakerMute = **(buffer);
                if (g_deviceComposite->audioUnified.curSpeakerMute)
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= MUTE_CODEC_TASK;
                }
                else
                {
                    g_deviceComposite->audioUnified.codecSpeakerTask |= UNMUTE_CODEC_TASK;
                }
            }
            else
            {
                /* no action */
            }
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_BASS_CONTROL:
            g_deviceComposite->audioUnified.curBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_MID_CONTROL:
            g_deviceComposite->audioUnified.curMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_TREBLE_CONTROL:
            g_deviceComposite->audioUnified.curTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_AUTOMATIC_GAIN_CONTROL:
            g_deviceComposite->audioUnified.curAutomaticGain = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_CUR_DELAY_CONTROL:
            g_deviceComposite->audioUnified.curDelay[0] = **(buffer);
            g_deviceComposite->audioUnified.curDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioUnified.curDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioUnified.curDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_VOLUME_CONTROL:
            g_deviceComposite->audioUnified.minVolume[0] = **(buffer);
            g_deviceComposite->audioUnified.minVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_BASS_CONTROL:
            g_deviceComposite->audioUnified.minBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_MID_CONTROL:
            g_deviceComposite->audioUnified.minMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_TREBLE_CONTROL:
            g_deviceComposite->audioUnified.minTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MIN_DELAY_CONTROL:
            g_deviceComposite->audioUnified.minDelay[0] = **(buffer);
            g_deviceComposite->audioUnified.minDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioUnified.minDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioUnified.minDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_VOLUME_CONTROL:
            g_deviceComposite->audioUnified.maxVolume[0] = **(buffer);
            g_deviceComposite->audioUnified.maxVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_BASS_CONTROL:
            g_deviceComposite->audioUnified.maxBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_MID_CONTROL:
            g_deviceComposite->audioUnified.maxMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_TREBLE_CONTROL:
            g_deviceComposite->audioUnified.maxTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_MAX_DELAY_CONTROL:
            g_deviceComposite->audioUnified.maxDelay[0] = **(buffer);
            g_deviceComposite->audioUnified.maxDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioUnified.maxDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioUnified.maxDelay[3] = *((*buffer) + 3);
#endif
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_VOLUME_CONTROL:
            g_deviceComposite->audioUnified.resVolume[0] = **(buffer);
            g_deviceComposite->audioUnified.resVolume[1] = *((*buffer) + 1);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_BASS_CONTROL:
            g_deviceComposite->audioUnified.resBass = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_MID_CONTROL:
            g_deviceComposite->audioUnified.resMid = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_TREBLE_CONTROL:
            g_deviceComposite->audioUnified.resTreble = **(buffer);
            break;
        case USB_DEVICE_AUDIO_FU_SET_RES_DELAY_CONTROL:
            g_deviceComposite->audioUnified.resDelay[0] = **(buffer);
            g_deviceComposite->audioUnified.resDelay[1] = *((*buffer) + 1);
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
            g_deviceComposite->audioUnified.resDelay[2] = *((*buffer) + 2);
            g_deviceComposite->audioUnified.resDelay[3] = *((*buffer) + 3);
#endif
            break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }
    return error;
}

usb_status_t USB_DeviceAudioUnifiedConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    usb_status_t error = kStatus_USB_Error;
    if (status)
    {
        if ((USB_AUDIO_RECORDER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (!(ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK)))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
        else if ((USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
#endif
        else
        {
        }
    }
    else
    {
        if ((USB_AUDIO_RECORDER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_CONTROL_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_AUDIO_SPEAKER_STREAM_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (!(ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK)))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
        else if ((USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 (ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
#endif
        else
        {
        }
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
    g_deviceComposite->audioUnified.startRec              = 0;
    g_deviceComposite->audioUnified.startRecHalfFull      = 0;
    g_deviceComposite->audioUnified.audioRecvCount        = 0;
    g_deviceComposite->audioUnified.usbSendTimes          = 0;
    g_deviceComposite->audioUnified.tdWriteNumberRec      = 0;
    g_deviceComposite->audioUnified.tdReadNumberRec       = 0;
    g_deviceComposite->audioUnified.recorderReservedSpace = 0;
}

/* The USB_DeviceAudioSpeakerStatusReset() function resets the audio speaker status to the initialized status */
void USB_DeviceAudioSpeakerStatusReset(void)
{
    g_deviceComposite->audioUnified.startPlayFlag                 = 0;
    g_deviceComposite->audioUnified.startPlayHalfFull             = 0;
    g_deviceComposite->audioUnified.tdReadNumberPlay              = 0;
    g_deviceComposite->audioUnified.tdWriteNumberPlay             = 0;
    g_deviceComposite->audioUnified.audioSpeakerReadDataCount[0]  = 0;
    g_deviceComposite->audioUnified.audioSpeakerReadDataCount[1]  = 0;
    g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[0] = 0;
    g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[1] = 0;
    g_deviceComposite->audioUnified.audioSendCount[0]             = 0;
    g_deviceComposite->audioUnified.audioSendCount[1]             = 0;
    g_deviceComposite->audioUnified.usbRecvCount                  = 0;
    g_deviceComposite->audioUnified.audioSendTimes                = 0;
    g_deviceComposite->audioUnified.usbRecvTimes                  = 0;
    g_deviceComposite->audioUnified.speakerIntervalCount          = 0;
    g_deviceComposite->audioUnified.speakerReservedSpace          = 0;
    g_deviceComposite->audioUnified.timesFeedbackCalculate        = 0;
    g_deviceComposite->audioUnified.speakerDetachOrNoInput        = 0;
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    g_deviceComposite->audioUnified.audioPllTicksPrev = 0U;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_deviceComposite->audioUnified.usbFroTicksPrev          = 0U;
    g_deviceComposite->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceComposite->audioUnified.usbFroTickEmaFrac        = 0U;
    g_deviceComposite->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_deviceComposite->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceComposite->audioUnified.audioPllTickEmaFrac        = 0U;
    g_deviceComposite->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_deviceComposite->audioUnified.stopDataLengthAudioAdjust  = 0U;
#else
    g_deviceComposite->audioUnified.firstCalculateFeedback = 0;
    g_deviceComposite->audioUnified.lastFrameCount         = 0;
    g_deviceComposite->audioUnified.currentFrameCount      = 0;
    g_deviceComposite->audioUnified.feedbackDiscardFlag    = 0U;
    g_deviceComposite->audioUnified.feedbackDiscardTimes   = AUDIO_SPEAKER_FEEDBACK_DISCARD_COUNT;

    /* use the last saved feedback value */
    if (g_deviceComposite->audioUnified.lastFeedbackValue)
    {
        AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, g_deviceComposite->audioUnified.lastFeedbackValue);
    }
#endif
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
usb_status_t USB_DeviceAudioUnifiedSetConfigure(usb_device_handle handle, uint8_t configure)
{
    usb_status_t error = kStatus_USB_Success;
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        g_deviceComposite->audioUnified.attach = 1U;
    }
    return error;
}

usb_status_t USB_DeviceAudioRecorderSetInterface(usb_device_handle handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;
    usb_status_t error = kStatus_USB_Error;

    epCallback.callbackFn    = USB_DeviceAudioIsoIN;
    epCallback.callbackParam = handle;

    epInitStruct.zlt          = 0U;
    epInitStruct.transferType = USB_ENDPOINT_ISOCHRONOUS;
    epInitStruct.endpointAddress =
        USB_AUDIO_RECORDER_STREAM_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
        epInitStruct.maxPacketSize = HS_ISO_IN_ENDP_PACKET_SIZE;
        epInitStruct.interval      = HS_ISO_IN_ENDP_INTERVAL;
#else
        epInitStruct.maxPacketSize = HS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE;
        epInitStruct.interval      = HS_ISO_IN_ENDP_INTERVAL;
#endif
    }
    else
    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
        epInitStruct.maxPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE;
        epInitStruct.interval      = FS_ISO_IN_ENDP_INTERVAL;
#else
        epInitStruct.maxPacketSize = FS_ISO_IN_ENDP_PACKET_SIZE + AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE;
        epInitStruct.interval      = FS_ISO_IN_ENDP_INTERVAL;
#endif
    }

    USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);
    error =
        USB_DeviceSendRequest(handle, USB_AUDIO_RECORDER_STREAM_ENDPOINT, &audioRecDataBuff[0],
                              (USB_SPEED_HIGH == g_deviceComposite->audioUnified.speed) ? (HS_ISO_IN_ENDP_PACKET_SIZE) :
                                                                                          (FS_ISO_IN_ENDP_PACKET_SIZE));
    return error;
}

usb_status_t USB_DeviceAudioSpeakerSetInterface(usb_device_handle handle, uint8_t interface, uint8_t alternateSetting)
{
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;
    usb_status_t error = kStatus_USB_Error;

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
    epCallback.callbackFn    = USB_DeviceAudioIsoIN;
    epCallback.callbackParam = handle;

    epInitStruct.zlt          = 0U;
    epInitStruct.interval     = 1U;
    epInitStruct.transferType = USB_ENDPOINT_ISOCHRONOUS;
    epInitStruct.endpointAddress =
        USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
        epInitStruct.maxPacketSize = HS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
    }
    else
    {
        epInitStruct.maxPacketSize = FS_ISO_FEEDBACK_ENDP_PACKET_SIZE;
    }
    USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);
    if (!feedbackValueUpdating)
    {
        *((uint32_t *)&usbAudioFeedBackBuffer[0]) = *((uint32_t *)&audioFeedBackBuffer[0]);
    }
    error = USB_DeviceSendRequest(handle, USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT, usbAudioFeedBackBuffer,
                                  (USB_SPEED_HIGH == g_deviceComposite->audioUnified.speed) ?
                                      HS_ISO_FEEDBACK_ENDP_PACKET_SIZE :
                                      FS_ISO_FEEDBACK_ENDP_PACKET_SIZE);
#endif

    epCallback.callbackFn    = USB_DeviceAudioIsoOut;
    epCallback.callbackParam = handle;

    epInitStruct.zlt          = 0U;
    epInitStruct.transferType = USB_ENDPOINT_ISOCHRONOUS;
    epInitStruct.endpointAddress =
        USB_AUDIO_SPEAKER_STREAM_ENDPOINT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    if (USB_SPEED_HIGH == g_deviceComposite->speed)
    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
        epInitStruct.maxPacketSize = (HS_ISO_OUT_ENDP_PACKET_SIZE);
        epInitStruct.interval      = HS_ISO_OUT_ENDP_INTERVAL;
#else
        epInitStruct.maxPacketSize = (HS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
        epInitStruct.interval      = HS_ISO_OUT_ENDP_INTERVAL;
#endif
    }
    else
    {
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
        epInitStruct.maxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE);
        epInitStruct.interval      = FS_ISO_OUT_ENDP_INTERVAL;
#else
        epInitStruct.maxPacketSize = (FS_ISO_OUT_ENDP_PACKET_SIZE + AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE);
        epInitStruct.interval      = FS_ISO_OUT_ENDP_INTERVAL;
#endif
    }
    USB_DeviceInitEndpoint(handle, &epInitStruct, &epCallback);
    error =
        USB_DeviceRecvRequest(handle, USB_AUDIO_SPEAKER_STREAM_ENDPOINT, &audioPlayDataBuff[0],
                              (USB_SPEED_HIGH == g_deviceComposite->audioUnified.speed) ? HS_ISO_OUT_ENDP_PACKET_SIZE :
                                                                                          FS_ISO_OUT_ENDP_PACKET_SIZE);

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
usb_status_t USB_DeviceAudioUnifiedInit(usb_device_composite_struct_t *deviceComposite)
{
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
    CTIMER_CaptureInit();
#else
    AUDIO_UPDATE_FEEDBACK_DATA(audioFeedBackBuffer, AUDIO_SAMPLING_RATE_TO_10_14);
#endif

    g_deviceComposite                                       = deviceComposite;
    g_deviceComposite->audioUnified.copyProtect             = 0x01U;
    g_deviceComposite->audioUnified.curSpeakerMute          = 0x00U;
    g_deviceComposite->audioUnified.curSpeakerVolume[0]     = 0x00U;
    g_deviceComposite->audioUnified.curSpeakerVolume[1]     = 0x1fU;
    g_deviceComposite->audioUnified.curMicrophoneMute       = 0x00U;
    g_deviceComposite->audioUnified.curMicrophoneVolume[0]  = 0x00U;
    g_deviceComposite->audioUnified.curMicrophoneVolume[1]  = 0x1fU;
    g_deviceComposite->audioUnified.minVolume[0]            = 0x00U;
    g_deviceComposite->audioUnified.minVolume[1]            = 0x00U;
    g_deviceComposite->audioUnified.maxVolume[0]            = 0x00U;
    g_deviceComposite->audioUnified.maxVolume[1]            = 0X43U;
    g_deviceComposite->audioUnified.resVolume[0]            = 0x01U;
    g_deviceComposite->audioUnified.resVolume[1]            = 0x00U;
    g_deviceComposite->audioUnified.curBass                 = 0x00U;
    g_deviceComposite->audioUnified.curBass                 = 0x00U;
    g_deviceComposite->audioUnified.minBass                 = 0x80U;
    g_deviceComposite->audioUnified.maxBass                 = 0x7FU;
    g_deviceComposite->audioUnified.resBass                 = 0x01U;
    g_deviceComposite->audioUnified.curMid                  = 0x00U;
    g_deviceComposite->audioUnified.minMid                  = 0x80U;
    g_deviceComposite->audioUnified.maxMid                  = 0x7FU;
    g_deviceComposite->audioUnified.resMid                  = 0x01U;
    g_deviceComposite->audioUnified.curTreble               = 0x01U;
    g_deviceComposite->audioUnified.minTreble               = 0x80U;
    g_deviceComposite->audioUnified.maxTreble               = 0x7FU;
    g_deviceComposite->audioUnified.resTreble               = 0x01U;
    g_deviceComposite->audioUnified.curAutomaticGain        = 0x01U;
    g_deviceComposite->audioUnified.curDelay[0]             = 0x00U;
    g_deviceComposite->audioUnified.curDelay[1]             = 0x40U;
    g_deviceComposite->audioUnified.minDelay[0]             = 0x00U;
    g_deviceComposite->audioUnified.minDelay[1]             = 0x00U;
    g_deviceComposite->audioUnified.maxDelay[0]             = 0xFFU;
    g_deviceComposite->audioUnified.maxDelay[1]             = 0xFFU;
    g_deviceComposite->audioUnified.resDelay[0]             = 0x00U;
    g_deviceComposite->audioUnified.resDelay[1]             = 0x01U;
    g_deviceComposite->audioUnified.curLoudness             = 0x01U;
    g_deviceComposite->audioUnified.curSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioUnified.curSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioUnified.curSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioUnified.minSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioUnified.minSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioUnified.minSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioUnified.maxSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioUnified.maxSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioUnified.maxSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioUnified.resSamplingFrequency[0] = 0x00U;
    g_deviceComposite->audioUnified.resSamplingFrequency[1] = 0x00U;
    g_deviceComposite->audioUnified.resSamplingFrequency[2] = 0x01U;
    g_deviceComposite->audioUnified.speed                   = USB_SPEED_FULL;
#if USB_DEVICE_CONFIG_AUDIO_CLASS_2_0
    g_deviceComposite->audioUnified.curSpeakerMute20         = 0U;
    g_deviceComposite->audioUnified.curMicrophoneMute20      = 0U;
    g_deviceComposite->audioUnified.curClockValid            = 1U;
    g_deviceComposite->audioUnified.curSpeakerVolume20[0]    = 0x00U;
    g_deviceComposite->audioUnified.curSpeakerVolume20[1]    = 0x1FU;
    g_deviceComposite->audioUnified.curMicrophoneVolume20[0] = 0x00U;
    g_deviceComposite->audioUnified.curMicrophoneVolume20[1] = 0x1FU;
    g_deviceComposite->audioUnified.curSampleFrequency       = 48000U;

    g_deviceComposite->audioUnified.freqControlRange.wNumSubRanges = 1U;
    g_deviceComposite->audioUnified.freqControlRange.wMIN          = 48000U;
    g_deviceComposite->audioUnified.freqControlRange.wMAX          = 48000U;
    g_deviceComposite->audioUnified.freqControlRange.wRES          = 0U;

    g_deviceComposite->audioUnified.volumeControlRange.wNumSubRanges = 1U;
    g_deviceComposite->audioUnified.volumeControlRange.wMIN          = 0x8001U;
    g_deviceComposite->audioUnified.volumeControlRange.wMAX          = 0x7FFFU;
    g_deviceComposite->audioUnified.volumeControlRange.wRES          = 1U;

#endif
    g_deviceComposite->audioUnified.tdReadNumberPlay              = 0;
    g_deviceComposite->audioUnified.tdWriteNumberPlay             = 0;
    g_deviceComposite->audioUnified.tdWriteNumberRec              = 0;
    g_deviceComposite->audioUnified.tdReadNumberRec               = 0;
    g_deviceComposite->audioUnified.audioSpeakerReadDataCount[1]  = 0;
    g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[0] = 0;
    g_deviceComposite->audioUnified.audioSpeakerWriteDataCount[1] = 0;
    g_deviceComposite->audioUnified.audioSendCount[0]             = 0;
    g_deviceComposite->audioUnified.audioSendCount[1]             = 0;
    g_deviceComposite->audioUnified.audioSendTimes                = 0;
    g_deviceComposite->audioUnified.usbRecvTimes                  = 0;
    g_deviceComposite->audioUnified.audioRecvCount                = 0;
    g_deviceComposite->audioUnified.usbSendTimes                  = 0;
    g_deviceComposite->audioUnified.startPlayFlag                 = 0;
    g_deviceComposite->audioUnified.startPlayHalfFull             = 0;
    g_deviceComposite->audioUnified.startRec                      = 0;
    g_deviceComposite->audioUnified.startRecHalfFull              = 0;
    g_deviceComposite->audioUnified.speakerIntervalCount          = 0;
    g_deviceComposite->audioUnified.speakerReservedSpace          = 0;
    g_deviceComposite->audioUnified.recorderReservedSpace         = 0;
    g_deviceComposite->audioUnified.timesFeedbackCalculate        = 0;
    g_deviceComposite->audioUnified.speakerDetachOrNoInput        = 0;
    for (uint8_t i = 0; i < USB_AUDIO_COMPOSITE_INTERFACE_COUNT; i++)
    {
        g_deviceComposite->audioUnified.currentInterfaceAlternateSetting[i] = 0;
    }
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    g_deviceComposite->audioUnified.froTrimIntervalCount     = 0;
    g_deviceComposite->audioUnified.usbFroTicksPrev          = 0;
    g_deviceComposite->audioUnified.usbFroTicksEma           = AUDIO_FRO_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceComposite->audioUnified.usbFroTickEmaFrac        = 0;
    g_deviceComposite->audioUnified.usbFroTickBasedPrecision = AUDIO_USB_FRO_TRIM_TICK_BASED_PRECISION;
#endif
    g_deviceComposite->audioUnified.curAudioPllFrac            = AUDIO_PLL_FRACTIONAL_DIVIDER;
    g_deviceComposite->audioUnified.audioPllTicksPrev          = 0;
    g_deviceComposite->audioUnified.audioPllTicksEma           = AUDIO_PLL_USB_SOF_INTERVAL_TICK_COUNT;
    g_deviceComposite->audioUnified.audioPllTickEmaFrac        = 0;
    g_deviceComposite->audioUnified.audioPllTickBasedPrecision = AUDIO_PLL_FRACTION_TICK_BASED_PRECISION;
    g_deviceComposite->audioUnified.stopDataLengthAudioAdjust  = 0U;
#endif
    return kStatus_USB_Success;
}

void USB_AudioCodecTask(void)
{
    if (g_deviceComposite->audioUnified.codecSpeakerTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curSpeakerMute20);
#else
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curSpeakerMute);
#endif
        BOARD_SetCodecMuteUnmute(true);
        g_deviceComposite->audioUnified.codecSpeakerTask &= ~MUTE_CODEC_TASK;
        g_CodecSpeakerMuteUnmute = true;
    }
    if (g_deviceComposite->audioUnified.codecMicrophoneTask & MUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curMicrophoneMute20);
#else
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curMicrophoneMute);
#endif
        /* here add code to set mute practically */
        g_deviceComposite->audioUnified.codecMicrophoneTask &= ~MUTE_CODEC_TASK;
        g_CodecMicrophoneMuteUnmute = true;
    }
    if (g_deviceComposite->audioUnified.codecSpeakerTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curSpeakerMute20);
#else
        usb_echo("Set Speaker Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curSpeakerMute);
#endif
        BOARD_SetCodecMuteUnmute(false);
        g_deviceComposite->audioUnified.codecSpeakerTask &= ~UNMUTE_CODEC_TASK;
        g_CodecSpeakerMuteUnmute = false;
    }
    if (g_deviceComposite->audioUnified.codecMicrophoneTask & UNMUTE_CODEC_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curMicrophoneMute20);
#else
        usb_echo("Set Microphone Cur Mute : %x\r\n", g_deviceComposite->audioUnified.curMicrophoneMute);
#endif
        BOARD_SetCodecMuteUnmute(false);
        g_deviceComposite->audioUnified.codecMicrophoneTask &= ~UNMUTE_CODEC_TASK;
        g_CodecMicrophoneMuteUnmute = false;
    }
    if (g_deviceComposite->audioUnified.codecSpeakerTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Speaker Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceComposite->audioUnified.curSpeakerVolume20[1] << 8U) |
                     g_deviceComposite->audioUnified.curSpeakerVolume20[0]);
#else
        usb_echo("Set Speaker Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceComposite->audioUnified.curSpeakerVolume[1] << 8U) |
                     g_deviceComposite->audioUnified.curSpeakerVolume[0]);
#endif
        g_deviceComposite->audioUnified.codecSpeakerTask &= ~VOLUME_CHANGE_TASK;
    }
    if (g_deviceComposite->audioUnified.codecMicrophoneTask & VOLUME_CHANGE_TASK)
    {
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
        usb_echo("Set Microphone Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceComposite->audioUnified.curMicrophoneVolume20[1] << 8U) |
                     g_deviceComposite->audioUnified.curMicrophoneVolume20[0]);
#else
        usb_echo("Set Microphone Cur Volume : %x\r\n",
                 (uint16_t)(g_deviceComposite->audioUnified.curMicrophoneVolume[1] << 8U) |
                     g_deviceComposite->audioUnified.curMicrophoneVolume[0]);
#endif
        g_deviceComposite->audioUnified.codecMicrophoneTask &= ~VOLUME_CHANGE_TASK;
    }
}

void USB_AudioSpeakerResetTask(void)
{
    if (g_deviceComposite->audioUnified.speakerDetachOrNoInput)
    {
        USB_DeviceAudioSpeakerStatusReset();
    }
}
