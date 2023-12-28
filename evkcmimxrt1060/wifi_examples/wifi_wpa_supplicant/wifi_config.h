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

#define CONFIG_MAX_RESCAN_LIMIT 30

#if defined(SD8978) || defined(SD8987) || defined(RW610) || defined(SD9177)
#define CONFIG_5GHz_SUPPORT 1
#endif

#define CONFIG_SDIO_MULTI_PORT_RX_AGGR 1

#if defined(SD8987) || defined(RW610) || defined(SD9177)
#define CONFIG_11AC
#undef CONFIG_WMM
#endif

#if defined(SD9177)
#define CONFIG_WMM
#define CONFIG_SDIO_MULTI_PORT_TX_AGGR 1
#define CONFIG_COMPRESS_TX_PWTBL
#define CONFIG_COMPRESS_RU_TX_PWTBL
#ifdef CONFIG_11AC
#define CONFIG_11AX
#endif
#endif


#define PRINTF_FLOAT_ENABLE 1

#define CONFIG_IPV6               1
#define CONFIG_MAX_IPV6_ADDRESSES 3

#if defined(SD8978) || defined(SD8987) || defined(SD8801) || defined(SD9177)
#define CONFIG_WIFI_CAPA        1
#define CONFIG_ROAMING          1
#define CONFIG_CLOUD_KEEP_ALIVE 1
#define CONFIG_TURBO_MODE       1
#if defined(SD8978) || defined(SD8987)
#define CONFIG_AUTO_RECONNECT 1
#undef CONFIG_WIFI_IND_DNLD
#undef CONFIG_WIFI_IND_RESET
#endif

#define CONFIG_OWE 1

#if !defined(SD8801)
#define CONFIG_EXT_SCAN_SUPPORT 1
#define CONFIG_WIFI_EU_CRYPTO   1
#define CONFIG_11R              1
#endif

#undef CONFIG_HOST_SLEEP
#undef CONFIG_MEF_CFG
#undef CONFIG_FIPS

#define CONFIG_11K 1
#define CONFIG_11V 1

/*
 * Config options for wpa supplicant
 */
#define CONFIG_WPA_SUPP 1

#ifdef CONFIG_WPA_SUPP
#define CONFIG_WPA_SUPP_WPS  1
#define CONFIG_WPA_SUPP_WPA3 1
#undef CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE

#if defined(SD9177)
#undef CONFIG_WPA_SUPP_DPP

#ifdef CONFIG_WPA_SUPP_DPP
#define CONFIG_WPA_SUPP_DPP2 1
#define CONFIG_WPA_SUPP_DPP3 1
#define CONFIG_RX_CHAN_INFO  1
#define CONFIG_TXPD_RXPD_V3  1
#endif
#endif

#ifdef CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE

#define CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE 1

#define CONFIG_EAP_TLS
#define CONFIG_EAP_PEAP
#define CONFIG_EAP_TTLS
#define CONFIG_EAP_FAST
#define CONFIG_EAP_SIM
#define CONFIG_EAP_AKA
#define CONFIG_EAP_AKA_PRIME

#if defined(CONFIG_EAP_PEAP) || defined(CONFIG_EAP_TTLS) || defined(CONFIG_EAP_FAST)
#define CONFIG_EAP_MSCHAPV2
#define CONFIG_EAP_GTC
#endif

#endif
#endif
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
#undef CONFIG_WIFI_PS_DEBUG
#undef CONFIG_WIFI_PKT_DEBUG
#undef CONFIG_WIFI_SCAN_DEBUG
#undef CONFIG_WIFI_IO_INFO_DUMP
#undef CONFIG_WIFI_IO_DEBUG
#undef CONFIG_WIFI_IO_DUMP
#undef CONFIG_WIFI_MEM_DEBUG
#undef CONFIG_WIFI_AMPDU_DEBUG
#undef CONFIG_WIFI_TIMER_DEBUG
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

/*
 * wpa supplicant debug options
 */
#define CONFIG_WPA_SUPP_DEBUG_LEVEL 6

#undef CONFIG_SUPP_DEBUG

#endif /* _WIFI_CONFIG_H_ */
