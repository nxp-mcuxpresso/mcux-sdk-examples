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

#include "usb_device_class.h"
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

typedef struct _usb_mtp_manipulate_disk_msgq_struct
{
    usb_device_mtp_cmd_data_struct_t dataInfo;
    uint32_t event;
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
extern void USB_DeviceEventTask(void *arg);
#endif
#endif

#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
void USB_DeviceHsPhyChirpIssueWorkaround(void);
void USB_DeviceDisconnected(void);
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

/* Data structure of mtp device, store the information, such as class handle */
usb_mtp_struct_t g_mtp;

/* The buffer is used to build path, please make sure the buffer have enough space to accommodate the longest path.
   If the path length exceeds MTP_PATH_MAX_LEN, the current transaction will end with a failure. */
USB_DMA_NONINIT_DATA_ALIGN(2U) uint16_t g_pathBuffer[MTP_PATH_MAX_LEN >> 1U];
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint32_t g_mtpTransferBuffer[USB_DEVICE_MTP_TRANSFER_BUFF_SIZE >> 2];
/*******************************************************************************
 * Code
 ******************************************************************************/
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U))
void USB1_HS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_mtp.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U))
void USB0_FS_IRQHandler(void)
{
    USB_DeviceKhciIsrFunction(g_mtp.deviceHandle);
}
#endif

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    SPC0->ACTIVE_VDELAY = 0x0500;
    /* Change the power DCDC to 1.8v (By deafult, DCDC is 1.8V), CORELDO to 1.1v (By deafult, CORELDO is 1.0V) */
    SPC0->ACTIVE_CFG &= ~SPC_ACTIVE_CFG_CORELDO_VDD_DS_MASK;
    SPC0->ACTIVE_CFG |= SPC_ACTIVE_CFG_DCDC_VDD_LVL(0x3) | SPC_ACTIVE_CFG_CORELDO_VDD_LVL(0x3) |
                        SPC_ACTIVE_CFG_SYSLDO_VDD_DS_MASK | SPC_ACTIVE_CFG_DCDC_VDD_DS(0x2u);
    /* Wait until it is done */
    while (SPC0->SC & SPC_SC_BUSY_MASK)
        ;
    if (0u == (SCG0->LDOCSR & SCG_LDOCSR_LDOEN_MASK))
    {
        SCG0->TRIM_LOCK = 0x5a5a0001U;
        SCG0->LDOCSR |= SCG_LDOCSR_LDOEN_MASK;
        /* wait LDO ready */
        while (0U == (SCG0->LDOCSR & SCG_LDOCSR_VOUT_OK_MASK))
            ;
    }
    SYSCON->AHBCLKCTRLSET[2] |= SYSCON_AHBCLKCTRL2_USB_HS_MASK | SYSCON_AHBCLKCTRL2_USB_HS_PHY_MASK;
    SCG0->SOSCCFG &= ~(SCG_SOSCCFG_RANGE_MASK | SCG_SOSCCFG_EREFS_MASK);
    /* xtal = 20 ~ 30MHz */
    SCG0->SOSCCFG = (1U << SCG_SOSCCFG_RANGE_SHIFT) | (1U << SCG_SOSCCFG_EREFS_SHIFT);
    SCG0->SOSCCSR |= SCG_SOSCCSR_SOSCEN_MASK;
    while (1)
    {
        if (SCG0->SOSCCSR & SCG_SOSCCSR_SOSCVLD_MASK)
        {
            break;
        }
    }
    SYSCON->CLOCK_CTRL |= SYSCON_CLOCK_CTRL_CLKIN_ENA_MASK | SYSCON_CLOCK_CTRL_CLKIN_ENA_FM_USBH_LPT_MASK;
    CLOCK_EnableClock(kCLOCK_UsbHs);
    CLOCK_EnableClock(kCLOCK_UsbHsPhy);
    CLOCK_EnableUsbhsPhyPllClock(kCLOCK_Usbphy480M, 24000000U);
    CLOCK_EnableUsbhsClock();
    USB_EhciPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    CLOCK_AttachClk(kCLK_48M_to_USB0);
    CLOCK_EnableClock(kCLOCK_Usb0Ram);
    CLOCK_EnableClock(kCLOCK_Usb0Fs);
    CLOCK_EnableUsbfsClock();
#endif
}

void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USBFS_IRQS;
    irqNumber                  = usbDeviceKhciIrq[CONTROLLER_ID - kUSB_ControllerKhci0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    USB_DeviceEhciTaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    USB_DeviceKhciTaskFunction(deviceHandle);
#endif
}
#endif

/*!
 * @brief device mtp callback function.
 *
 * This function handle the disk class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
usb_status_t USB_DeviceMtpCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;

    switch (event)
    {
        case kUSB_DeviceMtpEventOpenSession:
        case kUSB_DeviceMtpEventCloseSession:
        case kUSB_DeviceMtpEventGetDeviceInfo:
        case kUSB_DeviceMtpEventGetDevicePropDesc:
        case kUSB_DeviceMtpEventGetObjPropsSupported:
        case kUSB_DeviceMtpEventGetStorageIDs:
        case kUSB_DeviceMtpEventGetStorageInfo:
        case kUSB_DeviceMtpEventGetObjHandles:
        case kUSB_DeviceMtpEventGetObjPropDesc:
        case kUSB_DeviceMtpEventGetObjPropList:
        case kUSB_DeviceMtpEventGetObjInfo:
        case kUSB_DeviceMtpEventGetObj:
        case kUSB_DeviceMtpEventSendObjInfo:
        case kUSB_DeviceMtpEventSendObj:
        case kUSB_DeviceMtpEventGetDevicePropVal:
        case kUSB_DeviceMtpEventSetDevicePropVal:
        case kUSB_DeviceMtpEventGetObjPropVal:
        case kUSB_DeviceMtpEventSetObjPropVal:
        case kUSB_DeviceMtpEventGetObjReferences:
        case kUSB_DeviceMtpEventMoveObj:
            if (g_mtp.mutexUsbToDiskTask == 1U)
            {
                error = kStatus_USB_Error;
            }
            break;

        default:
            /* no action */
            break;
    }

    if (error != kStatus_USB_Success)
    {
        return error;
    }

    switch (event)
    {
        case kUSB_DeviceMtpEventDeviceResetRequest:
            /* Receiving class specific reset request, the device clears its command buffer,
               closes all open sessions, and returns to the configured State. */
            USB_DeviceCmdCloseSession(param);
            break;

        case kUSB_DeviceMtpEventGetExtendedEventData:
            error = kStatus_USB_InvalidRequest;
            break;

        case kUSB_DeviceMtpEventOpenSession:
            USB_DeviceCmdOpenSession(param);
            break;

        case kUSB_DeviceMtpEventCloseSession:
            USB_DeviceCmdCloseSession(param);
            break;

        case kUSB_DeviceMtpEventGetDeviceInfo:
        {
            usb_device_mtp_device_info_t deviceInfo;

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

            USB_DeviceCmdGetDeviceInfo(param, &deviceInfo);
            break;
        }

        case kUSB_DeviceMtpEventGetDevicePropDesc:
            USB_DeviceCmdGetDevicePropDesc(param);
            break;

        case kUSB_DeviceMtpEventGetObjPropsSupported:
            USB_DeviceCmdGetObjPropsSupported(param);
            break;

        case kUSB_DeviceMtpEventGetStorageIDs:
            USB_DeviceCmdGetStorageIDs(param);
            break;

        case kUSB_DeviceMtpEventGetStorageInfo:
            USB_DeviceCmdGetStorageInfo(param);
            break;

        case kUSB_DeviceMtpEventGetObjHandles:
            USB_DeviceCmdGetObjHandles(param);
            break;

        case kUSB_DeviceMtpEventGetObjPropDesc:
            USB_DeviceCmdGetObjPropDesc(param);
            break;

        case kUSB_DeviceMtpEventGetObjPropList:
            USB_DeviceCmdGetObjPropList(param);
            break;

        case kUSB_DeviceMtpEventGetObjInfo:
            USB_DeviceCmdGetObjInfo(param);
            break;

        case kUSB_DeviceMtpEventGetObj:
            USB_DeviceCmdGetObj(param);
            break;

        case kUSB_DeviceMtpEventSendObjInfo:
            USB_DeviceCmdSendObjInfo(param);
            break;

        case kUSB_DeviceMtpEventSendObj:
            USB_DeviceCmdSendObj(param);
            break;

        case kUSB_DeviceMtpEventGetDevicePropVal:
            USB_DeviceCmdGetDevicePropVal(param);
            break;

        case kUSB_DeviceMtpEventSetDevicePropVal:
            USB_DeviceCmdSetDevicePropVal(param);
            break;

        case kUSB_DeviceMtpEventGetObjPropVal:
            USB_DeviceCmdGetObjPropVal(param);
            break;

        case kUSB_DeviceMtpEventSetObjPropVal:
            USB_DeviceCmdSetObjPropVal(param);
            break;

        case kUSB_DeviceMtpEventGetObjReferences:
            USB_DeviceCmdGetObjReferences(param);
            break;

        case kUSB_DeviceMtpEventMoveObj:
            USB_DeviceCmdMoveObj(param);
            break;

        case kUSB_DeviceMtpEventCopyObj:
        case kUSB_DeviceMtpEventDeleteObj:
        {
            usb_mtp_disk_operation_msgq_struct_t msgQ;

            memcpy(&msgQ.dataInfo, param, sizeof(usb_device_mtp_cmd_data_struct_t));
            msgQ.event = event;

            xQueueSendToBack(g_mtp.queueHandle, &msgQ, 0);
            break;
        }

        default:
            error = kStatus_USB_InvalidRequest;
            break;
    }

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
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            g_mtp.attach               = 0;
            g_mtp.currentConfiguration = 0U;
            error                      = kStatus_USB_Success;

#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
            /* The work-around is used to fix the HS device Chirping issue.
             * Please refer to the implementation for the detail information.
             */
            USB_DeviceHsPhyChirpIssueWorkaround();
#endif
#endif

#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_mtp.speed))
            {
                USB_DeviceSetSpeed(handle, g_mtp.speed);
            }
#endif
        }
        break;

#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
        case kUSB_DeviceEventDetach:
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
            USB_DeviceDisconnected();
#endif
#endif
            USB_DeviceMtpCancelCurrentTransaction(g_mtp.mtpHandle);
            if (0U == g_mtp.mutexUsbToDiskTask)
            {
                USB_DeviceCmdCloseSession(NULL);
            }
            error = kStatus_USB_Success;
            break;
#endif

        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_mtp.attach               = 0;
                g_mtp.currentConfiguration = 0U;
                error                      = kStatus_USB_Success;
            }
            else if (USB_MTP_CONFIGURE_INDEX == (*temp8))
            {
                g_mtp.attach               = 1;
                g_mtp.currentConfiguration = *temp8;
                error                      = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_mtp.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_MTP_INTERFACE_COUNT)
                {
                    if (alternateSetting < USB_MTP_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_mtp.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                             = kStatus_USB_Success;
                    }
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                *temp8 = g_mtp.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_mtp.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
#if (defined(USB_DEVICE_CONFIG_CV_TEST) && (USB_DEVICE_CONFIG_CV_TEST > 0U))
        case kUSB_DeviceEventGetDeviceQualifierDescriptor:
            if (param)
            {
                /* Get Qualifier descriptor request */
                error = USB_DeviceGetDeviceQualifierDescriptor(
                    handle, (usb_device_get_device_qualifier_descriptor_struct_t *)param);
            }
            break;
#endif
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }
    return error;
}
/* USB device class information */
usb_device_class_config_struct_t mtp_config[1] = {{
    USB_DeviceMtpCallback,
    0,
    &g_UsbDeviceMtpConfig,
}};
/* USB device class configuration information */
usb_device_class_config_list_struct_t mtp_config_list = {
    mtp_config,
    USB_DeviceCallback,
    1,
};

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
    g_mtp.mtpHandle          = (class_handle_t)NULL;
    g_mtp.deviceHandle       = NULL;
    g_mtp.mutexUsbToDiskTask = 0U;

    g_mtp.queueHandle = xQueueCreate(1U, sizeof(usb_mtp_disk_operation_msgq_struct_t));
    if (NULL == g_mtp.queueHandle)
    {
        usb_echo("Queue create failed\r\n");
    }

    if (kStatus_USB_Success != USB_DeviceMtpFSInit((const uint16_t *)g_mtp.storageList->storageInfo[0].rootPath))
    {
        usb_echo("Disk init failed\r\n");
    }

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &mtp_config_list, &g_mtp.deviceHandle))
    {
        usb_echo("USB device init failed\r\n");
    }
    else
    {
        usb_echo("USB device mtp demo\r\n");
        g_mtp.mtpHandle = mtp_config_list.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_mtp.deviceHandle);
}

#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTask(void *handle)
{
    while (1U)
    {
        USB_DeviceTaskFn(handle);
    }
}
#endif

void USB_DeviceDiskOperationTask(void *arg)
{
    usb_mtp_disk_operation_msgq_struct_t msgQ;
    usb_device_mtp_response_struct_t response;

    (void)memset(&msgQ, 0, sizeof(msgQ));
    while (1)
    {
        if (pdTRUE == xQueueReceive(g_mtp.queueHandle, &msgQ, portMAX_DELAY))
        {
            g_mtp.mutexUsbToDiskTask = 1U;
            switch (msgQ.event)
            {
                case kUSB_DeviceMtpEventDeleteObj:
                    USB_DeviceCmdDeleteObj(&msgQ.dataInfo);
                    break;

                case kUSB_DeviceMtpEventCopyObj:
                    USB_DeviceCmdCopyObj(&msgQ.dataInfo);
                    break;

                default:
                    /* no action */
                    break;
            }
            g_mtp.mutexUsbToDiskTask = 0U;

            if ((pdTRUE == xQueuePeek(g_mtp.queueHandle, &msgQ, 0)) &&
                ((msgQ.dataInfo.curPhase == USB_DEVICE_MTP_PHASE_CANCELLATION) ||
                 (msgQ.event == kUSB_DeviceMtpEventDeviceResetRequest)))
            {
                /* If receiving cancellation or reset request during the above opertions, do not send reponse */
            }
            else
            {
                response.code        = msgQ.dataInfo.code;
                response.paramNumber = msgQ.dataInfo.curSize;
                while (msgQ.dataInfo.curSize != 0U)
                {
                    msgQ.dataInfo.curSize--;
                    ((uint32_t *)&response.param1)[msgQ.dataInfo.curSize] = msgQ.dataInfo.param[msgQ.dataInfo.curSize];
                }

                USB_DeviceMtpResponseSend(g_mtp.mtpHandle, &response);
            }
        }
    }
}

void APP_task(void *handle)
{
    USB_DeviceApplicationInit();

    if (g_mtp.deviceHandle)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        if (xTaskCreate(USB_DeviceTask,                  /* pointer to the task */
                        (char const *)"usb device task", /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE),  /* task stack size */
                        g_mtp.deviceHandle,              /* optional task startup argument */
                        5,                               /* initial priority */
                        &g_mtp.device_task_handle        /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device task create failed!\r\n");
            return;
        }

#if USB_DEVICE_CONFIG_USE_EVENT_TASK
        if (xTaskCreate(USB_DeviceEventTask,                   /* pointer to the task */
                        (char const *)"usb device event task", /* task name for kernel awareness debugging */
                        3000L / sizeof(portSTACK_TYPE),        /* task stack size */
                        &g_mtp,                                /* optional task startup argument */
                        4,                                     /* initial priority */
                        NULL                                   /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device event task create failed!\r\n");
            return;
        }
#endif
#endif
        if (xTaskCreate(USB_DeviceDiskOperationTask,    /* pointer to the task */
                        (char const *)"usb disk task",  /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE), /* task stack size */
                        NULL,                           /* optional task startup argument */
                        4,                              /* initial priority */
                        &g_mtp.device_disk_task_handle  /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device disk task create failed!\r\n");
            return;
        }
    }

    while (1)
    {
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to USDHC */
    CLOCK_SetClkDiv(kCLOCK_DivUSdhcClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_USDHC);

    /* Enables the clock for GPIO0 */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO2 */
    CLOCK_EnableClock(kCLOCK_Gpio2);

    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);
    BOARD_SD_Config(&g_sd, NULL, USB_DEVICE_INTERRUPT_PRIORITY - 1U, NULL);

    if (xTaskCreate(APP_task,                       /* pointer to the task */
                    (char const *)"app task",       /* task name for kernel awareness debugging */
                    5000L / sizeof(portSTACK_TYPE), /* task stack size */
                    &g_mtp,                         /* optional task startup argument */
                    3,                              /* initial priority */
                    &g_mtp.application_task_handle  /* optional task handle to create */
                    ) != pdPASS)
    {
        usb_echo("app task create failed!\r\n");
#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
        return 1;
#else
        return;
#endif
    }
    vTaskStartScheduler();

#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
    return 1;
#endif
}
