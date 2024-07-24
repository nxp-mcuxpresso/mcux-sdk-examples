/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2017, 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"
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
#include "fsl_mrt.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/* The printer class callback */
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);

void BOARD_InitHardware(void);
void USB_DeviceClockInit(void);
void USB_DeviceIsrEnable(void);
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTaskFn(void *deviceHandle);
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
volatile uint32_t hwTick;
uint32_t timerInterval;
uint32_t isConnectedToFsHost = 0U;
uint32_t isConnectedToHsHost = 0U;

USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t s_PrinterBuffer[USB_PRINTER_BUFFER_SIZE];
/* printer class specifice transfer buffer */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_PrinterClassBuffer[64];
/* printer device id for interface zero */
const uint8_t g_PrinterID[] = "xxMFG:NXP;MDL:ksdk printer demo;CMD:POSTSCRIPT";

usb_device_printer_app_t g_DevicePrinterApp;

extern uint8_t g_UsbDeviceCurrentConfigure;
extern uint8_t g_UsbDeviceInterface[USB_PRINTER_INTERFACE_COUNT];

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
    USB_DeviceLpcIp3511IsrFunction(g_DevicePrinterApp.deviceHandle);
}
#endif
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
void USB1_IRQHandler(void)
{
    USB_DeviceLpcIp3511IsrFunction(g_DevicePrinterApp.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
    /* enable USB IP clock */
    CLOCK_EnableUsbfs0DeviceClock(kCLOCK_UsbSrcFro, CLOCK_GetFroHfFreq());
#if defined(FSL_FEATURE_USB_USB_RAM) && (FSL_FEATURE_USB_USB_RAM)
    for (int i = 0; i < FSL_FEATURE_USB_USB_RAM; i++)
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
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Fs0];
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
    uint8_t usbDeviceIP3511Irq[] = USBHSD_IRQS;
    irqNumber                    = usbDeviceIP3511Irq[CONTROLLER_ID - kUSB_ControllerLpcIp3511Hs0];
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

/* ksdk debug console must have been initialized. */
static void USB_PrinterPrintData(uint8_t *data, uint32_t length)
{
    while (length--)
    {
        PUTCHAR(*(data++));
    }
}

/*!
 * @brief bulk IN endpoint callback function.
 */
static usb_status_t USB_DevicePrinterBulkInCallback(usb_device_handle handle,
                                                    usb_device_endpoint_callback_message_struct_t *message,
                                                    void *callbackParam)
{
    USB_DeviceSendRequest(g_DevicePrinterApp.deviceHandle, USB_PRINTER_BULK_ENDPOINT_IN, g_DevicePrinterApp.sendBuffer,
                          g_DevicePrinterApp.sendLength);
    return kStatus_USB_Success;
}

/*!
 * @brief bulk OUT endpoint callback function.
 */
static usb_status_t USB_DevicePrinterBulkOutCallback(usb_device_handle handle,
                                                     usb_device_endpoint_callback_message_struct_t *message,
                                                     void *callbackParam)
{
    usb_status_t status = kStatus_USB_Error;

    if ((g_DevicePrinterApp.attach) && (g_DevicePrinterApp.prnterTaskState == kPrinter_Receiving))
    {
        /* endpoint callback length is USB_CANCELLED_TRANSFER_LENGTH (0xFFFFFFFFU) when transfer is canceled */
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
    }

    return status;
}

usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param)
{
    usb_status_t status = kStatus_USB_InvalidRequest;
    uint8_t *param8p    = (uint8_t *)param;

    switch (event)
    {
        case kUSB_DeviceEventBusReset:
            /* Initialize the control IN and OUT pipes */
            USB_DeviceControlPipeInit(g_DevicePrinterApp.deviceHandle);
            g_DevicePrinterApp.attach          = 0U;
            g_DevicePrinterApp.printerState    = kPrinter_Idle;
            g_DevicePrinterApp.prnterTaskState = kPrinter_Idle;
            g_DevicePrinterApp.stateChanged    = 1;
            g_DevicePrinterApp.sendBuffer      = NULL;
            g_DevicePrinterApp.sendLength      = 0;

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
            if (kStatus_USB_Success ==
                USB_DeviceGetStatus(g_DevicePrinterApp.deviceHandle, kUSB_DeviceStatusSpeed, &g_DevicePrinterApp.speed))
            {
                USB_DeviceSetSpeed(g_DevicePrinterApp.speed);
            }
#endif
            status = kStatus_USB_Success;
            break;

#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))

        case kUSB_DeviceEventAttach:
            usb_echo("USB device attached.\r\n");
            /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
            SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            USB_DeviceRun(g_DevicePrinterApp.deviceHandle);
            status = kStatus_USB_Success;
            break;

        case kUSB_DeviceEventDetach:
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
#if !((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
            USB_DeviceDisconnected();
#endif
#endif
            usb_echo("USB device detached.\r\n");
            g_DevicePrinterApp.attach = 0;
            USB_DeviceStop(g_DevicePrinterApp.deviceHandle);
            status = kStatus_USB_Success;
            break;
#endif

        case kUSB_DeviceEventSetConfiguration:
            if (USB_PRINTER_CONFIGURE_INDEX == (*param8p))
            {
                /* If the configuration is valid, initialize the printer pipes */
                usb_device_endpoint_init_struct_t epInitStruct;
                usb_device_endpoint_callback_struct_t epCallback;

                epInitStruct.zlt          = 0U;
                epInitStruct.interval     = 0U;
                epInitStruct.transferType = USB_ENDPOINT_BULK;
                epCallback.callbackParam  = handle;

                /* initialize bulk out endpoint */
                epCallback.callbackFn = USB_DevicePrinterBulkOutCallback;
                epInitStruct.endpointAddress =
                    USB_PRINTER_BULK_ENDPOINT_OUT | (USB_OUT << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
                if (USB_SPEED_HIGH == g_DevicePrinterApp.speed)
                {
                    epInitStruct.maxPacketSize = HS_PRINTER_BULK_OUT_PACKET_SIZE;
                }
                else
                {
                    epInitStruct.maxPacketSize = FS_PRINTER_BULK_OUT_PACKET_SIZE;
                }

                USB_DeviceInitEndpoint(g_DevicePrinterApp.deviceHandle, &epInitStruct, &epCallback);

                /* initialize bulk in endpoint */
                epCallback.callbackFn = USB_DevicePrinterBulkInCallback;
                epInitStruct.endpointAddress =
                    USB_PRINTER_BULK_ENDPOINT_IN | (USB_IN << USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_SHIFT);
                if (USB_SPEED_HIGH == g_DevicePrinterApp.speed)
                {
                    epInitStruct.maxPacketSize = HS_PRINTER_BULK_IN_PACKET_SIZE;
                }
                else
                {
                    epInitStruct.maxPacketSize = FS_PRINTER_BULK_IN_PACKET_SIZE;
                }

                USB_DeviceInitEndpoint(g_DevicePrinterApp.deviceHandle, &epInitStruct, &epCallback);

                g_DevicePrinterApp.attach = 1U;
                /* demo run */
                g_DevicePrinterApp.printerState = kPrinter_ReceiveNeedPrime;
                g_DevicePrinterApp.stateChanged = 1;

                status = USB_DeviceSendRequest(g_DevicePrinterApp.deviceHandle, USB_PRINTER_BULK_ENDPOINT_IN,
                                           g_DevicePrinterApp.sendBuffer, g_DevicePrinterApp.sendLength);
            }
            break;
        case kUSB_DeviceEventSetInterface:
            status = kStatus_USB_Success;
            break;
        default:
            break;
    }

    return status;
}

/* Get setup buffer */
usb_status_t USB_DeviceGetSetupBuffer(usb_device_handle handle, usb_setup_struct_t **setupBuffer)
{
    /* Keep the setup is 4-byte aligned */
    static uint32_t printerAppSetup[2];
    if (NULL == setupBuffer)
    {
        return kStatus_USB_InvalidParameter;
    }
    *setupBuffer = (usb_setup_struct_t *)&printerAppSetup;
    return kStatus_USB_Success;
}

/* Configure device remote wakeup */
usb_status_t USB_DeviceConfigureRemoteWakeup(usb_device_handle handle, uint8_t enable)
{
    return kStatus_USB_InvalidRequest;
}

/* Configure the endpoint status (idle or stall) */
usb_status_t USB_DeviceConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status)
{
    if (status)
    {
        if ((USB_PRINTER_BULK_ENDPOINT_OUT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) == USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else if ((USB_PRINTER_BULK_ENDPOINT_IN == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                  USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN))
        {
            return USB_DeviceStallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    else
    {
        if ((USB_PRINTER_BULK_ENDPOINT_OUT == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
            ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) == USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_OUT))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else if ((USB_PRINTER_BULK_ENDPOINT_IN == (ep & USB_ENDPOINT_NUMBER_MASK)) &&
                 ((ep & USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_MASK) ==
                  USB_DESCRIPTOR_ENDPOINT_ADDRESS_DIRECTION_IN))
        {
            return USB_DeviceUnstallEndpoint(handle, ep);
        }
        else
        {
        }
    }
    return kStatus_USB_InvalidRequest;
}

/* Get class-specific request buffer */
usb_status_t USB_DeviceGetClassReceiveBuffer(usb_device_handle handle,
                                             usb_setup_struct_t *setup,
                                             uint32_t *length,
                                             uint8_t **buffer)
{
    *buffer = NULL;
    *length = 0;
    return kStatus_USB_InvalidRequest;
}

/* Handle class-specific request */
usb_status_t USB_DeviceProcessClassRequest(usb_device_handle handle,
                                           usb_setup_struct_t *setup,
                                           uint32_t *length,
                                           uint8_t **buffer)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
    uint32_t len;

    if ((setup->bmRequestType & USB_REQUEST_TYPE_RECIPIENT_MASK) != USB_REQUEST_TYPE_RECIPIENT_INTERFACE)
    {
        return error;
    }

    switch (setup->bRequest)
    {
        case USB_DEVICE_PRINTER_GET_DEVICE_ID:
            if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN)
                && (setup->wLength != 0U) && ((uint8_t)(setup->wIndex >> 8) == USB_PRINTER_INTERFACE_INDEX))
            {
                for (len = 0; len < sizeof(g_PrinterID); ++len)
                {
                    s_PrinterClassBuffer[len] = g_PrinterID[len];
                }
                len                     = sizeof(g_PrinterID) - 1;
                s_PrinterClassBuffer[0] = ((uint8_t)(len >> 8));
                s_PrinterClassBuffer[1] = (uint8_t)len;
                *buffer                 = s_PrinterClassBuffer;
                *length                 = len;
                error                   = kStatus_USB_Success;
            }
            break;

        case USB_DEVICE_PRINTER_GET_PORT_STATUS:
            if (((setup->bmRequestType & USB_REQUEST_TYPE_DIR_MASK) == USB_REQUEST_TYPE_DIR_IN)
                && (setup->wLength == 1U) && ((uint8_t)(setup->wIndex) == USB_PRINTER_INTERFACE_INDEX))
            {
                s_PrinterClassBuffer[0] = g_DevicePrinterApp.printerPortStatus;
                *buffer                 = s_PrinterClassBuffer;
                *length                 = 1U;
                error                   = kStatus_USB_Success;
            }
            break;

        case USB_DEVICE_PRINTER_SOFT_RESET:
            break;
        default:
            break;
    }

    return error;
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
    g_DevicePrinterApp.prnterTaskState   = kPrinter_Idle;
    g_DevicePrinterApp.speed             = USB_SPEED_FULL;
    g_DevicePrinterApp.attach            = 0U;
    g_DevicePrinterApp.deviceHandle      = NULL;
    g_DevicePrinterApp.printerBuffer     = s_PrinterBuffer;

    /* Initialize the usb stack and class drivers */
    if (kStatus_USB_Success != USB_DeviceInit(CONTROLLER_ID, USB_DeviceCallback, &g_DevicePrinterApp.deviceHandle))
    {
        usb_echo("USB device printer fail\r\n");
        return;
    }
    else
    {
        usb_echo("USB device printer demo\r\n");
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
            status = USB_DeviceRecvRequest(printerApp->deviceHandle, USB_PRINTER_BULK_ENDPOINT_OUT,
                                           printerApp->printerBuffer, USB_PRINTER_BUFFER_SIZE);

            if ((status == kStatus_USB_Success) || (status == kStatus_USB_Busy))
            {
                printerApp->prnterTaskState = kPrinter_Receiving;
            }
        }
    }
}

#if defined(__CC_ARM) || (defined(__ARMCC_VERSION)) || defined(__GNUC__)
int main(void)
#else
void main(void)
#endif
{
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* reset USB0 and USB1 device */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    BOARD_InitBootPins();
    BOARD_BootClockFROHF96M();
    BOARD_InitDebugConsole();
#if (defined USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS)
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY);
    /* enable usb1 host clock */
    CLOCK_EnableClock(kCLOCK_Usbh1);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBHSH_BASE + 0x50)) |= USBHSH_PORTMODE_DEV_ENABLE_MASK;
    /* enable usb1 host clock */
    CLOCK_DisableClock(kCLOCK_Usbh1);
    USB_TimerInit(0, 1000U);
#endif
#if (defined USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS)
    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB Phy */
    CLOCK_SetClkDiv(kCLOCK_DivUsb0Clk, 1, false);
    CLOCK_AttachClk(kFRO_HF_to_USB0_CLK);
    /* enable usb0 host clock */
    CLOCK_EnableClock(kCLOCK_Usbhsl0);
    /*According to reference mannual, device mode setting has to be set by access usb host register */
    *((uint32_t *)(USBFSH_BASE + 0x5C)) |= USBFSH_PORTMODE_DEV_ENABLE_MASK;
    /* disable usb0 host clock */
    CLOCK_DisableClock(kCLOCK_Usbhsl0);
#endif

    USB_DeviceApplicationInit();

    while (1U)
    {
#if USB_DEVICE_CONFIG_USE_TASK
        USB_DeviceTaskFn(g_DevicePrinterApp.deviceHandle);
#endif

        USB_DevicePrinterAppTask(&g_DevicePrinterApp);
    }
}
