/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _USB_DEVICE_COMPOSITE_H_
#define _USB_DEVICE_COMPOSITE_H_ 1

#include "virtual_com.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511FS) && (USB_DEVICE_CONFIG_LPCIP3511FS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Fs0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_LPCIP3511HS) && (USB_DEVICE_CONFIG_LPCIP3511HS > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerLpcIp3511Hs0
#endif
#endif

#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;                           /* USB device handle. */
    usb_cdc_vcom_struct_t cdcVcom[USB_DEVICE_CONFIG_CDC_ACM]; /* CDC virtual com device structure. */
    uint8_t speed;  /* Speed of USB device. USB_SPEED_FULL/USB_SPEED_LOW/USB_SPEED_HIGH.                 */
    uint8_t attach; /* A flag to indicate whether a usb device is attached. 1: attached, 0: not attached */
    uint8_t currentConfiguration; /* Current configuration value. */
    uint8_t
        currentInterfaceAlternateSetting[USB_INTERFACE_COUNT]; /* Current alternate setting value for each interface. */
} usb_device_composite_struct_t;

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief CDC class specific callback function.
 *
 * This function handles the CDC class specific requests.
 *
 * @param handle          The CDC ACM class handle.
 * @param setup           The setup buffer address of the request.
 * @param length          The active data length.
 * @param buffer          The active data buffer need to be sent or received.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceCdcVcomClassRequest(usb_device_handle handle,
                                                  usb_setup_struct_t *setup,
                                                  uint32_t *length,
                                                  uint8_t **buffer);
/*!
 * @brief USB configure endpoint function.
 *
 * This function configure endpoint status.
 *
 * @param handle The USB device handle.
 * @param ep Endpoint address.
 * @param status A flag to indicate whether to stall the endpoint. 1: stall, 0: unstall.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceCdcVcomConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status);
/*!
 * @brief Virtual COM device set configuration function.
 *
 * This function sets configuration for CDC class.
 *
 * @param handle The CDC ACM class handle.
 * @param configure The CDC ACM class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceCdcVcomSetConfigure(usb_device_handle handle, uint8_t configure);
/*!
 * @brief Virtual COM device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceCdcVcomInit(usb_device_composite_struct_t *deviceComposite);
/*!
 * @brief Application task function.
 *
 * This function runs the task for application.
 *
 * @return None.
 */
extern void USB_DeviceCdcVcomTask(void);

/*!
 * @brief MSC class specific callback function.
 *
 * This function handles the MSCclass specific requests.
 *
 * @param handle          The MSC class handle.
 * @param setup           The setup buffer address of the request.
 * @param length          The active data length.
 * @param buffer          The active data buffer need to be sent or received.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMscDiskClassRequest(usb_device_handle handle,
                                                  usb_setup_struct_t *setup,
                                                  uint32_t *length,
                                                  uint8_t **buffer);
/*!
 * @brief MSC disk device set configuration function.
 *
 * This function sets configuration for MSC class.
 *
 * @param handle The MSC class handle.
 * @param ep Endpoint address.
 * @param status A flag to indicate whether to stall the endpoint. 1: stall, 0: unstall.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMscDiskConfigureEndpointStatus(usb_device_handle handle, uint8_t ep, uint8_t status);

/*!
 * @brief MSC disk device set configuration function.
 *
 * This function sets configuration for MSC class.
 *
 * @param handle The MSC class handle.
 * @param configure The MSC class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMscDiskSetConfigure(usb_device_handle handle, uint8_t configure);
/*!
 * @brief MSC disk device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param deviceComposite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceMscDiskInit(usb_device_composite_struct_t *deviceComposite);

#endif /* _USB_DEVICE_COMPOSITE_H_ */
