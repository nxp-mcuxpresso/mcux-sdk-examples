/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*! @brief Whether USB Audio use syn mode or not, note that some socs may not support sync mode */
#define USB_DEVICE_AUDIO_USE_SYNC_MODE (0U)

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x0097U)

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION     (0x0101U)

#define USB_DEVICE_MAX_POWER (0x32U)

/* usb descriptor length */
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL        (sizeof(g_UsbDeviceConfigurationDescriptor))
#define USB_ENDPOINT_AUDIO_DESCRIPTOR_LENGTH           (9U)
#define USB_AUDIO_STANDARD_AS_ISO_DATA_ENDPOINT_LENGTH (7U)
#define USB_AUDIO_CLASS_SPECIFIC_ENDPOINT_LENGTH       (8U)
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH      (9U)
#define USB_DESCRIPTOR_LENGTH_AC_INTERRUPT_ENDPOINT    (9U)
#define USB_AUDIO_INPUT_TERMINAL_ONLY_DESC_SIZE        (12U)
#define USB_AUDIO_OUTPUT_TERMINAL_ONLY_DESC_SIZE       (9U)
#define USB_AUDIO_FEATURE_UNIT_ONLY_DESC_SIZE          (9U)
#define USB_AUDIO_STREAMING_IFACE_DESC_SIZE            (7U)
#define USB_AUDIO_STREAMING_ENDP_DESC_SIZE             (7U)
#define USB_AUDIO_STREAMING_TYPE_I_DESC_SIZE           (11U)

/* Configuration, interface and endpoint. */
#define USB_DEVICE_CONFIGURATION_COUNT (1U)
#define USB_DEVICE_STRING_COUNT        (3U)
#define USB_DEVICE_LANGUAGE_COUNT      (1U)
#define USB_INTERFACE_COUNT            (1U)
#define USB_AUDIO_ENDPOINT_COUNT       (1U)

#define USB_AUDIO_GENERATOR_CONFIGURE_INDEX (1U)
#define USB_AUDIO_CONTROL_INTERFACE_INDEX   (0U)
#define USB_AUDIO_STREAM_INTERFACE_INDEX    (1U)

#define USB_AUDIO_STREAM_ENDPOINT_COUNT  (1U)
#define USB_AUDIO_CONTROL_ENDPOINT_COUNT (1U)

#define USB_AUDIO_STREAM_ENDPOINT  (2U)
#define USB_AUDIO_CONTROL_ENDPOINT (1U)

#define USB_AUDIO_GENERATOR_INTERFACE_COUNT \
    (USB_AUDIO_GENERATOR_CONTROL_INTERFACE_COUNT + USB_AUDIO_GENERATOR_STREAM_INTERFACE_COUNT)
#define USB_AUDIO_GENERATOR_CONTROL_INTERFACE_COUNT (1U)
#define USB_AUDIO_GENERATOR_STREAM_INTERFACE_COUNT  (1U)

#define USB_AUDIO_GENERATOR_CONTROL_INTERFACE_ALTERNATE_COUNT (1U)
#define USB_AUDIO_GENERATOR_STREAM_INTERFACE_ALTERNATE_COUNT  (2U)
#define USB_AUDIO_GENERATOR_CONTROL_INTERFACE_ALTERNATE_0     (0U)
#define USB_AUDIO_GENERATOR_STREAM_INTERFACE_ALTERNATE_0      (0U)
#define USB_AUDIO_GENERATOR_STREAM_INTERFACE_ALTERNATE_1      (1U)
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
#define AUDIO_SAMPLING_RATE_KHZ (16U) /* 16 dedicates 16Khz */
#else
#define AUDIO_SAMPLING_RATE_KHZ (8U) /* 8 dedicates 8Khz */
#endif

/* Audio data format */
#define AUDIO_FORMAT_CHANNELS (0x01U)
#if defined(AUDIO_DATA_SOURCE_DMIC) && (AUDIO_DATA_SOURCE_DMIC > 0U)
#define AUDIO_FORMAT_BITS (16U)
#define AUDIO_FORMAT_SIZE (0x02U)
#else
#define AUDIO_FORMAT_BITS (8U)
#define AUDIO_FORMAT_SIZE (0x01U)
#endif

/* Packet size and interval. */
#define HS_INTERRUPT_IN_PACKET_SIZE (8U)
#define FS_INTERRUPT_IN_PACKET_SIZE (8U)

#define HS_ISO_IN_ENDP_PACKET_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)
#define FS_ISO_IN_ENDP_PACKET_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)

#define HS_ISO_IN_ENDP_INTERVAL  (0x04U)
#define FS_ISO_IN_ENDP_INTERVAL  (0x01U)
#define ISO_IN_ENDP_INTERVAL     (0x01U)
#define HS_INTERRUPT_IN_INTERVAL (0x07U) /* 2^(7-1) = 8ms */
#define FS_INTERRUPT_IN_INTERVAL (0x08U)

/* String descriptor length. */
#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))

/* Class code. */
#define USB_DEVICE_CLASS    (0x00U)
#define USB_DEVICE_SUBCLASS (0x00U)
#define USB_DEVICE_PROTOCOL (0x00U)

#define USB_AUDIO_CLASS           (0x01U)
#define USB_SUBCLASS_AUDIOCONTROL (0x01U)
#define USB_SUBCLASS_AUDIOSTREAM  (0x02U)
#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_PROTOCOL (0x20U)
#else
#define USB_AUDIO_PROTOCOL (0x00U)
#endif

#define USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR    (0x25U)
#define USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE (0x01U)

#if (USB_DEVICE_CONFIG_AUDIO_CLASS_2_0)
#define USB_AUDIO_RECORDER_CONTROL_CLOCK_SOURCE_ENTITY_ID (0x10U)
#endif
#define USB_AUDIO_RECORDER_CONTROL_INPUT_TERMINAL_ID  (0x01U)
#define USB_AUDIO_RECORDER_CONTROL_FEATURE_UNIT_ID    (0x02U)
#define USB_AUDIO_RECORDER_CONTROL_OUTPUT_TERMINAL_ID (0x03U)

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
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
#endif
