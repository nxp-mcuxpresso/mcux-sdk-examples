/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb.h"
#include "ff.h"
#include "diskio.h"
#include "mtp_file_system_adapter.h"
#include "mtp_object_handle.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FIL s_File;

/* 2-byte unicode, the file created when session is opened is used to save object handle lists. */
USB_RAM_ADDRESS_ALIGNMENT(2U)
const uint8_t g_ObjHandlePath[] = {
#if defined(SD_DISK_ENABLE)
    SDDISK + '0',
#elif defined(MMC_DISK_ENABLE)
    MMCDISK + '0',
#else
    '0',
#endif
    0x00U,        ':',   0x00U, '/',   0x00U, 'm',   0x00U, 't',   0x00U, 'p',   0x00U, '_',   0x00U, 'o',   0x00U,
    'b',          0x00U, 'j',   0x00U, '.',   0x00U, 't',   0x00U, 'x',   0x00U, 't',   0x00U, 0x00U, 0x00U,
};

/*******************************************************************************
 * Code
 ******************************************************************************/

usb_status_t USB_DeviceMtpObjHandleInit(void)
{
    FRESULT result;

    (void)f_close(&s_File);

    (void)f_unlink((const TCHAR *)&g_ObjHandlePath[0]);

    result = f_open(&s_File, (const TCHAR *)&g_ObjHandlePath[0], FA_READ | FA_WRITE | FA_CREATE_ALWAYS);

    (void)f_chmod((const TCHAR *)&g_ObjHandlePath[0], AM_SYS | AM_HID, AM_SYS | AM_HID);

    if (result != FR_OK)
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleDeinit(void)
{
    FRESULT result;

    (void)f_close(&s_File);

    result = f_unlink((const TCHAR *)&g_ObjHandlePath[0]);

    if (result != FR_OK)
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleRead(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct)
{
    FRESULT result;
    uint32_t size;

    result = f_lseek(&s_File, (objHandle - 1U) * sizeof(usb_mtp_obj_handle_t));

    if (result == FR_OK)
    {
        result = f_read(&s_File, objHandleStruct, sizeof(usb_mtp_obj_handle_t), (UINT *)&size);
    }

    if ((result != FR_OK) || (size < sizeof(usb_mtp_obj_handle_t)))
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}

usb_status_t USB_DeviceMtpObjHandleWrite(uint32_t objHandle, usb_mtp_obj_handle_t *objHandleStruct)
{
    FRESULT result;
    uint32_t size;

    result = f_lseek(&s_File, (objHandle - 1U) * sizeof(usb_mtp_obj_handle_t));

    if (result == FR_OK)
    {
        result = f_write(&s_File, objHandleStruct, sizeof(usb_mtp_obj_handle_t), (UINT *)&size);
    }

    if ((result != FR_OK) || (size < sizeof(usb_mtp_obj_handle_t)))
    {
        return kStatus_USB_Error;
    }

    return kStatus_USB_Success;
}
