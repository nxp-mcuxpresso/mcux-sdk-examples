/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
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

#define USB_DEVICE_SPECIFIC_BCD_VERSION (0x0200U)
#define USB_DEVICE_DEMO_BCD_VERSION (0x0101U)

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x009BU)

#define USB_DEVICE_CLASS (0x00U)
#define USB_DEVICE_SUBCLASS (0x00U)
#define USB_DEVICE_PROTOCOL (0x00U)

#define USB_DEVICE_MAX_POWER (0x32U)

#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))
#define USB_DESCRIPTOR_LENGTH_STRING0 (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1 (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2 (sizeof(g_UsbDeviceString2))

#define USB_DEVICE_CONFIGURATION_COUNT (1U)
#define USB_DEVICE_STRING_COUNT (3U)
#define USB_DEVICE_LANGUAGE_COUNT (1U)

#define USB_PRINTER_CONFIGURE_INDEX (1U)
#define USB_PRINTER_INTERFACE_COUNT (1U)

#define USB_PRINTER_INTERFACE_INDEX (0U)
#define USB_PRINTER_INTERFACE_ALTERNATE_COUNT (1U)
#define USB_PRINTER_INTERFACE_ALTERNATE_0 (0U)
#define USB_PRINTER_ENDPOINT_COUNT (2U)
#define USB_PRINTER_BULK_ENDPOINT_OUT (1U)
#define USB_PRINTER_BULK_ENDPOINT_IN (2U)

#define USB_PRINTER_CLASS (0x07U)
#define USB_PRINTER_SUBCLASS (0x01U)
#define USB_PRINTER_PROTOCOL (0x02U)

#define HS_PRINTER_BULK_OUT_PACKET_SIZE (64U)
#define FS_PRINTER_BULK_OUT_PACKET_SIZE (64U)
#define HS_PRINTER_BULK_IN_PACKET_SIZE (64U)
#define FS_PRINTER_BULK_IN_PACKET_SIZE (64U)
#define HS_PRINTER_BULK_OUT_INTERVAL (0x06U) /* 2^(6-1) = 4ms */
#define FS_PRINTER_BULK_OUT_INTERVAL (0x04U)
#define HS_PRINTER_BULK_IN_INTERVAL (0x06U) /* 2^(6-1) = 4ms */
#define FS_PRINTER_BULK_IN_INTERVAL (0x04U)

/*******************************************************************************
 * API
 ******************************************************************************/

/* Configure the device according to the USB speed. */
extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);

#endif /* __USB_DEVICE_DESCRIPTOR_H__ */
