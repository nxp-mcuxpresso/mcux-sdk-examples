/*
 *  Copyright 2020-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
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

#define CONFIG_BT_DEVICE_NAME           "broadcast_media_sender"
#define CONFIG_BT_DEVICE_NAME_DYNAMIC   1
#define CONFIG_BT_SMP                   1
#define CONFIG_BT_SETTINGS              0
#define CONFIG_BT_HOST_CRYPTO           1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 0
#define CONFIG_BT_RX_STACK_SIZE         2200

#define CONFIG_BT_BROADCASTER 1
#define CONFIG_BT_EXT_ADV     1
#define CONFIG_BT_PER_ADV     1

#define CONFIG_BT_ISO_BROADCASTER  1
#define CONFIG_BT_ISO_TX_BUF_COUNT (16 * CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT)
#define CONFIG_BT_ISO_MAX_CHAN     2

#define CONFIG_BT_AUDIO                  1
#define CONFIG_BT_AUDIO_TX               1

/* BIS */
#define CONFIG_BT_BAP_BROADCAST_SOURCE               1
#define CONFIG_BT_BAP_BROADCAST_SRC_SUBGROUP_COUNT   1
#define CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT     2

#define CONFIG_BT_HOST_USB_ENABLE     1
#define CONFIG_BT_HOST_USB_IRQ_ENABLE 1

#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_audio_config.h"
#include "edgefast_bluetooth_debug_config.h"