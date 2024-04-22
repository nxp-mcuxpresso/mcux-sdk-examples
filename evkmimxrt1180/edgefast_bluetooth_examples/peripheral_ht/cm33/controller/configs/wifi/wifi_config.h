/*
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef _WIFI_CONFIG_H__
#define _WIFI_CONFIG_H__

#define CONFIG_MAX_AP_ENTRIES 10
#define CONFIG_UAP_AMPDU_TX   1
#define CONFIG_UAP_AMPDU_RX   1
#define CONFIG_WIFI_MAX_PRIO (configMAX_PRIORITIES - 1)

#if defined(SD8977) || defined(SD8978) || defined(SD8987)
#define CONFIG_5GHz_SUPPORT 1
#endif

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS   1
#define CONFIG_ENABLE_WARNING_LOGS 1

/* WLCMGR debug */
#undef CONFIG_WLCMGR_DEBUG

/*
 * Wifi extra debug options
 */
#undef CONFIG_WIFI_EXTRA_DEBUG
#undef CONFIG_WIFI_EVENTS_DEBUG
#undef CONFIG_WIFI_CMD_RESP_DEBUG
#undef CONFIG_WIFI_SCAN_DEBUG
#undef CONFIG_WIFI_IO_INFO_DUMP
#undef CONFIG_WIFI_IO_DEBUG
#undef CONFIG_WIFI_IO_DUMP
#undef CONFIG_WIFI_MEM_DEBUG
#undef CONFIG_WIFI_AMPDU_DEBUG
#undef CONFIG_WIFI_TIMER_DEBUG
#undef CONFIG_WIFI_SDIO_DEBUG

#endif /* __WIFI_CONFIG_H__ */
