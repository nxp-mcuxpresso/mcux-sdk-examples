/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION     (0x0101U)
#define USB_DEVICE_MAX_POWER            (0x32U)

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x00AEU)

#define USB_CONFIGURE_COUNT       (1U)
#define USB_DEVICE_STRING_COUNT   (5U)
#define USB_DEVICE_LANGUAGE_COUNT (1U)
#define USB_INTERFACE_COUNT       (1U)

#define USB_MTP_CONFIGURE_INDEX (1U)

#define USB_MTP_ENDPOINT_COUNT        (3U)
#define USB_MTP_BULK_IN_ENDPOINT      (1U)
#define USB_MTP_BULK_OUT_ENDPOINT     (1U)
#define USB_MTP_INTERRUPT_IN_ENDPOINT (2U)

/* usb descriptor length */
#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))

#define USB_CONFIGURE_ONLY_DESCRIPTOR_LENGTH (9)
#define USB_INTERFACE_DESCRIPTOR_LENGTH      (9)
#define USB_ENDPOINT_DESCRIPTOR_LENGTH       (7)

#define HS_MTP_BULK_IN_PACKET_SIZE      (512U)
#define HS_MTP_BULK_OUT_PACKET_SIZE     (512U)
#define FS_MTP_BULK_IN_PACKET_SIZE      (64U)
#define FS_MTP_BULK_OUT_PACKET_SIZE     (64U)
#define HS_MTP_INTERRUPT_IN_PACKET_SIZE (28U)
#define FS_MTP_INTERRUPT_IN_PACKET_SIZE (28U)
#define HS_MTP_INTERRUPT_IN_INTERVAL    (0x06U) /* 2^(6-1) = 4ms */
#define FS_MTP_INTERRUPT_IN_INTERVAL    (0x06U)

#define USB_DESCRIPTOR_LENGTH_STRING0      (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1      (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2      (sizeof(g_UsbDeviceString2))
#define USB_DESCRIPTOR_LENGTH_STRING3      (sizeof(g_UsbDeviceString3))
#define USB_STRING_DESCRIPTOR_ERROR_LENGTH (sizeof(g_UsbDeviceStringN))

#define USB_MTP_INTERFACE_INDEX           (0U)
#define USB_MTP_INTERFACE_COUNT           (1U)
#define USB_MTP_INTERFACE_ALTERNATE_COUNT (1U)
#define USB_MTP_INTERFACE_ALTERNATE_0     (0U)

#define USB_DEVICE_CLASS    (0x00U)
#define USB_DEVICE_SUBCLASS (0x00U)
#define USB_DEVICE_PROTOCOL (0x00U)

#define USB_MTP_CLASS    (0x06U)
#define USB_MTP_SUBCLASS (0x01U)
#define USB_MTP_PROTOCOL (0x01U)

extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);
extern usb_status_t usb_device_standard_request(usb_setup_struct_t *setup, uint8_t **descriptor, uint32_t *size);

extern usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

#endif
