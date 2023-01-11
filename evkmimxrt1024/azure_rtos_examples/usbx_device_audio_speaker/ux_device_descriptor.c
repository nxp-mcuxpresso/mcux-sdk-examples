/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ux_device_descriptor.h"
#include "usb.h"

/* Vendor ID */
#define USB_DEVICE_VID (0x1FC9U)
/* Product ID */
#define USB_DEVICE_PID (0x00B3U)

/* High Speed */
#define USB_HS_BCD_VERSION (0x0200U)
#define USB_HS_CONTROL_MAX_PACKET_SIZE (64U)
#define USB_HS_CDC_ACM_BULK_PACKET_SIZE (512U)

/* Full Speed */
#define USB_FS_BCD_VERSION (0x0101U)
#define USB_FS_CONTROL_MAX_PACKET_SIZE (64U)
#define USB_FS_CDC_ACM_BULK_PACKET_SIZE (64U)

#define UX_DEMO_CFG_TOTAL_LENGTH        (146 + 7)
#define UX_DEMO_EP_ATTRIBUTES           (0x05)

#define D3(d) ((unsigned char)((d) >> 24))
#define D2(d) ((unsigned char)((d) >> 16))
#define D1(d) ((unsigned char)((d) >> 8))
#define D0(d) ((unsigned char)((d) >> 0))

#define WORD(w)     D0(w), D1(w)
#define DWORD(dw)   D0(dw), D1(dw), D2(dw), D3(dw)

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
unsigned char device_framework_full_speed[] = {
/* --------------------------------------- Device Descriptor */
/* 0  bLength, bDescriptorType                               */ 18,   0x01,
/* 2 bcdUSB                                                  */ D0(0x200),D1(0x200),
/* 4 bDeviceClass, bDeviceSubClass, bDeviceProtocol          */ 0x00,               0x00, 0x00,
/* 7 bMaxPacketSize0                                         */ 8,
/* 8  idVendor, idProduct                                    */ D0(USB_DEVICE_VID), D1(USB_DEVICE_VID), D0(USB_DEVICE_PID), D1(USB_DEVICE_PID),
/* 12 bcdDevice                                              */ D0(0x200),D1(0x200),
/* 14 iManufacturer, iProduct, iSerialNumber                 */ 1,    2,    3,
/* 17 bNumConfigurations                                     */ 1,

/* -------------------------------- Configuration Descriptor *//* 9+8+73+55=145 */
/* 0 bLength, bDescriptorType                                */ 9,    0x02,
/* 2 wTotalLength                                            */ WORD(UX_DEMO_CFG_TOTAL_LENGTH),
/* 4 bNumInterfaces, bConfigurationValue                     */ 2,    1,
/* 6 iConfiguration                                          */ 0,
/* 7 bmAttributes, bMaxPower                                 */ 0x80, 50,

/* ------------------------ Interface Association Descriptor */
/* 0 bLength, bDescriptorType                                */ 8,    0x0B,
/* 2 bFirstInterface, bInterfaceCount                        */ 0,    2,
/* 4 bFunctionClass, bFunctionSubClass, bFunctionProtocol    */ 0x01, 0x00, 0x20,
/* 7 iFunction                                               */ 0,

/* ------------------------------------ Interface Descriptor *//* 0 Control (9+64=73) */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 0,    0,
/* 4 bNumEndpoints                                           */ 0,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x01, 0x20,
/* 8 iInterface                                              */ 0,
/* ---------------- Audio 2.0 AC Interface Header Descriptor *//* (9+8+17+18+12=64) */
/* 0 bLength, bDescriptorType, bDescriptorSubtype            */ 9,                   0x24, 0x01,
/* 3 bcdADC, bCategory                                       */ D0(0x200),D1(0x200), 0x01,
/* 6 wTotalLength                                            */ D0(64),D1(64),
/* 8 bmControls                                              */ 0x00,
/* -------------------- Audio 2.0 AC Clock Source Descriptor */
/* 0 bLength, bDescriptorType, bDescriptorSubtype            */ 8,    0x24, 0x0A,
/* 3 bClockID, bmAttributes, bmControls                      */ 0x10, 0x05, 0x01,
/* 6 bAssocTerminal, iClockSource                            */ 0x00, 0,

/* ------------------- Audio 2.0 AC Input Terminal Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 17,   0x24,                   0x02,
/* 3  bTerminalID, wTerminalType                              */ 0x04, D0(0x0101),D1(0x0101),
/* 6  bAssocTerminal, bCSourceID                              */ 0x00, 0x10,
/* 8  bNrChannels, bmChannelConfig                            */ 0x02, D0(0),D1(0),D2(0),D3(0),
/* 13 iChannelNames, bmControls, iTerminal                    */ 0,    D0(0),D1(0),            0,

/* --------------------- Audio 2.0 AC Feature Unit Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 19,   0x24, 0x06,
/* 3  bUnitID, bSourceID bControlSize                         */ 0x05, 0x04, 0x04,
/* 5  bmaControls(0), bmaControls(...) ...                    */ D0(0x0),D1(0x0),D2(0x0),D3(0x0), D0(0),D1(0),D2(0),D3(0), D0(0),D1(0),D2(0),D3(0),
/* .  iFeature                                                */ 0,
/* ------------------ Audio 2.0 AC Output Terminal Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 12,          0x24,                 0x03,
/* 3  bTerminalID, wTerminalType                              */ 0x06,        D0(0x0301),D1(0x0301),
/* 6  bAssocTerminal, bSourceID, bCSourceID                   */ 0x00,        0x05,                 0x10,
/* 9  bmControls, iTerminal                                   */ D0(0),D1(0), 0,

/* ------------------------------------ Interface Descriptor *//* 2 Stream OUT (9+9+16+6+7+8=55) */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 1,    0,
/* 4 bNumEndpoints                                           */ 0,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x02, 0x20,
/* 8 iInterface                                              */ 0,
/* ------------------------------------ Interface Descriptor */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 1,    1,
/* 4 bNumEndpoints                                           */ 2,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x02, 0x20,
/* 8 iInterface                                              */ 0,
/* ------------------------ Audio 2.0 AS Interface Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 16,  0x24, 0x01,
/* 3  bTerminalLink, bmControls                               */ 0x04,0x00,
/* 5  bFormatType, bmFormats                                  */ 0x01,D0(1),D1(1),D2(1),D3(1),
/* 10 bNrChannels, bmChannelConfig                            */ 2,   D0(0),D1(0),D2(0),D3(0),
/* 15 iChannelNames                                           */ 0,
/* ---------------------- Audio 2.0 AS Format Type Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 6,    0x24, 0x02,
/* 3  bFormatType, bSubslotSize, bBitResolution               */ 0x01, 2,    16,
/* ------------------------------------- Endpoint Descriptor */
/* 0  bLength, bDescriptorType                                */ 7,               0x05,
/* 2  bEndpointAddress, bmAttributes                          */ 0x02,            UX_DEMO_EP_ATTRIBUTES,
/* 4  wMaxPacketSize, bInterval                               */ D0(256),D1(256), 1,
/* ---------- Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 8,    0x25,      0x01,
/* 3  bmAttributes, bmControls                                */ 0x00, 0x00,
/* 5  bLockDelayUnits, wLockDelay                             */ 0x00, D0(0),D1(0),
/* ------------------------------------- Endpoint Descriptor */
/* 0  bLength, bDescriptorType                                */ 7,               0x05,
/* 2  bEndpointAddress, bmAttributes                          */ 0x82,            0x11,
/* 4  wMaxPacketSize, bInterval                               */ WORD(4), 1,

};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
unsigned char device_framework_high_speed[] = {
/* --------------------------------------- Device Descriptor */
/* 0  bLength, bDescriptorType                               */ 18,   0x01,
/* 2  bcdUSB                                                 */ D0(USB_HS_BCD_VERSION), D1(USB_HS_BCD_VERSION),
/* 4  bDeviceClass, bDeviceSubClass, bDeviceProtocol         */ 0x00, 0x00, 0x00,
/* 7  bMaxPacketSize0                                        */ 64,
/* 8  idVendor, idProduct                                    */ D0(USB_DEVICE_VID), D1(USB_DEVICE_VID), D0(USB_DEVICE_PID), D1(USB_DEVICE_PID),
/* 12 bcdDevice                                              */ D0(0x200),D1(0x200),
/* 14 iManufacturer, iProduct, iSerialNumber                 */ 1,    2,    3,
/* 17 bNumConfigurations                                     */ 1,

/* ----------------------------- Device Qualifier Descriptor */
/* 0 bLength, bDescriptorType                                */ 10,                 0x06,
/* 2 bcdUSB                                                  */ D0(0x200),D1(0x200),
/* 4 bDeviceClass, bDeviceSubClass, bDeviceProtocol          */ 0x00,               0x00, 0x00,
/* 7 bMaxPacketSize0                                         */ 8,
/* 8 bNumConfigurations                                      */ 1,
/* 9 bReserved                                               */ 0,

/* -------------------------------- Configuration Descriptor *//* 9+8+73+55=145 */
/* 0 bLength, bDescriptorType                                */ 9,    0x02,
/* 2 wTotalLength                                            */ WORD(UX_DEMO_CFG_TOTAL_LENGTH),
/* 4 bNumInterfaces, bConfigurationValue                     */ 2,    1,
/* 6 iConfiguration                                          */ 0,
/* 7 bmAttributes, bMaxPower                                 */ 0x80, 50,

/* ------------------------ Interface Association Descriptor */
/* 0 bLength, bDescriptorType                                */ 8,    0x0B,
/* 2 bFirstInterface, bInterfaceCount                        */ 0,    2,
/* 4 bFunctionClass, bFunctionSubClass, bFunctionProtocol    */ 0x01, 0x00, 0x20,
/* 7 iFunction                                               */ 0,

/* ------------------------------------ Interface Descriptor *//* 0 Control (9+64=73) */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 0,    0,
/* 4 bNumEndpoints                                           */ 0,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x01, 0x20,
/* 8 iInterface                                              */ 0,
/* ---------------- Audio 2.0 AC Interface Header Descriptor *//* (9+8+17+18+12=64) */
/* 0 bLength, bDescriptorType, bDescriptorSubtype            */ 9,                   0x24, 0x01,
/* 3 bcdADC, bCategory                                       */ D0(0x200),D1(0x200), 0x01,
/* 6 wTotalLength                                            */ D0(64),D1(64),
/* 8 bmControls                                              */ 0x00,
/* -------------------- Audio 2.0 AC Clock Source Descriptor */
/* 0 bLength, bDescriptorType, bDescriptorSubtype            */ 8,    0x24, 0x0A,
/* 3 bClockID, bmAttributes, bmControls                      */ 0x10, 0x05, 0x01,
/* 6 bAssocTerminal, iClockSource                            */ 0x00, 0,
/* ------------------- Audio 2.0 AC Input Terminal Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 17,   0x24,                   0x02,
/* 3  bTerminalID, wTerminalType                              */ 0x04, D0(0x0101),D1(0x0101),
/* 6  bAssocTerminal, bCSourceID                              */ 0x00, 0x10,
/* 8  bNrChannels, bmChannelConfig                            */ 0x02, D0(3),D1(3),D2(3),D3(3),
/* 13 iChannelNames, bmControls, iTerminal                    */ 0,    D0(0),D1(0),            0,
/* --------------------- Audio 2.0 AC Feature Unit Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 19,   0x24, 0x06,
/* 3  bUnitID, bSourceID bControlSize                         */ 0x05, 0x04, 0x04,
/* 5  bmaControls(0), bmaControls(...) ...                    */ D0(0x0),D1(0x0),D2(0x0),D3(0x0), D0(0),D1(0),D2(0),D3(0), D0(0),D1(0),D2(0),D3(0),
/* .  iFeature                                                */ 0,
/* ------------------ Audio 2.0 AC Output Terminal Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 12,          0x24,                 0x03,
/* 3  bTerminalID, wTerminalType                              */ 0x06,        D0(0x0301),D1(0x0301),
/* 6  bAssocTerminal, bSourceID, bCSourceID                   */ 0x00,        0x05,                 0x10,
/* 9  bmControls, iTerminal                                   */ D0(0),D1(0), 0,

/* ------------------------------------ Interface Descriptor *//* 2 Stream OUT (9+9+16+6+7+8=55) */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 1,    0,
/* 4 bNumEndpoints                                           */ 0,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x02, 0x20,
/* 8 iInterface                                              */ 0,
/* ------------------------------------ Interface Descriptor */
/* 0 bLength, bDescriptorType                                */ 9,    0x04,
/* 2 bInterfaceNumber, bAlternateSetting                     */ 1,    1,
/* 4 bNumEndpoints                                           */ 2,
/* 5 bInterfaceClass, bInterfaceSubClass, bInterfaceProtocol */ 0x01, 0x02, 0x20,
/* 8 iInterface                                              */ 0,
/* ------------------------ Audio 2.0 AS Interface Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 16,  0x24, 0x01,
/* 3  bTerminalLink, bmControls                               */ 0x04,0x00,
/* 5  bFormatType, bmFormats                                  */ 0x01,D0(1),D1(1),D2(1),D3(1),
/* 10 bNrChannels, bmChannelConfig                            */ 2,   D0(3),D1(3),D2(3),D3(3),
/* 15 iChannelNames                                           */ 0,
/* ---------------------- Audio 2.0 AS Format Type Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 6,    0x24, 0x02,
/* 3  bFormatType, bSubslotSize, bBitResolution               */ 0x01, 2,    16,
/* ------------------------------------- Endpoint Descriptor */
/* 0  bLength, bDescriptorType                                */ 7,               0x05,
/* 2  bEndpointAddress, bmAttributes                          */ 0x02,            UX_DEMO_EP_ATTRIBUTES,
/* 4  wMaxPacketSize, bInterval                               */ D0(256),D1(256), 1,
/* ---------- Audio 2.0 AS ISO Audio Data Endpoint Descriptor */
/* 0  bLength, bDescriptorType, bDescriptorSubtype            */ 8,    0x25,      0x01,
/* 3  bmAttributes, bmControls                                */ 0x00, 0x00,
/* 5  bLockDelayUnits, wLockDelay                             */ 0x00, D0(0),D1(0),
/* ------------------------------------- Endpoint Descriptor */
/* 0  bLength, bDescriptorType                                */ 7,               0x05,
/* 2  bEndpointAddress, bmAttributes                          */ 0x82,            0x11,
/* 4  wMaxPacketSize, bInterval                               */ WORD(4), 4,
};

USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
unsigned char string_framework[] = {
    /* Manufacturer string descriptor : Index 1 */
    0x09, 0x04, 0x01, 18U,
    'N', 'X', 'P', ' ',
    'S', 'E', 'M', 'I', 'C', 'O', 'N', 'D', 'U', 'C', 'T', 'O', 'R', 'S',

    /* Product string descriptor : Index 2 */
    0x09, 0x04, 0x02, 14U,
    'U', 'S', 'B', ' ',
    'A', 'u', 'd', 'i', 'o', ' ', 'D', 'E', 'M', 'O',

    /* Serial Number string descriptor : Index 3 */
    0x09, 0x04, 0x03, 0x04, 0x30, 0x30, 0x30, 0x31
};

/* Multiple languages are supported on the device, to add
   a language besides English, the Unicode language code must
   be appended to the language_id_framework array and the length
   adjusted accordingly. */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
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
