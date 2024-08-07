/*
 *  Copyright 2022-2023 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* Wi-Fi boards configuration */
#if defined(BT_THIRD_PARTY_TRANSCEIVER)
/*
 * Wifi functions are not used with third party but wifi files require to
 * be built, so stub macro are defined. Wifi functions won't be used at
 * link stage for third party transceiver
 *
 */
#define SD8987
#elif (defined(WIFI_IW416_BOARD_MURATA_1XK_M2))
#define SD8978
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SD_TIMING_MAX kSD_TimingDDR50Mode
#define WIFI_BT_USE_M2_INTERFACE
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1XK_CA.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1XK_EU.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1XK_JP.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1XK_US.h" */
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1XK_WW.h"
#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0x0, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0x6 \
    }
#elif (defined(WIFI_88W8987_BOARD_MURATA_1ZM_M2))
#define SD8987
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SD_TIMING_MAX kSD_TimingDDR50Mode
#define WIFI_BT_USE_M2_INTERFACE
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1ZM_CA.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1ZM_EU.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1ZM_JP.h" */
/* #define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1ZM_US.h" */
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_murata_1ZM_WW.h"
#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0x6, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0x6 \
    }
#elif defined(WIFI_IW61x_BOARD_RD_USD)
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_WW.h"
#define IW61x
#define WIFI_BT_USE_USD_INTERFACE
#define SD_CLOCK_MAX (25000000U)
#define SDMMCHOST_OPERATION_VOLTAGE_3V3
#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0x9, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0xC \
    }
#elif (defined(WIFI_IW416_BOARD_AW_AM510_USD))
#define SD8978
/*#define SDMMCHOST_OPERATION_VOLTAGE_3V3*/
#define SDMMCHOST_OPERATION_VOLTAGE_1V8
#define SD_CLOCK_MAX                (25000000U)
#define CONFIG_BR_SCO_PCM_DIRECTION 1
#define WIFI_BT_TX_PWR_LIMITS       "wlan_txpwrlimit_cfg_WW.h"
#define WLAN_ED_MAC_CTRL                                                               \
    {                                                                                  \
        .ed_ctrl_2g = 0x1, .ed_offset_2g = 0x9, .ed_ctrl_5g = 0x1, .ed_offset_5g = 0xC \
    }
#else
#error "Please define macro related to wifi board"
#endif
