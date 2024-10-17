/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (1U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

/* Select witch beacon application to start */
#define BEACON_APP  1
#define IBEACON_APP 0
#define EDDYSTONE   0

#define CONFIG_BT_PERIPHERAL 1
#define CONFIG_BT_PHY_UPDATE            1
#define CONFIG_BT_AUTO_PHY_UPDATE       1
#define CONFIG_BT_DATA_LEN_UPDATE       1
#define CONFIG_BT_AUTO_DATA_LEN_UPDATE  1
#if (defined EDDYSTONE) && (EDDYSTONE)
#define CONFIG_BT_SETTINGS              1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#endif
#if defined(IBEACON_APP) && (IBEACON_APP == 1)
#define CONFIG_BT_DEVICE_NAME "ibeacon"
#elif defined(EDDYSTONE) && (EDDYSTONE == 1)
#define CONFIG_BT_DEVICE_NAME "eddystone"
#elif defined(BEACON_APP) && (BEACON_APP == 1)
#define CONFIG_BT_DEVICE_NAME "beacon"
#endif
#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_debug_config.h"
#include "monolithic_config.h"