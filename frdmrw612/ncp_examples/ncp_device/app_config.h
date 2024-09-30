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
#define CONFIG_NCP_OT     0

#if defined(WIFI_BOARD_RW610)
#if CONFIG_NCP_WIFI
#define CONFIG_MONOLITHIC_WIFI 1
#endif
#if (CONFIG_NCP_BLE) && (CONFIG_NCP_OT)
#define CONFIG_MONOLITHIC_BLE_15_4    1
#define CONFIG_MONOLITHIC_BLE         0
#else
#if CONFIG_NCP_BLE
#define CONFIG_MONOLITHIC_BLE_15_4    0
#define CONFIG_MONOLITHIC_BLE         1
#else
#define CONFIG_MONOLITHIC_BLE_15_4    0
#define CONFIG_MONOLITHIC_BLE         0
#endif
#endif
#endif

#if ((CONFIG_MONOLITHIC_WIFI) || (CONFIG_MONOLITHIC_BLE) || (CONFIG_MONOLITHIC_BLE_15_4))
#define CONFIG_SOC_SERIES_RW6XX_REVISION_A2 1
#endif

#if CONFIG_MONOLITHIC_BLE || CONFIG_MONOLITHIC_BLE_15_4
/** If set gPlatformMonolithicApp_d to 1, enable monolithic feature for loading CPU2 FW automatically */
#define gPlatformMonolithicApp_d        1

/** these macro are added to avoid build error (Error[Li006]) when using IAR compiler */
#define WIFI_FW_ADDRESS 0

#if CONFIG_MONOLITHIC_BLE
#define COMBO_FW_ADDRESS 0
#endif

#if CONFIG_MONOLITHIC_BLE_15_4
#define BLE_FW_ADDRESS 0
#endif

#endif /* CONFIG_MONOLITHIC_BLE || CONFIG_MONOLITHIC_BLE_15_4 */

#define CONFIG_NCP_UART   1
#define CONFIG_NCP_SPI    0
#define CONFIG_NCP_USB    0
#define CONFIG_NCP_SDIO   0

#define CONFIG_HOST_SLEEP           1
#define CONFIG_POWER_MANAGER        1

#if (CONFIG_NCP_BLE) || (CONFIG_NCP_OT)
#define configUSE_TICKLESS_IDLE 1
#define configUSE_IDLE_HOOK 0
#endif
//#if (CONFIG_NCP_WIFI)
#define CONFIG_APP_NOTIFY_DEBUG   1
#include "wifi_config.h"
//#endif

//#if (CONFIG_NCP_BLE)
#include "app_bluetooth_config.h"
//#endif

#if !defined(__GNUC__)
#if defined(CONFIG_NCP_OT) && (CONFIG_NCP_OT == 1)
#error "Ot ncp feature only supports arm gcc compilation"
#endif
#endif
