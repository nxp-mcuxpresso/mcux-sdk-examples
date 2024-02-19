/*
 *  Copyright 2020-2022 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#define CONFIG_WIFI_MAX_PRIO (configMAX_PRIORITIES - 1)

#define CONFIG_MAX_AP_ENTRIES 10

#if defined(SD8978) || defined(SD8987) || defined(RW610) || defined(SD9177)
#define CONFIG_5GHz_SUPPORT 1
#endif

#define CONFIG_SDIO_MULTI_PORT_RX_AGGR 1

#if defined(SD8987) || defined(SD9177)
#define CONFIG_11AC
#undef CONFIG_WMM
#endif

#if defined(SD8978) || defined(SD8987)
#define CONFIG_FW_VDLL 1
#endif

#define CONFIG_RF_TEST_MODE 1
#if defined(SD9177)
#ifdef CONFIG_11AC
#define CONFIG_11AX
#endif
#define CONFIG_EXT_SCAN_SUPPORT 1
#define CONFIG_OWE              1
#define CONFIG_COMPRESS_TX_PWTBL
#define CONFIG_COMPRESS_RU_TX_PWTBL
#undef CONFIG_UNII4_BAND_SUPPORT
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
#undef CONFIG_WIFI_PKT_DEBUG
#undef CONFIG_WIFI_SCAN_DEBUG
#undef CONFIG_WIFI_IO_INFO_DUMP
#undef CONFIG_WIFI_IO_DEBUG
#undef CONFIG_WIFI_IO_DUMP
#undef CONFIG_WIFI_MEM_DEBUG
#undef CONFIG_WIFI_AMPDU_DEBUG
#undef CONFIG_WIFI_TIMER_DEBUG
#undef CONFIG_WIFI_SDIO_DEBUG
#undef CONFIG_WIFI_FW_DEBUG
#undef CONFIG_WIFI_UAP_DEBUG
#undef CONFIG_WPS_DEBUG
#undef CONFIG_FW_VDLL_DEBUG
#undef CONFIG_DHCP_SERVER_DEBUG
#undef CONFIG_WIFI_SDIO_DEBUG
#undef CONFIG_FWDNLD_IO_DEBUG

/*
 * Heap debug options
 */
#undef CONFIG_HEAP_DEBUG
#undef CONFIG_HEAP_STAT

#endif /* _WIFI_CONFIG_H_ */
