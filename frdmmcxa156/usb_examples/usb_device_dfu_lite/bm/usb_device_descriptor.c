/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2017 - 2018 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_descriptor.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
extern usb_status_t USB_DeviceCallback(usb_device_handle deviceHandle, uint32_t event, void *param);
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern uint8_t g_detachRequest;
uint8_t g_UsbDeviceCurrentConfigure = 0U;
uint8_t g_UsbDeviceInterface[USB_DFU_INTERFACE_COUNT];

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_DEVICE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DEVICE,   /* DEVICE Descriptor Type */
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
    USB_DEVICE_CLASS,                                    /* Class code (assigned by the USB-IF). */
    USB_DEVICE_SUBCLASS,                                 /* Subclass code (assigned by the USB-IF). */
    USB_DEVICE_PROTOCOL,                                 /* Protocol code (assigned by the USB-IF). */
    USB_CONTROL_MAX_PACKET_SIZE,                         /* Maximum packet size for endpoint zero
                                                            (only 8, 16, 32, or 64 are valid) */
    USB_SHORT_GET_LOW(USB_DEVICE_VID),
    USB_SHORT_GET_HIGH(USB_DEVICE_VID), /* Vendor ID (assigned by the USB-IF) */
    USB_SHORT_GET_LOW(USB_DEVICE_PID),
    USB_SHORT_GET_HIGH(USB_DEVICE_PID), /* Product ID (assigned by the manufacturer) */
    USB_SHORT_GET_LOW(USB_DEVICE_DEMO_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_DEMO_BCD_VERSION), /* Device release number in binary-coded decimal */
    0x01U,                                           /* Index of string descriptor describing manufacturer */
    0x02U,                                           /* Index of string descriptor describing product */
    0x00U,                                           /* Index of string descriptor describing the
                                                        device's serial number */
    USB_DFU_INTERFACE_COUNT,                         /* Number of possible configurations */
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
    USB_SHORT_GET_LOW(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE +
                      USB_DESCRIPTOR_LENGTH_FUNCTINAL),
    USB_SHORT_GET_HIGH(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE +
                       USB_DESCRIPTOR_LENGTH_FUNCTINAL),
    /* Total length of data returned for this configuration. */
    USB_DFU_INTERFACE_COUNT, /* Number of interfaces supported by this configuration */
    USB_DFU_CONFIGURE_INDEX, /* Value to use as an argument to the
                                        SetConfiguration() request to select this configuration */
    0x00U,                   /* Index of string descriptor describing this configuration */
    (USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_D7_MASK) |
        (USB_DEVICE_CONFIG_SELF_POWER << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_SELF_POWERED_SHIFT) |
        (USB_DEVICE_CONFIG_REMOTE_WAKEUP << USB_DESCRIPTOR_CONFIGURE_ATTRIBUTE_REMOTE_WAKEUP_SHIFT),
    /* Configuration characteristics
         D7: Reserved (set to one)
         D6: Self-powered
         D5: Remote Wakeup
         D4...0: Reserved (reset to zero)
    */
    USB_DEVICE_MAX_POWER,            /* Maximum power consumption of the USB
                                      * device from the bus in this specific
                                      * configuration when the device is fully
                                      * operational. Expressed in 2 mA units
                                      * (i.e., 50 = 100 mA).
                                      */
    USB_DESCRIPTOR_LENGTH_INTERFACE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,   /* INTERFACE Descriptor Type */
    USB_DFU_INTERFACE_INDEX,         /* Number of this interface. */
    USB_DFU_INTERFACE_ALTERNATE_0,   /* Value used to select this alternate setting
                                        for the interface identified in the prior field */
    0x00,                            /* Only the control endpoint is used */
    USB_DFU_CLASS,                   /* Class code (assigned by the USB-IF). */
    USB_DFU_SUBCLASS,                /* Subclass code (assigned by the USB-IF). */
    USB_DFU_MODE_PROTOCOL,           /* Protocol code (assigned by the USB). */
    0x03U,                           /* Index of string descriptor describing this interface */

    USB_DESCRIPTOR_LENGTH_FUNCTINAL,    /* size of DFU functional descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DFU_FUNCTIONAL, /* DFU functional descriptor type */
    (USB_DFU_BIT_WILL_DETACH << 3U) | (USB_DFU_BIT_MANIFESTATION_TOLERANT << 2U) | (USB_DFU_BIT_CAN_UPLOAD << 1U) |
        USB_DFU_BIT_CAN_DNLOAD,                                                            /* DFU attributes */
    USB_SHORT_GET_LOW(USB_DFU_DETACH_TIMEOUT), USB_SHORT_GET_HIGH(USB_DFU_DETACH_TIMEOUT), /* wDetachTimeout */
    USB_SHORT_GET_LOW(MAX_TRANSFER_SIZE), USB_SHORT_GET_HIGH(MAX_TRANSFER_SIZE),           /* Max transfer size */
    USB_SHORT_GET_LOW(USB_DEVICE_DEMO_BCD_VERSION), USB_SHORT_GET_HIGH(USB_DEVICE_DEMO_BCD_VERSION), /* bcdDFUVersion */
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString0[] = {
    2U + 2U,
    USB_DESCRIPTOR_TYPE_STRING,
    0x09U,
    0x04U,
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString1[] = {
    2U + 2U * 18U, USB_DESCRIPTOR_TYPE_STRING,
    'N',           0x00U,
    'X',           0x00U,
    'P',           0x00U,
    ' ',           0x00U,
    'S',           0x00U,
    'E',           0x00U,
    'M',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    'O',           0x00U,
    'N',           0x00U,
    'D',           0x00U,
    'U',           0x00U,
    'C',           0x00U,
    'T',           0x00U,
    'O',           0x00U,
    'R',           0x00U,
    'S',           0x00U,
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString2[] = {
    2U + 2U * 10U, USB_DESCRIPTOR_TYPE_STRING,
    'D',           0x00U,
    'F',           0x00U,
    'U',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'E',           0x00U,
    'V',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    'E',           0x00U,
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString3[] = {
    2U + 2U * 10U, USB_DESCRIPTOR_TYPE_STRING,
    'D',           0x00U,
    'F',           0x00U,
    'U',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'E',           0x00U,
    'V',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    'E',           0x00U,
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceOSString[] = {
    (8 * 2 + 2), USB_DESCRIPTOR_TYPE_STRING,
    'M', /*Signature:*/
    0x00U,       'S',
    0x00U,       'F',
    0x00U,       'T',
    0x00U,       '1',
    0x00U,       '0',
    0x00U,       '0',
    0x00U,       0x00, /*Vendor Code*/
    0x00U,
};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceCompatibleIDDescriptor[] = {
    /*Microsoft Compatible ID Feature Descriptor*/
    /*The Header Section*/
    0x28U, 0x00U, 0x00U, 0x00U,                      /*Descriptor length of the complete extended compat ID descriptor*/
    0x00U, 0x01U,                                    /*Descriptor's version number*/
    0x04U, 0x00U,                                    /*Compatibility ID Descriptor index*/
    0x01U,                                           /*Number of sections */
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, /*Reserved */
    /*The Function Section*/
    0x00U,                                                 /*The interface or function number*/
    0x01U,                                                 /*Reserved */
    'W', 'I', 'N', 'U', 'S', 'B', 0x0U, 0x0U,              /*Compatible ID("WINUSB\0\0") */
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00, /*Sub-Compatible ID*/
    0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,              /*Reserved*/
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceOSExendedDescriptor[] = {
    /*Extended Properties Feature Descriptor*/
    /* Header Section*/
    0x8E,
    0x00,
    0x00,
    0x00, /*the length, in bytes, of the complete extended properties descriptor*/
    USB_SHORT_GET_LOW(USB_DEVICE_OS_DESCRIPTOR_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_OS_DESCRIPTOR_BCD_VERSION), /* The descriptor's version number. */
    0x05U,
    0x00U, /*Descriptor index*/
    0x01U,
    0x00U, /* The number of custom property sections that follow the header section */
    /*custom Property Section*/
    0x84,
    0x00,
    0x00,
    0x00, /*Size of the property section*/
    0x01,
    0x00,
    0x00,
    0x00, /* Property data type 1 stands for Unicode REG_SZ)  */
    0x28,
    0x00, /* Property name length (40 bytes) */
    /* Property Name ("DeviceInterfaceGUID")  */
    'D',
    0,
    'e',
    0,
    'v',
    0,
    'i',
    0,
    'c',
    0,
    'e',
    0,
    'I',
    0,
    'n',
    0,
    't',
    0,
    'e',
    0,
    'r',
    0,
    'f',
    0,
    'a',
    0,
    'c',
    0,
    'e',
    0,
    'G',
    0,
    'U',
    0,
    'I',
    0,
    'D',
    0,
    0,
    0,
    0x4E,
    0x00,
    0x00,
    0x00, /* Property data length (78 bytes) */
    /* Property Name "{36FC9E60-C465-11CF-8056-444553540000}"*/
    '{',
    0,
    '3',
    0,
    '6',
    0,
    'f',
    0,
    'c',
    0,
    '9',
    0,
    'e',
    0,
    '6',
    0,
    '0',
    0,
    '-',
    0,
    'c',
    0,
    '4',
    0,
    '6',
    0,
    '5',
    0,
    '-',
    0,
    '1',
    0,
    '1',
    0,
    'c',
    0,
    'f',
    0,
    '-',
    0,
    '8',
    0,
    '0',
    0,
    '5',
    0,
    '6',
    0,
    '-',
    0,
    '4',
    0,
    '4',
    0,
    '4',
    0,
    '5',
    0,
    '5',
    0,
    '3',
    0,
    '5',
    0,
    '4',
    0,
    '0',
    0,
    '0',
    0,
    '0',
    0,
    '0',
    0,
    '}',
    0,
    0,
    0,
};
uint32_t g_UsbDeviceStringDescriptorLength[USB_DEVICE_STRING_COUNT] = {
    sizeof(g_UsbDeviceString0),
    sizeof(g_UsbDeviceString1),
    sizeof(g_UsbDeviceString2),
    sizeof(g_UsbDeviceString3),

};

uint8_t *g_UsbDeviceStringDescriptorArray[USB_DEVICE_STRING_COUNT] = {
    g_UsbDeviceString0,
    g_UsbDeviceString1,
    g_UsbDeviceString2,
    g_UsbDeviceString3,

};

usb_language_t g_UsbDeviceLanguage[USB_DEVICE_LANGUAGE_COUNT] = {{
    g_UsbDeviceStringDescriptorArray,
    g_UsbDeviceStringDescriptorLength,
    (uint16_t)0x0409U,
}};

usb_language_list_t g_UsbDeviceLanguageList = {
    g_UsbDeviceString0,
    sizeof(g_UsbDeviceString0),
    g_UsbDeviceLanguage,
    USB_DEVICE_LANGUAGE_COUNT,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/* Get verdor descriptor request */
usb_status_t USB_DeviceGetVerdorDescriptor(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t errorReturn = kStatus_USB_InvalidRequest;

    if ((g_UsbDeviceOSString[16] != setup->bRequest) ||
        ((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) != USB_REQUEST_TYPE_DIR_IN))
    {
        /*only handle request to its own verdor*/
        return errorReturn;
    }

    if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_DEVICE)
    {
        if (USB_MICROSOFT_EXTENDED_COMPAT_ID == setup->wIndex)
        {
            *buffer     = g_UsbDeviceCompatibleIDDescriptor;
            *length     = USB_DESCRIPTOR_LENGTH_COMPAT;
            errorReturn = kStatus_USB_Success;
        }
        if (USB_MICROSOFT_EXTENDED_PROPERTIES_ID == setup->wIndex)
        {
            *buffer     = g_UsbDeviceOSExendedDescriptor;
            *length     = USB_DESCRIPTOR_LENGTH_OSExended;
            errorReturn = kStatus_USB_Success;
        }
        else
        {
            /* no action */
        }
    }
    else if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) == USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
    {
        /*add this based on wiki.*/
        /*https://github.com/pbatard/libwdi/wiki/WCID-Devices, Defining a Device Interface GUID or other device specific
         * properties IMPORTANT NOTE 1*/
        if (USB_MICROSOFT_EXTENDED_PROPERTIES_ID == setup->wIndex)
        {
            *buffer     = g_UsbDeviceOSExendedDescriptor;
            *length     = USB_DESCRIPTOR_LENGTH_OSExended;
            errorReturn = kStatus_USB_Success;
        }
    }
    else
    {
        /* no action */
    }

    return errorReturn;
}

/* Get descriptor request */
usb_status_t USB_DeviceGetDescriptor(usb_device_handle handle,
                                     usb_setup_struct_t *setup,
                                     uint32_t *length,
                                     uint8_t **buffer)
{
    usb_status_t error      = kStatus_USB_Success;
    uint8_t descriptorType  = (uint8_t)((setup->wValue & 0xFF00U) >> 8U);
    uint8_t descriptorIndex = (uint8_t)((setup->wValue & 0x00FFU));
    if (USB_REQUEST_STANDARD_GET_DESCRIPTOR != setup->bRequest)
    {
        return kStatus_USB_InvalidRequest;
    }
    switch (descriptorType)
    {
        case USB_DESCRIPTOR_TYPE_HID_REPORT:
        {
            /* Get HID report descriptor */
            error = kStatus_USB_InvalidRequest;
        }
        break;
        case USB_DESCRIPTOR_TYPE_STRING:
        {
            /* Get string descriptor */
            if (0U == descriptorIndex)
            {
                *buffer = (uint8_t *)g_UsbDeviceLanguageList.languageString;
                *length = g_UsbDeviceLanguageList.stringLength;
            }
            else
            {
                uint8_t languageId    = 0U;
                uint8_t languageIndex = USB_DEVICE_STRING_COUNT;

                for (; languageId < USB_DEVICE_LANGUAGE_COUNT; languageId++)
                {
                    if (setup->wIndex == g_UsbDeviceLanguageList.languageList[languageId].languageId)
                    {
                        if (descriptorIndex < USB_DEVICE_STRING_COUNT)
                        {
                            languageIndex = descriptorIndex;
                        }
                        break;
                    }
                }
                if (0xEEU == descriptorIndex)
                {
                    *buffer = (uint8_t *)g_UsbDeviceOSString;
                    *length = sizeof(g_UsbDeviceOSString);
                    return kStatus_USB_Success;
                }
                if (USB_DEVICE_STRING_COUNT == languageIndex)
                {
                    return kStatus_USB_InvalidRequest;
                }
                *buffer = (uint8_t *)g_UsbDeviceLanguageList.languageList[languageId].string[languageIndex];
                *length = g_UsbDeviceLanguageList.languageList[languageId].length[languageIndex];
            }
        }
        break;
        case USB_DESCRIPTOR_TYPE_DEVICE:
        {
            /*g_detachRequest = 0U;*/
            *buffer = g_UsbDeviceDescriptor;
            *length = USB_DESCRIPTOR_LENGTH_DEVICE;
        }
        break;
        case USB_DESCRIPTOR_TYPE_CONFIGURE:
        {
            /*g_detachRequest = 0U;*/
            *buffer = g_UsbDeviceConfigurationDescriptor;
            *length = USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL;
        }
        break;
        default:
            error = kStatus_USB_InvalidRequest;
            break;
    } /* End Switch */
    return error;
}
/* Set current confgiuration request */
usb_status_t USB_DeviceSetConfigure(usb_device_handle handle, uint8_t configure)
{
    if (!configure)
    {
        return kStatus_USB_Error;
    }
    g_UsbDeviceCurrentConfigure = configure;
    return USB_DeviceCallback(handle, kUSB_DeviceEventSetConfiguration, &configure);
}

/* Get current confgiuration request */
usb_status_t USB_DeviceGetConfigure(usb_device_handle handle, uint8_t *configure)
{
    *configure = g_UsbDeviceCurrentConfigure;
    return kStatus_USB_Success;
}
/* Set current alternate settting of the interface request */
usb_status_t USB_DeviceSetInterface(usb_device_handle handle, uint8_t interface, uint8_t alternateSetting)
{
    if (interface < USB_DFU_INTERFACE_COUNT)
    {
        g_UsbDeviceInterface[interface] = alternateSetting;
        return USB_DeviceCallback(handle, kUSB_DeviceEventSetInterface, &interface);
    }
    return kStatus_USB_InvalidRequest;
}

/* Get current alternate settting of the interface request */
usb_status_t USB_DeviceGetInterface(usb_device_handle handle, uint8_t interface, uint8_t *alternateSetting)
{
    if (interface < USB_DFU_INTERFACE_COUNT)
    {
        *alternateSetting = g_UsbDeviceInterface[interface];
        return kStatus_USB_Success;
    }
    return kStatus_USB_InvalidRequest;
}
/* Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
 * current speed.
 * As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
 * KHCI.
 * When the EHCI is enabled, the application needs to call this function to update device by using current speed.
 * The updated information includes endpoint max packet size, endpoint interval, etc. */
usb_status_t USB_DeviceSetSpeed(uint8_t speed)
{
    usb_descriptor_union_t *descriptorHead;
    usb_descriptor_union_t *descriptorTail;

    descriptorHead = (usb_descriptor_union_t *)&g_UsbDeviceConfigurationDescriptor[0];
    descriptorTail =
        (usb_descriptor_union_t *)(&g_UsbDeviceConfigurationDescriptor[USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL - 1U]);

    while (descriptorHead < descriptorTail)
    {
        if (descriptorHead->common.bDescriptorType == USB_DESCRIPTOR_TYPE_ENDPOINT)
        {
            /*TODO*/
        }
        descriptorHead = (usb_descriptor_union_t *)((uint8_t *)descriptorHead + descriptorHead->common.bLength);
    }
    return kStatus_USB_Success;
}
