/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018, 2020 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_msd.h"
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT))
#include "usb_phy.h"
#endif
#include "host_msd_fatfs.h"
#include "ff.h"
#include "diskio.h"
#include "stdio.h"
#include "fsl_device_registers.h"

#include "fsl_debug_console.h"

#ifndef CONFIG_BT_DEBUG_HOST_MSD_FATFS
#define CONFIG_BT_DEBUG_HOST_MSD_FATFS 0
#endif

#define LOG_ENABLE CONFIG_BT_DEBUG_HOST_MSD_FATFS
#define LOG_MODULE_NAME bt_host_msd_fatfs
#include "fsl_component_log.h"
LOG_MODULE_DEFINE(LOG_MODULE_NAME, kLOG_LevelTrace);

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))
#include "fsl_device_registers.h"
#define THROUGHPUT_BUFFER_SIZE (64U * 1024U) /* throughput test buffer */
#define MCU_CORE_CLOCK         (120000000U) /* mcu core clock, user need to configure it. */
#endif                                     /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */

#define MSD_ATTCH_TIMEOUT 1000U
#define MSD_ENUM_TIMEOUT  5000U

#define EVENT_BIT_MAP_ATTACHED        0x01U
#define EVENT_BIT_MAP_ENUMRATION_DONE 0x02U

/* Weak function. */
#if defined(__GNUC__)
#define __WEAK_FUNC __attribute__((weak))
#elif defined(__ICCARM__)
#define __WEAK_FUNC __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __WEAK_FUNC __attribute__((weak))
#endif

/*! @brief host app device attach/detach status */
typedef enum _usb_host_app_state
{
    kStatus_DEV_Idle = 0, /*!< there is no device attach/detach */
    kStatus_DEV_Attached, /*!< device is attached */
    kStatus_DEV_Detached, /*!< device is detached */
} usb_host_app_state_t;

/*! @brief 0 - execute normal fatfs test code; 1 - execute throughput test code */
#define MSD_FATFS_THROUGHPUT_TEST_ENABLE (0U)

/*! @brief host app run status */
typedef enum _usb_host_msd_run_state
{
    kUSB_HostMsdRunIdle = 0,         /*!< idle */
    kUSB_HostMsdRunSetInterface,     /*!< execute set interface code */
    kUSB_HostMsdRunWaitSetInterface, /*!< wait set interface done */
    kUSB_HostMsdRunMassStorageMount  /*!< execute mass storage test code */
} usb_host_msd_run_state_t;

/*! @brief USB host msd fatfs instance structure */
typedef struct _usb_host_msd_fatfs_instance
{
    usb_host_configuration_handle configHandle; /*!< configuration handle */
    usb_device_handle deviceHandle;             /*!< device handle */
    usb_host_class_handle classHandle;          /*!< class handle */
    usb_host_interface_handle interfaceHandle;  /*!< interface handle */
    usb_host_app_state_t   prevDeviceState;                    /*!< device attach/detach previous status */
    usb_host_app_state_t      deviceState;                        /*!< device attach/detach status */
    usb_host_msd_run_state_t runWaitState; /*!< application wait status, go to next run status when the wait status success */
    usb_host_msd_run_state_t runState;     /*!< application run status */
} usb_host_msd_fatfs_instance_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host msd control transfer callback.
 *
 * This function is used as callback function for control transfer .
 *
 * @param param      the host msd fatfs instance pointer.
 * @param data       data buffer pointer.
 * @param dataLength data length.
 * @status           transfer result status.
 */
static void USB_HostMsdAppControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status);

#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))
/*!
 * @brief msd fatfs test code execute done.
 */
static void USB_HostMsdFatfsTestDone(void);
#endif

#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))
/*!
 * @brief host msd fatfs throughput test.
 *
 * @param msdFatfsInstance   the host fatfs instance pointer.
 */
static void USB_HostMsdFatfsThroughputTest(usb_host_msd_fatfs_instance_t *msdFatfsInstance);

#else

/*!
 * @brief display file information.
 */
static void USB_HostMsdFatfsDisplayFileInfo(FILINFO *fileInfo);

/*!
 * @brief list files and sub-directory in one directory, the function don't check all sub-directories recursively.
 */
static FRESULT USB_HostMsdFatfsListDirectory(const TCHAR *path);

/*!
 * @brief forward function pointer for fatfs f_forward function.
 *
 * @param data_ptr   forward data pointer.
 * @param dataLength data length.
 */
#if ((defined _USE_FORWARD) && (_USE_FORWARD))&& ((defined _FS_TINY) && (_FS_TINY))
static uint32_t USB_HostMsdFatfsForward(const uint8_t *data_ptr, uint32_t dataLength);
#endif

/*!
 * @brief host msd fatfs test.
 *
 * This function implements msd fatfs test.
 *
 * @param msdFatfsInstance   the host fatfs instance pointer.
 */
static void USB_HostMsdFatfsMount(usb_host_msd_fatfs_instance_t *msdFatfsInstance);

#endif /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */

#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
extern usb_status_t USB_HostTestModeInit(usb_device_handle deviceHandle);
#endif

__WEAK_FUNC void USB_HostClockInit(void);
__WEAK_FUNC void USB_HostIsrEnable(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static EventGroupHandle_t s_msdAttachedEvent;

static usb_host_handle g_HostMsdFatfsHandle;

/*! @brief msd class handle array for fatfs */
extern usb_host_class_handle g_UsbFatfsClassHandle;

static usb_host_msd_fatfs_instance_t g_MsdFatfsInstance; /* global msd fatfs instance */
static FATFS fatfs;
/* control transfer on-going state. It should set to 1 when start control transfer, it is set to 0 in the callback */
static volatile uint8_t controlIng;
/* control transfer callback status */
static volatile usb_status_t controlStatus;

#if MSD_FATFS_THROUGHPUT_TEST_ENABLE
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint32_t testThroughputBuffer[THROUGHPUT_BUFFER_SIZE / 4]; /* the buffer for throughput test */
uint32_t testSizeArray[] = {20 * 1024, 20 * 1024};                /* test time and test size (uint: K)*/
#else
#if FF_USE_MKFS
/*fix build warning: declared but never used.*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t testBuffer[(FF_MAX_SS > 256) ? FF_MAX_SS : 256]; /* normal test buffer */
#endif
#endif /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */

/*******************************************************************************
 * Code
 ******************************************************************************/

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
    usb_host_configuration_t *configuration;
    usb_status_t status1;
    usb_status_t status2;
    uint8_t interfaceIndex = 0;
#endif
    usb_status_t status = kStatus_USB_Success;
    usb_host_event_t usb_event = (usb_host_event_t)(uint8_t)((uint8_t)eventCode & 0xFFU);

    switch (usb_event)
    {
        case kUSB_HostEventAttach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 == kStatus_USB_NotSupported) && (status2 == kStatus_USB_NotSupported))
            {
                status = kStatus_USB_NotSupported;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventNotSupported:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                if (((usb_descriptor_interface_t *)configuration->interfaceList[interfaceIndex].interfaceDesc)
                        ->bInterfaceClass == 9U) /* 9U is hub class code */
                {
                    break;
                }
            }

            if (interfaceIndex < configuration->interfaceCount)
            {
                LOG_ERR("unsupported hub\r\n");
            }
            else
            {
                LOG_ERR("Unsupported Device\r\n");
            }
#else
            LOG_ERR("Unsupported Device\r\n");
#endif
            break;

        case kUSB_HostEventEnumerationDone:
            LOG_DBG("New USB MSD disk arrived\r\n");
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventDetach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventEnumerationFail:
            LOG_ERR("Enumeration failed\r\n");
            break;

        default:
        	   /* Misra */
            break;
    }
    return status;
}

static void USB_HostMsdAppControlCallback(void *param, uint8_t *data, uint32_t dataLength, usb_status_t status)
{
    usb_host_msd_fatfs_instance_t *msdFatfsInstance = (usb_host_msd_fatfs_instance_t *)param;

    if (msdFatfsInstance->runWaitState == kUSB_HostMsdRunWaitSetInterface) /* set interface finish */
    {
        msdFatfsInstance->runWaitState = kUSB_HostMsdRunIdle;
        msdFatfsInstance->runState     = kUSB_HostMsdRunMassStorageMount;
    }
    controlIng    = 0;
    controlStatus = status;
}

#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))

static void USB_HostMsdFatfsTestDone(void)
{
    LOG_DBG("............................test done......................\r\n");
}

static void USB_HostMsdFatfsThroughputTest(usb_host_msd_fatfs_instance_t *msdFatfsInstance)
{
    uint64_t totalTime;
    FRESULT fatfsCode;
    FIL file;
    uint32_t resultSize;
    uint32_t testSize;
    uint8_t testIndex;
    char test_file_name[30];

    /* time delay (~100ms) */
    for (resultSize = 0; resultSize < 400000; ++resultSize)
    {
        __NOP();
    }

    LOG_DBG("............................fatfs test.....................\r\n");
    CoreDebug->DEMCR |= (1 << CoreDebug_DEMCR_TRCENA_Pos);

    for (testSize = 0; testSize < (THROUGHPUT_BUFFER_SIZE / 4); ++testSize)
    {
        testThroughputBuffer[testSize] = testSize;
    }

    sprintf(test_file_name, "%c:", USBDISK + '0');
    fatfsCode = f_mount(&fatfs, test_file_name, 1);
    if (fatfsCode)
    {
        LOG_ERR("fatfs mount error\r\n");
        USB_HostMsdFatfsTestDone();
        return;
    }

    sprintf(test_file_name, "%c:/thput.dat", USBDISK + '0');
    LOG_DBG("throughput test:\r\n");
    for (testIndex = 0; testIndex < (sizeof(testSizeArray) / 4); ++testIndex)
    {
        fatfsCode = f_unlink(test_file_name); /* delete the file if it is existed */
        if ((fatfsCode != FR_OK) && (fatfsCode != FR_NO_FILE))
        {
            USB_HostMsdFatfsTestDone();
            return;
        }

        fatfsCode = f_open(&file, test_file_name, FA_WRITE | FA_READ | FA_CREATE_ALWAYS); /* create one new file */
        if (fatfsCode)
        {
            USB_HostMsdFatfsTestDone();
            return;
        }

        totalTime = 0;
        testSize  = testSizeArray[testIndex] * 1024;
        while (testSize)
        {
            if (msdFatfsInstance->deviceState != kStatus_DEV_Attached)
            {
                USB_HostMsdFatfsTestDone();
                return;
            }
            DWT->CYCCNT = 0;
            DWT->CTRL |= (1 << DWT_CTRL_CYCCNTENA_Pos);
            fatfsCode = f_write(&file, testThroughputBuffer, THROUGHPUT_BUFFER_SIZE, &resultSize);
            if (fatfsCode)
            {
                LOG_ERR("write error\r\n");
                f_close(&file);
                USB_HostMsdFatfsTestDone();
                return;
            }
            totalTime += DWT->CYCCNT;
            DWT->CTRL &= ~(1 << DWT_CTRL_CYCCNTENA_Pos);
            testSize -= THROUGHPUT_BUFFER_SIZE;
        }
        testSize = testSizeArray[testIndex];
        LOG_DBG("    write %dKB data the speed is %d KB/s\r\n", testSize,
               (uint32_t)((uint64_t)testSize * (uint64_t)MCU_CORE_CLOCK / (uint64_t)totalTime));

        fatfsCode = f_lseek(&file, 0);
        if (fatfsCode)
        {
            USB_HostMsdFatfsTestDone();
            return;
        }
        totalTime = 0;
        testSize  = testSizeArray[testIndex] * 1024;
        while (testSize)
        {
            if (msdFatfsInstance->deviceState != kStatus_DEV_Attached)
            {
                USB_HostMsdFatfsTestDone();
                return;
            }
            DWT->CYCCNT = 0;
            DWT->CTRL |= (1 << DWT_CTRL_CYCCNTENA_Pos);
            fatfsCode = f_read(&file, testThroughputBuffer, THROUGHPUT_BUFFER_SIZE, &resultSize);
            if (fatfsCode)
            {
                LOG_ERR("read error\r\n");
                f_close(&file);
                USB_HostMsdFatfsTestDone();
                return;
            }
            totalTime += DWT->CYCCNT;
            DWT->CTRL &= ~(1 << DWT_CTRL_CYCCNTENA_Pos);
            testSize -= THROUGHPUT_BUFFER_SIZE;
        }
        testSize = testSizeArray[testIndex];
        LOG_DBG("    read %dKB data the speed is %d KB/s\r\n", testSize,
               (uint32_t)((uint64_t)testSize * (uint64_t)MCU_CORE_CLOCK / (uint64_t)totalTime));

        fatfsCode = f_close(&file);
        if (fatfsCode)
        {
            USB_HostMsdFatfsTestDone();
            return;
        }
    }

    USB_HostMsdFatfsTestDone();
}

#else

static void USB_HostMsdFatfsDisplayFileInfo(FILINFO *fileInfo)
{
    char *fileName;
#if ((defined _USE_LFN) && (_USE_LFN))
    fileName = (fileInfo->lfname[0] ? fileInfo->lfname : fileInfo->fname;
#else
    fileName = fileInfo->fname;
#endif /* _USE_LFN */
    /* note: if this file/directory don't have one attribute, '_' replace the attribute letter ('R' - readonly, 'H' - hide, 'S' - system) */
    LOG_DBG("    %s - %c%c%c - %s - %dBytes - %d-%d-%d %d:%d:%d\r\n",( (fileInfo->fattrib & (uint8_t)AM_DIR) != 0U) ? "dir" : "fil",
             ((fileInfo->fattrib & (uint8_t)AM_RDO) != 0U) ? 'R' : '_',
             ((fileInfo->fattrib & (uint8_t)AM_HID) != 0U) ? 'H' : '_',
             ((fileInfo->fattrib & (uint8_t)AM_SYS) != 0U) ? 'S' : '_',
             fileName,
             (fileInfo->fsize),
             (((uint32_t)fileInfo->fdate >> 9U) + (uint32_t)1980U) /* year */,
             (((uint32_t)fileInfo->fdate >> 5U) & (uint32_t)0x000Fu) /* month */,
             ((uint32_t)fileInfo->fdate & (uint32_t)0x001Fu) /* day */,
             (((uint32_t)fileInfo->ftime >> 11U) & (uint32_t)0x0000001Fu) /* hour */,
             (((uint32_t)fileInfo->ftime >> 5U) & (uint32_t)0x0000003Fu) /* minute */,
             ((uint32_t)fileInfo->ftime & (uint32_t)0x0000001Fu) /* second */
             );
    (void)fileName;
}

static FRESULT USB_HostMsdFatfsListDirectory(const TCHAR *path)
{
    FRESULT fatfsCode = FR_OK;
    FILINFO fileInfo;
    DIR dir;
    uint8_t outputLabel = 0;

#if ((defined _USE_LFN) && (_USE_LFN))
    static uint8_t fileNameBuffer[_MAX_LFN];
    fileInfo.lfname = fileNameBuffer;
    fileInfo.lfsize = _MAX_LFN;
#endif /* _USE_LFN */

    memset(&dir, 0, sizeof(dir));
    fatfsCode = f_opendir(&dir, path);
    if (fatfsCode != FR_OK)
    {
        return fatfsCode;
    }
    while (true)
    {
        fatfsCode = f_readdir(&dir, &fileInfo);
        if ((fatfsCode != FR_OK ) || ( (char)0 == fileInfo.fname[0]))
        {
            break;
        }
        outputLabel = 1U;
        USB_HostMsdFatfsDisplayFileInfo(&fileInfo);
    }
    if (outputLabel == 0U)
    {
        LOG_DBG("\r\n");
    }

    return fatfsCode;
}

#if ((defined _USE_FORWARD) && (_USE_FORWARD)) && ((defined _FS_TINY) && (_FS_TINY))
static uint32_t USB_HostMsdFatfsForward(const uint8_t *data, uint32_t dataLength)
{
    uint32_t resultCount = dataLength;

    if (dataLength == 0)
    {
        return 1;
    }
    else
    {
        do
        {
            LOG_DBG("%c", *data);
            data++;
            resultCount--;
        } while (resultCount);
        return dataLength;
    }
}
#endif

static void USB_HostMsdFatfsMount(usb_host_msd_fatfs_instance_t *msdFatfsInstance)
{
    FRESULT fatfsCode;
    FATFS *fs;

#if ((defined _USE_LFN) && (_USE_LFN))
    FILINFO fileInfo;
#endif /* _USE_LFN */
    uint32_t freeClusterNumber;
    uint8_t driver_number_buffer[3];

#if ((defined _USE_LFN) && (_USE_LFN))
    static uint8_t fileNameBuffer[_MAX_LFN];
    fileInfo.lfname = fileNameBuffer;
    fileInfo.lfsize = _MAX_LFN;
    (void)fileInfo; /*fix build warning: set but never used.*/
#endif /* _USE_LFN */

    /* time delay */
    for (freeClusterNumber = 0; freeClusterNumber < 10000U; ++freeClusterNumber)
    {
        __NOP();
    }

    LOG_DBG("fatfs mount as logiacal driver %d......", USBDISK);
    (void)sprintf((char *)&driver_number_buffer[0], "%c:", USBDISK + '0');
    fatfsCode = f_mount(&fatfs, (char const *)&driver_number_buffer[0], 0);
    if (fatfsCode != FR_OK)
    {
        LOG_ERR("Mount error\r\n");
        return;
    }

    usb_echo("usb-fatfs-mounted!\r\n");

#if (FF_FS_RPATH >= 2)
    fatfsCode = f_chdrive((char const *)&driver_number_buffer[0]);
    if (fatfsCode != FR_OK)
    {
        LOG_ERR("Change current drive error\r\n");
        return;
    }
#endif

#if 0
#if FF_USE_MKFS
    MKFS_PARM formatOptions;
    formatOptions.fmt = FM_SFD | FM_ANY;
    LOG_DBG("test f_mkfs......");
    fatfsCode = f_mkfs((char const *)&driver_number_buffer[0], &formatOptions, testBuffer, FF_MAX_SS);
    if (fatfsCode != FR_OK)
    {
        LOG_ERR("Make directory error\r\n");
        return;
    }
    LOG_DBG("success\r\n");
#endif /* FF_USE_MKFS */
#endif

    usb_echo("Get Disk information,\r\n");
    fatfsCode = f_getfree((char const *)&driver_number_buffer[0], (DWORD *)&freeClusterNumber, &fs);
    if (fatfsCode != FR_OK)
    {
        LOG_ERR("Get free info error\r\n");
        return;
    }
    if (fs->fs_type == (uint8_t)FS_FAT12)
    {
        usb_echo("    FAT type = FAT12\r\n");
    }
    else if (fs->fs_type == (uint8_t)FS_FAT16)
    {
        usb_echo("    FAT type = FAT16\r\n");
    }
    else
    {
        usb_echo("    FAT type = FAT32\r\n");
    }
    LOG_DBG("    bytes per cluster = %d; number of clusters=%lu \r\n", fs->csize * 512U, fs->n_fatent - 2U);
    LOG_DBG("    The free size: %dKB, the total size:%dKB\r\n", (freeClusterNumber * (fs->csize) / 2U),
           ((fs->n_fatent - 2U) * (fs->csize) / 2U));

    LOG_DBG("directory operation:\r\n");
    LOG_DBG("list root directory:\r\n");
    fatfsCode = USB_HostMsdFatfsListDirectory((char const *)&driver_number_buffer[0]);
    if (fatfsCode != FR_OK)
    {
        return;
    }
}

#endif /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */

void USB_HostMsdTask(void *arg)
{
    usb_status_t status;
    usb_host_msd_fatfs_instance_t *msdFatfsInstance = (usb_host_msd_fatfs_instance_t *)arg;

    if (msdFatfsInstance->deviceState != msdFatfsInstance->prevDeviceState)
    {
        msdFatfsInstance->prevDeviceState = msdFatfsInstance->deviceState;
        switch (msdFatfsInstance->deviceState)
        {
            case kStatus_DEV_Idle:
                break;

            case kStatus_DEV_Attached: /* deivce is attached and numeration is done */
                status                = USB_HostMsdInit(msdFatfsInstance->deviceHandle,
                                         &msdFatfsInstance->classHandle); /* msd class initialization */
                g_UsbFatfsClassHandle = msdFatfsInstance->classHandle;
                if (status != kStatus_USB_Success)
                {
                    LOG_DBG("usb host msd init fail\r\n");
                    return;
                }
                usb_echo("usb-otg attached!\r\n");
                msdFatfsInstance->runState = kUSB_HostMsdRunSetInterface;
                break;

            case kStatus_DEV_Detached: /* device is detached */
                msdFatfsInstance->deviceState = kStatus_DEV_Idle;
                msdFatfsInstance->runState    = kUSB_HostMsdRunIdle;
                (void)USB_HostMsdDeinit(msdFatfsInstance->deviceHandle,
                                  msdFatfsInstance->classHandle); /* msd class de-initialization */
                msdFatfsInstance->classHandle = NULL;
                usb_echo("usb-otg detached!\r\n");
                break;

            default:
            			/* Misra */
                break;
        }
    }

    /* run state */
    switch (msdFatfsInstance->runState)
    {
        case kUSB_HostMsdRunIdle:
            vTaskDelay(1000U/ portTICK_RATE_MS);
            break;

        case kUSB_HostMsdRunSetInterface: /* set msd interface */
            msdFatfsInstance->runState     = kUSB_HostMsdRunIdle;
            msdFatfsInstance->runWaitState = kUSB_HostMsdRunWaitSetInterface;
            status = USB_HostMsdSetInterface(msdFatfsInstance->classHandle, msdFatfsInstance->interfaceHandle, 0,
                                             USB_HostMsdAppControlCallback, msdFatfsInstance);
            if (status != kStatus_USB_Success)
            {
                LOG_ERR("set interface fail\r\n");
            }
            break;

        case kUSB_HostMsdRunMassStorageMount: /* set interface succeed */
#if ((defined MSD_FATFS_THROUGHPUT_TEST_ENABLE) && (MSD_FATFS_THROUGHPUT_TEST_ENABLE))
            USB_HostMsdFatfsThroughputTest(msdFatfsInstance); /* test throughput */
#else
            USB_HostMsdFatfsMount(msdFatfsInstance); /* test msd device */
#endif /* MSD_FATFS_THROUGHPUT_TEST_ENABLE */
            (void)xEventGroupSetBits(s_msdAttachedEvent, EVENT_BIT_MAP_ENUMRATION_DONE);
            msdFatfsInstance->runState = kUSB_HostMsdRunIdle;
            break;

        default:
        	   /* Misra */
            break;
    }
}

usb_status_t USB_HostMsdEvent(usb_device_handle deviceHandle,
                              usb_host_configuration_handle configurationHandle,
                              uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    usb_host_configuration_t *configuration;
    uint8_t interfaceIndex;
    usb_host_interface_t *interface;
    uint32_t pid = 0U;
    uint32_t vid = 0U;
    uint32_t address = 0U;
    uint8_t id;
    usb_host_event_t usb_event = (usb_host_event_t)(uint8_t)((uint8_t)eventCode & 0xFFU);

    switch (usb_event)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                id        = interface->interfaceDesc->bInterfaceClass;
                if (id != USB_HOST_MSD_CLASS_CODE)
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceSubClass;
                if ((id != USB_HOST_MSD_SUBCLASS_CODE_UFI) && (id != USB_HOST_MSD_SUBCLASS_CODE_SCSI))
                {
                    continue;
                }
                id = interface->interfaceDesc->bInterfaceProtocol;
                if (id != USB_HOST_MSD_PROTOCOL_BULK)
                {
                    continue;
                }
                else
                {
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        /* the interface is supported by the application */
                        g_MsdFatfsInstance.deviceHandle    = deviceHandle;
                        g_MsdFatfsInstance.interfaceHandle = interface;
                        g_MsdFatfsInstance.configHandle    = configurationHandle;
                        (void)xEventGroupSetBits(s_msdAttachedEvent, EVENT_BIT_MAP_ATTACHED);
                        return kStatus_USB_Success;
                    }
                    else
                    {
                        continue;
                    }
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            break;

        case kUSB_HostEventEnumerationDone:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                if ((g_MsdFatfsInstance.deviceHandle != NULL) && (g_MsdFatfsInstance.interfaceHandle != NULL))
                {
                    /* the device enumeration is done */
                    if (g_MsdFatfsInstance.deviceState == kStatus_DEV_Idle)
                    {
                        g_MsdFatfsInstance.deviceState = kStatus_DEV_Attached;

                        (void)USB_HostHelperGetPeripheralInformation(deviceHandle, (uint32_t)kUSB_HostGetDevicePID, &pid);
                        (void)USB_HostHelperGetPeripheralInformation(deviceHandle, (uint32_t)kUSB_HostGetDeviceVID, &vid);
                        (void)USB_HostHelperGetPeripheralInformation(deviceHandle, (uint32_t)kUSB_HostGetDeviceAddress, &address);
                        LOG_DBG("The USB MSD disk is attached (pid=0x%x ,vid=0x%x) with assigned address=%d", pid, vid,
                               address);
                    }
                    else
                    {
                        LOG_ERR("No slot for new arrived disk\r\n");
                        status = kStatus_USB_Error;
                    }
                }
            }
            break;

        case kUSB_HostEventDetach:
            if (g_MsdFatfsInstance.configHandle == configurationHandle)
            {
                /* the device is detached */
                g_UsbFatfsClassHandle           = NULL;
                g_MsdFatfsInstance.configHandle = NULL;
                if (g_MsdFatfsInstance.deviceState != kStatus_DEV_Idle)
                {
                    g_MsdFatfsInstance.deviceState = kStatus_DEV_Detached;
                }
            }
            break;

        default:
        	   /* Misra */
            break;
    }
    return status;
}

#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
usb_status_t USB_HostTestEvent(usb_device_handle deviceHandle,
                               usb_host_configuration_handle configurationHandle,
                               uint32_t eventCode)
{
    /* process the same supported device that is identified by configurationHandle */
    static usb_host_configuration_handle s_ConfigHandle = NULL;
    static usb_device_handle s_DeviceHandle             = NULL;
    static usb_host_interface_handle s_InterfaceHandle  = NULL;
    usb_status_t status                                 = kStatus_USB_Success;
    usb_host_configuration_t *configuration;
    uint8_t interfaceIndex;
    usb_host_interface_t *interface;
    uint32_t id;

    switch (eventCode)
    {
        case kUSB_HostEventAttach:
            /* judge whether is configurationHandle supported */
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                interface = &configuration->interfaceList[interfaceIndex];
                (void)USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceVID, &id);
                if (id == 0x1a0a) /* certification Vendor ID */
                {
                    LOG_DBG("cetification test device VID match\r\n");
                    s_DeviceHandle    = deviceHandle;
                    s_InterfaceHandle = interface;
                    s_ConfigHandle    = configurationHandle;
                    return kStatus_USB_Success;
                }
            }
            status = kStatus_USB_NotSupported;
            break;

        case kUSB_HostEventNotSupported:
            LOG_ERR("Unsupported Device\r\n");
            break;

        case kUSB_HostEventEnumerationDone:
            if (s_ConfigHandle == configurationHandle)
            {
                USB_HostTestModeInit(s_DeviceHandle);
            }
            break;

        case kUSB_HostEventDetach:
            if (s_ConfigHandle == configurationHandle)
            {
                LOG_DBG("PET test device detach\r\n");
                USB_HostCloseDeviceInterface(s_DeviceHandle, s_InterfaceHandle);
                /* the device is detached */
                s_DeviceHandle    = NULL;
                s_InterfaceHandle = NULL;
                s_ConfigHandle    = NULL;
            }
            break;

        default:
        	   /* Misra */
            break;
    }
    return status;
}
#endif

static void USB_HostTask(void *param)
{
    while (true)
    {
#if (defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U))
        USB_HostEhciTaskFunction(param);
#elif (defined(USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
        USB_HostIp3516HsTaskFunction(param);
#else
#error The controller is not supported!
#endif
    }
}

static void USB_HostApplicationTask(void *param)
{
    while (true)
    {
        USB_HostMsdTask(param);
    }
}

#if (defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U))
#if (defined USBHS_IRQHandler)
void USBHS_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostMsdFatfsHandle);
    SDK_ISR_EXIT_BARRIER;
}
#else
void USB_OTG1_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostMsdFatfsHandle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

void USB_OTG2_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostMsdFatfsHandle);
    SDK_ISR_EXIT_BARRIER;
}
#elif (defined(USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
void USB_IRQHandler(void)
{
    USB_HostIp3516HsIsrFunction(g_HostMsdFatfsHandle);
    SDK_ISR_EXIT_BARRIER;
}
void USB0_IRQHandler(void)
{
    USB_HostIp3516HsIsrFunction(g_HostMsdFatfsHandle);
    SDK_ISR_EXIT_BARRIER;
}           
#endif

int USB_HostMsdFatfsInit(void)
{
    /* Initialize USB for FS and create tasks */
    usb_status_t status = kStatus_USB_Success;
    EventBits_t uxBits;
    TickType_t xTicksToWait;

    s_msdAttachedEvent = xEventGroupCreate();

    if (NULL == s_msdAttachedEvent)
    {
        return -1;
    }

    USB_HostClockInit();

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit((uint8_t)CONTROLLER_ID, &g_HostMsdFatfsHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        LOG_ERR("host init error\r\n");
        return -1;
    }
    USB_HostIsrEnable();

    LOG_DBG("USB Host stack successfully initialized\r\n");

    if (xTaskCreate(USB_HostTask, "usb host task", ((int)2000UL / (int)sizeof(portSTACK_TYPE)), g_HostMsdFatfsHandle,
                    configMAX_PRIORITIES - 2, NULL) != pdPASS)
    {
        LOG_ERR("create host task error\r\n");
        return -2;
    }
    if (xTaskCreate(USB_HostApplicationTask, "app task", ((int)(2300UL) / (int)sizeof(portSTACK_TYPE)), &g_MsdFatfsInstance, 3,
                    NULL) != pdPASS)
    {
        LOG_ERR("create mouse task error\r\n");
        return -3;
    }

    xTicksToWait = MSD_ATTCH_TIMEOUT / portTICK_PERIOD_MS;
    /* Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
    the event group.  Clear the bits before exiting. */
    uxBits = xEventGroupWaitBits(
        s_msdAttachedEvent,                                     /* The event group being tested. */
        EVENT_BIT_MAP_ATTACHED | EVENT_BIT_MAP_ENUMRATION_DONE, /* The bits within the event group to wait for. */
        pdTRUE,        /* EVENT_BIT_MAP_ATTACHED & EVENT_BIT_MAP_ENUMRATION_DONE should be cleared before returning. */
        pdFALSE,       /* Don't wait for both bits, either bit will do. */
        xTicksToWait); /* Wait a maximum of 100ms for either bit to be set. */

    if ((uxBits & EVENT_BIT_MAP_ENUMRATION_DONE) == EVENT_BIT_MAP_ENUMRATION_DONE)
    {
    }
    else if ((uxBits & EVENT_BIT_MAP_ATTACHED) == EVENT_BIT_MAP_ATTACHED)
    {
        xTicksToWait = MSD_ENUM_TIMEOUT / portTICK_PERIOD_MS;
        (void)xEventGroupWaitBits(s_msdAttachedEvent,            /* The event group being tested. */
                                  EVENT_BIT_MAP_ENUMRATION_DONE, /* The bits within the event group to wait for. */
                                  pdFALSE, /* EVENT_BIT_MAP_ENUMRATION_DONE should not be cleared before returning. */
                                  pdFALSE, /* Don't wait for both bits, either bit will do. */
                                  xTicksToWait); /* Wait a maximum of 100ms for either bit to be set. */
    }
    else
    {
        /* Misra*/
    }
    return 0;
}

__WEAK_FUNC void USB_HostClockInit(void)
{
    assert(0);
}

__WEAK_FUNC void USB_HostIsrEnable(void)
{
    assert(0);
}
