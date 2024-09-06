/*
 *  Copyright 2020-2022 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#define CONFIG_WIFI_MAX_PRIO (configMAX_PRIORITIES - 1)

#ifndef RW610
#define CONFIG_MAX_AP_ENTRIES 10
#else
#define CONFIG_MAX_AP_ENTRIES 30
#endif

#if defined(SD8977) || defined(SD8978) || defined(SD8987) || defined(RW610)
#define CONFIG_5GHz_SUPPORT 1
#endif

#ifndef RW610
#define CONFIG_SDIO_MULTI_PORT_RX_AGGR 1
#endif

#if defined(SD8987) || defined(RW610)
#define CONFIG_11AC 1
#endif

#if defined(RW610)
#define PRINTF_FLOAT_ENABLE 1
#define CONFIG_11AX         1
#define CONFIG_IMU_GDMA     0
/* WMM options */
#define CONFIG_WMM                     1
#define CONFIG_AMSDU_IN_AMPDU          0
/* OWE mode */
#define CONFIG_DRIVER_OWE              1
/* WLAN SCAN OPT */
#define CONFIG_SCAN_WITH_RSSIFILTER    1
#define CONFIG_WIFI_DTIM_PERIOD        1
#define CONFIG_UART_INTERRUPT          1
#define CONFIG_WIFI_CAPA               1
#define CONFIG_WIFI_11D_ENABLE         1
#define CONFIG_WIFI_HIDDEN_SSID        1
#define CONFIG_WMM_UAPSD               1
#define CONFIG_WIFI_GET_LOG            1
#define CONFIG_ROAMING                 1

#define CONFIG_CSI                     1
#define CONFIG_WIFI_RESET              1
#define CONFIG_NET_MONITOR             1
#define CONFIG_WIFI_MEM_ACCESS         1
#define CONFIG_WIFI_REG_ACCESS         1
#define CONFIG_ECSA                    1
#define CONFIG_WIFI_EU_CRYPTO          1
#define CONFIG_EXT_SCAN_SUPPORT        1
#define CONFIG_EVENT_MEM_ACCESS        1
#define CONFIG_COMPRESS_TX_PWTBL       1
#define CONFIG_RX_ABORT_CFG            1
#define CONFIG_RX_ABORT_CFG_EXT        1
#define CONFIG_CCK_DESENSE_CFG         1
#define CONFIG_11AX_TWT                1
#define CONFIG_IPS                     1
#define CONFIG_SUBSCRIBE_EVENT_SUPPORT 1
#define CONFIG_TSP                     1
#define CONFIG_TX_RX_HISTOGRAM         1
#define CONFIG_TURBO_MODE              1
#define CONFIG_MMSF                    1
#define CONFIG_COEX_DUTY_CYCLE         1
/** If define CONFIG_TX_RX_ZERO_COPY 1, please make sure
 *  #define PBUF_POOL_BUFSIZE 1752
 *  in lwipopts.h
 */
#define CONFIG_TX_RX_ZERO_COPY         0
#define CONFIG_WIFI_CLOCKSYNC          1
#define CONFIG_INACTIVITY_TIMEOUT_EXT  1
#define CONFIG_UNII4_BAND_SUPPORT      1
#define CONFIG_MEF_CFG                 1
#define CONFIG_CAU_TEMPERATURE         1
#define CONFIG_AUTO_NULL_TX            1
#define CONFIG_RF_TEST_MODE            1
#define CONFIG_FW_VDLLV2               1
#define CONFIG_NCP_SOCKET_SEND_FIFO    0
#endif

#define CONFIG_IPV6               1
#define CONFIG_MAX_IPV6_ADDRESSES 3

#define CONFIG_11K 1
#define CONFIG_11V 1
#define CONFIG_11R 1

#define CONFIG_SCAN_CHANNEL_GAP 1

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS   1
#define CONFIG_ENABLE_WARNING_LOGS 1

/*NCP config*/
#define CONFIG_NCP_IPV6               1
#define CONFIG_NCP_5GHz_SUPPORT       1
#define CONFIG_NCP_WIFI_CAPA          1
#define CONFIG_NCP_WIFI_DTIM_PERIOD   1
#define CONFIG_NCP_11R                1
#define CONFIG_NCP_OWE                1
#define CONFIG_NCP_RF_TEST_MODE       1
#define CONFIG_NCP_DEBUG              0
#define CONFIG_NCP_11AC               1
#define CONFIG_NCP_11AX               1

/*https client and websocket*/
#define CONFIG_ENABLE_HTTPC_SECURE      1
#define CONFIG_ENABLE_HTTPC_MODIFY_TIME 0
#define CONFIG_ENABLE_HTTPD_STATS       0
#define CONFIG_ENABLE_TLS               1
#define APPCONFIG_WEB_SOCKET_SUPPORT    1
#define CONFIG_HTTPC_DEBUG              0
/* wmcypto internal use to alloc buffer */
#define MBEDTLS_SSL_BUFFER_SIZES      1

/* WLCMGR debug */
#define CONFIG_WLCMGR_DEBUG           0

/*
 * Wifi extra debug options
 */
#define CONFIG_WIFI_EXTRA_DEBUG     0
#define CONFIG_WIFI_EVENTS_DEBUG    0
#define CONFIG_WIFI_CMD_RESP_DEBUG  0
#define CONFIG_WIFI_PKT_DEBUG       0
#define CONFIG_WIFI_SCAN_DEBUG      0
#define CONFIG_WIFI_IO_INFO_DUMP    0
#define CONFIG_WIFI_IO_DEBUG        0
#define CONFIG_WIFI_IO_DUMP         0
#define CONFIG_WIFI_MEM_DEBUG       0
#define CONFIG_WIFI_AMPDU_DEBUG     0
#define CONFIG_WIFI_TIMER_DEBUG     0
#define CONFIG_WIFI_SDIO_DEBUG      0
#define CONFIG_WIFI_FW_DEBUG        0
#define CONFIG_WIFI_UAP_DEBUG       0
#define CONFIG_DRIVER_FIPS          0
/*
 * Heap debug options
 */
#define CONFIG_HEAP_DEBUG           0
#define CONFIG_HEAP_STAT            0

/*
 * Config options for supplicant
 */
#define WIFI_ADD_ON     1

#define CONFIG_NCP_SUPP   1
#if CONFIG_NCP_SUPP
#define CONFIG_WPA_SUPP        1
#define CONFIG_WIFI_NXP        1
#define CONFIG_WPA_SUPP_CRYPTO 1
#define CONFIG_WPA_SUPP_AP     1
#define CONFIG_HOSTAPD         1
#define CONFIG_WPA_SUPP_WPS    1
//#define CONFIG_WPA_SUPP_P2P 1
#define CONFIG_WPA_SUPP_WPA3                 1
#define CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE    1
#define CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE 1
#define UAP_HOST_MLME                        1
#define CONFIG_HOST_MLME                     1

#if CONFIG_WPA_SUPP_CRYPTO_ENTERPRISE || CONFIG_WPA_SUPP_CRYPTO_AP_ENTERPRISE
#define CONFIG_EAP_TLS       1
#define CONFIG_EAP_PEAP      1
#define CONFIG_EAP_TTLS      1
#define CONFIG_EAP_FAST      1
#define CONFIG_EAP_SIM       1
#define CONFIG_EAP_AKA       1
#define CONFIG_EAP_AKA_PRIME 1

#if CONFIG_EAP_PEAP || CONFIG_EAP_TTLS || CONFIG_EAP_FAST
#define CONFIG_EAP_MSCHAPV2  1
#define CONFIG_EAP_GTC       1
#endif
#endif

#define CONFIG_WPA_SUPP_DEBUG_LEVEL 3
#define CONFIG_LOG_BUFFER_SIZE      2048
//#define CONFIG_NO_STDOUT_DEBUG 1
//#define WPA_SUPPLICANT_CLEANUP_INTERVAL 120
#define HOSTAPD_CLEANUP_INTERVAL    120
#define CONFIG_WIFI_USB_FILE_ACCESS 0
#define CONFIG_NCP_SUPP_WPS      1
#endif

#define CONFIG_DRIVER_MBO 1

#if CONFIG_WIFI_USB_FILE_ACCESS && CONFIG_NCP_USB
#error " CONFIG_NCP_USB and CONFIG_WIFI_USB_FILE_ACCESS are exclusive for ncp and ncp_supp"
#endif

#endif /* _WIFI_CONFIG_H_ */
