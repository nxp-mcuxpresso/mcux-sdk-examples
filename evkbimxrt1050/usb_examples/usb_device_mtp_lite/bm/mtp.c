/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_mtp.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "ff.h"
#include "mtp_file_system_adapter.h"
#include "mtp_operation.h"
#include "mtp.h"
#include "diskio.h"

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#if (USB_DEVICE_CONFIG_USE_TASK < 1)
#error This application requires USB_DEVICE_CONFIG_USE_TASK value defined > 0 in usb_device_config.h. Please recompile with this option.
#endif
#include "sdmmc_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct _usb_mtp_disk_operation_msgq_struct
{
    usb_device_mtp_cmd_data_struct_t dataInfo[USB_DEVICE_MTP_MSG_QUEUE_COUNT];
    uint32_t event[USB_DEVICE_MTP_MSG_QUEUE_COUNT];
    uint8_t num;
    uint8_t head;
    uint8_t tail;
} usb_mtp_disk_operation_msgq_struct_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#if USB_DEVICE_CONFIG_USE_EVENT_TASK
extern void USB_DeviceEventTask(void);
extern void USB_DeviceEventInit(void);
#endif
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern sd_card_t g_sd;
const uint16_t g_OpSupported[] = {
    MTP_OPERATION_GET_DEVICE_INFO,
    MTP_OPERATION_OPEN_SESSION,
    MTP_OPERATION_CLOSE_SESSION,
    MTP_OPERATION_GET_STORAGE_IDS,
    MTP_OPERATION_GET_STORAGE_INFO,
    MTP_OPERATION_GET_OBJECT_HANDLES,
    MTP_OPERATION_GET_OBJECT_INFO,
    MTP_OPERATION_GET_OBJECT,
    MTP_OPERATION_DELETE_OBJECT,
    MTP_OPERATION_SEND_OBJECT_INFO,
    MTP_OPERATION_SEND_OBJECT,
    MTP_OPERATION_MOVE_OBJECT,
    MTP_OPERATION_COPY_OBJECT,
    MTP_OPERATION_GET_DEVICE_PROP_DESC,
    MTP_OPERATION_GET_DEVICE_PROP_VALUE,
    MTP_OPERATION_SET_DEVICE_PROP_VALUE,
    MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED,
    MTP_OPERATION_GET_OBJECT_PROP_DESC,
    MTP_OPERATION_GET_OBJECT_PROP_VALUE,
    MTP_OPERATION_SET_OBJECT_PROP_VALUE,
    MTP_OPERATION_GET_OBJECT_PROP_LIST,
    MTP_OPERATION_GET_OBJECT_REFERENCES,
};

const uint16_t g_EventSupported[] = {
    MTP_EVENT_OBJECT_ADDED,
    MTP_EVENT_OBJECT_REMOVED,
    MTP_EVENT_DEVICE_PROP_CHANGED,
    MTP_EVENT_OBJECT_INFO_CHANGED,
};

const uint16_t g_DevPropSupported[] = {
    MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,
};

const uint16_t g_ObjFormatSupported[] = {
    MTP_FORMAT_UNDEFINED,
    MTP_FORMAT_ASSOCIATION,
};

usb_device_mtp_dev_prop_desc_t g_DevPropDesc[] = {
    {
        .devPropCode    = MTP_DEVICE_PROPERTY_DEVICE_FRIENDLY_NAME,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x01U, /* Get/Set */
        .defaultVal.str = NULL,
        .currentVal.str = NULL,
        .formFlag       = 0x00U,
    },
};

usb_device_mtp_dev_prop_desc_list_t g_DevPropDescList = {
    .devPropDesc      = &g_DevPropDesc[0],
    .devPropDescCount = sizeof(g_DevPropDesc) / sizeof(g_DevPropDesc[0]),
};

usb_device_mtp_obj_prop_desc_t g_UndefinedOrAssociationObjPropDesc[] = {
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_STORAGE_ID,
        .dataType       = MTP_TYPE_UINT32,
        .getSet         = 0x00U, /* Get */
        .defaultVal.u32 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_OBJECT_FORMAT,
        .dataType       = MTP_TYPE_UINT16,
        .getSet         = 0x00U,
        .defaultVal.u16 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_PROTECTION_STATUS,
        .dataType       = MTP_TYPE_UINT16,
        .getSet         = 0x00U,
        .defaultVal.u16 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_OBJECT_SIZE,
        .dataType       = MTP_TYPE_UINT64,
        .getSet         = 0x00U,
        .defaultVal.u64 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_OBJECT_FILE_NAME,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x01U, /* Get/Set */
        .defaultVal.str = NULL,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_DATE_MODIFIED,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x00U,
        .defaultVal.u64 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x03U, /* DateTime form */
    },
    {
        .objPropCode = MTP_OBJECT_PROPERTY_PERSISTENT_UID,
        .dataType    = MTP_TYPE_UINT128,
        .getSet      = 0x00U,
        .defaultVal.u128 =
            NULL, /* The default value is 0x0...0 for Persistent UID. NULL will be interpret as 0x0...0. */
        .groupCode = 0x00U,
        .formFlag  = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_PARENT_OBJECT,
        .dataType       = MTP_TYPE_UINT32,
        .getSet         = 0x00U,
        .defaultVal.u32 = 0x00U,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_NAME,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x00U,
        .defaultVal.str = NULL,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_DISPLAY_NAME,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x00U,
        .defaultVal.str = NULL,
        .groupCode      = 0x00U,
        .formFlag       = 0x00U,
    },
    {
        .objPropCode    = MTP_OBJECT_PROPERTY_DATE_ADDED,
        .dataType       = MTP_TYPE_STR,
        .getSet         = 0x00U,
        .defaultVal.str = NULL,
        .groupCode      = 0x00U,
        .formFlag       = 0x03U, /* DateTime form */
    },
};

usb_device_mtp_obj_prop_t g_ObjProp[] = {
    {
        .objFormat = MTP_FORMAT_UNDEFINED,
        .objPropDescCount =
            sizeof(g_UndefinedOrAssociationObjPropDesc) / sizeof(g_UndefinedOrAssociationObjPropDesc[0]),
        .objPropDesc = &g_UndefinedOrAssociationObjPropDesc[0],
    },
    {
        .objFormat = MTP_FORMAT_ASSOCIATION,
        .objPropDescCount =
            sizeof(g_UndefinedOrAssociationObjPropDesc) / sizeof(g_UndefinedOrAssociationObjPropDesc[0]),
        .objPropDesc = &g_UndefinedOrAssociationObjPropDesc[0],
    },
};

usb_device_mtp_obj_prop_list_t g_ObjPropList = {
    .objProp      = &g_ObjProp[0],
    .objPropCount = sizeof(g_ObjProp) / sizeof(g_ObjProp[0]),
};

/* 2-byte unicode */
USB_DMA_INIT_DATA_ALIGN(2U)
uint8_t g_StorageRootPath[] = {
#if defined(SD_DISK_ENABLE)
    SDDISK + '0',
#elif defined(MMC_DISK_ENABLE)
    MMCDISK + '0',
#else
    '0',
#endif
    0x00U,        ':', 0x00U, '/', 0x00U, 0x00U, 0x00U,
};

usb_device_mtp_storage_info_t g_StorageInfo[MTP_STORAGE_COUNT] = {{
    .rootPath         = &g_StorageRootPath[0], /* 2-byte unicode */
    .storageDesc      = "NXP MTP",             /* ascll code, will convert to unicode when host gets this field. */
    .volumeID         = NULL,                  /* ascll code, will convert to unicode when host gets this field. */
    .storageID        = 0x00010001U,           /* should ensure its uniqueness. */
    .storageType      = MTP_STORAGE_FIXED_RAM,
    .fileSystemType   = MTP_STORAGE_FILESYSTEM_GENERIC_HIERARCHICAL,
    .accessCapability = MTP_STORAGE_READ_WRITE,
    .flag             = 0U,
}};

usb_device_mtp_storage_list_t g_StorageList = {
    .storageInfo  = &g_StorageInfo[0],
    .storageCount = sizeof(g_StorageInfo) / sizeof(g_StorageInfo[0]),
};

/* 2-byte unicode, the buffer is used to save device friendly name.
   If the device friendly name length set by host exceeds MTP_DEVICE_FRIENDLY_NAME_LEN, the name will be truncated. */
USB_DMA_INIT_DATA_ALIGN(2U)
uint8_t g_DevFriendlyName[MTP_DEVICE_FRIENDLY_NAME_LEN] = {
    'N', 0x00U, 'X', 0x00U, 'P', 0x00U, ' ', 0x00U, 'M', 0x00U, 'T', 0x00U, 'P', 0x00U, 0x00U, 0x00U,
};

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_device_mtp_container_t g_mtpContainer;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_device_mtp_event_container_t g_mtpEvent;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) usb_device_mtp_device_status_t g_mtpStatus;

/* Data structure of mtp device, store the information, such as class handle */
usb_mtp_struct_t g_mtp;
usb_device_mtp_struct_t *g_mtpHandle = &g_mtp.mtpStruct;

/* The buffer is used to build path, please make sure the buffer have enough space to accommodate the longest path.
   If the path length exceeds MTP_PATH_MAX_LEN, the current transaction will end with a failure. */
USB_DMA_NONINIT_DATA_ALIGN(2U) uint16_t g_pathBuffer[MTP_PATH_MAX_LEN >> 1U];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_mtpTransferBuffer[USB_DEVICE_MTP_TRANSFER_BUFF_SIZE >> 2];

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_SetupOutBuffer[8];

/* The queue between USB task and disk operation task. */
usb_mtp_disk_operation_msgq_struct_t g_mtpMsgQueue;
/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_mtp.deviceHandle);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_mtp.deviceHandle);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void USB_DeviceClockInit(void)
{
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };

    if (CONTROLLER_ID == kUSB_ControllerEhci0)
    {
        CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs0Clock(kCLOCK_Usb480M, 480000000U);
    }
    else
    {
        CLOCK_EnableUsbhs1PhyPllClock(kCLOCK_Usbphy480M, 480000000U);
        CLOCK_EnableUsbhs1Clock(kCLOCK_Usb480M, 480000000U);
    }
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceEhciTaskFunction(deviceHandle);
}
#endif

/*!
 * @brief mtp enter critical.
 *
 * This function is used to enter critical disable interrupt .
 *
 */
static void USB_BmEnterCritical(uint32_t *sr)
{
    *sr = DisableGlobalIRQ();
    __ASM("CPSID i");
}
/*!
 * @brief mtp exit critical.
 *
 * This function is used to exit critical ,enable interrupt .
 *
 */
static void USB_BmExitCritical(uint32_t sr)
{
    EnableGlobalIRQ(sr);
}

void USB_DeviceMtpInitQueue(void)
{
    g_mtpMsgQueue.num  = 0;
    g_mtpMsgQueue.head = 0;
    g_mtpMsgQueue.tail = 0;
}

usb_status_t USB_DeviceMtpQueueGet(usb_device_mtp_cmd_data_struct_t *dataInfo, uint32_t *event)
{
    uint32_t sr;
    usb_status_t status;

    USB_BmEnterCritical(&sr);
    if (g_mtpMsgQueue.num != 0U)
    {
        (void)memcpy(dataInfo, &g_mtpMsgQueue.dataInfo[g_mtpMsgQueue.tail], sizeof(usb_device_mtp_cmd_data_struct_t));
        *event = g_mtpMsgQueue.event[g_mtpMsgQueue.tail];

        g_mtpMsgQueue.tail++;
        g_mtpMsgQueue.num--;

        if (g_mtpMsgQueue.tail >= USB_DEVICE_MTP_MSG_QUEUE_COUNT)
        {
            g_mtpMsgQueue.tail = 0;
        }
        status = kStatus_USB_Success;
    }
    else
    {
        /* queue is empty */
        status = kStatus_USB_Error;
    }

    USB_BmExitCritical(sr);

    return status;
}

usb_status_t USB_DeviceMtpQueuePut(usb_device_mtp_cmd_data_struct_t *dataInfo, uint32_t event)
{
    uint32_t sr;
    usb_status_t status;

    USB_BmEnterCritical(&sr);

    if (g_mtpMsgQueue.num < USB_DEVICE_MTP_MSG_QUEUE_COUNT)
    {
        (void)memcpy(&g_mtpMsgQueue.dataInfo[g_mtpMsgQueue.head], dataInfo, sizeof(usb_device_mtp_cmd_data_struct_t));
        g_mtpMsgQueue.event[g_mtpMsgQueue.head] = event;

        g_mtpMsgQueue.head++;
        g_mtpMsgQueue.num++;

        if (g_mtpMsgQueue.head >= USB_DEVICE_MTP_MSG_QUEUE_COUNT)
        {
            g_mtpMsgQueue.head = 0;
        }
        status = kStatus_USB_Success;
    }
    else
    {
        /* queue is full */
        status = kStatus_USB_Error;
    }

    USB_BmExitCritical(sr);

    return status;
}

usb_status_t USB_DeviceMtpRecv(usb_device_mtp_struct_t *mtpHandle)
{
    uint32_t size;

    /* which one is smaller? */
    /* size =
       (mtpHandle->transferTotal > mtpHandle->transferLength) ? mtpHandle->transferLength : mtpHandle->transferTotal; */
    size = (USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH > mtpHandle->transferLength) ?
               mtpHandle->transferLength :
               USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH;

    return USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint,
                                 &mtpHandle->transferBuffer[mtpHandle->transferOffset], size);
}

usb_status_t USB_DeviceMtpSend(usb_device_mtp_struct_t *mtpHandle)
{
    uint32_t size;

    /* which one is smaller? */
    size =
        (mtpHandle->transferTotal > mtpHandle->transferLength) ? mtpHandle->transferLength : mtpHandle->transferTotal;
    size = (USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH > size) ? size : USB_DEVICE_MTP_MAX_SEND_TRANSFER_LENGTH;

    return USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkInEndpoint,
                                 &mtpHandle->transferBuffer[mtpHandle->transferOffset], size);
}

void USB_DeviceMtpPrimeCommand(usb_device_mtp_struct_t *mtpHandle)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_COMMAND;

    USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, (uint8_t *)mtpHandle->mtpContainer,
                          USB_DEVICE_MTP_COMMAND_LENGTH);
}

usb_status_t USB_DeviceMtpPrimeResponse(usb_device_mtp_struct_t *mtpHandle,
                                        uint16_t respCode,
                                        uint32_t *respParam,
                                        uint8_t respParamSize)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

    mtpHandle->mtpContainer->containerLength = USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH + respParamSize * 4U;
    mtpHandle->mtpContainer->containerType   = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
    mtpHandle->mtpContainer->code            = respCode;

    while (respParamSize--)
    {
        mtpHandle->mtpContainer->param[respParamSize] = respParam[respParamSize];
    }

    return USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkInEndpoint, (uint8_t *)mtpHandle->mtpContainer,
                                 mtpHandle->mtpContainer->containerLength);
}

void USB_DevicePrimeDataIn(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    usb_device_mtp_container_t *container;

    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_BULK_IN;

    mtpHandle->transferTotal  = dataInfo->totalSize;
    mtpHandle->transferLength = dataInfo->curSize;
    mtpHandle->transferBuffer = dataInfo->buffer;
    mtpHandle->transferDone   = 0;
    mtpHandle->transferOffset = 0;

    container                  = (usb_device_mtp_container_t *)&mtpHandle->transferBuffer[0];
    container->containerLength = (mtpHandle->transferTotal > USB_DEVICE_MTP_MAX_UINT32_VAL) ?
                                     USB_DEVICE_MTP_MAX_UINT32_VAL :
                                     mtpHandle->transferTotal;
    container->containerType = USB_DEVICE_MTP_CONTAINER_TYPE_DATA;
    container->code          = mtpHandle->mtpContainer->code;
    container->transactionID = mtpHandle->transactionID;

    USB_DeviceMtpSend(mtpHandle);
}

void USB_DevicePrimeDataOut(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_BULK_OUT;

    mtpHandle->transferTotal  = 0;
    mtpHandle->transferLength = dataInfo->curSize;
    mtpHandle->transferBuffer = dataInfo->buffer;
    mtpHandle->transferDone   = 0;
    mtpHandle->transferOffset = 0;

    USB_DeviceMtpRecv(mtpHandle);
}

void USB_DevicePrimeBulkInAndOutStall(usb_device_mtp_struct_t *mtpHandle, uint16_t code, uint8_t epNeed)
{
    mtpHandle->bulkInStallFlag  = USB_DEVICE_MTP_STATE_BULK_IN_STALL;
    mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_STALL;

    /* prepare data for getting device status */
    if (epNeed != 0U)
    {
        mtpHandle->deviceStatus->wLength   = 6U;
        mtpHandle->deviceStatus->epNumber1 = mtpHandle->bulkInEndpoint;
        mtpHandle->deviceStatus->epNumber2 = mtpHandle->bulkOutEndpoint;
    }
    else
    {
        mtpHandle->deviceStatus->wLength = 4U;
    }
    mtpHandle->deviceStatus->code = code;
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if ((kStatus_USB_Success != USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkInEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkOutEndpoint)))
    {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
        usb_echo("stall endpoint error\r\n");
#endif
    }
#else
    (void)USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkInEndpoint);
    (void)USB_DeviceStallEndpoint(mtpHandle->handle, mtpHandle->bulkOutEndpoint);
#endif
}

/*!
 * @brief Send event through interrupt in endpoint.
 *
 * The function is used to send event through interrupt in endpoint.
 * The function calls USB_DeviceSendRequest internally.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 * @param event Please refer to the structure usb_device_mtp_event_struct_t.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The return value just means if the sending request is successful or not; the transfer done is notified by
 * USB_DeviceMtpInterruptIn.
 * Currently, only one transfer request can be supported for one specific endpoint.
 * If there is a specific requirement to support multiple transfer requests for one specific endpoint, the application
 * should implement a queue in the application level.
 * The subsequent transfer could begin only when the previous transfer is done (get notification through the endpoint
 * callback).
 */
usb_status_t USB_DeviceMtpEventSend(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_event_struct_t *event)
{
    usb_status_t error = kStatus_USB_Error;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    if (0U != mtpHandle->interruptInBusy)
    {
        return kStatus_USB_Busy;
    }

    mtpHandle->interruptInBusy = 1U;

    /* build event structure. */
    mtpHandle->eventContainer->containerLength = (event->paramNumber * 4U) + USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH;
    mtpHandle->eventContainer->containerType   = USB_DEVICE_MTP_CONTAINER_TYPE_EVENT;
    mtpHandle->eventContainer->code            = event->code;
    mtpHandle->eventContainer->transactionID   = mtpHandle->mtpContainer->transactionID;
    mtpHandle->eventContainer->param1          = event->param1;
    mtpHandle->eventContainer->param2          = event->param2;
    mtpHandle->eventContainer->param3          = event->param3;

    error = USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->interruptInEndpoint,
                                  (uint8_t *)mtpHandle->eventContainer, mtpHandle->eventContainer->containerLength);

    if (kStatus_USB_Success != error)
    {
        mtpHandle->interruptInBusy = 0U;
    }
    return error;
}

/*!
 * @brief Send response through bulk in endpoint.
 *
 * The function is used to send response through bulk in endpoint.
 * The function calls USB_DeviceSendRequest internally.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 * @param response Please refer to the structure usb_device_mtp_response_struct_t.
 *
 * @return A USB error code or kStatus_USB_Success.
 *
 * @note The function is used to asynchronously send response to the host. Some operations may consume a lot of time to
 * handle current transaction, such as CopyObject or DeleteObject, which causes the subsequent USB event cannot be
 * responded in time. In this case, a separated task is needed to handle these operations. When the process is complete,
 * a response needs to be sent to the host by calling this function.
 */
usb_status_t USB_DeviceMtpResponseSend(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_response_struct_t *response)
{
    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    return USB_DeviceMtpPrimeResponse(mtpHandle, response->code, &response->param1, response->paramNumber);
}

void USB_DeviceMtpProcessCommand(usb_device_mtp_struct_t *mtpHandle, usb_device_mtp_cmd_data_struct_t *dataInfo)
{
    usb_status_t status = kStatus_USB_Success;

    do
    {
        /* In the command phase, check transaction and session ID */
        if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
        {
            /* Check transction ID */
            if (mtpHandle->mtpContainer->transactionID == (mtpHandle->transactionID + 1U))
            {
                mtpHandle->transactionID = mtpHandle->mtpContainer->transactionID;

                if (mtpHandle->transactionID == 0xFFFFFFFEU)
                {
                    mtpHandle->transactionID = 0;
                }
            }
            else
            {
                dataInfo->code = MTP_RESPONSE_INVALID_TRANSACTION_ID;
                break;
            }

            /* Check session ID */
            if (mtpHandle->mtpContainer->code != MTP_OPERATION_OPEN_SESSION &&
                mtpHandle->mtpContainer->code != MTP_OPERATION_GET_DEVICE_INFO)
            {
                if (mtpHandle->sessionID == 0U)
                {
                    dataInfo->code = MTP_RESPONSE_SESSION_NOT_OPEN;
                    break;
                }
            }

            /* Check Mutex */
            switch (mtpHandle->mtpContainer->code)
            {
                case MTP_OPERATION_OPEN_SESSION:
                case MTP_OPERATION_CLOSE_SESSION:
                case MTP_OPERATION_GET_DEVICE_INFO:
                case MTP_OPERATION_GET_DEVICE_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
                case MTP_OPERATION_GET_STORAGE_IDS:
                case MTP_OPERATION_GET_STORAGE_INFO:
                case MTP_OPERATION_GET_OBJECT_HANDLES:
                case MTP_OPERATION_GET_OBJECT_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROP_LIST:
                case MTP_OPERATION_GET_OBJECT_INFO:
                case MTP_OPERATION_GET_OBJECT:
                case MTP_OPERATION_SEND_OBJECT_INFO:
                case MTP_OPERATION_SEND_OBJECT:
                case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
                case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_REFERENCES:
                case MTP_OPERATION_MOVE_OBJECT:
                    if (g_mtp.mutexUsbToDiskTask == 1U)
                    {
                        status = kStatus_USB_Error;
                    }
                    break;

                default:
                    /* no action */
                    break;
            }

            if (status != kStatus_USB_Success)
            {
                break; /* return */
            }
        }

        /* Transaction and Session ID is correct, continue to proccess command. */
        switch (mtpHandle->mtpContainer->code)
        {
            case MTP_OPERATION_OPEN_SESSION:
                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                    if (0U == mtpHandle->mtpContainer->param[0])
                    {
                        dataInfo->code = MTP_RESPONSE_INVALID_PARAMETER;
                    }

                    if (mtpHandle->sessionID == mtpHandle->mtpContainer->param[0])
                    {
                        dataInfo->code = MTP_RESPONSE_SESSION_ALREADY_OPEN;
                    }

                    mtpHandle->sessionID = mtpHandle->mtpContainer->param[0];
                }

                USB_DeviceCmdOpenSession(dataInfo);
                break;

            case MTP_OPERATION_CLOSE_SESSION:
                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                    if (0U == mtpHandle->sessionID)
                    {
                        dataInfo->code = MTP_RESPONSE_SESSION_NOT_OPEN;
                    }

                    mtpHandle->sessionID     = 0;
                    mtpHandle->transactionID = 0xFFFFFFFFU;
                }

                USB_DeviceCmdCloseSession(dataInfo);
                break;

            case MTP_OPERATION_GET_DEVICE_INFO:
            {
                usb_device_mtp_device_info_t deviceInfo;

                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    deviceInfo.functionalMode         = MTP_FUNCTIONAL_MODE_STANDARD_MODE;
                    deviceInfo.mtpExtendsions         = NULL;
                    deviceInfo.opSupported            = &g_OpSupported[0];
                    deviceInfo.opSupportedLength      = sizeof(g_OpSupported);
                    deviceInfo.eventSupported         = &g_EventSupported[0];
                    deviceInfo.eventSupportedLength   = sizeof(g_EventSupported);
                    deviceInfo.devPropSupported       = &g_DevPropSupported[0];
                    deviceInfo.devPropSupportedLength = sizeof(g_DevPropSupported);
                    deviceInfo.captureFormat          = NULL;
                    deviceInfo.captureFormatLength    = 0;
                    deviceInfo.playbackFormat         = &g_ObjFormatSupported[0];
                    deviceInfo.playbackFormatLength   = sizeof(g_ObjFormatSupported);
                    deviceInfo.manufacturer           = "NXP";
                    deviceInfo.model                  = "NXP";
                    deviceInfo.deviceVersion          = "1.0";
                    deviceInfo.serialNumber           = "0123456789ABCDEF";
                }

                USB_DeviceCmdGetDeviceInfo(dataInfo, &deviceInfo);
                break;
            }

            case MTP_OPERATION_GET_DEVICE_PROP_DESC:
                USB_DeviceCmdGetDevicePropDesc(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
                USB_DeviceCmdGetObjPropsSupported(dataInfo);
                break;

            case MTP_OPERATION_GET_STORAGE_IDS:
                USB_DeviceCmdGetStorageIDs(dataInfo);
                break;

            case MTP_OPERATION_GET_STORAGE_INFO:
                USB_DeviceCmdGetStorageInfo(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_HANDLES:
                USB_DeviceCmdGetObjHandles(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_DESC:
                USB_DeviceCmdGetObjPropDesc(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_LIST:
                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    /* Object Property Code and Object Property Group Code equals to 0. */
                    if ((mtpHandle->mtpContainer->param[2] == 0U) && (mtpHandle->mtpContainer->param[3] == 0U))
                    {
                        dataInfo->code = MTP_RESPONSE_PARAMETER_NOT_SUPPORTED;
                    }
                }
                USB_DeviceCmdGetObjPropList(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_INFO:
                USB_DeviceCmdGetObjInfo(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT:
                USB_DeviceCmdGetObj(dataInfo);
                break;

            case MTP_OPERATION_SEND_OBJECT_INFO:
                USB_DeviceCmdSendObjInfo(dataInfo);
                break;

            case MTP_OPERATION_SEND_OBJECT:
                USB_DeviceCmdSendObj(dataInfo);
                break;

            case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
                USB_DeviceCmdGetDevicePropVal(dataInfo);
                break;

            case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
                USB_DeviceCmdSetDevicePropVal(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
                USB_DeviceCmdGetObjPropVal(dataInfo);
                break;

            case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
                USB_DeviceCmdSetObjPropVal(dataInfo);
                break;

            case MTP_OPERATION_GET_OBJECT_REFERENCES:
                USB_DeviceCmdGetObjReferences(dataInfo);
                break;

            case MTP_OPERATION_MOVE_OBJECT:
                mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;
                USB_DeviceCmdMoveObj(dataInfo);
                break;

            case MTP_OPERATION_DELETE_OBJECT:
                USB_DeviceMtpQueuePut((usb_device_mtp_cmd_data_struct_t *)dataInfo, kUSB_DeviceMtpEventDeleteObj);
                break;

            case MTP_OPERATION_COPY_OBJECT:
                USB_DeviceMtpQueuePut((usb_device_mtp_cmd_data_struct_t *)dataInfo, kUSB_DeviceMtpEventCopyObj);
                break;

            default:
                if (dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND)
                {
                    /* Invalid command, need to stall Bulk-in & Bulk-out endpoints */
                    dataInfo->code = MTP_RESPONSE_OPERATION_NOT_SUPPORTED;
                }
                break;
        }

        /* Prime first data block */
        if ((dataInfo->curPhase == USB_DEVICE_MTP_PHASE_COMMAND) && (dataInfo->code == MTP_RESPONSE_UNDEFINED) &&
            (status == kStatus_USB_Success))
        {
            switch (mtpHandle->mtpContainer->code)
            {
                case MTP_OPERATION_GET_DEVICE_INFO:
                case MTP_OPERATION_GET_DEVICE_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROPS_SUPPORTED:
                case MTP_OPERATION_GET_STORAGE_IDS:
                case MTP_OPERATION_GET_STORAGE_INFO:
                case MTP_OPERATION_GET_OBJECT_HANDLES:
                case MTP_OPERATION_GET_OBJECT_PROP_DESC:
                case MTP_OPERATION_GET_OBJECT_PROP_LIST:
                case MTP_OPERATION_GET_OBJECT_INFO:
                case MTP_OPERATION_GET_OBJECT:
                case MTP_OPERATION_GET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_PROP_VALUE:
                case MTP_OPERATION_GET_OBJECT_REFERENCES:
                    USB_DevicePrimeDataIn(mtpHandle, dataInfo); /* C */
                    break;

                case MTP_OPERATION_SEND_OBJECT_INFO:
                case MTP_OPERATION_SEND_OBJECT:
                case MTP_OPERATION_SET_DEVICE_PROP_VALUE:
                case MTP_OPERATION_SET_OBJECT_PROP_VALUE:
                    USB_DevicePrimeDataOut(mtpHandle, dataInfo); /* C */
                    break;

                default:
                    /* no action */
                    break;
            }
        }
    } while (0);

    if (MTP_RESPONSE_UNDEFINED != dataInfo->code)
    {
        if (USB_DEVICE_MTP_STATE_RESPONSE == mtpHandle->mtpState) /* C */
        {
            /* Command-Response or Command-Data-Response transaction, prime response block here. */
#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
            if (kStatus_USB_Success != USB_DeviceMtpPrimeResponse(mtpHandle, dataInfo->code,
                                                                  (uint32_t *)&dataInfo->param[0], dataInfo->curSize))
            {
#if (defined(DEVICE_ECHO) && (DEVICE_ECHO > 0U))
                usb_echo("prime response error\r\n");
#endif
            }
#else
            (void)USB_DeviceMtpPrimeResponse(mtpHandle, dataInfo->code, (uint32_t *)&dataInfo->param[0],
                                             dataInfo->curSize);
#endif
        }
        else
        {
            /* Command-Data-Response transaction, error, stall Bulk-in & Bulk-out endpoints. */
            USB_DevicePrimeBulkInAndOutStall(mtpHandle, dataInfo->code, 0);
        }
    }
}

/*!
 * @brief Cancel current transacion.
 *
 * The function is used to cancel current transaction in the bulk in, bulk out and interrupt in endpoints.
 * The function calls USB_DeviceCancel internally. If there is a transaction ongoing, the function will call
 * callback function to inform application that the transaction is cancelled.
 *
 * @param handle The MTP class handle got from usb_device_class_config_struct_t::classHandle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpCancelCurrentTransaction(usb_device_mtp_struct_t *mtpHandle)
{
    usb_device_mtp_cmd_data_struct_t dataInfo;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

#if (defined(USB_DEVICE_CONFIG_RETURN_VALUE_CHECK) && (USB_DEVICE_CONFIG_RETURN_VALUE_CHECK > 0U))
    if ((kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkInEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkOutEndpoint)) ||
        (kStatus_USB_Success != USB_DeviceCancel(mtpHandle->handle, mtpHandle->interruptInEndpoint)))
    {
        return kStatus_USB_Error;
    }
#else
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkInEndpoint);
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->bulkOutEndpoint);
    (void)USB_DeviceCancel(mtpHandle->handle, mtpHandle->interruptInEndpoint);
#endif

    /* callback to cancel current transaction */
    dataInfo.curSize  = 0;
    dataInfo.code     = MTP_RESPONSE_UNDEFINED;
    dataInfo.curPhase = USB_DEVICE_MTP_PHASE_CANCELLATION;
    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

    return kStatus_USB_Success;
}

static void USB_DeviceMtpStateMachine(usb_device_mtp_struct_t *mtpHandle,
                                      usb_device_endpoint_callback_message_struct_t *message)
{
    usb_device_mtp_cmd_data_struct_t dataInfo;

    /* Step B: state is not changed */
    /* Step C: change to new state */

    if (message->length == USB_UNINITIALIZED_VAL_32)
    {
        /* callback to cancel current transaction */
        dataInfo.curSize  = 0U;
        dataInfo.code     = MTP_RESPONSE_UNDEFINED;
        dataInfo.curPhase = USB_DEVICE_MTP_PHASE_CANCELLATION;
        memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
        USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
        return;
    }

    switch (mtpHandle->mtpState)
    {
        case USB_DEVICE_MTP_STATE_COMMAND: /* B */
        {
            if (USB_DEVICE_MTP_STATE_BULK_OUT_STALL == mtpHandle->bulkOutStallFlag)
            {
                break;
            }

            if ((message->length >= USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH) &&
                (message->length == mtpHandle->mtpContainer->containerLength) &&
                (mtpHandle->mtpContainer->containerType == USB_DEVICE_MTP_CONTAINER_TYPE_COMMAND))
            {
                dataInfo.curSize  = 0U;
                dataInfo.code     = MTP_RESPONSE_UNDEFINED;
                dataInfo.curPhase = USB_DEVICE_MTP_PHASE_COMMAND;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                /* Invalid command, stall bulk pipe. */
                USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
            }
            break;
        }

        case USB_DEVICE_MTP_STATE_BULK_IN: /* B */
        {
            mtpHandle->transferDone += message->length;
            mtpHandle->transferOffset += message->length;
            mtpHandle->transferLength -= message->length;

            if (USB_DEVICE_MTP_STATE_BULK_IN_STALL == mtpHandle->bulkInStallFlag)
            {
                break;
            }

            if (mtpHandle->transferDone == mtpHandle->transferTotal)
            {
                /* If the number of bytes specified in the first four bytes of the Data Block are an integral multiple
                   of the wMaxPacketSize field of the Endpoint Descriptor the Data phase, will end in a NULL packet,
                   so prime zero packet here. */
                if (((mtpHandle->transferDone % mtpHandle->bulkInMaxPacketSize) == 0U) &&
                    (mtpHandle->transferLength == 0U) && (message->length != 0U))
                {
                    /* Prime NULL packet */
                    USB_DeviceSendRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0);
                    break;
                }

                /* Transfer complete */
                mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                dataInfo.curPos   = mtpHandle->transferDone;
                dataInfo.curSize  = 0;
                dataInfo.code     = MTP_RESPONSE_OK;
                dataInfo.curPhase = USB_DEVICE_MTP_PHASE_RESPONSE;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                /* Prime response block here. */
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                if (((message->length % mtpHandle->bulkInMaxPacketSize) != 0U) || (message->length == 0U))
                {
                    /* A short packet indicates the end of a Data phase.
                      If the number of bytes transferred in the Data phase is less than that specified in the first
                      four bytes of the Data Block and the data receiver detects this condition before the
                      initiation of the Response phase the data transfer may be cancelled. */
                    USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
                    break;
                }

                /* Transfer not complete */
                if (mtpHandle->transferLength != 0U)
                {
                    /* continue to transfer */
                    USB_DeviceMtpSend(mtpHandle);
                }
                else
                {
                    /* callback to request data buffer. */
                    dataInfo.curPos   = mtpHandle->transferDone;
                    dataInfo.curSize  = 0;
                    dataInfo.code     = MTP_RESPONSE_UNDEFINED;
                    dataInfo.curPhase = USB_DEVICE_MTP_PHASE_DATA;
                    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

                    if (MTP_RESPONSE_UNDEFINED != dataInfo.code)
                    {
                        /* error, stall Bulk-in & Bulk-out endpoints. */
                    }
                    else
                    {
                        mtpHandle->transferLength = dataInfo.curSize;
                        mtpHandle->transferBuffer = dataInfo.buffer;
                        mtpHandle->transferOffset = 0U;

                        USB_DeviceMtpSend(mtpHandle);
                    }
                }
            }
            break;
        }

        case USB_DEVICE_MTP_STATE_BULK_OUT: /* B */
        {
            mtpHandle->transferDone += message->length;
            mtpHandle->transferOffset += message->length;
            mtpHandle->transferLength -= message->length;

            if (USB_DEVICE_MTP_STATE_BULK_OUT_STALL == mtpHandle->bulkOutStallFlag)
            {
                break;
            }

            if (mtpHandle->transferTotal == 0U)
            {
                /* Is the first packet. */
                mtpHandle->transferTotal = USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(message->buffer);

                /* Special process for receiving >4GB file. */
                if (mtpHandle->transferTotal == USB_DEVICE_MTP_MAX_UINT32_VAL)
                {
                    mtpHandle->transferTotal = USB_DEVICE_MTP_MAX_UINT64_VAL;
                }
            }

            if (mtpHandle->transferDone == mtpHandle->transferTotal)
            {
                /* If the number of bytes specified in the first four bytes of the Data Block are an integral multiple
                   of the wMaxPacketSize field of the Endpoint Descriptor the Data phase, will end in a NULL packet,
                   so prime zero packet here. */
                if (((mtpHandle->transferDone % mtpHandle->bulkOutMaxPacketSize) == 0U) &&
                    (mtpHandle->transferLength == 0U) && (message->length != 0U))
                {
                    /* Prime NULL packet */
                    USB_DeviceRecvRequest(mtpHandle->handle, mtpHandle->bulkOutEndpoint, NULL, 0);
                    break;
                }

                /* Transfer complete */
                mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                dataInfo.curPos   = mtpHandle->transferDone;
                dataInfo.curSize  = 0;
                dataInfo.code     = MTP_RESPONSE_OK;
                dataInfo.curPhase = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
                memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                /* Prime response block here. */
                USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
            }
            else
            {
                if (((message->length % mtpHandle->bulkOutMaxPacketSize) != 0U) || (message->length == 0U))
                {
                    /* Special process for receiving >4GB file. */
                    if ((mtpHandle->transferTotal != USB_DEVICE_MTP_MAX_UINT64_VAL) ||
                        ((mtpHandle->transferTotal == USB_DEVICE_MTP_MAX_UINT64_VAL) &&
                         (mtpHandle->transferDone < USB_DEVICE_MTP_MAX_UINT32_VAL)))
                    {
                        /* A short packet or NULL packet indicates the end of a Data phase.
                          If the number of bytes transferred in the Data phase is less than that specified in the first
                          four bytes of the Data Block and the data receiver detects this condition before the
                          initiation of the Response phase the data transfer may be cancelled. */
                        USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_TRANSACTION_CANCELLED, 1);
                        break;
                    }
                    else
                    {
                        /* Transfer complete */
                        mtpHandle->mtpState = USB_DEVICE_MTP_STATE_RESPONSE;

                        dataInfo.curPos   = mtpHandle->transferDone;
                        dataInfo.curSize  = 0;
                        dataInfo.code     = MTP_RESPONSE_OK;
                        dataInfo.curPhase = USB_DEVICE_MTP_CONTAINER_TYPE_RESPONSE;
                        memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                        /* Prime response block here. */
                        USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);
                        break;
                    }
                }

                /* Transfer not complete */
                if (mtpHandle->transferLength != 0U)
                {
                    /* continue to transfer */
                    USB_DeviceMtpRecv(mtpHandle);
                }
                else
                {
                    /* callback to request data buffer. */
                    dataInfo.totalSize = mtpHandle->transferTotal;
                    dataInfo.curPos    = mtpHandle->transferDone;
                    dataInfo.curSize   = 0;
                    dataInfo.code      = MTP_RESPONSE_UNDEFINED;
                    dataInfo.curPhase  = USB_DEVICE_MTP_PHASE_DATA;
                    memcpy(&dataInfo.param[0], &mtpHandle->mtpContainer->param[0], sizeof(dataInfo.param));

                    USB_DeviceMtpProcessCommand(mtpHandle, &dataInfo);

                    if (MTP_RESPONSE_UNDEFINED != dataInfo.code)
                    {
                        /* error, stall Bulk-in & Bulk-out endpoints. */
                    }
                    else
                    {
                        mtpHandle->transferLength = dataInfo.curSize;
                        mtpHandle->transferBuffer = dataInfo.buffer;
                        mtpHandle->transferOffset = 0U;

                        USB_DeviceMtpRecv(mtpHandle);
                    }
                }
            }

            break;
        }

        case USB_DEVICE_MTP_STATE_RESPONSE: /* B */
        {
            if (USB_DEVICE_MTP_STATE_BULK_IN_STALL == mtpHandle->bulkInStallFlag)
            {
                break;
            }

            if ((message->length >= USB_DEVICE_MTP_MINIMUM_CONTAINER_LENGTH) &&
                (message->length == mtpHandle->mtpContainer->containerLength))
            {
                USB_DeviceMtpPrimeCommand(mtpHandle); /* C */
            }
            else
            {
                USB_DevicePrimeBulkInAndOutStall(mtpHandle, MTP_RESPONSE_GENERAL_ERROR, 0);
            }

            break;
        }

        default:
            /* No action here */
            break;
    }
}

/*!
 * @brief Bulk IN endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Bulk IN pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Bulk IN pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpBulkIn(usb_device_handle handle,
                                 usb_device_endpoint_callback_message_struct_t *message,
                                 void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    USB_DeviceMtpStateMachine(mtpHandle, message);

    return error;
}

/*!
 * @brief Bulk OUT endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Bulk OUT pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Bulk OUT pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpBulkOut(usb_device_handle handle,
                                  usb_device_endpoint_callback_message_struct_t *message,
                                  void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    USB_DeviceMtpStateMachine(mtpHandle, message);

    return error;
}

/*!
 * @brief Interrupt IN endpoint callback function.
 *
 * This callback function is used to notify upper layer the transfer result of a transfer.
 * This callback pointer is passed when the Interrupt IN pipe initialized.
 *
 * @param handle          The device handle. It equals the value returned from USB_DeviceInit.
 * @param message         The result of the Interrupt IN pipe transfer.
 * @param callbackParam  The parameter for this callback. It is same with
 * usb_device_endpoint_callback_struct_t::callbackParam. In the class, the value is the MTP class handle.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpInterruptIn(usb_device_handle handle,
                                      usb_device_endpoint_callback_message_struct_t *message,
                                      void *callbackParam)
{
    usb_device_mtp_struct_t *mtpHandle = (usb_device_mtp_struct_t *)callbackParam;
    usb_status_t error                 = kStatus_USB_Success;

    if (NULL == mtpHandle)
    {
        return kStatus_USB_InvalidHandle;
    }

    mtpHandle->interruptInBusy = 0U;

    /* Notify the application data sent */

    return error;
}

/*!
 * @brief Initialize the endpoints of the mtp class.
 *
 * This callback function is used to initialize the endpoints of the mtp class.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpEndpointsInit(void)
{
    usb_status_t error = kStatus_USB_Error;
    usb_device_endpoint_init_struct_t epInitStruct;
    usb_device_endpoint_callback_struct_t epCallback;

    /* Bulk In */
    epCallback.callbackFn    = USB_DeviceMtpBulkIn;
    epCallback.callbackParam = (void *)&g_mtp.mtpStruct;

    epInitStruct.zlt          = 0;
    epInitStruct.interval     = 0;
    epInitStruct.transferType = USB_ENDPOINT_BULK;
    epInitStruct.endpointAddress =
        USB_MTP_BULK_IN_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    g_mtpHandle->bulkInEndpoint = epInitStruct.endpointAddress;
    if (USB_SPEED_HIGH == g_mtp.speed)
    {
        epInitStruct.maxPacketSize = HS_MTP_BULK_IN_PACKET_SIZE;
    }
    else
    {
        epInitStruct.maxPacketSize = FS_MTP_BULK_IN_PACKET_SIZE;
    }

    g_mtpHandle->bulkInMaxPacketSize = epInitStruct.maxPacketSize;
    error                            = USB_DeviceInitEndpoint(g_mtp.deviceHandle, &epInitStruct, &epCallback);

    /* Bulk Out */
    epCallback.callbackFn    = USB_DeviceMtpBulkOut;
    epCallback.callbackParam = (void *)&g_mtp.mtpStruct;

    epInitStruct.zlt          = 0;
    epInitStruct.interval     = 0;
    epInitStruct.transferType = USB_ENDPOINT_BULK;
    epInitStruct.endpointAddress =
        USB_MTP_BULK_OUT_ENDPOINT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    g_mtpHandle->bulkOutEndpoint = epInitStruct.endpointAddress;
    if (USB_SPEED_HIGH == g_mtp.speed)
    {
        epInitStruct.maxPacketSize = HS_MTP_BULK_OUT_PACKET_SIZE;
    }
    else
    {
        epInitStruct.maxPacketSize = FS_MTP_BULK_OUT_PACKET_SIZE;
    }

    g_mtpHandle->bulkOutMaxPacketSize = epInitStruct.maxPacketSize;
    error                             = USB_DeviceInitEndpoint(g_mtp.deviceHandle, &epInitStruct, &epCallback);

    /* Interrupt In */
    epCallback.callbackFn    = USB_DeviceMtpInterruptIn;
    epCallback.callbackParam = (void *)&g_mtp.mtpStruct;

    epInitStruct.zlt          = 0;
    epInitStruct.transferType = USB_ENDPOINT_INTERRUPT;
    epInitStruct.endpointAddress =
        USB_MTP_INTERRUPT_IN_ENDPOINT | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
    g_mtpHandle->interruptInEndpoint = epInitStruct.endpointAddress;
    if (USB_SPEED_HIGH == g_mtp.speed)
    {
        epInitStruct.interval      = HS_MTP_INTERRUPT_IN_INTERVAL;
        epInitStruct.maxPacketSize = HS_MTP_INTERRUPT_IN_PACKET_SIZE;
    }
    else
    {
        epInitStruct.interval      = FS_MTP_INTERRUPT_IN_INTERVAL;
        epInitStruct.maxPacketSize = FS_MTP_INTERRUPT_IN_PACKET_SIZE;
    }

    error = USB_DeviceInitEndpoint(g_mtp.deviceHandle, &epInitStruct, &epCallback);

    g_mtpHandle->mtpState             = USB_DEVICE_MTP_STATE_START;
    g_mtpHandle->bulkInStallFlag      = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
    g_mtpHandle->bulkOutStallFlag     = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
    g_mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL;
    g_mtpHandle->interruptInBusy      = 0U;

    return error;
}

/*!
 * @brief De-initialize the endpoints of the mtp class.
 *
 * This callback function is used to de-initialize the endpoints of the mtp class.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceMtpEndpointsDeinit(void)
{
    usb_status_t error = kStatus_USB_Error;

    error = USB_DeviceDeinitEndpoint(g_mtp.deviceHandle, g_mtpHandle->bulkInEndpoint);
    error = USB_DeviceDeinitEndpoint(g_mtp.deviceHandle, g_mtpHandle->bulkOutEndpoint);
    error = USB_DeviceDeinitEndpoint(g_mtp.deviceHandle, g_mtpHandle->interruptInEndpoint);
    return error;
}

/*!
 * @brief device callback function.
 *
 * This function handle the usb standard event. more information, please refer to usb spec chapter 9.
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 * @return  A USB error code or kStatus_USB_Success..
 */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint8_t *temp8     = (uint8_t *)param;
    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            g_mtp.currentConfiguration = 0U;
            g_mtpHandle->sessionID     = 0;
            g_mtpHandle->transactionID = 0xFFFFFFFFU;
            USB_DeviceControlPipeInit(g_mtp.deviceHandle);
            g_mtp.attach = 0;
            error        = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceGetStatus(g_mtp.deviceHandle, kUSB_DeviceStatusSpeed, &g_mtp.speed))
            {
                USB_DeviceSetSpeed(g_mtp.speed);
            }
#endif
        }
        break;

#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
        case kUSB_DeviceEventDetach:
            USB_DeviceMtpCancelCurrentTransaction(g_mtpHandle);
            USB_DeviceCmdCloseSession(NULL);
            break;
#endif

        case kUSB_DeviceEventSetConfiguration:
            if (g_mtp.currentConfiguration == *temp8)
            {
                error = kStatus_USB_Success;
            }
            else if (USB_MTP_CONFIGURE_INDEX == (*temp8))
            {
                error = USB_DeviceMtpEndpointsInit();
                if (kStatus_USB_Success == error)
                {
                    USB_DeviceMtpPrimeCommand(g_mtpHandle);
                }
                g_mtp.attach               = 1;
                g_mtp.currentConfiguration = *temp8;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest. */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            error = kStatus_USB_Success;
            break;
        default:
            break;
    }
    return error;
}
usb_status_t USB_DeviceGetSetupBuffer(usb_device_handle handle, usb_setup_struct_t **setupBuffer)
{
    static uint32_t mtp_setup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&mtp_setup;
    return kStatus_USB_Success;
}
usb_status_t USB_DeviceConfigureRemoteWakeup(usb_device_handle handle, uint8_t enable)
{
    return kStatus_USB_InvalidRequest;
}
usb_status_t USB_DeviceConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    usb_status_t error = kStatus_USB_Error;
    if (status)
    {
        if ((USB_MTP_BULK_IN_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            if (g_mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL)
            {
                g_mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_STALL;
                error                        = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else if ((USB_MTP_BULK_OUT_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (!(ep & 0x80)))
        {
            if (g_mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL)
            {
                g_mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_STALL;
                error                         = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else if ((USB_MTP_INTERRUPT_IN_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            if (g_mtpHandle->interruptInStallFlag == USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL)
            {
                g_mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL;
                error                             = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else
        {
        }
    }
    else
    {
        if ((USB_MTP_BULK_IN_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            if (g_mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL)
            {
                g_mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
                error                        = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else if ((USB_MTP_BULK_OUT_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (!(ep & 0x80)))
        {
            if (g_mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL)
            {
                g_mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
                error                         = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else if ((USB_MTP_INTERRUPT_IN_ENDPOINT == (ep & USB_ENDPOINT_NUMBER_MASK)) && (ep & 0x80))
        {
            if (g_mtpHandle->interruptInStallFlag == USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL)
            {
                g_mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
                error                             = USB_DeviceStallEndpoint(handle, ep);
            }
        }
        else
        {
        }
        if ((g_mtpHandle->bulkInStallFlag != USB_DEVICE_MTP_STATE_BULK_OUT_STALL) &&
            (g_mtpHandle->bulkOutStallFlag != USB_DEVICE_MTP_STATE_BULK_OUT_STALL))
        {
            USB_DeviceMtpPrimeCommand(g_mtpHandle);
        }
    }

    return error;
}
usb_status_t USB_DeviceGetClassReceiveBuffer(usb_device_handle handle,
                                             usb_setup_struct_t *setup,
                                             uint32_t *length,
                                             uint8_t **buffer)
{
    if ((NULL == buffer) || ((*length) > sizeof(s_SetupOutBuffer)))
    {
        return kStatus_USB_InvalidRequest;
    }
    *buffer = s_SetupOutBuffer;
    return kStatus_USB_Success;
}
usb_status_t USB_DeviceProcessVendorRequest(usb_device_handle handle,
                                            usb_setup_struct_t *setup,
                                            uint32_t *length,
                                            uint8_t **buffer)
{
    return kStatus_USB_InvalidRequest;
}
usb_status_t USB_DeviceGetVendorReceiveBuffer(usb_device_handle handle,
                                              usb_setup_struct_t *setup,
                                              uint32_t *length,
                                              uint8_t **buffer)
{
    return kStatus_USB_Error;
}
usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;

    if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
    {
        return error;
    }

    switch (setup->bRequest)
    {
        case USB_DEVICE_MTP_CANCEL_REQUEST:
            if ((setup->wIndex == USB_MTP_INTERFACE_INDEX) && (0U == g_mtpHandle->isHostCancel) &&
                (0U == setup->wValue) && (setup->wLength == 0x0006U) &&
                ((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT))
            {
                error = kStatus_USB_Success;

                if ((((usb_device_mtp_cancel_request_t *)&s_SetupOutBuffer[0])->code == MTP_EVENT_CANCEL_TRANSACTION) &&
                    (USB_LONG_FROM_LITTLE_ENDIAN_ADDRESS(
                         ((uint8_t *)&((usb_device_mtp_cancel_request_t *)&s_SetupOutBuffer[0])->transactionID)) ==
                     g_mtpHandle->transactionID))
                {
                    /* Host cancels current transaction */
                    g_mtpHandle->isHostCancel = 1;
                    USB_DeviceMtpCancelCurrentTransaction(g_mtpHandle);
                }
            }
            break;
        case USB_DEVICE_MTP_GET_EXTENDED_EVENT_DATA:
            if ((setup->wIndex == USB_MTP_INTERFACE_INDEX) && (0U == setup->wValue) && (0U != setup->wLength) &&
                ((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN))
            {
            }
            break;
        case USB_DEVICE_MTP_DEVICE_RESET_REQUEST:
            if ((setup->wIndex == USB_MTP_INTERFACE_INDEX) && (0U == setup->wValue) && (0U == setup->wLength) &&
                ((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_OUT))
            {
                /* The DEVICE clears its command buffer, closes all open sessions, and returns to the
                   Configured State. */
                g_mtpHandle->transactionID = 0xFFFFFFFFU;
                g_mtpHandle->sessionID     = 0U;

                if (g_mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL)
                {
                    g_mtpHandle->bulkInStallFlag = USB_DEVICE_MTP_STATE_BULK_IN_UNSTALL;
                    error                        = USB_DeviceUnstallEndpoint(handle, g_mtpHandle->bulkInEndpoint);
                    if (kStatus_USB_Success != error)
                    {
                        break;
                    }
                }
                if (g_mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL)
                {
                    g_mtpHandle->bulkOutStallFlag = USB_DEVICE_MTP_STATE_BULK_OUT_UNSTALL;
                    error                         = USB_DeviceUnstallEndpoint(handle, g_mtpHandle->bulkOutEndpoint);
                    if (kStatus_USB_Success != error)
                    {
                        break;
                    }
                }
                if (g_mtpHandle->interruptInStallFlag == USB_DEVICE_MTP_STATE_INTERRUPT_IN_STALL)
                {
                    g_mtpHandle->interruptInStallFlag = USB_DEVICE_MTP_STATE_INTERRUPT_IN_UNSTALL;
                    error = USB_DeviceUnstallEndpoint(handle, g_mtpHandle->interruptInEndpoint);
                    if (kStatus_USB_Success != error)
                    {
                        break;
                    }
                }

                /* Receiving class specific reset request, the device clears its command buffer,
                   closes all open sessions, and returns to the configured State. */
                USB_DeviceCmdCloseSession(NULL);

                USB_DeviceMtpPrimeCommand(g_mtpHandle);
            }
            break;
        case USB_DEVICE_MTP_GET_DEVICE_STATUS_REQUEST:
            if ((setup->wIndex == USB_MTP_INTERFACE_INDEX) && (0U == setup->wValue) && (4U <= setup->wLength) &&
                ((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN))
            {
                if ((g_mtpHandle->bulkInStallFlag == USB_DEVICE_MTP_STATE_BULK_IN_STALL) ||
                    (g_mtpHandle->bulkOutStallFlag == USB_DEVICE_MTP_STATE_BULK_OUT_STALL))
                {
                    /* do nothing, device status has been built before. */
                }
                else if (0U != g_mtpHandle->isHostCancel)
                {
                    g_mtpHandle->deviceStatus->wLength = 4U;
                    g_mtpHandle->deviceStatus->code    = MTP_RESPONSE_DEVICE_BUSY;

                    g_mtpHandle->isHostCancel = 0;
                    USB_DeviceMtpPrimeCommand(g_mtpHandle);
                }
                else
                {
                    g_mtpHandle->deviceStatus->wLength = 4U;
                    g_mtpHandle->deviceStatus->code    = MTP_RESPONSE_OK;
                }

                *buffer = (uint8_t *)g_mtpHandle->deviceStatus;
                *length = g_mtpHandle->deviceStatus->wLength;

                error = kStatus_USB_Success;
            }
            break;
        default:
            /*no action*/
            break;
    }

    return error;
}

/*!
 * @brief device application init function.
 *
 * This function init the usb stack and sdhc driver.
 *
 * @return None.
 */
void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    usb_echo("Please insert disk\r\n");

    g_mtp.devPropDescList = &g_DevPropDescList;
    g_mtp.storageList     = &g_StorageList;
    g_mtp.objPropList     = &g_ObjPropList;
    g_mtp.devFriendlyName = &g_DevFriendlyName[0];
    g_mtp.path            = (uint8_t *)&g_pathBuffer[0];

    g_mtp.speed              = USB_SPEED_FULL;
    g_mtp.attach             = 0;
    g_mtp.deviceHandle       = NULL;
    g_mtp.mutexUsbToDiskTask = 0U;

    USB_DeviceMtpInitQueue();

    if (kStatus_USB_Success != USB_DeviceMtpFSInit((const uint16_t *)g_mtp.storageList->storageInfo[0].rootPath))
    {
        usb_echo("Disk init failed\r\n");
    }

    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_mtp.deviceHandle))
    {
        usb_echo("USB device init failed\r\n");
    }
    else
    {
        usb_echo("USB device mtp demo\r\n");
    }

    g_mtpHandle->handle         = g_mtp.deviceHandle;
    g_mtpHandle->mtpContainer   = &g_mtpContainer;
    g_mtpHandle->eventContainer = &g_mtpEvent;
    g_mtpHandle->deviceStatus   = &g_mtpStatus;
    g_mtpHandle->configuration  = 0U;
    g_mtpHandle->alternate      = 0xFFU;
    g_mtpHandle->transactionID  = 0xFFFFFFFFU;
    g_mtpHandle->sessionID      = 0U;

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_mtp.deviceHandle);
}

void USB_DeviceDiskOperationTask(void)
{
    uint32_t event;
    usb_device_mtp_cmd_data_struct_t dataInfo;
    usb_device_mtp_response_struct_t response;

    if (kStatus_USB_Success == USB_DeviceMtpQueueGet(&dataInfo, &event))
    {
        g_mtp.mutexUsbToDiskTask = 1U;
        switch (event)
        {
            case kUSB_DeviceMtpEventDeleteObj:
                USB_DeviceCmdDeleteObj(&dataInfo);
                break;

            case kUSB_DeviceMtpEventCopyObj:
                USB_DeviceCmdCopyObj(&dataInfo);
                break;

            default:
                /* no action */
                break;
        }
        g_mtp.mutexUsbToDiskTask = 0U;

        if ((kStatus_USB_Success == USB_DeviceMtpQueueGet(&dataInfo, &event)) &&
            ((dataInfo.curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION) ||
             (event == kUSB_DeviceMtpEventDeviceResetRequest)))
        {
            /* If receiving cancellation or reset request during the above opertions, do not send reponse */
        }
        else
        {
            response.code        = dataInfo.code;
            response.paramNumber = dataInfo.curSize;
            while (dataInfo.curSize != 0U)
            {
                dataInfo.curSize--;
                ((uint32_t *)&response.param1)[dataInfo.curSize] = dataInfo.param[dataInfo.curSize];
            }

            USB_DeviceMtpResponseSend(g_mtpHandle, &response);
        }
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    BOARD_ConfigMPU();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_SD_Config(&g_sd, NULL, USB_DEVICE_INTERRUPT_PRIORITY - 1U, NULL);
    BOARD_InitDebugConsole();
    USB_DeviceApplicationInit();

#if (defined(USB_DEVICE_CONFIG_USE_TASK) && (USB_DEVICE_CONFIG_USE_TASK > 0)) && \
    (defined(USB_DEVICE_CONFIG_USE_EVENT_TASK) && (USB_DEVICE_CONFIG_USE_EVENT_TASK > 0))
    USB_DeviceEventInit();
#endif

    while (1)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_mtp.deviceHandle);
#endif
        USB_DeviceDiskOperationTask();

#if (defined(USB_DEVICE_CONFIG_USE_TASK) && (USB_DEVICE_CONFIG_USE_TASK > 0)) && \
    (defined(USB_DEVICE_CONFIG_USE_EVENT_TASK) && (USB_DEVICE_CONFIG_USE_EVENT_TASK > 0))
        USB_DeviceEventTask();
#endif
    }
}
