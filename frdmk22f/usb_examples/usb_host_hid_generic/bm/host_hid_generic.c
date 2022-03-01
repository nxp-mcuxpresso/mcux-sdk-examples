/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_hid.h"
#include "host_hid_generic.h"
#include "app.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief process hid data and print generic data.
 *
 * @param genericInstance   hid generic instance pointer.
 */
static void USB_HostHidGenericProcessBuffer(usb_host_hid_generic_instance_t *genericInstance);

/*!
 * @brief host hid generic control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param      the host hid generic instance pointer.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status           transfer result status.
 */
static void USB_HostHidControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

/*!
 * @brief host hid generic interrupt in transfer callback.
 *
 * This function is used as callback function when call USB_HostHidRecv .
 *
 * @param param      the host hid generic instance pointer.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status           transfer result status.
 */
static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

/*!
 * @brief host hid generic interrupt out transfer callback.
 *
 * This function is used as callback function when call USB_HostHidSend .
 *
 * @param param    the host hid generic instance pointer.
 * @param data     data buffer pointer.
 * @param dataLength data length.
 * @status         transfer result status.
 */
static void USB_HostHidOutCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

/*!
 * @brief host hid generic prepare data for sending.
 *
 * @param genericInstance  the host hid generic instance pointer.
 *
 * @retval kStatus_USB_Sucess  there is data.
 * @retval kStatus_USB_Error   data is sent done.
 */
static usb_status_t USB_HostHidGenericPrepareOutData(usb_host_hid_generic_instance_t *genericInstance);

/*******************************************************************************
 * Variables
 ******************************************************************************/

usb_host_hid_generic_instance_t g_HostHidGeneric; /* hid generic instance */
uint8_t testData[] =
    "Test string: This is usb host hid generic demo, it only support pid=0x00a2 and vid=0x1fc9 hid device. Host send "
    "this test string to device, device reply the data to host then host print the data\r\n";

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t s_GenericInBuffer[HID_GENERIC_IN_BUFFER_SIZE]; /*!< use to receive report descriptor and data */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t s_GenericOutBuffer[HID_GENERIC_IN_BUFFER_SIZE]; /*!< use to send data */

/*******************************************************************************
 * Code
 ******************************************************************************/

static void USB_HostHidGenericProcessBuffer(usb_host_hid_generic_instance_t *genericInstance)
{
    genericInstance->genericInBuffer[genericInstance->inMaxPacketSize] = 0;

    usb_echo("%s", genericInstance->genericInBuffer);
}

static void USB_HostHidControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_hid_generic_instance_t *genericInstance = (usb_host_hid_generic_instance_t *)param;

    if (kStatus_USB_TransferStall == status)
    {
        usb_echo("device don't support this ruquest \r\n");
    }
    else if (kStatus_USB_Success != status)
    {
        usb_echo("control transfer failed\r\n");
    }
    else
    {
    }

    if (genericInstance->runWaitState == kUSB_HostHidRunWaitSetInterface) /* set interface finish */
    {
        genericInstance->runState = kUSB_HostHidRunSetInterfaceDone;
    }
    else if (genericInstance->runWaitState == kUSB_HostHidRunWaitSetIdle) /* hid set idle finish */
    {
        genericInstance->runState = kUSB_HostHidRunSetIdleDone;
    }
    else if (genericInstance->runWaitState ==
             kUSB_HostHidRunWaitGetReportDescriptor) /* hid get report descriptor finish */
    {
        genericInstance->runState = kUSB_HostHidRunGetReportDescriptorDone;
    }
    else if (genericInstance->runWaitState == kUSB_HostHidRunWaitSetProtocol) /* hid set protocol finish */
    {
        genericInstance->runState = kUSB_HostHidRunSetProtocolDone;
    }
    else
    {
    }
}

static void USB_HostHidInCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_hid_generic_instance_t *genericInstance = (usb_host_hid_generic_instance_t *)param;

    if (genericInstance->runWaitState == kUSB_HostHidRunWaitDataReceived)
    {
        if (status == kStatus_USB_Success)
        {
            genericInstance->runState = kUSB_HostHidRunDataReceived; /* go to process data */
        }
        else
        {
            if (genericInstance->deviceState == kStatus_DEV_Attached)
            {
                genericInstance->runState = kUSB_HostHidRunPrimeDataReceive; /* go to prime next receiving */
            }
        }
    }
}

static void USB_HostHidOutCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{ /* NULL */
}

static usb_status_t USB_HostHidGenericPrepareOutData(usb_host_hid_generic_instance_t *genericInstance)
{
    uint16_t index = 0;

    if (genericInstance->sendIndex < (sizeof(testData) - 1)) /* sendIndex indicate the current position of testData */
    {
        /* get the max packet data, note: the data should be 0 when there is no actual data to send */
        for (index = 0; ((index + genericInstance->sendIndex) < (sizeof(testData) - 1)) &&
                        (index < genericInstance->outMaxPacketSize);
             ++index)
        {
            genericInstance->genericOutBuffer[index] = testData[index + genericInstance->sendIndex];
        }
        for (; index < genericInstance->outMaxPacketSize; ++index)
        {
            genericInstance->genericOutBuffer[index] = 0x00;
        }
        genericInstance->sendIndex += genericInstance->outMaxPacketSize;

        return kStatus_USB_Success;
    }
    else
    {
        return kStatus_USB_Error; /* there is no data to send */
    }
}

void USB_HostHidGenericTask(void *param)
{
    usb_status_t status;
    usb_host_hid_descriptor_t *hidDescriptor;
    uint32_t hidReportLength = 0;
    uint8_t *descriptor;
    uint32_t endPosition;
    usb_host_hid_generic_instance_t *genericInstance = (usb_host_hid_generic_instance_t *)param;

    /* device state changes, process once for each state */
    if (genericInstance->deviceState != genericInstance->prevState)
    {
        genericInstance->prevState = genericInstance->deviceState;
        switch (genericInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached: /* deivce is attached and numeration is done */
                genericInstance->runState = kUSB_HostHidRunSetInterface;
                if (USB_HostHidInit(genericInstance->deviceHandle, &genericInstance->classHandle) !=
                    kStatus_USB_Success)
                {
                    usb_echo("host hid class initialize fail\r\n");
                }
                else
                {
                    usb_echo("hid generic attached\r\n");
                }
                genericInstance->sendIndex = 0;
                break;

            case kStatus_DEV_Detached: /* device is detached */
                genericInstance->deviceState = kStatus_DEV_Idle;
                genericInstance->runState    = kUSB_HostHidRunIdle;
                USB_HostHidDeinit(genericInstance->deviceHandle, genericInstance->classHandle);
                genericInstance->classHandle = NULL;
                usb_echo("hid generic detached\r\n");
                break;

            default:
                break;
        }
    }

    /* run state */
    switch (genericInstance->runState)
    {
        case kUSB_HostHidRunIdle:
            break;

        case kUSB_HostHidRunSetInterface: /* 1. set hid interface */
            genericInstance->runWaitState = kUSB_HostHidRunWaitSetInterface;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetInterface(genericInstance->classHandle, genericInstance->interfaceHandle, 0,
                                        USB_HostHidControlCallback, genericInstance) != kStatus_USB_Success)
            {
                usb_echo("set interface error\r\n");
            }
            break;

        case kUSB_HostHidRunSetInterfaceDone: /* 2. hid set idle */
            genericInstance->inMaxPacketSize =
                USB_HostHidGetPacketsize(genericInstance->classHandle, USB_ENDPOINT_INTERRUPT, USB_IN);
            genericInstance->outMaxPacketSize =
                USB_HostHidGetPacketsize(genericInstance->classHandle, USB_ENDPOINT_INTERRUPT, USB_OUT);

            /* first: set idle */
            genericInstance->runWaitState = kUSB_HostHidRunWaitSetIdle;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidSetIdle(genericInstance->classHandle, 0, 0, USB_HostHidControlCallback, genericInstance) !=
                kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidSetIdle\r\n");
            }
            break;

        case kUSB_HostHidRunSetIdleDone: /* 3. hid get report descriptor */
            /* get report descriptor's length */
            hidDescriptor = NULL;
            descriptor    = (uint8_t *)((usb_host_interface_t *)genericInstance->interfaceHandle)->interfaceExtension;
            endPosition   = (uint32_t)descriptor +
                          ((usb_host_interface_t *)genericInstance->interfaceHandle)->interfaceExtensionLength;

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
                        hidReportLength =
                            (uint16_t)USB_SHORT_FROM_LITTLE_ENDIAN_ADDRESS(hidClassDescriptor->wDescriptorLength);
                        break;
                    }
                }
            }
            if (hidReportLength > HID_GENERIC_IN_BUFFER_SIZE)
            {
                usb_echo("hid buffer is too small\r\n");
                genericInstance->runState = kUSB_HostHidRunIdle;
                return;
            }

            genericInstance->runWaitState = kUSB_HostHidRunWaitGetReportDescriptor;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            /* second: get report descriptor */
            USB_HostHidGetReportDescriptor(genericInstance->classHandle, genericInstance->genericInBuffer,
                                           hidReportLength, USB_HostHidControlCallback, genericInstance);
            break;

        case kUSB_HostHidRunGetReportDescriptorDone: /* 4. hid set protocol */
            genericInstance->runWaitState = kUSB_HostHidRunWaitSetProtocol;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            /* third: set protocol */
            if (USB_HostHidSetProtocol(genericInstance->classHandle, USB_HOST_HID_REQUEST_PROTOCOL_REPORT,
                                       USB_HostHidControlCallback, genericInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidSetProtocol\r\n");
            }
            break;

        case kUSB_HostHidRunSetProtocolDone: /* 5. start to receive data and send data */
            genericInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(genericInstance->classHandle, genericInstance->genericInBuffer,
                                genericInstance->inMaxPacketSize, USB_HostHidInCallback,
                                genericInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            status = USB_HostHidGenericPrepareOutData(genericInstance);
            if (status == kStatus_USB_Success)
            {
                if (USB_HostHidSend(genericInstance->classHandle, genericInstance->genericOutBuffer,
                                    genericInstance->outMaxPacketSize, USB_HostHidOutCallback,
                                    genericInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostHidSend\r\n");
                }
            }
            break;

        case kUSB_HostHidRunDataReceived: /* process received data, receive next data and send next data */
            USB_HostHidGenericProcessBuffer(genericInstance);

            genericInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(genericInstance->classHandle, genericInstance->genericInBuffer,
                                genericInstance->inMaxPacketSize, USB_HostHidInCallback,
                                genericInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            status = USB_HostHidGenericPrepareOutData(genericInstance);
            if (status == kStatus_USB_Success)
            {
                if (USB_HostHidSend(genericInstance->classHandle, genericInstance->genericOutBuffer,
                                    genericInstance->outMaxPacketSize, USB_HostHidOutCallback,
                                    genericInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostHidSend\r\n");
                }
            }
            break;

        case kUSB_HostHidRunPrimeDataReceive: /* receive next data and send next data */
            genericInstance->runWaitState = kUSB_HostHidRunWaitDataReceived;
            genericInstance->runState     = kUSB_HostHidRunIdle;
            if (USB_HostHidRecv(genericInstance->classHandle, genericInstance->genericInBuffer,
                                genericInstance->inMaxPacketSize, USB_HostHidInCallback,
                                genericInstance) != kStatus_USB_Success)
            {
                usb_echo("Error in USB_HostHidRecv\r\n");
            }
            status = USB_HostHidGenericPrepareOutData(genericInstance);
            if (status == kStatus_USB_Success)
            {
                if (USB_HostHidSend(genericInstance->classHandle, genericInstance->genericOutBuffer,
                                    genericInstance->outMaxPacketSize, USB_HostHidOutCallback,
                                    genericInstance) != kStatus_USB_Success)
                {
                    usb_echo("Error in USB_HostHidSend\r\n");
                }
            }
            break;

        default:
            break;
    }
}

usb_status_t USB_HostHidGenericEvent(usb_device_handle deviceHandle,
                                     usb_host_configuration_handle configurationHandle,
                                     uint32_t eventCode)
{
    uint32_t pid = 0U;
    uint32_t vid = 0U;
    usb_host_configuration_t *configuration;
    usb_host_interface_t *interface;
    uint32_t infoValue  = 0U;
    usb_status_t status = kStatus_USB_Success;
    uint8_t interfaceIndex;
    uint8_t id;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[0];
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
                USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &pid);
                USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &vid);
                if ((pid == 0x00a2) && (vid == 0x1fc9))
                {
                    if (g_HostHidGeneric.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_HostHidGeneric.genericInBuffer  = s_GenericInBuffer;
                        g_HostHidGeneric.genericOutBuffer = s_GenericOutBuffer;
                        g_HostHidGeneric.deviceHandle     = deviceHandle;
                        g_HostHidGeneric.interfaceHandle  = interface;
                        g_HostHidGeneric.configHandle     = configurationHandle;
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
            if (g_HostHidGeneric.configHandle == configurationHandle)
            {
                if ((g_HostHidGeneric.deviceHandle != NULL) && (g_HostHidGeneric.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_HostHidGeneric.deviceState == kStatus_DEV_Idle)
                    {
                        g_HostHidGeneric.deviceState = kStatus_DEV_Attached;

                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDevicePID, &infoValue);
                        usb_echo("hid generic attached:pid=0x%x", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &infoValue);
                        usb_echo("vid=0x%x ", infoValue);
                        USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceAddress, &infoValue);
                        usb_echo("address=%d\r\n", infoValue);
                    }
                    else
                    {
                        usb_echo("not idle generic instance\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_HostHidGeneric.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_HostHidGeneric.configHandle = NULL;
                if (g_HostHidGeneric.deviceState != kStatus_DEV_Idle)
                {
                    g_HostHidGeneric.deviceState = kStatus_DEV_Detached;
                }
            }
            break;

        default:
            break;
    }
    return status;
}
