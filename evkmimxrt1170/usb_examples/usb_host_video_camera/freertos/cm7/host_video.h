/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HOST_VIDEO_H_
#define _HOST_VIDEO_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define FULL_SPEED_ISO_MAX_PACKET_SIZE               (1023U)
#define HIGH_SPEED_ISO_MAX_PACKET_SIZE_ZERO_ADDITION (1024U)

#define USB_VIDEO_PICTURE_COUNT (500U) /*!< the picture count need to be saved */
#define USB_VIDEO_STREAM_BUFFER_COUNT \
    (6U) /*!< the prime count, the value shouldn't be more than 6 because of USB dedicated ram limination */
#define USB_MJPEG_COMPRESSION_RATIO \
    (100U) /*!< the approximate mjep picture compression radio of device camera, 50 means 50% */

/*! @brief host app run status */
typedef enum _usb_host_vidio_run_state
{
    kUSB_HostVideoRunIdle = 0,                /*!< idle */
    kUSB_HostVideoRunSetControlInterface,     /*!< execute set control interface code */
    kUSB_HostVideoRunSetControlInterfaceDone, /*!< set control interface done */
    kUSB_HostVideoRunWaitSetControlInterface, /*!< wait control interface done */
    kUSB_HostVideoRunWaitSetStreamInterface,  /*!< wait steam interface done */
    kUSB_HostVideoRunSetInterfaceDone,        /*!< set interface done */
    kUSB_HostVideoRunFindOptimalSetting,      /*!< find optimal setting */
    kUSB_HostVideoRunGetSetProbeCommit,       /*!< set or get probe commit start*/
    kUSB_HostVideoRunWaitGetSetProbeCommit,   /*!< wait set or get probe commit done */
    kUSB_HostVideoRunGetSetProbeCommitDone,   /*!< set or get probe commit done */
    kUSB_HostVideoStart,                      /*!< start video camera */
    kUSB_HostVideoRunVideoDone,               /*!< video done */

} usb_host_video_camera_run_state_t;

/*! @brief USB host video camera instance structure */
typedef struct _usb_host_video_camera_instance
{
    usb_device_handle deviceHandle;              /*!< the video camera device handle */
    usb_host_class_handle classHandle;           /*!< the video camera class handle */
    usb_host_interface_handle controlIntfHandle; /*!< the video camera control interface handle */
    usb_host_interface_handle streamIntfHandle;  /*!< the video camera stream interface handle */
    usb_host_video_stream_payload_format_common_desc_t
        *videoStreamFormatDescriptor; /*!< the video camera stream common format descriptor pointer */
    usb_host_video_stream_payload_mjpeg_format_desc_t
        *videoStreamMjpegFormatDescriptor; /*!< the video camera stream mjpeg format descriptor pointer */
    usb_host_video_stream_payload_uncompressed_format_desc_t
        *videoStreamUncompressedFormatDescriptor; /*!< the video camera stream uncompressed format descriptor pointer */
    usb_host_video_stream_payload_frame_common_desc_t
        *videoStreamFrameDescriptor; /*!< the video camera stream common frame descriptor pointer */
    usb_host_video_stream_payload_mjpeg_frame_desc_t
        *videoStreamMjpegFrameDescriptor; /*!< the video camera stream mjpeg frame descriptor pointer */
    usb_host_video_stream_payload_uncompressed_frame_desc_t
        *videoStreamUncompressedFrameDescriptor; /*!< the video camera stream uncompressed frame descriptor pointer */
    uint32_t videoCameraPictureBufferSize;       /*!< the buffer size that is used to save one picture */
    volatile uint32_t discardPictureCount;       /*!< the discard picture count */
    uint16_t unMultipleIsoMaxPacketSize;         /*!< the packet size of using iso endpoint (<=1024)*/
    uint8_t cameraDeviceFormatType;              /*!< the current device format type */
    uint8_t devState;                            /*!< the device attach/detach status */
    uint8_t prevState;                           /*!< the device attach/detach previous status */
    uint8_t runState;                            /*!< the video camera application run status */
    uint8_t runWaitState;                        /*!< the video camera application run wait status */
    volatile uint8_t isControlTransferring;      /*!< the control transfer is processing */
    volatile uint8_t isStreamTransferring;       /*!< the stream transfer is processing */
    uint8_t currentStreamInterfaceAlternate;     /*!< the alternate of stream interface that include used endpoint */
    volatile uint8_t pictureBufferState[2];      /*!< the picture buffer state */
    volatile uint8_t pictureBufferIndex;         /*!< the picture buffer index */
    volatile uint8_t sdcardWriteBufferIndex;     /*!< the picture buffer index that is written into sd card */
    volatile uint8_t streamBufferIndex;          /*!< the prime buffer index */
    uint32_t sdcardPictureLength[2];             /*!< the picture length that write to sdcard */
    volatile uint8_t expect_frame_id;            /*!< the frame id host expects */
    uint8_t discardFirstPicture;                 /*!< dicard the first picture */
    uint8_t step;
} usb_host_video_camera_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief host video task function.
 *
 * This function implements the host video action, it is used to create task.
 *
 * @param param   the host video instance pointer.
 */
extern void USB_HostVideoTask(void *param);

/*!
 * @brief host video callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid video interface.
 */
extern usb_status_t USB_HostVideoEvent(usb_device_handle deviceHandle,
                                       usb_host_configuration_handle configurationHandle,
                                       uint32_t eventCode);

/*!
 * @brief host video sdcard initialization function.
 *
 * This function implements the sdcard detect and initialization
 *
 */
extern usb_status_t USB_HostVideoAppSDcardInit(void);

#endif /* _HOST_VIDEO_H_ */
