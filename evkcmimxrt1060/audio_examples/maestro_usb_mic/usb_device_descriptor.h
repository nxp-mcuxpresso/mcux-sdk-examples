/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* General USB information */
#define USB_DEVICE_VID (0x1FC9U)
/* Note: If any of the USB parameters change, it is often necessary to uninstall
 *       the device driver in the Windows OS or change the value of USB_DEVICE_PID
 *       definition, because the OS may remember the last device configuration.
 */
#define USB_DEVICE_PID (0x099U)

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION     (0x0101U)

#define USB_DEVICE_MAX_POWER (0x32U)

/* usb descriptor length */
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL        (sizeof(g_UsbDeviceConfigurationDescriptor))
#define USB_IAD_DESC_SIZE                              (0x08U)
#define USB_AUDIO_STANDARD_AS_ISO_DATA_ENDPOINT_LENGTH (0x07U)
#define USB_AUDIO_CLASS_SPECIFIC_ENDPOINT_LENGTH       (0x08U)
#define USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH      (0x09U)
#define USB_AUDIO_CONTROL_CLOCK_SOURCE_UNIT_LENGTH     (0x08U)
#define USB_AUDIO_CONTROL_INPUT_TERMINAL_LENGTH        (0x11U)
#define USB_AUDIO_CONTROL_FEATURE_UNIT_LENGTH          (0x0EU)
#define USB_AUDIO_CONTROL_OUTPUT_TERMINAL_LENGTH       (0x0CU)
#define USB_AUDIO_CONTROL_INTERFACE_TOTAL_LENGTH                                              \
    (USB_AUDIO_CONTROL_INTERFACE_HEADER_LENGTH + USB_AUDIO_CONTROL_CLOCK_SOURCE_UNIT_LENGTH + \
     USB_AUDIO_CONTROL_INPUT_TERMINAL_LENGTH + USB_AUDIO_CONTROL_FEATURE_UNIT_LENGTH +        \
     USB_AUDIO_CONTROL_OUTPUT_TERMINAL_LENGTH)
#define USB_AUDIO_STREAMING_INTERFACE_DESC_SIZE (0x10U)
#define USB_AUDIO_STREAMING_FORMAT_DESC_SIZE    (0x06U)
#define USB_AUDIO_STREAMING_ENDPOINT_DESC_SIZE  (0x08U)
#define USB_AUDIO_CONFIGURATION_DESC_TOTAL_LENGTH                                                                      \
    (USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_IAD_DESC_SIZE + USB_DESCRIPTOR_LENGTH_INTERFACE +                           \
     USB_AUDIO_CONTROL_INTERFACE_TOTAL_LENGTH + USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_INTERFACE +    \
     USB_AUDIO_STREAMING_INTERFACE_DESC_SIZE + USB_AUDIO_STREAMING_FORMAT_DESC_SIZE + USB_DESCRIPTOR_LENGTH_ENDPOINT + \
     USB_AUDIO_STREAMING_ENDPOINT_DESC_SIZE)

/* Configuration, interface and endpoint. */
#define USB_DEVICE_CONFIGURATION_COUNT (1)
#define USB_DEVICE_STRING_COUNT        (3)
#define USB_DEVICE_LANGUAGE_COUNT      (1)
#define USB_INTERFACE_COUNT            (1)
#define USB_AUDIO_ENDPOINT_COUNT       (1)

#define USB_AUDIO_MICROPHONE_CONFIGURE_INDEX (1)
#define USB_AUDIO_CONTROL_INTERFACE_INDEX    (0)
#define USB_AUDIO_STREAM_INTERFACE_INDEX     (1)

#define USB_AUDIO_CONTROL_ENDPOINT_COUNT (1)
#define USB_AUDIO_STREAM_ENDPOINT_COUNT  (1)

#define USB_AUDIO_CONTROL_ENDPOINT (1)
#define USB_AUDIO_STREAM_ENDPOINT  (2)

#define USB_AUDIO_MICROPHONE_CONTROL_INTERFACE_COUNT (1)
#define USB_AUDIO_MICROPHONE_STREAM_INTERFACE_COUNT  (1)
#define USB_AUDIO_MICROPHONE_INTERFACE_COUNT \
    (USB_AUDIO_MICROPHONE_CONTROL_INTERFACE_COUNT + USB_AUDIO_MICROPHONE_STREAM_INTERFACE_COUNT)

#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_COUNT (1)
#define USB_AUDIO_STREAM_INTERFACE_ALTERNATE_COUNT  (2)
#define USB_AUDIO_CONTROL_INTERFACE_ALTERNATE_0     (0)
#define USB_AUDIO_STREAM_INTERFACE_ALTERNATE_0      (0)
#define USB_AUDIO_STREAM_INTERFACE_ALTERNATE_1      (1)

#define AUDIO_SAMPLING_RATE_KHZ (48)                             /* 48 dedicates 48Khz */
#define AUDIO_SAMPLING_RATE     (AUDIO_SAMPLING_RATE_KHZ * 1000) /* sampling rate in Hz*/

/* Audio data format */
#define AUDIO_FORMAT_CHANNELS (0x01U)
#define AUDIO_FORMAT_BITS     (16)
#define AUDIO_FORMAT_SIZE     (0x02)

/* Packet size and interval. */
#define HS_INTERRUPT_IN_PACKET_SIZE (8)
#define FS_INTERRUPT_IN_PACKET_SIZE (8)

#define HS_ISO_IN_ENDP_PACKET_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)
#define FS_ISO_IN_ENDP_PACKET_SIZE (AUDIO_SAMPLING_RATE_KHZ * AUDIO_FORMAT_CHANNELS * AUDIO_FORMAT_SIZE)

#define HS_ISO_IN_ENDP_INTERVAL  (0x04)
#define FS_ISO_IN_ENDP_INTERVAL  (0x04)
#define ISO_IN_ENDP_INTERVAL     (0x01)
#define HS_INTERRUPT_IN_INTERVAL (0x07U) /* 2^(7-1) = 8ms */
#define FS_INTERRUPT_IN_INTERVAL (0x07U)

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
#define USB_AUDIO_PROTOCOL        (0x20)

#define USB_AUDIO_STREAM_ENDPOINT_DESCRIPTOR    (0x25)
#define USB_AUDIO_EP_GENERAL_DESCRIPTOR_SUBTYPE (0x01)

#define USB_AUDIO_MICROPHONE_CONTROL_CLOCK_SOURCE_ENTITY_ID (0x10)
#define USB_AUDIO_MICROPHONE_CONTROL_INPUT_TERMINAL_ID      (0x01)
#define USB_AUDIO_MICROPHONE_CONTROL_FEATURE_UNIT_ID        (0x02)
#define USB_AUDIO_MICROPHONE_CONTROL_OUTPUT_TERMINAL_ID     (0x03)

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief USB device set speed function.
 *
 * This function sets the speed of the USB device.
 *
 * @param handle The USB device handle.
 * @param speed Speed type. USB_SPEED_HIGH/USB_SPEED_FULL/USB_SPEED_LOW.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed);
/*!
 * @brief USB device get device descriptor function.
 *
 * This function gets the device descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param deviceDescriptor The pointer to the device descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetDeviceDescriptor(usb_device_handle handle,
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor);
/*!
 * @brief USB device get configuration descriptor function.
 *
 * This function gets the configuration descriptor of the USB device.
 *
 * @param handle The USB device handle.
 * @param configurationDescriptor The pointer to the configuration descriptor structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceGetConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor);

usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor);

#endif /* __USB_DESCRIPTOR_H__ */
