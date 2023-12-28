/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
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
#include "usb_device_hid.h"

#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"

#include "composite.h"

#include "hid_keyboard.h"
#include "hid_mouse.h"

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if (USB_DEVICE_CONFIG_HID < 2U)
#error USB_DEVICE_CONFIG_HID need to > 1U, Please change the MARCO USB_DEVICE_CONFIG_HID in file "usb_device_config.h".
#endif

#include "usb_phy.h"
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

static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
static void USB_DeviceApplicationInit(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern usb_device_class_struct_t g_UsbDeviceHidMouseConfig;
extern usb_device_class_struct_t g_UsbDeviceHidKeyboardConfig;

usb_device_composite_struct_t g_UsbDeviceComposite;

/* Set class configurations */
usb_device_class_config_struct_t g_CompositeClassConfig[USB_COMPOSITE_INTERFACE_COUNT] = {
    {
        USB_DeviceHidKeyboardCallback, /* HID keyboard class callback pointer */
        (class_handle_t)NULL,          /* The HID class handle, This field is set by USB_DeviceClassInit */
        &g_UsbDeviceHidKeyboardConfig, /* The HID keyboard configuration, including class code, subcode, and protocol,
                                  class
                                  type, transfer type, endpoint address, max packet size, etc.*/
    },
    {
        USB_DeviceHidMouseCallback, /* HID mouse class callback pointer */
        (class_handle_t)NULL,       /* The HID class handle, This field is set by USB_DeviceClassInit */
        &g_UsbDeviceHidMouseConfig, /* The HID mouse configuration, including class code, subcode, and protocol, class
                               type,
                               transfer type, endpoint address, max packet size, etc.*/
    }};

/* Set class configuration list */
usb_device_class_config_list_struct_t g_UsbDeviceCompositeConfigList = {
    g_CompositeClassConfig,        /* Class configurations */
    USB_DeviceCallback,            /* Device callback pointer */
    USB_COMPOSITE_INTERFACE_COUNT, /* Class count */
};

/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceComposite.deviceHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(g_UsbDeviceComposite.deviceHandle);
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

/* The Device callback */
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint16_t *temp16   = (uint16_t *)param;
    uint8_t *temp8     = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
        {
            /* USB bus reset signal detected */
            g_UsbDeviceComposite.attach               = 0U;
            g_UsbDeviceComposite.currentConfiguration = 0U;
            error                                     = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_UsbDeviceComposite.speed))
            {
                USB_DeviceSetSpeed(handle, g_UsbDeviceComposite.speed);
            }
#endif
        }
        break;
        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                g_UsbDeviceComposite.attach               = 0U;
                g_UsbDeviceComposite.currentConfiguration = 0U;
                error                                     = kStatus_USB_Success;
            }
            else if (USB_COMPOSITE_CONFIGURE_INDEX == (*temp8))
            {
                /* Set device configuration request */
                g_UsbDeviceComposite.attach = 1U;
                USB_DeviceHidMouseSetConfigure(g_UsbDeviceComposite.hidMouseHandle, *temp8);
                USB_DeviceHidKeyboardSetConfigure(g_UsbDeviceComposite.hidKeyboardHandle, *temp8);
                g_UsbDeviceComposite.currentConfiguration = *temp8;
                error                                     = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest. */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (g_UsbDeviceComposite.attach)
            {
                /* Set device interface request */
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (USB_HID_MOUSE_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_HID_MOUSE_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_UsbDeviceComposite.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        USB_DeviceHidMouseSetInterface(g_UsbDeviceComposite.hidMouseHandle, interface, alternateSetting);
                        error = kStatus_USB_Success;
                    }
                }
                else if (USB_HID_KEYBOARD_INTERFACE_INDEX == interface)
                {
                    if (alternateSetting < USB_HID_KEYBOARD_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_UsbDeviceComposite.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        USB_DeviceHidKeyboardSetInterface(g_UsbDeviceComposite.hidKeyboardHandle, interface,
                                                          alternateSetting);
                        error = kStatus_USB_Success;
                    }
                }
                else
                {
                    /* no action, return kStatus_USB_InvalidRequest. */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get current configuration request */
                *temp8 = g_UsbDeviceComposite.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_COMPOSITE_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | g_UsbDeviceComposite.currentInterfaceAlternateSetting[interface];
                    error   = kStatus_USB_Success;
                }
            }
            break;
        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                error = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                error = USB_DeviceGetConfigurationDescriptor(handle,
                                                             (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidDescriptor:
            if (param)
            {
                /* Get hid descriptor request */
                error = USB_DeviceGetHidDescriptor(handle, (usb_device_get_hid_descriptor_struct_t *)param);
            }
            break;
        case kUSB_DeviceEventGetHidReportDescriptor:
            if (param)
            {
                /* Get hid report descriptor request */
                error =
                    USB_DeviceGetHidReportDescriptor(handle, (usb_device_get_hid_report_descriptor_struct_t *)param);
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
        case kUSB_DeviceEventGetHidPhysicalDescriptor:
            if (param)
            {
                /* Get hid physical descriptor request */
                error = USB_DeviceGetHidPhysicalDescriptor(handle,
                                                           (usb_device_get_hid_physical_descriptor_struct_t *)param);
            }
            break;
        default:
            break;
    }

    return error;
}

/* Application initialization */
static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Set composite device to default state */
    g_UsbDeviceComposite.speed             = USB_SPEED_FULL;
    g_UsbDeviceComposite.attach            = 0U;
    g_UsbDeviceComposite.hidMouseHandle    = (class_handle_t)NULL;
    g_UsbDeviceComposite.hidKeyboardHandle = (class_handle_t)NULL;
    g_UsbDeviceComposite.deviceHandle      = NULL;

    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDeviceCompositeConfigList, &g_UsbDeviceComposite.deviceHandle))
    {
        usb_echo("USB device composite demo init failed\r\n");
        return;
    }
    else
    {
        usb_echo("USB device composite demo\r\n");
        /* Get the HID keyboard class handle */
        g_UsbDeviceComposite.hidKeyboardHandle = g_UsbDeviceCompositeConfigList.config[0].classHandle;
        /* Get the HID mouse class handle */
        g_UsbDeviceComposite.hidMouseHandle = g_UsbDeviceCompositeConfigList.config[1].classHandle;

        USB_DeviceHidKeyboardInit(&g_UsbDeviceComposite);
        USB_DeviceHidMouseInit(&g_UsbDeviceComposite);
    }

    USB_DeviceIsrEnable();

    /* Start the device function */
    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_UsbDeviceComposite.deviceHandle);
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

    while (1U)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_UsbDeviceComposite.deviceHandle);
#endif
    }
}
