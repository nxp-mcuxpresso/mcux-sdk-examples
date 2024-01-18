/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
#include "usb_device_class.h"
#include "usb_device_printer.h"
#include "usb_device_ch9.h"
#include "usb_device_descriptor.h"
#include "device_printer.h"
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

#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* The printer class callback */
static usb_status_t USB_DevicePrinterAppCallback(class_handle_t classHandle, uint32_t event, void *param);
static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t s_PrinterBuffer[USB_PRINTER_BUFFER_SIZE];
/* printer class specifice transfer buffer */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_PrinterClassBuffer[64];
/* printer device id for interface zero */
const uint8_t g_PrinterID[] = "xxMFG:NXP;MDL:ksdk printer demo;CMD:POSTSCRIPT";

usb_device_printer_app_t g_DevicePrinterApp;

extern usb_device_class_struct_t g_UsbDevicePrinterClass;

/* set class configurations */
usb_device_class_config_struct_t g_UsbDevicePrinterClassConfig[1] = {{
    USB_DevicePrinterAppCallback, /* printer class callback pointer */
    (class_handle_t)NULL,         /* The printer class handle, This field is set by USB_DeviceClassInit */
    &g_UsbDevicePrinterClass,     /* The printer configuration, including class code, subclass code, and protocol, class
                           type,
                           transfer type, endpoint address, max packet size, etc.*/
}};

/* set class configuration list */
usb_device_class_config_list_struct_t g_UsbDevicePrinterConfigList = {
    g_UsbDevicePrinterClassConfig, /* Class configurations */
    USB_DeviceCallback,            /* Device callback pointer */
    1U,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
void USB0_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_DevicePrinterApp.deviceHandle);
}
#endif

void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbfsSrcFro, CLOCK_GetFroHfFreq());
#endif
}
void USB_DeviceIsrEnable(void)
{
    uint8_t irqNumber;
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USB_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
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
}
#endif

/* ksdk debug console must have been initialized. */
static void USB_PrinterPrintData(uint8_t *data, uint32_t length)
{
    while (length--)
    {
        PUTCHAR(*(data++));
    }
}

static usb_status_t USB_DevicePrinterAppCallback(class_handle_t classHandle, uint32_t event, void *param)
{
    usb_status_t status = kStatus_USB_InvalidRequest;
    usb_device_printer_class_request_t *classRequest;
    usb_device_endpoint_callback_message_struct_t *message;

    uint32_t len;

    switch (event)
    {
        case kUSB_DevicePrinterEventGetDeviceId:
            classRequest = (usb_device_printer_class_request_t *)param;
            if ((classRequest->configIndex == 0U) && (classRequest->interface == USB_PRINTER_INTERFACE_INDEX) &&
                (classRequest->alternateSetting == 0))
            {
                for (len = 0; len < sizeof(g_PrinterID); ++len)
                {
                    s_PrinterClassBuffer[len] = g_PrinterID[len];
                }
                len                     = sizeof(g_PrinterID) - 1;
                s_PrinterClassBuffer[0] = ((uint8_t)(len >> 8));
                s_PrinterClassBuffer[1] = (uint8_t)len;
                classRequest->buffer    = s_PrinterClassBuffer;
                classRequest->length    = len;
                status                  = kStatus_USB_Success;
            }
            break;

        case kUSB_DevicePrinterEventGetPortStatus:
            classRequest            = (usb_device_printer_class_request_t *)param;
            s_PrinterClassBuffer[0] = g_DevicePrinterApp.printerPortStatus;
            classRequest->buffer    = s_PrinterClassBuffer;
            classRequest->length    = 1U;
            status                  = kStatus_USB_Success;
            break;

        case kUSB_DevicePrinterEventSoftReset:
            status = kStatus_USB_Success;
            break;

        case kUSB_DevicePrinterEventRecvResponse:
            message = (usb_device_endpoint_callback_message_struct_t *)param;
            if ((g_DevicePrinterApp.attach) && (g_DevicePrinterApp.prnterTaskState == kPrinter_Receiving))
            {
                if ((message != NULL) && (message->length != USB_CANCELLED_TRANSFER_LENGTH))
                {
                    g_DevicePrinterApp.printerState      = kPrinter_Received;
                    g_DevicePrinterApp.dataReceiveLength = message->length;
                }
                else
                {
                    g_DevicePrinterApp.printerState = kPrinter_ReceiveNeedPrime;
                }
                g_DevicePrinterApp.stateChanged = 1;
                status = kStatus_USB_Success;
            }
            break;

        case kUSB_DevicePrinterEventSendResponse:
            status = USB_DevicePrinterSend(g_DevicePrinterApp.classHandle, USB_PRINTER_BULK_ENDPOINT_IN,
                                           g_DevicePrinterApp.sendBuffer, g_DevicePrinterApp.sendLength);
            break;

        default:
            /* no action, return kStatus_USB_InvalidRequest */
            break;
    }

    return status;
}

static usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t status = kStatus_USB_InvalidRequest;
    uint8_t *param8p    = (uint8_t *)param;
    uint16_t *param16p  = (uint16_t *)param;
    uint8_t interface;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
            g_DevicePrinterApp.attach               = 0U;
            g_DevicePrinterApp.printerState         = kPrinter_Idle;
            g_DevicePrinterApp.prnterTaskState      = kPrinter_Idle;
            g_DevicePrinterApp.stateChanged         = 1;
            g_DevicePrinterApp.sendBuffer           = NULL;
            g_DevicePrinterApp.sendLength           = 0;
            g_DevicePrinterApp.currentConfiguration = 0U;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &g_DevicePrinterApp.speed))
            {
                USB_DeviceSetSpeed(handle, g_DevicePrinterApp.speed);
            }
#endif
            status = kStatus_USB_Success;
            break;

#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))

        case kUSB_DeviceEventAttach:
            usb_echo("USB device attached.\r\n");
            /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
            SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            status = USB_DeviceRun(g_DevicePrinterApp.deviceHandle);
            break;

        case kUSB_DeviceEventDetach:
            usb_echo("USB device detached.\r\n");
            g_DevicePrinterApp.attach = 0;
            status = USB_DeviceStop(g_DevicePrinterApp.deviceHandle);
            break;
#endif

        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*param8p))
            {
                g_DevicePrinterApp.attach               = 0U;
                g_DevicePrinterApp.currentConfiguration = 0U;
                g_DevicePrinterApp.printerState         = kPrinter_Idle;
                g_DevicePrinterApp.stateChanged         = 1;
                g_DevicePrinterApp.sendBuffer           = NULL;
                g_DevicePrinterApp.sendLength           = 0;
                status                                  = kStatus_USB_Success;
            }
            else if (USB_PRINTER_CONFIGURE_INDEX == *param8p)
            {
                /* Set device configuration request */
                g_DevicePrinterApp.attach               = 1U;
                g_DevicePrinterApp.currentConfiguration = *param8p;

                /* demo run */
                g_DevicePrinterApp.printerState = kPrinter_ReceiveNeedPrime;
                g_DevicePrinterApp.stateChanged = 1;
                status = USB_DevicePrinterSend(g_DevicePrinterApp.classHandle, USB_PRINTER_BULK_ENDPOINT_IN,
                                               g_DevicePrinterApp.sendBuffer, g_DevicePrinterApp.sendLength);
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;

        case kUSB_DeviceEventSetInterface:
            if (g_DevicePrinterApp.attach)
            {
                /* Set device interface request */
                interface                = (uint8_t)((*param16p & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*param16p & 0x00FFU);

                if (interface < USB_PRINTER_INTERFACE_COUNT)
                {
                    /* demo run */
                    if (alternateSetting < USB_PRINTER_INTERFACE_ALTERNATE_COUNT)
                    {
                        g_DevicePrinterApp.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        g_DevicePrinterApp.printerState = kPrinter_ReceiveNeedPrime;
                        g_DevicePrinterApp.stateChanged = 1;
                        g_DevicePrinterApp.printerState = kPrinter_Idle;

                        status = USB_DevicePrinterSend(g_DevicePrinterApp.classHandle, USB_PRINTER_BULK_ENDPOINT_IN,
                                                       g_DevicePrinterApp.sendBuffer, g_DevicePrinterApp.sendLength);
                    }
                }
            }
            break;

        case kUSB_DeviceEventGetConfiguration:
            if (param)
            {
                /* Get current configuration request */
                *param8p = g_DevicePrinterApp.currentConfiguration;
                status   = kStatus_USB_Success;
            }
            break;

        case kUSB_DeviceEventGetInterface:
            if (param)
            {
                /* Get current alternate setting of the interface request */
                interface = (uint8_t)((*param16p & 0xFF00U) >> 0x08U);
                if (interface < USB_PRINTER_INTERFACE_COUNT)
                {
                    *param16p = (*param16p & 0xFF00U) | g_DevicePrinterApp.currentInterfaceAlternateSetting[interface];
                    status    = kStatus_USB_Success;
                }
                else
                {
                    /* no action, return kStatus_USB_InvalidRequest */
                }
            }
            break;

        case kUSB_DeviceEventGetDeviceDescriptor:
            if (param)
            {
                /* Get device descriptor request */
                status = USB_DeviceGetDeviceDescriptor(handle, (usb_device_get_device_descriptor_struct_t *)param);
            }
            break;

        case kUSB_DeviceEventGetConfigurationDescriptor:
            if (param)
            {
                /* Get device configuration descriptor request */
                status = USB_DeviceGetConfigurationDescriptor(
                    handle, (usb_device_get_configuration_descriptor_struct_t *)param);
            }
            break;

        case kUSB_DeviceEventGetStringDescriptor:
            if (param)
            {
                /* Get device string descriptor request */
                status = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;

        default:
            /* no action, return kStatus_USB_InvalidRequest */
            break;
    }

    return status;
}

static void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* set printer app to default state */
    g_DevicePrinterApp.printerPortStatus = USB_DEVICE_PRINTER_PORT_STATUS_DEFAULT_VALUE;
    g_DevicePrinterApp.printerState      = kPrinter_Idle;
    g_DevicePrinterApp.prnterTaskState   = kPrinter_Idle;
    g_DevicePrinterApp.speed             = USB_SPEED_FULL;
    g_DevicePrinterApp.attach            = 0U;
    g_DevicePrinterApp.classHandle       = (void *)NULL;
    g_DevicePrinterApp.deviceHandle      = NULL;
    g_DevicePrinterApp.printerBuffer     = s_PrinterBuffer;

    /* Initialize the usb stack and class drivers */
    if (kStatus_USB_Success !=
        USB_DeviceClassInit(CONTROLLER_ID, &g_UsbDevicePrinterConfigList, &g_DevicePrinterApp.deviceHandle))
    {
        usb_echo("USB device printer fail\r\n");
        return;
    }
    else
    {
        usb_echo("USB device printer demo\r\n");
        /* Get the printer class handle */
        g_DevicePrinterApp.classHandle = g_UsbDevicePrinterConfigList.config->classHandle;
    }

    /* Install isr, set priority, and enable IRQ. */
    USB_DeviceIsrEnable();

    /* Start USB printer demo */
    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(g_DevicePrinterApp.deviceHandle);
}

void USB_DevicePrinterAppTask(void *parameter)
{
    usb_device_printer_app_t *printerApp = (usb_device_printer_app_t *)parameter;
    usb_status_t status                  = kStatus_USB_Error;
    uint32_t irqMaskValue;

    if (printerApp->attach)
    {
        irqMaskValue = DisableGlobalIRQ();
        if (printerApp->stateChanged)
        {
            printerApp->stateChanged = 0;
            EnableGlobalIRQ(irqMaskValue);
            if (printerApp->printerState == kPrinter_Received)
            {
                USB_PrinterPrintData(printerApp->printerBuffer, printerApp->dataReceiveLength);
                printerApp->prnterTaskState = kPrinter_ReceiveNeedPrime;
            }

            if (printerApp->printerState == kPrinter_ReceiveNeedPrime)
            {
                printerApp->prnterTaskState = kPrinter_ReceiveNeedPrime;
            }
        }
        else
        {
            EnableGlobalIRQ(irqMaskValue);
        }

        if (printerApp->prnterTaskState == kPrinter_ReceiveNeedPrime)
        {
            status = USB_DevicePrinterRecv(printerApp->classHandle, USB_PRINTER_BULK_ENDPOINT_OUT,
                                           printerApp->printerBuffer, USB_PRINTER_BUFFER_SIZE);

            if ((status == kStatus_USB_Success) || (status == kStatus_USB_Busy))
            {
                printerApp->prnterTaskState = kPrinter_Receiving;
            }
        }
    }
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

void APP_task(void *handle)
{
    USB_DeviceApplicationInit();

#if USB_DEVICE_CONFIG_USE_TASK
    if (g_DevicePrinterApp.deviceHandle)
    {
        if (xTaskCreate(USB_DeviceTask,                  /* pointer to the task */
                        "usb device task",               /* task name for kernel awareness debugging */
                        5000L / sizeof(portSTACK_TYPE),  /* task stack size */
                        g_DevicePrinterApp.deviceHandle, /* optional task startup argument */
                        5U,                              /* initial priority */
                        NULL,                            /* optional task handle to create */
                        ) != pdPASS)
        {
            usb_echo("usb device task create failed!\r\n");
            return;
        }
    }
#endif

    while (1U)
    {
        USB_DevicePrinterAppTask(&g_DevicePrinterApp);
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 0u, false);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom0Clk, 1u, true);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB0 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0_DEV_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);

#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USBFSPHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif

    if (xTaskCreate(APP_task,                       /* pointer to the task */
                    "app task",                     /* task name for kernel awareness debugging */
                    2000L / sizeof(portSTACK_TYPE), /* task stack size */
                    &g_DevicePrinterApp,            /* optional task startup argument */
                    4U,                             /* initial priority */
                    NULL                            /* optional task handle to create */
                    ) != pdPASS)
    {
        usb_echo("app task create failed!\r\n");
#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
        return 1U;
#else
        return;
#endif
    }

    vTaskStartScheduler();

#if (defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__))
    return 1U;
#endif
}
