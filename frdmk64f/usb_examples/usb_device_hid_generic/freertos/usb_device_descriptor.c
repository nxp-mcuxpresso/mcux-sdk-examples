/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_hid.h"

#include "usb_device_descriptor.h"
#include "hid_generic.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* hid generic endpoint information */
usb_device_endpoint_struct_t g_UsbDeviceHidGenericEndpoints[USB_HID_GENERIC_ENDPOINT_COUNT] = {
    /* HID generic interrupt IN pipe */
    {
        USB_HID_GENERIC_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_INTERRUPT,
        FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE,
        FS_HID_GENERIC_INTERRUPT_IN_INTERVAL,
    },
    /* HID generic interrupt OUT pipe */
    {
        USB_HID_GENERIC_ENDPOINT_OUT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
        USB_ENDPOINT_INTERRUPT,
        FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE,
        FS_HID_GENERIC_INTERRUPT_OUT_INTERVAL,
    }};

/* HID generic interface information */
usb_device_interface_struct_t g_UsbDeviceHidGenericInterface[] = {{
    USB_HID_GENERIC_INTERFACE_ALTERNATE_0, /* The alternate setting of the interface */
    {
        USB_HID_GENERIC_ENDPOINT_COUNT, /* Endpoint count */
        g_UsbDeviceHidGenericEndpoints, /* Endpoints handle */
    },
    NULL,
}};

usb_device_interfaces_struct_t g_UsbDeviceHidGenericInterfaces[USB_HID_GENERIC_INTERFACE_COUNT] = {
    {
        USB_HID_GENERIC_CLASS,           /* HID generic class code */
        USB_HID_GENERIC_SUBCLASS,        /* HID generic subclass code */
        USB_HID_GENERIC_PROTOCOL,        /* HID generic protocol code */
        USB_HID_GENERIC_INTERFACE_INDEX, /* The interface number of the HID generic */
        g_UsbDeviceHidGenericInterface,  /* Interfaces handle */
        sizeof(g_UsbDeviceHidGenericInterface) / sizeof(usb_device_interface_struct_t),
    },
};

usb_device_interface_list_t g_UsbDeviceHidGenericInterfaceList[USB_DEVICE_CONFIGURATION_COUNT] = {
    {
        USB_HID_GENERIC_INTERFACE_COUNT, /* The interface count of the HID generic */
        g_UsbDeviceHidGenericInterfaces, /* The interfaces handle */
    },
};

usb_device_class_struct_t g_UsbDeviceHidGenericConfig = {
    g_UsbDeviceHidGenericInterfaceList, /* The interface list of the HID generic */
    kUSB_DeviceClassTypeHid,            /* The HID class type */
    USB_DEVICE_CONFIGURATION_COUNT,     /* The configuration count */
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceHidGenericReportDescriptor[] = {
    0x05U, 0x81U, /* Usage Page (Vendor defined)*/
    0x09U, 0x82U, /* Usage (Vendor defined) */
    0xA1U, 0x01U, /* Collection (Application) */
    0x09U, 0x83U, /* Usage (Vendor defined) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* Logical Minimum (-128) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x08U, /* Report Count (8U) */
    0x81U, 0x02U, /* Input(Data, Variable, Absolute) */

    0x09U, 0x84U, /* Usage (Vendor defined) */
    0x15U, 0x80U, /* Logical Minimum (-128) */
    0x25U, 0x7FU, /* Logical Maximum (127) */
    0x75U, 0x08U, /* Report Size (8U) */
    0x95U, 0x08U, /* Report Count (8U) */
    0x91U, 0x02U, /* Output(Data, Variable, Absolute) */
    0xC0U,        /* End collection */
};

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
#if (defined(USB_DEVICE_CONFIG_ROOT2_TEST) && (USB_DEVICE_CONFIG_ROOT2_TEST > 0U))
    0x03U, /* Index of string descriptor describing the
              device's serial number */
#else
    0x00U,
#endif
    USB_DEVICE_CONFIGURATION_COUNT, /* Number of possible configurations */
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceConfigurationDescriptor[] = {
    USB_DESCRIPTOR_LENGTH_CONFIGURE, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_CONFIGURE,   /* CONFIGURATION Descriptor Type */
    USB_SHORT_GET_LOW(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_HID +
                      USB_DESCRIPTOR_LENGTH_ENDPOINT + USB_DESCRIPTOR_LENGTH_ENDPOINT),
    USB_SHORT_GET_HIGH(USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE + USB_DESCRIPTOR_LENGTH_HID +
                       USB_DESCRIPTOR_LENGTH_ENDPOINT +
                       USB_DESCRIPTOR_LENGTH_ENDPOINT), /* Total length of data returned for this configuration. */
    USB_HID_GENERIC_INTERFACE_COUNT,                    /* Number of interfaces supported by this configuration */
    USB_HID_GENERIC_CONFIGURE_INDEX,                    /* Value to use as an argument to the
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
    USB_DEVICE_MAX_POWER,                  /* Maximum power consumption of the USB
                                            * device from the bus in this specific
                                            * configuration when the device is fully
                                            * operational. Expressed in 2 mA units
                                            *  (i.e., 50 = 100 mA).
                                            */
    USB_DESCRIPTOR_LENGTH_INTERFACE,       /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_INTERFACE,         /* INTERFACE Descriptor Type */
    USB_HID_GENERIC_INTERFACE_INDEX,       /* Number of this interface. */
    USB_HID_GENERIC_INTERFACE_ALTERNATE_0, /* Value used to select this alternate setting
                                        for the interface identified in the prior field */
    USB_HID_GENERIC_ENDPOINT_COUNT,        /* Number of endpoints used by this
                                              interface (excluding endpoint zero). */
    USB_HID_GENERIC_CLASS,                 /* Class code (assigned by the USB-IF). */
    USB_HID_GENERIC_SUBCLASS,              /* Subclass code (assigned by the USB-IF). */
    USB_HID_GENERIC_PROTOCOL,              /* Protocol code (assigned by the USB). */
    0x00U,                                 /* Index of string descriptor describing this interface */

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
    USB_SHORT_GET_LOW(USB_DESCRIPTOR_LENGTH_HID_GENERIC_REPORT),
    USB_SHORT_GET_HIGH(USB_DESCRIPTOR_LENGTH_HID_GENERIC_REPORT),
    /* Numeric expression that is the total size of the
       Report descriptor. */
    USB_DESCRIPTOR_LENGTH_ENDPOINT, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_ENDPOINT,   /* ENDPOINT Descriptor Type */
    USB_HID_GENERIC_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
    /* The address of the endpoint on the USB device
       described by this descriptor. */
    USB_ENDPOINT_INTERRUPT, /* This field describes the endpoint's attributes */
    USB_SHORT_GET_LOW(FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE),
    USB_SHORT_GET_HIGH(FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE),
    /* Maximum packet size this endpoint is capable of
       sending or receiving when this configuration is
       selected. */
    FS_HID_GENERIC_INTERRUPT_IN_INTERVAL, /* Interval for polling endpoint for data transfers. */

    USB_DESCRIPTOR_LENGTH_ENDPOINT, /* Size of this descriptor in bytes */
    USB_DESCRIPTOR_TYPE_ENDPOINT,   /* ENDPOINT Descriptor Type */
    USB_HID_GENERIC_ENDPOINT_OUT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT),
    /* The address of the endpoint on the USB device
       described by this descriptor. */
    USB_ENDPOINT_INTERRUPT, /* This field describes the endpoint's attributes */
    USB_SHORT_GET_LOW(FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE),
    USB_SHORT_GET_HIGH(FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE),
    /* Maximum packet size this endpoint is capable of
       sending or receiving when this configuration is
       selected. */
    FS_HID_GENERIC_INTERRUPT_OUT_INTERVAL, /* Interval for polling endpoint for data transfers. */
};

#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceQualifierDescriptor[] = {
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
    2U + 2U * 18U, USB_DESCRIPTOR_TYPE_STRING,
    'H',           0x00U,
    'I',           0x00U,
    'D',           0x00U,
    ' ',           0x00U,
    'G',           0x00U,
    'E',           0x00U,
    'N',           0x00U,
    'E',           0x00U,
    'R',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    ' ',           0x00U,
    'D',           0x00U,
    'E',           0x00U,
    'V',           0x00U,
    'I',           0x00U,
    'C',           0x00U,
    'E',           0x00U,
};

#if (defined(USB_DEVICE_CONFIG_ROOT2_TEST) && (USB_DEVICE_CONFIG_ROOT2_TEST > 0U))
/* ROOT2 requirement: Serial String must have 16 Hexadecimal digits, leading four digits are zero */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
uint8_t g_UsbDeviceString3[] = {
    (16 * 2 + 2), USB_DESCRIPTOR_TYPE_STRING,
    '0',          0,
    '0',          0,
    '0',          0,
    '0',          0,
    '1',          0,
    '2',          0,
    '3',          0,
    '4',          0,
    '5',          0,
    '6',          0,
    '7',          0,
    '8',          0,
    '9',          0,
    '0',          0,
    '0',          0,
    '0',          0,
};

/* Microsoft OS must be 12 03 4D 00 53 00 46 00 54 00 31 00 30 00 30 00 90 00 */
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
    0x00U,       0x90U, /*Vendor Code*/
    0x00U,
};
#endif /* USB_DEVICE_CONFIG_ROOT2_TEST */

uint32_t g_UsbDeviceStringDescriptorLength[USB_DEVICE_STRING_COUNT] = {
    sizeof(g_UsbDeviceString0),
    sizeof(g_UsbDeviceString1),
    sizeof(g_UsbDeviceString2),
#if (defined(USB_DEVICE_CONFIG_ROOT2_TEST) && (USB_DEVICE_CONFIG_ROOT2_TEST > 0U))
    sizeof(g_UsbDeviceString3),
#endif
};

uint8_t *g_UsbDeviceStringDescriptorArray[USB_DEVICE_STRING_COUNT] = {
    g_UsbDeviceString0,
    g_UsbDeviceString1,
    g_UsbDeviceString2,
#if (defined(USB_DEVICE_CONFIG_ROOT2_TEST) && (USB_DEVICE_CONFIG_ROOT2_TEST > 0U))
    g_UsbDeviceString3,
#endif
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

/* Get device descriptor request */
usb_status_t USB_DeviceGetDeviceDescriptor(usb_device_handle handle,
                                           usb_device_get_device_descriptor_struct_t *deviceDescriptor)
{
    deviceDescriptor->buffer = g_UsbDeviceDescriptor;
    deviceDescriptor->length = USB_DESCRIPTOR_LENGTH_DEVICE;
    return kStatus_USB_Success;
}
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
/* Get device qualifier descriptor request */
usb_status_t USB_DeviceGetDeviceQualifierDescriptor(
    usb_device_handle handle, usb_device_get_device_qualifier_descriptor_struct_t *deviceQualifierDescriptor)
{
    deviceQualifierDescriptor->buffer = g_UsbDeviceQualifierDescriptor;
    deviceQualifierDescriptor->length = USB_DESCRIPTOR_LENGTH_DEVICE_QUALITIER;
    return kStatus_USB_Success;
}
#endif
/* Get device configuration descriptor request */
usb_status_t USB_DeviceGetConfigurationDescriptor(
    usb_device_handle handle, usb_device_get_configuration_descriptor_struct_t *configurationDescriptor)
{
    if (USB_HID_GENERIC_CONFIGURE_INDEX > configurationDescriptor->configuration)
    {
        configurationDescriptor->buffer = g_UsbDeviceConfigurationDescriptor;
        configurationDescriptor->length = USB_DESCRIPTOR_LENGTH_CONFIGURATION_ALL;
        return kStatus_USB_Success;
    }
    return kStatus_USB_InvalidRequest;
}

/* Get device string descriptor request */
usb_status_t USB_DeviceGetStringDescriptor(usb_device_handle handle,
                                           usb_device_get_string_descriptor_struct_t *stringDescriptor)
{
    if (stringDescriptor->stringIndex == 0U)
    {
        stringDescriptor->buffer = (uint8_t *)g_UsbDeviceLanguageList.languageString;
        stringDescriptor->length = g_UsbDeviceLanguageList.stringLength;
    }
    else
    {
        uint8_t languageId    = 0U;
        uint8_t languageIndex = USB_DEVICE_STRING_COUNT;

        for (; languageId < USB_DEVICE_LANGUAGE_COUNT; languageId++)
        {
            if (stringDescriptor->languageId == g_UsbDeviceLanguageList.languageList[languageId].languageId)
            {
                if (stringDescriptor->stringIndex < USB_DEVICE_STRING_COUNT)
                {
                    languageIndex = stringDescriptor->stringIndex;
                }
                break;
            }
        }
#if (defined(USB_DEVICE_CONFIG_ROOT2_TEST) && (USB_DEVICE_CONFIG_ROOT2_TEST > 0U))
        if (0xEE == stringDescriptor->stringIndex)
        {
            stringDescriptor->buffer = (uint8_t *)g_UsbDeviceOSString;
            stringDescriptor->length = sizeof(g_UsbDeviceOSString);
            return kStatus_USB_Success;
        }
#endif
        if (USB_DEVICE_STRING_COUNT == languageIndex)
        {
            return kStatus_USB_InvalidRequest;
        }
        stringDescriptor->buffer = (uint8_t *)g_UsbDeviceLanguageList.languageList[languageId].string[languageIndex];
        stringDescriptor->length = g_UsbDeviceLanguageList.languageList[languageId].length[languageIndex];
    }
    return kStatus_USB_Success;
}

/* Get hid descriptor request */
usb_status_t USB_DeviceGetHidDescriptor(usb_device_handle handle, usb_device_get_hid_descriptor_struct_t *hidDescriptor)
{
    if (USB_HID_GENERIC_INTERFACE_INDEX == hidDescriptor->interfaceNumber)
    {
        hidDescriptor->buffer =
            &g_UsbDeviceConfigurationDescriptor[USB_DESCRIPTOR_LENGTH_CONFIGURE + USB_DESCRIPTOR_LENGTH_INTERFACE];
        hidDescriptor->length = USB_DESCRIPTOR_LENGTH_HID;
    }
    else
    {
        return kStatus_USB_InvalidRequest;
    }
    return kStatus_USB_Success;
}

/* Get hid report descriptor request */
usb_status_t USB_DeviceGetHidReportDescriptor(usb_device_handle handle,
                                              usb_device_get_hid_report_descriptor_struct_t *hidReportDescriptor)
{
    if (USB_HID_GENERIC_INTERFACE_INDEX == hidReportDescriptor->interfaceNumber)
    {
        hidReportDescriptor->buffer = g_UsbDeviceHidGenericReportDescriptor;
        hidReportDescriptor->length = USB_DESCRIPTOR_LENGTH_HID_GENERIC_REPORT;
    }
    else
    {
        return kStatus_USB_InvalidRequest;
    }
    return kStatus_USB_Success;
}

/* Get hid physical descriptor request */
usb_status_t USB_DeviceGetHidPhysicalDescriptor(usb_device_handle handle,
                                                usb_device_get_hid_physical_descriptor_struct_t *hidPhysicalDescriptor)
{
    return kStatus_USB_InvalidRequest;
}

/* Due to the difference of HS and FS descriptors, the device descriptors and configurations need to be updated to match
 * current speed.
 * As the default, the device descriptors and configurations are configured by using FS parameters for both EHCI and
 * KHCI.
 * When the EHCI is enabled, the application needs to call this function to update device by using current speed.
 * The updated information includes endpoint max packet size, endpoint interval, etc. */
usb_status_t USB_DeviceSetSpeed(usb_device_handle handle, uint8_t speed)
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
                    (USB_HID_GENERIC_ENDPOINT_IN ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = HS_HID_GENERIC_INTERRUPT_IN_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(HS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else if (((descriptorHead->endpoint.bEndpointAddress &
                           USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                          USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT) &&
                         (USB_HID_GENERIC_ENDPOINT_OUT ==
                          (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = HS_HID_GENERIC_INTERRUPT_OUT_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(HS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else
                {
                }
            }
            else
            {
                if (((descriptorHead->endpoint.bEndpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                     USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN) &&
                    (USB_HID_GENERIC_ENDPOINT_IN ==
                     (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = FS_HID_GENERIC_INTERRUPT_IN_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else if (((descriptorHead->endpoint.bEndpointAddress &
                           USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                          USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT) &&
                         (USB_HID_GENERIC_ENDPOINT_OUT ==
                          (descriptorHead->endpoint.bEndpointAddress & USB_ENDPOINT_NUMBER_MASK)))
                {
                    descriptorHead->endpoint.bInterval = FS_HID_GENERIC_INTERRUPT_OUT_INTERVAL;
                    USB_SHORT_TO_LITTLE_ENDIAN_ADDRESS(FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE,
                                                       descriptorHead->endpoint.wMaxPacketSize);
                }
                else
                {
                }
            }
        }
        descriptorHead = (usb_descriptor_union_t *)((uint8_t *)descriptorHead + descriptorHead->common.bLength);
    }

    for (int i = 0U; i < USB_HID_GENERIC_ENDPOINT_COUNT; i++)
    {
        if (USB_SPEED_HIGH == speed)
        {
            if (g_UsbDeviceHidGenericEndpoints[i].endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK)
            {
                g_UsbDeviceHidGenericEndpoints[i].maxPacketSize = HS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE;
                g_UsbDeviceHidGenericEndpoints[i].interval      = HS_HID_GENERIC_INTERRUPT_IN_INTERVAL;
            }
            else
            {
                g_UsbDeviceHidGenericEndpoints[i].maxPacketSize = HS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE;
                g_UsbDeviceHidGenericEndpoints[i].interval      = HS_HID_GENERIC_INTERRUPT_OUT_INTERVAL;
            }
        }
        else
        {
            if (g_UsbDeviceHidGenericEndpoints[i].endpointAddress & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK)
            {
                g_UsbDeviceHidGenericEndpoints[i].maxPacketSize = FS_HID_GENERIC_INTERRUPT_IN_PACKET_SIZE;
                g_UsbDeviceHidGenericEndpoints[i].interval      = FS_HID_GENERIC_INTERRUPT_IN_INTERVAL;
            }
            else
            {
                g_UsbDeviceHidGenericEndpoints[i].maxPacketSize = FS_HID_GENERIC_INTERRUPT_OUT_PACKET_SIZE;
                g_UsbDeviceHidGenericEndpoints[i].interval      = FS_HID_GENERIC_INTERRUPT_OUT_INTERVAL;
            }
        }
    }

    return kStatus_USB_Success;
}
