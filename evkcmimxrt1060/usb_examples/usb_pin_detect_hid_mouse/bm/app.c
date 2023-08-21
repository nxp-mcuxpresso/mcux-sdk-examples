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
#include "usb_host_hid.h"
#include "fsl_debug_console.h"
#include "host_mouse.h"
#include "fsl_common.h"
#include "pin_mux.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#include "app.h"
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif
#endif /* USB_HOST_CONFIG_EHCI */

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI))
#error Please enable USB_HOST_CONFIG_KHCI or USB_HOST_CONFIG_EHCI in file usb_host_config.
#endif

#include "usb.h"
#include "usb_device_config.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "device_mouse.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitHardware(void);
extern void BOARD_UsbVbusOn(uint8_t on);
extern void Device_AppInit(void);
extern void Device_AppDeinit(void);
extern void Device_AppTaskFunction(void);
extern void Host_AppInit(void);
extern void Host_AppDeinit(void);
extern void Host_AppTaskFunction(void);
extern void USB_DeviceKhciIsr(void);
extern void USB_DeviceEhciIsr(void);
extern void USB_HostKhciIsr(void);
extern void USB_HostEhciIsr(void);
extern void USB_HostClockInit(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t g_idPinStatus       = 0;
volatile uint32_t g_idPinStatusChange = 0;
volatile uint32_t g_deviceMode        = 0;
volatile uint32_t g_timer             = 0x0;
volatile USBHS_Type *ehciRegisterBase;
/*******************************************************************************
 * Code
 ******************************************************************************/

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

void USB_DeviceIsrDisable(void)
{
    uint8_t irqNumber;

    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    DisableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
    USB_DeviceEhciTaskFunction(deviceHandle);
}
#endif
void USB_OTG1_IRQHandler(void)
{
    USB_Comom_IRQHandler();
}

void USB_OTG2_IRQHandler(void)
{
    USB_Comom_IRQHandler();
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

void USB_HostIsrDisable(void)
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
    DisableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}

/*!
 * @brief  board USB Vbus enable or not
 */
void BOARD_UsbVbusOn(uint8_t on)
{
    /* Some time delay waitfor power stable */
    for (int i = 0U; i < 1000000U; i++)
    {
        __ASM("nop");
    }
}

/*!
 * @brief get USB id pin status
 */
uint8_t USB_GetIdPinStatus(void)
{
    return ((ehciRegisterBase->OTGSC & USBHS_OTGSC_ID_MASK) >> USBHS_OTGSC_ID_SHIFT);
}

/*!
 * @brief task makes a concession
 */
uint8_t TaskConcess(uint8_t ms)
{
    g_timer++;
    if (g_timer == 1000 * ms)
    {
        g_timer = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*!
 * @brief ehci host isr
 */
void USB_Comom_IRQHandler(void)
{
    if ((ehciRegisterBase->OTGSC & USBHS_OTGSC_IDIS_MASK) && (ehciRegisterBase->OTGSC & USBHS_OTGSC_IDIE_MASK))
    {
        ehciRegisterBase->OTGSC |= USBHS_OTGSC_IDIS_MASK;
        if (USB_GetIdPinStatus())
        {
            g_idPinStatus = 1;
        }
        else
        {
            g_idPinStatus = 0;
        }
        g_idPinStatusChange = 1;
    }
    else
    {
        if ((g_deviceMode == 0))
        {
            USB_HostEhciIsr();
        }
        else if ((g_deviceMode == 1))
        {
            USB_DeviceEhciIsr();
        }
        else
        {
        }
    }
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

/*!
 * @brief app initialization.
 */
void APP_init(void)
{
#if defined(USBHS_STACK_BASE_ADDRS)
    uint32_t usbhsBaseAddrs[] = USBHS_STACK_BASE_ADDRS;
#else
    uint32_t usbhsBaseAddrs[] = USBHS_BASE_ADDRS;
#endif

    if (CONTROLLER_ID - kUSB_ControllerEhci0 >= (sizeof(usbhsBaseAddrs) / sizeof(usbhsBaseAddrs[0])))
    {
        usb_echo("Pin detect:controller is not found!\r\n");
        return;
    }
    ehciRegisterBase = (USBHS_Type *)usbhsBaseAddrs[CONTROLLER_ID - kUSB_ControllerEhci0];
    USB_HostClockInit();
#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
    /* Some time delay waitfor phy ID status stable */
    for (volatile int i = 0U; i < 1000000U; i++)
    {
        __NOP();
    }
    if (USB_GetIdPinStatus())
    {
        g_idPinStatus = 1;
        g_deviceMode  = 1;
        BOARD_UsbVbusOn(0);
        Device_AppInit();
    }
    else
    {
        g_idPinStatus = 0;
        g_deviceMode  = 0;
        BOARD_UsbVbusOn(1);
        Host_AppInit();
    }
    ehciRegisterBase->OTGSC |= USBHS_OTGSC_IDIE_MASK;
}

/*!
 * @brief pin detect task function.
 */
void Pin_DetectTaskFunction(void)
{
    if (g_idPinStatusChange == 1)
    {
        if (g_idPinStatus == 0)
        {
            Device_AppDeinit();
            g_deviceMode = 0;
            BOARD_UsbVbusOn(1);
            Host_AppInit();
        }
        else
        {
            if (TaskConcess(1))
            {
                Host_AppDeinit();
                g_deviceMode = 1;
                BOARD_UsbVbusOn(0);
                Device_AppInit();
            }
            else
                return;
        }
        g_idPinStatusChange = 0;
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
    BOARD_InitDebugConsole();

    APP_init();
    Pin_DetectTaskFunction();

    while (1)
    {
        Pin_DetectTaskFunction();
        if (g_deviceMode == 0)
        {
            Host_AppTaskFunction();
        }
        else if (g_deviceMode == 1)
        {
            Device_AppTaskFunction();
        }
        else
        {
        }
    }
}
