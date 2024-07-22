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
#define WIFI_BT_USE_IMU_INTERFACE  1
#define WIFI_BT_TX_PWR_LIMITS "wlan_txpwrlimit_cfg_WW_rw610.h"
#else
#error "Please define macro for RW610 board"
#endif


/*NCP config*/
#define CONFIG_NCP    1
#define CONFIG_NCP_DEBUG  0
#define CONFIG_CRC32_HW_ACCELERATE  1

#define CONFIG_NCP_WIFI   1
#define CONFIG_NCP_BLE    0

#define CONFIG_NCP_UART   1
#define CONFIG_NCP_SPI    0
#define CONFIG_NCP_USB    0
#define CONFIG_NCP_SDIO   0

//#if (CONFIG_NCP_WIFI)
#define CONFIG_APP_NOTIFY_DEBUG   1
#include "wifi_config.h"
//#endif

//#if (CONFIG_NCP_BLE)
#include "app_bluetooth_config.h"
//#endif
