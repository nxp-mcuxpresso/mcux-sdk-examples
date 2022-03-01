/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __USB_DEVICE_PRINTER_H__
#define __USB_DEVICE_PRINTER_H__

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief The class code of the printer class */
#define USB_DEVICE_CONFIG_PRINTER_CLASS_CODE (0x07)

/*! @brief class-specific request GET_DEVICE_ID */
#define USB_DEVICE_PRINTER_GET_DEVICE_ID (0x00U)
/*! @brief class-specific request GET_PORT_STATUS */
#define USB_DEVICE_PRINTER_GET_PORT_STATUS (0x01U)
/*! @brief class-specific request SOFT_RESET */
#define USB_DEVICE_PRINTER_SOFT_RESET (0x02U)

/*! @brief Paper empty bit mask for GET_PORT_STATUS */
#define USB_DEVICE_PRINTER_PORT_STATUS_PAPER_EMPTRY_MASK (0x20U)
/*! @brief Select bit mask for GET_PORT_STATUS */
#define USB_DEVICE_PRINTER_PORT_STATUS_SELECT_MASK (0x10U)
/*! @brief Error bit mask for GET_PORT_STATUS */
#define USB_DEVICE_PRINTER_PORT_STATUS_NOT_ERROR_MASK (0x08U)

#define USB_DEVICE_PRINTER_PORT_STATUS_DEFAULT_VALUE \
    (USB_DEVICE_PRINTER_PORT_STATUS_SELECT_MASK | USB_DEVICE_PRINTER_PORT_STATUS_NOT_ERROR_MASK)

#endif /* __USB_DEVICE_PRINTER_H__ */
