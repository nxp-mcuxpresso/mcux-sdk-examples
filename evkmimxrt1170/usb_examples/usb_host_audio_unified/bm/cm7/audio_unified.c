/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "audio_unified.h"
#include "usb_host_audio.h"
#include "fsl_debug_console.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* should set it as 1024U, set as 256U for saving memory */
#define MAX_ISO_PACKET_SIZE          (256U)
#define NUMBER_OF_BUFFER             (0x4U)
#define NUMBER_OF_COMMAND_RETRY      (0x3U)
#define USB_AUDIO_RING_BUFFER_LENGTH (10U * 1024U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Audio_RecvData(void);
void Audio_SendData(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* USB audio transfer Types string*/
static char *strTransferType[4] = {"Control", "Isochronous", "Bulk", "Interrupt"};
/* USB audio Sync Types string*/
static char *strSyncType[4] = {"No synchronization", "Asynchronous", "Adaptive", "Synchronous"};
/* USB audio Data Types string*/
static char *strDataType[4] = {"Data endpoint", "Feedback endpoint", "Implicit feedback", "Reserved"};
usb_device_handle g_audioDeviceHandle;
usb_host_interface_handle g_audioControlIfHandle   = NULL;
usb_host_interface_handle g_audioOutStreamIfHandle = NULL;
usb_host_interface_handle g_audioInStreamIfHandle  = NULL;
audio_unified_instance_t g_audio;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_sendBuff[MAX_ISO_PACKET_SIZE * NUMBER_OF_BUFFER];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_recvBuff[MAX_ISO_PACKET_SIZE * NUMBER_OF_BUFFER];
static uint8_t s_wav_buffer[USB_AUDIO_RING_BUFFER_LENGTH];
volatile static uint32_t s_wav_put_index = 0U;
volatile static uint32_t s_wav_get_index = 0U;
uint8_t g_hostCurVolume                  = 4U;
uint16_t g_deviceVolumeStep;
volatile static uint8_t s_speaker_stream_index                          = 0U;
volatile static uint8_t s_recorder_stream_index                         = 0U;
volatile static uint8_t s_audio_loop_back                               = 0U;
usb_audio_stream_format_type_desc_t *g_speakerFormatTypeDescPtr         = NULL;
usb_audio_stream_format_type_desc_t *g_recorderFormatTypeDescPtr        = NULL;
usb_audio_2_0_stream_format_type_desc_t *g_speakerFormatTypeDescPtr_20  = NULL;
usb_audio_2_0_stream_format_type_desc_t *g_recorderFormatTypeDescPtr_20 = NULL;
usb_audio_2_0_stream_spepific_as_intf_desc_t *g_speakerGeneralDesc_20   = NULL;
usb_audio_2_0_stream_spepific_as_intf_desc_t *g_recorderGeneralDesc_20  = NULL;
usb_audio_stream_spepific_as_intf_desc_t *g_speakerAsItfDescPtr         = NULL;
usb_audio_stream_spepific_as_intf_desc_t *g_recorderAsItfDescPtr        = NULL;
usb_descriptor_endpoint_t *g_speakerIsoEndpDescPtr                      = NULL;
usb_descriptor_endpoint_t *g_recorderIsoEndpDescPtr                     = NULL;
static uint16_t g_curVolume;
static uint16_t g_minVolume;
static uint16_t g_maxVolume;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout2_struct_t g_volumeRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout3_struct_t g_speakerFrequencyRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout3_struct_t g_recorderFrequencyRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curMute[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t minVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t maxVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t resVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_speakerSampleFreq;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_recorderSampleFreq;
uint8_t *g_speakerFrequencyAllRang;
uint8_t *g_recorderFrequencyAllRang;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief usb host audio control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
void Audio_ControlCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    audio_unified_instance_t *audio_ptr = (audio_unified_instance_t *)param;
    static uint32_t retryCount          = 0;

    if (status != kStatus_USB_Success)
    {
        retryCount++;
        if (kUSB_HostAudioRunWaitAudioSpeakerSetCurSamplingFreq == audio_ptr->runWaitState)
        {
            usb_echo("audio speaker doest not support SamplingFreq request!\r\n");
            /* set idle, don't do retry */
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetOutStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetInStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceRecDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMinVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetMinVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetOutStreamInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMaxVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetMaxVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetMaxVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetResVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetResVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetResVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetVolumeRang request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetOutStreamInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetCurrentVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }

        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSpeakerGetSampleFrequencyRange)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio speaker doest not support GetSampleFrequencyRange request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioSpeakerGetSampleFrequencyRange;
        }
        else /* other states, don't do retry */
        {
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }

        if (kUSB_HostAudioRunIdle != audio_ptr->runState)
        {
            if (retryCount < NUMBER_OF_COMMAND_RETRY)
            {
                return;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunIdle;
            }
        }
    }
    if (audio_ptr->runState == kUSB_HostAudioRunIdle)
    {
        retryCount = 0;
        if (audio_ptr->runWaitState == kUSB_HostAudioRunSetControlInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetOutStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetOutStreamInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetInStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetInStreamInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetControlInterfaceRec)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceRecDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioSpeakerGetSampleFrequencyRange;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSpeakerGetSampleFrequencyRange)
        {
            /*device support discrete frequency list, will get all the frequency */
            if (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_speakerFrequencyRang.wNumSubRanges[0])) > 1U)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioSpeakerGetAllSampleFrequencyRange;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioRecorderGetSampleFrequencyRange;
            }
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioRecorderGetSampleFrequencyRange)
        {
            /*device support discrete frequency list, will get all the frequency */
            if (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_recorderFrequencyRang.wNumSubRanges[0])) > 1U)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioRecorderGetAllSampleFrequencyRange;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
            }
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSpeakerGetAllSampleFrequencyRange)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioRecorderGetSampleFrequencyRange;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioRecorderGetAllSampleFrequencyRange)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMinVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetMaxVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMaxVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetResVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetResVolume)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel1Vol)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel2Vol;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSpeakerSetCurSamplingFreq)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioRecorderSetCurSamplingFreq;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioRecorderSetCurSamplingFreq)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel2Vol)
        {
            if (g_audio.deviceIsUsed == 0)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq;
            }
            else if (g_audio.deviceIsUsed == 1)
            {
                audio_ptr->runState = kUSB_HostAudioRunIdle;
            }
            else
            {
            }
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigMute)
        {
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
        else
        {
        }
    }
}

/*!
 * @brief put usb recorder transfer data into wave stream buffer.
 */
static void USB_AudioRecorderPutBuffer(uint8_t *buffer, uint8_t *data, uint32_t dataLen)
{
    uint8_t diff                   = 0U;
    static uint8_t format_match    = 0U;
    static uint8_t speaker_format  = 0U;
    static uint8_t recorder_format = 0U;
    static uint8_t resolution_byte = 0U;
    static uint8_t channel_count   = 0U;
    uint32_t buffer_length         = 0U;
    uint32_t partLength            = 0U;

    if (0U != dataLen)
    {
        if (0U != g_audio.format_change)
        {
            format_match          = 0U;
            speaker_format        = 0U;
            recorder_format       = 0U;
            resolution_byte       = 0U;
            channel_count         = 0U;
            g_audio.format_change = 0U;

            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                if ((g_speakerFormatTypeDescPtr->bbitresolution == g_recorderFormatTypeDescPtr->bbitresolution) &&
                    (g_speakerFormatTypeDescPtr->bnrchannels == g_recorderFormatTypeDescPtr->bnrchannels))
                {
                    format_match = 1U;
                }
                else if ((g_speakerFormatTypeDescPtr->bbitresolution == g_recorderFormatTypeDescPtr->bbitresolution) &&
                         (g_speakerFormatTypeDescPtr->bnrchannels != g_recorderFormatTypeDescPtr->bnrchannels))
                {
                    resolution_byte = (g_recorderFormatTypeDescPtr->bbitresolution + 7U) / 8U;
                    speaker_format  = g_speakerFormatTypeDescPtr->bnrchannels;
                    recorder_format = g_recorderFormatTypeDescPtr->bnrchannels;
                    format_match    = 2U;
                }
                else /* channel count is same,  resolution is different */
                {
                    speaker_format  = (g_speakerFormatTypeDescPtr->bbitresolution + 7U) / 8U;
                    recorder_format = (g_recorderFormatTypeDescPtr->bbitresolution + 7U) / 8U;
                    channel_count   = g_recorderFormatTypeDescPtr->bnrchannels;
                    format_match    = 3U;
                }
            }
            else if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                if ((g_speakerFormatTypeDescPtr_20->bBitResolution == g_recorderFormatTypeDescPtr_20->bBitResolution) &&
                    (g_recorderGeneralDesc_20->bNrChannels == g_speakerGeneralDesc_20->bNrChannels))
                {
                    format_match = 1U;
                }
                else if ((g_speakerFormatTypeDescPtr_20->bBitResolution ==
                          g_recorderFormatTypeDescPtr_20->bBitResolution) &&
                         (g_recorderGeneralDesc_20->bNrChannels != g_speakerGeneralDesc_20->bNrChannels))
                {
                    resolution_byte = (g_recorderFormatTypeDescPtr_20->bBitResolution + 7U) / 8U;
                    speaker_format  = g_speakerGeneralDesc_20->bNrChannels;
                    recorder_format = g_recorderGeneralDesc_20->bNrChannels;
                    format_match    = 2U;
                }
                else /* channel count is same,  resolution is different */
                {
                    speaker_format  = (g_speakerFormatTypeDescPtr_20->bBitResolution + 7U) / 8U;
                    recorder_format = (g_recorderFormatTypeDescPtr_20->bBitResolution + 7U) / 8U;
                    channel_count   = g_recorderGeneralDesc_20->bNrChannels;
                    format_match    = 3U;
                }
            }
            else
            {
                /* no action */
            }
        }

        switch (format_match)
        {
            case 1U:
                buffer_length = s_wav_put_index + dataLen;
                if (buffer_length < USB_AUDIO_RING_BUFFER_LENGTH)
                {
                    memcpy((void *)&buffer[s_wav_put_index], (void *)data, dataLen);
                    s_wav_put_index += dataLen;
                }
                else
                {
                    partLength = USB_AUDIO_RING_BUFFER_LENGTH - s_wav_put_index;
                    memcpy((void *)&buffer[s_wav_put_index], (void *)data, partLength);
                    buffer_length = dataLen - partLength; /* the remain data length */
                    if (buffer_length > 0U)
                    {
                        memcpy((void *)&buffer[0], (void *)&data[partLength], buffer_length);
                    }
                    s_wav_put_index = buffer_length;
                }
                break;

            case 2U:
                if (speaker_format > recorder_format)
                {
                    diff = speaker_format - recorder_format;
                    for (uint32_t i = 0U; i < dataLen;)
                    {
                        for (uint8_t j = 0U; j < (resolution_byte * recorder_format); j++)
                        {
                            buffer[s_wav_put_index++] = data[i + j];
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        for (uint8_t j = 0U; j < (diff * resolution_byte); j++)
                        {
                            buffer[s_wav_put_index++] = 0U;
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        i = i + (resolution_byte * recorder_format);
                    }
                }
                else
                {
                    diff = recorder_format - speaker_format;
                    for (uint32_t i = 0U; i < dataLen;)
                    {
                        for (uint8_t j = 0U; j < (resolution_byte * speaker_format); j++)
                        {
                            buffer[s_wav_put_index++] = data[i + j];
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        i = i + (resolution_byte * recorder_format);
                    }
                }
                break;

            case 3:
                if (speaker_format > recorder_format)
                {
                    diff = speaker_format - recorder_format;
                    for (uint32_t i = 0U; i < dataLen;)
                    {
                        for (uint8_t j = 0U; j < (recorder_format * channel_count); j++)
                        {
                            buffer[s_wav_put_index++] = data[i + j];
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        for (uint8_t j = 0U; j < (diff * channel_count); j++)
                        {
                            buffer[s_wav_put_index++] = 0U;
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        i = i + (resolution_byte * channel_count);
                    }
                }
                else
                {
                    for (uint32_t i = 0U; i < dataLen;)
                    {
                        for (uint8_t j = 0U; j < (speaker_format * channel_count); j++)
                        {
                            buffer[s_wav_put_index++] = data[i + j];
                            if (s_wav_put_index == USB_AUDIO_RING_BUFFER_LENGTH)
                            {
                                s_wav_put_index = 0U;
                            }
                        }
                        i = i + (recorder_format * channel_count);
                    }
                }
                break;
            default:
                usb_echo("Not right format match\r\n");
                break;
        }
    }
}

/*!
 * @brief host audio  iso  in transfer callback.
 *
 * This function is used as callback function when call usb_host_audio_recv .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Audio_InCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    audio_unified_instance_t *audio_recorder_ptr = (audio_unified_instance_t *)param;
    if (status == kStatus_USB_TransferCancel)
    {
        return;
    }
    if (status == kStatus_USB_Success)
    {
        if (audio_recorder_ptr->devState == kStatus_DEV_Attached)
        {
            USB_AudioRecorderPutBuffer(s_wav_buffer, data, dataLen);
            Audio_RecvData();
        }
        else
        {
            audio_recorder_ptr->runState = kUSB_HostAudioRunIdle;
        }
    }
    else
    {
        if (audio_recorder_ptr->devState == kStatus_DEV_Attached)
        {
            USB_AudioRecorderPutBuffer(s_wav_buffer, data, dataLen);
            Audio_RecvData();
        }
    }
}

/*!
 * @brief host audio  iso  out transfer callback.
 *
 * This function is used as callback function when call usb_host_audio_send .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Audio_OutCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    audio_unified_instance_t *audio_speaker_ptr = (audio_unified_instance_t *)param;
    if (status == kStatus_USB_TransferCancel)
    {
        return;
    }

    if (status == kStatus_USB_Success)
    {
        if (audio_speaker_ptr->devState == kStatus_DEV_Attached)
        {
            Audio_SendData();
        }
        else
        {
            audio_speaker_ptr->runState = kUSB_HostAudioRunIdle;
        }
    }
    else
    {
        if (audio_speaker_ptr->devState == kStatus_DEV_Attached)
        {
            Audio_SendData();
        }
    }
}

static void USB_HostAudioAppCalculateTransferSampleCount(void)
{
    uint32_t sendSampleCountInOneSecond;
    uint32_t sendFreq;
    uint32_t interval;
    uint32_t speed = 0U;

    sendFreq = g_speakerSampleFreq;

    interval = 1U << (g_speakerIsoEndpDescPtr->bInterval - 1U);
    (void)USB_HostHelperGetPeripheralInformation(g_audio.deviceHandle, (uint32_t)kUSB_HostGetDeviceSpeed, &speed);
    if (speed == USB_SPEED_HIGH)
    {
        if (interval < 8U)
        {
            uint32_t oneSampleSize;
            uint32_t maxBytesInOneSecond;

            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                oneSampleSize =
                    g_speakerFormatTypeDescPtr->bnrchannels * ((g_speakerFormatTypeDescPtr->bbitresolution + 7) / 8U);
            }
            else
            {
                oneSampleSize =
                    g_speakerGeneralDesc_20->bNrChannels * ((g_speakerFormatTypeDescPtr_20->bBitResolution + 7) / 8U);
            }

            maxBytesInOneSecond = ((sendFreq + 999U) / 1000U) * oneSampleSize;
            if (maxBytesInOneSecond <= ((USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_speakerIsoEndpDescPtr->wMaxPacketSize) &
                                         USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK)))
            {
                /* change as 1ms interval to send data */
                USB_HostAudioSetStreamOutDataInterval(g_audio.speakerClassHandle, 0x04);
                sendSampleCountInOneSecond = 1000U;
            }
            else if ((maxBytesInOneSecond / 2) <=
                     ((USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_speakerIsoEndpDescPtr->wMaxPacketSize) &
                       USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK)))
            {
                /* change as 0.5ms interval to send data */
                USB_HostAudioSetStreamOutDataInterval(g_audio.speakerClassHandle, 0x03);
                sendSampleCountInOneSecond = 2000U;
            }
            else
            {
                sendSampleCountInOneSecond = 1000U * (8 / interval);
            }
        }
        else
        {
            sendSampleCountInOneSecond = 1000U / (interval / 8);
        }
    }
    else
    {
        sendSampleCountInOneSecond = 1000U / (interval);
    }

    g_audio.oneTransferLessSampleCount = sendFreq / sendSampleCountInOneSecond;
    if (g_audio.oneTransferLessSampleCount * sendSampleCountInOneSecond != sendFreq)
    {
        /*
         * g_audio.oneTransferLessSampleCount * g_audio.sendLessSampleTotalCount + (g_audio.oneTransferLessSampleCount +
         * 1) * (sendSampleCountInOneSecond - g_audio.sendLessSampleTotalCount) = sendFreq
         */
        g_audio.sendLessSampleTotalCount =
            g_audio.oneTransferLessSampleCount * sendSampleCountInOneSecond + sendSampleCountInOneSecond - sendFreq;
        g_audio.sendMoreSampleTotalCount = sendSampleCountInOneSecond - g_audio.sendLessSampleTotalCount;
        if (g_audio.sendLessSampleTotalCount >= g_audio.sendMoreSampleTotalCount)
        {
            g_audio.toggleCount = g_audio.sendLessSampleTotalCount / g_audio.sendMoreSampleTotalCount;
        }
        else
        {
            g_audio.toggleCount = g_audio.sendMoreSampleTotalCount / g_audio.sendLessSampleTotalCount;
        }
    }
    else
    {
        g_audio.toggleCount = 0U;
    }

    g_audio.sendLessSampleCurrentCount = g_audio.sendLessSampleTotalCount;
    g_audio.sendMoreSampleCurrentCount = g_audio.sendMoreSampleTotalCount;
    g_audio.toggleCurrentCount         = 0U;
}

static uint32_t USB_HostAudioAppGetTransferSampleCount(void)
{
    uint32_t returnVal = 0U;

    if (g_audio.toggleCount == 0U)
    {
        returnVal = g_audio.oneTransferLessSampleCount;
    }
    else
    {
        if ((g_audio.sendLessSampleCurrentCount != 0U) && (g_audio.sendMoreSampleCurrentCount != 0U))
        {
            if (g_audio.sendLessSampleTotalCount >= g_audio.sendMoreSampleTotalCount)
            {
                if (g_audio.toggleCurrentCount == g_audio.toggleCount)
                {
                    g_audio.toggleCurrentCount = 0U;
                    g_audio.sendMoreSampleCurrentCount--;
                    returnVal = g_audio.oneTransferLessSampleCount + 1U;
                }
                else
                {
                    g_audio.toggleCurrentCount++;
                    g_audio.sendLessSampleCurrentCount--;
                    returnVal = g_audio.oneTransferLessSampleCount;
                }
            }
            else
            {
                if (g_audio.toggleCurrentCount == g_audio.toggleCount)
                {
                    g_audio.toggleCurrentCount = 0U;
                    g_audio.sendLessSampleCurrentCount--;
                    returnVal = g_audio.oneTransferLessSampleCount;
                }
                else
                {
                    g_audio.toggleCurrentCount++;
                    g_audio.sendMoreSampleCurrentCount--;
                    returnVal = g_audio.oneTransferLessSampleCount + 1;
                }
            }
        }
        else
        {
            if (g_audio.sendLessSampleCurrentCount != 0U)
            {
                g_audio.sendLessSampleCurrentCount--;
                returnVal = g_audio.oneTransferLessSampleCount;
            }
            else
            {
                g_audio.sendMoreSampleCurrentCount--;
                returnVal = g_audio.oneTransferLessSampleCount + 1;
            }

            if ((g_audio.sendLessSampleCurrentCount == 0U) && (g_audio.sendMoreSampleCurrentCount == 0U))
            {
                /* reset */
                g_audio.sendLessSampleCurrentCount = g_audio.sendLessSampleTotalCount;
                g_audio.sendMoreSampleCurrentCount = g_audio.sendMoreSampleTotalCount;
                g_audio.toggleCurrentCount         = 0U;
            }
        }
    }

    return returnVal;
}

/*!
 * @brief prepare usb transfer data.
 */
static uint32_t USB_AudioSpeakerGetBuffer(uint8_t *buffer)
{
    uint32_t k = 0U;
    uint8_t channelIndex;
    uint8_t sampleIndex;
    uint8_t channels;
    uint8_t oneChannelSampleSize;
    uint32_t sampleCount = USB_HostAudioAppGetTransferSampleCount();

    if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
    {
        channels             = g_speakerFormatTypeDescPtr->bnrchannels;
        oneChannelSampleSize = ((g_speakerFormatTypeDescPtr->bbitresolution + 7) / 8U);
    }
    else
    {
        channels             = g_speakerGeneralDesc_20->bNrChannels;
        oneChannelSampleSize = ((g_speakerFormatTypeDescPtr_20->bBitResolution + 7) / 8U);
    }

    for (k = 0U; k < sampleCount; ++k)
    {
        for (channelIndex = 0U; channelIndex < channels; ++channelIndex)
        {
            if (channelIndex < 2)
            {
                sampleIndex = 0U;
                if (oneChannelSampleSize > 2U)
                {
                    for (; sampleIndex < (oneChannelSampleSize - 2U); ++sampleIndex)
                    {
                        buffer[k * oneChannelSampleSize + sampleIndex] = 0U;
                    }
                }
                for (; sampleIndex < oneChannelSampleSize; ++sampleIndex)
                {
                    buffer[k * (oneChannelSampleSize * channels) + channelIndex * oneChannelSampleSize + sampleIndex] =
                        s_wav_buffer[s_wav_get_index];
                    s_wav_get_index++;
                    if (s_wav_get_index >= USB_AUDIO_RING_BUFFER_LENGTH)
                    {
                        s_wav_get_index = 0U;
                    }
                }
            }
            else
            {
                for (sampleIndex = 0U; sampleIndex < oneChannelSampleSize; ++sampleIndex)
                {
                    buffer[k * (oneChannelSampleSize * channels) + channelIndex * oneChannelSampleSize + sampleIndex] =
                        0U;
                }
            }
        }
    }

    return sampleCount * channels * oneChannelSampleSize;
}

void Audio_RecvData(void)
{
    usb_status_t status = kStatus_USB_Success;

    status = USB_HostAudioStreamRecv(g_audio.recorderClassHandle,
                                     (unsigned char *)&g_recvBuff[MAX_ISO_PACKET_SIZE * s_recorder_stream_index],
                                     g_audio.audioRecorderIsoMaxPacketSize, Audio_InCallback, &g_audio);
    if (status != kStatus_USB_Success)
    {
        usb_echo("Error in USB_HostAudioStreamRecv: %x\r\n", status);
        return;
    }
    s_recorder_stream_index++;
    if (s_recorder_stream_index == NUMBER_OF_BUFFER)
    {
        s_recorder_stream_index = 0U;
    }
}

/*!
 * @brief host audio iso out transfer send data function.
 *
 * This function is used to send iso data.
 *
 */
void Audio_SendData(void)
{
    usb_status_t status = kStatus_USB_Success;

    if (0U == s_audio_loop_back)
    {
        if (s_wav_put_index >= (USB_AUDIO_RING_BUFFER_LENGTH / 2))
        {
            s_audio_loop_back = 1;
        }
    }

    if (1U == s_audio_loop_back)
    {
        status = USB_HostAudioStreamSend(
            g_audio.speakerClassHandle, (unsigned char *)&g_sendBuff[MAX_ISO_PACKET_SIZE * s_speaker_stream_index],
            USB_AudioSpeakerGetBuffer(&g_sendBuff[MAX_ISO_PACKET_SIZE * s_speaker_stream_index]), Audio_OutCallback,
            &g_audio);
    }
    else
    {
        status = USB_HostAudioStreamSend(g_audio.speakerClassHandle, (unsigned char *)&g_sendBuff[0], 12U,
                                         Audio_OutCallback, &g_audio);
    }
    s_speaker_stream_index++;
    if (s_speaker_stream_index == NUMBER_OF_BUFFER)
    {
        s_speaker_stream_index = 0U;
    }
    if (status != kStatus_USB_Success)
    {
        usb_echo("Error in USB_HostAudioStreamSend: %x\r\n", status);
        return;
    }
}

/*!
 * @brief host usb audio task function.
 *
 * This function implements the host audio action, it is used to create task.
 *
 * @param param  the host audio instance pointer.
 */
void USB_AudioTask(void *arg)
{
    uint32_t bsamfreqtype_g_index;

    /* device state changes */
    if (g_audio.devState != g_audio.prevState)
    {
        g_audio.prevState = g_audio.devState;
        switch (g_audio.devState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached:
                g_audio.runState     = kUSB_HostAudioRunSetControlInterface;
                g_audio.deviceIsUsed = 0;
                g_hostCurVolume      = 4;
                USB_HostAudioInit(g_audio.deviceHandle, &g_audio.speakerClassHandle);
                USB_HostAudioInit(g_audio.deviceHandle, &g_audio.recorderClassHandle);
                usb_echo("USB audio unified device attached\r\n");
                break;

            case kStatus_DEV_Detached:
                g_audio.devState = kStatus_DEV_Idle;
                g_audio.runState = kUSB_HostAudioRunIdle;
                USB_HostAudioDeinit(g_audio.deviceHandle, g_audio.speakerClassHandle);
                USB_HostAudioDeinit(g_audio.deviceHandle, g_audio.recorderClassHandle);
                g_audio.speakerClassHandle  = NULL;
                g_audio.recorderClassHandle = NULL;
                s_speaker_stream_index      = 0U;
                if (NULL != g_speakerFrequencyAllRang)
                {
                    free(g_speakerFrequencyAllRang);
                    g_speakerFrequencyAllRang = NULL;
                }
                usb_echo("usb audio unified device detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (g_audio.runState)
    {
        case kUSB_HostAudioRunIdle:
            break;

        case kUSB_HostAudioRunSetControlInterface:
            g_audio.runWaitState = kUSB_HostAudioRunSetControlInterface;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioControlSetInterface(g_audio.speakerClassHandle, g_audio.controlIntfHandle, 0,
                                                 Audio_ControlCallback, &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetControlInterfaceDone:
            g_audio.runWaitState = kUSB_HostAudioRunWaitSetOutStreamInterface;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioStreamSetInterface(g_audio.speakerClassHandle, g_audio.outStreamIntfHandle, 1,
                                                Audio_ControlCallback, &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetOutStreamInterfaceDone:
            g_audio.runWaitState = kUSB_HostAudioRunWaitSetControlInterfaceRec;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioControlSetInterface(g_audio.recorderClassHandle, g_audio.controlIntfHandle, 0,
                                                 Audio_ControlCallback, &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetControlInterfaceRecDone:
            g_audio.runWaitState = kUSB_HostAudioRunWaitSetInStreamInterface;
            g_audio.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioStreamSetInterface(g_audio.recorderClassHandle, g_audio.inStreamIntfHandle, 1,
                                                Audio_ControlCallback, &g_audio) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetInStreamInterfaceDone:
            g_audio.runState = kUSB_HostAudioRunIdle;
            {
                usb_audio_ctrl_common_header_desc_t *commonHeader;
                if (USB_HostAudioControlGetCurrentAltsettingSpecificDescriptors(
                        g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_DESC_SUBTYPE_AUDIO_CS_HEADER, (void *)&commonHeader) != kStatus_USB_Success)
                {
                    usb_echo("Get header descriptor error\r\n");
                    g_audio.runState = kUSB_HostAudioRunIdle;
                }
                else
                {
                    g_audio.deviceAudioVersion =
                        USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)commonHeader->bcdcdc));
                    if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 1.0 device\r\n");
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_speakerIsoEndpDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker data endpoint descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.recorderClassHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_recorderIsoEndpDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder data endpoint descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE,
                                (void *)&g_speakerFormatTypeDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.recorderClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE,
                                (void *)&g_recorderFormatTypeDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL,
                                (void *)&g_speakerAsItfDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker audio general descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.recorderClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL,
                                (void *)&g_recorderAsItfDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder audio general descriptor error\r\n");
                        }

                        g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetMinVolume;
                        g_audio.runState     = kUSB_HostAudioRunIdle;
                        if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                                       g_audio.speakerClassHandle,
                                                       (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U),
                                                       (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_GET_MIN),
                                                       &minVol, sizeof(minVol), Audio_ControlCallback, &g_audio))
                        {
                            g_audio.runState = kUSB_HostAudioRunAudioDone;
                        }
                        usb_echo("AUDIO_GET_MIN_VOLUME\r\n");
                    }
                    else if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 2.0 device\r\n");

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_speakerIsoEndpDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker data endpoint descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.recorderClassHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_recorderIsoEndpDescPtr) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder data endpoint descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE_20,
                                (void *)&g_speakerFormatTypeDescPtr_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.recorderClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE_20,
                                (void *)&g_recorderFormatTypeDescPtr_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL_20,
                                (void *)&g_speakerGeneralDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get speaker audio general descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audio.speakerClassHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL_20,
                                (void *)&g_recorderGeneralDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get recorder audio general descriptor error\r\n");
                        }

                        g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetVolumeRang;
                        g_audio.runState     = kUSB_HostAudioRunIdle;
                        if (kStatus_USB_Success !=
                            USB_HostAudioGetSetFeatureUnitRequest(
                                g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), &g_volumeRang,
                                sizeof(g_volumeRang), Audio_ControlCallback, &g_audio))
                        {
                            g_audio.runState = kUSB_HostAudioRunAudioDone;
                        }
                        usb_echo("AUDIO_GET_VOLUME_RANG\r\n");
                    }
                    else
                    {
                        usb_echo("unsupported audio device, current host only support audio1.0 and audio2.0\r\n");
                    }
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetMaxVolume:
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetMaxVolume;
                g_audio.runState     = kUSB_HostAudioRunIdle;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U),
                                               (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_MAX), &maxVol,
                                               sizeof(maxVol), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;

        case kUSB_HostAudioRunAudioGetResVolume:
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetResVolume;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U),
                                               (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_RES), &resVol,
                                               sizeof(resVol), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetCurrentVolume:
            if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), &g_curVolume,
                                               sizeof(g_curVolume), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;

        case kUSB_HostAudioRunAudioSpeakerGetSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioSpeakerGetSampleFrequencyRange;
                if (kStatus_USB_Success !=
                    USB_HostAudioGetSetClockSourceRequest(
                        g_audio.speakerClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                        ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), &g_speakerFrequencyRang,
                        sizeof(g_speakerFrequencyRang), Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioSpeakerGetAllSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                /*fresurency use layout 3 Parameter Block, so the lenght is 2+12*n*/
                g_speakerFrequencyAllRang = malloc(
                    2U + 12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_speakerFrequencyRang.wNumSubRanges[0]))));
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioSpeakerGetAllSampleFrequencyRange;
                if (kStatus_USB_Success !=
                    USB_HostAudioGetSetClockSourceRequest(
                        g_audio.speakerClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                        ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), g_speakerFrequencyAllRang,
                        (2U + 12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_speakerFrequencyRang.wNumSubRanges[0])))),
                        Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioRecorderGetSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioRecorderGetSampleFrequencyRange;
                if (kStatus_USB_Success !=
                    USB_HostAudioGetSetClockSourceRequest(
                        g_audio.recorderClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                        ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), &g_recorderFrequencyRang,
                        sizeof(g_recorderFrequencyRang), Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioRecorderGetAllSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                /*fresurency use layout 3 Parameter Block, so the lenght is 2+12*n*/
                g_recorderFrequencyAllRang = malloc(
                    2U + 12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_recorderFrequencyRang.wNumSubRanges[0]))));
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioSpeakerGetAllSampleFrequencyRange;
                if (kStatus_USB_Success !=
                    USB_HostAudioGetSetClockSourceRequest(
                        g_audio.recorderClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                        ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), g_recorderFrequencyAllRang,
                        (2U +
                         12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_recorderFrequencyRang.wNumSubRanges[0])))),
                        Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioConfigChannel:
            g_audio.runState = kUSB_HostAudioRunIdle;
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_minVolume = (uint16_t)(minVol[1] << 8U) | (minVol[0]);
                g_maxVolume = (uint16_t)(maxVol[1] << 8U) | (maxVol[0]);
                g_deviceVolumeStep =
                    (int16_t)(((int16_t)(g_maxVolume) - (int16_t)(g_minVolume)) / (HOST_MAX_VOLUME - HOST_MIN_VOLUME));
                g_curVolume          = (int16_t)(g_minVolume + g_deviceVolumeStep * g_hostCurVolume);
                g_curVol[0]          = (uint8_t)((uint16_t)g_curVolume & 0x00FF);
                g_curVol[1]          = (uint8_t)((uint16_t)g_curVolume >> 8U);
                g_curMute[0]         = 0U;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               (uint32_t)((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), &g_curMute,
                                               sizeof(g_curMute), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else if (AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion)
            {
                g_minVolume          = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMIN[0]));
                g_maxVolume          = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMAX[0]));
                g_deviceVolumeStep   = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wRES[0]));
                g_curVol[0]          = (uint8_t)((uint16_t)g_curVolume & 0x00FF);
                g_curVol[1]          = (uint8_t)((uint16_t)g_curVolume >> 8U);
                g_curMute[0]         = 0U;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else
            {
                /*TO DO*/
            }
            break;
        case kUSB_HostAudioRunAudioConfigMute:
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]         = !g_curMute[0];
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audio))

                {
                    usb_echo("config mute failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]         = !g_curMute[0];
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audio))
                {
                    usb_echo("config mute failed\n\r");
                }
            }
            else
            {
                /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioConfigChannel1Vol:
            g_audio.runState     = kUSB_HostAudioRunIdle;
            g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel1Vol;
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq;
                    usb_echo("config volume failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 1U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq;
                    usb_echo("config volume failed\n\r");
                }
            }
            else
            {
                /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioConfigChannel2Vol:
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 2U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audio))

                {
                    g_audio.runState = kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq;
                    usb_echo("config channel 2 volume failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                g_audio.runState     = kUSB_HostAudioRunIdle;
                g_audio.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audio.speakerClassHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 2U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audio))
                {
                    g_audio.runState = kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq;
                    usb_echo("config channel 2 volume failed\n\r");
                }
            }
            else
            {
                /*TO DO*/
            }

            break;

        case kUSB_HostAudioRunAudioSpeakerSetCurSamplingFreq:
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                usb_echo("Audio recorder information:\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_recorderFormatTypeDescPtr->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("   - Frequency device support      : %d Hz\n\r",
                             (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }

                usb_echo("   - Bit resolution : %d bits\n\r", g_recorderFormatTypeDescPtr->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_recorderFormatTypeDescPtr->bnrchannels);
                usb_echo("   - Transfer type : %s\n\r",
                         strTransferType[(g_recorderIsoEndpDescPtr->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_recorderIsoEndpDescPtr->bmAttributes >> 2) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_recorderIsoEndpDescPtr->bmAttributes >> 4) & EP_TYPE_MASK]);

                usb_echo("This audio recorder records audio with these properties:\r\n");
                usb_echo("   - Sample rate    :\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_recorderFormatTypeDescPtr->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("                    : %d Hz\n\r",
                             (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }
                usb_echo("   - Sample size    : %d bits\n\r", g_recorderFormatTypeDescPtr->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_recorderFormatTypeDescPtr->bnrchannels);
                g_recorderSampleFreq = (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[0][2]) << 16U) |
                                       (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[0][1]) << 8U) |
                                       (((uint32_t)g_recorderFormatTypeDescPtr->tsamfreq[0][0]) << 0U);

                usb_echo("\nAudio speaker information:\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_speakerFormatTypeDescPtr->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("   - Frequency device support      : %d Hz\n\r",
                             (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }

                usb_echo("   - Bit resolution : %d bits\n\r", g_speakerFormatTypeDescPtr->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_speakerFormatTypeDescPtr->bnrchannels);
                usb_echo("   - Transfer type : %s\n\r",
                         strTransferType[(g_speakerIsoEndpDescPtr->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_speakerIsoEndpDescPtr->bmAttributes >> 2) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_speakerIsoEndpDescPtr->bmAttributes >> 4) & EP_TYPE_MASK]);

                usb_echo("This audio speaker plays audio with these properties:\r\n");
                usb_echo("   - Sample rate    :\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_speakerFormatTypeDescPtr->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("                    : %d Hz\n\r",
                             (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }
                usb_echo("   - Sample size    : %d bits\n\r", g_speakerFormatTypeDescPtr->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_speakerFormatTypeDescPtr->bnrchannels);
                g_speakerSampleFreq = (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[0][2]) << 16U) |
                                      (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[0][1]) << 8U) |
                                      (((uint32_t)g_speakerFormatTypeDescPtr->tsamfreq[0][0]) << 0U);
                if ((g_recorderSampleFreq != g_speakerSampleFreq) ||
                    ((g_recorderFormatTypeDescPtr->bbitresolution != g_speakerFormatTypeDescPtr->bbitresolution) &&
                     (g_recorderFormatTypeDescPtr->bnrchannels != g_speakerFormatTypeDescPtr->bnrchannels)))
                {
                    usb_echo(
                        "USB host unified example can not support this device, please change to use another one\r\n");
                    g_audio.runState     = kUSB_HostAudioRunIdle;
                    g_audio.runWaitState = kUSB_HostAudioRunIdle;
                    break;
                }
                else
                {
                    usb_echo(
                        "USB host unfied example is recording %dk_%dbit_%dch format audio, then loop playback "
                        "%dk_%dbit_%dch format recorded audio.\r\n",
                        g_recorderSampleFreq / 1000U, g_recorderFormatTypeDescPtr->bbitresolution,
                        g_recorderFormatTypeDescPtr->bnrchannels, g_speakerSampleFreq / 1000U,
                        g_speakerFormatTypeDescPtr->bbitresolution, g_speakerFormatTypeDescPtr->bnrchannels);
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                uint8_t *frequencyArray;
                uint8_t *frequencyArrayStart;
                usb_echo("Audio recorder information:\r\n");
                if (1U < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_recorderFrequencyRang.wNumSubRanges[0])))
                {
                    /*frequency array location in layout 3, offset 2 is the start address of frequency array*/
                    frequencyArray = (uint8_t *)(g_recorderFrequencyAllRang + 2U);
                    usb_echo("more than one frequenuy is support by recorder\r\n");
                }
                else
                {
                    frequencyArray = (uint8_t *)&g_recorderFrequencyRang.wMIN[0];
                }
                frequencyArrayStart = frequencyArray;
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                                                                           (&g_recorderFrequencyRang.wNumSubRanges[0]));
                     bsamfreqtype_g_index++)
                {
                    /*frequency array location in layout 3*/
                    usb_echo(
                        "   - Frequency device support frequency rang is :MIN %d Hz,  MAX %d Hz, RES attributes %dHz, "
                        "\n\r",
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 4U)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 8U)));
                    if (48000U == USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)))
                    {
                        frequencyArray = frequencyArrayStart;
                    }
                    frequencyArrayStart += sizeof(usb_audio_2_0_layout3_struct_t);
                }

                g_recorderSampleFreq = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArray));
                usb_echo("   - Bit resolution : %d bits\n\r", g_recorderFormatTypeDescPtr_20->bBitResolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_recorderGeneralDesc_20->bNrChannels);
                usb_echo("   - Transfer type : %s\n\r",
                         strTransferType[(g_recorderIsoEndpDescPtr->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_recorderIsoEndpDescPtr->bmAttributes >> 2U) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_recorderIsoEndpDescPtr->bmAttributes >> 4U) & EP_TYPE_MASK]);

                usb_echo("\nAudio speaker information:\r\n");
                if (1U < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_speakerFrequencyRang.wNumSubRanges[0])))
                {
                    /*frequency array location in layout 3, offset 2 is the start address of frequency array*/
                    frequencyArray = (uint8_t *)(g_speakerFrequencyAllRang + 2U);
                    usb_echo("more than one frequenuy is support by speaker\r\n");
                }
                else
                {
                    frequencyArray = (uint8_t *)&g_speakerFrequencyRang.wMIN[0];
                }
                frequencyArrayStart = frequencyArray;
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                                                                           (&g_speakerFrequencyRang.wNumSubRanges[0]));
                     bsamfreqtype_g_index++)
                {
                    /*frequency array location in layout 3*/
                    usb_echo(
                        "   - Frequency device support frequency rang is :MIN %d Hz,  MAX %d Hz, RES attributes %dHz, "
                        "\n\r",
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 4U)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 8U)));
                    if (48000U == USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)))
                    {
                        frequencyArray = frequencyArrayStart;
                    }
                    frequencyArrayStart += sizeof(usb_audio_2_0_layout3_struct_t);
                }
                g_speakerSampleFreq = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArray));
                usb_echo("   - Bit resolution : %d bits\n\r", g_speakerFormatTypeDescPtr_20->bBitResolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_speakerGeneralDesc_20->bNrChannels);
                usb_echo("   - Transfer type : %s\n\r",
                         strTransferType[(g_speakerIsoEndpDescPtr->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_speakerIsoEndpDescPtr->bmAttributes >> 2U) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_speakerIsoEndpDescPtr->bmAttributes >> 4U) & EP_TYPE_MASK]);
                if ((g_recorderSampleFreq != g_speakerSampleFreq) ||
                    ((g_recorderFormatTypeDescPtr_20->bBitResolution !=
                      g_speakerFormatTypeDescPtr_20->bBitResolution) &&
                     (g_recorderGeneralDesc_20->bNrChannels != g_speakerGeneralDesc_20->bNrChannels)))
                {
                    usb_echo(
                        "USB host unified example can not support this device, please change to use another one\r\n");
                    g_audio.runState     = kUSB_HostAudioRunIdle;
                    g_audio.runWaitState = kUSB_HostAudioRunIdle;
                    break;
                }
                else
                {
                    usb_echo(
                        "USB host unfied example is recording %dk_%dbit_%dch format audio, then loop playback "
                        "%dk_%dbit_%dch format recorded audio.\r\n",
                        g_recorderSampleFreq / 1000U, g_recorderFormatTypeDescPtr_20->bBitResolution,
                        g_recorderGeneralDesc_20->bNrChannels, g_speakerSampleFreq / 1000U,
                        g_speakerFormatTypeDescPtr_20->bBitResolution, g_speakerGeneralDesc_20->bNrChannels);
                }
            }
            else
            {
                /*TO DO*/
            }

            g_audio.audioRecorderIsoMaxPacketSize =
                (uint16_t)((USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_recorderIsoEndpDescPtr->wMaxPacketSize) &
                            USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK));
            g_audio.runState     = kUSB_HostAudioRunIdle;
            g_audio.runWaitState = kUSB_HostAudioRunWaitAudioSpeakerSetCurSamplingFreq;

            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                USB_HostAudioGetSetEndpointRequest(
                    g_audio.speakerClassHandle, (uint32_t)((USB_AUDIO_EP_CS_SAMPING_FREQ_CONTROL << 8U)),
                    ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), &g_speakerSampleFreq, sizeof(g_speakerSampleFreq),
                    Audio_ControlCallback, &g_audio);
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                USB_HostAudioGetSetClockSourceRequest(
                    g_audio.speakerClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                    ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), &g_speakerSampleFreq, sizeof(g_speakerSampleFreq),
                    Audio_ControlCallback, &g_audio);
            }
            else
            {
                /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioRecorderSetCurSamplingFreq:
            g_audio.runState     = kUSB_HostAudioRunIdle;
            g_audio.runWaitState = kUSB_HostAudioRunWaitAudioRecorderSetCurSamplingFreq;
            if (AUDIO_DEVICE_VERSION_01 == g_audio.deviceAudioVersion)
            {
                USB_HostAudioGetSetEndpointRequest(
                    g_audio.recorderClassHandle, (uint32_t)((USB_AUDIO_EP_CS_SAMPING_FREQ_CONTROL << 8U)),
                    ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), &g_recorderSampleFreq,
                    sizeof(g_recorderSampleFreq), Audio_ControlCallback, &g_audio);
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audio.deviceAudioVersion))
            {
                USB_HostAudioGetSetClockSourceRequest(
                    g_audio.recorderClassHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                    ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), &g_recorderSampleFreq,
                    sizeof(g_recorderSampleFreq), Audio_ControlCallback, &g_audio);
            }
            else
            {
                /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioDone:
            s_speaker_stream_index  = 0U;
            s_recorder_stream_index = 0U;
            s_wav_put_index         = 0U;
            s_wav_get_index         = 0U;
            s_audio_loop_back       = 0U;
            (void)memset((void *)g_sendBuff, 0, MAX_ISO_PACKET_SIZE * NUMBER_OF_BUFFER);
            (void)memset((void *)g_recvBuff, 0, MAX_ISO_PACKET_SIZE * NUMBER_OF_BUFFER);
            g_audio.runState      = kUSB_HostAudioRunIdle;
            g_audio.format_change = 1U;
            USB_HostAudioAppCalculateTransferSampleCount();
            for (bsamfreqtype_g_index = 0; bsamfreqtype_g_index < NUMBER_OF_BUFFER; ++bsamfreqtype_g_index)
            {
                Audio_SendData();
            }
            for (bsamfreqtype_g_index = 0; bsamfreqtype_g_index < NUMBER_OF_BUFFER; ++bsamfreqtype_g_index)
            {
                Audio_RecvData();
            }
            g_audio.deviceIsUsed = 1;
            break;

        default:
            break;
    }
}

/*!
 * @brief host usb audio mute request.
 *
 * This function implements the host audio mute request action, it is used to mute or unmute audio device.
 *
 */
void Audio_MuteRequest(void)
{
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigMute;
}

/*!
 * @brief host usb audio increase volume request.
 *
 * This function implements the host audio increase volume request action, it is used to increase the volume of audio
 * device.
 *
 * @param channel   the audio device channel number.
 */
void Audio_IncreaseVolumeRequest(uint8_t channel)
{
    static uint32_t i;
    uint8_t max_audio_channel;
    max_audio_channel = 2U;
    if (channel > max_audio_channel)
    {
        usb_echo("  err: Channel number larger than max channel\n\r");
        return;
    }
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }
    if (channel == 1U)
    {
        /* Send set mute request */
        if ((g_hostCurVolume + HOST_VOLUME_STEP) > HOST_MAX_VOLUME)
        {
            g_hostCurVolume = HOST_MAX_VOLUME;
            i               = 0U;
        }
        else
        {
            g_hostCurVolume += HOST_VOLUME_STEP;
            i = 1U;
        }
        g_curVolume += (int16_t)(i * HOST_VOLUME_STEP * g_deviceVolumeStep);
        g_curVol[0] = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
        g_curVol[1] = (int8_t)((uint16_t)(g_curVolume) >> 8U);
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
}

/*!
 * @brief host usb audio decrease volume request.
 *
 * This function implements the host audio decrease volume request action, it is used to decrease the volume of audio
 * device.
 *
 * @param channel   the audio device channel number.
 */
void Audio_DecreaseVolumeRequest(uint8_t channel)
{
    static uint32_t i;
    uint8_t max_audio_channel;
    max_audio_channel = g_speakerFormatTypeDescPtr->bnrchannels;
    if (channel > max_audio_channel)
    {
        usb_echo("  err: Channel number larger than max channel\n\r");
        return;
    }
    if (g_audio.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Speaker is not connected\n\r");
        return;
    }

    if (g_audio.deviceIsUsed == 0U)
    {
        usb_echo("  err: Audio Speaker is not Ready\n\r");
        return;
    }
    if (channel == 1U)
    {
        if (g_hostCurVolume < (HOST_VOLUME_STEP + HOST_MIN_VOLUME))
        {
            g_hostCurVolume = HOST_MIN_VOLUME;
            i               = 0U;
        }
        else
        {
            g_hostCurVolume -= HOST_VOLUME_STEP;
            i = 1U;
        }
        g_curVolume -= (int16_t)(i * HOST_VOLUME_STEP * g_deviceVolumeStep);
        g_curVol[0] = (int8_t)((uint16_t)(g_curVolume)&0x00FF);
        g_curVol[1] = (int8_t)((uint16_t)(g_curVolume) >> 8U);
    }

    g_audio.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
}

/*!
 * @brief host audio callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode              callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
usb_status_t USB_HostAudioEvent(usb_device_handle deviceHandle,
                                usb_host_configuration_handle configurationHandle,
                                uint32_t eventCode)
{
    uint8_t id;
    usb_host_configuration_t *configuration_ptr;
    uint8_t interface_g_index;
    usb_host_interface_t *interface_ptr;
    uint32_t info_value = 0U;
    usb_status_t status = kStatus_USB_Success;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration_ptr        = (usb_host_configuration_t *)configurationHandle;
            g_audioDeviceHandle      = NULL;
            g_audioControlIfHandle   = NULL;
            g_audioOutStreamIfHandle = NULL;
            g_audioInStreamIfHandle  = NULL;
            for (interface_g_index = 0U; interface_g_index < configuration_ptr->interfaceCount; ++interface_g_index)
            {
                interface_ptr = &configuration_ptr->interfaceList[interface_g_index];
                id            = interface_ptr->interfaceDesc->bInterfaceClass;
                if (id != USB_AUDIO_CLASS_CODE)
                {
                    continue;
                }
                id = interface_ptr->interfaceDesc->bInterfaceSubClass;
                if (id == USB_AUDIO_SUBCLASS_CODE_CONTROL)
                {
                    g_audioControlIfHandle = interface_ptr;
                    continue;
                }
                else if (id == USB_AUDIO_SUBCLASS_CODE_AUDIOSTREAMING)
                {
                    usb_host_interface_t alternate1Interface;
                    if (interface_ptr->alternateSettingNumber > 0U)
                    {
                        USB_HostHelperParseAlternateSetting(interface_ptr, 1, &alternate1Interface);
                        for (info_value = 0U; info_value < alternate1Interface.epCount; ++info_value)
                        {
                            if ((alternate1Interface.epList[info_value].epDesc->bEndpointAddress &
                                 USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                                USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT)
                            {
                                g_audioOutStreamIfHandle = interface_ptr;
                                break;
                            }

                            if ((alternate1Interface.epList[info_value].epDesc->bEndpointAddress &
                                 USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                                USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN)
                            {
                                g_audioInStreamIfHandle = interface_ptr;
                                break;
                            }
                        }
                    }

                    if ((g_audioOutStreamIfHandle != NULL) && (g_audioInStreamIfHandle != NULL))
                    {
                        g_audioDeviceHandle = deviceHandle;
                        break;
                    }
                }
                else
                {
                    continue;
                }
            }

            if (g_audioDeviceHandle != NULL)
            {
                return kStatus_USB_Success;
            }
            else
            {
                usb_echo("Please insert a device supported both speaker and recorder.\r\n");
                status = kStatus_USB_NotSupported;
            }
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_audio.devState == kStatus_DEV_Idle)
            {
                if ((g_audioDeviceHandle != NULL) && (g_audioControlIfHandle != NULL))
                {
                    g_audio.devState            = kStatus_DEV_Attached;
                    g_audio.deviceHandle        = g_audioDeviceHandle;
                    g_audio.controlIntfHandle   = g_audioControlIfHandle;
                    g_audio.outStreamIntfHandle = g_audioOutStreamIfHandle;
                    g_audio.inStreamIntfHandle  = g_audioInStreamIfHandle;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("hid audio attached:pid=0x%x", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &info_value);
                    usb_echo("vid=0x%x ", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &info_value);
                    usb_echo("address=%d\r\n", info_value);
                }
            }
            else
            {
                usb_echo("not idle audio instance\r\n");
            }
            break;

        case kUSB_HostEventDetach:
            if (g_audio.devState != kStatus_DEV_Idle)
            {
                g_audio.devState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}
