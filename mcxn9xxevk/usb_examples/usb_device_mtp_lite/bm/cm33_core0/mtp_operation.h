/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_MTP_OPERATION_H__
#define __USB_MTP_OPERATION_H__

#define MTP_STANDARD_VERSION    (100U) /* Represent 1.00 */
#define MTP_VENDOR_EXTENSION_ID (6UL)
#define MTP_VERSION             (100U) /* Represent 1.00 */

#define MTP_PATH_MAX_LEN (256U * 2U) /* The maximum length of path */

#define MTP_STORAGE_COUNT (1U) /* The number of storage */

#define MTP_DEVICE_FRIENDLY_NAME_LEN (8U * 2U) /* The length of device friendly name */

#define MTP_OBJECT_FILE        (1U << 0U)
#define MTP_OBJECT_DIR         (1U << 1U)
#define MTP_OBJECT_DELETE      (1U << 2U)
#define MTP_OBJECT_ALREADY_GET (1U << 7U)

typedef struct _usb_device_mtp_device_info
{
    const char *mtpExtendsions;
    const uint16_t *opSupported;
    const uint16_t *eventSupported;
    const uint16_t *devPropSupported;
    const uint16_t *captureFormat;
    const uint16_t *playbackFormat;
    const char *manufacturer;
    const char *model;
    const char *deviceVersion;
    const char *serialNumber;

    uint32_t opSupportedLength;
    uint32_t eventSupportedLength;
    uint32_t devPropSupportedLength;
    uint32_t captureFormatLength;
    uint32_t playbackFormatLength;

    uint16_t functionalMode;
} usb_device_mtp_device_info_t;

typedef union _usb_device_mtp_prop_type
{
    char *str;
    uint8_t u8;
    int8_t s8;
    uint16_t u16;
    int16_t s16;
    uint32_t u32;
    int32_t s32;
    uint64_t u64;
    int64_t s64;
    uint8_t *u128;
    uint8_t *s128;
} usb_device_mtp_prop_type_t;

typedef struct _usb_device_mtp_range_form
{
    usb_device_mtp_prop_type_t minVal;
    usb_device_mtp_prop_type_t maxVal;
    usb_device_mtp_prop_type_t stepSize;
} usb_device_mtp_range_form_t;

typedef struct _usb_device_mtp_enum_form
{
    uint16_t num;
    usb_device_mtp_prop_type_t *val;
} usb_device_mtp_enum_form_t;

typedef struct _usb_device_mtp_dev_prop_desc
{
    uint16_t devPropCode;
    uint16_t dataType;
    uint8_t getSet;
    usb_device_mtp_prop_type_t defaultVal;
    usb_device_mtp_prop_type_t currentVal;
    uint8_t formFlag;
    union
    {
        usb_device_mtp_range_form_t rangeForm;
        usb_device_mtp_enum_form_t enumForm;
    } form;
} usb_device_mtp_dev_prop_desc_t;

typedef struct _usb_device_mtp_dev_prop_desc_list
{
    usb_device_mtp_dev_prop_desc_t *devPropDesc;
    uint32_t devPropDescCount;
} usb_device_mtp_dev_prop_desc_list_t;

typedef struct _usb_device_mtp_obj_prop_desc
{
    uint16_t objPropCode;
    uint16_t dataType;
    uint8_t getSet;
    usb_device_mtp_prop_type_t defaultVal;
    uint32_t groupCode;
    uint8_t formFlag;
    union
    {
        usb_device_mtp_range_form_t rangeForm;
        usb_device_mtp_enum_form_t enumForm;
        uint32_t fixedLengthArray;
        char *regularExpression;
        uint32_t byteArray;
        uint32_t longString;
    } form;
} usb_device_mtp_obj_prop_desc_t;

typedef struct _usb_device_mtp_obj_prop
{
    uint16_t objFormat;
    uint16_t objPropDescCount;
    usb_device_mtp_obj_prop_desc_t *objPropDesc;
} usb_device_mtp_obj_prop_t;

typedef struct _usb_device_mtp_obj_prop_list
{
    usb_device_mtp_obj_prop_t *objProp;
    uint32_t objPropCount;
} usb_device_mtp_obj_prop_list_t;

typedef struct _usb_device_mtp_storage_info
{
    uint8_t *rootPath;
    char *storageDesc;
    char *volumeID;
    uint32_t storageID;
    uint16_t storageType;
    uint16_t fileSystemType;
    uint16_t accessCapability;
    uint8_t flag;
} usb_device_mtp_storage_info_t;

typedef struct _usb_device_mtp_storage_list
{
    usb_device_mtp_storage_info_t *storageInfo;
    uint32_t storageCount;
} usb_device_mtp_storage_list_t;

#if 0
typedef struct _usb_mtp_obj_handle_hdr
{
    /*uint32_t objHandle;     will be used */
    uint32_t delObjHandle; /* point to the first object handle deleted. */
    uint32_t nextHandleID;
} usb_mtp_obj_handle_hdr_t;

typedef struct _usb_device_mtp_storage_info
{
    uint16_t storageType;
    uint16_t fileSystemType;
    uint16_t accessCapability;
    uint64_t maxCapability;
    uint64_t freeSpaceInBytes;
    uint32_t freeSpaceInObjects;
    char *storageDesc;
    char *volumeID;
} usb_device_mtp_storage_info_t;

typedef struct _usb_mtp_obj_info
{
    uint32_t storageID;
    uint32_t objFormat;
    uint16_t protectStatus;
    uint32_t objSize;
    uint16_t thmbFormat;
    uint32_t thmbSize;
    uint32_t thmbPixWidth;
    uint32_t thmbPixHeight;
    uint32_t imagePixWidth;
    uint32_t imagePixHeight;
    uint32_t imageBitDepth;
    uint32_t parentObj;
    uint16_t assoType;
    uint32_t assoDesc;
    uint32_t seqNumber;
    const char *fileName;
    const char *dateCreated;
    const char *dateModified;
    const char *keywords;
} usb_mtp_obj_info_t;

typedef struct _usb_mtp_file_info
{
    uint32_t parentID;
    uint32_t storageID;
    uint32_t date;
    uint16_t *name;
    uint64_t size;
    uint8_t flag;
} usb_mtp_file_info_t;
#endif

void USB_DeviceCmdOpenSession(void *param);
void USB_DeviceCmdCloseSession(void *param);
void USB_DeviceCmdGetDeviceInfo(void *param, usb_device_mtp_device_info_t *deviceInfo);
void USB_DeviceCmdGetDevicePropDesc(void *param);
void USB_DeviceCmdGetObjPropsSupported(void *param);
void USB_DeviceCmdGetStorageIDs(void *param);
void USB_DeviceCmdGetStorageInfo(void *param);
void USB_DeviceCmdGetObjHandles(void *param);
void USB_DeviceCmdGetObjPropDesc(void *param);
void USB_DeviceCmdGetObjPropList(void *param);
void USB_DeviceCmdGetObjInfo(void *param);
void USB_DeviceCmdGetObj(void *param);
void USB_DeviceCmdSendObjInfo(void *param);
void USB_DeviceCmdSendObj(void *param);
void USB_DeviceCmdDeleteObj(void *param);
void USB_DeviceCmdGetDevicePropVal(void *param);
void USB_DeviceCmdSetDevicePropVal(void *param);
void USB_DeviceCmdGetObjPropVal(void *param);
void USB_DeviceCmdSetObjPropVal(void *param);
void USB_DeviceCmdGetObjReferences(void *param);
void USB_DeviceCmdMoveObj(void *param);
void USB_DeviceCmdCopyObj(void *param);

#endif
