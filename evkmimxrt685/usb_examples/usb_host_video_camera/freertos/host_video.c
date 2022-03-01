/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_video.h"
#include "host_video.h"
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
/*!
 * @brief usb host video command complete callback.
 *
 * This function is used as callback function for completed command.
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
static void USB_HostVideoCommandCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status);

/*!
 * @brief usb host video control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
static void USB_HostVideoControlCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status);

/*!
 * @brief host video stream iso in transfer callback.
 *
 * This function is used as callback function when call USB_HosVideoStreamRecv .
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
static void USB_HostVideoStreamDataInCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usb_device_handle g_VideoDeviceHandle;
usb_host_video_camera_instance_t g_Video;
/*the stream interface handle , this handle is init in the class init function*/
usb_host_interface_handle g_VideoStreamInterfaceHandle;
/*the control interface handle , this handle is init in the class init function*/
usb_host_interface_handle g_VideoControlInterfaceHandle;
/* the current video camera's probe value */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_host_video_probe_commit_controls_t s_VideoProbe;
/* probe buffer to transfer */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_host_video_probe_commit_controls_t s_Probe[3];
/* usb video stream transfer buffer */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_streamBuffer[USB_VIDEO_STREAM_BUFFER_COUNT][HIGH_SPEED_ISO_MAX_PACKET_SIZE_ZERO_ADDITION];
/* the pointer to picture ping/pong buffer */
static uint32_t *s_pictureBuffer[2];
/* byte length for picture ping/pong buffer which indicates the actual picture size */
static uint32_t s_pictureBufferDataLength[2] = {0};
/* picture index, the index for the first saved picture is 1 */
static volatile int s_PictureIndex = 1;

/* GUID, Globally Unique Identifier used to identify stream-encoding format */
const uint8_t g_YUY2FormatGUID[] = {0x59, 0x55, 0x59, 0x32, 0x00, 0x00, 0x10, 0x00,
                                    0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};
const uint8_t g_NV12FormatGUID[] = {0x4E, 0x56, 0x31, 0x32, 0x00, 0x00, 0x10, 0x00,
                                    0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};
const uint8_t g_M420FormatGUID[] = {0x4D, 0x34, 0x32, 0x30, 0x00, 0x00, 0x10, 0x00,
                                    0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};
const uint8_t g_I420FormatGUID[] = {0x49, 0x34, 0x32, 0x30, 0x00, 0x00, 0x10, 0x00,
                                    0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71};

/* File system object */
static FATFS g_fileSystem;
const TCHAR g_DriverNumberBuffer[] = {SDDISK + '0', ':', '/', '\0'};
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief get proper endpoint maxpacksize and the subordinate interface alternate.
 *
 * This function is used select proper interface alternate setting according to the expected max packet size
 *
 * @param classHandle   the class handle.
 * @param expectMaxPacketSize   the expected max packet size
 * @param interfaceAlternate   the pointer to the selected interface alternate setting
 * @param unMultipleIsoPacketSize  the actually used endpoint max packet size
 *
 * @retval kStatus_USB_Success        successfully get the proper interface aternate setting
 * @retval kStatus_USB_InvalidHandle  The streamInterface is NULL pointer.
 * @retval kStatus_USB_Error          There is no proper interface alternate
 */
static uint8_t USB_HostGetProperEndpointInterfaceInfo(usb_host_class_handle classHandle,
                                                      uint16_t expectMaxPacketSize,
                                                      uint8_t *interfaceAlternate,
                                                      uint16_t *unMultipleIsoPacketSize)
{
    usb_host_video_instance_struct_t *videoInstance = (usb_host_video_instance_struct_t *)classHandle;
    usb_host_interface_t *streamInterface;
    streamInterface = (usb_host_interface_t *)videoInstance->streamIntfHandle;
    usb_host_video_descriptor_union_t descriptor;
    uint8_t epCount   = 0;
    uint16_t length   = 0;
    uint8_t alternate = 0;

    if (NULL == streamInterface)
    {
        return kStatus_USB_InvalidHandle;
    }
    if (NULL == streamInterface->interfaceDesc)
    {
        return kStatus_USB_InvalidParameter;
    }

    descriptor.bufr = streamInterface->interfaceExtension;
    length          = 0;
    while (length < streamInterface->interfaceExtensionLength)
    {
        if (descriptor.common->bDescriptorType == USB_DESCRIPTOR_TYPE_INTERFACE)
        {
            break;
        }
        length += descriptor.common->bLength;
        descriptor.bufr += descriptor.common->bLength;
    }

    while (length < streamInterface->interfaceExtensionLength)
    {
        if (descriptor.common->bDescriptorType == USB_DESCRIPTOR_TYPE_INTERFACE)
        {
            alternate = descriptor.interface->bAlternateSetting;
            epCount   = descriptor.interface->bNumEndpoints;
            while (epCount)
            {
                if (descriptor.endpoint->bDescriptorType == USB_DESCRIPTOR_TYPE_ENDPOINT)
                {
                    if ((USB_SHORT_FROM_LITTLE_ENDIAN_DATA(descriptor.endpoint->wMaxPacketSize) <=
                         expectMaxPacketSize) &&
                        (*unMultipleIsoPacketSize <
                         USB_SHORT_FROM_LITTLE_ENDIAN_DATA(descriptor.endpoint->wMaxPacketSize)))
                    {
                        *unMultipleIsoPacketSize =
                            USB_SHORT_FROM_LITTLE_ENDIAN_DATA(descriptor.endpoint->wMaxPacketSize);
                        /* save interface alternate for the current proper endpoint */
                        *interfaceAlternate = alternate;
                    }
                    epCount--;
                }
                length += descriptor.common->bLength;
                descriptor.bufr += descriptor.common->bLength;
            }
        }
        else
        {
            length += descriptor.common->bLength;
            descriptor.bufr += descriptor.common->bLength;
        }
    }
    if ((0 == *unMultipleIsoPacketSize) || (0 == *interfaceAlternate))
    {
        return kStatus_USB_Error;
    }
    else
    {
        return kStatus_USB_Success;
    }
}

/*!
 * @brief usb host video command complete callback.
 *
 * This function is used as callback function for completed command.
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
static void USB_HostVideoCommandCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    usb_host_video_camera_instance_t *videoInstance = (usb_host_video_camera_instance_t *)param;
    videoInstance->isControlTransferring            = 0;
}

/*!
 * @brief usb host video control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status         transfer result status.
 */
static void USB_HostVideoControlCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    usb_host_video_camera_instance_t *videoInstance = (usb_host_video_camera_instance_t *)param;
    if (status != kStatus_USB_Success)
    {
        usb_echo("data transfer error = %d , status \r\n");
        return;
    }

    if (videoInstance->runState == kUSB_HostVideoRunIdle)
    {
        if (videoInstance->runWaitState == kUSB_HostVideoRunSetControlInterface)
        {
            videoInstance->runState = kUSB_HostVideoRunSetControlInterfaceDone;
        }
        else if (videoInstance->runWaitState == kUSB_HostVideoRunWaitSetStreamInterface)
        {
            videoInstance->runState = kUSB_HostVideoRunSetInterfaceDone;
        }
        else if (videoInstance->runWaitState == kUSB_HostVideoRunWaitGetSetProbeCommit)
        {
            videoInstance->runState = kUSB_HostVideoRunGetSetProbeCommitDone;
        }
    }
}

static void USB_HostVideoSdcardBufferState(usb_host_video_camera_instance_t *videoAppInstance,
                                           uint8_t index,
                                           uint8_t state)
{
    OSA_SR_ALLOC();
    OSA_ENTER_CRITICAL();
    videoAppInstance->pictureBufferState[index] = state;
    OSA_EXIT_CRITICAL();
}

/*!
 * @brief host video stream iso in transfer callback.
 *
 * This function is used as callback function when call USB_HosVideoStreamRecv .
 *
 * @param param    the host video instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
static void USB_HostVideoStreamDataInCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    usb_host_video_camera_instance_t *videoAppInstance = (usb_host_video_camera_instance_t *)param;
    uint32_t headLength;
    uint32_t dataSize;
    uint8_t endOfFrame;
    uint8_t frame_id;
    uint32_t presentationTime;
    static uint8_t s_savePicture            = 0;
    static uint32_t s_currentFrameTimeStamp = 0;
    OSA_SR_ALLOC();

    if (videoAppInstance->devState != kStatus_DEV_Attached)
    {
        return;
    }

    endOfFrame = ((usb_host_video_payload_header_t *)s_streamBuffer[videoAppInstance->streamBufferIndex])
                     ->HeaderInfo.bitMap.end_of_frame;
    frame_id = ((usb_host_video_payload_header_t *)s_streamBuffer[videoAppInstance->streamBufferIndex])
                   ->HeaderInfo.bitMap.frame_id;
    presentationTime = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
        ((usb_host_video_payload_header_t *)s_streamBuffer[videoAppInstance->streamBufferIndex])->dwPresentationTime);

    headLength =
        ((usb_host_video_payload_header_t *)s_streamBuffer[videoAppInstance->streamBufferIndex])->bHeaderLength;

    if (dataLen > 0)
    {
        /* the standard header is 12 bytes */
        if (dataLen > 11)
        {
            dataSize = dataLen - headLength;
            /* there is payload for this transfer */
            if (dataSize)
            {
                if (s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] == 0U)
                {
                    if (s_currentFrameTimeStamp != presentationTime)
                    {
                        s_currentFrameTimeStamp = presentationTime;
                    }
                }
                /* presention time should be the same for the same frame, if not, discard this picture */
                else if (presentationTime != s_currentFrameTimeStamp)
                {
                    s_savePicture = 0;
                }
                /* the current picture buffers are not available, now discard the receiving picture */
                if ((videoAppInstance->pictureBufferState[videoAppInstance->pictureBufferIndex] && s_savePicture))
                {
                    s_savePicture = 0;
                }
                else if (s_savePicture)
                {
                    if (dataSize > (videoAppInstance->videoCameraPictureBufferSize -
                                    s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex])) /* error here */
                    {
                        s_savePicture                                                   = 0;
                        s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] = 0U;
                    }
                    else
                    {
                        /* the same frame id indicates they belong to the same frame */
                        if (frame_id == videoAppInstance->expect_frame_id)
                        {
                            /* copy data to picture buffer */
                            memcpy((void *)(((uint8_t *)&s_pictureBuffer[videoAppInstance->pictureBufferIndex][0]) +
                                            s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex]),
                                   (void *)(((uint8_t *)&s_streamBuffer[videoAppInstance->streamBufferIndex][0]) +
                                            headLength),
                                   dataSize);
                            OSA_ENTER_CRITICAL();
                            s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] += dataSize;
                            OSA_EXIT_CRITICAL();
                        }
                        else /* for the payload that has different frame id, discard it */
                        {
                            s_savePicture = 0;
                        }
                    }
                }
                else
                {
                    /* no action */
                }
            }

            if (s_savePicture)
            {
                if (endOfFrame)
                {
                    if (s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] != 0)
                    {
                        USB_HostVideoSdcardBufferState(videoAppInstance, videoAppInstance->pictureBufferIndex, 1);
                        videoAppInstance->sdcardPictureLength[videoAppInstance->pictureBufferIndex] =
                            s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex];
                        /* toggle the expected frame id */
                        videoAppInstance->expect_frame_id = 1 - videoAppInstance->expect_frame_id;
                        s_savePicture                     = 1;
                        s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] = 0U;
                        /* switch to another buffer to save picture frame */
                        videoAppInstance->pictureBufferIndex = 1 - videoAppInstance->pictureBufferIndex;
                    }
                }
            }
            else
            {
                /* the last frame of one picture */
                if (endOfFrame)
                {
                    if (videoAppInstance->discardFirstPicture)
                    {
                        videoAppInstance->discardFirstPicture = 0;
                        videoAppInstance->expect_frame_id     = 1 - frame_id;
                    }

                    if (s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] != 0)
                    {
                        videoAppInstance->discardPictureCount++;
                    }
                    s_pictureBufferDataLength[videoAppInstance->pictureBufferIndex] = 0U;
                    if (!videoAppInstance->discardFirstPicture)
                    {
                        videoAppInstance->expect_frame_id = 1 - frame_id;
                    }
                    s_savePicture = 1;
                }
            }
        }
    }

    /* prime transfer for this usb stream buffer again */
    USB_HosVideoStreamRecv(
        videoAppInstance->classHandle, (uint8_t *)&s_streamBuffer[videoAppInstance->streamBufferIndex][0],
        videoAppInstance->unMultipleIsoMaxPacketSize, USB_HostVideoStreamDataInCallback, videoAppInstance);
    videoAppInstance->streamBufferIndex++;
    if (videoAppInstance->streamBufferIndex == USB_VIDEO_STREAM_BUFFER_COUNT)
    {
        videoAppInstance->streamBufferIndex = 0;
    }
}

/*!
 * @brief write picture into sd card.
 */
static void USB_HostVideoWriteSDCard(void *param)
{
    usb_host_video_camera_instance_t *videoAppInstance = (usb_host_video_camera_instance_t *)param;
    static uint8_t s_FileOpened                        = 0;
    static FIL fileObj;
    char fileName[32];
    FRESULT fileStatus;
    uint32_t writeSize = 0;
    OSA_SR_ALLOC();

    /* sdcard write picture buffer has had picture */
    if (videoAppInstance->pictureBufferState[videoAppInstance->sdcardWriteBufferIndex])
    {
        if (!s_FileOpened)
        {
            if (s_PictureIndex > USB_VIDEO_PICTURE_COUNT) /* finished pictures, start overlay from the first pic */
            {
                s_PictureIndex = 1;
            }
            if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG)
            {
                sprintf(fileName, "%spic%d.jpg", &g_DriverNumberBuffer[0], s_PictureIndex);
            }
            else if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED)
            {
                sprintf(fileName, "%spic%d.yuv", &g_DriverNumberBuffer[0], s_PictureIndex);
            }
            else
            {
                return;
            }

            fileStatus = f_open(&fileObj, _T(fileName), FA_WRITE | FA_CREATE_ALWAYS);
            if (FR_OK != fileStatus)
            {
                usb_echo("sdcard operate fail\r\n");
                return;
            }
            s_FileOpened = 1;
        }

        fileStatus = f_write(&fileObj, (uint8_t *)&s_pictureBuffer[videoAppInstance->sdcardWriteBufferIndex][0],
                             videoAppInstance->sdcardPictureLength[videoAppInstance->sdcardWriteBufferIndex],
                             (UINT *)&writeSize);
        if ((fileStatus != FR_OK) ||
            (videoAppInstance->sdcardPictureLength[videoAppInstance->sdcardWriteBufferIndex] != writeSize))
        {
            usb_echo("sdcard operate fail\r\n");
        }

        OSA_ENTER_CRITICAL();

        USB_HostVideoSdcardBufferState(videoAppInstance, videoAppInstance->sdcardWriteBufferIndex, 0);
        videoAppInstance->sdcardPictureLength[videoAppInstance->sdcardWriteBufferIndex] = 0;
        OSA_EXIT_CRITICAL();

        /* one frame picture finihsed and sync it */
        s_FileOpened = 0;
        fileStatus   = f_sync(&fileObj);
        if (FR_OK != fileStatus)
        {
            usb_echo("file sync fail\r\n");
            return;
        }
        s_PictureIndex++;

        /* toggle the sdcard write picture index */
        videoAppInstance->sdcardWriteBufferIndex = 1 - videoAppInstance->sdcardWriteBufferIndex;
    }
}

/*!
 * @brief host usb video task function.
 *
 * This function implements the host video action, it is used to create task.
 *
 * @param param  the host video instance pointer.
 */
void USB_HostVideoTask(void *param)
{
    static usb_status_t status                                   = kStatus_USB_Success;
    usb_host_video_stream_payload_frame_common_desc_t *frameDesc = NULL;
    usb_host_video_camera_instance_t *videoAppInstance           = (usb_host_video_camera_instance_t *)param;
    uint32_t index, count;
    uint32_t minResolution        = 0xFFFFFFFF;
    uint8_t minSolutionFrameIndex = 0xFF;
    uint32_t frameInterval        = 0;
    uint32_t resolution           = 0;
    uint32_t speed                = 0U;
    uint8_t i                     = 0;

    /* device state changes */
    if (videoAppInstance->devState != videoAppInstance->prevState)
    {
        videoAppInstance->prevState = videoAppInstance->devState;
        switch (videoAppInstance->devState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached:
                videoAppInstance->runState                        = kUSB_HostVideoRunSetControlInterface;
                videoAppInstance->isControlTransferring           = 0;
                videoAppInstance->isStreamTransferring            = 0;
                videoAppInstance->streamBufferIndex               = 0;
                videoAppInstance->pictureBufferIndex              = 0;
                videoAppInstance->sdcardWriteBufferIndex          = 0;
                videoAppInstance->discardPictureCount             = 0;
                videoAppInstance->pictureBufferState[0]           = 0;
                videoAppInstance->pictureBufferState[1]           = 0;
                videoAppInstance->cameraDeviceFormatType          = 0;
                videoAppInstance->unMultipleIsoMaxPacketSize      = 0;
                videoAppInstance->currentStreamInterfaceAlternate = 0;
                videoAppInstance->expect_frame_id                 = 0xFF;
                videoAppInstance->discardFirstPicture =
                    1; /* dicard the first picture because the first picture may be not complete */
                s_pictureBufferDataLength[0] = 0;
                s_pictureBufferDataLength[1] = 0;
                videoAppInstance->step       = 0;
                s_PictureIndex               = 1; /* the first picture index is 1 */
                USB_HostVideoInit(videoAppInstance->deviceHandle, &videoAppInstance->classHandle);
                usb_echo("USB video attached\r\n");
                break;

            case kStatus_DEV_Detached:
                videoAppInstance->devState     = kStatus_DEV_Idle;
                videoAppInstance->runState     = kUSB_HostVideoRunIdle;
                videoAppInstance->runWaitState = kStatus_DEV_Idle;
                /* free the two picture buffer */
                vPortFree((void *)s_pictureBuffer[0]);
                vPortFree((void *)s_pictureBuffer[1]);
                USB_HostVideoDeinit(videoAppInstance->deviceHandle, videoAppInstance->classHandle);
                videoAppInstance->classHandle = NULL;
                usb_echo("USB video detached\r\n");
                if (s_PictureIndex > 0)
                {
                    usb_echo("the last saved picture index is %d\r\n", s_PictureIndex - 1U);
                }
                usb_echo("the total discarded picture count is %d\r\n", videoAppInstance->discardPictureCount);
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (videoAppInstance->runState)
    {
        case kUSB_HostVideoRunIdle:
            break;

        case kUSB_HostVideoRunSetControlInterface:
            videoAppInstance->runWaitState = kUSB_HostVideoRunSetControlInterface;
            videoAppInstance->runState     = kUSB_HostVideoRunIdle;
            if (USB_HostVideoControlSetInterface(videoAppInstance->classHandle, videoAppInstance->controlIntfHandle, 0,
                                                 USB_HostVideoControlCallback, &g_Video) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostVideoRunSetControlInterfaceDone:
            videoAppInstance->runWaitState                    = kUSB_HostVideoRunWaitSetStreamInterface;
            videoAppInstance->runState                        = kUSB_HostVideoRunIdle;
            videoAppInstance->currentStreamInterfaceAlternate = 0;
            if (USB_HostVideoStreamSetInterface(videoAppInstance->classHandle, videoAppInstance->streamIntfHandle,
                                                videoAppInstance->currentStreamInterfaceAlternate,
                                                USB_HostVideoControlCallback, &g_Video) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;
        case kUSB_HostVideoRunSetInterfaceDone:
            videoAppInstance->runState = kUSB_HostVideoRunFindOptimalSetting;
            break;
        case kUSB_HostVideoRunFindOptimalSetting:
            /* Firstly get MJPEG format descriptor */
            status = USB_HostVideoStreamGetFormatDescriptor(videoAppInstance->classHandle,
                                                            USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG,
                                                            (void *)&videoAppInstance->videoStreamFormatDescriptor);
            if (status == kStatus_USB_InvalidHandle)
            {
                usb_echo("videoAppInstance->classHandle is invalid\r\n");
            }
            else if (status == kStatus_USB_Error)
            {
                /* the camera device doesn't support MJPEG format, try to get UNCOMPRESSED format */
                status = USB_HostVideoStreamGetFormatDescriptor(videoAppInstance->classHandle,
                                                                USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED,
                                                                (void *)&videoAppInstance->videoStreamFormatDescriptor);
                if (status == kStatus_USB_InvalidHandle)
                {
                    usb_echo("videoAppInstance->classHandle is invalid\r\n");
                }
                else if (status == kStatus_USB_Error)
                {
                    usb_echo(" host can't support this format camera device\r\n");
                    videoAppInstance->runState = kUSB_HostVideoRunIdle;
                }
                else
                {
                }
            }
            else
            {
            }

            /* Successfully get MJPEG or UNCOMPRESSED format descriptor */
            if (status == kStatus_USB_Success)
            {
                count = videoAppInstance->videoStreamFormatDescriptor->bNumFrameDescriptors;
                videoAppInstance->cameraDeviceFormatType =
                    videoAppInstance->videoStreamFormatDescriptor->bDescriptorSubtype;
                if (videoAppInstance->cameraDeviceFormatType ==
                    USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG) /* camera device supports mjpeg format */
                {
                    videoAppInstance->videoStreamMjpegFormatDescriptor =
                        (usb_host_video_stream_payload_mjpeg_format_desc_t *)
                            videoAppInstance->videoStreamFormatDescriptor;
                }
                else if (videoAppInstance->cameraDeviceFormatType ==
                         USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED) /* camera device supports uncompressed format */
                {
                    videoAppInstance->videoStreamUncompressedFormatDescriptor =
                        (usb_host_video_stream_payload_uncompressed_format_desc_t *)
                            videoAppInstance->videoStreamFormatDescriptor;
                }
                /* Choose a minimum resolution video stream frame descriptor */
                for (index = 1; index <= count; index++)
                {
                    /* Get the subordinate frame descriptor */
                    if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG)
                    {
                        status = USB_HostVideoStreamGetFrameDescriptor(
                            videoAppInstance->classHandle, videoAppInstance->videoStreamFormatDescriptor,
                            USB_HOST_DESC_SUBTYPE_VS_FRAME_MJPEG, index, (void *)&frameDesc);
                    }
                    else if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED)
                    {
                        status = USB_HostVideoStreamGetFrameDescriptor(
                            videoAppInstance->classHandle, videoAppInstance->videoStreamFormatDescriptor,
                            USB_HOST_DESC_SUBTYPE_VS_FRAME_UNCOMPRESSED, index, (void *)&frameDesc);
                    }

                    /* choose a frame descriptor that has a minimum resolution */
                    if ((kStatus_USB_Success == status) && (NULL != frameDesc))
                    {
                        resolution = ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(frameDesc->wHeight))) *
                                     ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(frameDesc->wWitd)));
                        if (minResolution > resolution)
                        {
                            minResolution         = resolution;
                            minSolutionFrameIndex = index;
                            if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG)
                            {
                                videoAppInstance->videoStreamMjpegFrameDescriptor =
                                    (usb_host_video_stream_payload_mjpeg_frame_desc_t *)frameDesc;
                                videoAppInstance->videoStreamUncompressedFrameDescriptor = NULL;
                            }
                            else if (videoAppInstance->cameraDeviceFormatType ==
                                     USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED)
                            {
                                videoAppInstance->videoStreamUncompressedFrameDescriptor =
                                    (usb_host_video_stream_payload_uncompressed_frame_desc_t *)frameDesc;
                                videoAppInstance->videoStreamMjpegFrameDescriptor = NULL;
                            }
                        }
                        else
                        {
                        }
                    }
                }

                /* successfully get frame descriptor that has a minimum resolution, or go into idle state */
                if (minSolutionFrameIndex != 0xFF)
                {
                    videoAppInstance->runState = kUSB_HostVideoRunGetSetProbeCommit;
                    videoAppInstance->step     = 0;
                }
                else
                {
                    status                     = kStatus_USB_Error;
                    videoAppInstance->runState = kUSB_HostVideoRunIdle;
                }
            }
            break;
        case kUSB_HostVideoRunGetSetProbeCommit:
            switch (videoAppInstance->step)
            {
                case 0:
                    /* get the current setting of device camera */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_CUR,
                                                       (void *)&s_Probe[0], USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                    /* the maximum setting for device camera */
                case 1:
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_MAX,
                                                       (void *)&s_Probe[1], USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 2:
                    /* the minimum value for device camera */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_MIN,
                                                       (void *)&s_Probe[2], USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 3:
                    /* the seleted frame can support multiple interval, choose the maximum one */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG)
                        {
                            if (videoAppInstance->videoStreamMjpegFrameDescriptor->bFrameIntervalType > 0)
                            {
                                frameInterval = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
                                    ((uint8_t *)&(
                                         videoAppInstance->videoStreamMjpegFrameDescriptor->dwMinFrameInterval[0]) +
                                     (videoAppInstance->videoStreamMjpegFrameDescriptor->bFrameIntervalType - 1) * 4));
                            }
                            else
                            {
                                frameInterval = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
                                    ((uint8_t *)&(
                                         videoAppInstance->videoStreamMjpegFrameDescriptor->dwMinFrameInterval[0]) +
                                     4));
                            }
                            /* save the frame interval, frame index and format index */
                            USB_LONG_TO_LITTLE_ENDIAN_DATA(frameInterval, s_VideoProbe.dwFrameInterval);
                            s_VideoProbe.bFormatIndex =
                                videoAppInstance->videoStreamMjpegFormatDescriptor->bFormatIndex;
                            s_VideoProbe.bFrameIndex = videoAppInstance->videoStreamMjpegFrameDescriptor->bFrameIndex;
                        }
                        else if (videoAppInstance->cameraDeviceFormatType ==
                                 USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED)
                        {
                            if (videoAppInstance->videoStreamUncompressedFrameDescriptor->bFrameIntervalType > 0)
                            {
                                frameInterval = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS((
                                    (uint8_t *)&(videoAppInstance->videoStreamUncompressedFrameDescriptor
                                                     ->dwMinFrameInterval[0]) +
                                    (videoAppInstance->videoStreamUncompressedFrameDescriptor->bFrameIntervalType - 1) *
                                        4));
                            }
                            else
                            {
                                frameInterval = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
                                    ((uint8_t *)&(videoAppInstance->videoStreamUncompressedFrameDescriptor
                                                      ->dwMinFrameInterval[0]) +
                                     4));
                            }
                            /* save the frame interval, frame index and format index */
                            USB_LONG_TO_LITTLE_ENDIAN_DATA(frameInterval, s_VideoProbe.dwFrameInterval);
                            s_VideoProbe.bFormatIndex =
                                videoAppInstance->videoStreamUncompressedFormatDescriptor->bFormatIndex;
                            s_VideoProbe.bFrameIndex =
                                videoAppInstance->videoStreamUncompressedFrameDescriptor->bFrameIndex;
                        }
                        videoAppInstance->isControlTransferring = 1;
                        /* set device camera using the new probe */
                        status = USB_HostVideoSetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_SET_CUR,
                                                       (void *)&s_VideoProbe, USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 4:
                case 5:
                case 6:
                    /* get the current/min/max state of device camera */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        if (videoAppInstance->step == 4)
                        {
                            status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_CUR,
                                                           (void *)&s_Probe[0], USB_HostVideoCommandCallback, &g_Video);
                        }
                        else if (videoAppInstance->step == 5)
                        {
                            status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_MAX,
                                                           (void *)&s_Probe[1], USB_HostVideoCommandCallback, &g_Video);
                        }
                        else
                        {
                            status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_MIN,
                                                           (void *)&s_Probe[2], USB_HostVideoCommandCallback, &g_Video);
                        }
                        videoAppInstance->step++;
                    }
                    break;
                case 7:
                    /* do multiple get/set cur requests to make sure a stable setting is obtained */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        memcpy((void *)&s_VideoProbe, (void *)&s_Probe[0], 22);
                        status = USB_HostVideoSetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_SET_CUR,
                                                       (void *)&s_VideoProbe, USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 8:
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_CUR,
                                                       (void *)&s_Probe[0], USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 9:
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        memcpy((void *)&s_VideoProbe, (void *)&s_Probe[0], 26);
                        status = USB_HostVideoSetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_SET_CUR,
                                                       (void *)&s_VideoProbe, USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 10:
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        status = USB_HostVideoGetProbe(videoAppInstance->classHandle, USB_HOST_VIDEO_GET_CUR,
                                                       (void *)&s_Probe[0], USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 11:
                    /* configure the hardware with the negotiated parameters */
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        videoAppInstance->isControlTransferring = 1;
                        memcpy((void *)&s_VideoProbe, (void *)&s_Probe[0], 26);
                        /* delay 20ms to make sure device is ready and then configure actually */
                        SDK_DelayAtLeastUs(20000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
                        status = USB_HostVideoSetCommit(videoAppInstance->classHandle, USB_HOST_VIDEO_SET_CUR,
                                                        (void *)&s_VideoProbe, USB_HostVideoCommandCallback, &g_Video);
                        videoAppInstance->step++;
                    }
                    break;
                case 12:
                    if (videoAppInstance->isControlTransferring == 0)
                    {
                        USB_HostHelperGetPeripheralInformation(videoAppInstance->deviceHandle, kUSB_HostGetDeviceSpeed,
                                                               &speed);
                        /* According to the device speed mode, choose a proper video stream interface */
                        videoAppInstance->unMultipleIsoMaxPacketSize = 0;
                        if (USB_SPEED_FULL == speed)
                        {
                            /* if device is full speed, the selected endpoint's maxPacketSize is maximum device supports
                             * and is not more than 1023B */
                            USB_HostGetProperEndpointInterfaceInfo(videoAppInstance->classHandle,
                                                                   FULL_SPEED_ISO_MAX_PACKET_SIZE,
                                                                   &videoAppInstance->currentStreamInterfaceAlternate,
                                                                   &videoAppInstance->unMultipleIsoMaxPacketSize);
                        }
                        else if (USB_SPEED_HIGH == speed)
                        {
                            /* if device is high speed, the selected endpoint's maxPacketSize is maximum device supports
                             * and is not more than 1024B */
                            USB_HostGetProperEndpointInterfaceInfo(videoAppInstance->classHandle,
                                                                   HIGH_SPEED_ISO_MAX_PACKET_SIZE_ZERO_ADDITION,
                                                                   &videoAppInstance->currentStreamInterfaceAlternate,
                                                                   &videoAppInstance->unMultipleIsoMaxPacketSize);
                        }

                        if (videoAppInstance->currentStreamInterfaceAlternate)
                        {
                            /* set interface by the proper alternate setting */
                            videoAppInstance->runWaitState = kUSB_HostVideoRunWaitGetSetProbeCommit;
                            videoAppInstance->runState     = kUSB_HostVideoRunIdle;
                            if (USB_HostVideoStreamSetInterface(
                                    videoAppInstance->classHandle, videoAppInstance->streamIntfHandle,
                                    videoAppInstance->currentStreamInterfaceAlternate, USB_HostVideoControlCallback,
                                    &g_Video) != kStatus_USB_Success)
                            {
                                usb_echo("set interface error\r\n");
                            }
                        }
                        else
                        {
                            /* no proper alternate setting */
                            videoAppInstance->runWaitState = kUSB_HostVideoRunIdle;
                            videoAppInstance->runState     = kUSB_HostVideoRunIdle;
                            usb_echo("no proper alternate setting\r\n");
                        }
                    }
                    break;
                default:
                    break;
            }
            break;
        case kUSB_HostVideoRunGetSetProbeCommitDone:
        {
            OSA_SR_ALLOC();

            /* malloc enough buffer for one raw data or mjpeg picture */
            if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_MJPEG)
            {
                /* print the camera device format info*/
                usb_echo("Camera setting is: %d(w)*%d(h)@%dfps\r\n",
                         ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                             videoAppInstance->videoStreamMjpegFrameDescriptor->wWitd))),
                         ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                             videoAppInstance->videoStreamMjpegFrameDescriptor->wHeight))),
                         10000000 / USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(s_VideoProbe.dwFrameInterval));
                /* the picture buffer byte size */
                videoAppInstance->videoCameraPictureBufferSize =
                    ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                        videoAppInstance->videoStreamMjpegFrameDescriptor->wHeight))) *
                    ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                        videoAppInstance->videoStreamMjpegFrameDescriptor->wWitd)));
                /* consider the compression rate of MJPEG, allocate a reasonable size, compression rate can be changed
                 * according to the actual situation */
                videoAppInstance->videoCameraPictureBufferSize = videoAppInstance->videoCameraPictureBufferSize / 100U;
                videoAppInstance->videoCameraPictureBufferSize =
                    videoAppInstance->videoCameraPictureBufferSize * USB_MJPEG_COMPRESSION_RATIO;
                s_pictureBuffer[0] = (uint32_t *)pvPortMalloc(videoAppInstance->videoCameraPictureBufferSize);
                s_pictureBuffer[1] = (uint32_t *)pvPortMalloc(videoAppInstance->videoCameraPictureBufferSize);
                usb_echo("picture format is MJPEG\r\n");
            }
            else if (videoAppInstance->cameraDeviceFormatType == USB_HOST_DESC_SUBTYPE_VS_FORMAT_UNCOMPRESSED)
            {
                usb_echo("Camera setting is: %d(w)*%d(h)@%dfps\r\n",
                         ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                             videoAppInstance->videoStreamUncompressedFrameDescriptor->wWitd))),
                         ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                             videoAppInstance->videoStreamUncompressedFrameDescriptor->wHeight))),
                         10000000 / USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(s_VideoProbe.dwFrameInterval));
                /* Uncompressed format has no compression */
                videoAppInstance->videoCameraPictureBufferSize =
                    ((uint32_t)(videoAppInstance->videoStreamUncompressedFormatDescriptor->bBitsPerPixel / 8U *
                                USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                                    videoAppInstance->videoStreamUncompressedFrameDescriptor->wHeight))) *
                    ((uint32_t)(USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(
                        videoAppInstance->videoStreamUncompressedFrameDescriptor->wWitd)));
                /* allocate one more max packet size buffer to avoid array out of bounds access  */
                videoAppInstance->videoCameraPictureBufferSize += videoAppInstance->unMultipleIsoMaxPacketSize;
                s_pictureBuffer[0] = (uint32_t *)pvPortMalloc(videoAppInstance->videoCameraPictureBufferSize);
                s_pictureBuffer[1] = (uint32_t *)pvPortMalloc(videoAppInstance->videoCameraPictureBufferSize);

                usb_echo("picture format is ");
                if (memcmp(&videoAppInstance->videoStreamUncompressedFormatDescriptor->guidFormat[0],
                           &g_YUY2FormatGUID[0], 16) == 0)
                {
                    usb_echo("YUY2\r\n");
                }
                else if (memcmp(&videoAppInstance->videoStreamUncompressedFormatDescriptor->guidFormat[0],
                                &g_NV12FormatGUID[0], 16) == 0)
                {
                    usb_echo("NV12\r\n");
                }
                else if (memcmp(&videoAppInstance->videoStreamUncompressedFormatDescriptor->guidFormat[0],
                                &g_M420FormatGUID[0], 16) == 0)
                {
                    usb_echo("M420\r\n");
                }
                else if (memcmp(&videoAppInstance->videoStreamUncompressedFormatDescriptor->guidFormat[0],
                                &g_I420FormatGUID[0], 16) == 0)
                {
                    usb_echo("I420\r\n");
                }
                else
                {
                    /* directly print GUID*/
                    for (uint8_t index = 0; index < 16; ++index)
                    {
                        usb_echo("%x ", videoAppInstance->videoStreamUncompressedFormatDescriptor->guidFormat[index]);
                    }
                    usb_echo("\r\n");
                }
            }
            if ((s_pictureBuffer[0] == NULL) || (s_pictureBuffer[1] == NULL))
            {
                usb_echo(
                    "picture buffer malloc failed, please make sure the heap size is enough. If it is raw data now, "
                    "please use other format like MJPEG \r\n");
                videoAppInstance->runState     = kUSB_HostVideoRunIdle;
                videoAppInstance->runWaitState = kUSB_HostVideoRunIdle;
                return;
            }

            /* delay to make sure the device camera is ready, delay 2ms */
            SDK_DelayAtLeastUs(2000 * 1000U, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            usb_echo("Start getting %d pictures from device camera and write them into sdcard\r\n",
                     USB_VIDEO_PICTURE_COUNT);
            usb_echo("usb host firstly choose mjepg format, if device can't support, will choose raw data format\r\n");
            usb_echo("if finish %d pictures, overlay from the first picture\r\n", USB_VIDEO_PICTURE_COUNT);
            OSA_ENTER_CRITICAL();
            /* prime multiple transfers */
            for (i = 0; i < USB_VIDEO_STREAM_BUFFER_COUNT; i++)
            {
                USB_HosVideoStreamRecv(videoAppInstance->classHandle, (uint8_t *)&s_streamBuffer[i][0],
                                       videoAppInstance->unMultipleIsoMaxPacketSize, USB_HostVideoStreamDataInCallback,
                                       &g_Video);
            }
            OSA_EXIT_CRITICAL();
            videoAppInstance->runState = kUSB_HostVideoStart;
            break;
        }
        case kUSB_HostVideoStart:
            USB_HostVideoWriteSDCard(param);
            break;
        default:
            break;
    }
}

usb_status_t USB_HostVideoEvent(usb_device_handle deviceHandle,
                                usb_host_configuration_handle configurationHandle,
                                uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    uint8_t id;
    usb_host_configuration_t *configuration;
    uint8_t interface_index;
    usb_host_interface_t *hostInterface;
    uint32_t info_value = 0U;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration                 = (usb_host_configuration_t *)configurationHandle;
            g_VideoDeviceHandle           = NULL;
            g_VideoControlInterfaceHandle = NULL;
            g_VideoStreamInterfaceHandle  = NULL;

            for (interface_index = 0U; interface_index < configuration->interfaceCount; ++interface_index)
            {
                hostInterface = &configuration->interfaceList[interface_index];
                id            = hostInterface->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_VIDEO_CLASS_CODE)
                {
                    continue;
                }
                id = hostInterface->interfaceDesc->bInterfaceSubClass;
                if (id == USB_HOST_VIDEO_SUBCLASS_CODE_CONTROL)
                {
                    g_VideoDeviceHandle           = deviceHandle;
                    g_VideoControlInterfaceHandle = hostInterface;
                    continue;
                }
                else if (id == USB_HOST_VIDEO_SUBCLASS_CODE_STREAM)
                {
                    g_VideoDeviceHandle          = deviceHandle;
                    g_VideoStreamInterfaceHandle = hostInterface;
                    continue;
                }
                else
                {
                }
            }
            if (g_VideoDeviceHandle != NULL)
            {
                return kStatus_USB_Success;
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_Video.devState == kStatus_DEV_Idle)
            {
                if ((g_VideoDeviceHandle != NULL) && (g_VideoControlInterfaceHandle != NULL))
                {
                    g_Video.devState          = kStatus_DEV_Attached;
                    g_Video.deviceHandle      = g_VideoDeviceHandle;
                    g_Video.controlIntfHandle = g_VideoControlInterfaceHandle;
                    g_Video.streamIntfHandle  = g_VideoStreamInterfaceHandle;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("video device attached:pid=0x%x", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &info_value);
                    usb_echo("vid=0x%x ", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &info_value);
                    usb_echo("address=%d\r\n", info_value);
                }
            }
            else
            {
                usb_echo("not idle vide instance\r\n");
            }
            break;

        case kUSB_HostEventDetach:
            if (g_Video.devState != kStatus_DEV_Idle)
            {
                g_Video.devState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}

/*!
 * @brief host video sdcard initialization function.
 *
 * This function implements the sdcard detect and initialization
 *
 */
usb_status_t USB_HostVideoAppSDcardInit(void)
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
    sprintf(fileName, "%svideo.txt", &g_DriverNumberBuffer[0]);
    if (f_open(&fileObj, _T(fileName), FA_WRITE | FA_CREATE_ALWAYS) == FR_OK)
    {
        f_close(&fileObj);
        (void)f_unlink(fileName);
    }
    return kStatus_USB_Success;
}
