/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016,2019 NXP
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
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U))
/*! @brief USB DCD charging detect status */
typedef enum _usb_device_dcd_dev_status
{
    kUSB_DeviceDCDDectionInit = 0x0U,
    kUSB_DeviceDCDDectionError,
    kUSB_DeviceDCDDectionTimeOut,
    kUSB_DeviceDCDDectionSDP,
    kUSB_DeviceDCDDectionCDP,
    kUSB_DeviceDCDDectionDCP,
    kUSB_DeviceDCDDectionFinished,
} usb_device_dcd_dev_status_t;
#endif
typedef struct _usb_hid_mouse_struct
{
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U)) && \
    (defined(FSL_FEATURE_SOC_USB_ANALOG_COUNT) && (FSL_FEATURE_SOC_USB_ANALOG_COUNT > 0U))
    volatile uint64_t hwTick;
#endif
    usb_device_handle deviceHandle;
    class_handle_t hidHandle;
    uint8_t *buffer;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_HID_MOUSE_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
    volatile uint8_t connectStateChanged;
    volatile uint8_t connectState;
#endif
#if (defined(USB_DEVICE_CONFIG_CHARGER_DETECT) && (USB_DEVICE_CONFIG_CHARGER_DETECT > 0U))
    usb_device_dcd_dev_status_t dcdDectionStatus;
#endif
} usb_hid_mouse_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* __USB_HID_MOUSE_H__ */
