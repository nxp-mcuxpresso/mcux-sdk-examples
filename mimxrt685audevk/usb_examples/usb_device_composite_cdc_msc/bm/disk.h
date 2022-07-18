/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_MSC_DISK_H_
#define _USB_MSC_DISK_H_ 1

/*******************************************************************************
 * Definitions
 ******************************************************************************/

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

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
/* Length of Each Logical Address Block */
#define LENGTH_OF_EACH_LBA (512)
/* total number of logical blocks present */
#define TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL (48)
/* Net Disk Size , default disk is 48*512, that is 24kByte, however , the disk recognized by that PC only has 4k Byte,
 * This is caused by that the file system also need memory*/
#define DISK_SIZE_NORMAL (TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL * LENGTH_OF_EACH_LBA)
/* application define logical unit number, if LOGICAL_UNIT_SUPPORTED > USB_DEVICE_MSC_MAX_LUN, update
 * USB_DEVICE_MSC_MAX_LUN in class driver usb_device_msc.h*/
#define LOGICAL_UNIT_SUPPORTED (1)

typedef struct _usb_msc_struct
{
    usb_device_handle deviceHandle;
    class_handle_t mscHandle;
    uint8_t *storageDisk;
    uint8_t diskLock;
    uint8_t readWriteError;
    uint8_t currentConfiguration;
    uint8_t speed;
    uint8_t attach;
    uint8_t stop; /* indicates this media keeps stop or not, 1: stop, 0: start */
} usb_msc_struct_t;

#endif /* _USB_MSC_DISK_H_ */
