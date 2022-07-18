/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
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

/*! @brief Whether UAC 7.1 is enabled or not. Note that this macro may be also defined in usb_audio_config.h in some
 * cases, make sure the consistent change. */
#define USB_AUDIO_CHANNEL7_1 (0)

/*! @brief Whether UAC 5.1 is enabled or not. Note that this macro may be also defined in usb_audio_config.h in some
 * cases, make sure the consistent change. */
#define USB_AUDIO_CHANNEL5_1 (0)

/*! @brief Whether UAC 3.1 is enabled or not. Note that this macro may be also defined in usb_audio_config.h in some
 * cases, make sure the consistent change. */
#define USB_AUDIO_CHANNEL3_1 (0)

#if defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)
#define USB_AUDIO_7_1_CHANNEL_PAIR_SEL                                                                               \
    (0x01) /* 0x01: front left, front right; 0x02: front center, subwoofer; 0x03: rear left, rear right; 0x04: front \
              left of center, front right of center  */
#endif

#if defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
#define USB_AUDIO_5_1_CHANNEL_PAIR_SEL \
    (0x01) /* 0x01: front left, front right; 0x02: front center, subwoofer; 0x03: rear left, rear right */
#endif

#if defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)
#define USB_AUDIO_3_1_CHANNEL_PAIR_SEL (0x01) /* 0x01: front left, front right; 0x02: front center, subwoofer */
#endif

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x0098U)

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
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#define USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH               (9)
#define USB_AUDIO_CLASS_SPECIFIC_ENDPOINT_LENGTH           (8)
#define USB_AUDIO_STANDARD_AS_ISO_DATA_ENDPOINT_LENGTH     (7)
#define USB_AUDIO_STANDARD_AS_ISO_FEEDBACK_ENDPOINT_LENGTH (7)
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH          (9)
#define USB_DESCRIPTOR_LENGTH_AC_INTERRUPT_ENDPOINT        (9)
#define USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE            (12)
#define USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE           (9)
#define USB_AUDIO_FEATURE_UNIT_ONLY_DESC_SIZE(ch, n)       (0x07 + (ch + 1) * n)
#define USB_AUDIO_STREAMING_IFACE_DESC_SIZE                (7)
#define USB_AUDIO_STREAMING_ENDP_DESC_SIZE                 (7)
#define USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE               (11)

/* Configuration, interface and endpoint. */
#define USB_DEVICE_CONFIGURATION_COUNT (1)
#define USB_DEVICE_STRING_COUNT        (3)
#define USB_DEVICE_LANGUAGE_COUNT      (1)

#define USB_AUDIO_SPEAKER_CONFIGURE_INDEX        (1)
#define USB_AUDIO_CONTROL_INTERFACE_INDEX        (0)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_INDEX (1)

#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT        (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_COUNT (2)

#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_0        (0x00U)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_0 (0x00U)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_ALTERNATE_1 (0x01U)

#if defined(USB_DEVICE_AUDIO_USE_SYNC_MODE) && (USB_DEVICE_AUDIO_USE_SYNC_MODE > 0U)
#define USB_AUDIO_STREAM_ENDPOINT_COUNT (1)
#else
#define USB_AUDIO_STREAM_ENDPOINT_COUNT (2)
#endif
#define USB_AUDIO_CONTROL_ENDPOINT_COUNT (1)

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

#define USB_AUDIO_SPEAKER_INTERFACE_COUNT \
    (USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT + USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT)
#define USB_AUDIO_SPEAKER_CONTROL_INTERFACE_COUNT (1)
#define USB_AUDIO_SPEAKER_STREAM_INTERFACE_COUNT  (1)

/* Audio data format */
#if defined(USB_AUDIO_CHANNEL7_1) && (USB_AUDIO_CHANNEL7_1 > 0U)
#define AUDIO_FORMAT_CHANNELS (0x08)
#elif defined(USB_AUDIO_CHANNEL5_1) && (USB_AUDIO_CHANNEL5_1 > 0U)
#define AUDIO_FORMAT_CHANNELS (0x06)
#elif defined(USB_AUDIO_CHANNEL3_1) && (USB_AUDIO_CHANNEL3_1 > 0U)
#define AUDIO_FORMAT_CHANNELS (0x04)
#else
#define AUDIO_FORMAT_CHANNELS (0x02)
#endif
#define AUDIO_FORMAT_BITS (16)
#define AUDIO_FORMAT_SIZE (0x02)

/* transfer length during 1 ms */
#define AUDIO_OUT_TRANSFER_LENGTH_ONE_FRAME (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)

/* Packet size and interval. */
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

#define USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR    (0x25)
#define USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE (0x01)

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_SPEAKER_CONTROL_CLOCK_SOURCE_ENTITY_ID (0x10)
#define USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID     (0x40)
#define USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID        (0x30)
#define USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID      (0x20)
#else
#define USB_AUDIO_SPEAKER_CONTROL_INPUT_TERMINAL_ID  (0x01)
#define USB_AUDIO_SPEAKER_CONTROL_FEATURE_UNIT_ID    (0x02)
#define USB_AUDIO_SPEAKER_CONTROL_OUTPUT_TERMINAL_ID (0x03)
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief USB device set speed function.
 *
 * This function sets the speed of the USB device.
 *
 * Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
 * current speed.
 * As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
 * KHCI.
 * When the EHCI is enabled, the application needs to call this function to update device by using current speed.
 * The updated information includes endpoint max packet size, endpoint interval, etc.
 *
 * @param speed Speed type. USB_SPEED_HIGH/USB_SPEED_FULL/USB_SPEED_LOW.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);

#endif /* __USB_DEVICE_DESCRIPTOR_H__ */
