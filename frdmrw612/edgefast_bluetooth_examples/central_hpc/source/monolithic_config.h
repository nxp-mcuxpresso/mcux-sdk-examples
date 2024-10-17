/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

/* This Monolithic config */
/** Monolithic feature default disabled, if set gPlatformMonolithicApp_d to 1, enable monolithic feature for loading CPU2 FW automatically */
#define gPlatformMonolithicApp_d        0

#if (defined(gPlatformMonolithicApp_d) && (gPlatformMonolithicApp_d > 0))

#ifndef RW610
#define RW610
#endif

#define CONFIG_SOC_SERIES_RW6XX_REVISION_A2     1
#define CONFIG_MONOLITHIC_BLE                   1

/** these macro are added to avoid build error (Error[Li006]) when using IAR compiler */
#define WIFI_FW_ADDRESS  0
#define COMBO_FW_ADDRESS 0

#endif /* gPlatformMonolithicApp_d */