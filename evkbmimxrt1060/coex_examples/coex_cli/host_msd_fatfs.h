/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016, 2018, 2020 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HOST_MSD_FATFS_H_
#define _HOST_MSD_FATFS_H_

#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_host_msd.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/

int USB_HostMsdFatfsInit(void);

/*!
 * @brief host msd callback function.
 *
 * This function should be called in the host callback function.
 *
 * @param deviceHandle           device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The configuration don't contain msd interface.
 * @retval kStatus_USB_Error                There is no idle msd instance.
 */
usb_status_t USB_HostMsdEvent(usb_device_handle deviceHandle,
                                     usb_host_configuration_handle configurationHandle,
                                     uint32_t eventCode);


/*!
 * @brief host msd fatfs task function.
 *
 * This function implements the host msd fatfs action, it is used to create task.
 *
 * @param arg   the host msd fatfs instance pointer.
 */
void USB_HostMsdTask(void *arg);

#endif /* _HOST_MSD_FATFS_H_ */
