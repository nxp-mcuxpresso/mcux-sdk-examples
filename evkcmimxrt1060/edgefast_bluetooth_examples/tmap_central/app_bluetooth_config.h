/*
 *  Copyright 2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */
#define USB_HOST_CONFIG_EHCI (2U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

/* Controller config
 * Supported controller list,
 * WIFI_IW612_BOARD_MURATA_2EL_M2
 *
 * If Murata Type 2EL module(Rev-A1 M2 only) used , define macro WIFI_IW612_BOARD_MURATA_2EL_M2 in following.
 */

/* @TEST_ANCHOR */
#define WIFI_IW612_BOARD_MURATA_2EL_M2
/* @END_TEST_ANCHOR */
/*#define WIFI_IW612_BOARD_MURATA_2EL_M2*/

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#include "wifi_bt_module_config.h"
#include "wifi_config.h"
#else
#error The Wi-Fi module is unsupported
#endif

#define CONFIG_BT_DEVICE_NAME           "TMAP Central"
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#define CONFIG_BT_RX_STACK_SIZE         2200
#define CONFIG_BT_L2CAP_TX_BUF_COUNT    20
#define CONFIG_BT_L2CAP_DYNAMIC_CHANNEL 1

#define CONFIG_BT_ATT_RX_MAX 20

#define CONFIG_BT 1
#define CONFIG_BT_CENTRAL 1
#define CONFIG_BT_AUDIO 1
#define CONFIG_BT_AUDIO_TX 1
#define CONFIG_BT_AUDIO_RX 1

// TMAP support
#define CONFIG_BT_TMAP 1

// CAP support
#define CONFIG_BT_CAP_INITIATOR 1

// CSIP support
#define CONFIG_BT_CSIP_SET_COORDINATOR 1

// BAP support
#define CONFIG_BT_BAP_UNICAST_CLIENT 1

// VCP support
#define CONFIG_BT_VCP_VOL_CTLR 1

// MCP support
#define CONFIG_BT_MPL 1
#define CONFIG_BT_MCS 1
#define CONFIG_BT_MCC 1
#define CONFIG_BT_MCC_OTS 1
#define CONFIG_BT_OTS_CLIENT 1
#define CONFIG_MCTL_LOCAL_PLAYER_REMOTE_CONTROL 1
#define CONFIG_UTF8 1
#define CONFIG_MCTL_LOCAL_PLAYER_CONTROL 1
#define CONFIG_MCTL 1

// CCP support
#define CONFIG_BT_TBS 1
#define CONFIG_BT_GTBS 1
#define CONFIG_BT_TBS_SUPPORTED_FEATURES 3
#define CONFIG_BT_CCID 1

// Support an ISO channel per ASE
#define CONFIG_BT_ISO_TX_BUF_COUNT 2
#define CONFIG_BT_ISO_MAX_CHAN 2
#define CONFIG_BT_ISO_CENTRAL 1
#define CONFIG_BT_ISO_UNICAST 1
#define CONFIG_BT_BAP_UNICAST 1
#define CONFIG_BT_BAP_UNICAST_CLIENT_GROUP_STREAM_COUNT 1
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT 2
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT 2
#define CONFIG_BT_GATT_DYNAMIC_DB 1
#define CONFIG_BT_GATT_AUTO_DISCOVER_CCC 1
#define CONFIG_BT_GATT_AUTO_UPDATE_MTU   1

#define CONFIG_BT_EXT_ADV 1

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_audio_config.h"
#include "edgefast_bluetooth_debug_config.h"