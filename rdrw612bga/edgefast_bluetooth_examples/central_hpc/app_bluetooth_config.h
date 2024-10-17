/*
 *  Copyright 2020-2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (1U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define CONFIG_BT_DEVICE_NAME           "central_hpc"
#define CONFIG_BT_PHY_UPDATE            1
#define CONFIG_BT_AUTO_PHY_UPDATE       1
#define CONFIG_BT_DATA_LEN_UPDATE       1
#define CONFIG_BT_AUTO_DATA_LEN_UPDATE  1
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              1
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1

#define CONFIG_BT_RX_STACK_SIZE 2200

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_debug_config.h"
#include "monolithic_config.h"