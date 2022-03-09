/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "audio_recorder.h"
#include "usb_host_audio.h"
#include "fsl_debug_console.h"
#include "fsl_sd.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint32_t USB_HostAudioRecorderBufferSpaceUsed(void);
static void USB_HostAudioRecorderWriteSdcard();
static void USB_HostAudioRecorderWaitCommand();
/*******************************************************************************
 * Variables
 ******************************************************************************/
/* should set it as 1024U, set as 256U for saving memory */
#define MAX_ISO_PACKET_SIZE                     (256U)
#define USB_AUDIO_RECORDER_STREAM_PRIME_COUNT   (5U)
#define USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH (1024U * 3U)
#define USB_AUDIO_RECORDER_SDCARD_WRITE_LENGTH  (1024U)
#define NUMBER_OF_COMMAND_RETRY                 0x3U
/* USB audio transfer Types string*/
static char *strTransferType[4] = {"Control", "Isochronous", "Bulk", "Interrupt"};
/* USB audio Sync Types string*/
static char *strSyncType[4] = {"No synchronization", "Asynchronous", "Adaptive", "Synchronous"};
/* USB audio Data Types string*/
static char *strDataType[4] = {"Data endpoint", "Feedback endpoint", "Implicit feedback", "Reserved"};
usb_device_handle g_audioRecorderDeviceHandle;
usb_host_interface_handle g_audioRecorderOutControlifHandle;
usb_host_interface_handle g_audioRecorderOutStreamifHandle;
audio_recorder_instance_t g_audioRecorder;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_audioRecordStreamBuffer[USB_AUDIO_RECORDER_STREAM_PRIME_COUNT][MAX_ISO_PACKET_SIZE];
uint8_t g_audioDataSdcardBuffer[USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH];
extern const unsigned char wav_data[];
extern const uint32_t wav_size;
uint8_t g_hostCurVolume = 4U;
uint16_t g_deviceVolumeStep;
usb_audio_stream_format_type_desc_t *g_pFormatTypeDesc         = NULL;
usb_audio_2_0_stream_format_type_desc_t *g_pFormatTypeDesc_20  = NULL;
usb_audio_2_0_stream_spepific_as_intf_desc_t *g_generalDesc_20 = NULL;
usb_audio_stream_spepific_as_intf_desc_t *g_pAsItfDesc         = NULL;
usb_descriptor_endpoint_t *g_pIsoEndpDesc                      = NULL;
static uint16_t g_curVolume;
static uint16_t g_minVolume;
static uint16_t g_maxVolume;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout2_struct_t g_volumeRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_audio_2_0_control_range_layout3_struct_t g_frequencyRang;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_curMute[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t minVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t maxVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint16_t resVol[2];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t freq;
uint8_t *g_frequencyAllRang;
/* File system object */
static FATFS g_fileSystem;
const TCHAR g_DriverNumberBuffer[] = {SDDISK + '0', ':', '/', '\0'};
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
    audio_recorder_instance_t *audio_ptr = (audio_recorder_instance_t *)param;
    static uint32_t retryCount           = 0;

    if (status != kStatus_USB_Success)
    {
        retryCount++;
        if (kUSB_HostAudioRunWaitAudioSetCurSamplingFreq == audio_ptr->runWaitState)
        {
            usb_echo("audio recorder doest not support SamplingFreq request!\r\n");
            /* set idle, don't do retry */
            audio_ptr->runState = kUSB_HostAudioRunIdle;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetControlInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMinVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetMinVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetMaxVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetMaxVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetMaxVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetResVolume)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetResVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetResVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetVolumeRang request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetCurrentVolume request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }

        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange)
        {
            if (kStatus_USB_TransferStall == status)
            {
                usb_echo("audio recorder doest not support GetSampleFrequencyRange request!\r\n");
            }
            audio_ptr->runState = kUSB_HostAudioRunAudioGetSampleFrequencyRange;
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
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitSetStreamInterface)
        {
            audio_ptr->runState = kUSB_HostAudioRunSetInterfaceDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetVolumeRang)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetCurrentVolume;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioGetSampleFrequencyRange;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange)
        {
            /*device support discrete frequency list, will get all the frequency */
            if (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0])) > 1U)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioGetAllSampleFrequencyRange;
            }
            else
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioConfigChannel;
            }
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioGetAllSampleFrequencyRange)
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
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioSetCurSamplingFreq)
        {
            audio_ptr->runState = kUSB_HostAudioRunAudioDone;
        }
        else if (audio_ptr->runWaitState == kUSB_HostAudioRunWaitAudioConfigChannel2Vol)
        {
            if (g_audioRecorder.deviceIsUsed == 0)
            {
                audio_ptr->runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
            }
            else if (g_audioRecorder.deviceIsUsed == 1)
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
 * @brief host audio  iso  out transfer callback.
 *
 * This function is used as callback function when call usb_host_audio_send .
 *
 * @param param    the host audio instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Audio_RecorderInCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t transfer_status)
{
    usb_status_t status    = kStatus_USB_Error;
    uint32_t buffer_length = 0U;
    uint32_t partLength    = 0U;

    if (transfer_status == kStatus_USB_TransferCancel)
    {
        return;
    }

    if (0U != dataLen)
    {
        buffer_length = g_audioRecorder.audioRecorderStreamPutBufferIndex + dataLen;
        if (buffer_length < USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH)
        {
            memcpy(
                (void *)((uint8_t *)&g_audioDataSdcardBuffer[g_audioRecorder.audioRecorderStreamPutBufferIndex]),
                (void *)((uint8_t *)&s_audioRecordStreamBuffer[g_audioRecorder.audioRecorderStreamUsbBufferIndex][0]),
                dataLen);
            g_audioRecorder.audioRecorderStreamPutBufferIndex += dataLen;
        }
        else
        {
            partLength = USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH - g_audioRecorder.audioRecorderStreamPutBufferIndex;
            memcpy(
                (void *)((uint8_t *)&g_audioDataSdcardBuffer[g_audioRecorder.audioRecorderStreamPutBufferIndex]),
                (void *)((uint8_t *)&s_audioRecordStreamBuffer[g_audioRecorder.audioRecorderStreamUsbBufferIndex][0]),
                partLength);
            buffer_length = dataLen - partLength; /* the remain data length */
            if (buffer_length > 0U)
            {
                memcpy((void *)((uint8_t *)&g_audioDataSdcardBuffer[0]),
                       (void *)(uint8_t *)&s_audioRecordStreamBuffer[g_audioRecorder.audioRecorderStreamUsbBufferIndex]
                                                                    [partLength],
                       buffer_length);
            }
            g_audioRecorder.audioRecorderStreamPutBufferIndex = buffer_length;
        }
    }

    if (1U == g_audioRecorder.audioRecorderRunFlag)
    {
        /* prime the next usb stream transfer */
        status = USB_HostAudioStreamRecv(
            g_audioRecorder.classHandle,
            (unsigned char *)&s_audioRecordStreamBuffer[g_audioRecorder.audioRecorderStreamUsbBufferIndex][0],
            g_audioRecorder.audioRecorderIsoMaxPacketSize, Audio_RecorderInCallback, &g_audioRecorder);
        if (status != kStatus_USB_Success)
        {
            usb_echo("usb transfer error");
        }
    }
    g_audioRecorder.audioRecorderStreamUsbBufferIndex++;
    if (g_audioRecorder.audioRecorderStreamUsbBufferIndex == USB_AUDIO_RECORDER_STREAM_PRIME_COUNT)
    {
        g_audioRecorder.audioRecorderStreamUsbBufferIndex = 0;
    }
}

/* The USB_AudioRecorderBufferSpaceUsed() function gets the used recorder ringbuffer used size */
static uint32_t USB_HostAudioRecorderBufferSpaceUsed(void)
{
    if (g_audioRecorder.audioRecorderStreamPutBufferIndex >= g_audioRecorder.audioRecorderStreamGetBufferIndex)
    {
        g_audioRecorder.recorderUsedSpace =
            g_audioRecorder.audioRecorderStreamPutBufferIndex - g_audioRecorder.audioRecorderStreamGetBufferIndex;
    }
    else
    {
        g_audioRecorder.recorderUsedSpace = g_audioRecorder.audioRecorderStreamPutBufferIndex +
                                            USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH -
                                            g_audioRecorder.audioRecorderStreamGetBufferIndex;
    }
    return g_audioRecorder.recorderUsedSpace;
}

/*!
 * @brief write picture into sd card.
 */
static void USB_HostAudioRecorderWriteSdcard()
{
    static uint8_t s_FileOpened = 0;
    static FIL fileObj;
    char fileName[32];
    FRESULT fileStatus;
    uint32_t writeSize   = 0U;
    uint32_t used_size   = 0U;
    uint32_t actual_size = 0U;
    uint32_t part_length = 0U;

    if (g_audioRecorder.audioRecorderWriteSdcardFlag)
    {
        if (!s_FileOpened)
        {
            sprintf(fileName, "%saudio%d.pcm", &g_DriverNumberBuffer[0], g_audioRecorder.audioFileIndex);

            fileStatus = f_open(&fileObj, _T(fileName), FA_WRITE | FA_CREATE_ALWAYS);
            if (FR_OK != fileStatus)
            {
                usb_echo("sdcard operate fail\r\n");
                return;
            }
            s_FileOpened = 1;
        }

        used_size = USB_HostAudioRecorderBufferSpaceUsed();
        if (used_size < USB_AUDIO_RECORDER_SDCARD_WRITE_LENGTH)
        {
            if (1U == g_audioRecorder.audioRecorderRunFlag)
            {
                return;
            }
            else
            {
                actual_size = used_size;
            }
        }
        else
        {
            actual_size = USB_AUDIO_RECORDER_SDCARD_WRITE_LENGTH;
        }

        if (0U != actual_size)
        {
            part_length = g_audioRecorder.audioRecorderStreamGetBufferIndex + actual_size;
            fileStatus  = FR_INVALID_PARAMETER;
            if (part_length <= USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH)
            {
                fileStatus = f_write(
                    &fileObj, (uint8_t *)&g_audioDataSdcardBuffer[g_audioRecorder.audioRecorderStreamGetBufferIndex],
                    actual_size, (UINT *)&writeSize);
            }

            if ((fileStatus != FR_OK) || (actual_size != writeSize))
            {
                usb_echo("sdcard operate fail\r\n");
                g_audioRecorder.runState                     = kUSB_HostAudioRunAudioWaitCommand;
                g_audioRecorder.audioRecorderWriteSdcardFlag = 0U;
                return;
            }
        }

        g_audioRecorder.audioRecorderStreamGetBufferIndex += actual_size;
        if (g_audioRecorder.audioRecorderStreamGetBufferIndex >= USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH)
        {
            g_audioRecorder.audioRecorderStreamGetBufferIndex =
                g_audioRecorder.audioRecorderStreamGetBufferIndex - USB_AUDIO_RECORDER_SDCARD_BUFFER_LENGTH;
        }

        /* Stop recording and the buffer data has been saved in sdcard */
        if ((actual_size < USB_AUDIO_RECORDER_SDCARD_WRITE_LENGTH) && (0U == g_audioRecorder.audioRecorderRunFlag))
        {
            s_FileOpened = 0;
            fileStatus   = f_sync(&fileObj);
            f_close(&fileObj);
            if (FR_OK != fileStatus)
            {
                usb_echo("file sync fail\r\n");
                return;
            }
            usb_echo("Recording stops, AUDIO%d.PCM is saved\r\n", g_audioRecorder.audioFileIndex);
            if (kStatus_DEV_Detached == g_audioRecorder.devState)
            {
                g_audioRecorder.devState     = kStatus_DEV_Idle;
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunIdle;
                USB_HostAudioDeinit(g_audioRecorder.deviceHandle, g_audioRecorder.classHandle);
                g_audioRecorder.classHandle = NULL;
            }
            g_audioRecorder.audioFileIndex++;
            g_audioRecorder.runState                     = kUSB_HostAudioRunAudioWaitCommand;
            g_audioRecorder.audioRecorderWriteSdcardFlag = 0U;
        }
    }
}

/*!
 * @brief check user command to start record.
 */
static void USB_HostAudioRecorderWaitCommand()
{
    uint8_t command;

    if (kStatus_USB_Success != DbgConsole_TryGetchar((char *)&command))
    {
        command = 0;
    }
    if ('r' == command)
    {
        g_audioRecorder.audioRecorderRunFlag = 1U;
    }
    else if ('s' == command)
    {
        g_audioRecorder.audioRecorderRunFlag = 0U;
    }
    else
    {
        /* TO DO */
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
    uint8_t pcm_format = 0U;
    uint8_t *frequencyArray;
    uint8_t *frequencyArrayStart;

    /* device state changes */
    if (g_audioRecorder.devState != g_audioRecorder.prevState)
    {
        g_audioRecorder.prevState = g_audioRecorder.devState;
        switch (g_audioRecorder.devState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached:
                g_audioRecorder.runState     = kUSB_HostAudioRunSetControlInterface;
                g_audioRecorder.deviceIsUsed = 0;
                g_hostCurVolume              = 4;
                USB_HostAudioInit(g_audioRecorder.deviceHandle, &g_audioRecorder.classHandle);
                usb_echo("USB audio attached\r\n");
                break;

            case kStatus_DEV_Detached:
                /* when recording, stop recording and de-initialize host when finishing writing sdcard */
                if (1U == g_audioRecorder.audioRecorderRunFlag)
                {
                    g_audioRecorder.audioRecorderRunFlag = 0U;
                }
                else
                {
                    g_audioRecorder.devState = kStatus_DEV_Idle;
                    g_audioRecorder.runState = kUSB_HostAudioRunIdle;
                    USB_HostAudioDeinit(g_audioRecorder.deviceHandle, g_audioRecorder.classHandle);
                    g_audioRecorder.classHandle = NULL;
                }
                if (NULL != g_frequencyAllRang)
                {
                    free(g_frequencyAllRang);
                    g_frequencyAllRang = NULL;
                }
                usb_echo("audio recorder detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (g_audioRecorder.runState)
    {
        case kUSB_HostAudioRunIdle:
            break;

        case kUSB_HostAudioRunSetControlInterface:
            g_audioRecorder.runWaitState = kUSB_HostAudioRunSetControlInterface;
            g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioControlSetInterface(g_audioRecorder.classHandle, g_audioRecorder.controlIntfHandle, 0,
                                                 Audio_ControlCallback, &g_audioRecorder) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostAudioRunSetControlInterfaceDone:
            g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitSetStreamInterface;
            g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
            if (USB_HostAudioStreamSetInterface(g_audioRecorder.classHandle, g_audioRecorder.streamIntfHandle, 1,
                                                Audio_ControlCallback, &g_audioRecorder) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostAudioRunSetInterfaceDone:
            g_audioRecorder.runState = kUSB_HostAudioRunIdle;
            {
                usb_audio_ctrl_common_header_desc_t *commonHeader;
                if (USB_HostAudioControlGetCurrentAltsettingSpecificDescriptors(
                        g_audioRecorder.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                        USB_DESC_SUBTYPE_AUDIO_CS_HEADER, (void *)&commonHeader) != kStatus_USB_Success)
                {
                    usb_echo("Get header descriptor error\r\n");
                    g_audioRecorder.runState = kUSB_HostAudioRunIdle;
                }
                else
                {
                    g_audioRecorder.deviceAudioVersion =
                        USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(((uint8_t *)commonHeader->bcdcdc));
                    if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 1.0 device\r\n");
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_pIsoEndpDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get data endpoint descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE,
                                (void *)&g_pFormatTypeDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL, (void *)&g_pAsItfDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get audio general descriptor error\r\n");
                        }
                        g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetMinVolume;
                        g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                        if (kStatus_USB_Success !=
                            USB_HostAudioGetSetFeatureUnitRequest(
                                g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_GET_MIN), &minVol, sizeof(minVol),
                                Audio_ControlCallback, &g_audioRecorder))
                        {
                            g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                        }
                        usb_echo("AUDIO_GET_MIN_VOLUME\r\n");
                    }
                    else if (AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion)
                    {
                        usb_echo("AUDIO 2.0 device\r\n");

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_DESCRIPTOR_TYPE_ENDPOINT, USB_AUDIO_EP_TYPE_DATA,
                                (void *)&g_pIsoEndpDesc) != kStatus_USB_Success)
                        {
                            usb_echo("Get data endpoint descriptor error\r\n");
                        }
                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_FORMAT_TYPE_20,
                                (void *)&g_pFormatTypeDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get format descriptor error\r\n");
                        }

                        if (USB_HostAudioStreamGetCurrentAltsettingSpecificDescriptors(
                                g_audioRecorder.classHandle, USB_AUDIO_DESCRIPTOR_TYPE_CS_INTERFACE,
                                USB_AUDIO_DESC_SUBTYPE_AS_GENERAL_20, (void *)&g_generalDesc_20) != kStatus_USB_Success)
                        {
                            usb_echo("Get audio general descriptor error\r\n");
                        }

                        g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetVolumeRang;
                        g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                        if (kStatus_USB_Success !=
                            USB_HostAudioGetSetFeatureUnitRequest(
                                g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                (uint32_t)((1U << 8) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), &g_volumeRang,
                                sizeof(g_volumeRang), Audio_ControlCallback, &g_audioRecorder))
                        {
                            g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
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
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetMaxVolume;
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_MAX), &maxVol,
                                               sizeof(maxVol), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;

        case kUSB_HostAudioRunAudioGetResVolume:
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetResVolume;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               (uint32_t)((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_GET_RES), &resVol,
                                               sizeof(resVol), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetCurrentVolume:
            if (AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetCurrentVolumeDone;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), &g_curVolume,
                                               sizeof(g_curVolume), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;

        case kUSB_HostAudioRunAudioGetSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetSampleFrequencyRange;
                if (kStatus_USB_Success != USB_HostAudioGetSetClockSourceRequest(
                                               g_audioRecorder.classHandle,
                                               (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                                               ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), &g_frequencyRang,
                                               sizeof(g_frequencyRang), Audio_ControlCallback, &g_audioRecorder))
                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioGetAllSampleFrequencyRange:
            if (AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion)
            {
                /*fresurency use layout 3 Parameter Block, so the lenght is 2+12*n*/
                g_frequencyAllRang =
                    malloc(2U + 12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0]))));
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioGetAllSampleFrequencyRange;
                if (kStatus_USB_Success !=
                    USB_HostAudioGetSetClockSourceRequest(
                        g_audioRecorder.classHandle, (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                        ((1U << 8U) | USB_AUDIO_CS_REQUEST_CODE_RANGE_20), g_frequencyAllRang,
                        (2U + 12U * (USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0])))),
                        Audio_ControlCallback, &g_audioRecorder))
                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioDone;
                }
            }
            break;
        case kUSB_HostAudioRunAudioConfigChannel:
            g_audioRecorder.runState = kUSB_HostAudioRunIdle;
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                g_minVolume = (uint16_t)(minVol[1] << 8U) | (minVol[0]);
                g_maxVolume = (uint16_t)(maxVol[1] << 8U) | (maxVol[0]);
                g_deviceVolumeStep =
                    (int16_t)(((int16_t)(g_maxVolume) - (int16_t)(g_minVolume)) / (HOST_MAX_VOLUME - HOST_MIN_VOLUME));
                g_curVolume                  = (int16_t)(g_minVolume + g_deviceVolumeStep * g_hostCurVolume);
                g_curVol[0]                  = (uint8_t)((uint16_t)g_curVolume & 0x00FF);
                g_curVol[1]                  = (uint8_t)((uint16_t)g_curVolume >> 8U);
                g_curMute[0]                 = 0U;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               (uint32_t)((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), &g_curMute,
                                               sizeof(g_curMute), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else if (AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion)
            {
                g_minVolume                  = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMIN[0]));
                g_maxVolume                  = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wMAX[0]));
                g_deviceVolumeStep           = USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_volumeRang.wRES[0]));
                g_curVol[0]                  = (uint8_t)((uint16_t)g_curVolume & 0x00FF);
                g_curVol[1]                  = (uint8_t)((uint16_t)g_curVolume >> 8U);
                g_curMute[0]                 = 0U;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audioRecorder))
                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
                }
            }
            else
            {
                /*TO DO*/
            }
            break;
        case kUSB_HostAudioRunAudioConfigMute:
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]                 = !g_curMute[0];
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audioRecorder))

                {
                    usb_echo("config mute failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_curMute[0]                 = !g_curMute[0];
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigMute;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_MUTE << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curMute,
                                               sizeof(g_curMute[0]), Audio_ControlCallback, &g_audioRecorder))
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
            g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
            g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel1Vol;
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
                    usb_echo("config volume failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle, (uint32_t)((AUDIO_FU_VOLUME << 8U) | 0U),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audioRecorder))
                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
                    usb_echo("config volume failed\n\r");
                }
            }
            else
            {
                /*TO DO*/
            }
            break;

        case kUSB_HostAudioRunAudioConfigChannel2Vol:
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle,
                                               (uint32_t)((AUDIO_FU_VOLUME << 8U) | g_pFormatTypeDesc->bnrchannels),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audioRecorder))

                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
                    usb_echo("config channel 2 volume failed\n\r");
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioConfigChannel2Vol;
                if (kStatus_USB_Success != USB_HostAudioGetSetFeatureUnitRequest(
                                               g_audioRecorder.classHandle,
                                               (uint32_t)((AUDIO_FU_VOLUME << 8U) | g_generalDesc_20->bNrChannels),
                                               ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), (void *)g_curVol,
                                               sizeof(g_curVol), Audio_ControlCallback, &g_audioRecorder))
                {
                    g_audioRecorder.runState = kUSB_HostAudioRunAudioSetCurSamplingFreq;
                    usb_echo("config channel 2 volume failed\n\r");
                }
            }
            else
            {
                /*TO DO*/
            }

            break;

        case kUSB_HostAudioRunAudioSetCurSamplingFreq:
            usb_echo("Audio Recorder device information:\r\n");
            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_pFormatTypeDesc->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("   - Frequency device support      : %d Hz\n\r",
                             (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }

                usb_echo("   - Bit resolution : %d bits\n\r", g_pFormatTypeDesc->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_pFormatTypeDesc->bnrchannels);
                usb_echo("   - Transfer type : %s\n\r", strTransferType[(g_pIsoEndpDesc->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 2) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 4) & EP_TYPE_MASK]);

                usb_echo("This audio device supports play audio files with these properties:\r\n");
                usb_echo("   - Sample rate    :\r\n");
                for (bsamfreqtype_g_index = 0U; bsamfreqtype_g_index < g_pFormatTypeDesc->bsamfreqtype;
                     bsamfreqtype_g_index++)
                {
                    usb_echo("                    : %d Hz\n\r",
                             (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][2]) << 16U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][1]) << 8U) |
                                 (((uint32_t)g_pFormatTypeDesc->tsamfreq[bsamfreqtype_g_index][0]) << 0U));
                }
                usb_echo("   - Sample size    : %d bits\n\r", g_pFormatTypeDesc->bbitresolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_pFormatTypeDesc->bnrchannels);
                usb_echo("USB Recorder example try to record %dk_%dbit_%dch audio using PCM format.\r\n",
                         ((((uint32_t)g_pFormatTypeDesc->tsamfreq[0][2]) << 16U) |
                          (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][1]) << 8U) |
                          (((uint32_t)g_pFormatTypeDesc->tsamfreq[0][0]) << 0U)) /
                             1000U,
                         g_pFormatTypeDesc->bbitresolution, g_pFormatTypeDesc->bnrchannels);
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                if (1U < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0])))
                {
                    /*frequency array location in layout 3, offset 2 is the start address of frequency array*/
                    frequencyArray = (uint8_t *)(g_frequencyAllRang + 2U);
                    usb_echo("more than one frequenuy is support by device\r\n");
                }
                else
                {
                    frequencyArray = (uint8_t *)&g_frequencyRang.wMIN[0];
                }
                frequencyArrayStart = frequencyArray;
                for (bsamfreqtype_g_index = 0U;
                     bsamfreqtype_g_index < USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((&g_frequencyRang.wNumSubRanges[0]));
                     bsamfreqtype_g_index++)
                {
                    /*frequency array location in layout 3*/
                    usb_echo(
                        "   - Frequency device support frequency rang is :MIN %d Hz,  MAX %d Hz, RES attributes %dHz, "
                        "\n\r",
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 4U)),
                        USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArrayStart + 8U)));
                    frequencyArrayStart += sizeof(usb_audio_2_0_layout3_struct_t);
                }
                usb_echo("   - Bit resolution : %d bits\n\r", g_pFormatTypeDesc_20->bBitResolution);
                usb_echo("   - Number of channels : %d channels\n\r", g_generalDesc_20->bNrChannels);
                usb_echo("   - Transfer type : %s\n\r", strTransferType[(g_pIsoEndpDesc->bmAttributes) & EP_TYPE_MASK]);
                usb_echo("   - Sync type : %s\n\r",
                         strSyncType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 2U) & EP_TYPE_MASK]);
                usb_echo("   - Usage type : %s  \n\r",
                         strDataType[(uint8_t)(g_pIsoEndpDesc->bmAttributes >> 4U) & EP_TYPE_MASK]);
                usb_echo("USB Host Recorder example try to record %dk_%dbit_%d ch audio using PCM format.\r\n",
                         USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS((frequencyArray)) / 1000U,
                         g_pFormatTypeDesc_20->bBitResolution, g_generalDesc_20->bNrChannels);
            }
            else
            {
                /*TO DO*/
            }
            g_audioRecorder.audioRecorderIsoMaxPacketSize =
                (uint16_t)((USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_pIsoEndpDesc->wMaxPacketSize) &
                            USB_DESCRIPTOR_ENDPOINT_MAXPACKETSIZE_SIZE_MASK));
            g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
            g_audioRecorder.runWaitState = kUSB_HostAudioRunWaitAudioSetCurSamplingFreq;
            if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
            }

            if (AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion)
            {
                USB_HostAudioGetSetEndpointRequest(g_audioRecorder.classHandle,
                                                   (uint32_t)((USB_AUDIO_EP_CS_SAMPING_FREQ_CONTROL << 8U)),
                                                   ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_SET_CUR), &freq,
                                                   sizeof(freq), Audio_ControlCallback, &g_audioRecorder);
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                USB_HostAudioGetSetClockSourceRequest(g_audioRecorder.classHandle,
                                                      (uint32_t)((USB_AUDIO_CS_SAM_FREQ_CONTROL_20 << 8U) | 0U),
                                                      ((0U << 8U) | USB_AUDIO_CS_REQUEST_CODE_CUR_20), &freq,
                                                      sizeof(freq), Audio_ControlCallback, &g_audioRecorder);
            }
            else
            {
                /*TO DO*/
            }
            break;
        case kUSB_HostAudioRunAudioDone:
            if ((AUDIO_DEVICE_VERSION_01 == g_audioRecorder.deviceAudioVersion))
            {
                /* only Audio Data Format Type I support PCM format */
                if (USB_AUDIO_FORMAT_TYPE_I == g_pFormatTypeDesc->bformattype)
                {
                    if ((0x0001U == USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_pAsItfDesc->wformattag)) ||
                        (0x0002U == USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(g_pAsItfDesc->wformattag)))
                    {
                        pcm_format = 1U;
                    }
                }
            }
            else if ((AUDIO_DEVICE_VERSION_02 == g_audioRecorder.deviceAudioVersion))
            {
                /* only Audio Data Format Type I and IV support PCM format */
                if ((USB_AUDIO_FORMAT_TYPE_I == g_generalDesc_20->bFormatType) ||
                    (USB_AUDIO_FORMAT_TYPE_IV == g_generalDesc_20->bFormatType))
                {
                    /* PCM or PCM8 */
                    if ((g_generalDesc_20->bmFormats[0] & 0x01U) || (g_generalDesc_20->bmFormats[0] & 0x02U))
                    {
                        pcm_format = 1U;
                    }
                }
            }
            else
            {
                /*TO DO*/
            }
            if (0U == pcm_format)
            {
                g_audioRecorder.runState     = kUSB_HostAudioRunIdle;
                g_audioRecorder.runWaitState = kUSB_HostAudioRunIdle;
                usb_echo("Do not support this device, USB Host only supports PCM/PCM8 format\r\n");
                return;
            }
            usb_echo("Enter character 'r' to start recording or 's' to stop recording\r\n");
            g_audioRecorder.audioFileIndex = 0;
            g_audioRecorder.runState       = kUSB_HostAudioRunAudioWaitCommand;
        case kUSB_HostAudioRunAudioWaitCommand:
            g_audioRecorder.audioRecorderRunFlag              = 0U;
            g_audioRecorder.audioRecorderWriteSdcardFlag      = 0U;
            g_audioRecorder.audioRecorderStreamUsbBufferIndex = 0U;
            g_audioRecorder.audioRecorderStreamPutBufferIndex = 0U;
            g_audioRecorder.audioRecorderStreamGetBufferIndex = 0U;
            g_audioRecorder.recorderUsedSpace                 = 0U;
            USB_HostAudioRecorderWaitCommand();
            if (1U == g_audioRecorder.audioRecorderRunFlag)
            {
                /* prime total transfers */
                for (uint8_t i = 0; i < USB_AUDIO_RECORDER_STREAM_PRIME_COUNT; i++)
                {
                    USB_HostAudioStreamRecv(
                        g_audioRecorder.classHandle, (unsigned char *)&s_audioRecordStreamBuffer[i][0],
                        g_audioRecorder.audioRecorderIsoMaxPacketSize, Audio_RecorderInCallback, &g_audioRecorder);
                }
                g_audioRecorder.runState                     = kUSB_HostAudioRecorderStartWriteSdcard;
                g_audioRecorder.audioRecorderWriteSdcardFlag = 1;
                usb_echo("Recording starts...\r\n");
            }
            break;
        case kUSB_HostAudioRecorderStartWriteSdcard:
            USB_HostAudioRecorderWaitCommand();
            USB_HostAudioRecorderWriteSdcard();
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
    if (g_audioRecorder.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Recorder is not connected\n\r");
        return;
    }

    if (g_audioRecorder.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Recorder is not Ready\n\r");
        return;
    }

    g_audioRecorder.runState = kUSB_HostAudioRunAudioConfigMute;
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
    if (g_audioRecorder.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Recorder is not connected\n\r");
        return;
    }

    if (g_audioRecorder.deviceIsUsed == 0)
    {
        usb_echo("  err: Audio Recorder is not Ready\n\r");
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

    g_audioRecorder.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
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
    max_audio_channel = g_pFormatTypeDesc->bnrchannels;
    if (channel > max_audio_channel)
    {
        usb_echo("  err: Channel number larger than max channel\n\r");
        return;
    }
    if (g_audioRecorder.devState > kStatus_DEV_Detached)
    {
        usb_echo("  err: Audio Recorder is not connected\n\r");
        return;
    }

    if (g_audioRecorder.deviceIsUsed == 0U)
    {
        usb_echo("  err: Audio Recorder is not Ready\n\r");
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

    g_audioRecorder.runState = kUSB_HostAudioRunAudioConfigChannel1Vol;
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
            configuration_ptr                 = (usb_host_configuration_t *)configurationHandle;
            g_audioRecorderDeviceHandle       = NULL;
            g_audioRecorderOutControlifHandle = NULL;
            g_audioRecorderOutStreamifHandle  = NULL;
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
                    g_audioRecorderOutControlifHandle = interface_ptr;
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
                                USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN)
                            {
                                g_audioRecorderDeviceHandle      = deviceHandle;
                                g_audioRecorderOutStreamifHandle = interface_ptr;
                                break;
                            }
                        }
                    }

                    if (g_audioRecorderDeviceHandle != NULL)
                    {
                        break;
                    }
                }
                else
                {
                    continue;
                }
            }
            if (g_audioRecorderDeviceHandle != NULL)
            {
                return kStatus_USB_Success;
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_audioRecorder.devState == kStatus_DEV_Idle)
            {
                if ((g_audioRecorderDeviceHandle != NULL) && (g_audioRecorderOutControlifHandle != NULL))
                {
                    g_audioRecorder.devState          = kStatus_DEV_Attached;
                    g_audioRecorder.deviceHandle      = g_audioRecorderDeviceHandle;
                    g_audioRecorder.controlIntfHandle = g_audioRecorderOutControlifHandle;
                    g_audioRecorder.streamIntfHandle  = g_audioRecorderOutStreamifHandle;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("audio generator attached:pid=0x%x", info_value);
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
            if (g_audioRecorder.devState != kStatus_DEV_Idle)
            {
                g_audioRecorder.devState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}

/*!
 * @brief host audio sdcard initialization function.
 *
 * This function implements the sdcard detect and initialization
 *
 */
usb_status_t USB_HostAudioAppSDcardInit(void)
{
    FATFS *fs;
    FIL fileObj;
    uint32_t freeClusterNumber;
    uint32_t freeMemorySizeMB;
    char fileName[25];

    usb_echo("please insert SD card\r\n");
    if (f_mount(&g_fileSystem, g_DriverNumberBuffer, 1U))
    {
        usb_echo("f_mount failed.\r\n");
        return kStatus_USB_Error;
    }
    else
    {
        usb_echo("sdcard inserted\r\n");
    }

    if (f_getfree(&g_DriverNumberBuffer[0], (DWORD *)&freeClusterNumber, &fs))
    {
        usb_echo("f_getfree failed.\r\n");
        return kStatus_USB_Error;
    }
    else
    {
        freeMemorySizeMB = freeClusterNumber * (fs->csize) / 2048;
        usb_echo("sdcard free size: %d MB\r\n", freeMemorySizeMB);
    }

    /* this operation is to make sure the sdcard and fatfs system is ready fully */
    sprintf(fileName, "%saudio.txt", &g_DriverNumberBuffer[0]);
    if (f_open(&fileObj, _T(fileName), FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        f_close(&fileObj);
        (void)f_unlink(fileName);
    }
    return kStatus_USB_Success;
}
