/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (2U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define FLASH_ADAPTER_SIZE 0x10000

/* Controller config
 * Supported controller list,
 * BT_THIRD_PARTY_TRANSCEIVER
 * WIFI_IW612_BOARD_RD_USD
 * WIFI_IW612_BOARD_MURATA_2EL_M2
 *
 * If third party controller used , define macro BT_THIRD_PARTY_TRANSCEIVER in following.
 * If IW612 is used, define the macro WIFI_IW612_BOARD_RD_USD in following.
 * If Murata Type 2EL module(M2 only) used , define macro WIFI_IW612_BOARD_MURATA_2EL_M2 in following.
 */

/* @TEST_ANCHOR */
#define WIFI_IW612_BOARD_MURATA_2EL_M2
/* @END_TEST_ANCHOR */

#if defined(BT_THIRD_PARTY_TRANSCEIVER) || defined(WIFI_IW612_BOARD_RD_USD) || defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#include "wifi_bt_module_config.h"
#include "wifi_config.h"
#else
#error The transceiver module is unsupported
#endif

#define CONFIG_BT_A2DP                  0
#define CONFIG_BT_A2DP_SINK             0
#if CONFIG_BT_A2DP_SINK
#define CONFIG_BT_DEVICE_NAME           "a2dp_bridge"
#else
#define CONFIG_BT_DEVICE_NAME           "unicast_media_sender"
#endif
#define CONFIG_BT_DEVICE_NAME_DYNAMIC   1
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 0
#define CONFIG_BT_RX_STACK_SIZE         2200

#if CONFIG_BT_A2DP_SINK
#define CONFIG_BT_MAX_CONN              3
#define CONFIG_BT_MAX_PAIRED            3
#define CONFIG_BT_ID_MAX                3
#else
#define CONFIG_BT_MAX_CONN              2
#define CONFIG_BT_MAX_PAIRED            2
#define CONFIG_BT_ID_MAX                2
#endif
#define CONFIG_BT_L2CAP_TX_BUF_COUNT    12

#define CONFIG_BT_CENTRAL     1
#if CONFIG_BT_A2DP_SINK
#define CONFIG_BT_PERIPHERAL  1
#endif
#define CONFIG_BT_EXT_ADV     1
#define CONFIG_BT_PER_ADV     1

#define CONFIG_BT_ISO_CENTRAL      1
#define CONFIG_BT_ISO_UNICAST      1
#define CONFIG_BT_ISO_TX_BUF_COUNT (16 * CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT)
#define CONFIG_BT_ISO_MAX_CHAN     2

#define CONFIG_BT_GATT_AUTO_DISCOVER_CCC 1
#define CONFIG_BT_GATT_AUTO_UPDATE_MTU   1
#define CONFIG_BT_GATT_DYNAMIC_DB        1

#define CONFIG_BT_AUDIO                1
#define CONFIG_BT_AUDIO_TX             1

/* CIS */
#define CONFIG_BT_BAP_UNICAST        1
#define CONFIG_BT_BAP_UNICAST_CLIENT 1

#define CONFIG_BT_BAP_UNICAST_CLIENT_GROUP_STREAM_COUNT 2
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT      2
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT      0

/* VCP */
#define CONFIG_BT_VCP_VOL_CTLR 1

/* MCS */
#define CONFIG_BT_MCS 1
#define CONFIG_UTF8   1
#define CONFIG_BT_CCID 1
#define CONFIG_MCTL 1
#define CONFIG_MCTL_LOCAL_PLAYER_CONTROL 1
#define CONFIG_MCTL_LOCAL_PLAYER_LOCAL_CONTROL 1
#define CONFIG_MCTL_LOCAL_PLAYER_REMOTE_CONTROL 1
#define CONFIG_MCTL_REMOTE_PLAYER_CONTROL 0
#define CONFIG_MCTL_REMOTE_PLAYER_CONTROL_OBJECTS 0

/* MPL */
#define CONFIG_BT_MPL 1

/* CSIP */
#define CONFIG_BT_CSIP_SET_COORDINATOR 1
#define CONFIG_BT_ATT_TX_COUNT 16
#define CONFIG_LITTLE_ENDIAN 1

#define LE_CONN_COUNT 2

#define CONFIG_BT_HOST_USB_ENABLE     1
#define CONFIG_BT_HOST_USB_IRQ_ENABLE 1

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_audio_config.h"
#include "edgefast_bluetooth_debug_config.h"