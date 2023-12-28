/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HOST_KEYBOARD_H_
#define _HOST_KEYBOARD_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#if ((defined USB_HOST_CONFIG_KHCI) && (USB_HOST_CONFIG_KHCI))
#ifndef HOST_CONTROLLER_ID
#define HOST_CONTROLLER_ID kUSB_ControllerKhci0
#endif
#endif /* USB_HOST_CONFIG_KHCI */
#if ((defined USB_HOST_CONFIG_EHCI) && (USB_HOST_CONFIG_EHCI))
#ifndef HOST_CONTROLLER_ID
#define HOST_CONTROLLER_ID kUSB_ControllerEhci1
#endif
#endif /* USB_HOST_CONFIG_EHCI */
#if ((defined USB_HOST_CONFIG_OHCI) && (USB_HOST_CONFIG_OHCI))
#ifndef HOST_CONTROLLER_ID
#define HOST_CONTROLLER_ID kUSB_ControllerOhci0
#endif
#endif /* USB_HOST_CONFIG_OHCI */
#if ((defined USB_HOST_CONFIG_IP3516HS) && (USB_HOST_CONFIG_IP3516HS))
#ifndef HOST_CONTROLLER_ID
#define HOST_CONTROLLER_ID kUSB_ControllerIp3516Hs0
#endif
#endif /* USB_HOST_CONFIG_IP3516HS */
#define USB_HOST_INTERRUPT_PRIORITY (3U)

/*! @brief buffer for receiving report descriptor and data */
#define HID_BUFFER_SIZE (100U)

/*! @brief host app device attach/detach status */
typedef enum _usb_host_app_state
{
    kStatus_DEV_Idle = 0, /*!< there is no device attach/detach */
    kStatus_DEV_Attached, /*!< device is attached */
    kStatus_DEV_Detached, /*!< device is detached */
} usb_host_app_state_t;

/*! @brief host app run status */
typedef enum _usb_host_hid_run_state
{
    kUSB_HostHidRunIdle = 0,                /*!< idle */
    kUSB_HostHidRunSetInterface,            /*!< execute set interface code */
    kUSB_HostHidRunWaitSetInterface,        /*!< wait set interface done */
    kUSB_HostHidRunSetInterfaceDone,        /*!< set interface is done, execute next step */
    kUSB_HostHidRunWaitSetIdle,             /*!< wait set idle done */
    kUSB_HostHidRunSetIdleDone,             /*!< set idle is done, execute next step */
    kUSB_HostHidRunWaitGetReportDescriptor, /*!< wait get report descriptor done */
    kUSB_HostHidRunGetReportDescriptorDone, /*!< get report descriptor is done, execute next step */
    kUSB_HostHidRunWaitSetProtocol,         /*!< wait set protocol done */
    kUSB_HostHidRunSetProtocolDone,         /*!< set protocol is done, execute next step */
    kUSB_HostHidRunWaitDataReceived,        /*!< wait interrupt in data */
    kUSB_HostHidRunDataReceived,            /*!< interrupt in data received */
    kUSB_HostHidRunPrimeDataReceive,        /*!< prime interrupt in receive */
} usb_host_hid_run_state_t;

/*! @brief USB host keyboard instance structure */
typedef struct _usb_host_keyboard_instance
{
    usb_host_configuration_handle configHandle; /*!< the keybaord's configuration handle */
    usb_device_handle deviceHandle;             /*!< the keybaord's device handle */
    usb_host_class_handle classHandle;          /*!< the keybaord's class handle */
    usb_host_interface_handle interfaceHandle;  /*!< the keybaord's interface handle */
    uint16_t maxPacketSize;                     /*!< interrupt in max packet size */
    uint8_t deviceState;                        /*!< device attach/detach status */
    uint8_t prevState;                          /*!< device attach/detach previous status */
    uint8_t runState;                           /*!< keyboard application run status */
    uint8_t runWaitState; /*!< keyboard application wait status, go to next run status when the wait status success */
    uint8_t *keyboardBuffer; /*!< use to receive report descriptor and data */
} usb_host_keyboard_instance_t;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief host keyboard task function.
 *
 * This function implements the host keyboard action, it is used to create task.
 *
 * @param param   the host keyboard instance pointer.
 */
extern void USB_HostHidKeyboardTask(void *param);

/*!
 * @brief host keyboard callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle         device handle.
 * @param configurationHandle  attached device's configuration descriptor information.
 * @param eventCode            callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid keyboard interface.
 */
extern usb_status_t USB_HostHidKeyboardEvent(usb_device_handle deviceHandle,
                                             usb_host_configuration_handle configurationHandle,
                                             uint32_t eventCode);

#endif /* _HOST_KEYBOARD_H_ */
