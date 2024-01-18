/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ux_device_descriptor.h"

/* Vendor ID */
#define USB_DEVICE_VID (0x1FC9U)
/* Product ID */
#define USB_DEVICE_PID (0x00B1U)

/* High Speed */
#define USB_HS_BCD_VERSION (0x0200U)
#define USB_HS_CONTROL_MAX_PACKET_SIZE (64U)
#define USB_HS_CDC_ACM_BULK_PACKET_SIZE (512U)

/* Full Speed */
#define USB_FS_BCD_VERSION (0x0101U)
#define USB_FS_CONTROL_MAX_PACKET_SIZE (64U)
#define USB_FS_CDC_ACM_BULK_PACKET_SIZE (64U)

#define D3(d) ((unsigned char)((d) >> 24))
#define D2(d) ((unsigned char)((d) >> 16))
#define D1(d) ((unsigned char)((d) >> 8))
#define D0(d) ((unsigned char)((d) >> 0))

unsigned char device_framework_high_speed[] = {
    /* Device Descriptor */
    0x12, /* bLength */
    0x01, /* bDescriptorType */
    D0(USB_HS_BCD_VERSION), /* bcdUSB lo */
    D1(USB_HS_BCD_VERSION), /* bcdUSB hi */
    0x00, /* DeviceClass */
    0x00, /* DeviceSubClass */
    0x00, /* DeviceProtocol */
    USB_HS_CONTROL_MAX_PACKET_SIZE, /* bMaxPacketSize */
    D0(USB_DEVICE_VID), /* idVendor lo */
    D1(USB_DEVICE_VID), /* idVendor hi */
    D0(USB_DEVICE_PID), /* idProduct lo */
    D0(USB_DEVICE_PID), /* idProduct hi */
    0x00, /* bcdDevice lo */
    0x01, /* bcdDevice hi */
    0x01, /* iManufacturer*/
    0x02, /* iProduct */
    0x03, /* iSerialNumber */
    0x01, /* bNumConfigurations */

    /* Device Qualifier Descriptor */
    0x0A, /* bLength */
    0x06, /* bDescriptorType */
    D0(USB_HS_BCD_VERSION), /* bcdUSB lo */
    D1(USB_HS_BCD_VERSION), /* bcdUSB hi */
    0x00, /* DeviceClass */
    0x00, /* DeviceSubClass */
    0x00, /* DeviceProtocol */
    USB_HS_CONTROL_MAX_PACKET_SIZE, /* bMaxPacketSize */
    0x01, /* bNumConfigurations */
    0x00, /* bReserved */

    /* Configuration Descriptor */
    /* 9+8+120+55*2=247 */
    0x09, /* bLength */
    0x02, /* bDescriptorType */
    D0(247), /* wTotalLength lo */
    D1(247), /* wTotalLength hi */
    0x03, /* bNumInterfaces */
    0x01, /* bConfigurationValue */
    0x00 /* iConfiguration */,
    0x80, /* bmAttributes */
    0x32, /* bMaxPower 0x32 * 2ma */

    /* Interface Association Descriptor (IAD) */
    0x08, /* bLength */
    0x0B, /* bDescriptorType */
    0x00, /* bFirstInterface */
    0x03, /* bInterfaceCount */
    0x01, /* bFunctionClass  */
    0x00, /* bFunctionSubClass */
    0x20, /* bFunctionProtocol */
    0x00, /* iFunction */

    /* Interface Descriptor */
    /* 0 Control (9+111=120) */
    0x09, /* bLength */
    0x04, /* bDescriptorType */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x01, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AC Interface Header Descriptor */
    /* (9+8+17*2+18*2+12*2=111) */
    0x09, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x00, /* bcdCDC_lo */
    0x02, /* bcdCDC_hi */
    0x08, /* bCategory */
    D0(111), /* wTotalLength_lo */
    D1(111), /* wTotalLength_hi */
    0x00, /* bmControls */

    /* Audio 2.0 AC Clock Source Descriptor */
    0x08, /* bLength */
    0x24, /* bDescriptorType */
    0x0A, /* bDescriptorSubtype */
    0x10, /* bClockID */
    0x05, /* bmAttributes */
    0x01, /* bmControls */
    0x00, /* bAssocTerminal */
    0x00, /* iClockSource */

    /* Audio 2.0 AC Input Terminal Descriptor  */
    0x11, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x02, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x10, /* bCSourceID */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Feature Unit Descriptor */
    0x12, /* bLength */
    0x24, /* bDescriptorType */
    0x06, /* bDescriptorSubtype */
    0x02, /* bUnitID */
    0x01, /* bSourceID */
    0x0F, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* iFeature */

    /* Audio 2.0 AC Output Terminal Descriptor */
    0x0C, /* bLength */
    0x24, /* bDescriptorType */
    0x03, /* bDescriptorSubtype */
    0x03, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x01, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x02, /* bSourceID */
    0x10, /* bCSourceID */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Input Terminal Descriptor  */
    0x11, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x04, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x01, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x10, /* bCSourceID */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Feature Unit Descriptor */
    0x12, /* bLength */
    0x24, /* bDescriptorType */
    0x06, /* bDescriptorSubtype */
    0x05, /* bUnitID */
    0x04, /* bSourceID */
    0x0F, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* iFeature */

    /* Audio 2.0 AC Output Terminal Descriptor */
    0x0C, /* bLength */
    0x24, /* bDescriptorType */
    0x03, /* bDescriptorSubtype */
    0x06, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x03, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x05, /* bSourceID */
    0x10, /* bCSourceID */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Interface Descriptor */
    /* 1 Stream IN (9+9+16+6+7+8=55) */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x01, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Interface Descriptor */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x01, /* bInterfaceNumber */
    0x01, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AS Interface Descriptor */
    0x10, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x03, /* bTerminalLink */
    0x00, /* bmControls */
    0x01, /* bFormatType */
    D0(1), /* bmFormats */
    D1(1), /* bmFormats */
    D2(1), /* bmFormats */
    D3(1), /* bmFormats */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */

    /*Audio 2.0 AS Format Type I Descriptor */
    0x06, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bFormatType */
    0x02, /* bSubslotSize */
    0x10, /* bFormatType */

    /* Endpoint descriptor */
    0x07, /* bLength */
    0x05, /* bDescriptorType */
    0x81, /* bEndpointAddress */
    0x0D, /* bmAttribute */
    D0(256), /* wMAXPacketSize_lo */
    D1(256), /* wMAXPacketSize_hi */
    0x04, /* bInterval */

    /* Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
    0x08, /* bLength */
    0x25, /* bDescriptorType */
    0x01, /* bEndpointAddress */
    0x00, /* bmAttribute */
    0x00, /* bmControls */
    0x00, /* bLockDelayUnits */
    D0(0), /* wLockDelay_lo */
    D1(0), /* wLockDelaye_hi */

    /* Interface Descriptor */
    /* 2 Stream OUT (9+9+16+6+7+8=55) */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x02, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Interface Descriptor */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x02, /* bInterfaceNumber */
    0x01, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AS Interface Descriptor */
    0x10, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x04, /* bTerminalLink */
    0x00, /* bmControls */
    0x01, /* bFormatType */
    D0(1), /* bmFormats */
    D1(1), /* bmFormats */
    D2(1), /* bmFormats */
    D3(1), /* bmFormats */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */

    /*Audio 2.0 AS Format Type Descriptor */
    0x06, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bFormatType */
    0x02, /* bSubslotSize */
    0x10, /* bFormatType */

    /* Endpoint descriptor */
    0x07, /* bLength */
    0x05, /* bDescriptorType */
    0x02, /* bEndpointAddress */
    0x0D, /* bmAttribute */
    D0(256), /* wMAXPacketSize_lo */
    D1(256), /* wMAXPacketSize_hi */
    0x04, /* bInterval */

    /* Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
    0x08, /* bLength */
    0x25, /* bDescriptorType */
    0x01, /* bEndpointAddress */
    0x00, /* bmAttribute */
    0x00, /* bmControls */
    0x00, /* bLockDelayUnits */
    D0(0), /* wLockDelay_lo */
    D1(0), /* wLockDelaye_hi */
};

/* FS Device Descriptor */
unsigned char device_framework_full_speed[] = {
    /* Device Descriptor */
    0x12, /* bLength */
    0x01, /* bDescriptorType */
    D0(USB_FS_BCD_VERSION), /* bcdUSB lo */
    D1(USB_FS_BCD_VERSION), /* bcdUSB hi */
    0x00, /* DeviceClass */
    0x00, /* DeviceSubClass */
    0x00, /* DeviceProtocol */
    USB_FS_CONTROL_MAX_PACKET_SIZE, /* bMaxPacketSize */
    D0(USB_DEVICE_VID), /* idVendor lo */
    D1(USB_DEVICE_VID), /* idVendor hi */
    D0(USB_DEVICE_PID), /* idProduct lo */
    D0(USB_DEVICE_PID), /* idProduct hi */
    0x00, /* bcdDevice lo */
    0x01, /* bcdDevice hi */
    0x01, /* iManufacturer*/
    0x02, /* iProduct */
    0x03, /* iSerialNumber */
    0x01, /* bNumConfigurations */

    /* Device Qualifier Descriptor */
    0x0A, /* bLength */
    0x06, /* bDescriptorType */
    D0(USB_FS_BCD_VERSION), /* bcdUSB lo */
    D1(USB_FS_BCD_VERSION), /* bcdUSB hi */
    0x00, /* DeviceClass */
    0x00, /* DeviceSubClass */
    0x00, /* DeviceProtocol */
    USB_FS_CONTROL_MAX_PACKET_SIZE, /* bMaxPacketSize 8 */
    0x01, /* bNumConfigurations */
    0x00, /* bReserved */

    /* Configuration Descriptor */
    /* 9+8+120+55*2=247 */
    0x09, /* bLength */
    0x02, /* bDescriptorType */
    D0(247), /* wTotalLength lo */
    D1(247), /* wTotalLength hi */
    0x03, /* bNumInterfaces */
    0x01, /* bConfigurationValue */
    0x00 /* iConfiguration */,
    0x80, /* bmAttributes */
    0x32, /* bMaxPower 0x32 * 2ma */

    /* Interface Association Descriptor (IAD) */
    0x08, /* bLength */
    0x0B, /* bDescriptorType */
    0x00, /* bFirstInterface */
    0x03, /* bInterfaceCount */
    0x01, /* bFunctionClass  */
    0x00, /* bFunctionSubClass */
    0x20, /* bFunctionProtocol */
    0x00, /* iFunction */

    /* Interface Descriptor */
    /* 0 Control (9+111=120) */
    0x09, /* bLength */
    0x04, /* bDescriptorType */
    0x00, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x01, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AC Interface Header Descriptor */
    /* (9+8+17*2+18*2+12*2=111) */
    0x09, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x00, /* bcdCDC_lo */
    0x02, /* bcdCDC_hi */
    0x08, /* bCategory */
    D0(111), /* wTotalLength_lo */
    D1(111), /* wTotalLength_hi */
    0x00, /* bmControls */

    /* Audio 2.0 AC Clock Source Descriptor */
    0x08, /* bLength */
    0x24, /* bDescriptorType */
    0x0A, /* bDescriptorSubtype */
    0x10, /* bClockID */
    0x05, /* bmAttributes */
    0x01, /* bmControls */
    0x00, /* bAssocTerminal */
    0x00, /* iClockSource */

    /* Audio 2.0 AC Input Terminal Descriptor  */
    0x11, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x02, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x10, /* bCSourceID */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Feature Unit Descriptor */
    0x12, /* bLength */
    0x24, /* bDescriptorType */
    0x06, /* bDescriptorSubtype */
    0x02, /* bUnitID */
    0x01, /* bSourceID */
    0x0F, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* iFeature */

    /* Audio 2.0 AC Output Terminal Descriptor */
    0x0C, /* bLength */
    0x24, /* bDescriptorType */
    0x03, /* bDescriptorSubtype */
    0x03, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x01, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x02, /* bSourceID */
    0x10, /* bCSourceID */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Input Terminal Descriptor  */
    0x11, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x04, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x01, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x10, /* bCSourceID */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Audio 2.0 AC Feature Unit Descriptor */
    0x12, /* bLength */
    0x24, /* bDescriptorType */
    0x06, /* bDescriptorSubtype */
    0x05, /* bUnitID */
    0x04, /* bSourceID */
    0x0F, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls0 for master channel 0 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls1 for logical channel 1 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* bmaControls2 for logical channel 2 */
    0x00, /* iFeature */

    /* Audio 2.0 AC Output Terminal Descriptor */
    0x0C, /* bLength */
    0x24, /* bDescriptorType */
    0x03, /* bDescriptorSubtype */
    0x06, /* bTerminalID */
    0x01, /* wTerminalType_lo */
    0x03, /* wTerminalType_hi */
    0x00, /* bAssocTerminal */
    0x05, /* bSourceID */
    0x10, /* bCSourceID */
    0x00, /* bmControls_lo */
    0x00, /* bmControls_hi */
    0x00, /* iTerminal */

    /* Interface Descriptor */
    /* 1 Stream IN (9+9+16+6+7+8=55) */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x01, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Interface Descriptor */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x01, /* bInterfaceNumber */
    0x01, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AS Interface Descriptor */
    0x10, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x03, /* bTerminalLink */
    0x00, /* bmControls */
    0x01, /* bFormatType */
    D0(1), /* bmFormats */
    D1(1), /* bmFormats */
    D2(1), /* bmFormats */
    D3(1), /* bmFormats */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */

    /*Audio 2.0 AS Format Type I Descriptor */
    0x06, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bFormatType */
    0x02, /* bSubslotSize */
    0x10, /* bFormatType */

    /* Endpoint descriptor */
    0x07, /* bLength */
    0x05, /* bDescriptorType */
    0x81, /* bEndpointAddress */
    0x0D, /* bmAttribute */
    D0(256), /* wMAXPacketSize_lo */
    D1(256), /* wMAXPacketSize_hi */
    0x01, /* bInterval */

    /* Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
    0x08, /* bLength */
    0x25, /* bDescriptorType */
    0x01, /* bEndpointAddress */
    0x00, /* bmAttribute */
    0x00, /* bmControls */
    0x00, /* bLockDelayUnits */
    D0(0), /* wLockDelay_lo */
    D1(0), /* wLockDelaye_hi */

    /* Interface Descriptor */
    /* 2 Stream OUT (9+9+16+6+7+8=55) */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x02, /* bInterfaceNumber */
    0x00, /* bAlternateSetting */
    0x00, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Interface Descriptor */
    0x09, /* bLength */
    0x04, /* bDescriptor */
    0x02, /* bInterfaceNumber */
    0x01, /* bAlternateSetting */
    0x01, /* bNumEndpoints */
    0x01, /* bInterfaceClass */
    0x02, /* bInterfaceSubClass */
    0x20, /* bInterfaceProtocol */
    0x00, /* iInterface */

    /* Audio 2.0 AS Interface Descriptor */
    0x10, /* bLength */
    0x24, /* bDescriptorType */
    0x01, /* bDescriptorSubtype */
    0x04, /* bTerminalLink */
    0x00, /* bmControls */
    0x01, /* bFormatType */
    D0(1), /* bmFormats */
    D1(1), /* bmFormats */
    D2(1), /* bmFormats */
    D3(1), /* bmFormats */
    0x02, /* bNrChannels */
    D0(0), /* bmChannelConfig */
    D1(0), /* bmChannelConfig */
    D2(0), /* bmChannelConfig */
    D3(0), /* bmChannelConfig */
    0x00, /* iChannelNames */

    /*Audio 2.0 AS Format Type Descriptor */
    0x06, /* bLength */
    0x24, /* bDescriptorType */
    0x02, /* bDescriptorSubtype */
    0x01, /* bFormatType */
    0x02, /* bSubslotSize */
    0x10, /* bFormatType */

    /* Endpoint descriptor */
    0x07, /* bLength */
    0x05, /* bDescriptorType */
    0x02, /* bEndpointAddress */
    0x0D, /* bmAttribute */
    D0(256), /* wMAXPacketSize_lo */
    D1(256), /* wMAXPacketSize_hi */
    0x01, /* bInterval */

    /* Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
    0x08, /* bLength */
    0x25, /* bDescriptorType */
    0x01, /* bEndpointAddress */
    0x00, /* bmAttribute */
    0x00, /* bmControls */
    0x00, /* bLockDelayUnits */
    D0(0), /* wLockDelay_lo */
    D1(0), /* wLockDelaye_hi */
};

unsigned char string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 18U,
    'N', 'X', 'P', ' ',
    'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R', 'S',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 14U,
    'U', 'S', 'B', ' ',
    'A', 'U', 'D', 'I', 'O', ' ', 'D', 'E', 'M', 'O',

    /* Serial Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x04, 0x30, 0x30, 0x30, 0x31
};

/* Multiple languages are supported on the device, to add
   a language besides English, the Unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */
unsigned char language_id_framework[] = {
    /* English. */
    0x09, 0x04
};

unsigned char * ux_get_hs_framework(void)
{
    return device_framework_high_speed;
}

unsigned long ux_get_hs_framework_length(void)
{
    return sizeof(device_framework_high_speed);
}

unsigned char * ux_get_fs_framework(void)
{
    return device_framework_full_speed;
}

unsigned long ux_get_fs_framework_length()
{
    return sizeof(device_framework_full_speed);
}

unsigned char * ux_get_string_framework(void)
{
    return string_framework;
}

unsigned long ux_get_string_framework_length(void)
{
    return sizeof(string_framework);
}

unsigned char * ux_get_language_framework(void)
{
    return language_id_framework;
}

unsigned long ux_get_language_framework_length(void)
{
    return sizeof(language_id_framework);
}
