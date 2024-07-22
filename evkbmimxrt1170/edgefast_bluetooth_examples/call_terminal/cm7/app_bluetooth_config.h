/*
 *  Copyright 2020-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (2U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

#define FLASH_ADAPTER_SIZE 0x10000

#define GENERIC_LIST_LIGHT 0

#define PRINTF_FLOAT_ENABLE 1
#define PRINTF_ADVANCED_ENABLE 1

/* Controller config
 * Supported controller list,
 * WIFI_IW612_BOARD_MURATA_2EL_M2
 *
 * If Murata Type 2EL module(Rev-A1 M2 only) used , define macro WIFI_IW612_BOARD_MURATA_2EL_M2 in following.
 */

/* @TEST_ANCHOR */
#define WIFI_IW612_BOARD_MURATA_2EL_M2
/* @END_TEST_ANCHOR */

#if (defined(WIFI_IW612_BOARD_MURATA_2EL_M2))
#include "wifi_bt_module_config.h"
#include "wifi_config.h"
#else
#error The transceiver module is unsupported
#endif

#if defined(WIFI_IW612_BOARD_MURATA_2EL_M2)
#undef SD_TIMING_MAX
#define SD_TIMING_MAX kSD_TimingDDR50Mode
#endif /*#define WIFI_IW612_BOARD_MURATA_2EL_M2*/

#define CONFIG_BT_PERIPHERAL            1
#define CONFIG_BT_DEVICE_NAME           "call terminal"
#define CONFIG_BT_DEVICE_NAME_DYNAMIC   1
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#define CONFIG_BT_RX_STACK_SIZE         2200

#define CONFIG_BT_SMP_SC_PAIR_ONLY 1

#define CONFIG_BT_PERIPHERAL 1
#define CONFIG_BT_EXT_ADV 1
#define CONFIG_BT_PER_ADV_SYNC 0

#define CONFIG_BT_ISO_UNICAST       1
#define CONFIG_BT_ISO_PERIPHERAL    1
#define CONFIG_BT_ISO_SYNC_RECEIVER 0
#define CONFIG_BT_ISO_MAX_CHAN      2

#define CONFIG_BT_GATT_DYNAMIC_DB        1
#define CONFIG_BT_GATT_CACHING           1
#define CONFIG_BT_GATT_AUTO_DISCOVER_CCC 1
#define CONFIG_BT_PACS                   1
#define CONFIG_BT_ASCS                   1

/* CIS */
#define CONFIG_BT_AUDIO                1
#define CONFIG_BT_AUDIO_RX             1
#define CONFIG_BT_AUDIO_TX             1
#define CONFIG_BT_BAP_UNICAST          1
#define CONFIG_BT_BAP_UNICAST_SERVER   1
#define CONFIG_BT_PAC_SNK 1
#define CONFIG_BT_ASCS_ASE_SNK_COUNT   1
#define CONFIG_BT_PAC_SRC 1
#define CONFIG_BT_ASCS_ASE_SRC_COUNT   1

/* BIS */
#define CONFIG_BT_AUDIO_BROADCAST_SINK             0
#define CONFIG_BT_AUDIO_BROADCAST_SNK_STREAM_COUNT 2
#define CONFIG_BT_AUDIO_CAPABILITY                 1

#define CONFIG_BT_PAC_SNK 1

/* VCP */
#define CONFIG_BT_VCP_VOL_REND 1

/* MCC */
#define CONFIG_BT_MCC 1

/* TBS */
#define CONFIG_BT_TBS_CLIENT_GTBS 1
#define CONFIG_BT_TBS_CLIENT_TBS  1

#if CONFIG_BT_TBS_CLIENT_GTBS || CONFIG_BT_TBS_CLIENT_TBS
#define CONFIG_BT_TBS_CLIENT_ACCEPT_CALL 1
#define CONFIG_BT_TBS_CLIENT_TERMINATE_CALL 1
#define CONFIG_BT_TBS_CLIENT_HOLD_CALL 1
#define CONFIG_BT_TBS_CLIENT_RETRIEVE_CALL 1
#define CONFIG_BT_TBS_CLIENT_ORIGINATE_CALL 1
#define CONFIG_BT_TBS_CLIENT_JOIN_CALLS 1
#define CONFIG_BT_TBS_CLIENT_MAX_CALLS 3
#endif

/* Mandatory to support at least 1 for ASCS */
#define CONFIG_BT_ATT_PREPARE_COUNT    1

#define CONFIG_BT_HOST_USB_ENABLE     1
#define CONFIG_BT_HOST_USB_IRQ_ENABLE 1

/* The noise issue will occur if the BT SNOOP feature is enabled */
#define CONFIG_BT_SNOOP 0

/* Debug Settings */
#if 0
#define CONFIG_BT_DEBUG 1
#define CONFIG_BT_DEBUG_TBS_CLIENT 1
#define CONFIG_BT_DEBUG_ATT 1
#define CONFIG_BT_DEBUG_CONN 1
#define CONFIG_BT_DEBUG_GATT 1
#define CONFIG_BT_DEBUG_HCI_CORE 1
#define CONFIG_BT_DEBUG_KEYS 1
#define CONFIG_BT_DEBUG_RPA 1
#define CONFIG_NET_BUF_LOG 1
#define CONFIG_BT_DEBUG_SMP 1
#define CONFIG_BT_DEBUG_L2CAP 1
#define CONFIG_BT_DEBUG_FIFO 1

#define LOG_MAX_BUFF_LOG_COUNT 128

#define BOARD_DEBUG_UART_BAUDRATE 1000000

#endif

#define CONFIG_LITTLE_ENDIAN 1

#define CONFIG_BT_ATT_TX_COUNT 128
#define CONFIG_BT_L2CAP_TX_BUF_COUNT 128

#define CONFIG_BT_BUF_ACL_RX_COUNT 32

#define CONFIG_BT_BUF_EVT_RX_COUNT 32

#define CONFIG_WORK_QUEUE_MSG_QUEUE_COUNT 128

#define LOG_MAX_BUFF_LOG_COUNT 128

#define CONFIG_BT_MSG_QUEUE_COUNT 64

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_audio_config.h"
