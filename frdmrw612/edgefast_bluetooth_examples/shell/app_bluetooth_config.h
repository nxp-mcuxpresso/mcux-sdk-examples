/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (1U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define CONFIG_BT_PERIPHERAL           1
#define CONFIG_BT_CENTRAL              1
#define CONFIG_BT_L2CAP_IFRAME_SUPPORT 1

#define CONFIG_BT_DEVICE_NAME_DYNAMIC   1
#define CONFIG_BT_PHY_UPDATE            1
#define CONFIG_BT_AUTO_PHY_UPDATE       1
#define CONFIG_BT_DATA_LEN_UPDATE       1
#define CONFIG_BT_AUTO_DATA_LEN_UPDATE  1
#define CONFIG_BT_ID_MAX                4
#define CONFIG_BT_PRIVACY               1
#define CONFIG_BT_SETTINGS              1
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_MAX_PAIRED            16
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#define CONFIG_BT_SIGNING               1
#define CONFIG_BT_GATT_SERVICE_CHANGED  1
#define CONFIG_BT_GATT_CACHING          1
#define CONFIG_BT_GATT_DYNAMIC_DB       1
#define CONFIG_BT_L2CAP_TX_BUF_COUNT    8
#define CONFIG_BT_L2CAP_DYNAMIC_CHANNEL 1
#define CONFIG_BT_DATA_LEN_UPDATE       1
#define CONFIG_BT_USER_DATA_LEN_UPDATE  1
#define CONFIG_BT_WHITELIST             1
#define CONFIG_BT_PHY_UPDATE            1
#define CONFIG_BT_USER_PHY_UPDATE       1
#if 0
#define CONFIG_BT_EXT_ADV 1
#define CONFIG_BT_PER_ADV 1
#endif

#define CONFIG_BT_SNOOP        1
#define CONFIG_BT_RF_TEST_MODE 1
#if 0
#define CONFIG_BT_SMP_SELFTEST 1

#define CONFIG_BT_DEBUG 1

#define CONFIG_NET_BUF_LOG        1
#define CONFIG_NET_BUF_POOL_USAGE 1
#endif

#undef CONFIG_BT_BREDR
#define CONFIG_WORK_QUEUE_TASK_STACK_SIZE 4096

#define SHELL_BUFFER_SIZE 512
#define SHELL_MAX_ARGS    20
#include "edgefast_bluetooth_config.h"

#include "edgefast_bluetooth_debug_config.h"

#include "monolithic_config.h"

#define DEBUG_CONSOLE_TX_RELIABLE_ENABLE (0U)

