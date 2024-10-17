/*
 * Copyright (c) 2024, Freescale Semiconductor, Inc.
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */
#include <stdio.h>
#include <stdlib.h>
/*${standard_header_anchor}*/
#include "fsl_device_registers.h"
#include "clock_config.h"
#include "fsl_debug_console.h"
#include "board.h"

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#include "usb_device_class.h"
#include "usb_device_cdc_acm.h"
#include "usb_device_ch9.h"

#include "usb_device_descriptor.h"
#include "usb_device_cdc_app.h"
#include "pin_mux.h"
#include "host_sleep.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

#if ((defined FSL_FEATURE_SOC_USBPHY_COUNT) && (FSL_FEATURE_SOC_USBPHY_COUNT > 0U))
#include "usb_phy.h"
#endif

#include "usb_device_dci.h"

#if CONFIG_NCP_USB
#include "ncp_intf_usb_device_cdc.h"
#endif
#include "ncp_adapter.h"
#include "ncp_tlv_adapter.h"
#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_free_rtos.h"

#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
extern uint8_t USB_EnterLowpowerMode(void);
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
void USB_ControllerSuspended(void);

usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param);
usb_status_t USB_DeviceCallback(usb_device_handle handle, uint32_t event, void *param);
void USB_DeviceTask(void *handle);
#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
void usb_pm_task(void *handle);
#endif
/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_device_endpoint_struct_t g_UsbDeviceCdcVcomDicEndpoints[];
extern usb_device_class_struct_t g_UsbDeviceCdcVcomConfig;
/* Data structure of virtual com device */
usb_cdc_vcom_struct_t s_cdcVcom;

#if USB_DEVICE_CONFIG_USE_TASK
/*Hard code for debug, will delete later*/
#define OS_PRIO_0                                4 /** High **/
#define OS_PRIO_1                                (4 - 1)
#define OS_PRIO_2                                (4 - 2)
#define OS_PRIO_3                                (4 - 3)
#define OS_PRIO_4                                (4 - 4) /** Low **/
void USB_DeviceTaskFn(void *deviceHandle);

/* NCP usb device task */
static OSA_TASK_HANDLE_DEFINE(ncp_usb_device_thread);
static OSA_TASK_DEFINE(USB_DeviceTask, OS_PRIO_2, 1, 1024, 0);

#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
/* NCP usb device pm task */
static OSA_TASK_HANDLE_DEFINE(ncp_usb_pm_thread);
static OSA_MSGQ_HANDLE_DEFINE(ncp_usb_pm_event_queue, 10, sizeof(usb_cdc_status_t));
static OSA_TASK_DEFINE(usb_pm_task, OSA_TASK_PRIORITY_MIN, 1, 1024, 0);
#endif
#endif

/* Line coding of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_lineCoding[LINE_CODING_SIZE] = {
    /* E.g. 0x00,0xC2,0x01,0x00 : 0x0001C200 is 115200 bits per second */
    (LINE_CODING_DTERATE >> 0U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 8U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 16U) & 0x000000FFU,
    (LINE_CODING_DTERATE >> 24U) & 0x000000FFU,
    LINE_CODING_CHARFORMAT,
    LINE_CODING_PARITYTYPE,
    LINE_CODING_DATABITS};

/* Abstract state of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_abstractState[COMM_FEATURE_DATA_SIZE] = {(STATUS_ABSTRACT_STATE >> 0U) & 0x00FFU,
                                                          (STATUS_ABSTRACT_STATE >> 8U) & 0x00FFU};

/* Country code of cdc device */
USB_DMA_INIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE)
static uint8_t s_countryCode[COMM_FEATURE_DATA_SIZE] = {(COUNTRY_SETTING >> 0U) & 0x00FFU,
                                                        (COUNTRY_SETTING >> 8U) & 0x00FFU};

/* CDC ACM information */
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static usb_cdc_acm_info_t s_usbCdcAcmInfo;
/* Data buffer for receiving and sending*/
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) static uint8_t s_currRecvBuf[DATA_BUFF_SIZE];
volatile static uint32_t s_recvSize = 0;
volatile static uint32_t s_sendSize = 0;

/* USB device class information */
static usb_device_class_config_struct_t s_cdcAcmConfig[1] = {{
    USB_DeviceCdcVcomCallback,
    0,
    &g_UsbDeviceCdcVcomConfig,
}};

/* USB device class configuration information */
static usb_device_class_config_list_struct_t s_cdcAcmConfigList = {
    s_cdcAcmConfig,
    USB_DeviceCallback,
    1,
};

#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
volatile static uint8_t s_waitForDataReceive = 0;
volatile static uint8_t s_comOpen            = 0;
#endif


/*******************************************************************************
 * Code
 ******************************************************************************/


void USB_ControllerSuspended(void)
{
    while (SYSCTL0->USBCLKSTAT & (SYSCTL0_USBCLKSTAT_DEV_NEED_CLKST_MASK))
    {
        __ASM("nop");
    }
}

/*!
 * @brief CDC class specific callback function.
 *
 * This function handles the CDC class specific requests.
 *
 * @param handle          The CDC ACM class handle.
 * @param event           The CDC ACM class event type.
 * @param param           The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
usb_status_t USB_DeviceCdcVcomCallback(class_handle_t handle, uint32_t event, void *param)
{
    usb_status_t error = kStatus_USB_InvalidRequest;
#if ((defined USB_DEVICE_CONFIG_CDC_CIC_EP_DISABLE) && (USB_DEVICE_CONFIG_CDC_CIC_EP_DISABLE > 0U))
#else
    uint32_t len;
#endif
    uint8_t *uartBitmap;
    usb_cdc_acm_info_t *acmInfo = &s_usbCdcAcmInfo;
    usb_device_cdc_acm_request_param_struct_t *acmReqParam;
    usb_device_endpoint_callback_message_struct_t *epCbParam;
    acmReqParam = (usb_device_cdc_acm_request_param_struct_t *)param;
    epCbParam   = (usb_device_endpoint_callback_message_struct_t *)param;
    switch (event)
    {
        case kUSB_DeviceCdcEventSendResponse:
        {
            if ((epCbParam->length != 0) &&
                (0U == (epCbParam->length % g_UsbDeviceCdcVcomDicEndpoints[0].maxPacketSize)))
            {
                /* If the last packet is the size of endpoint, then send also zero-ended packet,
                 ** meaning that we want to inform the host that we do not have any additional
                 ** data, so it can flush the output.
                 */
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_BULK_IN_ENDPOINT, NULL, 0);
            }
            else
            {
#if CONFIG_NCP_USB
                ncp_usb_put_tx_sem();
#endif
            }
        }
        break;
        case kUSB_DeviceCdcEventRecvResponse:
        {
            if ((1U == s_cdcVcom.attach) /* && (1U == s_cdcVcom.startTransactions)*/)
            {
                s_recvSize = epCbParam->length;
                error      = kStatus_USB_Success;

#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
                s_waitForDataReceive = 0;
                USB0->INTEN |= USB_INTEN_SOFTOKEN_MASK;
#endif
                /* s_recvSize will be as a error value when hotplug
                 * but it's very important for wifi driver, so check it
                 */
                if (s_recvSize > 0 && s_recvSize <= DATA_BUFF_SIZE)
                {
#if CONFIG_NCP_USB
                    ncp_usb_device_recv(&s_currRecvBuf[0], s_recvSize);
#endif
                }

                /* Schedule buffer for next receive event */
                error = USB_DeviceCdcAcmRecv(handle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                             g_UsbDeviceCdcVcomDicEndpoints[1].maxPacketSize);
#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
                s_waitForDataReceive = 1;
                USB0->INTEN &= ~USB_INTEN_SOFTOKEN_MASK;
#endif
            }
        }
        break;
        case kUSB_DeviceCdcEventSerialStateNotif:
            ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 0;
            error                                                 = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSendEncapsulatedCommand:
            break;
        case kUSB_DeviceCdcEventGetEncapsulatedResponse:
            break;
        case kUSB_DeviceCdcEventSetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                if (1U == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_abstractState;
                    *(acmReqParam->length) = sizeof(s_abstractState);
                }
                else
                {
                    /* no action, data phase, s_abstractState has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                if (1U == acmReqParam->isSetup)
                {
                    *(acmReqParam->buffer) = s_countryCode;
                    *(acmReqParam->length) = sizeof(s_countryCode);
                }
                else
                {
                    /* no action, data phase, s_countryCode has been assigned */
                }
                error = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventGetCommFeature:
            if (USB_DEVICE_CDC_FEATURE_ABSTRACT_STATE == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_abstractState;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else if (USB_DEVICE_CDC_FEATURE_COUNTRY_SETTING == acmReqParam->setupValue)
            {
                *(acmReqParam->buffer) = s_countryCode;
                *(acmReqParam->length) = COMM_FEATURE_DATA_SIZE;
                error                  = kStatus_USB_Success;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceCdcEventClearCommFeature:
            break;
        case kUSB_DeviceCdcEventGetLineCoding:
            *(acmReqParam->buffer) = s_lineCoding;
            *(acmReqParam->length) = LINE_CODING_SIZE;
            error                  = kStatus_USB_Success;
            break;
        case kUSB_DeviceCdcEventSetLineCoding:
        {
            if (1U == acmReqParam->isSetup)
            {
                *(acmReqParam->buffer) = s_lineCoding;
                *(acmReqParam->length) = sizeof(s_lineCoding);
            }
            else
            {
                /* no action, data phase, s_lineCoding has been assigned */
            }
            error = kStatus_USB_Success;
        }
        break;
        case kUSB_DeviceCdcEventSetControlLineState:
        {
            s_usbCdcAcmInfo.dteStatus = acmReqParam->setupValue;
            /* activate/deactivate Tx carrier */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_TX_CARRIER;
            }

            /* activate carrier and DTE. Com port of terminal tool running on PC is open now */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE)
            {
                acmInfo->uartState |= USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }
            /* Com port of terminal tool running on PC is closed now */
            else
            {
                acmInfo->uartState &= (uint16_t)~USB_DEVICE_CDC_UART_STATE_RX_CARRIER;
            }

            /* Indicates to DCE if DTE is present or not */
            acmInfo->dtePresent = (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_DTE_PRESENCE) ? true : false;

            /* Initialize the serial state buffer */
            acmInfo->serialStateBuf[0] = NOTIF_REQUEST_TYPE;                /* bmRequestType */
            acmInfo->serialStateBuf[1] = USB_DEVICE_CDC_NOTIF_SERIAL_STATE; /* bNotification */
            acmInfo->serialStateBuf[2] = 0x00;                              /* wValue */
            acmInfo->serialStateBuf[3] = 0x00;
            acmInfo->serialStateBuf[4] = 0x00; /* wIndex */
            acmInfo->serialStateBuf[5] = 0x00;
            acmInfo->serialStateBuf[6] = UART_BITMAP_SIZE; /* wLength */
            acmInfo->serialStateBuf[7] = 0x00;
            /* Notify to host the line state */
            acmInfo->serialStateBuf[4] = acmReqParam->interfaceIndex;
            /* Lower byte of UART BITMAP */
            uartBitmap    = (uint8_t *)&acmInfo->serialStateBuf[NOTIF_PACKET_SIZE + UART_BITMAP_SIZE - 2];
            uartBitmap[0] = acmInfo->uartState & 0xFFu;
            uartBitmap[1] = (acmInfo->uartState >> 8) & 0xFFu;
#if ((defined USB_DEVICE_CONFIG_CDC_CIC_EP_DISABLE) && (USB_DEVICE_CONFIG_CDC_CIC_EP_DISABLE > 0U))
#else
            len = (uint32_t)(NOTIF_PACKET_SIZE + UART_BITMAP_SIZE);
            if (0U == ((usb_device_cdc_acm_struct_t *)handle)->hasSentState)
            {
                error = USB_DeviceCdcAcmSend(handle, USB_CDC_VCOM_INTERRUPT_IN_ENDPOINT, acmInfo->serialStateBuf, len);
                if (kStatus_USB_Success != error)
                {
                    usb_echo("kUSB_DeviceCdcEventSetControlLineState error!");
                }
                ((usb_device_cdc_acm_struct_t *)handle)->hasSentState = 1;
            }
#endif
            /* Update status */
            if (acmInfo->dteStatus & USB_DEVICE_CDC_CONTROL_SIG_BITMAP_CARRIER_ACTIVATION)
            {
                /*  To do: CARRIER_ACTIVATED */
            }
            else
            {
                /* To do: CARRIER_DEACTIVATED */
            }

            if (1U == s_cdcVcom.attach)
            {
                s_cdcVcom.startTransactions = 1;
#if defined(FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED) && (FSL_FEATURE_USB_KHCI_KEEP_ALIVE_ENABLED > 0U) && \
    defined(USB_DEVICE_CONFIG_KEEP_ALIVE_MODE) && (USB_DEVICE_CONFIG_KEEP_ALIVE_MODE > 0U) &&             \
    defined(FSL_FEATURE_USB_KHCI_USB_RAM) && (FSL_FEATURE_USB_KHCI_USB_RAM > 0U)
                s_waitForDataReceive = 1;
                USB0->INTEN &= ~USB_INTEN_SOFTOKEN_MASK;
                s_comOpen = 1;
                usb_echo("USB_APP_CDC_DTE_ACTIVATED\r\n");
#endif
            }
            error = kStatus_USB_Success;
        }
        break;
        case kUSB_DeviceCdcEventSendBreak:
            break;
        default:
            break;
    }

    return error;
}

static void USB_DevicePmEventPut(usb_cdc_status_t event)
{
    usb_cdc_status_t msg = event;
    (void)OSA_MsgQPut(ncp_usb_pm_event_queue, &msg);
}

void USB_DevicePmStartResume(void)
{
    if (s_cdcVcom.attach)
    {
        usb_echo("PM task StartResume!\r\n");
        USB_DevicePmEventPut(kStatus_StartResume);
    }
    else
    {
        USB_DevicePmEventPut(kStatus_Idle);
    }
}

/*!
 * @brief USB device callback function.
 *
 * This function handles the usb device specific requests.
 *
 * @param handle          The USB device handle.
 * @param event           The USB device event type.
 * @param param           The parameter of the device specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
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
            s_cdcVcom.attach               = 0;
            s_cdcVcom.currentConfiguration = 0U;
            error                          = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U))
            /* Get USB speed to configure the device, including max packet size and interval of the endpoints. */
            if (kStatus_USB_Success == USB_DeviceClassGetSpeed(CONTROLLER_ID, &s_cdcVcom.speed))
            {
                USB_DeviceSetSpeed(handle, s_cdcVcom.speed);
            }
#endif
        }
        break;
#if (defined(USB_DEVICE_CONFIG_DETACH_ENABLE) && (USB_DEVICE_CONFIG_DETACH_ENABLE > 0U))
        case kUSB_DeviceEventAttach:
        {
            error = kStatus_USB_Success;
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))
#else
            /*Add one delay here to make the DP pull down long enough to allow host to detect the previous
             * disconnection.*/
            SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
            USB_DeviceRun(s_cdcVcom.deviceHandle);
#endif
        }
        break;
        case kUSB_DeviceEventDetach:
        {
            error            = kStatus_USB_Success;
            s_cdcVcom.attach = 0;
#if (defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)) || \
    (defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U))

#else
            USB_DeviceStop(s_cdcVcom.deviceHandle);
#endif
        }
            usb_echo("USB device detach\r\n");
            break;
#endif

        case kUSB_DeviceEventSuspend:
        {
            /* USB device bus suspend signal detected */
            if (s_cdcVcom.attach)
            {
                usb_echo("USB device start suspend\r\n");
                USB_ControllerSuspended();
                s_cdcVcom.startTick = s_cdcVcom.hwTick;
                USB_DevicePmEventPut(kStatus_StartSuspend);
                error               = kStatus_USB_Success;
            }
        }
        break;

        case kUSB_DeviceEventResume:
        {
            /* USB device bus resume signal detected */
            if ((s_cdcVcom.attach) && (kStatus_Idle != s_cdcVcom.suspend))
            {
                s_cdcVcom.isResume = 1U;
                /* Mark wakeup source as ncp host side remote wakeup
                 * Active remote wakeup step1: recved remote wakeup signal
                 * Passive remote wakeup step3
                 */
                s_cdcVcom.selfWakeup = 0U;
                USB_DevicePmEventPut(kStatus_Resumed);

                usb_echo("USB device start resume\r\n");
                error = kStatus_USB_Success;
            }
        }
        break;

        case kUSB_DeviceEventSetRemoteWakeup:
            if (param)
            {
                if (s_cdcVcom.attach)
                {
                    s_cdcVcom.remoteWakeup = *temp8;
                    usb_echo("USB device remote wakeup state: %d\r\n", s_cdcVcom.remoteWakeup);
                    error = kStatus_USB_Success;
                }
            }
            break;

        case kUSB_DeviceEventSetConfiguration:
            if (0U == (*temp8))
            {
                s_cdcVcom.attach               = 0;
                s_cdcVcom.currentConfiguration = 0U;
                error                          = kStatus_USB_Success;
            }
            else if (USB_CDC_VCOM_CONFIGURE_INDEX == (*temp8))
            {
                s_cdcVcom.currentConfiguration = *temp8;
                error                          = kStatus_USB_Success;
                /* Schedule buffer for receive */
                USB_DeviceCdcAcmRecv(s_cdcVcom.cdcAcmHandle, USB_CDC_VCOM_BULK_OUT_ENDPOINT, s_currRecvBuf,
                                     g_UsbDeviceCdcVcomDicEndpoints[1].maxPacketSize);
                s_cdcVcom.attach               = 1;
            }
            else
            {
                /* no action, return kStatus_USB_InvalidRequest */
            }
            break;
        case kUSB_DeviceEventSetInterface:
            if (0U != s_cdcVcom.attach)
            {
                uint8_t interface        = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                uint8_t alternateSetting = (uint8_t)(*temp16 & 0x00FFU);
                if (interface == USB_CDC_VCOM_COMM_INTERFACE_INDEX)
                {
                    if (alternateSetting < USB_CDC_VCOM_COMM_INTERFACE_ALTERNATE_COUNT)
                    {
                        s_cdcVcom.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                 = kStatus_USB_Success;
                    }
                }
                else if (interface == USB_CDC_VCOM_DATA_INTERFACE_INDEX)
                {
                    if (alternateSetting < USB_CDC_VCOM_DATA_INTERFACE_ALTERNATE_COUNT)
                    {
                        s_cdcVcom.currentInterfaceAlternateSetting[interface] = alternateSetting;
                        error                                                 = kStatus_USB_Success;
                    }
                }
                else
                {
                    /* no action, return kStatus_USB_InvalidRequest */
                }
            }
            break;
        case kUSB_DeviceEventGetConfiguration:
            if (NULL != param)
            {
                /* Get current configuration request */
                *temp8 = s_cdcVcom.currentConfiguration;
                error  = kStatus_USB_Success;
            }
            break;
        case kUSB_DeviceEventGetInterface:
            if (NULL != param)
            {
                /* Get current alternate setting of the interface request */
                uint8_t interface = (uint8_t)((*temp16 & 0xFF00U) >> 0x08U);
                if (interface < USB_CDC_VCOM_INTERFACE_COUNT)
                {
                    *temp16 = (*temp16 & 0xFF00U) | s_cdcVcom.currentInterfaceAlternateSetting[interface];
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
        case kUSB_DeviceEventGetStringDescriptor:
            if (NULL != param)
            {
                /* Get device string descriptor request */
                error = USB_DeviceGetStringDescriptor(handle, (usb_device_get_string_descriptor_struct_t *)param);
            }
            break;
        default:
            /* no action, return kStatus_USB_InvalidRequest */
            break;
    }

    return error;
}

static void CDC_VCOM_FreeRTOSEnterCritical(uint32_t *sr)
{
    *sr = DisableGlobalIRQ();
    __ASM("CPSID i");
}

static void CDC_VCOM_FreeRTOSExitCritical(uint32_t sr)
{
    EnableGlobalIRQ(sr);
}

/*!
 * @brief Application initialization function.
 *
 * This function initializes the application.
 *
 * @return None.
 */
void USB_DeviceApplicationInit(void)
{
    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    s_cdcVcom.speed        = USB_SPEED_FULL;
    s_cdcVcom.attach       = 0;
    s_cdcVcom.cdcAcmHandle = (class_handle_t)NULL;
    s_cdcVcom.deviceHandle = NULL;
    s_cdcVcom.isResume     = 0U;
    s_cdcVcom.selfWakeup   = 0U;
    s_cdcVcom.remoteWakeup = 0U;

    if (kStatus_USB_Success != USB_DeviceClassInit(CONTROLLER_ID, &s_cdcAcmConfigList, &s_cdcVcom.deviceHandle))
    {
        usb_echo("USB device init failed\r\n");
    }
    else
    {
        usb_echo("USB device CDC virtual com demo\r\n");
        s_cdcVcom.cdcAcmHandle = s_cdcAcmConfigList.config->classHandle;
    }

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(s_cdcVcom.deviceHandle);
}
#if CONFIG_NCP_USB
void USBHS_IRQHandler(void)
{
    USB_DeviceEhciIsrFunction(s_cdcVcom.deviceHandle);
}
#endif
void USB_DeviceClockInit(void)
{
    /* reset USB */
    RESET_PeripheralReset(kUSB_RST_SHIFT_RSTn);
    /* enable usb clock */
    CLOCK_EnableClock(kCLOCK_Usb);
    /* enable usb phy clock */
    CLOCK_EnableUsbhsPhyClock();
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
 * @brief USB task function.
 *
 * This function runs the task for USB device.
 *
 * @return None.
 */
#if USB_DEVICE_CONFIG_USE_TASK
void USB_DeviceTask(void *handle)
{
    while (1U)
    {
        USB_DeviceTaskFn(handle);
    }
}
#endif

#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))

void usb_pm_task(void *handle)
{
    usb_cdc_status_t usbPmEvent;

    while (1)
    {
        (void)OSA_MsgQGet((osa_msgq_handle_t)ncp_usb_pm_event_queue, &usbPmEvent, osaWaitForever_c);

        switch (usbPmEvent)
        {
            case kStatus_Idle:
                s_cdcVcom.suspend = kStatus_Idle;
                break;
            case kStatus_StartSuspend:
                /* Defualt wakeup by ncp device side irq*/
                s_cdcVcom.selfWakeup = 1U;
                USB_DevicePmEventPut(kStatus_Suspending);
                break;
            case kStatus_Suspending:
                s_cdcVcom.suspend = kStatus_Suspending;
                usb_echo("USB device suspended.\r\n");
                if (s_cdcVcom.remoteWakeup)
                {
                    usb_echo("support remote wakeup.\r\n");
                }
                USB_DevicePmEventPut(kStatus_Suspended);
                break;
            case kStatus_Suspended:
                s_cdcVcom.suspend = kStatus_Suspended;
                USB_DeviceSetStatus(s_cdcVcom.deviceHandle, kUSB_DeviceStatusBusSuspend, NULL);

                if (kStatus_Success != ncp_config_suspend_mode(2))
                {
                    usb_echo("Enter VLPS mode failed!\r\n");
                    USB_DevicePmEventPut(kStatus_Idle);
                    break;
                }
                else
                {
                    /* Do nothing */
                }

#if CONFIG_NCP_WIFI
                /* In wlan manual low power mode, cpu3 will start resume usb interface at here */
                USB_DevicePmStartResume();
#endif
                break;
            case kStatus_StartResume:
                s_cdcVcom.suspend = kStatus_StartResume;
                /* s_cdcVcom.selfWakeup
                 * true  : wakeup by ncp device side irq source
                 * false : wakeup by remote wakeup of ncp host
                 */
                /* Active remote wakeup step3:
                 *    If wakeup from PM2 with remote wakeup, skip this step
                 * Passive remote wakeup step2:
                 *    If wakeup from ncp device side irq source, need to use remote wakeup to align usb states with ncp host side
                 */
                if (s_cdcVcom.selfWakeup)
                {
                    s_cdcVcom.selfWakeup = 0U;
                    if (s_cdcVcom.remoteWakeup)
                    {
                        usb_echo("Remote wakeup the host.\r\n");
                        if (kStatus_USB_Success ==
                            USB_DeviceSetStatus(s_cdcVcom.deviceHandle, kUSB_DeviceStatusBusResume, NULL))
                        {
                            USB_DevicePmEventPut(kStatus_Resuming);
                        }
                        else
                        {
                            usb_echo("Send resume signal failed.\r\n");
                            USB_DevicePmEventPut(kStatus_StartResume);
                        }
                    }
                    else
                    {
                        USB_DevicePmEventPut(kStatus_Resuming);
                    }
                }
                else
                {
                    USB_DevicePmEventPut(kStatus_Resumed);
                }
                break;
            case kStatus_Resuming:
                s_cdcVcom.suspend = kStatus_Resuming;
                break;
            case kStatus_Resumed:
                s_cdcVcom.isResume = 0U;
                s_cdcVcom.suspend = kStatus_Resumed;
                //Passive remote wakeup step4
                usb_echo("USB device resumed.\r\n");
                USB_DevicePmEventPut(kStatus_Idle);
                break;
            default:
                USB_DevicePmEventPut(kStatus_Idle);
                break;
        }
    }
}
#endif

/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
int usb_device_init()
{

    USB_DeviceApplicationInit();

#if USB_DEVICE_CONFIG_USE_TASK
    (void)OSA_TaskCreate((osa_task_handle_t)ncp_usb_device_thread, OSA_TASK(USB_DeviceTask), s_cdcVcom.deviceHandle);

#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
    (void)OSA_TaskCreate((osa_task_handle_t)ncp_usb_pm_thread, OSA_TASK(usb_pm_task), (osa_task_param_t)NULL);
    (void)OSA_MsgQCreate((osa_msgq_handle_t)ncp_usb_pm_event_queue, 10, sizeof(usb_cdc_status_t));
#endif
#endif

    return NCP_STATUS_SUCCESS;
}

int USB_DeviceEnterPowerDown(void)
{
    usb_device_struct_t *deviceHandle = (usb_device_struct_t *)s_cdcVcom.deviceHandle;

    /* De-initialize the USB device*/
    (void)deviceHandle->controllerInterface->deviceDeinit(deviceHandle->controllerHandle);

    return NCP_STATUS_SUCCESS;
}

int USB_DeviceExitPowerDown(void)
{
    usb_device_struct_t *deviceHandle = (usb_device_struct_t *)s_cdcVcom.deviceHandle;

    USB_DeviceClockInit();
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
    SYSMPU_Enable(SYSMPU, 0);
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */

    /* Clear the virtual com device */
    s_cdcVcom.attach       = 0;
    s_cdcVcom.isResume     = 0U;
    s_cdcVcom.selfWakeup   = 0U;
    s_cdcVcom.remoteWakeup = 0U;

    /* Clear the device address */
    deviceHandle->deviceAddress = 0U;
    /* Clear the device reset state */
    deviceHandle->isResetting = 0U;

    /* Re-initialize the USB device*/
    (void)deviceHandle->controllerInterface->deviceInit(deviceHandle->controllerId, deviceHandle, &deviceHandle->controllerHandle);
    /* Set the device to default state */
    deviceHandle->state = (uint8_t)kUSB_DeviceStateDefault;

    USB_DeviceIsrEnable();

    /*Add one delay here to make the DP pull down long enough to allow host to detect the previous disconnection.*/
    SDK_DelayAtLeastUs(5000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
    USB_DeviceRun(deviceHandle);

    return NCP_STATUS_SUCCESS;
}

int usb_device_deinit(void)
{
    OSA_DisableScheduler();
    OSA_TaskDestroy(ncp_usb_device_thread);
#if ((defined(USB_DEVICE_CONFIG_LOW_POWER_MODE)) && (USB_DEVICE_CONFIG_LOW_POWER_MODE > 0U))
    OSA_TaskDestroy(ncp_usb_pm_thread);
#endif
    OSA_EnableScheduler();

    //	USB_DeviceIsrDisable();
    if (0 != USB_DeviceClassDeinit(CONTROLLER_ID))
    {
        usb_echo("USB device deinit failed\r\n");
        return NCP_STATUS_ERROR;
    }

    return NCP_STATUS_SUCCESS;
}

int usb_device_reinit(void)
{
    CLOCK_EnableXtal32K(true);
    /* TODO: Use XTAL32K on real board */
    CLOCK_AttachClk(kRC32K_to_CLK32K);
    CLOCK_AttachClk(kLPOSC_to_OSTIMER_CLK);

    return usb_device_init();
}
