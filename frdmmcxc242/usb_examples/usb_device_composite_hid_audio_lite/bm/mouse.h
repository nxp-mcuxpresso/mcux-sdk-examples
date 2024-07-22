/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_HID_MOUSE_H__
#define __USB_HID_MOUSE_H__ 1

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define USB_HID_MOUSE_REPORT_LENGTH 0x04
typedef struct _usb_hid_mouse_struct
{
    usb_device_handle deviceHandle;
    uint8_t *buffer;
    uint8_t speed;
    uint8_t attach;
    uint8_t idleRate;
} usb_hid_mouse_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
extern usb_status_t USB_DeviceHidInterruptIn(usb_device_handle deviceHandle,
                                             usb_device_endpoint_callback_message_struct_t *event,
                                             void *arg);
extern usb_hid_mouse_struct_t g_hid_mouse;
extern usb_status_t USB_DeviceMouseAction(void);

#endif /* __USB_HID_MOUSE_H__ */
