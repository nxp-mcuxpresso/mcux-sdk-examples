/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "hid_keyboard.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static usb_status_t USB_DeviceHidKeyboardAction(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_KeyboardBuffer[USB_HID_KEYBOARD_REPORT_LENGTH];
static usb_device_composite_struct_t *s_UsbDeviceComposite;
static usb_device_hid_keyboard_struct_t s_UsbDeviceHidKeyboard;

/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_DeviceHidKeyboardAction(void)
{
    static int x = 0U;
    enum
    {
        DOWN,
        UP
    };
    static uint8_t dir = DOWN;

    s_UsbDeviceHidKeyboard.buffer[2] = 0x00U;
    switch (dir)
    {
        case DOWN:
            x++;
            if (x > 200U)
            {
                dir++;
                s_UsbDeviceHidKeyboard.buffer[2] = KEY_PAGEUP;
            }
            break;
        case UP:
            x--;
            if (x < 1U)
            {
                dir                              = DOWN;
                s_UsbDeviceHidKeyboard.buffer[2] = KEY_PAGEDOWN;
            }
            break;
        default:
            break;
    }
    return USB_DeviceHidSend(s_UsbDeviceComposite->hidKeyboardHandle, USB_HID_KEYBOARD_ENDPOINT_IN,
                             s_UsbDeviceHidKeyboard.buffer, USB_HID_KEYBOARD_REPORT_LENGTH);
}

usb_status_t USB_DeviceHidKeyboardCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (s_UsbDeviceComposite->attach)
            {
                return USB_DeviceHidKeyboardAction();
            }
            break;
        case kUSB_DeviceHidEventGetReport:
        case kUSB_DeviceHidEventSetReport:
        case kUSB_DeviceHidEventRequestReportBuffer:
            break;
        case kUSB_DeviceHidEventGetIdle:
        case kUSB_DeviceHidEventGetProtocol:
        case kUSB_DeviceHidEventSetIdle:
        case kUSB_DeviceHidEventSetProtocol:
            error = kStatus_USB_Success;
            break;
        default:
            break;
    }

    return error;
}

usb_status_t USB_DeviceHidKeyboardSetConfigure(class_handle_t handle, uint8_t configure)
{
    if (USB_COMPOSITE_CONFIGURE_INDEX == configure)
    {
        return USB_DeviceHidKeyboardAction(); /* run the cursor movement code */
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    if (USB_HID_KEYBOARD_INTERFACE_INDEX == interface)
    {
        return USB_DeviceHidKeyboardAction(); /* run the cursor movement code */
    }
    return kStatus_USB_Error;
}

usb_status_t USB_DeviceHidKeyboardInit(usb_device_composite_struct_t *deviceComposite)
{
    s_UsbDeviceComposite          = deviceComposite;
    s_UsbDeviceHidKeyboard.buffer = s_KeyboardBuffer;
    return kStatus_USB_Success;
}
