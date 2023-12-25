/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "usb_host_msd.h"
#include "host_msd_fatfs.h"
#include "fsl_common.h"
#include "board.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#include "usb_support.h"
#include "wm_os.h"
#include "ff.h"

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static FATFS fatfs;
FIL file;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host callback function.
 *
 * device attach/detach callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);

/*!
 * @brief app initialization.
 */
static void USB_HostApplicationInit(void);

static void USB_HostTask(void *param);

static void USB_HostApplicationTask(void *param);

extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);
extern void USB_HostTaskFn(void *param);
void BOARD_InitHardware(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif
/*! @brief USB host msd fatfs instance global variable */
extern usb_host_msd_fatfs_instance_t g_MsdFatfsInstance;
usb_host_handle g_HostHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief USB isr function.
 */

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventNotSupported:
            usb_echo("device not supported.\r\n");
            break;

        case kUSB_HostEventEnumerationDone:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventDetach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}

static void USB_HostApplicationInit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostClockInit();

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit(CONTROLLER_ID, &g_HostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host init error\r\n");
        return;
    }
    USB_HostIsrEnable();

    usb_echo("host init done\r\n");
}

static void USB_HostTask(void *param)
{
    while (1)
    {
        USB_HostTaskFn(param);
        os_thread_sleep(os_msec_to_ticks(1));
    }
}

static void USB_HostApplicationTask(void *param)
{
    while (1)
    {
        USB_HostMsdTask(param);
        os_thread_sleep(os_msec_to_ticks(1));
    }
}

FATFS *fs;
uint32_t freeClusterNumber;
uint8_t driverNumberBuffer[3];
#define USBDISK 1 /* usb disk to physical drive 1 */

int usb_mount()
{
    FRESULT fatfsCode;

    /* time delay */
    for (freeClusterNumber = 0; freeClusterNumber < 10000; ++freeClusterNumber)
    {
        __NOP();
    }

    usb_echo("............................fatfs test.....................\r\n");

    usb_echo("fatfs mount as logical drive %d......", USBDISK);
    sprintf((char *)&driverNumberBuffer[0], "%c:", USBDISK + '0');
    fatfsCode = f_mount(&fatfs, (char const *)&driverNumberBuffer[0], 0);
    if (fatfsCode)
    {
        usb_echo("error\r\n");
        return -WM_FAIL;
    }
    usb_echo("success\r\n");
    for (freeClusterNumber = 0; freeClusterNumber < 10000; ++freeClusterNumber)
    {
        __NOP();
    }
#if (FF_FS_RPATH >= 2)
    fatfsCode = f_chdrive((char const *)&driverNumberBuffer[0]);
    if (fatfsCode)
    {
        usb_echo("chdrive error\r\n");
        return -WM_FAIL;
    }
    usb_echo("chdrive success\r\n");
#endif
    return WM_SUCCESS;
}

int usb_file_open(char *test_file_name)
{
    FRESULT fatfsCode;
    fatfsCode = f_unlink(test_file_name); /* delete the file if it is existed */
    if ((fatfsCode != FR_OK) && (fatfsCode != FR_NO_FILE))
    {
        usb_echo("fatfs file delete error\r\n");
        return -WM_FAIL;
    }
    fatfsCode = f_open(&file, test_file_name, FA_WRITE | FA_READ | FA_CREATE_ALWAYS); /* create one new file */
    if (fatfsCode)
    {
        usb_echo("fatfs file opening error\r\n");
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}

int usb_file_open_by_mode(char *test_file_name, uint8_t mode)
{
    FRESULT fatfsCode;
    fatfsCode = f_open(&file, test_file_name, mode);
    if (fatfsCode)
    {
        usb_echo("fatfs file opening error\r\n");
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}

int usb_file_lseek(size_t lseek_size)
{
    FRESULT fatfsCode;
    fatfsCode = f_lseek(&file, lseek_size);
    if (fatfsCode)
    {
        PRINTF("lseek error\r\n");
        f_close(&file);
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}

int usb_file_write(uint8_t *data, size_t data_len)
{
    unsigned int resultSize;
    FRESULT fatfsCode;
    fatfsCode = f_write(&file, data, data_len, &resultSize);
    if (fatfsCode)
    {
        PRINTF("write error\r\n");
        f_close(&file);
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}

int usb_file_read(uint8_t *data, size_t data_len)
{
    unsigned int resultSize;
    FRESULT fatfsCode;

    fatfsCode = f_read(&file, data, data_len, &resultSize);
    if (fatfsCode)
    {
        PRINTF("read error\r\n");
        f_close(&file);
        return -WM_FAIL;
    }
    return resultSize;
}

int usb_file_size(void)
{
    return f_size(&file);
}

int usb_file_close()
{
    FRESULT fatfsCode;
    fatfsCode = f_close(&file);
    if (fatfsCode)
    {
        PRINTF("file close error\r\n");
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

void usb_init(void)
{
    USB_HostApplicationInit();

    if (xTaskCreate(USB_HostTask, "usb host task", 2000L / sizeof(portSTACK_TYPE), g_HostHandle, 4, NULL) != pdPASS)
    {
        usb_echo("create host task error\r\n");
    }
    if (xTaskCreate(USB_HostApplicationTask, "app task", 2300L / sizeof(portSTACK_TYPE), &g_MsdFatfsInstance, 3,
                    NULL) != pdPASS)
    {
        PRINTF("create mouse task error\r\n");
    }
}
