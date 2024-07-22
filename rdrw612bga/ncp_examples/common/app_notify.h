/* @file app_notify.h
 *
 *  @brief This file contains ncp device API functions definitions
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __APP_NOTIFY_H__
#define __APP_NOTIFY_H__

#include <wmlog.h>

#define app_e(...) wmlog_e("app_notify", ##__VA_ARGS__)
#define app_w(...) wmlog_w("app_notify", ##__VA_ARGS__)

#if CONFIG_APP_NOTIFY_DEBUG
#define app_d(...) wmlog("app_notify", ##__VA_ARGS__)
#else
#define app_d(...)
#endif

typedef enum
{
    /** Scan Result */
    APP_EVT_SCAN_RESULT = 0,
    /** Disconnect from the current network by request */
    APP_EVT_USER_DISCONNECT,
    /** Event for WLAN UAP PROVSION START RESULT */
    APP_EVT_UAP_PROV_START,
    /** connect the network by request */
    APP_EVT_USER_CONNECT,
    /** start the network by request */
    APP_EVT_USER_START_NETWORK,
    /** stop the network by request */
    APP_EVT_USER_STOP_NETWORK,
    /** WPS session is done */
    APP_EVT_WPS_DONE,
    /** Event for MCU sleep config */
    APP_EVT_HS_CONFIG,
    /** Event for suspend sleep status */
    APP_EVT_SUSPEND,
    /** Event for MCU enter sleep */
    APP_EVT_MCU_SLEEP_ENTER,
    /** Event for MCU exit sleep */
    APP_EVT_MCU_SLEEP_EXIT,
    /** Event for mDNS answers */
    APP_EVT_MDNS_SEARCH_RESULT,
    /** Event for resolving mDNS domain name */
    APP_EVT_MDNS_RESOLVE_DOMAIN_NAME,
    /** Event for invalid command */
    APP_EVT_INVALID_CMD,
    /** Event for CSI data*/
    APP_EVT_CSI_DATA,
} app_notify_event_t;

typedef enum
{
    APP_EVT_REASON_SUCCESS = 0,
    APP_EVT_REASON_FAILURE,
} app_event_reason_t;

#define APP_NOTIFY_SUSPEND_EVT 0x1U
#define APP_NOTIFY_SUSPEND_CFM 0x2U

/* app notify event queue message */
typedef struct
{
    uint16_t event;
    int reason;
    int data_len;
    void *data;
} app_notify_msg_t;

int app_notify_event(uint16_t event, int result, void *data, int len);

int app_notify_init(void);

#endif /* __APP_NOTIFY_H__ */
