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
#include "host_mouse.h"
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#include "app.h"
#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

#include "usb_phy.h"
#include "fsl_power.h"
#include "fsl_adapter_gpio.h"
#include "fsl_adapter_timer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if ((defined(USB_HOST_CONFIG_OHCI)) && (USB_HOST_CONFIG_OHCI > 0U))
#define APP_EXCLUDE_FROM_DEEPSLEEP  (kPDRUNCFG_PD_LDOFLASHNV | kPDRUNCFG_PD_USB0_PHY | kPDRUNCFG_PD_LDOMEM)
#define APP_DEEPSLEEP_WAKEUP_SOURCE (WAKEUP_GPIO_GLOBALINT0 | WAKEUP_USB0_NEEDCLK | WAKEUP_USB0)
#endif
#if ((defined(USB_HOST_CONFIG_IP3516HS)) && (USB_HOST_CONFIG_IP3516HS > 0U))
#define APP_EXCLUDE_FROM_DEEPSLEEP \
    (kPDRUNCFG_PD_LDOFLASHNV | kPDRUNCFG_PD_USB1_PHY | kPDRUNCFG_PD_LDOMEM | kPDRUNCFG_PD_LDOUSBHS)
#define APP_DEEPSLEEP_WAKEUP_SOURCE       (WAKEUP_GPIO_GLOBALINT0 | WAKEUP_USB1_NEEDCLK | WAKEUP_USB1)
#define USB_HOST_HS_NOT_SUPPORT_DEVICE_LS (1U)
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
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
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

status_t DbgConsole_Deinit(void);

void BOARD_InitPins(void);
void BOARD_DeinitPins(void);
void SW_IntControl(uint8_t enable);
char *SW_GetName(void);
void HW_TimerControl(uint8_t enable);
void USB_LowpowerModeInit(void);
void USB_PreLowpowerMode(void);
uint8_t USB_EnterLowpowerMode(void);
void USB_PostLowpowerMode(void);
void USB_ControllerSuspended(void);
void USB_WaitClockLocked(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#define TIMER_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
extern usb_host_mouse_instance_t g_HostHidMouse;
extern usb_host_handle g_HostHandle;
uint32_t g_halTimerHandle[(HAL_TIMER_HANDLE_SIZE + 3) / 4];
uint32_t g_gpioHandle[(HAL_GPIO_HANDLE_SIZE + 3) / 4];
/*! @brief USB host mouse instance global variable */
extern usb_host_mouse_instance_t g_HostHidMouse;
usb_host_handle g_HostHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_DeinitPins(void)
{
}

void SW_IntControl(uint8_t enable)
{
    if (enable)
    {
        g_HostHidMouse.selfWakeup = 0U;
    }
    HAL_GpioWakeUpSetting(g_gpioHandle, enable);
}

void SW_Callback(void *param)
{
    g_HostHidMouse.selfWakeup = 1U;
    SW_IntControl(0);
}

void SW_Init(void)
{
    hal_gpio_pin_config_t s_GpioInputPin;
    s_GpioInputPin.direction = kHAL_GpioDirectionIn;
    s_GpioInputPin.port      = BOARD_SW1_GPIO_PORT;
    s_GpioInputPin.pin       = BOARD_SW1_GPIO_PIN;

    HAL_GpioInit(g_gpioHandle, &s_GpioInputPin);
    HAL_GpioInstallCallback(g_gpioHandle, SW_Callback, NULL);
    HAL_GpioSetTriggerMode(g_gpioHandle, kHAL_GpioInterruptFallingEdge);
}

char *SW_GetName(void)
{
    return BOARD_SW1_NAME;
}

void HW_TimerCallback(void *param)
{
    g_HostHidMouse.hwTick++;
    USB_HostUpdateHwTick(g_HostHandle, g_HostHidMouse.hwTick);
}

void HW_TimerInit(void)
{
    hal_timer_config_t halTimerConfig;
    halTimerConfig.timeout            = 1000;
    halTimerConfig.srcClock_Hz        = TIMER_SOURCE_CLOCK;
    halTimerConfig.instance           = 0U;
    hal_timer_handle_t halTimerHandle = &g_halTimerHandle[0];
    HAL_TimerInit(halTimerHandle, &halTimerConfig);
    HAL_TimerInstallCallback(halTimerHandle, HW_TimerCallback, NULL);
}

void HW_TimerControl(uint8_t enable)
{
    if (enable)
    {
        HAL_TimerEnable(g_halTimerHandle);
    }
    else
    {
        HAL_TimerDisable(g_halTimerHandle);
    }
}

void USB_LowpowerModeInit(void)
{
    SW_Init();
    HW_TimerInit();
}

void USB_PreLowpowerMode(void)
{
    __disable_irq();
#if ((defined(USB_HOST_CONFIG_OHCI)) && (USB_HOST_CONFIG_OHCI > 0U))
    if (SYSCON->USB0NEEDCLKSTAT & (SYSCON_USB0NEEDCLKSTAT_DEV_NEEDCLK_MASK))
    {
        /* enable usb0 device clock */
        CLOCK_EnableClock(kCLOCK_Usbd0);
        while (SYSCON->USB0NEEDCLKSTAT & (SYSCON_USB0NEEDCLKSTAT_DEV_NEEDCLK_MASK))
        {
            __ASM("nop");
        }
        /* disable usb0 device clock */
        CLOCK_DisableClock(kCLOCK_Usbd0);
    }
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
#endif
#if ((defined(USB_HOST_CONFIG_IP3516HS)) && (USB_HOST_CONFIG_IP3516HS > 0U))
    if (SYSCON->USB1NEEDCLKSTAT & (SYSCON_USB1NEEDCLKSTAT_DEV_NEEDCLK_MASK))
    {
        /* enable usb1 device clock */
        CLOCK_EnableClock(kCLOCK_Usbd1);
        while (SYSCON->USB1NEEDCLKSTAT & (SYSCON_USB1NEEDCLKSTAT_DEV_NEEDCLK_MASK))
        {
            __ASM("nop");
        }
        /* disable usb1 device clock */
        CLOCK_DisableClock(kCLOCK_Usbd1);
    }
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_SW_CTRL_PDCOM_MASK;
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_SW_PDCOM_MASK;

    /* clear this field to decrease power consumption */
    USBPHY->USB1_VBUS_DETECT_CLR = USBPHY_USB1_VBUS_DETECT_PWRUP_CMPS_MASK;
#endif

    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK); /*!< Switch to 12MHz first to ensure we can change voltage without
                                          accidentally being below the voltage for current speed */
    ANACTRL->FRO192M_CTRL &= ~(ANACTRL_FRO192M_CTRL_USBCLKADJ_MASK);
    POWER_SetVoltageForFreq(12000000U); /*!< Set voltage for core */
}

uint8_t USB_EnterLowpowerMode(void)
{
    /* Enter Deep Sleep mode */
    POWER_EnterDeepSleep(APP_EXCLUDE_FROM_DEEPSLEEP, 0x0, APP_DEEPSLEEP_WAKEUP_SOURCE, 0x0);
    return kStatus_Success;
}

void USB_PostLowpowerMode(void)
{
    __enable_irq();
#if ((defined(USB_HOST_CONFIG_IP3516HS)) && (USB_HOST_CONFIG_IP3516HS > 0U))
    *((uint32_t *)(USBHSH_BASE + 0x50)) &= ~USBHSH_PORTMODE_SW_PDCOM_MASK;
    *((uint32_t *)(USBHSH_BASE + 0x50)) &= ~USBHSH_PORTMODE_SW_CTRL_PDCOM_MASK;
#endif
}

void USB_ControllerSuspended(void)
{
#if ((defined(USB_HOST_CONFIG_OHCI)) && (USB_HOST_CONFIG_OHCI > 0U))
    while (SYSCON->USB0NEEDCLKSTAT & (SYSCON_USB0NEEDCLKSTAT_HOST_NEEDCLK_MASK))
    {
        __ASM("nop");
    }
    SYSCON->USB0NEEDCLKCTRL |= SYSCON_USB0NEEDCLKCTRL_POL_FS_HOST_NEEDCLK_MASK;
#endif
#if ((defined(USB_HOST_CONFIG_IP3516HS)) && (USB_HOST_CONFIG_IP3516HS > 0U))
    while (SYSCON->USB1NEEDCLKSTAT & (SYSCON_USB1NEEDCLKSTAT_HOST_NEEDCLK_MASK))
    {
        __ASM("nop");
    }
    SYSCON->USB1NEEDCLKCTRL |= SYSCON_USB1NEEDCLKCTRL_POL_HS_HOST_NEEDCLK_MASK;
#endif
}

void USB0_NEEDCLK_IRQHandler(void)
{
}

void USB1_NEEDCLK_IRQHandler(void)
{
}

void USB_WaitClockLocked(void)
{
    BOARD_InitBootClocks();
}

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
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    usb_phy_config_struct_t phyConfig = {
        BOARD_USB_PHY_D_CAL,
        BOARD_USB_PHY_TXCAL45DP,
        BOARD_USB_PHY_TXCAL45DM,
    };
#endif

#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
    CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbfsSrcPll1, 48000000U);
#if ((defined FSL_FEATURE_USBFSH_USB_RAM) && (FSL_FEATURE_USBFSH_USB_RAM > 0U))
    for (int i = 0; i < (FSL_FEATURE_USBFSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBFSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
#endif
#endif /* USB_HOST_CONFIG_OHCI */

#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    CLOCK_EnableUsbhs0PhyPllClock(kCLOCK_UsbPhySrcExt, BOARD_XTAL0_CLK_HZ);
    CLOCK_EnableUsbhs0HostClock(kCLOCK_UsbSrcUnused, 0U);
    USB_EhciLowPowerPhyInit(CONTROLLER_ID, BOARD_XTAL0_CLK_HZ, &phyConfig);
#if ((defined FSL_FEATURE_USBHSH_USB_RAM) && (FSL_FEATURE_USBHSH_USB_RAM > 0U))
    for (int i = 0; i < (FSL_FEATURE_USBHSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBHSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
#endif
#endif /* USB_HOST_CONFIG_IP3511HS */
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI > 0U))
    IRQn_Type usbHsIrqs[] = {(IRQn_Type)USB0_IRQn};
    irqNumber             = usbHsIrqs[CONTROLLER_ID - kUSB_ControllerOhci0];
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS > 0U))
    IRQn_Type usbHsIrqs[] = {(IRQn_Type)USB1_IRQn};
    irqNumber             = usbHsIrqs[CONTROLLER_ID - kUSB_ControllerIp3516Hs0];
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

/*!
 * @brief USB isr function.
 */

static void USB_HostRemoteWarkupCallback(void *param, usb_host_transfer_t *transfer, usb_status_t status)
{
    if (NULL == param)
    {
        return;
    }
    USB_HostFreeTransfer(param, transfer);

    if (kStatus_USB_Success == status)
    {
        if (kStatus_SuspendWaitClearRemoteWakeup == g_HostHidMouse.suspendResumeState)
        {
            usb_echo("Remote wakeup feature cleared.\r\n");
            g_HostHidMouse.isSetRemoteWakeup  = 0U;
            g_HostHidMouse.suspendResumeState = kStatus_Suspending;
        }
        else if (kStatus_SuspendWaitSetRemoteWakeup == g_HostHidMouse.suspendResumeState)
        {
            usb_echo("Remote wakeup feature set.\r\n");
            g_HostHidMouse.isSetRemoteWakeup  = 1U;
            g_HostHidMouse.suspendResumeState = kStatus_Suspending;
        }
        else
        {
        }
    }
    else
    {
        g_HostHidMouse.suspendResumeState = kStatus_SuspendFailRemoteWakeup;
        usb_echo(
            "\tSend clear remote wakeup feature request failed.\r\nWhether need to continue? "
            "Please ENTER y(es) or n(o): ");
    }
    DbgConsole_Flush();
}

usb_status_t USB_HostControlRemoteWakeup(usb_host_handle hostHandle,
                                         usb_device_handle deviceHandle,
                                         host_inner_transfer_callback_t callbackFn,
                                         void *callbackParam,
                                         uint8_t enable)
{
    usb_host_transfer_t *transfer;
    uint32_t infoValue = 0U;

    if (hostHandle == NULL)
    {
        return kStatus_USB_InvalidHandle;
    }

    /* malloc one transfer */
    if (USB_HostMallocTransfer(hostHandle, &transfer) != kStatus_USB_Success)
    {
#ifdef HOST_ECHO
        usb_echo("error to get transfer\r\n");
#endif
        return kStatus_USB_Busy;
    }
    /* initialize transfer */
    transfer->transferBuffer = NULL;
    transfer->transferLength = 0;
    transfer->callbackFn     = callbackFn;
    transfer->callbackParam  = callbackParam;
    transfer->setupPacket->bmRequestType =
        USB_REQUEST_TYPE_RECIPIENT_DEVICE | USB_REQUEST_TYPE_DIR_OUT | USB_REQUEST_TYPE_TYPE_STANDARD;
    transfer->setupPacket->bRequest = (enable ? USB_REQUEST_STANDARD_SET_FEATURE : USB_REQUEST_STANDARD_CLEAR_FEATURE);
    transfer->setupPacket->wValue =
        USB_SHORT_TO_LITTLE_ENDIAN(USB_REQUEST_STANDARD_FEATURE_SELECTOR_DEVICE_REMOTE_WAKEUP);
    transfer->setupPacket->wIndex  = USB_SHORT_TO_LITTLE_ENDIAN(0x00U);
    transfer->setupPacket->wLength = USB_SHORT_TO_LITTLE_ENDIAN(0x00U);

    USB_HostHelperGetPeripheralInformation(deviceHandle, kUSB_HostGetDeviceControlPipe, &infoValue);

    if (USB_HostSendSetup(hostHandle, (usb_host_pipe_handle)infoValue, transfer) !=
        kStatus_USB_Success) /* call host driver api */
    {
#ifdef HOST_ECHO
        usb_echo("failed for USB_HostControlRemoteWakeup\r\n");
#endif
        USB_HostFreeTransfer(hostHandle, transfer);
        return kStatus_USB_Error;
    }
    return kStatus_USB_Success;
}

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
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
        case kUSB_HostEventNotSuspended:
            if (kStatus_Idle != g_HostHidMouse.suspendResumeState)
            {
                if (g_HostHidMouse.suspendBus)
                {
                    usb_echo("Suspend BUS failed.\r\n");
                }
                else
                {
                    usb_echo("Suspend device failed.\r\n");
                }
            }
            g_HostHidMouse.suspendResumeState = kStatus_Idle;
            break;
        case kUSB_HostEventSuspended:
            if (kStatus_Idle != g_HostHidMouse.suspendResumeState)
            {
                USB_ControllerSuspended();
                g_HostHidMouse.suspendResumeState = kStatus_Suspended;
            }
            else
            {
                g_HostHidMouse.suspendResumeState = kStatus_Idle;
            }
            break;
        case kUSB_HostEventDetectResume:
            if (kStatus_Idle != g_HostHidMouse.suspendResumeState)
            {
                USB_WaitClockLocked();
            }
            break;
        case kUSB_HostEventResumed:
            if (kStatus_Idle != g_HostHidMouse.suspendResumeState)
            {
                if (g_HostHidMouse.suspendBus)
                {
                    usb_echo("BUS has been resumed.\r\n");
                }
                else
                {
                    usb_echo("Device has been resumed.\r\n");
                }
                DbgConsole_Flush();
            }
            g_HostHidMouse.suspendResumeState = kStatus_Idle;
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

void USB_PowerPreSwitchHook(void)
{
    HW_TimerControl(0U);

    DbgConsole_Deinit();

    BOARD_DeinitPins();

    USB_PreLowpowerMode();
}

void USB_PowerPostSwitchHook(void)
{
    USB_WaitClockLocked();
    USB_PostLowpowerMode();
    BOARD_InitPins();
    BOARD_InitDebugConsole();
    HW_TimerControl(1U);
}

void USB_HostSuspendResumeTask(void)
{
    usb_status_t usb_error;
    uint8_t command;

    if (kStatus_USB_Success != DbgConsole_TryGetchar((char *)&command))
    {
        command = 0;
    }
    switch (g_HostHidMouse.suspendResumeState)
    {
        case kStatus_Idle:
            if ('s' == command)
            {
                g_HostHidMouse.suspendResumeState = kStatus_SartSuspend;
                usb_echo("Start suspend USB BUS...\r\n");
            }
            else
            {
                if (command)
                {
                    usb_echo("Please Enter 's' to start suspend test\r\n");
                }
            }
            break;
        case kStatus_SartSuspend:
            g_HostHidMouse.suspendBus = 1;
            if (g_HostHidMouse.supportRemoteWakeup)
            {
                usb_echo(
                    "\r\nPlease Enter: \r\n\t1. Enable remote wakeup feature.\r\n\t2. Disable remote wakeup "
                    "feature.\r\n");
                g_HostHidMouse.suspendResumeState = kStatus_SuspendSetRemoteWakeup;
            }
            else
            {
                g_HostHidMouse.suspendResumeState = kStatus_Suspending;
            }
            break;
        case kStatus_SuspendSetRemoteWakeup:
            if ('1' == command)
            {
                usb_echo("1");
                usb_error = USB_HostControlRemoteWakeup(g_HostHandle, g_HostHidMouse.deviceHandle,
                                                        USB_HostRemoteWarkupCallback, g_HostHandle, 1);
                if (kStatus_USB_Success == usb_error)
                {
                    g_HostHidMouse.suspendResumeState = kStatus_SuspendWaitSetRemoteWakeup;
                }
                else
                {
                    g_HostHidMouse.suspendResumeState = kStatus_SuspendFailRemoteWakeup;
                    usb_echo(
                        "\tSend set remote wakeup feature request failed.\r\nWhether need to continue? "
                        "Please ENTER y(es) or n(o): ");
                }
            }
            else if ('2' == command)
            {
                usb_echo("2");
                usb_error = USB_HostControlRemoteWakeup(g_HostHandle, g_HostHidMouse.deviceHandle,
                                                        USB_HostRemoteWarkupCallback, g_HostHandle, 0);
                if (kStatus_USB_Success == usb_error)
                {
                    g_HostHidMouse.suspendResumeState = kStatus_SuspendWaitClearRemoteWakeup;
                }
                else
                {
                    g_HostHidMouse.suspendResumeState = kStatus_SuspendFailRemoteWakeup;
                    usb_echo(
                        "\tSend clear remote wakeup feature request failed.\r\nWhether need to continue? "
                        "Please ENTER y(es) or n(o): ");
                }
            }
            else
            {
            }
            DbgConsole_Flush();
            break;
        case kStatus_SuspendWaitSetRemoteWakeup:
        case kStatus_SuspendWaitClearRemoteWakeup:
            break;
        case kStatus_SuspendFailRemoteWakeup:
            if ('y' == command)
            {
                usb_echo("y");
                g_HostHidMouse.suspendResumeState = kStatus_Suspending;
            }
            else if ('n' == command)
            {
                usb_echo("n");
                g_HostHidMouse.suspendResumeState = kStatus_Idle;
            }
            else
            {
            }
            break;
        case kStatus_Suspending:
            g_HostHidMouse.suspendResumeState = kStatus_SuspendRequest;
            if (kStatus_USB_Success ==
                USB_HostSuspendDeviceResquest(g_HostHandle,
                                              g_HostHidMouse.suspendBus ? NULL : g_HostHidMouse.deviceHandle))
            {
            }
            else
            {
                usb_echo("Send suspend request failed.\r\n");
                g_HostHidMouse.suspendResumeState = kStatus_Idle;
            }
            break;
        case kStatus_SuspendRequest:
            break;
        case kStatus_Suspended:
            DbgConsole_Flush();
            if (g_HostHidMouse.suspendBus)
            {
                usb_echo("BUS has been suspended.\r\n");
            }
            else
            {
                usb_echo("Device has been suspended.\r\n");
            }
            DbgConsole_Flush();
            usb_echo("Please Press wakeup switch(%s) to start resume test.\r\n", SW_GetName());
            if (g_HostHidMouse.isSetRemoteWakeup)
            {
                usb_echo("Or, wait for device sends resume signal.\r\n");
            }
            /*flush the output befor enter lowpower*/
            DbgConsole_Flush();
            USB_PowerPreSwitchHook();
            SW_IntControl(1);

            g_HostHidMouse.suspendResumeState = kStatus_WaitResume;
            if (kStatus_Success != USB_EnterLowpowerMode())
            {
                g_HostHidMouse.selfWakeup = 1U;
                USB_PowerPostSwitchHook();
                usb_echo("Enter VLPS mode failed!\r\n");
            }
            else
            {
                USB_PowerPostSwitchHook();
            }

            if (g_HostHidMouse.isSetRemoteWakeup)
            {
            }
            break;
        case kStatus_WaitResume:
            if (g_HostHidMouse.selfWakeup)
            {
                g_HostHidMouse.selfWakeup = 0U;
                usb_echo("Start resume the device.\r\n");
                g_HostHidMouse.suspendResumeState = kStatus_ResumeRequest;
                if (kStatus_USB_Success ==
                    USB_HostResumeDeviceResquest(g_HostHandle,
                                                 g_HostHidMouse.suspendBus ? NULL : g_HostHidMouse.deviceHandle))
                {
                }
                else
                {
                    g_HostHidMouse.suspendResumeState = kStatus_Idle;
                    usb_echo("Send resume signal failed.\r\n");
                }
            }
            break;
        case kStatus_ResumeRequest:
            break;
        default:
            break;
    }
    command = 0;
}

int main(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);
#if (defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS)
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB Phy */
    /* enable USB IP clock */
    CLOCK_EnableUsbhs0HostClock(kCLOCK_UsbSrcUnused, 0U);
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /* enable usb1 device clock */
    CLOCK_EnableClock(kCLOCK_Usbd1);
    *((uint32_t *)(USBHSH_BASE + 0x50)) &= ~USBHSH_PORTMODE_DEV_ENABLE_MASK;
    *((uint32_t *)(USBHSH_BASE + 0x50)) &= ~USBHSH_PORTMODE_SW_CTRL_PDCOM_MASK;
    /* disable usb1 device clock */
    CLOCK_DisableClock(kCLOCK_Usbd1);
    /* disable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
#if ((defined(USB_HOST_HS_NOT_SUPPORT_DEVICE_LS)) && (USB_HOST_HS_NOT_SUPPORT_DEVICE_LS > 0U))
    if (0x00u == Chip_GetVersion())
    {
        usb_echo("LPC55S6x A0 can't support low speed device directly\r\n");
    }
#endif
#endif
#if (defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhmr0);
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /* enable usb0 device clock */
    CLOCK_EnableClock(kCLOCK_Usbd0);
    *((uint32_t *)(USBFSH_BASE + 0x5C)) &= ~USBFSH_PORTMODE_DEV_ENABLE_MASK;
    while (SYSCON->USB0NEEDCLKSTAT & (SYSCON_USB0NEEDCLKSTAT_DEV_NEEDCLK_MASK))
    {
        __ASM("nop");
    }
    /* disable usb0 device clock */
    CLOCK_DisableClock(kCLOCK_Usbd0);
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
    CLOCK_DisableClock(kCLOCK_Usbhmr0);
#endif

#if ((defined(USB_HOST_CONFIG_LOW_POWER_MODE)) && (USB_HOST_CONFIG_LOW_POWER_MODE > 0U))
    USB_LowpowerModeInit();
#endif

    USB_HostApplicationInit();

#if ((defined(USB_HOST_CONFIG_LOW_POWER_MODE)) && (USB_HOST_CONFIG_LOW_POWER_MODE > 0U))
    HW_TimerControl(1);
    usb_echo("Please Enter 's' to start suspend test\r\n");
#endif

    while (1)
    {
        USB_HostTaskFn(g_HostHandle);
        USB_HostHidMouseTask(&g_HostHidMouse);

        USB_HostSuspendResumeTask();
    }
}
