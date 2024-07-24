/*
 * Copyright (c) 2015 - 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_device_config.h"
#include "usb_host.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_descriptor.h"
#include "fsl_device_registers.h"
#include "usb_host_hid.h"
#include "host_keyboard.h"
#include "device_mouse.h"
#include "pin_mux.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if (USB_HOST_CONFIG_KHCI) || (USB_DEVICE_CONFIG_EHCI)
#if ((!defined USB_DEVICE_CONFIG_EHCI) || (!USB_DEVICE_CONFIG_EHCI))
#error Please enable USB_DEVICE_CONFIG_EHCI in file usb_device_config.
#endif
#endif

#if (USB_HOST_CONFIG_OHCI) || (USB_HOST_CONFIG_IP3516HS) || (USB_DEVICE_CONFIG_LPCIP3511FS) || \
    (USB_DEVICE_CONFIG_LPCIP3511HS)
#if ((USB_HOST_CONFIG_OHCI) && (USB_DEVICE_CONFIG_LPCIP3511FS)) || \
    ((USB_HOST_CONFIG_IP3516HS) && (USB_DEVICE_CONFIG_LPCIP3511HS))
#error "Cannot enable HS/FS's host and device at the same time"
#endif
#endif

#include "fsl_power.h"
#include "fsl_mrt.h"
#include "usb.h"
#include "usb_device_hid.h"
#include "usb_device_ch9.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

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
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);

/*!
 * @brief application initialization.
 */
static void USB_HostApplicationInit(void);

static void USB_DeviceApplicationInit(void);

void BOARD_InitHardware(void);

void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

void USB_HostClockInit(void);
void USB_HostIsrEnable(void);
void USB_HostTaskFn(void *param);

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint32_t hwTick;
uint32_t timerInterval;
uint32_t isConnectedToFsHost = 0U;
uint32_t isConnectedToHsHost = 0U;

extern usb_host_keyboard_instance_t g_HostHidKeyboard;
extern usb_device_class_config_list_struct_t g_UsbDeviceHidConfigList;
usb_hid_mouse_struct_t g_UsbDeviceHidMouse;
usb_host_handle g_HostHandle;
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_MouseBuffer[USB_HID_MOUSE_REPORT_LENGTH];

/*******************************************************************************
 * Code
 ******************************************************************************/
void USB_TimerInit(uint8_t instance, uint32_t interval)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    IRQn_Type instanceIrq[]  = MRT_IRQS;
    /* Structure of initialize MRT */
    mrt_config_t mrtConfig;
    /* mrtConfig.enableMultiTask = false; */
    MRT_GetDefaultConfig(&mrtConfig);
    /* Init mrt module */
    MRT_Init(instanceList[instance], &mrtConfig);
    /* Setup Channel 0 to be repeated */
    MRT_SetupChannelMode(instanceList[instance], kMRT_Channel_0, kMRT_RepeatMode);
    /* Enable timer interrupts for channel 0 */
    MRT_EnableInterrupts(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptEnable);
    timerInterval = interval;
    /* Enable at the NVIC */
    EnableIRQ(instanceIrq[instance]);
}
void USB_TimerInt(uint8_t instance, uint8_t enable)
{
    MRT_Type *instanceList[] = MRT_BASE_PTRS;
    uint32_t mrt_clock;
    mrt_clock = CLOCK_GetFreq(kCLOCK_BusClk);
    if (enable)
    {
        /* Start channel 0 */
        MRT_StartTimer(instanceList[instance], kMRT_Channel_0, USEC_TO_COUNT(timerInterval, mrt_clock));
    }
    else
    {
        /* Stop channel 0 */
        MRT_StopTimer(instanceList[instance], kMRT_Channel_0);
        /* Clear interrupt flag.*/
        MRT_ClearStatusFlags(instanceList[instance], kMRT_Channel_0, kMRT_TimerInterruptFlag);
    }
}
void MRT0_IRQHandler(void)
{
    /* Clear interrupt flag.*/
    MRT_ClearStatusFlags(MRT0, kMRT_Channel_0, kMRT_TimerInterruptFlag);
    if (hwTick)
    {
        hwTick--;
        if (!hwTick)
        {
            USB_TimerInt(0, 0);
        }
    }
    else
    {
        USB_TimerInt(0, 0);
    }
}
void USB_DeviceDisconnected(void)
{
    isConnectedToFsHost = 0U;
}
/*
 * This is a work-around to fix the HS device Chirping issue.
 * The device (IP3511HS controller) will not work sometimes when the cable
 * is attached at the first time after a Power-on Reset.
 */
void USB_DeviceHsPhyChirpIssueWorkaround(void)
{
    uint32_t startFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
    uint32_t currentFrame;
    uint32_t isConnectedToFsHostFlag = 0U;
    if ((!isConnectedToHsHost) && (!isConnectedToFsHost))
    {
        if (((USBHSD->DEVCMDSTAT & USBHSD_DEVCMDSTAT_Speed_MASK) >> USBHSD_DEVCMDSTAT_Speed_SHIFT) == 0x01U)
        {
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK))) |
                                 USBHSD_DEVCMDSTAT_PHY_TEST_MODE(0x05U);
            hwTick = 100;
            USB_TimerInt(0, 1);
            usb_echo("The USB device PHY chirp work-around is working\r\n");
            while (hwTick)
            {
            }
            currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
            if (currentFrame != startFrame)
            {
                isConnectedToHsHost = 1U;
            }
            else
            {
                hwTick = 1;
                USB_TimerInt(0, 1);
                while (hwTick)
                {
                }
                currentFrame = USBHSD->INFO & USBHSD_INFO_FRAME_NR_MASK;
                if (currentFrame != startFrame)
                {
                    isConnectedToHsHost = 1U;
                }
                else
                {
                    isConnectedToFsHostFlag = 1U;
                }
            }
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_PHY_TEST_MODE_MASK)));
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U | USBHSD_DEVCMDSTAT_DCON_MASK)));
            hwTick             = 510;
            USB_TimerInt(0, 1);
            while (hwTick)
            {
            }
            USBHSD->DEVCMDSTAT = (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USB_DEVCMDSTAT_DCON_C_MASK;
            USBHSD->DEVCMDSTAT =
                (USBHSD->DEVCMDSTAT & (~(0x0F000000U))) | USBHSD_DEVCMDSTAT_DCON_MASK | USB_DEVCMDSTAT_DRES_C_MASK;
            if (isConnectedToFsHostFlag)
            {
                isConnectedToFsHost = 1U;
            }
        }
    }
}

#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceHidMouse.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_UsbDeviceHidMouse.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFreq(kCLOCK_FroHf));

#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USB_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }

#endif
#endif

#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0DeviceClock(kCLOCK_UsbSrcUsbPll, 0U);
#if defined(FSL_FEATURE_USBHSD_USB_RAM) && (FSL_FEATURE_USBHSD_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USBHSD_USB_RAM; i++)
    {
        ((uint8_t *)FSL_FEATURE_USBHSD_USB_RAM_BASE_ADDRESS)[i] = 0x00U;
    }
#endif
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[DEVICE_CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[DEVICE_CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
#endif
    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_DEVICE_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    USB_DeviceLpcIp3511TaskFunction(deviceHandle);
#endif
}
#endif

#if (defined(USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
void USB0_IRQHandler(void)
{
    USB_HostOhciIsrFunction(g_HostHandle);
}
#endif /* USB_HOST_CONFIG_OHCI */
#if (defined(USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
void USB1_IRQHandler(void)
{
    USB_HostIp3516HsIsrFunction(g_HostHandle);
}
#endif /* USB_HOST_CONFIG_IP3516HS */

void USB_HostClockInit(void)
{
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
    CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbSrcUsbPll, 48000000U);

#endif /* USB_HOST_CONFIG_OHCI */

#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    CLOCK_EnableUsbhs0HostClock(kCLOCK_UsbSrcUsbPll, 48000000U);

#endif /* USB_HOST_CONFIG_IP3511HS */
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
    IRQn_Type usbHsIrqs[] = {(IRQn_Type)USB0_IRQn};
    irqNumber             = usbHsIrqs[HOST_CONTROLLER_ID - kUSB_ControllerOhci0];
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    IRQn_Type usbHsIrqs[] = {(IRQn_Type)USB1_IRQn};
    irqNumber             = usbHsIrqs[HOST_CONTROLLER_ID - kUSB_ControllerIp3516Hs0];
#endif /* USB_HOST_CONFIG_IP3511HS */

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
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
    USB_HostOhciTaskFunction(param);
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    USB_HostIp3516HsTaskFunction(param);
#endif /* USB_HOST_CONFIG_IP3516HS */
}

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
    return USB_HostHidKeyboardEvent(deviceHandle, configurationHandle, eventCode);
}

static void USB_HostApplicationInit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostClockInit();

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    status = USB_HostInit(HOST_CONTROLLER_ID, &g_HostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host init error\r\n");
        return;
    }
    USB_HostIsrEnable();

    usb_echo("w: move up\r\n");
    usb_echo("s: move down\r\n");
    usb_echo("a: move left\r\n");
    usb_echo("d: move right\r\n");
}

static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Set HID mouse to default state */
    g_UsbDeviceHidMouse.speed        = USB_SPEED_FULL;
    g_UsbDeviceHidMouse.attach       = 0U;
    g_UsbDeviceHidMouse.hidHandle    = (class_handle_t)NULL;
    g_UsbDeviceHidMouse.deviceHandle = NULL;
    g_UsbDeviceHidMouse.buffer       = s_MouseBuffer;

    /* Initialize the usb stack and class drivers */
    if (kStatus_USB_Success !=
        USB_DeviceClassInit(DEVICE_CONTROLLER_ID, &g_UsbDeviceHidConfigList, &g_UsbDeviceHidMouse.deviceHandle))
    {
        usb_echo("USB device mouse failed\r\n");
        return;
    }
    else
    {
        /* Get the HID mouse class handle */
        g_UsbDeviceHidMouse.hidHandle = g_UsbDeviceHidConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /* Start USB device HID mouse */
    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_UsbDeviceHidMouse.deviceHandle);
}

int main(void)
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB Phy */

/* initialize device */
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
    USB_TimerInit(0, 1000U);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif

#if ((defined FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
    USB_DeviceApplicationInit();
    USB_HostApplicationInit();

    while (1)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_UsbDeviceHidMouse.deviceHandle);
#endif

        USB_HostTaskFn(g_HostHandle);

        USB_HostHidKeyboardTask(&g_HostHidKeyboard);
    }
}
