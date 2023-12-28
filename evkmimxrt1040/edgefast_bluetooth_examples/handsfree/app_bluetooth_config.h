/*
 *  Copyright 2020-2021 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#define USB_HOST_CONFIG_EHCI (2U)
#define CONTROLLER_ID        kUSB_ControllerEhci0

/* Controller config
 * Supported controller list,
 * WIFI_IW416_BOARD_MURATA_1XK_M2
 * WIFI_88W8987_BOARD_MURATA_1ZM_M2
 *
 * If Murata Type 1XK module used, define macro WIFI_IW416_BOARD_MURATA_1XK_M2 in following.
 * If Murata Type 1ZM module used, define macro WIFI_88W8987_BOARD_MURATA_1ZM_M2 in following.
 */

/* @TEST_ANCHOR */
#define WIFI_IW416_BOARD_MURATA_1XK_M2
/* @END_TEST_ANCHOR */
/*#define WIFI_IW416_BOARD_MURATA_1XK_M2*/
/*#define WIFI_88W8987_BOARD_MURATA_1ZM_M2*/

#if defined(WIFI_IW416_BOARD_MURATA_1XK_M2) || defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2)
#include "wifi_bt_module_config.h"
#include "wifi_config.h"
#else
#error The Wi-Fi module is unsupported
#endif

#if defined(WIFI_88W8987_BOARD_AW_CM358_USD)
#if defined(SDMMCHOST_OPERATION_VOLTAGE_3V3)
#elif defined(SDMMCHOST_OPERATION_VOLTAGE_1V8)
#undef SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SDMMCHOST_OPERATION_VOLTAGE_3V3
#endif
#endif

#define CONFIG_BT_RFCOMM                1
#define CONFIG_BT_HFP_HF                1
#define CONFIG_BT_DEBUG                 0
#define CONFIG_BT_DEBUG_HCI_CORE        0
#define CONFIG_BT_DEBUG_CONN            0
#define CONFIG_BT_DEBUG_GATT            0
#define CONFIG_BT_DEBUG_ATT             0
#define CONFIG_BT_DEBUG_SMP             0
#define CONFIG_BT_DEBUG_KEYS            0
#define CONFIG_BT_DEBUG_L2CAP           0
#define CONFIG_BT_DEBUG_SERVICE         0
#define CONFIG_BT_DEBUG_HFP_HF          0
#define CONFIG_BT_SETTINGS              1
#define CONFIG_BT_KEYS_OVERWRITE_OLDEST 1


#if (defined(CONFIG_BT_SMP) && (CONFIG_BT_SMP > 0U))
    #define CONFIG_BT_RX_STACK_SIZE 2500
#else
    #define CONFIG_BT_RX_STACK_SIZE 1024
#endif

#define CONFIG_BT_BLE_DISABLE 1
#include "edgefast_bluetooth_config.h"
#include "edgefast_bluetooth_debug_config.h"