/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#if ((defined(USB_DEVICE_CONFIG_PRINTER)) && (USB_DEVICE_CONFIG_PRINTER > 0U))
#include "usb_device_printer.h"

#endif
