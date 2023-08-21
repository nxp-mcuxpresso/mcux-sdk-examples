/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "usb_device_config.h"
#include "usb.h"
#include "usb_device.h"

#if ((defined(USB_DEVICE_CONFIG_MTP)) && (USB_DEVICE_CONFIG_MTP > 0U))
#include "usb_device_mtp.h"

#endif /* USB_DEVICE_CONFIG_MTP */
