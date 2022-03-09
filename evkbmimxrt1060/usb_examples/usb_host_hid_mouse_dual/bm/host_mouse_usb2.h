/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HOST_MOUSE_USB2_H_
#define _HOST_MOUSE_USB2_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief buffer for receiving report descriptor and data */
#define HID_BUFFER_SIZE_USB2 200

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief host mouse task function.
 *
 * This function implements the host mouse action, it is used to create task.
 *
 * @param param   the host mouse instance pointer.
 */
extern void USB_HostHidMouseTask_2(void *param);

/*!
 * @brief host mouse callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain hid mouse interface.
 */
extern usb_status_t USB_HostHidMouseEvent_2(usb_device_handle deviceHandle,
                                            usb_host_configuration_handle configurationHandle,
                                            uint32_t eventCode);

#endif /* _HOST_MOUSE_H_ */
