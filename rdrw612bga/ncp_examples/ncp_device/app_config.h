/*
 *  Copyright 2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* @TEST_ANCHOR */
#define WIFI_BOARD_RW610
/* @END_TEST_ANCHOR */

#if defined(WIFI_BOARD_RW610)
#define RW610
#define WIFI_BT_USE_IMU_INTERFACE
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_WW_rw610.h"
#else
#error "Please define macro for RW610 board"
#endif


/*NCP config*/
#define CONFIG_NCP_BRIDGE
#undef CONFIG_NCP_BRIDGE_DEBUG
#define CONFIG_CRC32_HW_ACCELERATE

#define CONFIG_NCP_WIFI
#undef CONFIG_NCP_BLE

#define CONFIG_NCP_UART
#undef CONFIG_NCP_SPI
#undef CONFIG_NCP_USB
#undef CONFIG_NCP_SDIO

//#if defined(CONFIG_NCP_WIFI)
#define CONFIG_APP_NOTIFY_DEBUG
#include "wifi_config.h"
//#endif

//#if defined(CONFIG_NCP_BLE)
#include "app_bluetooth_config.h"
//#endif
