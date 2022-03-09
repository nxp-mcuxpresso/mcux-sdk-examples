/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_MTP_FILE_SYSTEM_ADAPTER_H__
#define __USB_MTP_FILE_SYSTEM_ADAPTER_H__

#define USB_DEVICE_MTP_FILE_INSTANCE (4U)
#define USB_DEVICE_MTP_DIR_INSTANCE  (20U)

#define MTP_NAME_MAX_LEN (256U * 2U) /* The maximum length of file name */

#if FF_USE_LFN
#if (MTP_NAME_MAX_LEN <= (FF_LFN_BUF << 1U))
#warning "the configured name length is less than FF_LFN_BUF"
#endif
#endif

typedef void *usb_device_mtp_file_handle_t;
typedef void *usb_device_mtp_dir_handle_t;
typedef uint16_t usb_device_mtp_path_t;

/*! @brief MTP file access mode */
#define USB_DEVICE_MTP_READ          (0x01U)
#define USB_DEVICE_MTP_WRITE         (0x02U)
#define USB_DEVICE_MTP_CREATE_NEW    (0x04U)
#define USB_DEVICE_MTP_CREATE_ALWAYS (0x08U)

/*! @brief MTP file attribute */
#define USB_DEVICE_MTP_READ_ONLY (0x01U)
#define USB_DEVICE_MTP_DIR       (0x02U)

typedef union _usb_device_mtp_date_union
{
    uint32_t date;
    struct
    {
        uint16_t year;
        uint8_t month;
        uint8_t day;
    } dateBitField;
} usb_device_mtp_date_union_t;

typedef union _usb_device_mtp_time_union
{
    uint32_t time;
    struct
    {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t reserved;
    } timeBitField;
} usb_device_mtp_time_union_t;

typedef struct _usb_device_mtp_file_info
{
    uint32_t size;
    usb_device_mtp_date_union_t dateUnion;
    usb_device_mtp_time_union_t timeUnion;
    uint32_t attrib;                       /*!< please refer to the macro MTP file attribute */
    uint16_t name[MTP_NAME_MAX_LEN >> 1U]; /*!< 2-byte unicode */
} usb_device_mtp_file_info_t;

typedef struct _usb_device_mtp_file_time_stamp
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} usb_device_mtp_file_time_stamp_t;

usb_status_t USB_DeviceMtpOpen(usb_device_mtp_file_handle_t *file, const uint16_t *fileName, uint32_t flags);
usb_status_t USB_DeviceMtpClose(usb_device_mtp_file_handle_t file);
usb_status_t USB_DeviceMtpLseek(usb_device_mtp_file_handle_t file, uint32_t offset);
usb_status_t USB_DeviceMtpRead(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize);
usb_status_t USB_DeviceMtpWrite(usb_device_mtp_file_handle_t file, void *buffer, uint32_t size, uint32_t *actualsize);
usb_status_t USB_DeviceMtpFstat(const uint16_t *fileName, usb_device_mtp_file_info_t *fileInfo);
usb_status_t USB_DeviceMtpUtime(const uint16_t *path, usb_device_mtp_file_time_stamp_t *timeStamp);
usb_status_t USB_DeviceMtpOpenDir(usb_device_mtp_dir_handle_t *dir, const uint16_t *dirName);
usb_status_t USB_DeviceMtpCloseDir(usb_device_mtp_dir_handle_t dir);
usb_status_t USB_DeviceMtpReadDir(usb_device_mtp_dir_handle_t dir, usb_device_mtp_file_info_t *fileInfo);
usb_status_t USB_DeviceMtpMakeDir(const uint16_t *fileName);
usb_status_t USB_DeviceMtpUnlink(const uint16_t *fileName);
usb_status_t USB_DeviceMtpRename(const uint16_t *oldName, const uint16_t *newName);
usb_status_t USB_DeviceMtpGetDiskTotalBytes(const uint16_t *path, uint64_t *totalBytes);
usb_status_t USB_DeviceMtpGetDiskFreeBytes(const uint16_t *path, uint64_t *freeBytes);
usb_status_t USB_DeviceMtpFSInit(const uint16_t *path);

#endif /* __USB_MTP_FILE_SYSTEM_ADAPTER_H__ */
