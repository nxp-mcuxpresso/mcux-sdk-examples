/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_MTP_OBJECT_HANDLE_H__
#define __USB_MTP_OBJECT_HANDLE_H__

/*! @brief The structure is used to save object handle information. */
typedef struct _usb_mtp_obj_handle
{
    uint32_t handleID;
    union
    {
        uint32_t parentID;
        uint32_t nextDelHandleID;
    } idUnion;
    uint32_t storageID;
    usb_device_mtp_date_union_t dateUnion;
    usb_device_mtp_time_union_t timeUnion;
    uint64_t size;
    uint16_t name[MTP_NAME_MAX_LEN >> 1U];
    uint8_t flag;
} usb_mtp_obj_handle_t;

usb_status_t USB_DeviceMtpObjHandleInit(void);
usb_status_t USB_DeviceMtpObjHandleDeinit(void);
usb_status_t USB_DeviceMtpObjHandleRead(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct);
usb_status_t USB_DeviceMtpObjHandleWrite(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct);

#endif /* __USB_MTP_OBJECT_HANDLE_H__ */
