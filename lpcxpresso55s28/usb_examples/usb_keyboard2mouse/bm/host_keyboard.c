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
#include "host_keyboard.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define KEYBOARD_USAGE_ID_NUMBER (57U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief process hid data and print keyboard action.
 *
 * @param buffer   hid data buffer.
 */
static void USB_HostKeyboardProcessBuffer(usb_host_keyboard_instance_t *keyboardInstance);

/*!
 * @brief host keyboard interrupt in transfer callback.
 *
 * This function is used as callback function when call USB_HostHidRecv .
 *
 * @param param    the host keyboard instance pointer.
 * @param data     data buffer pointer.
 * @param dataLength data length.
 * @status         transfer result status.
 */
static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

extern usb_status_t USB_DeviceHidMouseAction(uint32_t moveAction);

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_KeyboardBuffer[HID_BUFFER_SIZE]; /*!< use to receive report descriptor and data */
usb_host_keyboard_instance_t g_HostHidKeyboard;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*Host hid example doesn't support HID report descriptor analysis, this example assume that the received data are sent
 * by specific order. */
static void USB_HostKeyboardProcessBuffer(usb_host_keyboard_instance_t *keyboardInstance)
{
    uint8_t index;
    uint8_t key;
    uint8_t moveAction = 0;

    /* if all packet data is same the packet is invalid */
    key = keyboardInstance->keyboardBuffer[0];
    for (index = 1; index < 8; ++index)
    {
        if (key != keyboardInstance->keyboardBuffer[index])
        {
            break;
        }
    }
    if (index >= 8) /* all packet data is same */
    {
        return;
    }

    for (index = 2; index < 8; ++index)
    {
        key = keyboardInstance->keyboardBuffer[index];
        if (key == 0)
        {
            break;
        }

        if (key == 26) /* w */
        {
            moveAction |= (0x01); /* up */
        }
        else if (key == 22) /* s */
        {
            moveAction |= (uint8_t)(0x01 << 1); /* down */
        }
        else if (key == 4) /* a */
        {
            moveAction |= (uint8_t)(0x01 << 2); /* left */
        }
        else if (key == 7) /* d */
        {
            moveAction |= (uint8_t)(0x01 << 3); /* right */
        }
        else
        {
        }
    }

    if (moveAction)
    {
        USB_DeviceHidMouseAction(moveAction); /* mouse move */
    }
}

static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_keyboard_instance_t *keyboardInstance = (usb_host_keyboard_instance_t *)param;

    if (keyboardInstance->runWaitState == kUSB_HostHidRunWaitDataReceived)
    {
        if (keyboardInstance->deviceState == kStatus_DEV_Attached)
        {
            if (status == kStatus_USB_Success)
            {
                keyboardInstance->runState = kUSB_HostHidRunDataReceived; /* go to process data */
            }
            else
            {
                keyboardInstance->runState = kUSB_HostHidRunPrimeDataReceive; /* go to prime next receiving */
            }
        }
    }
}

static void USB_HostHidControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_keyboard_instance_t *keyboardInstance = (usb_host_keyboard_instance_t *)param;

    if (keyboardInstance->runWaitState == kUSB_HostHidRunWaitSetInterface) /* set interface done */
    {
        keyboardInstance->runState = kUSB_HostHidRunSetInterfaceDone;
    }
    else if (keyboardInstance->runWaitState == kUSB_HostHidRunWaitSetIdle) /* hid set idle done */
    {
        keyboardInstance->runState = kUSB_HostHidRunSetIdleDone;
    }
    else if (keyboardInstance->runWaitState ==
             kUSB_HostHidRunWaitGetReportDescriptor) /* hid get report descriptor done */
    {
        keyboardInstance->runState = kUSB_HostHidRunGetReportDescriptorDone;
    }
    else if (keyboardInstance->runWaitState == kUSB_HostHidRunWaitSetProtocol) /* hid set protocol done */
    {
        keyboardInstance->runState = kUSB_HostHidRunSetProtocolDone;
    }
    else
    {
    }
}

void USB_HostHidKeyboardTask(void *param)
{
    usb_host_keyboard_instance_t *keyboardInstance = (usb_host_keyboard_instance_t *)param;
    usb_host_hid_descriptor_t *hidDescriptor;
    uint32_t keyboardReportLength = 0;
    uint8_t *descriptor;
    uint32_t endPosition;
    uint8_t index;

    /* device state changes, process once for each state */
    if (keyboardInstance->deviceState != keyboardInstance->prevState)
    {
        keyboardInstance->prevState = keyboardInstance->deviceState;
        switch (keyboardInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached: /* deivce is attached and numeration is done */
                keyboardInstance->runState = kUSB_HostHidRunSetInterface;
                USB_HostHidInit(keyboardInstance->deviceHandle,
                                &keyboardInstance->classHandle); /* hid class initialization */
                usb_echo("keyboard attached\r\n");
                break;

            case kStatus_DEV_Detached: /* device is detached */
                keyboardInstance->runState    = kUSB_HostHidRunIdle;
                keyboardInstance->deviceState = kStatus_DEV_Idle;
                USB_HostHidDeinit(keyboardInstance->deviceHandle,
                                  keyboardInstance->classHandle); /* hid class de-initialization */
                keyboardInstance->classHandle = NULL;
                usb_echo("keyboard detached\r\n");
                break;

            default:
                break;
        }
    }

    switch (keyboardInstance->runState)
    {
        case kUSB_HostHidRunIdle:
            break;

        case kUSB_HostHidRunSetInterface: /* 1. set hid interface */
            keyboardInstance->runWaitState = kUSB_HostHidRunWaitSetInterface;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetInterface(keyboardInstance->classHandle, keyboardInstance->interfaceHandle, 0,
                                        USB_HostHidControlCallback, keyboardInstance) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostHidRunSetInterfaceDone: /* 2. hid set idle */
            keyboardInstance->maxPacketSize =
                USB_HostHidGetPacketsize(keyboardInstance->classHandle, USB_ENDPOINT_INTERRUPT, USB_IN);

            /* first: set idle */
            keyboardInstance->runWaitState = kUSB_HostHidRunWaitSetIdle;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetIdle(keyboardInstance->classHandle, 0, 0, USB_HostHidControlCallback, keyboardInstance) !=
                kStatus_USB_Success)
            {
                usb_echo("error in USB_HostHidSetIdle\r\n");
            }
            break;

        case kUSB_HostHidRunSetIdleDone: /* 3. hid get report descriptor */
            /* get report descriptor */
            hidDescriptor = NULL;
            descriptor    = (uint8_t *)((usb_host_interface_t *)keyboardInstance->interfaceHandle)->interfaceExtension;
            endPosition   = (uint32_t)descriptor +
                          ((usb_host_interface_t *)keyboardInstance->interfaceHandle)->interfaceExtensionLength;

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
                for (index = 0; index < hidDescriptor->bNumDescriptors; ++index)
                {
                    hidClassDescriptor += index;
                    if (hidClassDescriptor->bHidDescriptorType == USB_DESCRIPTOR_TYPE_HID_REPORT)
                    {
                        keyboardReportLength =
                            USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(hidClassDescriptor->wDescriptorLength);
                        break;
                    }
                }
            }
            if (keyboardReportLength > HID_BUFFER_SIZE)
            {
                usb_echo("hid buffer is too small\r\n");
                keyboardInstance->runState = kUSB_HostHidRunIdle;
                return;
            }

            if (keyboardReportLength > 0) /* when report descriptor length is zero, go to next step */
            {
                keyboardInstance->runWaitState = kUSB_HostHidRunWaitGetReportDescriptor;
                keyboardInstance->runState     = kUSB_HostHidRunIdle;
                /* second: get report descriptor */
                USB_HostHidGetReportDescriptor(keyboardInstance->classHandle, keyboardInstance->keyboardBuffer,
                                               keyboardReportLength, USB_HostHidControlCallback, keyboardInstance);
                break;
            }

        case kUSB_HostHidRunGetReportDescriptorDone: /* 4. hid set protocol */
            keyboardInstance->runWaitState = kUSB_HostHidRunWaitSetProtocol;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            /* third: set protocol */
            if (USB_HostHidSetProtocol(keyboardInstance->classHandle, USB_HOST_HID_REQUEST_PROTOCOL_REPORT,
                                       USB_HostHidControlCallback, keyboardInstance) != kStatus_USB_Success)
            {
                usb_echo("error in USB_HostHidSetProtocol\r\n");
            }
            break;

        case kUSB_HostHidRunSetProtocolDone: /* 5. start to receive data */
            keyboardInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(keyboardInstance->classHandle, keyboardInstance->keyboardBuffer,
                                keyboardInstance->maxPacketSize, USB_HostHidInCallback,
                                keyboardInstance) != kStatus_USB_Success)
            {
                usb_echo("error in USB_HostHidRecv\r\n");
            }
            break;

        case kUSB_HostHidRunDataReceived: /* process received data and receive next data */
            USB_HostKeyboardProcessBuffer(keyboardInstance);

            keyboardInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(keyboardInstance->classHandle, keyboardInstance->keyboardBuffer,
                                keyboardInstance->maxPacketSize, USB_HostHidInCallback,
                                keyboardInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            break;

        case kUSB_HostHidRunPrimeDataReceive: /* receive data */
            keyboardInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            keyboardInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(keyboardInstance->classHandle, keyboardInstance->keyboardBuffer,
                                keyboardInstance->maxPacketSize, USB_HostHidInCallback,
                                keyboardInstance) != kStatus_USB_Success)
            {
                usb_echo("error in USB_HostHidRecv\r\n");
            }
            break;

        default:
            break;
    }
}

usb_status_t USB_HostHidKeyboardEvent(usb_device_handle deviceHandle,
                                      usb_host_configuration_handle configurationHandle,
                                      uint32_t eventCode)
{
    usb_host_configuration_t *configuration;
    usb_host_interface_t *interface;
    uint32_t infoValue  = 0U;
    usb_status_t status = kStatus_USB_Success;
    uint8_t interfaceIndex;
    uint8_t id;

    switch (eventCode & 0x0000FFFFU)
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
                if (id != USB_HOST_HID_PROTOCOL_KEYBOARD)
                {
                    continue;
                }
                else
                {
                    if (g_HostHidKeyboard.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_HostHidKeyboard.keyboardBuffer  = s_KeyboardBuffer;
                        g_HostHidKeyboard.deviceHandle    = deviceHandle;
                        g_HostHidKeyboard.interfaceHandle = interface;
                        g_HostHidKeyboard.configHandle    = configurationHandle;
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
            if (g_HostHidKeyboard.configHandle == configurationHandle)
            {
                if ((g_HostHidKeyboard.deviceHandle != NULL) && (g_HostHidKeyboard.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_HostHidKeyboard.deviceState == kStatus_DEV_Idle)
                    {
                        g_HostHidKeyboard.deviceState = kStatus_DEV_Attached;

                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &infoValue);
                        usb_echo("hid keyboard attached:pid=0x%x", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &infoValue);
                        usb_echo("vid=0x%x ", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &infoValue);
                        usb_echo("address=%d\r\n", infoValue);
                    }
                    else
                    {
                        usb_echo("not idle host keyboard instance\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_HostHidKeyboard.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_HostHidKeyboard.configHandle = NULL;
                if (g_HostHidKeyboard.deviceState != kStatus_DEV_Idle)
                {
                    g_HostHidKeyboard.deviceState = kStatus_DEV_Detached;
                }
            }
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}
