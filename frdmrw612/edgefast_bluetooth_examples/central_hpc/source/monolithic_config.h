/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* Monolithic config */
#define gPlatformMonolithicApp_d        0

#if (defined(gPlatformMonolithicApp_d) && (gPlatformMonolithicApp_d > 0))

#ifndef RW610
#define RW610
#endif

#define CONFIG_SOC_SERIES_RW6XX_REVISION_A2     1
/* if load wifi fw_bin automatically need define CONFIG_MONOLITHIC_WIFI
   if load ble_only fw_bin automatically need define CONFIG_MONOLITHIC_BT
   or to load combo fw_bin automatically need define CONFIG_MONOLITHIC_IEEE802154 */
#define CONFIG_MONOLITHIC_BT            1
#define CONFIG_MONOLITHIC_WIFI          0
#define CONFIG_MONOLITHIC_IEEE802154    0

/* WIFI_FW_ADDRESS should be defined to not load cpu1 automatically, otherwise need be undefined
   COMBO_FW_ADDRESS should be defined if not loading combo fw_bin, otherwise need be undefined
   BLE_FW_ADDRESS should be defined if not loading ble_only fw_bin, otherwise need be undefined */
#define WIFI_FW_ADDRESS  0
#define COMBO_FW_ADDRESS 0
#undef BLE_FW_ADDRESS
#endif