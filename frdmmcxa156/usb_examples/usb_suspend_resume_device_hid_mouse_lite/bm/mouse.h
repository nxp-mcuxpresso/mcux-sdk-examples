/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_HID_MOUSE_H__
#define __USB_HID_MOUSE_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#define USB_HID_MOUSE_REPORT_LENGTH (0x04U)

typedef enum _usb_hid_mouse_status
{
    kStatus_MouseIdle = 0U,
    kStatus_MouseStartSuspend,
    kStatus_MouseSuspending,
    kStatus_MouseSuspended,
    kStatus_MouseStartResume,
    kStatus_MouseResuming,
    kStatus_MouseResumed,
} usb_hid_mouse_status_t;

typedef struct _usb_hid_mouse_struct
{
    volatile uint64_t hwTick;
    uint64_t startTick;
    usb_device_handle deviceHandle;
    uint8_t *buffer;
    uint8_t speed;
    volatile uint8_t attach;
    volatile uint8_t remoteWakeup;
    volatile uint8_t selfWakeup;
    volatile uint8_t isResume;
    volatile usb_hid_mouse_status_t suspend;
} usb_hid_mouse_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* __USB_HID_MOUSE_H__ */
