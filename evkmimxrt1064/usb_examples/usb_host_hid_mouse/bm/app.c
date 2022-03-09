/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "usb_host_hid.h"
#include "host_mouse.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#include "app.h"
#if (defined(USB_HOST_CONFIG_BATTERY_CHARGER) && (USB_HOST_CONFIG_BATTERY_CHARGER > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
#include "usb_charger_detect.h"
#endif

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

#include "usb_phy.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if (defined(USB_HOST_CONFIG_BATTERY_CHARGER) && (USB_HOST_CONFIG_BATTERY_CHARGER > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
char *SW_GetName(void);
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host callback function.
 *
 * device attach/detach callback function.
 *
 * @param deviceHandle          device handle.
 * @param configurationHandle   attached device's configuration descriptor information.
 * @param eventCode             callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                           usb_host_configuration_handle configurationHandle,
                           uint32_t eventCode);

/*!
 * @brief application initialization.
 */
static void USB_HostApplicationInit(void);

extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);
extern void USB_HostTaskFn(void *param);
void BOARD_InitHardware(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief USB host mouse instance global variable */
extern usb_host_mouse_instance_t g_HostHidMouse;
usb_host_handle g_HostHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

void USB_OTG1_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

void USB_OTG2_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

void USB_HostClockInit(void)
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

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
/* USB_HOST_CONFIG_EHCI */

/* Install isr, set priority, and enable IRQ. */
#if defined(__GIC_PRIO_BITS)
    GIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#else
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
#endif
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}

/*!
 * @brief USB isr function.
 */

usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                           usb_host_configuration_handle configurationHandle,
                           uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;

    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
            status = USB_HostHidMouseEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventNotSupported:
            usb_echo("device not supported.\r\n");
            break;

        case kUSB_HostEventEnumerationDone:
            status = USB_HostHidMouseEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventDetach:
            status = USB_HostHidMouseEvent(deviceHandle, configurationHandle, eventCode);
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
    uint32_t usbHostVersion;
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
#if (defined(USB_HOST_CONFIG_BATTERY_CHARGER) && (USB_HOST_CONFIG_BATTERY_CHARGER > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
    USB_HostSetChargerType(g_HostHandle, kUSB_DcdSDP);
    g_HostHidMouse.switchChargerTypeTriggered = 0U;
    g_HostHidMouse.chargerType                = 0;
    g_HostHidMouse.vbusState                  = 1;
#endif
    USB_HostIsrEnable();

    USB_HostGetVersion(&usbHostVersion);
    usb_echo("host init done, the host stack version is %d.%d.%d.\r\n", ((uint8_t)(usbHostVersion >> 16)),
             ((uint8_t)(usbHostVersion >> 8)), ((uint8_t)(usbHostVersion)));

#if (defined(USB_HOST_CONFIG_BATTERY_CHARGER) && (USB_HOST_CONFIG_BATTERY_CHARGER > 0U)) && \
    (defined(FSL_FEATURE_SOC_USBHSDCD_COUNT) && (FSL_FEATURE_SOC_USBHSDCD_COUNT > 0U))
    usb_echo("Please Press the switch(%s) to change the charge type.\r\n", SW_GetName());
#endif
}

int main(void)
{
    BOARD_ConfigMPU();

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    USB_HostApplicationInit();

    while (1)
    {
        USB_HostTaskFn(g_HostHandle);
        USB_HostHidMouseTask(&g_HostHidMouse);
    }
}
