/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _WEIGHSCALE_H_
#define _WEIGHSCALE_H_

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

/*! @brief USB interrupt priority */
#define USB_DEVICE_INTERRUPT_PRIORITY (3U) /*! @brief Association request length */
#define ASSOCIATION_REQUEST_LENGTH (54U)
/*! @brief Configuration event report length */
#define EVENT_REPORT_CONFIGURATION_LENGTH (166U)
/*! @brief DIM get response length */
#define EVENT_RESPONSE_GET_LENGTH (114U)
/*! @brief DIM data transfer length */
#define EVENT_REPORT_DATA_LENGTH (94U)
/*! @brief Weight scale application event */
#define APP_EVENT_SEND_ASSOCIATION_REQUEST (0x00U)
#define APP_EVENT_SEND_DEVICE_CONFIGURATION (0x01U)
#define APP_EVENT_SEND_MDS_OBJECT (0x02U)
#define APP_EVENT_SEND_MEASUREMENT_DATA (0x03U)
#define APP_EVENT_UNDEFINED (uint8_t) - 1

/* structure for the measurements that are changing */
typedef struct _weightscale_measurement_struct
{
    uint16_t weight[2U];        /*!< body weight */
    uint16_t bodyMassIndex[2U]; /*!< body mass index */
} weightscale_measurement_struct_t;
#endif /* _WEIGHSCALE_H_ */
