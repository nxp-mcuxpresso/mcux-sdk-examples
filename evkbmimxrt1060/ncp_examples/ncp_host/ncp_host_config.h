/*
 *  Copyright 2020-2022 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *  The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef _WIFI_CONFIG_H_
#define _WIFI_CONFIG_H_

#define PRINTF_FLOAT_ENABLE 1

#define CONFIG_MAX_AP_ENTRIES   30
#define CONFIG_NCP_5GHz_SUPPORT 1
#define CONFIG_NCP_11AC         1
#define CONFIG_NCP_11AX         1
/* OWE mode */
#define CONFIG_NCP_OWE          1
/* WLAN SCAN OPT */
#define CONFIG_NCP_SCAN_WITH_RSSIFILTER  1
#define CONFIG_NCP_WIFI_DTIM_PERIOD      1
#define CONFIG_NCP_WIFI_CAPA             1
#define CONFIG_NCP_11R                   1


#define CONFIG_NCP_IPV6                  1
#define CONFIG_MAX_IPV6_ADDRESSES        3

/* Logs */
#define CONFIG_ENABLE_ERROR_LOGS   1
#define CONFIG_ENABLE_WARNING_LOGS 1

#define CONFIG_NCP_SUPP    1

#define CONFIG_NCP  1
/*NCP Host debug options*/
#define CONFIG_NCP_DEBUG             0
#define CONFIG_NCP_HOST_IO_DUMP      0
#define CONFIG_CRC32_HW_ACCELERATE   0
#define CONFIG_NCP_RF_TEST_MODE      1
#define CONFIG_NCP_MEM_MONITOR_DEBUG 1

/*https client and websocket*/
#define CONFIG_ENABLE_HTTPC_SECURE   1
#define CONFIG_ENABLE_TLS            1
#define APPCONFIG_WEB_SOCKET_SUPPORT 1
#define CONFIG_HTTPC_DEBUG           0

/* Interface options */
#define CONFIG_NCP_UART                   1
#define CONFIG_NCP_SPI                    0
#define CONFIG_NCP_USB                    0
#define CONFIG_NCP_SDIO                   0
#define COFNIG_NCP_SDIO_TEST_LOOPBACK     0
#define CONFIG_SDIO_IO_DEBUG              0

/* protocol options */
#define CONFIG_NCP_WIFI                   1
#define CONFIG_NCP_BLE                    0
#define CONFIG_NCP_OT                     0

/* NCP BLE mode options */
#if (CONFIG_NCP_BLE)
#define CONFIG_NCP_BLE_PROFILE_MODE       1
#if !CONFIG_NCP_BLE_PROFILE_MODE
#define CONFIG_NCP_BLE_NO_PROFILE_MODE    1
#endif /* CONFIG_NCP_BLE_PROFILE_MODE */
#endif /* CONFIG_NCP_BLE */

/* BLE profile options */
#if (CONFIG_NCP_BLE_PROFILE_MODE)
#define CONFIG_NCP_HTS      1
#define CONFIG_NCP_HTC      1
#define CONFIG_NCP_HRS      1
#define CONFIG_NCP_HRC      1
#define CONFIG_NCP_BAS      1
#endif /* CONFIG_NCP_BLE_PROFILE_MODE */

/*
 * Heap debug options
 */
#define CONFIG_NCP_HEAP_DEBUG  0
#define CONFIG_NCP_HEAP_STAT   0

#if CONFIG_NCP_SUPP
#define CONFIG_NCP_WPA_SUPP    1
#if CONFIG_NCP_WPA_SUPP
//#define CONFIG_WPA_SUPP_P2P 1
#define CONFIG_NCP_WPA_SUPP_DPP 0
#define CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE 1
#define CONFIG_NCP_WPA_SUPP_CRYPTO_AP_ENTERPRISE 1

#if (CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE) || (CONFIG_NCP_WPA_SUPP_CRYPTO_AP_ENTERPRISE)
#define CONFIG_NCP_EAP_TLS        1
#define CONFIG_NCP_EAP_PEAP       1
#define CONFIG_NCP_EAP_TTLS       1
#define CONFIG_NCP_EAP_FAST       1
#define CONFIG_NCP_EAP_SIM        1
#define CONFIG_NCP_EAP_AKA        1
#define CONFIG_NCP_EAP_AKA_PRIME  1

#if (CONFIG_NCP_EAP_PEAP) || (CONFIG_NCP_EAP_TTLS) || (CONFIG_NCP_EAP_FAST)
#define CONFIG_NCP_EAP_MSCHAPV2   1
#define CONFIG_NCP_EAP_GTC        1
#endif
#endif

#else

#endif

#if (CONFIG_WIFI_USB_FILE_ACCESS) && (CONFIG_NCP_USB)
    #error " CONFIG_NCP_USB and CONFIG_WIFI_USB_FILE_ACCESS are exclusive for ncp and ncp_supp"
#endif

#define CONFIG_NCP_SUPP_WPS   1
#else
#define CONFIG_NCP_WPA_SUPP   0
#endif /*CONFIG_NCP_SUPP*/

#endif /* _WIFI_CONFIG_H_ */
