/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DISK_H__
#define __USB_DISK_H__ 1

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
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
/* Length of Each Logical Address Block */
#define LENGTH_OF_EACH_LBA (512U)
/* total number of logical blocks present */
#define TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL (48U)
/* Net Disk Size , default disk is 48*512, that is 24kByte, however , the disk recognized by that PC only has 4k Byte,
 * This is caused by that the file system also need memory*/
#define DISK_SIZE_NORMAL (TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL * LENGTH_OF_EACH_LBA)

#define LOGICAL_UNIT_SUPPORTED (1U)

typedef struct _usb_msc_struct
{
    usb_device_handle deviceHandle;
    usb_device_msc_struct_t mscStruct;
    uint8_t *storageDisk;

    uint8_t diskLock;
    uint8_t read_write_error;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_MSC_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
    uint8_t stop; /* indicates this media keeps stop or not, 1: stop, 0: start */
} usb_msc_struct_t;

#endif
