/*
 *  Copyright 2023-2024 NXP
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

#define CONFIG_BT_PERIPHERAL            1
#define CONFIG_BT_DEVICE_NAME           "TMAP Peripheral"
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#define CONFIG_BT_RX_STACK_SIZE         2200
#define CONFIG_BT_ATT_TX_COUNT          20
#define CONFIG_BT_L2CAP_TX_BUF_COUNT    20
#define CONFIG_BT_PRIVACY               1

#define CONFIG_BT_ATT_RX_MAX 20

// Demo setting.
#define CONFIG_BT_CSIP_SET_MEMBER 0
#define CONFIG_BT_CAP_ACCEPTOR_SET_MEMBER 0

// Earbuds type
#define CONFIG_TMAP_PERIPHERAL_SINGLE   1 /* Single ear headset */
#if CONFIG_BT_CSIP_SET_MEMBER
    #define CONFIG_TMAP_PERIPHERAL_DUO  0 /* Duo headset */
#endif

#if defined(CONFIG_TMAP_PERIPHERAL_DUO) && (CONFIG_TMAP_PERIPHERAL_DUO > 0)
    #define CONFIG_TMAP_PERIPHERAL_SET_RANK 1
    //#define CONFIG_TMAP_PERIPHERAL_SET_RANK 2
#endif

// Select the Earbud location.
#define CONFIG_TMAP_PERIPHERAL_LEFT  0
#define CONFIG_TMAP_PERIPHERAL_RIGHT 1

#define CONFIG_BT_AUDIO 1
#define CONFIG_BT_AUDIO_RX 1
#define CONFIG_BT_AUDIO_TX 1
//#define CONFIG_UTF8 1

// TMAP support
#define CONFIG_BT_TMAP 1

// CAP
#define CONFIG_BT_CAP_ACCEPTOR 1

// BAP support
#define CONFIG_BT_BAP_UNICAST_SERVER 1

#if CONFIG_BT_BAP_UNICAST_SERVER
#define CONFIG_BT_BAP_UNICAST        1
#define CONFIG_BT_ISO_UNICAST        1
#define CONFIG_BT_ISO_PERIPHERAL     1
#define CONFIG_BT_GATT_CACHING       1
#define CONFIG_BT_ASCS               1
#endif

// Mandatory to support at least 1 for ASCS
#define CONFIG_BT_ATT_PREPARE_COUNT 1

// VCP support
#define CONFIG_BT_VCP_VOL_REND 1

// MCP support
#define CONFIG_BT_MCC 1

// Support an ISO channel per ASE
#define CONFIG_BT_PAC_SNK 1
#define CONFIG_BT_ASCS_ASE_SNK_COUNT 1
#define CONFIG_BT_PAC_SRC 1
#define CONFIG_BT_ASCS_ASE_SRC_COUNT 1
// Support an ISO channel per ASE
#define CONFIG_BT_ISO_MAX_CHAN 2

// Sink Contexts Supported: Unspecified, Conversational, Media
#define CONFIG_BT_PACS_SNK_CONTEXT 0x0007
// Source Contexts Supported: Unspecified, Conversational, Media
#define CONFIG_BT_PACS_SRC_CONTEXT 0x0007

// Sink PAC Location Support
#define CONFIG_BT_PAC_SNK_LOC 1
// Source PAC Location Support
#define CONFIG_BT_PAC_SRC_LOC 1

// CCP Client Support
#define CONFIG_BT_TBS_CLIENT 1
#define CONFIG_BT_TBS_CLIENT_GTBS 1
#define CONFIG_BT_TBS_CLIENT_MAX_TBS_INSTANCES 0
#define CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL 1
#define CONFIG_BT_TBS_CLIENT_TERMINATE_CALL 1
#define CONFIG_BT_TBS_CLIENT_BEARER_URI_SCHEMES_SUPPORTED_LIST 1
#define CONFIG_BT_TBS_CLIENT_MAX_CALLS 2

// Generic config
#define CONFIG_BT_GATT_DYNAMIC_DB 1
#define CONFIG_BT_GATT_CLIENT 1
#define CONFIG_BT_GATT_AUTO_DISCOVER_CCC 1
#define CONFIG_BT_EXT_ADV 1

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_audio_config.h"
#include "edgefast_bluetooth_debug_config.h"