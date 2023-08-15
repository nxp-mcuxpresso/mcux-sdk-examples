/*
 * Copyright 2020-2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WEBCONFIG_H_
#define _WEBCONFIG_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

#define WIFI_SECURITY_LENGTH  63

/* Common Wi-Fi parameters */
#ifndef WIFI_SSID
#define WIFI_SSID "nxp_configuration_access_point"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "NXP0123456789"
#endif

#define WIFI_NETWORK_LABEL "MyWifi"

/* Parameters that apply to AP mode only */
#ifndef WIFI_AP_CHANNEL
#define WIFI_AP_CHANNEL 1
#endif

#define MAX_RETRY_TICKS 50

#ifndef HTTPD_STACKSIZE
#define HTTPD_STACKSIZE 512
#endif
#ifndef HTTPD_PRIORITY
#define HTTPD_PRIORITY DEFAULT_THREAD_PRIO
#endif

#define CONNECTION_INFO_FILENAME ("connection_info.dat")

#define WEBCONFIG_DEBUG

#ifdef WEBCONFIG_DEBUG
#define WC_DEBUG(__fmt__, ...) PRINTF(__fmt__, ##__VA_ARGS__)
#else
#define WC_DEBUG(...)
#endif

#endif /* _WEBCONFIG_H_ */
