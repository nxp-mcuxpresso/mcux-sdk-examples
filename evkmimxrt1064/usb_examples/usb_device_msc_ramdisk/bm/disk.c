/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2019 NXP
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
#include "usb_device_msc.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "disk.h"

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

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_inquiry_data_fromat_struct_t g_InquiryInfo = {
    (USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER << USB_DEVICE_MSC_UFI_PERIPHERAL_QUALIFIER_SHIFT) |
        USB_DEVICE_MSC_UFI_PERIPHERAL_DEVICE_TYPE,
    (uint8_t)(USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT << USB_DEVICE_MSC_UFI_REMOVABLE_MEDIUM_BIT_SHIFT),
    USB_DEVICE_MSC_UFI_VERSIONS,
    0x02,
    USB_DEVICE_MSC_UFI_ADDITIONAL_LENGTH,
    {0x00, 0x00, 0x00},
    {'N', 'X', 'P', ' ', 'S', 'E', 'M', 'I'},
    {'N', 'X', 'P', ' ', 'M', 'A', 'S', 'S', ' ', 'S', 'T', 'O', 'R', 'A', 'G', 'E'},
    {'0', '0', '0', '1'}};
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
usb_device_mode_parameters_header_struct_t g_ModeParametersHeader = {
    /*refer to ufi spec mode parameter header*/
    0x0000, /*!< Mode Data Length*/
    0x00,   /*!<Default medium type (current mounted medium type)*/
    0x00,   /*!MODE SENSE command, a Write Protected bit of zero indicates the medium is write enabled*/
    {0x00, 0x00, 0x00, 0x00} /*!<This bit should be set to zero*/
};
/* Data structure of msc device, store the information ,such as class handle */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_StorageDisk[DISK_SIZE_NORMAL];
usb_msc_struct_t g_msc;
/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_msc.deviceHandle);
    /* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
    exception return operation might vector to incorrect interrupt */
    __DSB();
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_msc.deviceHandle);
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
 * @brief device msc callback function.
 *
 * This function handle the disk class specified event.
 * @param handle          The USB class  handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the class specific event.
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceMscCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_Success;
    usb_device_lba_information_struct_t *lbaInformationStructure;
    usb_device_lba_app_struct_t *lbaData;
    usb_device_ufi_app_struct_t *ufi;
    usb_device_capacity_information_struct_t *capacityInformation;

    switch (event)
    {
        case kUSB_DeviceMscEventReadResponse:
            lbaData = (usb_device_lba_app_struct_t *)param;
            break;
        case kUSB_DeviceMscEventWriteResponse:
            lbaData = (usb_device_lba_app_struct_t *)param;
            break;
        case kUSB_DeviceMscEventWriteRequest:
            lbaData = (usb_device_lba_app_struct_t *)param;
            /*offset is the write start address get from write command, refer to class driver*/
            lbaData->buffer = g_msc.storageDisk + lbaData->offset * LENGTH_OF_EACH_LBA;
            break;
        case kUSB_DeviceMscEventReadRequest:
            lbaData = (usb_device_lba_app_struct_t *)param;
            /*offset is the read start address get from read command, refer to class driver*/
            lbaData->buffer = g_msc.storageDisk + lbaData->offset * LENGTH_OF_EACH_LBA;
            break;
        case kUSB_DeviceMscEventGetLbaInformation:
            lbaInformationStructure                             = (usb_device_lba_information_struct_t *)param;
            lbaInformationStructure->logicalUnitNumberSupported = LOGICAL_UNIT_SUPPORTED;

            lbaInformationStructure->logicalUnitInformations[0].lengthOfEachLba = LENGTH_OF_EACH_LBA;
            lbaInformationStructure->logicalUnitInformations[0].totalLbaNumberSupports =
                TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            lbaInformationStructure->logicalUnitInformations[0].bulkInBufferSize  = DISK_SIZE_NORMAL;
            lbaInformationStructure->logicalUnitInformations[0].bulkOutBufferSize = DISK_SIZE_NORMAL;
            break;
        case kUSB_DeviceMscEventTestUnitReady:
            /*change the test unit ready command's sense data if need, be careful to modify*/
            if (1U == g_msc.stop)
            {
                ufi                                    = (usb_device_ufi_app_struct_t *)param;
                ufi->requestSense->senseKey            = USB_DEVICE_MSC_UFI_NOT_READY;
                ufi->requestSense->additionalSenseCode = USB_DEVICE_MSC_UFI_ASC_MEDIUM_NOT_PRESENT;
            }
            break;
        case kUSB_DeviceMscEventInquiry:
            ufi         = (usb_device_ufi_app_struct_t *)param;
            ufi->size   = sizeof(usb_device_inquiry_data_fromat_struct_t);
            ufi->buffer = (uint8_t *)&g_InquiryInfo;
            break;
        case kUSB_DeviceMscEventModeSense:
            ufi         = (usb_device_ufi_app_struct_t *)param;
            ufi->size   = sizeof(usb_device_mode_parameters_header_struct_t);
            ufi->buffer = (uint8_t *)&g_ModeParametersHeader;
            break;
        case kUSB_DeviceMscEventModeSelectResponse:
            ufi = (usb_device_ufi_app_struct_t *)param;
            break;
        case kUSB_DeviceMscEventModeSelect:
        case kUSB_DeviceMscEventFormatComplete:
        case kUSB_DeviceMscEventRemovalRequest:
            error = kStatus_USB_InvalidRequest;
            break;
        case kUSB_DeviceMscEventRequestSense:
            break;
        case kUSB_DeviceMscEventReadCapacity:
            capacityInformation                         = (usb_device_capacity_information_struct_t *)param;
            capacityInformation->lengthOfEachLba        = LENGTH_OF_EACH_LBA;
            capacityInformation->totalLbaNumberSupports = TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            break;
        case kUSB_DeviceMscEventReadFormatCapacity:
            capacityInformation                         = (usb_device_capacity_information_struct_t *)param;
            capacityInformation->lengthOfEachLba        = LENGTH_OF_EACH_LBA;
            capacityInformation->totalLbaNumberSupports = TOTAL_LOGICAL_ADDRESS_BLOCKS_NORMAL;
            break;
        case kUSB_DeviceMscEventStopEjectMedia:
            ufi = (usb_device_ufi_app_struct_t *)param;
            if (0x00U == (ufi->cbwcb[4] & 0x01U)) /* check start bit */
            {
                g_msc.stop = 1U; /* stop command */
            }
            break;
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
 * @return kStatus_USB_Success or error.
 */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;
    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            g_msc.attach               = 0U;
            g_msc.currentConfiguration = 0U;
            g_msc.stop                 = 0U;
            error                      = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_msc.speed))
            {
                USB_DeviceSetSpeed(handle, g_msc.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_msc.attach               = 0U;
                g_msc.currentConfiguration = 0U;
                error                      = kStatus_USB_Success;
            }
            else if (USB_MSC_CONFIGURE_INDEX == (*temp8))
            {
                g_msc.attach               = 1U;
                g_msc.currentConfiguration = *temp8;
                error                      = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (0U != g_msc.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface < USB_MSC_INTERFACE_COUNT)
                {
                    if (alternateSetting < USB_MSC_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_msc.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                             = kStatus_USB_Success;
                    }
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (NULL != param)
            {
                *temp8 = g_msc.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (NULL != param)
            {
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_msc.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (NULL != param)
            {
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (NULL != param)
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
            if (NULL != param)
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
usb_device_class_config_struct_t msc_config[1] = {{
    USB_DeviceMscCallback,
    0U,
    &g_UsbDeviceMscConfig,
}};
/* USB device class configuration information */
usb_device_class_config_list_struct_t msc_config_list = {
    msc_config,
    USB_DeviceCallback,
    1U,
};

/*!
 * @brief device application init function.
 *
 * This function init the usb stack and sdhc driver.
 *
 * @return None.
 */
static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    g_msc.speed        = USB_SPEED_FULL;
    g_msc.attach       = 0U;
    g_msc.mscHandle    = (class_handle_t)NULL;
    g_msc.deviceHandle = NULL;
    g_msc.storageDisk  = &s_StorageDisk[0];

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &msc_config_list, &g_msc.deviceHandle))
    {
        usb_echo("USB device init failed\r\n");
    }
    else
    {
        usb_echo("USB device mass storage demo\r\n");
        g_msc.mscHandle = msc_config_list.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_msc.deviceHandle);
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
    BOARD_InitDebugConsole();

    USB_DeviceApplicationInit();

    while (1)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_msc.deviceHandle);
#endif
    }
}
