/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DEVICE_PRINTER_H__
#define __DEVICE_PRINTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#define USB_PRINTER_BUFFER_SIZE                                                                            \
    (HS_PRINTER_BULK_OUT_PACKET_SIZE > FS_PRINTER_BULK_OUT_PACKET_SIZE ? HS_PRINTER_BULK_OUT_PACKET_SIZE : \
                                                                         FS_PRINTER_BULK_OUT_PACKET_SIZE)

typedef enum _usb_device_printer_state
{
    kPrinter_Idle = 0x00,
    kPrinter_ReceiveNeedPrime,
    kPrinter_Receiving,
    kPrinter_Received,
} usb_device_printer_buffer_t;

typedef struct _usb_device_printer_app
{
    usb_device_handle deviceHandle;

    uint32_t dataReceiveLength;
    /*!< buffer for send, still NULL in this demo.
    if user's application parse the received data and need send back the status information,
    user need set the data to this buffer and length*/
    uint8_t *sendBuffer;
    uint32_t sendLength;
    uint8_t *printerBuffer;
    volatile uint8_t printerState;
    volatile uint8_t stateChanged;
    volatile uint8_t prnterTaskState;
    uint8_t attach;
    uint8_t speed;
    uint8_t printerPortStatus;
} usb_device_printer_app_t;

/*******************************************************************************
 * API
 ******************************************************************************/

#endif
