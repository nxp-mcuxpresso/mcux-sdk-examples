/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_hid.h"

#include "usb_device_descriptor.h"
#include "mouse.h"

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

uint8_t g_UsbDeviceCurrentConfigure = 0U;
uint8_t g_UsbDeviceInterface[USB_HID_MOUSE_INTERFACE_COUNT];

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
    USB_DEVICE_CONFIGURATION_COUNT,                  /* Number of possible configurations */
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceHidMouseReportDescriptor[] = {
    0x05U, 0x01U, /* Usage Page (Generic Desktop)*/
    0x09U, 0x02U, /* Usage (Mouse) */
    0xA1U, 0x01U, /* Collection (Application) */
    0x09U, 0x01U, /* Usage (Pointer) */

    0xA1U, 0x00U, /* Collection (Physical) */
    0x05U, 0x09U, /* Usage Page (Buttons) */
    0x19U, 0x01U, /* Usage Minimum (01U) */
    0x29U, 0x03U, /* Usage Maximum (03U) */

    0x15U, 0x00U, /* Logical Minimum (0U) */
    0x25U, 0x01U, /* Logical Maximum (1U) */
    0x95U, 0x03U, /* Report Count (3U) */
    0x75U, 0x01U, /* Report Size (1U) */

    0x81U, 0x02U, /* Input(Data, Variable, Absolute) 3U button bit fields */
    0x95U, 0x01U, /* Report Count (1U) */
    0x75U, 0x05U, /* Report Size (5U) */
    0x81U, 0x01U, /* Input (Constant), 5U constant field */

    0x05U, 0x01U, /* Usage Page (Generic Desktop) */
    0x09U, 0x30U, /* Usage (X) */
    0x09U, 0x31U, /* Usage (Y) */
    0x09U, 0x38U, /* Usage (Z) */

    0x15U, 0x81U, /* Logical Minimum (-127) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x03U, /* Report Count (3U) */

    0x81U, 0x06U, /* Input(Data, Variable, Relative), Three position bytes (X & Y & Z)*/
    0xC0U,        /* End collection, Close Pointer collection*/
    0xC0U         /* End collection, Close Mouse collection */
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
    USB_SHORT_GET_LOW(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_HID +
                      USB_DESCRIPTOR_LENGTH_ENDPOINT),
    USB_SHORT_GET_HIGH(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_HID +
                       USB_DESCRIPTOR_LENGTH_ENDPOINT), /* Total length of data returned for this configuration. */
    USB_HID_MOUSE_INTERFACE_COUNT,                      /* Number of interfaces supported by this configuration */
    USB_HID_MOUSE_CONFIGURE_INDEX,                      /* Value to use as an argument to the
                                                             SetConfiguration() request to select this configuration */
    0x00U,                                              /* Index of string descriptor describing this configuration */
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
                                      *  (i.e., 50 = 100 mA).
                                      */
    USB_DESCRIPTOR_LENGTH_INTERFACE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,   /* INTERFACE Descriptor Type */
    USB_HID_MOUSE_INTERFACE_INDEX,   /* Number of this interface. */
    USB_HID_MOUSE_INTERFACE_ALTERNATE_0, /* Value used to select this alternate setting
                                        for the interface identified in the prior field */
    USB_HID_MOUSE_ENDPOINT_COUNT,    /* Number of endpoints used by this
                                          interface (excluding endpoint zero). */
    USB_HID_MOUSE_CLASS,             /* Class code (assigned by the USB-IF). */
    USB_HID_MOUSE_SUBCLASS,          /* Subclass code (assigned by the USB-IF). */
    USB_HID_MOUSE_PROTOCOL,          /* Protocol code (assigned by the USB). */
    0x00U,                           /* Index of string descriptor describing this interface */

    USB_DESCRIPTOR_LENGTH_HID,      /* Numeric expression that is the total size of the
                                       HID descriptor. */
    USB_DESCRIPTOR_TYPE_HID,        /* Constant name specifying type of HID
                                       descriptor. */
    0x00U, 0x01U,                   /* Numeric expression identifying the HID Class
                                       Specification release. */
    0x00U,                          /* Numeric expression identifying country code of
                                       the localized hardware */
    0x01U,                          /* Numeric expression specifying the number of
                                       class descriptors(at least one report descriptor) */
    USB_DESCRIPTOR_TYPE_HID_REPORT, /* Constant name identifying type of class descriptor. */
    USB_SHORT_GET_LOW(USB_DESCRIPTOR_LENGTH_HID_MOUSE_REPORT),
    USB_SHORT_GET_HIGH(USB_DESCRIPTOR_LENGTH_HID_MOUSE_REPORT),
    /* Numeric expression that is the total size of the
       Report descriptor. */
    USB_DESCRIPTOR_LENGTH_ENDPOINT, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_ENDPOINT,   /* ENDPOINT Descriptor Type */
    USB_HID_MOUSE_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
    /* The address of the endpoint on the USB device
       described by this descriptor. */
    USB_ENDPOINT_INTERRUPT, /* This field describes the endpoint's attributes */
    USB_SHORT_GET_LOW(FS_HID_MOUSE_INTERRUPT_IN_PACKET_SIZE), USB_SHORT_GET_HIGH(FS_HID_MOUSE_INTERRUPT_IN_PACKET_SIZE),
    /* Maximum packet size this endpoint is capable of
       sending or receiving when this configuration is
       selected. */
    FS_HID_MOUSE_INTERRUPT_IN_INTERVAL, /* Interval for polling endpoint for data transfers. */
};

#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceQualifierDescriptor[USB_DESCRIPTOR_LENGTH_DEVICE_QUALITIER] = {
    USB_DESCRIPTOR_LENGTH_DEVICE_QUALITIER, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_DEVICE_QUALITIER,   /* DEVICE Descriptor Type */
    USB_SHORT_GET_LOW(USB_DEVICE_SPECIFIC_BCD_VERSION),
    USB_SHORT_GET_HIGH(USB_DEVICE_SPECIFIC_BCD_VERSION), /* USB Specification Release Number in
                                                            Binary-Coded Decimal (i.e., 2.10 is 210H). */
    USB_DEVICE_CLASS,                                    /* Class code (assigned by the USB-IF). */
    USB_DEVICE_SUBCLASS,                                 /* Subclass code (assigned by the USB-IF). */
    USB_DEVICE_PROTOCOL,                                 /* Protocol code (assigned by the USB-IF). */
    USB_CONTROL_MAX_PACKET_SIZE,                         /* Maximum packet size for endpoint zero
                                                            (only 8, 16, 32, or 64 are valid) */
    0x00U,                                               /* Number of Other-speed Configurations */
    0x00U,                                               /* Reserved for future use, must be zero */
};
#endif

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
    2U + 2U * 16U, USB_DESCRIPTOR_TYPE_STRING,
    'H',           0x00U,
    'I',           0x00U,
    'D',           0x00U,
    ' ',           0x00U,
    'M',           0x00U,
    'O',           0x00U,
    'U',           0x00U,
    'S',           0x00U,
    'E',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'E',           0x00U,
    'V',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    'E',           0x00U,
};

uint32_t g_UsbDeviceStringDescriptorLength[USB_DEVICE_STRING_COUNT] = {
    sizeof(g_UsbDeviceString0),
    sizeof(g_UsbDeviceString1),
    sizeof(g_UsbDeviceString2),
};

uint8_t *g_UsbDeviceStringDescriptorArray[USB_DEVICE_STRING_COUNT] = {
    g_UsbDeviceString0,
    g_UsbDeviceString1,
    g_UsbDeviceString2,
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
            *buffer = g_UsbDeviceHidMouseReportDescriptor;
            *length = USB_DESCRIPTOR_LENGTH_HID_MOUSE_REPORT;
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
            /* Get device descriptor */
            *buffer = g_UsbDeviceDescriptor;
            *length = USB_DESCRIPTOR_LENGTH_DEVICE;
        }
        break;
        case USB_DESCRIPTOR_TYPE_CONFIGURE:
        {
            /* Get configuration descriptor */
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
    if (interface < USB_HID_MOUSE_INTERFACE_COUNT)
    {
        g_UsbDeviceInterface[interface] = alternateSetting;
        return USB_DeviceCallback(handle, kUSB_DeviceEventSetInterface, &interface);
    }
    return kStatus_USB_InvalidRequest;
}

/* Get current alternate settting of the interface request */
usb_status_t USB_DeviceGetInterface(usb_device_handle handle, uint8_t interface, uint8_t *alternateSetting)
{
    if (interface < USB_HID_MOUSE_INTERFACE_COUNT)
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
            if (USB_SPEED_HIGH == speed)
            {
                if (((descriptorHead->endpoint.bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                     USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN) &&
                    (USB_HID_MOUSE_ENDPOINT_IN ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = HS_HID_MOUSE_INTERRUPT_IN_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(HS_HID_MOUSE_INTERRUPT_IN_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
            }
            else
            {
                if (((descriptorHead->endpoint.bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                     USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN) &&
                    (USB_HID_MOUSE_ENDPOINT_IN ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = FS_HID_MOUSE_INTERRUPT_IN_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(FS_HID_MOUSE_INTERRUPT_IN_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
            }
        }
        descriptorHead = (usb_descriptor_union_t *)((uint8_t *)descriptorHead + descriptorHead->common.bLength);
    }

    return kStatus_USB_Success;
}
