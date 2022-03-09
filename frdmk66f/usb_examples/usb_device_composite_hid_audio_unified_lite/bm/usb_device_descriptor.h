/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017,2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

#include "usb_audio_config.h"
#include "usb_device_audio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x00A4U)

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION     (0x0101U)

#define USB_DEVICE_MAX_POWER (0x32U)

/*! @brief Workaround for USB audio 2.0 supported by Windows OS. Please set 1 when meets the following conditions:
    1. device is full speed running audio 2.0
    2. usb host is Windows OS that supports USB audio 2.0, like Win 10
    3. use feedback endpoint
*/
#ifndef USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS
#define USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS (0U)
#endif

/* usb descriptor length */
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL     (sizeof(g_UsbDeviceConfigurationDescriptor))
#define USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH        (9)
#define USB_DESCRIPTOR_LENGTH_HID                   (9U)
#define USB_DESCRIPTOR_LENGTH_HID_KEYBOARD_REPORT   (sizeof(g_UsbDeviceHidKeyboardReportDescriptor))
#define USB_AUDIO_CLASS_SPECIFIC_ENDPOINT_LENGTH    (8)
#define USB_AUDIO_CLOCK_SOURCE_LENGTH               (8)
#define USB_DESCRIPTOR_LENGTH_AC_INTERRUPT_ENDPOINT (9)
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH (9)
#else
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH (10)
#endif
#define USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE            (12)
#define USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE           (9)
#define USB_AUDIO_FEATURE_UNIT_ONLY_DESC_SIZE(ch, n)       (0x07 + (ch + 1) * n)
#define USB_AUDIO_STREAMING_IFACE_DESC_SIZE                (7)
#define USB_AUDIO_STREAMING_ENDP_DESC_SIZE                 (7)
#define USB_AUDIO_STANDARD_AS_ISO_FEEDBACK_ENDPOINT_LENGTH (7)
#define USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE               (11)

/* Configuration, interface and endpoint. */
#define USB_DEVICE_CONFIGURATION_COUNT (1)
#define USB_DEVICE_STRING_COUNT        (3)
#define USB_DEVICE_LANGUAGE_COUNT      (1)
#define USB_DEVICE_INTERFACE_COUNT     (4)
#define USB_AUDIO_ENDPOINT_COUNT       (1)

#define USB_AUDIO_SPEAKER_CONFIGURE_INDEX         (1)
#define USB_COMPOSITE_CONFIGURE_INDEX             (1)
#define USB_AUDIO_CONTROL_INTERFACE_INDEX         (0)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_INDEX (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX  (2)
#define USB_HID_KEYBOARD_INTERFACE_INDEX          (3)

#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT         (1)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_COUNT (2)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT  (2)
#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_0             (0)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_0     (0)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_ALTERNATE_1     (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0      (0)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1      (1)

#define USB_HID_KEYBOARD_INTERFACE_ALTERNATE_COUNT (1)
#define USB_HID_KEYBOARD_INTERFACE_ALTERNATE_0     (0)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#define USB_AUDIO_SPEAKER_STREAM_ENDPOINT_COUNT (1)
#else
#define USB_AUDIO_SPEAKER_STREAM_ENDPOINT_COUNT (2)
#endif
#define USB_AUDIO_CONTROL_ENDPOINT_COUNT         (1)
#define USB_AUDIO_RECORDER_STREAM_ENDPOINT_COUNT (1)
#define USB_HID_KEYBOARD_ENDPOINT_COUNT          (1)

#define USB_AUDIO_SPEAKER_STREAM_ENDPOINT (2)
#define USB_AUDIO_CONTROL_ENDPOINT        (1)
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
/*If multiple data endpoints are to be serviced by the same feedback endpoint, the data endpoints must have ascending
ordered-but not necessarily consecutive-endpoint numbers. The first data endpoint and the feedback endpoint must have
the same endpoint number (and opposite direction). For more information, please refer to Universal Serial Bus
Specification, Revision 2.0 chapter 9.6.6*/
#define USB_AUDIO_SPEAKER_FEEDBACK_ENDPOINT (2)
#endif
#define USB_AUDIO_RECORDER_STREAM_ENDPOINT (3)
#define USB_HID_KEYBOARD_ENDPOINT          (4)

#define USB_AUDIO_COMPOSITE_INTERFACE_COUNT                                                 \
    (USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT + USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT + \
     USB_AUDIO_RECORDER_STREAM_INTERFACE_COUNT)
#define USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT  (1)
#define USB_AUDIO_RECORDER_STREAM_INTERFACE_COUNT (1)
#define USB_HID_KEYBOARD_INTERFACE_COUNT          (1)
#define USB_COMPOSITE_INTERFACE_COUNT             (4)

#define AUDIO_IN_SAMPLING_RATE_KHZ  (48)
#define AUDIO_OUT_SAMPLING_RATE_KHZ (48)

#define AUDIO_SAMPLING_RATE_16KHZ (16)
/*reference macro for audio sample macro */
#define AUDIO_IN_SAMPLING_RATE  (AUDIO_IN_SAMPLING_RATE_KHZ * 1000)
#define AUDIO_OUT_SAMPLING_RATE (AUDIO_OUT_SAMPLING_RATE_KHZ * 1000)
/*if audio in and audio out sample rate are different, please use AUDIO_OUT_SAMPLING_RATE and AUDIO_IN_SAMPLING_RATE
to initialize out and in sample rate respectively*/
#define AUDIO_SAMPLING_RATE (AUDIO_OUT_SAMPLING_RATE)

/* Audio data format */
#define AUDIO_OUT_FORMAT_CHANNELS (0x02U)
#define AUDIO_OUT_FORMAT_BITS     (16)
#define AUDIO_OUT_FORMAT_SIZE     (0x02)
#define AUDIO_IN_FORMAT_CHANNELS  (0x02U)
#define AUDIO_IN_FORMAT_BITS      (16)
#define AUDIO_IN_FORMAT_SIZE      (0x02)
/* transfer length during 1 ms */
#define AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME \
    (AUDIO_OUT_SAMPLING_RATE_KHZ * AUDIO_OUT_FORMAT_CHANNELS * AUDIO_OUT_FORMAT_SIZE)

#if (AUDIO_IN_FORMAT_BITS != AUDIO_OUT_FORMAT_BITS)
/*defalut in sample rate and out sample rate are same, if the sample setting is different,please remove this error
 manually, then check the AUDIO_FORMAT_BITS .*/
#error This application default configuration requires AUDIO_IN_FORMAT_BITS equal to AUDIO_OUT_FORMAT_BITS.
#endif
#define AUDIO_FORMAT_BITS (AUDIO_OUT_FORMAT_BITS)
#if (AUDIO_IN_FORMAT_CHANNELS != AUDIO_OUT_FORMAT_CHANNELS)
/*defalut in channel and out channel are same, if the channel setting is different,please remove this error manually,
 then check the AUDIO_FORMAT_CHANNELS .*/
#error This application default configuration requires AUDIO_IN_FORMAT_CHANNELS equal to AUDIO_OUT_FORMAT_CHANNELS.
#endif
#define AUDIO_FORMAT_CHANNELS (AUDIO_OUT_FORMAT_CHANNELS)

#define HS_ISO_IN_ENDP_INTERVAL (0x04)
#if ((!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) && ((HS_ISO_OUT_ENDP_INTERVAL != 4) || (HS_ISO_IN_ENDP_INTERVAL != 4)))
#error "iso data and sync endpoint interval must be 1 ms for usb audio 1.0"
#endif
#define FS_ISO_OUT_ENDP_INTERVAL          (0x01) /* one frame, 1ms*/
#define FS_ISO_IN_ENDP_INTERVAL           (0x01)
#define ISO_OUT_ENDP_INTERVAL             (0x01)
#define HS_AUDIO_INTERRUPT_IN_PACKET_SIZE (8)
#define FS_AUDIO_INTERRUPT_IN_PACKET_SIZE (8)
#define HS_AUDIO_INTERRUPT_IN_INTERVAL    (0x07U) /* 2^(7-1) = 8ms */
#define FS_AUDIO_INTERRUPT_IN_INTERVAL    (0x08U)
#if ((!USB_DEVICE_CONFIG_AUDIO_CLASS_2_0) && (HS_ISO_OUT_ENDP_INTERVAL != 4))
#error "iso feedback endpoint interval must be 1 ms for usb audio 1.0"
#endif
#if (HS_ISO_OUT_ENDP_INTERVAL < 4)
#if (HS_ISO_OUT_ENDP_INTERVAL == 1U)
#define HS_ISO_OUT_ENDP_PACKET_SIZE ((AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME) / 8U)
#elif (HS_ISO_OUT_ENDP_INTERVAL == 2U)
#define HS_ISO_OUT_ENDP_PACKET_SIZE ((AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME) / 4U)
#elif (HS_ISO_OUT_ENDP_INTERVAL == 3U)
#define HS_ISO_OUT_ENDP_PACKET_SIZE ((AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME) / 2U)
#endif
#else
#define HS_ISO_OUT_ENDP_PACKET_SIZE (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME)
#endif
#define FS_ISO_OUT_ENDP_PACKET_SIZE (AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME)
#define FS_ISO_IN_ENDP_PACKET_SIZE  (AUDIO_IN_SAMPLING_RATE_KHZ * AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)
#define HS_ISO_IN_ENDP_PACKET_SIZE  (AUDIO_IN_SAMPLING_RATE_KHZ * AUDIO_IN_FORMAT_CHANNELS * AUDIO_IN_FORMAT_SIZE)
#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#else
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define HS_ISO_FEEDBACK_ENDP_PACKET_SIZE (4)
#if defined(USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS) && (USB_DEVICE_WORKAROUND_AUDIO_20_WINDOWS > 0)
#define FS_ISO_FEEDBACK_ENDP_PACKET_SIZE (4)
#else
#define FS_ISO_FEEDBACK_ENDP_PACKET_SIZE (3)
#endif
#else
#define HS_ISO_FEEDBACK_ENDP_PACKET_SIZE (3)
#define FS_ISO_FEEDBACK_ENDP_PACKET_SIZE (3)
#endif
#endif
#define HS_HID_KEYBOARD_INTERRUPT_IN_PACKET_SIZE (8U)
#define FS_HID_KEYBOARD_INTERRUPT_IN_PACKET_SIZE (8U)
#define HS_HID_KEYBOARD_INTERRUPT_IN_INTERVAL    (0x10U) /* 2^(6-1) = 4ms */
#define FS_HID_KEYBOARD_INTERRUPT_IN_INTERVAL    (0x10U)

#define HS_ISO_FEEDBACK_ENDP_INTERVAL (0x04U)
#define FS_ISO_FEEDBACK_ENDP_INTERVAL (0x01U)

/* String descriptor length. */
#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))

/* Class code. */
#define USB_DEVICE_CLASS    (0x00)
#define USB_DEVICE_SUBCLASS (0x00)
#define USB_DEVICE_PROTOCOL (0x00)

#define USB_AUDIO_CLASS           (0x01)
#define USB_SUBCLASS_AUDIOCONTROL (0x01)
#define USB_SUBCLASS_AUDIOSTREAM  (0x02)
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_PROTOCOL (0x20)
#else
#define USB_AUDIO_PROTOCOL (0x00)
#endif
#define USB_HID_KEYBOARD_CLASS    (0x03)
#define USB_HID_KEYBOARD_SUBCLASS (0x00)
#define USB_HID_KEYBOARD_PROTOCOL (0x00)

#define USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR    (0x25)
#define USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE (0x01)

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_CONTROL_CLOCK_SOURCE_ENTITY_ID (0x10)
#endif

#define USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID  (0x01)
#define USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID    (0x02)
#define USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID (0x03)

#define USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID  (0x04)
#define USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID    (0x05)
#define USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID (0x06)

#define USB_HID_KEYBOARD_REPORT_LENGTH (0x01U)
/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle		  The USB device handle.
 * @param event 		  The USB device event type.
 * @param param 		  The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);

#endif /* _USB_DESCRIPTOR_H_ */
