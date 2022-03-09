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
#include "clock_config.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#include "app.h"
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#include "usb_phy.h"
#endif /* USB_HOST_CONFIG_EHCI */

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI))
#error Please enable USB_HOST_CONFIG_KHCI or USB_HOST_CONFIG_EHCI in file usb_host_config.
#endif

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "device_mouse.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
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
/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif
volatile uint32_t g_idPinStatus       = 0;
volatile uint32_t g_idPinStatusChange = 0;
volatile uint32_t g_deviceMode        = 0;
volatile USBHS_Type *ehciRegisterBase;
/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief board  pin and gpio init
 */
void USB_HostClockInit(void)
{
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif

#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0Clock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciPhyInit(kUSB_ControllerEhci0, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
}

#if ((defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
     (defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)))
void USBHS_IRQHandler(void)
{
    USB_Comom_IRQHandler();
}
#endif
#if ((defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)) || \
     (defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)))
void USB0_IRQHandler(void)
{
    USB_Comom_IRQHandler();
}
#endif

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[0];
#endif /* USB_HOST_CONFIG_EHCI */
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    uint8_t usbHOSTKhciIrq[] = USB_IRQS;
    irqNumber                = usbHOSTKhciIrq[0];
#endif /* USB_HOST_CONFIG_KHCI */

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
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[0];
#endif /* USB_HOST_CONFIG_EHCI */
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    uint8_t usbHOSTKhciIrq[] = USB_IRQS;
    irqNumber                = usbHOSTKhciIrq[0];
#endif /* USB_HOST_CONFIG_KHCI */

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
#if defined(USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI > 0U)
    USB_HostEhciTaskFunction(param);
#endif
#if defined(USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI > 0U)
    USB_HostKhciTaskFunction(param);
#endif
}

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
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0Clock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciPhyInit(kUSB_ControllerEhci0, BOARD_XTAL0_CLK_HZ, &phyConfig);
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    SystemCoreClockUpdate();
    CLOCK_EnableUsbfs0Clock(kCLOCK_UsbSrcPll0, CLOCK_GetFreq(kCLOCK_PllFllSelClk));
/*
 * If the SOC has USB KHCI dedicated RAM, the RAM memory needs to be clear after
 * the KHCI clock is enabled. When the demo uses USB EHCI IP, the USB KHCI dedicated
 * RAM can not be used and the memory can't be accessed.
 */
#if (defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U))
#if (defined(FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS) && (FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS > 0U))
    for (int i = 0; i < FSL_FEATURE_USB_KHCI_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif /* FSL_FEATURE_USB_KHCI_USB_RAM_BASE_ADDRESS */
#endif /* FSL_FEATURE_USB_KHCI_USB_RAM */
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[0];
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USB_IRQS;
    irqNumber                  = usbDeviceKhciIrq[0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_DeviceIsrDisable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
    uint8_t usbDeviceEhciIrq[] = USBHS_IRQS;
    irqNumber                  = usbDeviceEhciIrq[0];
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
    uint8_t usbDeviceKhciIrq[] = USB_IRQS;
    irqNumber                  = usbDeviceKhciIrq[0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    DisableIRQ((IRQn_Type)irqNumber);
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
 * @brief get  USB id pin status
 */
uint8_t USB_GetIdPinStatus(void)
{
    return ((ehciRegisterBase->OTGSC & USBHS_OTGSC_ID_MASK) >> USBHS_OTGSC_ID_SHIFT);
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
 * @brief host mouse freertos task function.
 *
 * @param param   the host mouse instance pointer.
 */
void Host_AppTask(void *param)
{
    while (1)
    {
        if (g_deviceMode == 0)
        {
            Host_AppTaskFunction();
        }
    }
}

/*!
 * @brief device mouse freertos task function.
 *
 * @param param   the host mouse instance pointer.
 */
void Device_AppTask(void *param)
{
    while (1)
    {
        if (g_deviceMode == 1)
        {
            Device_AppTaskFunction();
        }
    }
}

/*!
 * @brief pin detect  task function.
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
            vTaskDelay(100);
            Host_AppDeinit();
            g_deviceMode = 1;
            BOARD_UsbVbusOn(0);
            Device_AppInit();
        }
        g_idPinStatusChange = 0;
    }
}

void Pin_DetectTask(void *param)
{
    while (1)
    {
        Pin_DetectTaskFunction();
        vTaskDelay(100);
    }
}
/*!
 * @brief app initialization.
 */
/*!
 * @brief app initialization.
 */
void APP_init(void)
{
    uint32_t usbhsBaseAddrs[] = USBHS_BASE_ADDRS;

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

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    port_pin_config_t portConfig = {0};

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Enable USB ID PIN */
    portConfig.pullSelect          = kPORT_PullUp;
    portConfig.openDrainEnable     = 1;
    portConfig.mux                 = kPORT_MuxAlt7;
    portConfig.driveStrength       = kPORT_LowDriveStrength;
    portConfig.slewRate            = kPORT_SlowSlewRate;
    portConfig.passiveFilterEnable = 0;
#if defined(FSL_FEATURE_PORT_HAS_PIN_CONTROL_LOCK) && FSL_FEATURE_PORT_HAS_PIN_CONTROL_LOCK
    portConfig.lockRegister = 0;
#endif
    PORT_SetPinConfig(PORTE, 10UL, &portConfig);

    APP_init();
    Pin_DetectTaskFunction();

    if (xTaskCreate(Pin_DetectTask, "pin detect task", 2000L / sizeof(portSTACK_TYPE), NULL, 5, NULL) != pdPASS)
    {
        usb_echo("create pin detect task error\r\n");
    }
    if (xTaskCreate(Device_AppTask, "usb device task", 2000L / sizeof(portSTACK_TYPE), NULL, 4, NULL) != pdPASS)
    {
        usb_echo("create usb device task error\r\n");
    }
    if (xTaskCreate(Host_AppTask, "usb host task", 2000L / sizeof(portSTACK_TYPE), NULL, 4, NULL) != pdPASS)
    {
        usb_echo("create usb host task error\r\n");
    }
    vTaskStartScheduler();

    while (1)
    {
        ;
    }
}
