/*
 *  Copyright 2020-2024 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#include "app_config.h"
#ifndef RW610
#include "wifi_bt_module_config.h"
#endif

#define CONFIG_IPV6 1
#define CONFIG_MAX_IPV6_ADDRESSES 3

#if defined(SD9177)
#define CONFIG_WMM 1
#define CONFIG_SDIO_MULTI_PORT_TX_AGGR 1
#define CONFIG_WIFI_FEATURES 1
#define CONFIG_OFFLOAD       1
#endif

#if defined(SD8978) || defined(SD8987) || defined(SD8801) || defined(SD9177)
#define CONFIG_WIFI_CAPA 1
#define CONFIG_ROAMING    1
#define CONFIG_TURBO_MODE       1
#define CONFIG_CLOUD_KEEP_ALIVE 1
#define CONFIG_AUTO_RECONNECT   1
#define CONFIG_EXT_SCAN_SUPPORT 1
#define CONFIG_WIFI_EU_CRYPTO 1
#define CONFIG_11R 1
#define CONFIG_DRIVER_OWE 1
#define CONFIG_11K 1
#define CONFIG_11V 1
#endif

#if defined(RW610)
#define CONFIG_MAX_RESCAN_LIMIT 30
#define PRINTF_FLOAT_ENABLE 1
#define CONFIG_HOST_SLEEP 1
#define CONFIG_POWER_MANAGER 1
#define CONFIG_CLOUD_KEEP_ALIVE 1
#define CONFIG_MEF_CFG 1
/** If define CONFIG_TX_RX_ZERO_COPY 1, please make sure
 *  #define PBUF_POOL_BUFSIZE 1752
 *  in lwipopts.h
 */
#define CONFIG_TX_RX_ZERO_COPY 1
#define CONFIG_ANT_DETECT 1
#endif

/* WLCMGR debug */
#define CONFIG_WLCMGR_DEBUG 0

/*
 * Wifi extra debug options
 */
#define CONFIG_WIFI_EXTRA_DEBUG 0
#define CONFIG_WIFI_EVENTS_DEBUG 0
#define CONFIG_WIFI_CMD_RESP_DEBUG 0
#define CONFIG_WIFI_PKT_DEBUG 0
#define CONFIG_WIFI_SCAN_DEBUG 0
#define CONFIG_WIFI_IO_INFO_DUMP 0
#define CONFIG_WIFI_IO_DEBUG 0
#define CONFIG_WIFI_IO_DUMP 0
#define CONFIG_WIFI_MEM_DEBUG 0
#define CONFIG_WIFI_AMPDU_DEBUG 0
#define CONFIG_WIFI_TIMER_DEBUG 0
#define CONFIG_WIFI_SDIO_DEBUG 0
#define CONFIG_WIFI_FW_DEBUG 0
#define CONFIG_WIFI_UAP_DEBUG 0
#define CONFIG_WPS_DEBUG 0
#define CONFIG_FW_VDLL_DEBUG 0
#define CONFIG_DHCP_SERVER_DEBUG 0
#define CONFIG_FWDNLD_IO_DEBUG 0

/*
 * Heap debug options
 */
#define CONFIG_HEAP_DEBUG 0
#define CONFIG_HEAP_STAT 0

#endif /* _WIFI_CONFIG_H_ */
