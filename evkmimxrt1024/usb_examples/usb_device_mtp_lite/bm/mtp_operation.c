/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_mtp.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "mtp_file_system_adapter.h"
#include "mtp_object_handle.h"
#include "mtp_operation.h"
#include "mtp.h"

#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MTP_DATE_TIME_LEN (8U * 2U) /* The length of datetime */
#define MTP_BLOCK_SIZE    (512U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

#if USB_DEVICE_CONFIG_USE_TASK
extern void USB_DeviceTaskFn(void *deviceHandle);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/

extern usb_mtp_struct_t g_mtp;
extern uint32_t g_mtpTransferBuffer[USB_DEVICE_MTP_TRANSFER_BUFF_SIZE >> 2];

/*******************************************************************************
 * Code
 ******************************************************************************/

uint32_t USB_DeviceMemCopyArray(void *desBuf, const void *srcBuf, uint32_t size, uint32_t elementSize, uint32_t index)
{
    uint8_t *buffer     = (uint8_t *)desBuf;
    uint32_t elementCnt = size / elementSize;

    buffer += index;

    *buffer++ = (uint8_t)elementCnt;
    *buffer++ = (uint8_t)(elementCnt >> 8U);
    *buffer++ = (uint8_t)(elementCnt >> 16U);
    *buffer++ = (uint8_t)(elementCnt >> 24U);
    index += 4U;

    (void)memcpy((void *)buffer, (void *)srcBuf, size);

    return (index + size);
}

/* ascll to unicode */
uint32_t USB_DeviceMemCopyString(void *desBuf, const char *srcBuf, uint32_t index)
{
    uint8_t *buffer = (uint8_t *)desBuf;
    uint32_t i;

    if (srcBuf == NULL)
    {
        buffer += index;
        buffer[0] = 0x00U;

        return (index + 1U);
    }
    else
    {
        i = 0;

        index += 1U;
        buffer += index;

        while (srcBuf[i] != 0x00U)
        {
            buffer[(i << 1U)]      = srcBuf[i];
            buffer[(i << 1U) + 1U] = 0x00U;
            i++;
        }

        buffer[(i << 1U)]      = 0x00U;
        buffer[(i << 1U) + 1U] = 0x00U; /* terminate with 0x00, 0x00 */

        i += 1;
        *(buffer - 1U) = i; /* the number of char */

        return (index + (i << 1U));
    }
}

uint32_t USB_DeviceMemCopyByte(void *desBuf, uint8_t byte, uint32_t index)
{
    uint8_t *buffer = (uint8_t *)desBuf;

    buffer += index;

    buffer[0] = byte;

    return (index + 1U);
}

uint32_t USB_DeviceMemCopyWord(void *desBuf, uint16_t word, uint32_t index)
{
    uint8_t *buffer = (uint8_t *)desBuf;

    buffer += index;

    buffer[0] = (uint8_t)word;
    buffer[1] = (uint8_t)(word >> 8U);

    return (index + 2U);
}

uint32_t USB_DeviceMemCopyDword(void *desBuf, uint32_t dword, uint32_t index)
{
    uint8_t *buffer = (uint8_t *)desBuf;

    buffer += index;

    buffer[0] = (uint8_t)dword;
    buffer[1] = (uint8_t)(dword >> 8U);
    buffer[2] = (uint8_t)(dword >> 16U);
    buffer[3] = (uint8_t)(dword >> 24U);

    return (index + 4U);
}

uint32_t USB_DeviceMemCopyQword(void *desBuf, uint64_t dword, uint32_t index)
{
    uint8_t *buffer = (uint8_t *)desBuf;

    buffer += index;

    (void)memcpy((void *)buffer, (void *)&dword, 8U);

    return (index + 8U);
}

uint32_t USB_DeviceMemCopyUnicodeString(void *desBuf, const uint16_t *srcBuf, uint32_t index)
{
    uint8_t *buffer    = (uint8_t *)desBuf;
    const uint8_t *src = (const uint8_t *)srcBuf;
    uint32_t count;

    if (srcBuf == NULL)
    {
        if (index != 0U)
        {
            buffer += index;
            buffer[0] = 0x00U;

            return (index + 1U);
        }

        return 0;
    }
    else
    {
        count = 0U;

        if (index != 0U)
        {
            index += 1U;
        }

        buffer += index;

        while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
        {
            buffer[count] = src[count];
            count++;
        }

        buffer[count]      = 0U;
        buffer[count + 1U] = 0U; /* terminate with 0x00, 0x00 */

        count += 2;

        if (index != 0U)
        {
            *(buffer - 1U) = count >> 1U; /* the number of char */
        }

        return (count + index);
    }
}

uint32_t USB_DeviceUnicodeStringLength(const uint16_t *srcBuf)
{
    const uint8_t *src = (const uint8_t *)srcBuf;
    uint32_t count     = 0;

    if (srcBuf == NULL)
    {
        return 0;
    }

    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
    {
        count++;
    }

    return count;
}

static uint32_t USB_DeviceParseDataType(uint16_t dataType, void *buffer, void *val, uint32_t offset)
{
    uint32_t index;

    switch (dataType)
    {
        case MTP_TYPE_STR:
            offset = USB_DeviceMemCopyString(buffer, val, offset);
            break;

        case MTP_TYPE_INT8:
        case MTP_TYPE_UINT8:
            offset = USB_DeviceMemCopyByte(buffer, *(uint8_t *)val, offset);
            break;

        case MTP_TYPE_INT16:
        case MTP_TYPE_UINT16:
            offset = USB_DeviceMemCopyWord(buffer, *(uint16_t *)val, offset);
            break;

        case MTP_TYPE_INT32:
        case MTP_TYPE_UINT32:
            offset = USB_DeviceMemCopyDword(buffer, *(uint32_t *)val, offset);
            break;

        case MTP_TYPE_INT64:
        case MTP_TYPE_UINT64:
            offset = USB_DeviceMemCopyQword(buffer, *(uint64_t *)val, offset);
            break;

        case MTP_TYPE_INT128:
        case MTP_TYPE_UINT128:
            /* Only property of Persistent UID is UINT128 type.
               The default value is 0x0...0, so there is a special process*/
            if (val == NULL)
            {
                offset = USB_DeviceMemCopyQword(buffer, 0x00U, offset);
                offset = USB_DeviceMemCopyQword(buffer, 0x00U, offset);
            }
            else
            {
                offset = USB_DeviceMemCopyQword(buffer, *(uint64_t *)val, offset);
                offset = USB_DeviceMemCopyQword(buffer, *((uint64_t *)val + 1U), offset);
            }
            break;

        case MTP_TYPE_AINT8:
        case MTP_TYPE_AUINT8:
            /* For array, the default value in the property description is only 0x00U. */
            offset = USB_DeviceMemCopyDword(buffer, *(uint32_t *)val, offset);
            for (index = 0; index < *(uint32_t *)val; index++)
            {
                offset = USB_DeviceMemCopyByte(buffer, ((uint8_t *)val)[index + 4U], offset);
            }
            break;

        case MTP_TYPE_AINT16:
        case MTP_TYPE_AUINT16:
            /* For array, the default value in the property description is only 0x00U. */
            offset = USB_DeviceMemCopyDword(buffer, *(uint32_t *)val, offset);
            for (index = 0; index < *(uint32_t *)val; index++)
            {
                offset = USB_DeviceMemCopyWord(buffer, ((uint16_t *)val)[index + 2U], offset);
            }
            break;

        default:
            /* no action */
            break;
    }

    return offset;
}

static void USB_DeviceParseDateTime(const uint16_t *dateTime, usb_device_mtp_file_time_stamp_t *timeStamp)
{
    const uint8_t *src = (const uint8_t *)dateTime;

    memset((void *)timeStamp, 0U, sizeof(usb_device_mtp_file_time_stamp_t));

    /* dateTime is unicode string. */
    timeStamp->year += (*src - '0') * 1000U;
    src += 2U;
    timeStamp->year += (*src - '0') * 100U;
    src += 2U;
    timeStamp->year += (*src - '0') * 10U;
    src += 2U;
    timeStamp->year += (*src - '0');
    src += 2U;
    timeStamp->month += (*src - '0') * 10U;
    src += 2U;
    timeStamp->month += (*src - '0');
    src += 2U;
    timeStamp->day += (*src - '0') * 10U;
    src += 2U;
    timeStamp->day += (*src - '0');
    src += 2U;

    src += 2U; /* skip delimiter 'T' */

    timeStamp->hour += (*src - '0') * 10U;
    src += 2U;
    timeStamp->hour += (*src - '0');
    src += 2U;
    timeStamp->minute += (*src - '0') * 10U;
    src += 2U;
    timeStamp->minute += (*src - '0');
    src += 2U;
    timeStamp->second += (*src - '0') * 10U;
    src += 2U;
    timeStamp->second += (*src - '0');
}

/* return number of bytes. */
static uint32_t USB_DeviceGetRootPath(usb_mtp_struct_t *mtp, uint32_t storageID, uint8_t *path)
{
    uint8_t i;

    for (i = 0; i < mtp->storageList->storageCount; i++)
    {
        if (storageID == mtp->storageList->storageInfo[i].storageID)
        {
            return USB_DeviceMemCopyUnicodeString(path, (const uint16_t *)mtp->storageList->storageInfo[i].rootPath, 0);
        }
    }

    return 0;
}

static usb_status_t USB_DeviceBuildPath(usb_mtp_struct_t *mtp, uint32_t storageID, uint32_t objHandle, uint8_t *path)
{
    usb_status_t result;
    usb_mtp_obj_handle_t objHandleStruct;
    uint8_t nameLen;
    uint32_t backwardOffset;
    uint32_t forwardOffset;
    uint32_t i;

    backwardOffset = USB_DeviceGetRootPath(mtp, storageID, path);
    backwardOffset -= 2U; /* remove null-terminated */
    forwardOffset = MTP_PATH_MAX_LEN;

    objHandleStruct.idUnion.parentID = objHandle;
    while (objHandleStruct.idUnion.parentID != 0U)
    {
        result = USB_DeviceMtpObjHandleRead(objHandleStruct.idUnion.parentID, &objHandleStruct);

        if ((result != kStatus_USB_Success) || ((objHandleStruct.flag & MTP_OBJECT_DELETE) != 0U))
        {
            usb_echo("USB build path failed\r\n");
            return kStatus_USB_Error;
        }

        nameLen = USB_DeviceUnicodeStringLength(objHandleStruct.name);
        forwardOffset -= nameLen;
        forwardOffset -= 2U;

        if ((forwardOffset < backwardOffset) || (forwardOffset > MTP_PATH_MAX_LEN))
        {
            /* please increase the length of path. */
            usb_echo("Please increase the length of path\r\n");
            return kStatus_USB_Error;
        }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        /* now i indicates the offset */
        i = USB_DeviceMemCopyUnicodeString(&path[forwardOffset], objHandleStruct.name, 0);
#else
        (void)USB_DeviceMemCopyUnicodeString(&path[forwardOffset], objHandleStruct.name, 0);
#endif
        path[forwardOffset + nameLen]      = '/';
        path[forwardOffset + nameLen + 1U] = '\0';
    }

    if (forwardOffset < MTP_PATH_MAX_LEN)
    {
        path[MTP_PATH_MAX_LEN - 1U] = '\0';
        path[MTP_PATH_MAX_LEN - 2U] = '\0';

        path[backwardOffset - 2U] = '/';
        path[backwardOffset - 1U] = '\0';

        if (forwardOffset > backwardOffset)
        {
            for (i = 0; i < (MTP_PATH_MAX_LEN - forwardOffset); i++)
            {
                path[i + backwardOffset] = path[i + forwardOffset];
            }
        }
        else
        {
            /* no action, the path has been built, forwardOffset == backwardOffset */
        }
    }
    else
    {
        /* no action, root path, forwardOffset == MTP_PATH_MAX_LEN */
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceGetObjHandleInfo(usb_mtp_struct_t *mtp,
                                        uint32_t objHandle,
                                        usb_mtp_obj_handle_t *objHandleStruct)
{
    usb_status_t result;

    result = USB_DeviceMtpObjHandleRead(objHandle, objHandleStruct);

    if ((result != kStatus_USB_Success) || ((objHandleStruct->flag & MTP_OBJECT_DELETE) != 0U) ||
        (objHandleStruct->handleID != objHandle))
    {
        /* Invalid object handle */
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

static uint16_t USB_DeviceIdentifyFileType(usb_mtp_obj_handle_t *objHandleStruct)
{
    /* TODO: identify more object properties. */
    if ((objHandleStruct->flag & MTP_OBJECT_DIR) != 0U)
    {
        return MTP_FORMAT_ASSOCIATION;
    }
    else
    {
        return MTP_FORMAT_UNDEFINED;
    }
}

uint32_t USB_DeviceCheckDirDepth(const uint16_t *path)
{
    const uint8_t *src  = (const uint8_t *)path;
    uint32_t count      = 0;
    uint32_t depthCount = 0;

    if (path == NULL)
    {
        return 0;
    }

    while ((src[count] != 0U) || (src[count + 1U] != 0U) || ((count % 2U) != 0U))
    {
        if ((src[count] == '/') && (src[count + 1U] == '\0') && ((count % 2U) == 0U))
        {
            depthCount++;
        }
        count++;
    }

    return depthCount;
}

usb_status_t USB_DeviceDeleteDir(uint8_t *path)
{
    static usb_device_mtp_file_info_t fno;
    static uint32_t nestedDepth = 0U;
    static uint32_t length2;
    usb_status_t result;
    usb_device_mtp_dir_handle_t dir;
    uint32_t length;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    uint32_t offset = 0U;
#endif

    if (nestedDepth >= USB_DEVICE_MTP_DIR_INSTANCE)
    {
        usb_echo("The nested directory is too deep, please increase the dir instance\r\n");
        return kStatus_USB_Error;
    }
    nestedDepth++;

    result = USB_DeviceMtpOpenDir(&dir, (const uint16_t *)path);

    if (result == kStatus_USB_Success)
    {
        length         = USB_DeviceUnicodeStringLength((const uint16_t *)&path[0]);
        path[length++] = '/';
        path[length++] = '\0';

        for (;;)
        {
            result = USB_DeviceMtpReadDir(dir, &fno);
            if (result != kStatus_USB_Success)
            {
                break; /* Break on error or end of dir */
            }

#if USB_DEVICE_CONFIG_USE_TASK
            USB_DeviceTaskFn(g_mtp.deviceHandle);
#endif

            /* check buffer length */
            length2 = length + USB_DeviceUnicodeStringLength((const uint16_t *)(&fno.name[0]));
            length2 += 2U;

            if (length2 > MTP_PATH_MAX_LEN)
            {
                /* please increase the length of path. */
                usb_echo("Please increase the length of path\r\n");
                result = kStatus_USB_Error;
                break;
            }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            offset = USB_DeviceMemCopyUnicodeString(&path[length], (const uint16_t *)&fno.name[0], 0);
#else
            (void)USB_DeviceMemCopyUnicodeString(&path[length], (const uint16_t *)&fno.name[0], 0);
#endif

            if ((fno.attrib & USB_DEVICE_MTP_DIR) != 0U)
            {
                result = USB_DeviceDeleteDir(path); /* recursively delete directory */
                if (result != kStatus_USB_Success)
                {
                    break;
                }
            }
            else
            {
                result = USB_DeviceMtpUnlink((const uint16_t *)path); /* delete file */
                if (result != kStatus_USB_Success)
                {
                    break;
                }
            }
        }
        path[--length] = '\0';
        path[--length] = '\0';
    }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceMtpCloseDir(dir))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceMtpCloseDir(dir);
#endif
    /* kStatus_USB_InvalidRequest means end of dir, so delete the current directory. */
    if (result == kStatus_USB_InvalidRequest)
    {
        result = USB_DeviceMtpUnlink((const uint16_t *)path); /* delete self */
    }

    nestedDepth--;
    return result;
}

usb_status_t USB_DeviceAppendNameToPath(uint16_t *path, const uint16_t *name)
{
    uint32_t length;
    uint32_t length2;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    uint32_t offset = 0U;
#endif

    if ((path == NULL) && (name == NULL))
    {
        return kStatus_USB_Error;
    }

    length = USB_DeviceUnicodeStringLength((const uint16_t *)path);

    /* check path validity */
    if (length < 2U)
    {
        return kStatus_USB_Error;
    }

    /* if there is not '/' at the end of path, append '/'. */
    if (path[(length - 2U) >> 1U] != '/')
    {
        path[length >> 1U] = '/';
        length += 2U;
    }

    /* check buffer length */
    length2 = length + USB_DeviceUnicodeStringLength((const uint16_t *)name);
    length2 += 2U;

    if (length2 > MTP_PATH_MAX_LEN)
    {
        /* please increase the length of path. */
        usb_echo("Please increase the length of path\r\n");
        return kStatus_USB_Error;
    }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    offset = USB_DeviceMemCopyUnicodeString(&path[length >> 1U], (const uint16_t *)name, 0);
#else
    (void)USB_DeviceMemCopyUnicodeString(&path[length >> 1U], (const uint16_t *)name, 0);
#endif

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceCopyFile(uint8_t *destPath, uint8_t *srcPath)
{
    usb_device_mtp_file_handle_t fileSrc;
    usb_device_mtp_file_handle_t fileDest;
    uint32_t readSize;
    uint32_t writeSize;
    usb_status_t result;

    result = USB_DeviceMtpOpen(&fileSrc, (const uint16_t *)srcPath, USB_DEVICE_MTP_READ);
    if (result != kStatus_USB_Success)
    {
        return kStatus_USB_Error;
    }

#if USB_DEVICE_CONFIG_USE_TASK
    USB_DeviceTaskFn(g_mtp.deviceHandle);
#endif

    result = USB_DeviceMtpOpen(&fileDest, (const uint16_t *)destPath,
                               USB_DEVICE_MTP_CREATE_ALWAYS | USB_DEVICE_MTP_READ | USB_DEVICE_MTP_WRITE);
    if (result != kStatus_USB_Success)
    {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        result = USB_DeviceMtpClose(fileSrc);
#else
        (void)USB_DeviceMtpClose(fileSrc);
#endif
        return kStatus_USB_Error;
    }

    for (;;)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_mtp.deviceHandle);
#endif

        result = USB_DeviceMtpRead(fileSrc, &g_mtpTransferBuffer[0], USB_DEVICE_MTP_TRANSFER_BUFF_SIZE, &readSize);
        if ((result != kStatus_USB_Success) || (readSize == 0U))
        {
            break;
        }
        result = USB_DeviceMtpWrite(fileDest, &g_mtpTransferBuffer[0], readSize, &writeSize);
        if ((result != kStatus_USB_Success) || (readSize < writeSize))
        {
            break;
        }
    }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if ((kStatus_USB_Success != USB_DeviceMtpClose(fileSrc)) || (kStatus_USB_Success != USB_DeviceMtpClose(fileDest)))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceMtpClose(fileSrc);
    (void)USB_DeviceMtpClose(fileDest);
#endif

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceCopyDir(uint8_t *destPath, uint8_t *srcPath)
{
    static usb_device_mtp_file_info_t fno;
    static uint32_t length;
    static uint32_t length2;
    usb_device_mtp_dir_handle_t dirSrc;
    uint32_t lengthSrc;
    uint32_t lengthDest;
    usb_status_t result;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    uint32_t offset = 0U;
#endif

    lengthSrc  = USB_DeviceUnicodeStringLength((const uint16_t *)srcPath);
    lengthDest = USB_DeviceUnicodeStringLength((const uint16_t *)destPath);

    if ((lengthSrc == 0U) || (lengthDest == 0U))
    {
        return kStatus_USB_Error; /* invalid source or destination path */
    }

    lengthSrc--;
    while ((lengthSrc > 0U) && ((srcPath[lengthSrc] != '\0') || (srcPath[lengthSrc - 1U] != '/')))
    {
        lengthSrc--;
    }
    if (lengthSrc == 0U)
    {
        return kStatus_USB_Error; /* cannot find the directory name. */
    }
    lengthSrc++;

    destPath[lengthDest++] = '/';
    destPath[lengthDest++] = '\0';

    /* Build destination path */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    offset = USB_DeviceMemCopyUnicodeString(&destPath[lengthDest], (const uint16_t *)&srcPath[lengthSrc], 0U);
#else
    (void)USB_DeviceMemCopyUnicodeString(&destPath[lengthDest], (const uint16_t *)&srcPath[lengthSrc], 0U);
#endif

    length2 = USB_DeviceCheckDirDepth((const uint16_t *)&destPath[0]);

    if (length2 > USB_DEVICE_MTP_DIR_INSTANCE)
    {
        usb_echo("The created directory is too deep, please increase the dir instance\r\n");
        result = kStatus_USB_Error;
    }
    else
    {
        result = USB_DeviceMtpMakeDir((const uint16_t *)destPath); /* Create directory */
    }

    if (result == kStatus_USB_Success)
    {
        /* Open directory */
        result = USB_DeviceMtpOpenDir(&dirSrc, (const uint16_t *)srcPath);
    }

    if (result == kStatus_USB_Success)
    {
        lengthSrc            = USB_DeviceUnicodeStringLength((const uint16_t *)srcPath);
        srcPath[lengthSrc++] = '/';
        srcPath[lengthSrc++] = '\0';

        for (;;)
        {
            result = USB_DeviceMtpReadDir(dirSrc, &fno);
            if ((result == kStatus_USB_InvalidRequest) || (result != kStatus_USB_Success))
            {
                if (result == kStatus_USB_InvalidRequest)
                {
                    result = kStatus_USB_Success; /* return success when reaching end of dir */
                }
                break; /* Break on error or end of dir */
            }

#if USB_DEVICE_CONFIG_USE_TASK
            USB_DeviceTaskFn(g_mtp.deviceHandle);
#endif

            /* check buffer length */
            length2 = lengthSrc + USB_DeviceUnicodeStringLength((const uint16_t *)(&fno.name[0]));
            length2 += 2U;

            if (length2 > MTP_PATH_MAX_LEN)
            {
                /* please increase the length of path. */
                usb_echo("Please increase the length of path\r\n");
                result = kStatus_USB_Error;
                break;
            }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            offset = USB_DeviceMemCopyUnicodeString(&srcPath[lengthSrc], (const uint16_t *)&fno.name[0], 0);
#else
            (void)USB_DeviceMemCopyUnicodeString(&srcPath[lengthSrc], (const uint16_t *)&fno.name[0], 0);
#endif
            if ((fno.attrib & USB_DEVICE_MTP_DIR) != 0U)
            {
                result = USB_DeviceCopyDir(destPath, srcPath); /* recursively copy directory */
                if (result != kStatus_USB_Success)
                {
                    break;
                }
            }
            else
            {
                length             = USB_DeviceUnicodeStringLength((const uint16_t *)destPath);
                destPath[length++] = '/';
                destPath[length++] = '\0';

                /* check buffer length */
                length2 = length + USB_DeviceUnicodeStringLength((const uint16_t *)(&fno.name[0]));
                length2 += 2U;

                if (length2 > MTP_PATH_MAX_LEN)
                {
                    /* please increase the length of path. */
                    usb_echo("Please increase the length of path\r\n");
                    result = kStatus_USB_Error;
                    break;
                }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                offset = USB_DeviceMemCopyUnicodeString(&destPath[length], (const uint16_t *)&fno.name[0], 0);
#else
                (void)USB_DeviceMemCopyUnicodeString(&destPath[length], (const uint16_t *)&fno.name[0], 0);
#endif
                result = USB_DeviceCopyFile(destPath, srcPath); /* copy file */
                if (result != kStatus_USB_Success)
                {
                    break;
                }
                destPath[--length] = '\0';
                destPath[--length] = '\0';
            }
        }
        srcPath[--lengthSrc] = '\0';
        srcPath[--lengthSrc] = '\0';
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        result = USB_DeviceMtpCloseDir(dirSrc);
#else
        (void)USB_DeviceMtpCloseDir(dirSrc);
#endif
    }

    destPath[--lengthDest] = '\0';
    destPath[--lengthDest] = '\0';

    return result;
}

static uint32_t USB_DeviceAssignDevPropVal(
    uint16_t devPropCode, uint16_t dataType, void *buffer, void *val, uint32_t offset)
{
    switch (devPropCode)
    {
        case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
            offset = USB_DeviceMemCopyUnicodeString(buffer, (const uint16_t *)g_mtp.devFriendlyName, offset);
            break;

        default:
            /* TODO: identify more device property code. */
            offset = USB_DeviceParseDataType(dataType, buffer, val, offset);
            break;
    }

    return offset;
}

static uint32_t USB_DeviceAssignObjPropVal(
    uint16_t objPropCode, uint16_t dataType, void *buffer, usb_mtp_obj_handle_t *objHandleStruct, uint32_t offset)
{
    uint32_t temp;
    uint32_t uid[4];
    char dateTimeBuffer[MTP_DATE_TIME_LEN];

    switch (objPropCode)
    {
        case MTP_OBJECT_PROPERTY_STORAGE_ID:
            offset = USB_DeviceParseDataType(dataType, buffer, &objHandleStruct->storageID, offset);
            break;

        case MTP_OBJECT_PROPERTY_OBJECT_FORMAT:
            temp   = USB_DeviceIdentifyFileType(objHandleStruct);
            offset = USB_DeviceParseDataType(dataType, buffer, &temp, offset);
            break;

        case MTP_OBJECT_PROPERTY_PROTECTION_STATUS:
            temp   = 0;
            offset = USB_DeviceParseDataType(dataType, buffer, &temp, offset);
            break;

        case MTP_OBJECT_PROPERTY_OBJECT_SIZE:
            offset = USB_DeviceParseDataType(dataType, buffer, &objHandleStruct->size, offset);
            break;

        case MTP_OBJECT_PROPERTY_OBJECT_FILE_NAME:
            offset = USB_DeviceMemCopyUnicodeString(buffer, &objHandleStruct->name[0], offset);
            break;

        case MTP_OBJECT_PROPERTY_DATE_MODIFIED:
            snprintf(&dateTimeBuffer[0], sizeof(dateTimeBuffer), "%.4d%.2d%.2dT%.2d%.2d%.2d",
                     objHandleStruct->dateUnion.dateBitField.year, objHandleStruct->dateUnion.dateBitField.month,
                     objHandleStruct->dateUnion.dateBitField.day, objHandleStruct->timeUnion.timeBitField.hour,
                     objHandleStruct->timeUnion.timeBitField.minute, objHandleStruct->timeUnion.timeBitField.second);

            offset = USB_DeviceParseDataType(dataType, buffer, &dateTimeBuffer[0], offset);
            break;

        case MTP_OBJECT_PROPERTY_PERSISTENT_UID:
            uid[0] = objHandleStruct->handleID;
            uid[1] = objHandleStruct->storageID;
            uid[2] = 0;
            uid[3] = 0;
            offset = USB_DeviceParseDataType(dataType, buffer, &uid[0], offset);
            break;

        case MTP_OBJECT_PROPERTY_PARENT_OBJECT:
            offset = USB_DeviceParseDataType(dataType, buffer, &objHandleStruct->idUnion.parentID, offset);
            break;

        case MTP_OBJECT_PROPERTY_NAME:
            offset = USB_DeviceMemCopyUnicodeString(buffer, &objHandleStruct->name[0], offset);
            break;

        case MTP_OBJECT_PROPERTY_DISPLAY_NAME:
            offset = USB_DeviceMemCopyUnicodeString(buffer, &objHandleStruct->name[0], offset);
            break;

        case MTP_OBJECT_PROPERTY_DATE_ADDED:
            snprintf(&dateTimeBuffer[0], sizeof(dateTimeBuffer), "%.4d%.2d%.2dT%.2d%.2d%.2d",
                     objHandleStruct->dateUnion.dateBitField.year, objHandleStruct->dateUnion.dateBitField.month,
                     objHandleStruct->dateUnion.dateBitField.day, objHandleStruct->timeUnion.timeBitField.hour,
                     objHandleStruct->timeUnion.timeBitField.minute, objHandleStruct->timeUnion.timeBitField.second);

            offset = USB_DeviceParseDataType(dataType, buffer, &dateTimeBuffer[0], offset);
            break;

        default:
            /* no action */
            break;
    }

    return offset;
}

void USB_DeviceCmdOpenSession(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    usb_status_t result;
    uint8_t i;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        g_mtp.nextHandleID = 0x01U;

        g_mtp.validObjInfo = 0x00U;

        for (i = 0; i < g_mtp.storageList->storageCount; i++)
        {
            g_mtp.storageList->storageInfo[i].flag &= ~MTP_OBJECT_ALREADY_GET;
        }

        result = USB_DeviceMtpObjHandleInit();

        if (result != kStatus_USB_Success)
        {
            dataInfo->code = MTP_RESPONSE_DEVICE_BUSY;
        }
        else
        {
            dataInfo->code = MTP_RESPONSE_OK;
        }
    }
}

void USB_DeviceCmdCloseSession(void *param)
{
    uint8_t i;

    g_mtp.nextHandleID = 0x01U;

    g_mtp.validObjInfo = 0x00U;

    for (i = 0; i < g_mtp.storageList->storageCount; i++)
    {
        g_mtp.storageList->storageInfo[i].flag &= ~MTP_OBJECT_ALREADY_GET;
    }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if (kStatus_USB_Success != USB_DeviceMtpObjHandleDeinit())
    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
        usb_echo("mtp handle de-init error\r\n");
#endif
    }
#else
    (void)USB_DeviceMtpObjHandleDeinit();
#endif

    if (param != NULL)
    {
        ((usb_device_mtp_cmd_data_struct_t *)param)->code = MTP_RESPONSE_OK;
    }
}

void USB_DeviceCmdGetDeviceInfo(void *param, usb_device_mtp_device_info_t *deviceInfo)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        /* Build DeviceInfo dataset here. For more information about DeviceInfo dataset,
           please refer to chapter 5.1.1 in the MTP spec Rev1.1. */
        offset = USB_DeviceMemCopyWord(readBuf, MTP_STANDARD_VERSION, offset);         /* standard version */
        offset = USB_DeviceMemCopyDword(readBuf, MTP_VENDOR_EXTENSION_ID, offset);     /* MTP vendor extension ID */
        offset = USB_DeviceMemCopyWord(readBuf, MTP_VERSION, offset);                  /* MTP version */
        offset = USB_DeviceMemCopyString(readBuf, deviceInfo->mtpExtendsions, offset); /* MTP extensions */
        offset = USB_DeviceMemCopyWord(readBuf, deviceInfo->functionalMode, offset);   /* functional Mode */
        offset = USB_DeviceMemCopyArray(readBuf, deviceInfo->opSupported, deviceInfo->opSupportedLength, 2U, offset);
        offset = USB_DeviceMemCopyArray(readBuf, deviceInfo->eventSupported, deviceInfo->eventSupportedLength, 2U,
                                        offset); /* event supported */
        offset = USB_DeviceMemCopyArray(readBuf, deviceInfo->devPropSupported, deviceInfo->devPropSupportedLength, 2U,
                                        offset); /* device properties supported */
        offset = USB_DeviceMemCopyArray(readBuf, deviceInfo->captureFormat, deviceInfo->captureFormatLength, 2U,
                                        offset); /* capture formats */
        offset = USB_DeviceMemCopyArray(readBuf, deviceInfo->playbackFormat, deviceInfo->playbackFormatLength, 2U,
                                        offset);                                      /* playback formats */
        offset = USB_DeviceMemCopyString(readBuf, deviceInfo->manufacturer, offset);  /* manufacturer */
        offset = USB_DeviceMemCopyString(readBuf, deviceInfo->model, offset);         /* model */
        offset = USB_DeviceMemCopyString(readBuf, deviceInfo->deviceVersion, offset); /* device version */
        offset = USB_DeviceMemCopyString(readBuf, deviceInfo->serialNumber, offset);  /* serial number */

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdGetDevicePropDesc(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint16_t devPropCode                       = dataInfo->param[0];
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t i;
    uint32_t j;
    usb_device_mtp_dev_prop_desc_t *devPropDesc;
    uint32_t count;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        devPropDesc = g_mtp.devPropDescList->devPropDesc;
        count       = g_mtp.devPropDescList->devPropDescCount;

        for (i = 0; i < count; i++)
        {
            if (devPropDesc[i].devPropCode == devPropCode)
            {
                /* Build device property describing dataset here. For more information about device property describing
                   dataset, please refer to chapter 5.1.2 in the MTP spec Rev1.1. */
                offset = USB_DeviceMemCopyWord(readBuf, devPropDesc[i].devPropCode, offset);
                offset = USB_DeviceMemCopyWord(readBuf, devPropDesc[i].dataType, offset);
                offset = USB_DeviceMemCopyByte(readBuf, devPropDesc[i].getSet, offset);

                offset = USB_DeviceParseDataType(devPropDesc[i].dataType, readBuf, &devPropDesc[i].defaultVal, offset);

                offset = USB_DeviceAssignDevPropVal(devPropDesc[i].devPropCode, devPropDesc[i].dataType, readBuf,
                                                    &devPropDesc[i].currentVal, offset);

                offset = USB_DeviceMemCopyByte(readBuf, devPropDesc[i].formFlag, offset);

                switch (devPropDesc[i].formFlag)
                {
                    case MTP_FORM_FLAG_RANGE:
                    {
                        offset = USB_DeviceParseDataType(devPropDesc[i].dataType, readBuf,
                                                         &devPropDesc[i].form.rangeForm.minVal, offset);
                        offset = USB_DeviceParseDataType(devPropDesc[i].dataType, readBuf,
                                                         &devPropDesc[i].form.rangeForm.maxVal, offset);
                        offset = USB_DeviceParseDataType(devPropDesc[i].dataType, readBuf,
                                                         &devPropDesc[i].form.rangeForm.stepSize, offset);
                        break;
                    }

                    case MTP_FORM_FLAG_ENUMERATION:
                    {
                        offset = USB_DeviceMemCopyWord(readBuf, devPropDesc[i].form.enumForm.num, offset);
                        for (j = 0; j < devPropDesc[i].form.enumForm.num; j++)
                        {
                            offset = USB_DeviceParseDataType(devPropDesc[i].dataType, readBuf,
                                                             &devPropDesc[i].form.enumForm.val[j], offset);
                        }
                        break;
                    }

                    default:
                        /* no action */
                        break;
                }

                break; /* exit from loop. */
            }
        }

        if (i == count)
        {
            /* not support device property */
            dataInfo->code = MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
        }
        else
        {
            dataInfo->totalSize = offset;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
}

void USB_DeviceCmdGetObjPropsSupported(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint16_t objPropCode                       = dataInfo->param[0];
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t i;
    uint32_t j;
    usb_device_mtp_obj_prop_t *objPropDescList;
    uint32_t count;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        objPropDescList = g_mtp.objPropList->objProp;
        count           = g_mtp.objPropList->objPropCount;

        for (i = 0; i < count; i++)
        {
            if (objPropDescList[i].objFormat == objPropCode)
            {
                offset = USB_DeviceMemCopyDword(readBuf, (uint32_t)objPropDescList[i].objPropDescCount, offset);
                for (j = 0; j < objPropDescList[i].objPropDescCount; j++)
                {
                    offset = USB_DeviceMemCopyWord(readBuf, objPropDescList[i].objPropDesc[j].objPropCode, offset);
                }
                break;
            }
        }

        if (i == count)
        {
            /* not support device property */
            dataInfo->code = MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
        }
        else
        {
            dataInfo->totalSize = offset;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
}

void USB_DeviceCmdGetStorageIDs(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint8_t i;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        offset = USB_DeviceMemCopyDword(readBuf, g_mtp.storageList->storageCount, offset);
        for (i = 0; i < g_mtp.storageList->storageCount; i++)
        {
            offset = USB_DeviceMemCopyDword(readBuf, g_mtp.storageList->storageInfo[i].storageID, offset);
        }

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdGetStorageInfo(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t storageID                         = dataInfo->param[0];
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint8_t i;
    const uint8_t *rootPath;
    uint64_t freeBytes;
    uint64_t totalBytes;
    usb_device_mtp_storage_info_t *storageInfo;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        rootPath    = NULL;
        storageInfo = g_mtp.storageList->storageInfo;

        for (i = 0; i < g_mtp.storageList->storageCount; i++)
        {
            if (storageInfo[i].storageID == storageID)
            {
                rootPath = storageInfo[i].rootPath;
                break;
            }
        }

        if (rootPath == NULL)
        {
            /* invalid storage ID */
            dataInfo->code = MTP_RESPONSE_INVALID_STORAGE_ID;
        }
        else
        {
            /* Build StorageInfo dataset here. For more information about StorageInfo dataset,
               please refer to chapter 5.2.2 in the MTP spec Rev1.1. */
            USB_DeviceMtpGetDiskTotalBytes((const uint16_t *)rootPath, &totalBytes);
            USB_DeviceMtpGetDiskFreeBytes((const uint16_t *)rootPath, &freeBytes);

            offset = USB_DeviceMemCopyWord(readBuf, storageInfo[i].storageType, offset);
            offset = USB_DeviceMemCopyWord(readBuf, storageInfo[i].fileSystemType, offset);
            offset = USB_DeviceMemCopyWord(readBuf, storageInfo[i].accessCapability, offset);
            offset = USB_DeviceMemCopyQword(readBuf, totalBytes, offset);
            offset = USB_DeviceMemCopyQword(readBuf, freeBytes, offset);
            offset = USB_DeviceMemCopyDword(readBuf, USB_DEVICE_MTP_MAX_UINT32_VAL, offset);
            offset = USB_DeviceMemCopyString(readBuf, storageInfo[i].storageDesc, offset);
            offset = USB_DeviceMemCopyString(readBuf, storageInfo[i].volumeID, offset);

            dataInfo->totalSize = offset;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
}

void USB_DeviceCmdGetObjHandles(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset;
    uint8_t *readBuf       = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t storageID     = dataInfo->param[0];
    uint16_t objFormatCode = dataInfo->param[1];
    uint32_t objHandle     = dataInfo->param[2];
    usb_status_t result;
    usb_device_mtp_dir_handle_t dir;
    usb_device_mtp_file_info_t fno;
    usb_mtp_obj_handle_t objHandleStruct;
    uint32_t objHandleCount;
    uint32_t j;
    uint8_t i;
    uint8_t storageCount                       = g_mtp.storageList->storageCount;
    usb_device_mtp_storage_info_t *storageInfo = g_mtp.storageList->storageInfo;

    /* Check storage ID */
    if (storageID != USB_DEVICE_MTP_MAX_UINT32_VAL)
    {
        for (i = 0; i < storageCount; i++)
        {
            if (storageInfo[i].storageID == storageID)
            {
                break;
            }
        }

        if (i == storageCount)
        {
            /* invalid storage ID */
            dataInfo->code = MTP_RESPONSE_INVALID_STORAGE_ID;
            return;
        }

        storageInfo  = &storageInfo[i];
        storageCount = 1;
    }
    else
    {
        storageID = storageInfo[0].storageID;
    }

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        /* Check format code */
        if (objFormatCode != 0U)
        {
            /* unspported format code */
            dataInfo->code = MTP_RESPONSE_SPECIFICATION_BY_FORMAT_UNSUPPORTED;
            return;
        }

        objHandleCount = 0U;
        offset         = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH + 4U;

        for (i = 0; i < storageCount; i++)
        {
            if (storageCount > 1)
            {
                storageID = storageInfo[i].storageID;
            }

            if (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL)
            {
                /* root path */
                objHandle = 0;

                if ((storageInfo[i].flag & MTP_OBJECT_ALREADY_GET) == 0U)
                {
                    USB_DeviceGetRootPath(&g_mtp, storageID, g_mtp.path);
                }
            }
            else
            {
                /* build path here. */
                if ((objHandle >= g_mtp.nextHandleID) || (objHandle == 0U) ||
                    (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
                    (USB_DeviceBuildPath(&g_mtp, storageID, objHandle, g_mtp.path) != kStatus_USB_Success))
                {
                    /* invalid object handle */
                    dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
                    return;
                }

                if ((objHandleStruct.flag & MTP_OBJECT_DIR) == 0U)
                {
                    /* invalid parent object */
                    dataInfo->code = MTP_RESPONSE_INVALID_PARENT_OBJECT;
                    return;
                }
            }

            if (((objHandle != 0U) && ((objHandleStruct.flag & MTP_OBJECT_ALREADY_GET) != 0U)) ||
                ((objHandle == 0U) && ((storageInfo[i].flag & MTP_OBJECT_ALREADY_GET) != 0U)))
            {
                /* traverse object lists to find its child */
                for (j = 1U; j < g_mtp.nextHandleID; j++)
                {
                    result = USB_DeviceMtpObjHandleRead(j, &objHandleStruct);
                    if (result != kStatus_USB_Success)
                    {
                        break; /* reach the end */
                    }

                    if ((objHandleStruct.idUnion.parentID == objHandle) &&
                        ((objHandleStruct.flag & MTP_OBJECT_DELETE) == 0U))
                    {
                        objHandleCount++;
                        if (offset < USB_DEVICE_MTP_TRANSFER_BUFF_SIZE) /* Buffer is full, need to chunk? */
                        {
                            offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.handleID, offset);
                        }
                    }
                }
            }
            else
            {
                /* traverse directory to find its child */
                result = USB_DeviceMtpOpenDir(&dir, (const uint16_t *)&g_mtp.path[0]);
                for (;;)
                {
                    result = USB_DeviceMtpReadDir(dir, &fno);
                    if (result != kStatus_USB_Success)
                    {
                        break; /* Break on error or end of dir */
                    }

                    if (offset < USB_DEVICE_MTP_TRANSFER_BUFF_SIZE) /* Buffer is full, need to chunk? */
                    {
                        objHandleStruct.handleID = g_mtp.nextHandleID;
                        g_mtp.nextHandleID++;

                        objHandleStruct.idUnion.parentID = objHandle;
                        objHandleStruct.storageID        = storageID;
                        objHandleStruct.size             = fno.size;
                        objHandleStruct.dateUnion.date   = fno.dateUnion.date;
                        objHandleStruct.timeUnion.time   = fno.timeUnion.time;
                        objHandleStruct.flag             = 0U;
                        if ((fno.attrib & USB_DEVICE_MTP_DIR) != 0U)
                        {
                            objHandleStruct.flag |= MTP_OBJECT_DIR;
                        }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                        /* now j indicates the offset */
                        j = USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)&fno.name[0],
                                                           0U);
#else
                        (void)USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)&fno.name[0],
                                                             0U);
#endif
                        result = USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct);

                        offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.handleID, offset);
                    }
                    objHandleCount++;
                }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                result = USB_DeviceMtpCloseDir(dir);
#else
                (void)USB_DeviceMtpCloseDir(dir);
#endif
            }
        }

        /* number of object Handle */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        j = USB_DeviceMemCopyDword(readBuf, objHandleCount, USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH);
#else
        (void)USB_DeviceMemCopyDword(readBuf, objHandleCount, USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH);
#endif

        g_mtp.transferTotalSize = objHandleCount * 4U + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH + 4U;
        dataInfo->totalSize     = g_mtp.transferTotalSize;
        dataInfo->curSize       = offset;
        dataInfo->buffer        = readBuf;
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_DATA)
    {
        /* Buffer is full, need to chunk. */
        if (dataInfo->curPos < g_mtp.transferTotalSize)
        {
            offset = 0U;

            /* the number of object handle has been sent. */
            objHandleCount = (dataInfo->curPos - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH - 4U) / 4U;

            for (i = 0; i < storageCount; i++)
            {
                if (storageCount > 1)
                {
                    storageID = storageInfo[i].storageID;
                }

                if (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL)
                {
                    /* root path */
                    objHandle = 0;

                    if ((storageInfo[i].flag & MTP_OBJECT_ALREADY_GET) == 0U)
                    {
                        USB_DeviceGetRootPath(&g_mtp, storageID, g_mtp.path);
                    }
                }
                else
                {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    if ((kStatus_USB_Success != USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct)) ||
                        USB_DeviceBuildPath(&g_mtp, storageID, objHandle, g_mtp.path))
                    {
                        return;
                    }
#else
                    (void)USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct);
                    (void)USB_DeviceBuildPath(&g_mtp, storageID, objHandle, g_mtp.path);
#endif
                }

                if (((objHandle != 0U) && ((objHandleStruct.flag & MTP_OBJECT_ALREADY_GET) != 0U)) ||
                    ((objHandle == 0U) && ((storageInfo[i].flag & MTP_OBJECT_ALREADY_GET) != 0U)))
                {
                    /* traverse object lists to find its child. */
                    for (j = 1U; j < g_mtp.nextHandleID; j++)
                    {
                        result = USB_DeviceMtpObjHandleRead(j, &objHandleStruct);

                        if (result != kStatus_USB_Success)
                        {
                            break; /* reach the end */
                        }

                        if ((objHandleStruct.idUnion.parentID == objHandle) &&
                            ((objHandleStruct.flag & MTP_OBJECT_DELETE) == 0U))
                        {
                            if (objHandleCount == 0U)
                            {
                                if (offset < USB_DEVICE_MTP_TRANSFER_BUFF_SIZE) /* Buffer is full, need to chunk? */
                                {
                                    offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.handleID, offset);
                                }
                            }
                            else
                            {
                                objHandleCount--;
                            }
                        }
                    }
                }
                else
                {
                    /* traverse directory to find its child */
                    result = USB_DeviceMtpOpenDir(&dir, (const uint16_t *)&g_mtp.path[0]);
                    for (;;)
                    {
                        result = USB_DeviceMtpReadDir(dir, &fno);
                        if (result != kStatus_USB_Success)
                        {
                            break; /* Break on error or end of dir */
                        }
                        if (objHandleCount == 0U)
                        {
                            if (offset < USB_DEVICE_MTP_TRANSFER_BUFF_SIZE) /* Buffer is full, need to chunk? */
                            {
                                objHandleStruct.handleID = g_mtp.nextHandleID;
                                g_mtp.nextHandleID++;

                                objHandleStruct.idUnion.parentID = objHandle;
                                objHandleStruct.storageID        = storageID;
                                objHandleStruct.size             = fno.size;
                                objHandleStruct.dateUnion.date   = fno.dateUnion.date;
                                objHandleStruct.timeUnion.time   = fno.timeUnion.time;
                                objHandleStruct.flag             = 0U;
                                if ((fno.attrib & USB_DEVICE_MTP_DIR) != 0U)
                                {
                                    objHandleStruct.flag |= MTP_OBJECT_DIR;
                                }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                                /* now j indicates the offset */
                                j = USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0],
                                                                   (const uint16_t *)&fno.name[0], 0U);
#else
                                (void)USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0],
                                                                     (const uint16_t *)&fno.name[0], 0U);
#endif

                                result = USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct);

                                offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.handleID, offset);
                            }
                        }
                        else
                        {
                            objHandleCount--;
                        }
                    }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                    result = USB_DeviceMtpCloseDir(dir);
#else
                    (void)USB_DeviceMtpCloseDir(dir);
#endif
                }
            }

            dataInfo->totalSize = g_mtp.transferTotalSize;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE)
    {
        /* record the directory has been accessed. */
        for (i = 0; i < storageCount; i++)
        {
            if (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL)
            {
                /* root path */
                storageInfo[i].flag |= MTP_OBJECT_ALREADY_GET;
            }
            else
            {
                result = USB_DeviceMtpObjHandleRead(objHandle, &objHandleStruct);
                objHandleStruct.flag |= MTP_OBJECT_ALREADY_GET;
                result = USB_DeviceMtpObjHandleWrite(objHandle, &objHandleStruct);
            }
        }
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdGetObjPropDesc(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint16_t objPropCode                       = dataInfo->param[0];
    uint16_t objFormatCode                     = dataInfo->param[1];
    uint32_t i;
    uint32_t j;
    uint16_t n;
    usb_device_mtp_obj_prop_desc_t *propDesc;
    usb_device_mtp_obj_prop_t *prop;
    uint32_t propCount;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        prop      = g_mtp.objPropList->objProp;
        propCount = g_mtp.objPropList->objPropCount;

        for (i = 0U; i < propCount; i++)
        {
            if (prop[i].objFormat == objFormatCode)
            {
                break;
            }
        }

        if (i == propCount)
        {
            /* invalid object format code */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_FORMAT_CODE;
            return;
        }

        for (j = 0U; j < prop[i].objPropDescCount; j++)
        {
            if (prop[i].objPropDesc[j].objPropCode == objPropCode)
            {
                break;
            }
        }

        if (j == prop[i].objPropDescCount)
        {
            /* invalid object property code */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_PROP_CODE;
            return;
        }

        propDesc = &prop[i].objPropDesc[j];

        /* Build object property describing dataset here. For more information about object property describing dataset,
           please refer to chapter 5.3.2.3 in the MTP spec Rev1.1. */
        offset = USB_DeviceMemCopyWord(readBuf, propDesc->objPropCode, offset);
        offset = USB_DeviceMemCopyWord(readBuf, propDesc->dataType, offset);
        offset = USB_DeviceMemCopyByte(readBuf, propDesc->getSet, offset);
        offset = USB_DeviceParseDataType(propDesc->dataType, readBuf, &propDesc->defaultVal, offset);
        offset = USB_DeviceMemCopyDword(readBuf, propDesc->groupCode, offset); /* group code */
        offset = USB_DeviceMemCopyByte(readBuf, propDesc->formFlag, offset);   /* form flag */

        switch (propDesc->formFlag)
        {
            case MTP_FORM_FLAG_RANGE:
            {
                offset = USB_DeviceParseDataType(propDesc->dataType, readBuf, &propDesc->form.rangeForm.minVal, offset);
                offset = USB_DeviceParseDataType(propDesc->dataType, readBuf, &propDesc->form.rangeForm.maxVal, offset);
                offset =
                    USB_DeviceParseDataType(propDesc->dataType, readBuf, &propDesc->form.rangeForm.stepSize, offset);
                break;
            }

            case MTP_FORM_FLAG_ENUMERATION:
            {
                offset = USB_DeviceMemCopyWord(readBuf, propDesc->form.enumForm.num, offset);

                for (n = 0; n < propDesc->form.enumForm.num; n++)
                {
                    offset =
                        USB_DeviceParseDataType(propDesc->dataType, readBuf, &propDesc->form.enumForm.val[n], offset);
                }
                break;
            }

            case MTP_FORM_FLAG_FIXED_LENGTH_ARRAY:
            case MTP_FORM_FLAG_BYTE_ARRAY:
            case MTP_FORM_FLAG_LONG_STRING:
                offset = USB_DeviceMemCopyDword(readBuf, propDesc->form.fixedLengthArray, offset);
                break;

            case MTP_FORM_FLAG_REGULAR_EXPRESSION:
                offset = USB_DeviceMemCopyString(readBuf, propDesc->form.regularExpression, offset);
                break;

            case MTP_FORM_FLAG_DATA_TIME:
            default:
                /* no action */
                break;
        }

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdGetObjPropList(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t objHandle                         = dataInfo->param[0];
    uint32_t objFormatCode                     = dataInfo->param[1];
    uint32_t objPropCode                       = dataInfo->param[2];
    uint32_t objPropGroupCode                  = dataInfo->param[3];
    uint32_t depth                             = dataInfo->param[4];
    uint32_t i;
    uint32_t j;
    usb_mtp_obj_handle_t objHandleStruct;
    uint16_t dataType;
    usb_device_mtp_obj_prop_t *objProp;
    uint32_t objPropCount;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if (objFormatCode != 0U)
        {
            /* unsupported format */
            dataInfo->code = MTP_RESPONSE_SPECIFICATION_BY_FORMAT_UNSUPPORTED;
            return;
        }

        if (objPropCode == 0U)
        {
            /* unsupported group code */
            dataInfo->code = MTP_RESPONSE_SPECIFICATION_BY_GROUP_UNSUPPORTED;
            return;
        }

        if (depth != 0U)
        {
            /* unsupported depth */
            dataInfo->code = MTP_RESPONSE_SPECIFICATION_BY_DEPTH_UNSUPPORTED;
            return;
        }

        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        /* identify file type */
        objFormatCode = USB_DeviceIdentifyFileType(&objHandleStruct);

        objProp      = g_mtp.objPropList->objProp;
        objPropCount = g_mtp.objPropList->objPropCount;

        if (objPropCode != USB_DEVICE_MTP_MAX_UINT32_VAL)
        {
            j = 0U;
            for (i = 0U; i < objPropCount; i++)
            {
                if (objProp[i].objFormat == objFormatCode)
                {
                    for (; j < objProp[i].objPropDescCount; j++)
                    {
                        if (objProp[i].objPropDesc[j].objPropCode == (uint16_t)objPropCode)
                        {
                            break;
                        }
                    }
                    break;
                }
            }

            if ((i == objPropCount) || (j == objProp[i].objPropDescCount))
            {
                /* unsupported property code */
                dataInfo->code = MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED;
                return;
            }

            dataType = objProp[i].objPropDesc[j].dataType;

            /* Build object property list dataset here. For more information about object property list dataset,
               please refer to Appendix E.2.1.1 in the MTP spec Rev1.1. */
            offset = USB_DeviceMemCopyDword(readBuf, 1U, offset);         /* number of elements */
            offset = USB_DeviceMemCopyDword(readBuf, objHandle, offset);  /* object handle */
            offset = USB_DeviceMemCopyWord(readBuf, objPropCode, offset); /* object property */
            offset = USB_DeviceMemCopyWord(readBuf, dataType, offset);    /* data type */
            offset =
                USB_DeviceAssignObjPropVal(objPropCode, dataType, readBuf, &objHandleStruct, offset); /* object value */
        }
        else
        {
            for (i = 0U; i < objPropCount; i++)
            {
                if (objProp[i].objFormat == objFormatCode)
                {
                    objProp = &objProp[i];
                    break;
                }
            }

            offset = USB_DeviceMemCopyDword(readBuf, objProp->objPropDescCount, offset); /* number of elements */
            for (j = 0; j < objProp->objPropDescCount; j++)
            {
                dataType    = objProp->objPropDesc[j].dataType;
                objPropCode = objProp->objPropDesc[j].objPropCode;

                offset = USB_DeviceMemCopyDword(readBuf, objHandle, offset);  /* object handle */
                offset = USB_DeviceMemCopyWord(readBuf, objPropCode, offset); /* object property */
                offset = USB_DeviceMemCopyWord(readBuf, dataType, offset);    /* data type */
                offset = USB_DeviceAssignObjPropVal(objPropCode, dataType, readBuf, &objHandleStruct,
                                                    offset); /* object value */
            }
        }

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;

        (void)objPropGroupCode; /* avoid compiler warning */
    }
}

void USB_DeviceCmdGetObjInfo(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint32_t objHandle                         = dataInfo->param[0];
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    usb_mtp_obj_handle_t objHandleStruct;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        /* Build ObjectInfo dataset here. For more information about ObjectInfo dataset,
           please refer to chapter 5.3.1 in the MTP spec Rev1.1. */
        offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.storageID, offset); /* storage ID */
        offset =
            USB_DeviceMemCopyWord(readBuf, USB_DeviceIdentifyFileType(&objHandleStruct), offset); /* object format */
        offset = USB_DeviceMemCopyWord(readBuf, 0, offset);                                 /* protection status */
        offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.size, offset);             /* object compressed size */
        offset = USB_DeviceMemCopyWord(readBuf, 0, offset);                                 /* thumb format */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* thumb compressed size */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* thumb pix width */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* thumb pix height */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* image pix width */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* image pix height */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                                /* image bit depth */
        offset = USB_DeviceMemCopyDword(readBuf, objHandleStruct.idUnion.parentID, offset); /* parent object */
        offset = USB_DeviceMemCopyWord(readBuf, 0, offset);                                 /* association type */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                            /* association description */
        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);                            /* sequence number */
        offset = USB_DeviceMemCopyUnicodeString(readBuf, objHandleStruct.name, offset); /* file name */
        offset = USB_DeviceAssignObjPropVal(MTP_OBJECT_PROPERTY_DATE_ADDED, MTP_TYPE_STR, readBuf, &objHandleStruct,
                                            offset); /* date created */
        offset = USB_DeviceAssignObjPropVal(MTP_OBJECT_PROPERTY_DATE_MODIFIED, MTP_TYPE_STR, readBuf, &objHandleStruct,
                                            offset);                    /* date modified */
        offset = USB_DeviceMemCopyUnicodeString(readBuf, NULL, offset); /* keywords */

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdGetObj(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset;
    uint32_t objHandle = dataInfo->param[0];
    uint8_t *readBuf   = (uint8_t *)&g_mtpTransferBuffer[0];
    usb_mtp_obj_handle_t objHandleStruct;
    uint32_t sizeRead;
    usb_status_t result;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
            ((objHandleStruct.flag & MTP_OBJECT_DIR) != 0U) ||
            (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, g_mtp.path) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        offset =
            (objHandleStruct.size > (USB_DEVICE_MTP_TRANSFER_BUFF_SIZE - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH)) ?
                (USB_DEVICE_MTP_TRANSFER_BUFF_SIZE - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH) :
                objHandleStruct.size;

        result = USB_DeviceMtpOpen(&g_mtp.file, (const uint16_t *)&g_mtp.path[0], USB_DEVICE_MTP_READ);
        result = USB_DeviceMtpRead(g_mtp.file, readBuf + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH, offset, &sizeRead);

        if ((result != kStatus_USB_Success) || (sizeRead < offset))
        {
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
        }
        else
        {
            g_mtp.transferTotalSize = objHandleStruct.size + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;

            dataInfo->totalSize = g_mtp.transferTotalSize;
            dataInfo->curSize   = offset + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
            dataInfo->buffer    = readBuf;
        }
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_DATA)
    {
        offset = ((g_mtp.transferTotalSize - dataInfo->curPos) > USB_DEVICE_MTP_TRANSFER_BUFF_SIZE) ?
                     USB_DEVICE_MTP_TRANSFER_BUFF_SIZE :
                     (g_mtp.transferTotalSize - dataInfo->curPos);

        result = USB_DeviceMtpLseek(g_mtp.file, dataInfo->curPos - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH);
        result = USB_DeviceMtpRead(g_mtp.file, readBuf, offset, &sizeRead);

        if ((result != kStatus_USB_Success) || (sizeRead < offset))
        {
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
        }
        else
        {
            dataInfo->totalSize = g_mtp.transferTotalSize;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
    else if ((dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE) ||
             (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION))
    {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        result = USB_DeviceMtpClose(g_mtp.file);
#else
        (void)USB_DeviceMtpClose(g_mtp.file);
#endif
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdSendObjInfo(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t storageID                         = dataInfo->param[0];
    uint32_t objHandle                         = dataInfo->param[1];
    uint8_t *writeBuf;
    usb_mtp_obj_handle_t objHandleStruct;
    uint32_t objFormat;
    uint32_t objSize;
    uint64_t freeBytes;
    usb_device_mtp_file_handle_t file;
    usb_device_mtp_file_info_t fno;
    uint32_t length;
    uint32_t length2;
    uint16_t stringLength;
    usb_status_t result;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    uint32_t offset = 0U;
#endif

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if (USB_DeviceGetRootPath(&g_mtp, storageID, g_mtp.path) == 0U)
        {
            /* invalid storage ID */
            dataInfo->code = MTP_RESPONSE_INVALID_STORAGE_ID;
            return;
        }

        if (objHandle != USB_DEVICE_MTP_MAX_UINT32_VAL)
        {
            if ((objHandle >= g_mtp.nextHandleID) || (objHandle == 0U) ||
                (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success))
            {
                /* invalid object handle */
                dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
                return;
            }

            if ((objHandleStruct.flag & MTP_OBJECT_DIR) == 0U)
            {
                /* invalid parent object */
                dataInfo->code = MTP_RESPONSE_INVALID_PARENT_OBJECT;
                return;
            }

            /* build path here. */
            if (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, g_mtp.path) != kStatus_USB_Success)
            {
                /* invalid object handle */
                dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
                return;
            }
        }
        else
        {
            /* root path */
        }

        dataInfo->curSize = USB_DEVICE_MTP_TRANSFER_BUFF_SIZE;
        dataInfo->buffer  = (uint8_t *)&g_mtpTransferBuffer[0];
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE)
    {
        writeBuf = (uint8_t *)&g_mtpTransferBuffer[0] + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;

        objFormat    = *(uint16_t *)(writeBuf + 4U); /* object format */
        objSize      = *(uint32_t *)(writeBuf + 8U); /* object compressed size */
        stringLength = *(uint8_t *)(writeBuf + 52U); /* the length of file name */

        if (stringLength == 0U)
        {
            dataInfo->code = MTP_RESPONSE_INVALID_DATASET;
            return;
        }

        if (USB_DeviceUnicodeStringLength((const uint16_t *)(writeBuf + 53U)) != ((stringLength - 1U) << 1U))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_DATASET;
            return;
        }

        USB_DeviceMtpGetDiskFreeBytes((const uint16_t *)&g_mtp.path[0], &freeBytes);

        if (freeBytes < objSize)
        {
            dataInfo->code = MTP_RESPONSE_OBJECT_TOO_LARGE;
            return;
        }

        if (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL)
        {
            objHandle = 0;
        }

        /* build path here(append file name to the path) */
        if (USB_DeviceAppendNameToPath((uint16_t *)g_mtp.path, (const uint16_t *)(writeBuf + 53U)) !=
            kStatus_USB_Success)
        {
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
            return;
        }

        if (objFormat == MTP_FORMAT_ASSOCIATION)
        {
            length2 = USB_DeviceCheckDirDepth((const uint16_t *)&g_mtp.path[0]);

            if (length2 > USB_DEVICE_MTP_DIR_INSTANCE)
            {
                usb_echo("The created directory is too deep, please increase the dir instance\r\n");
                result = kStatus_USB_Error;
            }
            else
            {
                result = USB_DeviceMtpMakeDir((const uint16_t *)&g_mtp.path[0]); /* Create directory */
            }

            if (result != kStatus_USB_Success)
            {
                dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
                return;
            }
        }
        else
        {
            result = USB_DeviceMtpOpen(
                &file, (const uint16_t *)&g_mtp.path[0],
                USB_DEVICE_MTP_READ | USB_DEVICE_MTP_WRITE | USB_DEVICE_MTP_CREATE_ALWAYS); /* Create file */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_USB_Success != USB_DeviceMtpClose(file))
            {
                return;
            }
#else
            (void)USB_DeviceMtpClose(file);
#endif

            if (result != kStatus_USB_Success)
            {
                dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
                return;
            }
        }

        length       = (stringLength << 1U) + 53U;         /* Date Created Offset */
        stringLength = *(uint8_t *)(writeBuf + length);    /* Date Created length */
        length       = (stringLength << 1U) + length + 1U; /* Date Modify Offset */

        if (*(uint8_t *)(writeBuf + length) != 0U)
        {
            USB_DeviceParseDateTime((const uint16_t *)(writeBuf + length + 1U), &g_mtp.timeStamp);
        }
        else
        {
            /* Get date and time. */
            USB_DeviceMtpFstat((const uint16_t *)&g_mtp.path[0], &fno);
            *(uint32_t *)(&g_mtp.timeStamp)        = fno.dateUnion.date;
            *((uint32_t *)(&g_mtp.timeStamp) + 1U) = fno.timeUnion.time;
        }

        /* assign object handle */
        objHandleStruct.handleID = g_mtp.nextHandleID;
        g_mtp.nextHandleID++;

        objHandleStruct.idUnion.parentID = objHandle;
        objHandleStruct.storageID        = storageID;
        objHandleStruct.size             = objSize;
        objHandleStruct.dateUnion.date   = *(uint32_t *)(&g_mtp.timeStamp);
        objHandleStruct.timeUnion.time   = *((uint32_t *)(&g_mtp.timeStamp) + 1U);
        objHandleStruct.flag             = 0U;
        if (objFormat == MTP_FORMAT_ASSOCIATION)
        {
            objHandleStruct.flag |= MTP_OBJECT_DIR;
            /* modify time stamp */
            USB_DeviceMtpUtime((const uint16_t *)&g_mtp.path[0], &g_mtp.timeStamp);
        }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        offset = USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)(writeBuf + 53U), 0U);
#else
        (void)USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)(writeBuf + 53U), 0U);
#endif

        result = USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct);

        dataInfo->code     = MTP_RESPONSE_OK;
        dataInfo->curSize  = 3U;
        dataInfo->param[0] = storageID;
        dataInfo->param[1] = objHandle;
        dataInfo->param[2] = objHandleStruct.handleID;

        g_mtp.validObjInfo      = 1;
        g_mtp.transferTotalSize = objSize + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdSendObj(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint8_t *writeBuf;
    uint32_t size;
    uint32_t sizeWritten;
    uint64_t freeBytes;
    usb_status_t result;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if (g_mtp.validObjInfo == 0U)
        {
            dataInfo->code = MTP_RESPONSE_NO_VALID_OBJECT_INFO;
            return;
        }

        dataInfo->curSize = USB_DEVICE_MTP_TRANSFER_BUFF_SIZE;
        dataInfo->buffer  = (uint8_t *)&g_mtpTransferBuffer[0];

        g_mtp.transferDoneSize = 0;

        result = USB_DeviceMtpOpen(&g_mtp.file, (const uint16_t *)&g_mtp.path[0],
                                   USB_DEVICE_MTP_READ | USB_DEVICE_MTP_WRITE);
    }
    else if ((dataInfo->curPhase == USB_DEVICE_MTP_PHASE_DATA) || (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE))
    {
        /* Special process for receiving >4GB object. */
        if (dataInfo->totalSize == USB_DEVICE_MTP_MAX_UINT64_VAL)
        {
            g_mtp.transferTotalSize = USB_DEVICE_MTP_MAX_UINT64_VAL;

            USB_DeviceMtpGetDiskFreeBytes((const uint16_t *)&g_mtp.path[0], &freeBytes);

            /* ensure at least 4GB space for this object. */
            if (freeBytes < USB_DEVICE_MTP_MAX_UINT32_VAL)
            {
                dataInfo->code = MTP_RESPONSE_OBJECT_TOO_LARGE;
                return;
            }
        }

        if (g_mtp.transferTotalSize < dataInfo->curPos)
        {
            dataInfo->code = MTP_RESPONSE_STORAGE_FULL;
            return;
        }

        /* here 512 byte aligned processing is to improve performance. */
        if (g_mtp.transferDoneSize == 0U)
        {
            if (dataInfo->curPhase != USB_DEVICE_MTP_PHASE_RESPONSE)
            {
                /* first packet in the data phase */
                size = dataInfo->curPos - MTP_BLOCK_SIZE;
            }
            else
            {
                /* both first and last packet */
                size = dataInfo->curPos - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
            }
        }
        else
        {
            if (dataInfo->curPhase != USB_DEVICE_MTP_PHASE_RESPONSE)
            {
                /* intermediate packet */
                size = dataInfo->curPos - g_mtp.transferDoneSize;
            }
            else
            {
                /* last packet */
                size = dataInfo->curPos - g_mtp.transferDoneSize + MTP_BLOCK_SIZE -
                       USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
            }
            /* g_mtp.transferDoneSize = g_mtp.transferDoneSize - MTP_BLOCK_SIZE; */
        }

        writeBuf = (uint8_t *)&g_mtpTransferBuffer[0] + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;

        /* result = USB_DeviceMtpLseek(g_mtp.file, g_mtp.transferDoneSize); */
        result = USB_DeviceMtpWrite(g_mtp.file, writeBuf, size, &sizeWritten);

        if (dataInfo->curPhase != USB_DEVICE_MTP_PHASE_RESPONSE)
        {
            (void)memcpy(writeBuf, writeBuf + size, MTP_BLOCK_SIZE - USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH);
        }

        if ((result != kStatus_USB_Success) || (sizeWritten < size))
        {
            dataInfo->code = MTP_RESPONSE_INCOMPLETE_TRANSFER;
            return;
        }
        else
        {
            dataInfo->curSize = USB_DEVICE_MTP_TRANSFER_BUFF_SIZE - MTP_BLOCK_SIZE;
            dataInfo->buffer  = (uint8_t *)&g_mtpTransferBuffer[0] + MTP_BLOCK_SIZE;
        }

        g_mtp.transferDoneSize = dataInfo->curPos;

        if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE)
        {
            g_mtp.validObjInfo = 0;
            dataInfo->code     = MTP_RESPONSE_OK;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            result = USB_DeviceMtpClose(g_mtp.file);
#else
            (void)USB_DeviceMtpClose(g_mtp.file);
#endif

            /* modify time stamp */
            USB_DeviceMtpUtime((const uint16_t *)&g_mtp.path[0], &g_mtp.timeStamp);
        }
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION)
    {
        g_mtp.validObjInfo = 0;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        result = USB_DeviceMtpClose(g_mtp.file);
#else
        (void)USB_DeviceMtpClose(g_mtp.file);
#endif
        result = USB_DeviceMtpUnlink((const uint16_t *)&g_mtp.path[0]);
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdDeleteObj(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t objHandle                         = dataInfo->param[0];
    usb_mtp_obj_handle_t objHandleStruct;
    usb_status_t result;
    uint32_t i;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION)
    {
        return;
    }

    dataInfo->curSize = 0; /* the number of the response parameter */

    if (objHandle != USB_DEVICE_MTP_MAX_UINT32_VAL)
    {
        /* build path here */
        if ((objHandle >= g_mtp.nextHandleID) || (objHandle == 0U) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
            (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, g_mtp.path) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandleStruct.flag & MTP_OBJECT_DIR) != 0U)
        {
            result = USB_DeviceDeleteDir(&g_mtp.path[0]);
        }
        else
        {
            result = USB_DeviceMtpUnlink((const uint16_t *)&g_mtp.path[0]);
        }

        if (result == kStatus_USB_Success)
        {
            objHandleStruct.flag |= MTP_OBJECT_DELETE;

            result = USB_DeviceMtpObjHandleWrite(objHandle, &objHandleStruct);

            dataInfo->code = MTP_RESPONSE_OK;
        }
        else
        {
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
        }
    }
    else
    {
        result = kStatus_USB_Error;
        /* root path */
        for (i = 0; i < g_mtp.storageList->storageCount; i++)
        {
            result = USB_DeviceDeleteDir(g_mtp.storageList->storageInfo[i].rootPath);
            if (result != kStatus_USB_Success)
            {
                break;
            }
        }

        if (result == kStatus_USB_Success)
        {
            dataInfo->code = MTP_RESPONSE_OK;
        }
        else
        {
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
        }
    }
}

void USB_DeviceCmdGetDevicePropVal(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint16_t devPropCode                       = dataInfo->param[0];
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t i;
    usb_device_mtp_dev_prop_desc_t *devPropDesc;
    uint32_t count;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        devPropDesc = g_mtp.devPropDescList->devPropDesc;
        count       = g_mtp.devPropDescList->devPropDescCount;

        for (i = 0; i < count; i++)
        {
            if (devPropDesc[i].devPropCode == devPropCode)
            {
                offset = USB_DeviceAssignDevPropVal(devPropDesc[i].devPropCode, devPropDesc[i].dataType, readBuf,
                                                    &devPropDesc[i].currentVal, offset);
                break;
            }
        }

        if (i == count)
        {
            /* not support device property */
            dataInfo->code = MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
        }
        else
        {
            dataInfo->totalSize = offset;
            dataInfo->curSize   = offset;
            dataInfo->buffer    = readBuf;
        }
    }
}

void USB_DeviceCmdSetDevicePropVal(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint16_t devPropCode                       = dataInfo->param[0];
    uint8_t *writeBuf;
    uint32_t i;
    uint8_t size;
    usb_device_mtp_dev_prop_desc_t *devPropDesc;
    uint32_t count;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        devPropDesc = g_mtp.devPropDescList->devPropDesc;
        count       = g_mtp.devPropDescList->devPropDescCount;

        for (i = 0; i < count; i++)
        {
            if (devPropDesc[i].devPropCode == devPropCode)
            {
                break;
            }
        }

        if (i == count)
        {
            /* not support device property */
            dataInfo->code = MTP_RESPONSE_DEVICE_PROP_NOT_SUPPORTED;
            return;
        }

        if (devPropDesc[i].getSet == 0U)
        {
            /* only get, cannot set, return access denied */
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
            return;
        }

        dataInfo->curSize = USB_DEVICE_MTP_TRANSFER_BUFF_SIZE;
        dataInfo->buffer  = (uint8_t *)&g_mtpTransferBuffer[0];
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE)
    {
        writeBuf = (uint8_t *)&g_mtpTransferBuffer[0] + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;

        switch (devPropCode)
        {
            case MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME:
            {
                size = USB_DeviceUnicodeStringLength((const uint16_t *)(writeBuf + 1U));
                size += 2U;

                if ((size >> 1U) != *(uint8_t *)writeBuf)
                {
                    dataInfo->code = MTP_RESPONSE_INVALID_DEVICE_PROP_VALUE;
                    return;
                }

                if (size > MTP_DEVICE_FRIENDLY_NAME_LEN)
                {
                    size = MTP_DEVICE_FRIENDLY_NAME_LEN;
                }

                (void)memcpy(g_mtp.devFriendlyName, (const uint16_t *)(writeBuf + 1U), size);

                g_mtp.devFriendlyName[MTP_DEVICE_FRIENDLY_NAME_LEN - 1U] = '\0';
                g_mtp.devFriendlyName[MTP_DEVICE_FRIENDLY_NAME_LEN - 2U] = '\0';
                break;
            }

            default:
                /* TODO: identify more device property code. */
                break;
        }

        dataInfo->code = MTP_RESPONSE_OK;
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdGetObjPropVal(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint32_t objHandle                         = dataInfo->param[0];
    uint16_t objPropCode                       = dataInfo->param[1];
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    uint32_t i;
    uint32_t j;
    usb_mtp_obj_handle_t objHandleStruct;
    usb_device_mtp_obj_prop_t *prop;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        prop = g_mtp.objPropList->objProp;
        j    = 0U;
        for (i = 0U; i < g_mtp.objPropList->objPropCount; i++)
        {
            if (prop[i].objFormat == USB_DeviceIdentifyFileType(&objHandleStruct))
            {
                for (; j < prop[i].objPropDescCount; j++)
                {
                    if (prop[i].objPropDesc[j].objPropCode == objPropCode)
                    {
                        break;
                    }
                }
                break;
            }
        }

        if ((i == g_mtp.objPropList->objPropCount) || (j == prop[i].objPropDescCount))
        {
            /* invalid object property code */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_PROP_CODE;
            return;
        }

        offset = USB_DeviceAssignObjPropVal(objPropCode, prop[i].objPropDesc[j].dataType, readBuf, &objHandleStruct,
                                            offset); /* object value */

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdSetObjPropVal(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t objHandle                         = dataInfo->param[0];
    uint16_t objPropCode                       = dataInfo->param[1];
    uint8_t *writeBuf;
    uint32_t i;
    uint32_t j;
    uint32_t size;
    usb_mtp_obj_handle_t objHandleStruct;
    uint16_t renameBuffer[MTP_PATH_MAX_LEN >> 1U];
    usb_status_t result;
    usb_device_mtp_obj_prop_t *prop;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
            (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, g_mtp.path) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        prop = g_mtp.objPropList->objProp;
        j    = 0U;
        for (i = 0U; i < g_mtp.objPropList->objPropCount; i++)
        {
            if (prop[i].objFormat == USB_DeviceIdentifyFileType(&objHandleStruct))
            {
                for (; j < prop[i].objPropDescCount; j++)
                {
                    if (prop[i].objPropDesc[j].objPropCode == objPropCode)
                    {
                        break;
                    }
                }
                break;
            }
        }

        if ((i == g_mtp.objPropList->objPropCount) || (j == prop[i].objPropDescCount))
        {
            /* invalid object property code */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_PROP_CODE;
            return;
        }

        if (prop[i].objPropDesc[j].getSet == 0U)
        {
            /* only get, cannot set, return access denied */
            dataInfo->code = MTP_RESPONSE_ACCESS_DENIED;
            return;
        }

        dataInfo->curSize = USB_DEVICE_MTP_TRANSFER_BUFF_SIZE;
        dataInfo->buffer  = (uint8_t *)&g_mtpTransferBuffer[0];
    }
    else if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_RESPONSE)
    {
        writeBuf = (uint8_t *)&g_mtpTransferBuffer[0] + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;

        switch (objPropCode)
        {
            case MTP_OBJECT_PROPERTY_OBJECT_FILE_NAME:
            {
                size = USB_DeviceUnicodeStringLength((const uint16_t *)(writeBuf + 1U));
                size = (size + 2U) >> 1U;

                if (size != *(uint8_t *)writeBuf)
                {
                    dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_PROP_VALUE;
                    return;
                }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                if (kStatus_USB_Success != USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct))
                {
                    return;
                }
#else
                (void)USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct);
#endif

                /* build rename buffer */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                /* j indicates the offset */
                j = USB_DeviceMemCopyUnicodeString(&renameBuffer[0], (const uint16_t *)&g_mtp.path[0], 0);
#else
                (void)USB_DeviceMemCopyUnicodeString(&renameBuffer[0], (const uint16_t *)&g_mtp.path[0], 0);
#endif
                size = USB_DeviceUnicodeStringLength((const uint16_t *)&g_mtp.path[0]);
                size >>= 1U;
                while (renameBuffer[size] != '/')
                {
                    size--;
                };
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                /* j indicates the offset */
                j = USB_DeviceMemCopyUnicodeString(&renameBuffer[size + 1U], (const uint16_t *)(writeBuf + 1U), 0);
#else
                (void)USB_DeviceMemCopyUnicodeString(&renameBuffer[size + 1U], (const uint16_t *)(writeBuf + 1U), 0);
#endif

                result = USB_DeviceMtpRename((const uint16_t *)&g_mtp.path[0], (const uint16_t *)&renameBuffer[0]);

                if (result != kStatus_USB_Success)
                {
                    dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_PROP_VALUE;
                    return;
                }
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
                /* j indicates the offset */
                j = USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)(writeBuf + 1U), 0);
#else
                (void)USB_DeviceMemCopyUnicodeString(&objHandleStruct.name[0], (const uint16_t *)(writeBuf + 1U), 0);
#endif

                result = USB_DeviceMtpObjHandleWrite(objHandle, &objHandleStruct);

                (void)result;
                break;
            }

            default:
                /* TODO: identify more object property code. */
                break;
        }
    }
    else
    {
        /* no action */
    }
}

void USB_DeviceCmdGetObjReferences(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t offset                            = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    uint32_t objHandle                         = dataInfo->param[0];
    uint8_t *readBuf                           = (uint8_t *)&g_mtpTransferBuffer[0];
    usb_mtp_obj_handle_t objHandleStruct;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
    {
        if ((objHandle == 0U) || (objHandle == USB_DEVICE_MTP_MAX_UINT32_VAL))
        {
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        if ((objHandle >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success))
        {
            /* invalid object handle */
            dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
            return;
        }

        offset = USB_DeviceMemCopyDword(readBuf, 0, offset);

        dataInfo->totalSize = offset;
        dataInfo->curSize   = offset;
        dataInfo->buffer    = readBuf;
    }
}

void USB_DeviceCmdMoveObj(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t objHandle                         = dataInfo->param[0];
    uint32_t storageID                         = dataInfo->param[1];
    uint32_t newParentObj                      = dataInfo->param[2];
    usb_mtp_obj_handle_t objHandleStruct;
    uint16_t destPath[MTP_PATH_MAX_LEN >> 1U];
    usb_status_t result;
    uint8_t i;

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION)
    {
        return;
    }

    for (i = 0; i < g_mtp.storageList->storageCount; i++)
    {
        if (g_mtp.storageList->storageInfo[i].storageID == storageID)
        {
            break;
        }
    }

    if (i == g_mtp.storageList->storageCount)
    {
        /* invalid storage ID */
        dataInfo->code = MTP_RESPONSE_INVALID_STORAGE_ID;
        return;
    }

    /* build destination path */
    if (newParentObj == 0U)
    {
        USB_DeviceGetRootPath(&g_mtp, storageID, (uint8_t *)&destPath[0]);
    }
    else
    {
        if ((newParentObj >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, newParentObj, &objHandleStruct) != kStatus_USB_Success) ||
            ((objHandleStruct.flag & MTP_OBJECT_DIR) == 0U) ||
            (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, newParentObj, (uint8_t *)&destPath[0]) !=
             kStatus_USB_Success))
        {
            /* invalid parent object */
            dataInfo->code = MTP_RESPONSE_INVALID_PARENT_OBJECT;
            return;
        }
    }

    /* build source path */
    if ((objHandle >= g_mtp.nextHandleID) || (objHandle == 0U) ||
        (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
        (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, g_mtp.path) != kStatus_USB_Success))
    {
        /* invalid object handle */
        dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
        return;
    }

    /* append file name to the path */
    result = USB_DeviceAppendNameToPath(&destPath[0], (const uint16_t *)&objHandleStruct.name[0]);

    /* Move obejct */
    if (result == kStatus_USB_Success)
    {
        result = USB_DeviceMtpRename((uint16_t *)g_mtp.path, (uint16_t *)&destPath[0]);
    }

    if (result != kStatus_USB_Success)
    {
        dataInfo->code = MTP_RESPONSE_GENERAL_ERROR;
    }
    else
    {
        /* replace parent handle with new parent handle */
        objHandleStruct.idUnion.parentID = newParentObj;
        USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct);
        dataInfo->code = MTP_RESPONSE_OK;
    }
}

void USB_DeviceCmdCopyObj(void *param)
{
    usb_device_mtp_cmd_data_struct_t *dataInfo = (usb_device_mtp_cmd_data_struct_t *)param;
    uint32_t objHandle                         = dataInfo->param[0];
    uint32_t storageID                         = dataInfo->param[1];
    uint32_t newParentObj                      = dataInfo->param[2];
    usb_mtp_obj_handle_t objHandleStruct;
    uint16_t destPath[MTP_PATH_MAX_LEN >> 1U];
    uint16_t srcPath[MTP_PATH_MAX_LEN >> 1U];
    usb_device_mtp_file_info_t fno;
    usb_status_t result;
    uint8_t i;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    uint32_t offset = 0U;
#endif

    if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION)
    {
        return;
    }

    dataInfo->curSize = 0; /* the number of the response parameter */

    for (i = 0; i < g_mtp.storageList->storageCount; i++)
    {
        if (g_mtp.storageList->storageInfo[i].storageID == storageID)
        {
            break;
        }
    }

    if (i == g_mtp.storageList->storageCount)
    {
        /* invalid storage ID */
        dataInfo->code = MTP_RESPONSE_INVALID_STORAGE_ID;
        return;
    }

    /* build destination path */
    if (newParentObj == 0U)
    {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        offset = USB_DeviceGetRootPath(&g_mtp, storageID, (uint8_t *)&destPath[0]);
#else
        (void)USB_DeviceGetRootPath(&g_mtp, storageID, (uint8_t *)&destPath[0]);
#endif
    }
    else
    {
        if ((newParentObj >= g_mtp.nextHandleID) ||
            (USB_DeviceGetObjHandleInfo(&g_mtp, newParentObj, &objHandleStruct) != kStatus_USB_Success) ||
            ((objHandleStruct.flag & MTP_OBJECT_DIR) == 0U) ||
            (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, newParentObj, (uint8_t *)&destPath[0]) !=
             kStatus_USB_Success))
        {
            /* invalid parent object */
            dataInfo->code = MTP_RESPONSE_INVALID_PARENT_OBJECT;
            return;
        }
    }

    /* build source path */
    if ((objHandle >= g_mtp.nextHandleID) || (objHandle == 0U) ||
        (USB_DeviceGetObjHandleInfo(&g_mtp, objHandle, &objHandleStruct) != kStatus_USB_Success) ||
        (USB_DeviceBuildPath(&g_mtp, objHandleStruct.storageID, objHandle, (uint8_t *)&srcPath[0]) !=
         kStatus_USB_Success))
    {
        /* invalid object handle */
        dataInfo->code = MTP_RESPONSE_INVALID_OBJECT_HANDLE;
        return;
    }

    if ((objHandleStruct.flag & MTP_OBJECT_DIR) != 0U)
    {
        result = USB_DeviceCopyDir((uint8_t *)destPath, (uint8_t *)&srcPath[0]);

        if (result == kStatus_USB_Success)
        {
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_USB_Success !=
                USB_DeviceAppendNameToPath(&destPath[0], (const uint16_t *)&objHandleStruct.name[0]))
            {
                return;
            }
#else
            (void)USB_DeviceAppendNameToPath(&destPath[0], (const uint16_t *)&objHandleStruct.name[0]);
#endif
        }
    }
    else
    {
        /* append file name to the path */
        result = USB_DeviceAppendNameToPath(&destPath[0], (const uint16_t *)&objHandleStruct.name[0]);

        if (result == kStatus_USB_Success)
        {
            result = USB_DeviceCopyFile((uint8_t *)destPath, (uint8_t *)&srcPath[0]);
        }
    }

    if (result != kStatus_USB_Success)
    {
        dataInfo->code = MTP_RESPONSE_GENERAL_ERROR;
    }
    else
    {
        /* get date and time */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if (kStatus_USB_Success != USB_DeviceMtpFstat((const uint16_t *)&destPath[0], &fno))
        {
            return;
        }
#else
        (void)USB_DeviceMtpFstat((const uint16_t *)&destPath[0], &fno);
#endif

        objHandleStruct.handleID = g_mtp.nextHandleID;
        g_mtp.nextHandleID++;

        objHandleStruct.idUnion.parentID = newParentObj;
        objHandleStruct.storageID        = storageID;
        objHandleStruct.dateUnion.date   = fno.dateUnion.date;
        objHandleStruct.timeUnion.time   = fno.timeUnion.time;
        objHandleStruct.flag &= ~MTP_OBJECT_ALREADY_GET;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
        if (kStatus_USB_Success != USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct))
        {
            return;
        }
#else
        (void)USB_DeviceMtpObjHandleWrite(objHandleStruct.handleID, &objHandleStruct);
#endif

        dataInfo->code     = MTP_RESPONSE_OK;
        dataInfo->curSize  = 1;
        dataInfo->param[0] = objHandleStruct.handleID;
    }
}
