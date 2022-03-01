/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "host_keypad.h"
#include "usb_host_hid.h"
#include "fsl_debug_console.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern void Audio_MuteRequest(void);
extern void Audio_IncreaseVolumeRequest(uint8_t channel);
extern void Audio_DecreaseVolumeRequest(uint8_t channel);

/*******************************************************************************
 * Variables
 ******************************************************************************/
usb_device_handle g_keypadDeviceHandle;
usb_host_interface_handle g_keypadIntfHandle;
host_keypad_instance_t g_keypad;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t g_keypadBuffer[HID_BUFFER_SIZE];
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief process hid data and print mouse action.
 *
 * @param buffer   hid data buffer.
 */
void process_keypadBuffer(uint8_t *buffer)
{
    if (buffer[0] & 0x01)
    {
        Audio_MuteRequest();
    }
    if (buffer[0] & 0x04)
    {
        Audio_DecreaseVolumeRequest(1);
    }
    if (buffer[0] & 0x02)
    {
        Audio_IncreaseVolumeRequest(1);
    }
}

/*!
 * @brief host keypad control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param    the host keypad instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Hid_ControlCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    host_keypad_instance_t *keypad_ptr = (host_keypad_instance_t *)param;

    if (status != kStatus_USB_Success)
    {
        usb_echo("control transfer error\r\n");
    }

    if (keypad_ptr->runWaitState == kUSB_HostHidRunWaitSetInterface)
    {
        keypad_ptr->runState = kUSB_HostHidRunSetInterfaceDone;
    }
}

/*!
 * @brief host keypad interrupt in transfer callback.
 *
 * This function is used as callback function when call USB_HostHidRecv .
 *
 * @param param    the host keypad instance pointer.
 * @param data      data buffer pointer.
 * @param dataLen data length.
 * @param status    transfer result status.
 */
void Hid_InCallback(void *param, uint8_t *data, uint32_t dataLen, usb_status_t status)
{
    host_keypad_instance_t *keypad_ptr = (host_keypad_instance_t *)param;

    if (keypad_ptr->runWaitState == kUSB_HostHidRunWaitDataReceived)
    {
        if (status == kStatus_USB_Success)
        {
            keypad_ptr->runState = kUSB_HostHidRunDataReceived;
        }
        else
        {
            if (keypad_ptr->devState == kStatus_DEV_Attached)
            {
                keypad_ptr->runState = kUSB_HostHidRunPrimeDataReceive;
            }
        }
    }
}

/*!
 * @brief host keypad task function.
 *
 * This function implements the host keypad action, it is used to create task.
 *
 * @param param  the host keypad instance pointer.
 */
void USB_KeypadTask(void *arg)
{
    usb_status_t status = kStatus_USB_Success;

    /* device state changes */
    if (g_keypad.devState != g_keypad.prevState)
    {
        g_keypad.prevState = g_keypad.devState;
        switch (g_keypad.devState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached:
                g_keypad.runState = kUSB_HostHidRunSetInterface;
                status            = USB_HostHidInit(g_keypad.deviceHandle, &g_keypad.classHandle);
                usb_echo("keypad attached\r\n");
                break;

            case kStatus_DEV_Detached:
                g_keypad.devState = kStatus_DEV_Idle;
                g_keypad.runState = kUSB_HostHidRunIdle;
                USB_HostHidDeinit(g_keypad.deviceHandle, g_keypad.classHandle);
                g_keypad.classHandle = NULL;
                usb_echo("keypad detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (g_keypad.runState)
    {
        case kUSB_HostHidRunIdle:
            break;

        case kUSB_HostHidRunSetInterface:
            g_keypad.runWaitState = kUSB_HostHidRunWaitSetInterface;
            g_keypad.runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetInterface(g_keypad.classHandle, g_keypad.interfaceHandle, 0, Hid_ControlCallback,
                                        &g_keypad) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostHidRunSetInterfaceDone:
            g_keypad.maxPacketSize = USB_HostHidGetPacketsize(g_keypad.classHandle, USB_ENDPOINT_INTERRUPT, USB_IN);
            g_keypad.runWaitState  = kUSB_HostHidRunWaitDataReceived;
            g_keypad.runState      = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(g_keypad.classHandle, g_keypad.keypadBuffer, g_keypad.maxPacketSize, Hid_InCallback,
                                &g_keypad) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv: %x\r\n", status);
            }
            break;

        case kUSB_HostHidRunDataReceived:
            process_keypadBuffer(g_keypad.keypadBuffer);

            g_keypad.runWaitState = kUSB_HostHidRunWaitDataReceived;
            g_keypad.runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(g_keypad.classHandle, g_keypad.keypadBuffer, g_keypad.maxPacketSize, Hid_InCallback,
                                &g_keypad) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv: %x\r\n", status);
            }
            break;

        case kUSB_HostHidRunPrimeDataReceive:
            g_keypad.runWaitState = kUSB_HostHidRunWaitDataReceived;
            g_keypad.runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(g_keypad.classHandle, g_keypad.keypadBuffer, g_keypad.maxPacketSize, Hid_InCallback,
                                &g_keypad) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv: %x\r\n", status);
            }
            break;

        default:
            break;
    }
}

/*!
 * @brief host keypad callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode              callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
usb_status_t USB_HostKeypadEvent(usb_device_handle deviceHandle,
                                 usb_host_configuration_handle configurationHandle,
                                 uint32_t eventCode)
{
    uint8_t id;
    usb_host_configuration_t *configuration_ptr;
    uint8_t interface_index;
    usb_host_interface_t *interface_ptr;
    uint32_t info_value = 0U;
    usb_status_t status = kStatus_USB_Success;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration_ptr     = (usb_host_configuration_t *)configurationHandle;
            g_keypadDeviceHandle  = NULL;
            g_keypadIntfHandle    = NULL;
            g_keypad.keypadBuffer = &g_keypadBuffer[0];
            for (interface_index = 0; interface_index < configuration_ptr->interfaceCount; ++interface_index)
            {
                interface_ptr = &configuration_ptr->interfaceList[interface_index];
                id            = interface_ptr->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_HID_CLASS_CODE)
                {
                    continue;
                }
                id = interface_ptr->interfaceDesc->bInterfaceSubClass;
                if ((id != USB_HOST_HID_SUBCLASS_CODE_NONE))
                {
                    continue;
                }
                id = interface_ptr->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_HID_PROTOCOL_NONE)
                {
                    continue;
                }
                else
                {
                    g_keypadDeviceHandle = deviceHandle;
                    g_keypadIntfHandle   = interface_ptr;
                    return kStatus_USB_Success;
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:

            if (g_keypad.devState == kStatus_DEV_Idle)
            {
                if ((g_keypadDeviceHandle != NULL) && (g_keypadIntfHandle != NULL))
                {
                    g_keypad.devState        = kStatus_DEV_Attached;
                    g_keypad.deviceHandle    = g_keypadDeviceHandle;
                    g_keypad.interfaceHandle = g_keypadIntfHandle;

                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &info_value);
                    usb_echo("hid keypad attached:pid=0x%x", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &info_value);
                    usb_echo("vid=0x%x ", info_value);
                    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &info_value);
                    usb_echo("address=%d\r\n", info_value);
                }
            }
            else
            {
                usb_echo("not idle keypad instance\r\n");
            }
            break;

        case kUSB_HostEventDetach:
            if (g_keypad.devState != kStatus_DEV_Idle)
            {
                g_keypad.devState = kStatus_DEV_Detached;
            }
            break;

        default:
            break;
    }
    return status;
}
