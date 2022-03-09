/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "mouse.h"

#include "fsl_device_registers.h"
#include "clock_config.h"
#include "board.h"

#include "composite.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_MouseBuffer[USB_HID_MOUSE_REPORT_LENGTH];
static usb_device_composite_struct_t *g_deviceComposite;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief device mouse moves action.
 *
 * This function handle the mouse moves action.
 */
usb_status_t USB_DeviceMouseAction(void)
{
    static int8_t x = 0U;
    static int8_t y = 0U;
    enum
    {
        RIGHT,
        DOWN,
        LEFT,
        UP
    };
    static uint8_t dir = RIGHT;

    switch (dir)
    {
        case RIGHT:
            g_deviceComposite->hidMouse.buffer[1] = 2;
            g_deviceComposite->hidMouse.buffer[2] = 0U;
            x++;
            if (x > 5U)
            {
                dir++;
            }
            break;
        case DOWN:
            g_deviceComposite->hidMouse.buffer[1] = 0U;
            g_deviceComposite->hidMouse.buffer[2] = 2;
            y++;
            if (y > 5U)
            {
                dir++;
            }
            break;
        case LEFT:
            g_deviceComposite->hidMouse.buffer[1] = (uint8_t)(-2);
            g_deviceComposite->hidMouse.buffer[2] = 0U;
            x--;
            if (x < 1U)
            {
                dir++;
            }
            break;
        case UP:
            g_deviceComposite->hidMouse.buffer[1] = 0U;
            g_deviceComposite->hidMouse.buffer[2] = (uint8_t)(-2);
            y--;
            if (y < 1U)
            {
                dir = RIGHT;
            }
            break;
        default:
            break;
    }
    return USB_DeviceHidSend(g_deviceComposite->hidMouse.hidHandle, USB_HID_MOUSE_ENDPOINT,
                             g_deviceComposite->hidMouse.buffer, USB_HID_MOUSE_REPORT_LENGTH);
}

/*!
 * @brief device Hid callback function.
 *
 * This function handle the Hid class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
usb_status_t USB_DeviceHidMouseCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    switch (event)
    {
        case kUSB_DeviceHidEventSendResponse:
            if (g_deviceComposite->hidMouse.attach)
            {
                error = USB_DeviceMouseAction();
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

/*!
 * @brief Hid device set configuration function.
 *
 * This function sets configuration for msc class.
 *
 * @param handle The Hid class handle.
 * @param configure Hid class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceHidMouseSetConfigure(class_handle_t handle, uint8_t configure)
{
    g_deviceComposite->hidMouse.attach = 1U;
    g_deviceComposite->hidMouse.buffer = s_MouseBuffer;
    return USB_DeviceMouseAction();
}

/*!
 * @brief Hid device set interface function.
 *
 * This function sets interface for msc class.
 *
 * @param handle The Hid class handle.
 * @param alternateSetting Hid class alternateSetting.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceHidMouseSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting)
{
    return kStatus_USB_Error;
}

/*!
 * @brief device Hid init function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite          The pointer to the composite device structure.
 * @return kStatus_USB_Success .
 */
usb_status_t USB_DeviceHidMouseInit(usb_device_composite_struct_t *device_composite)
{
    g_deviceComposite = device_composite;
    return kStatus_USB_Success;
}
