/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_DESCRIPTOR_H__
#define __USB_DEVICE_DESCRIPTOR_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USB_DEVICE_SPECIFIC_BCD_VERSION      (0x0200U)
#define USB_DEVICE_SPECIFIC_BCD_VERSION_DFU  (0x0100U)
#define USB_DEVICE_DEMO_BCD_VERSION          (0x0101U)
#define USB_DEVICE_OS_DESCRIPTOR_BCD_VERSION (0x0100U)

#define USB_DEVICE_VID (0x1FC9U)
#define USB_DEVICE_PID (0x00A1U)

#define USB_DEVICE_CLASS    (0x00U)
#define USB_DEVICE_SUBCLASS (0x00U)
#define USB_DEVICE_PROTOCOL (0x00U)

#define USB_DEVICE_MAX_POWER (0x32U)

#define USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL (sizeof(g_UsbDeviceConfigurationDescriptor))
#define USB_DESCRIPTOR_LENGTH_FUNCTINAL         (9U)
#define USB_DESCRIPTOR_LENGTH_STRING0           (sizeof(g_UsbDeviceString0))
#define USB_DESCRIPTOR_LENGTH_STRING1           (sizeof(g_UsbDeviceString1))
#define USB_DESCRIPTOR_LENGTH_STRING2           (sizeof(g_UsbDeviceString2))
#define USB_DESCRIPTOR_LENGTH_STRING3           (sizeof(g_UsbDeviceString3))
#define USB_DESCRIPTOR_LENGTH_OSExended         (sizeof(g_UsbDeviceOSExendedDescriptor))
#define USB_DESCRIPTOR_LENGTH_COMPAT            (sizeof(g_UsbDeviceCompatibleIDDescriptor))

#define USB_DFU_INTERFACE_INDEX            (0U)
#define USB_DESCRIPTOR_TYPE_DFU_FUNCTIONAL (0x21U)
#define USB_DFU_DETACH_TIMEOUT             (0x6400U)
#define USB_DFU_BIT_WILL_DETACH            (1U)
#define USB_DFU_BIT_MANIFESTATION_TOLERANT (0U)
#define USB_DFU_BIT_CAN_UPLOAD             (0U)
#define USB_DFU_BIT_CAN_DNLOAD             (1U)
#define MAX_TRANSFER_SIZE                  (0x200U)

#define USB_DFU_INTERFACE_COUNT (1U)

#define USB_DFU_INTERFACE_ALTERNATE_COUNT (1U)
#define USB_DFU_INTERFACE_ALTERNATE_0     (0U)

#define USB_DEVICE_STRING_COUNT (4U)

#define USB_DEVICE_LANGUAGE_COUNT (1U)

#define USB_DFU_CONFIGURE_INDEX (1U)
#define USB_DFU_CLASS           (0xFEU)
#define USB_DFU_SUBCLASS        (0x01U)

#define USB_DFU_PROTOCOL      (0x01U)
#define USB_DFU_MODE_PROTOCOL (0x02U)

#define USB_MICROSOFT_EXTENDED_COMPAT_ID     (0x0004U)
#define USB_MICROSOFT_EXTENDED_PROPERTIES_ID (0x0005U)
/*******************************************************************************
 * API
 ******************************************************************************/

extern usb_status_t USB_DeviceSetSpeed(uint8_t speed);
extern usb_status_t USB_DeviceGetVerdorDescriptor(usb_device_handle handle,
                                                  usb_setup_struct_t *setup,
                                                  uint32_t *length,
                                                  uint8_t **buffer);

#endif /* __USB_DEVICE_DESCRIPTOR_H__ */
