/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __USB_AUDIO_MICROPHONE_H__
#define __USB_AUDIO_MICROPHONE_H__ 1

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_audio.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* audio data buffer depth*/
#define AUDIO_MICROPHONE_DATA_WHOLE_BUFFER_LENGTH (32)
#define AUDIO_BUFFER_UPPER_LIMIT(x)               (((x)*5) / 8)
#define AUDIO_BUFFER_LOWER_LIMIT(x)               (((x)*3) / 8)

#ifndef CONTROLLER_ID
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#if defined(__GIC_PRIO_BITS)
#define USB_DEVICE_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_DEVICE_INTERRUPT_PRIORITY (6U)
#else
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#endif

#define MUTE_CODEC_TASK    (1UL << 0U)
#define UNMUTE_CODEC_TASK  (1UL << 1U)
#define VOLUME_CHANGE_TASK (1UL << 2U)

/* Define the types for application */
typedef struct _usb_audio_microphone_struct
{
    usb_device_handle deviceHandle; /* USB device handle. */
    class_handle_t audioHandle;     /* USB AUDIO MICROPHONE class handle. */
    uint8_t copyProtect;
    uint8_t curMute;
    uint8_t curVolume[2]; /* need to consider the endians */
    uint8_t minVolume[2]; /* need to consider the endians */
    uint8_t maxVolume[2]; /* need to consider the endians */
    uint8_t resVolume[2]; /* need to consider the endians */
    uint8_t curBass;
    uint8_t minBass;
    uint8_t maxBass;
    uint8_t resBass;
    uint8_t curMid;
    uint8_t minMid;
    uint8_t maxMid;
    uint8_t resMid;
    uint8_t curTreble;
    uint8_t minTreble;
    uint8_t maxTreble;
    uint8_t resTreble;
    uint8_t curAutomaticGain;
    uint8_t curDelay[4];             /* need to consider the endians */
    uint8_t minDelay[4];             /* need to consider the endians */
    uint8_t maxDelay[4];             /* need to consider the endians */
    uint8_t resDelay[4];             /* need to consider the endians */
    uint8_t curLoudness;
    uint8_t curSamplingFrequency[3]; /* need to consider the endians */
    uint8_t minSamplingFrequency[3]; /* need to consider the endians */
    uint8_t maxSamplingFrequency[3]; /* need to consider the endians */
    uint8_t resSamplingFrequency[3]; /* need to consider the endians */
    uint8_t curClockValid;
    uint32_t curSampleFrequency;
    usb_device_control_range_layout3_struct_t freqControlRange;
    usb_device_control_range_layout2_struct_t volumeControlRange;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_AUDIO_MICROPHONE_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    volatile uint8_t start;
    volatile uint8_t startHalfFull;
    volatile uint32_t usbSendTimes;
    volatile uint32_t tdWriteNumber;
    volatile uint32_t tdReadNumber;
    volatile uint32_t reservedSpace;
    volatile uint32_t codecTask;
} usb_audio_microphone_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Enables interrupt service routines for device.
 */
void USB_DeviceIsrEnable(void);

/*!
 * @brief Audio class specific request function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle           The Audio class handle.
 * @param event            The Audio class event type.
 * @param param            The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioRequest(class_handle_t handle, uint32_t event, void *param);

/*!
 * @brief This function returns the ring buffer size.
 *
 * This function calculates and returns the used microphone ring buffer size.
 *
 * @return None.
 */
uint32_t USB_AudioMicrophoneBufferSpaceUsed(void);

/*! @brief This function gets audioMicPacket.
 *
 * This function gets audioMicPacket from the audioMicDataBuff in every callback.
 *
 * @return None.
 */
void USB_AudioMicrophoneGetBuffer(uint8_t *buffer, uint32_t size);

/*! @brief This function increase/decrease the adjusted packet interval according to the reserved ring buffer size
 *
 * This function increase/decrease the adjusted packet interval according to the reserved ring buffer size
 *
 * @return PacketSize.
 */
uint32_t USB_MicrophoneDataMatch(uint32_t reservedspace);

/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle            The Audio class handle.
 * @param event             The Audio class event type.
 * @param param             The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceAudioCallback(class_handle_t handle, uint32_t event, void *param);

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle           The USB device handle.
 * @param event            The USB device event type.
 * @param param            The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

/*!
 * @brief Set interface event callback function of the microphone.
 *
 * Set interface event callbacks implementation of the audio streaming kUSB_DeviceEventSetInterface event.
 *
 * @return None.
 */
usb_status_t USB_DeviceAudioMicrophoneSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting);

/*!
 * @brief This function resets the audio microphone status.
 *
 * This function resets the audio microphone status to the initialized status.
 *
 * @return None.
 */
void USB_DeviceAudioMicrophoneStatusReset(void);

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void USB_DeviceApplicationInit(void);

/*!
 * @brief Audio codec task function.
 *
 * This function provide demo implementation of the audio codec task,
 * but in this mastro demo is not used.
 *
 * @return None.
 */
void USB_AudioCodecTask(void);

#endif /* __USB_AUDIO_MICROPHONE_H__ */
