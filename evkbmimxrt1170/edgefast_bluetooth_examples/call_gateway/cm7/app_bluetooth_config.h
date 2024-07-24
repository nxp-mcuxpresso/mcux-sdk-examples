/*
 *  Copyright 2020-2023 NXP
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

#define CONFIG_BT_DEVICE_NAME           "call gateway"
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1
#define CONFIG_BT_RX_STACK_SIZE         2200

#define CONFIG_BT_MAX_CONN              2
#define CONFIG_BT_MAX_PAIRED            2
#define CONFIG_BT_ID_MAX                2

#define CONFIG_BT_PERIPHERAL  1
#define CONFIG_BT_CENTRAL     1
#define CONFIG_BT_BROADCASTER 1
#define CONFIG_BT_EXT_ADV     1
#define CONFIG_BT_PER_ADV     0

#define CONFIG_BT_ISO_UNICAST      1
#define CONFIG_BT_ISO_CENTRAL      1
#define CONFIG_BT_ISO_BROADCASTER  0
#define CONFIG_BT_ISO_TX_BUF_COUNT (16 * CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT)
#define CONFIG_BT_ISO_MAX_CHAN     2

#define CONFIG_BT_GATT_AUTO_DISCOVER_CCC 1
#define CONFIG_BT_GATT_AUTO_UPDATE_MTU   1
#define CONFIG_BT_GATT_DYNAMIC_DB        1

/* CIS */
#define CONFIG_BT_AUDIO                1
#define CONFIG_BT_AUDIO_TX             1
#define CONFIG_BT_AUDIO_RX             1
#define CONFIG_BT_BAP_UNICAST          1
#define CONFIG_BT_BAP_UNICAST_CLIENT   1

#define CONFIG_BT_BAP_UNICAST_CLIENT_GROUP_STREAM_COUNT 2
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT      2
#define CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT      2

/* BIS */
#define CONFIG_BT_AUDIO_BROADCAST_SOURCE             0
#define CONFIG_BT_AUDIO_BROADCAST_SRC_SUBGROUP_COUNT 1
#define CONFIG_BT_AUDIO_BROADCAST_SRC_STREAM_COUNT   2

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

/* TBS */
#define CONFIG_BT_TBS 1

/* CSIP set coordinator */
#define CONFIG_BT_CSIP_SET_COORDINATOR 1

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
