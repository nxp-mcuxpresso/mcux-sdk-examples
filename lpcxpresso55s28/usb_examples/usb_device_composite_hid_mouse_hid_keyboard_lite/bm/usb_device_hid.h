/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_HID_H__
#define __USB_DEVICE_HID_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief The class code of the HID class */
#define USB_DEVICE_CONFIG_HID_CLASS_CODE (0x03U)

/*! @brief Request code to get report of HID class. */
#define USB_DEVICE_HID_REQUEST_GET_REPORT (0x01U)
#define USB_DEVICE_HID_REQUEST_GET_REPORT_TYPE_INPUT (0x01U)
#define USB_DEVICE_HID_REQUEST_GET_REPORT_TYPE_OUPUT (0x02U)
#define USB_DEVICE_HID_REQUEST_GET_REPORT_TYPE_FEATURE (0x03U)
/*! @brief Request code to get idle of HID class. */
#define USB_DEVICE_HID_REQUEST_GET_IDLE (0x02U)
/*! @brief Request code to get protocol of HID class. */
#define USB_DEVICE_HID_REQUEST_GET_PROTOCOL (0x03U)
/*! @brief Request code to set report of HID class. */
#define USB_DEVICE_HID_REQUEST_SET_REPORT (0x09U)
/*! @brief Request code to set idle of HID class. */
#define USB_DEVICE_HID_REQUEST_SET_IDLE (0x0AU)
/*! @brief Request code to set protocol of HID class. */
#define USB_DEVICE_HID_REQUEST_SET_PROTOCOL (0x0BU)

/*******************************************************************************
 * API
 ******************************************************************************/

#endif /* __USB_DEVICE_HID_H__ */
