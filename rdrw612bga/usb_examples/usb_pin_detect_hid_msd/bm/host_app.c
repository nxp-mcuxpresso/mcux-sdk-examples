/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "usb_host_msd.h"
#include "host_msd_fatfs.h"
#include "fsl_common.h"
#include "board.h"
#if (defined(FSL_FEATURE_SOC_SYSMPU_COUNT) && (FSL_FEATURE_SOC_SYSMPU_COUNT > 0U))
#include "fsl_sysmpu.h"
#endif /* FSL_FEATURE_SOC_SYSMPU_COUNT */
#include "app.h"

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

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
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);

/*!
 * @brief app initialization.
 */
extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);
extern void USB_HostIsrDisable(void);
extern void USB_HostTaskFn(void *param);
/*******************************************************************************
 * Variables
 ******************************************************************************/

/*! @brief USB host msd fatfs instance global variable */
usb_host_handle g_HostHandle;
extern usb_host_msd_fatfs_instance_t g_MsdFatfsInstance; /* global msd fatfs instance */
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief USB isr function.
 */

#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
extern usb_status_t USB_HostTestEvent(usb_device_handle deviceHandle,
                                      usb_host_configuration_handle configurationHandle,
                                      uint32_t eventCode);
#endif

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
    usb_host_configuration_t *configuration;
    usb_status_t status1;
    usb_status_t status2;
    uint8_t interfaceIndex = 0;
#endif
    usb_status_t status = kStatus_USB_Success;
    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 == kStatus_USB_NotSupported) && (status2 == kStatus_USB_NotSupported))
            {
                status = kStatus_USB_NotSupported;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventNotSupported:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            configuration = (usb_host_configuration_t *)configurationHandle;
            for (interfaceIndex = 0; interfaceIndex < configuration->interfaceCount; ++interfaceIndex)
            {
                if (((usb_descriptor_interface_t *)configuration->interfaceList[interfaceIndex].interfaceDesc)
                        ->bInterfaceClass == 9U) /* 9U is hub class code */
                {
                    break;
                }
            }

            if (interfaceIndex < configuration->interfaceCount)
            {
                usb_echo("unsupported hub\r\n");
            }
            else
            {
                usb_echo("Unsupported Device\r\n");
            }
#else
            usb_echo("Unsupported Device\r\n");
#endif
            break;

        case kUSB_HostEventEnumerationDone:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventDetach:
#if ((defined USB_HOST_CONFIG_COMPLIANCE_TEST) && (USB_HOST_CONFIG_COMPLIANCE_TEST))
            status1 = USB_HostTestEvent(deviceHandle, configurationHandle, eventCode);
            status2 = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            if ((status1 != kStatus_USB_Success) && (status2 != kStatus_USB_Success))
            {
                status = kStatus_USB_Error;
            }
#else
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
#endif
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}

/*!
 * @brief USB host ehci isr function.
 */
void USB_HostEhciIsr(void)
{
#if (USB_HOST_CONFIG_EHCI)
    USB_HostEhciIsrFunction(g_HostHandle);
#endif
}

/*!
 * @brief USB host khci isr function.
 */
void USB_HostKhciIsr(void)
{
#if (USB_HOST_CONFIG_KHCI)
    USB_HostKhciIsrFunction(g_hostHandle);
#endif
}

void Host_AppInit(void)
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

/*!
 * @brief host app deinitialization.
 */
void Host_AppDeinit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostIsrDisable();
    status = USB_HostDeinit(g_HostHandle);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host deinit error\r\n");
        return;
    }

    usb_echo("host deinit done\r\n");
}

/*!
 * @brief host app task function.
 */
void Host_AppTaskFunction(void)
{
    USB_HostTaskFn(g_HostHandle);
    USB_HostMsdTask(&g_MsdFatfsInstance);
}
