/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_MTP_H__
#define __USB_MTP_H__

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

#if defined(__GIC_PRIO_BITS)
#define USB_DEVICE_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_DEVICE_INTERRUPT_PRIORITY (6U)
#else
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#endif

#define USB_DEVICE_MTP_MSG_QUEUE_COUNT (1U)

/* USB MTP config*/
/*buffer size for mtp example. the larger the buffer size ,the faster the data transfer speed is ,*/
/*the block size should be multiple of 512, the least value is 1024*/

#define USB_DEVICE_MTP_TRANSFER_BUFF_SIZE (512 * 9U)


typedef struct _usb_mtp_struct
{
    usb_device_handle deviceHandle;
    class_handle_t mtpHandle;
    usb_device_mtp_dev_prop_desc_list_t *devPropDescList;
    usb_device_mtp_storage_list_t *storageList;
    usb_device_mtp_obj_prop_list_t *objPropList;
    uint32_t nextHandleID;
    uint8_t *path;
    uint8_t *devFriendlyName;
    uint64_t transferDoneSize;
    uint64_t transferTotalSize;
    usb_device_mtp_file_handle_t file;          /* file handle is used when receiving or sending a file. */
    usb_device_mtp_file_time_stamp_t timeStamp; /* timeStamp is used when receiving a file. */
    uint16_t functionalMode;
    volatile uint8_t mutexUsbToDiskTask;
    uint8_t validObjInfo;
    uint8_t read_write_error;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_MTP_INTERFACE_COUNT];
    uint8_t speed;
    uint8_t attach;
} usb_mtp_struct_t;

#endif
