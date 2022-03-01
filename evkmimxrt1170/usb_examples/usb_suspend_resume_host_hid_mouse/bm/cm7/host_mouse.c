/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_hid.h"
#include "host_mouse.h"
#include "app.h"
#include "fsl_debug_console.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief process hid data and print mouse action.
 *
 * @param buffer   hid data buffer.
 */
static void USB_HostMouseProcessBuffer(uint8_t *buffer);

/*!
 * @brief host mouse control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param      the host mouse instance pointer.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status         transfer result status.
 */
static void USB_HostHidControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

/*!
 * @brief host mouse interrupt in transfer callback.
 *
 * This function is used as callback function when call USB_HostHidRecv .
 *
 * @param param      the host mouse instance pointer.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status           transfer result status.
 */
static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_MouseBuffer[HID_BUFFER_SIZE]; /*!< use to receive report descriptor and data */
usb_host_mouse_instance_t g_HostHidMouse;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*Host hid example doesn't support HID report descriptor analysis, this example assume that the received data are sent
 * by specific order. */
static void USB_HostMouseProcessBuffer(uint8_t *buffer)
{
    static uint8_t left_move_count  = 0;
    static uint8_t right_move_count = 0;
    static uint8_t up_move_count    = 0;
    static uint8_t down_move_count  = 0;

    DbgConsole_Flush();
    /* 1. Left key action */
    if (buffer[0] & 0x01)
    {
        usb_echo("Left Click\r\n");
    }

    /* 2. Middle key action */
    if (buffer[0] & 0x04)
    {
        usb_echo("Middle Click\r\n");
    }

    /* 3. Right key action */
    if (buffer[0] & 0x02)
    {
        usb_echo("Right Click\r\n");
    }

    /* 4. Left/Right movement */
    if (buffer[1])
    {
        if (buffer[1] > 127)
        {
            left_move_count++;
            if (100U == left_move_count)
            {
                usb_echo("Left move events: %d times\r\n", left_move_count);
                left_move_count = 0U;
            }
        }
        else
        {
            right_move_count++;
            if (100U == right_move_count)
            {
                usb_echo("Right move events: %d times\r\n", right_move_count);
                right_move_count = 0U;
            }
        }
    }

    /* 5. UP/Down movement */
    if (buffer[2])
    {
        if (buffer[2] > 127)
        {
            up_move_count++;
            if (100U == up_move_count)
            {
                usb_echo("Up move events: %d times\r\n", up_move_count);
                up_move_count = 0U;
            }
        }
        else
        {
            down_move_count++;
            if (100U == down_move_count)
            {
                usb_echo("Down move events: %d times\r\n", down_move_count);
                down_move_count = 0U;
            }
        }
    }

    /* 6. Whell Down/Wheel UP action */
    if (buffer[3])
    {
        if (buffer[3] > 127)
        {
            usb_echo("Wheel Down\r\n");
        }
        else
        {
            usb_echo("Wheel UP\r\n");
        }
    }
}

static void USB_HostHidControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_mouse_instance_t *mouseInstance = (usb_host_mouse_instance_t *)param;

    if (mouseInstance->runWaitState == kUSB_HostHidRunWaitSetInterface) /* set interface finish */
    {
        mouseInstance->runState = kUSB_HostHidRunSetInterfaceDone;
    }
    else if (mouseInstance->runWaitState == kUSB_HostHidRunWaitSetIdle) /* hid set idle finish */
    {
        mouseInstance->runState = kUSB_HostHidRunSetIdleDone;
    }
    else if (mouseInstance->runWaitState ==
             kUSB_HostHidRunWaitGetReportDescriptor) /* hid get report descriptor finish */
    {
        mouseInstance->runState = kUSB_HostHidRunGetReportDescriptorDone;
    }
    else if (mouseInstance->runWaitState == kUSB_HostHidRunWaitSetProtocol) /* hid set protocol finish */
    {
        mouseInstance->runState = kUSB_HostHidRunSetProtocolDone;
    }
    else
    {
    }
}

static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_mouse_instance_t *mouseInstance = (usb_host_mouse_instance_t *)param;

    if (mouseInstance->runWaitState == kUSB_HostHidRunWaitDataReceived)
    {
        if (mouseInstance->deviceState == kStatus_DEV_Attached)
        {
            if (status == kStatus_USB_Success)
            {
                mouseInstance->runState = kUSB_HostHidRunDataReceived; /* go to process data */
            }
            else
            {
                mouseInstance->runState = kUSB_HostHidRunPrimeDataReceive; /* go to prime next receiving */
            }
        }
    }
}

void USB_HostHidMouseTask(void *param)
{
    usb_host_hid_descriptor_t *hidDescriptor;
    uint32_t mouseReportLength = 0;
    uint8_t *descriptor;
    uint32_t endPosition;
    usb_host_mouse_instance_t *mouseInstance = (usb_host_mouse_instance_t *)param;

    /* device state changes, process once for each state */
    if (mouseInstance->deviceState != mouseInstance->prevState)
    {
        mouseInstance->prevState = mouseInstance->deviceState;
        switch (mouseInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached: /* device is attached and numeration is done */
                mouseInstance->suspendResumeState = kStatus_Idle;
                mouseInstance->runState           = kUSB_HostHidRunSetInterface;
                /* hid class initialization */
                if (USB_HostHidInit(mouseInstance->deviceHandle, &mouseInstance->classHandle) != kStatus_USB_Success)
                {
                    usb_echo("host hid class initialize fail\r\n");
                }
                else
                {
                    usb_echo("mouse attached\r\n");
                }
                break;

            case kStatus_DEV_Detached: /* device is detached */
                mouseInstance->deviceState         = kStatus_DEV_Idle;
                mouseInstance->runState            = kUSB_HostHidRunIdle;
                mouseInstance->suspendResumeState  = kStatus_Idle;
                mouseInstance->supportRemoteWakeup = 0U;
                mouseInstance->isSetRemoteWakeup   = 0U;
                mouseInstance->suspendBus          = 0U;
                mouseInstance->selfWakeup          = 0U;
                USB_HostHidDeinit(mouseInstance->deviceHandle,
                                  mouseInstance->classHandle); /* hid class de-initialization */
                mouseInstance->classHandle  = NULL;
                mouseInstance->deviceHandle = NULL;
                usb_echo("mouse detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (mouseInstance->runState)
    {
        case kUSB_HostHidRunIdle:
            break;

        case kUSB_HostHidRunSetInterface: /* 1. set hid interface */
            mouseInstance->runWaitState = kUSB_HostHidRunWaitSetInterface;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetInterface(mouseInstance->classHandle, mouseInstance->interfaceHandle, 0,
                                        USB_HostHidControlCallback, mouseInstance) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostHidRunSetInterfaceDone: /* 2. hid set idle */
            mouseInstance->maxPacketSize =
                USB_HostHidGetPacketsize(mouseInstance->classHandle, USB_ENDPOINT_INTERRUPT, USB_IN);

            /* first: set idle */
            mouseInstance->runWaitState = kUSB_HostHidRunWaitSetIdle;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetIdle(mouseInstance->classHandle, 0, 0, USB_HostHidControlCallback, mouseInstance) !=
                kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidSetIdle\r\n");
            }
            break;

        case kUSB_HostHidRunSetIdleDone: /* 3. hid get report descriptor */
            /* get report descriptor's length */
            hidDescriptor = NULL;
            descriptor    = (uint8_t *)((usb_host_interface_t *)mouseInstance->interfaceHandle)->interfaceExtension;
            endPosition   = (uint32_t)descriptor +
                          ((usb_host_interface_t *)mouseInstance->interfaceHandle)->interfaceExtensionLength;

            while ((uint32_t)descriptor < endPosition)
            {
                if (*(descriptor + 1) == USB_DESCRIPTOR_TYPE_HID) /* descriptor type */
                {
                    hidDescriptor = (usb_host_hid_descriptor_t *)descriptor;
                    break;
                }
                else
                {
                    descriptor = (uint8_t *)((uint32_t)descriptor + (*descriptor)); /* next descriptor */
                }
            }

            if (hidDescriptor != NULL)
            {
                usb_host_hid_class_descriptor_t *hidClassDescriptor;
                hidClassDescriptor = (usb_host_hid_class_descriptor_t *)&(hidDescriptor->bHidDescriptorType);
                for (uint8_t index = 0; index < hidDescriptor->bNumDescriptors; ++index)
                {
                    hidClassDescriptor += index;
                    if (hidClassDescriptor->bHidDescriptorType == USB_DESCRIPTOR_TYPE_HID_REPORT)
                    {
                        mouseReportLength =
                            (uint16_t)USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(hidClassDescriptor->wDescriptorLength);
                        break;
                    }
                }
            }
            if (mouseReportLength > HID_BUFFER_SIZE)
            {
                usb_echo("hid buffer is too small\r\n");
                mouseInstance->runState = kUSB_HostHidRunIdle;
                return;
            }

            if (mouseReportLength > 0) /* when report descriptor length is zero, go to next step */
            {
                mouseInstance->runWaitState = kUSB_HostHidRunWaitGetReportDescriptor;
                mouseInstance->runState     = kUSB_HostHidRunIdle;
                /* second: get report descriptor */
                USB_HostHidGetReportDescriptor(mouseInstance->classHandle, mouseInstance->mouseBuffer,
                                               mouseReportLength, USB_HostHidControlCallback, mouseInstance);
                break;
            }

        case kUSB_HostHidRunGetReportDescriptorDone: /* 4. hid set protocol */
            mouseInstance->runWaitState = kUSB_HostHidRunWaitSetProtocol;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            /* third: set protocol */
            if (USB_HostHidSetProtocol(mouseInstance->classHandle, USB_HOST_HID_REQUEST_PROTOCOL_REPORT,
                                       USB_HostHidControlCallback, mouseInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidSetProtocol\r\n");
            }
            break;

        case kUSB_HostHidRunSetProtocolDone: /* 5. start to receive data */
            mouseInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(mouseInstance->classHandle, mouseInstance->mouseBuffer, mouseInstance->maxPacketSize,
                                USB_HostHidInCallback, mouseInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            break;

        case kUSB_HostHidRunDataReceived: /* process received data and receive next data */
            USB_HostMouseProcessBuffer(mouseInstance->mouseBuffer);

            mouseInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(mouseInstance->classHandle, mouseInstance->mouseBuffer, mouseInstance->maxPacketSize,
                                USB_HostHidInCallback, mouseInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            break;

        case kUSB_HostHidRunPrimeDataReceive: /* receive data */
            mouseInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            mouseInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(mouseInstance->classHandle, mouseInstance->mouseBuffer, mouseInstance->maxPacketSize,
                                USB_HostHidInCallback, mouseInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            break;

        default:
            break;
    }
}

usb_status_t USB_HostHidMouseEvent(usb_device_handle deviceHandle,
                                   usb_host_configuration_handle configurationHandle,
                                   uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    uint8_t id;
    usb_host_configuration_t *configuration;
    uint8_t interfaceIndex;
    usb_host_interface_t *interface;
    uint32_t infoValue = 0U;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                id        = interface->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_HID_CLASS_CODE)
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceSubClass;
                if ((id != USB_HOST_HID_SUBCLASS_CODE_NONE) && (id != USB_HOST_HID_SUBCLASS_CODE_BOOT))
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_HID_PROTOCOL_MOUSE)
                {
                    continue;
                }
                else
                {
                    if (g_HostHidMouse.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_HostHidMouse.mouseBuffer     = s_MouseBuffer;
                        g_HostHidMouse.deviceHandle    = deviceHandle;
                        g_HostHidMouse.interfaceHandle = interface;
                        g_HostHidMouse.configHandle    = configurationHandle;
                        if (configuration->configurationDesc->bmAttributes &
                            USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_MASK)
                        {
                            g_HostHidMouse.supportRemoteWakeup = 1U;
                        }
                        else
                        {
                            g_HostHidMouse.supportRemoteWakeup = 0U;
                        }
                        return kStatus_USB_Success;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_HostHidMouse.configHandle == configurationHandle)
            {
                if ((g_HostHidMouse.deviceHandle != NULL) && (g_HostHidMouse.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_HostHidMouse.deviceState == kStatus_DEV_Idle)
                    {
                        g_HostHidMouse.deviceState = kStatus_DEV_Attached;

                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &infoValue);
                        usb_echo("hid mouse attached:pid=0x%x", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &infoValue);
                        usb_echo("vid=0x%x ", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &infoValue);
                        usb_echo("address=%d\r\n", infoValue);
                    }
                    else
                    {
                        usb_echo("not idle mouse instance\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_HostHidMouse.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_HostHidMouse.configHandle = NULL;
                if (g_HostHidMouse.deviceState != kStatus_DEV_Idle)
                {
                    g_HostHidMouse.deviceState = kStatus_DEV_Detached;
                }
            }
            break;

        default:
            break;
    }
    return status;
}
