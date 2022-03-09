/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_COMPOSITE_H__
#define __USB_DEVICE_COMPOSITE_H__ 1
#include "FreeRTOS.h"
#include "semphr.h"
#include "event_groups.h"

#include "audio_generator.h"
#include "mouse.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if defined(USB_DEVICE_CONFIG_EHCI) && (USB_DEVICE_CONFIG_EHCI > 0U)
#ifndef CONTROLLER_ID
#define CONTROLLER_ID kUSB_ControllerEhci0
#endif
#endif
#if defined(USB_DEVICE_CONFIG_KHCI) && (USB_DEVICE_CONFIG_KHCI > 0U)
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

#if defined(__GIC_PRIO_BITS)
#define USB_DEVICE_INTERRUPT_PRIORITY (25U)
#elif defined(__NVIC_PRIO_BITS) && (__NVIC_PRIO_BITS >= 3)
#define USB_DEVICE_INTERRUPT_PRIORITY (6U)
#else
#define USB_DEVICE_INTERRUPT_PRIORITY (3U)
#endif

typedef struct _usb_device_composite_struct
{
    usb_device_handle deviceHandle;
    usb_audio_generator_struct_t audioGenerator;
    usb_hid_mouse_struct_t hidMouse;
    TaskHandle_t applicationTaskHandle;
    TaskHandle_t deviceTaskHandle;
    uint8_t speed;
    uint8_t attach;
    uint8_t currentConfiguration;
    uint8_t currentInterfaceAlternateSetting[USB_DEVICE_INTERFACE_COUNT];
} usb_device_composite_struct_t;

extern void audio_generator_task(void);

/*******************************************************************************
 * API
 ******************************************************************************/
/*!
 * @brief Audio class specific callback function.
 *
 * This function handles the Audio class specific requests.
 *
 * @param handle		  The Audio class handle.
 * @param event 		  The Audio class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioGeneratorCallback(class_handle_t handle, uint32_t event, void *param);
/*!
 * @brief Audio device set configuration function.
 *
 * This function sets configuration for Audio class.
 *
 * @param handle The Audio class handle.
 * @param configure The Audio class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioGeneratorSetConfigure(class_handle_t handle, uint8_t configure);
/*!
 * @brief Audio device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceAudioGeneratorInit(usb_device_composite_struct_t *device_composite);
extern usb_status_t USB_DeviceAudioGeneratorSetInterface(class_handle_t handle,
                                                         uint8_t interface,
                                                         uint8_t alternateSetting);

/*!
 * @brief HID class specific callback function.
 *
 * This function handles the HID class specific requests.
 *
 * @param handle		  The HID class handle.
 * @param event 		  The HID class event type.
 * @param param 		  The parameter of the class specific request.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidMouseCallback(class_handle_t handle, uint32_t event, void *param);
/*!
 * @brief HID device set configuration function.
 *
 * This function sets configuration for HID class.
 *
 * @param handle The HID class handle.
 * @param configure The HID class configure index.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidMouseSetConfigure(class_handle_t handle, uint8_t configure);
extern usb_status_t USB_DeviceHidMouseSetInterface(class_handle_t handle, uint8_t interface, uint8_t alternateSetting);
/*!
 * @brief HID device initialization function.
 *
 * This function initializes the device with the composite device class information.
 *
 * @param device_composite The pointer to the composite device structure.
 *
 * @return A USB error code or kStatus_USB_Success.
 */
extern usb_status_t USB_DeviceHidMouseInit(usb_device_composite_struct_t *device_composite);

#endif
