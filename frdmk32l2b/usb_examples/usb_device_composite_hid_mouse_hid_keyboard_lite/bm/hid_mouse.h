/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_HID_MOUSE_H__
#define __USB_DEVICE_HID_MOUSE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _usb_device_hid_mouse_struct
{
    uint8_t *buffer;
    uint8_t idleRate;
} usb_device_hid_mouse_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

extern usb_status_t USB_DeviceHidMouseSetConfigure(usb_device_handle handle, uint8_t configure);
extern usb_status_t USB_DeviceHidMouseClassRequest(usb_device_handle handle,
                                                   usb_setup_struct_t *setup,
                                                   uint8_t **buffer,
                                                   uint32_t *length);
extern usb_status_t USB_DeviceHidMouseInit(usb_device_composite_struct_t *deviceComposite);
extern usb_status_t USB_DeviceHidMouseEndpointUnstall(usb_device_handle handle, uint8_t ep);
extern usb_status_t USB_DeviceHidMouseEndpointStall(usb_device_handle handle, uint8_t ep);

#endif /* __USB_DEVICE_HID_MOUSE_H__ */
