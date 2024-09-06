/*
 * Copyright 2020-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

#ifndef _UAP_PROV_H_
#define _UAP_PROV_H_

#include <wmlog.h>
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* @TEST_ANCHOR */

/* Common Wi-Fi parameters */
#define DEF_UAP_SSID "nxpdemo-"
#define DEF_UAP_PASS "12345678"
#define DEF_UAP_SEC  WLAN_SECURITY_WPA2
#define DEF_UAP_CH   0

#define DEF_UAP_NETWORK_LABEL "uap_prov_uap_label"
#define DEF_STA_NETWORK_LABEL "uap_prov_sta_label"

#define DEF_UAP_IP_ADDR "192.168.1.1"

#define MAX_RETRY_TICKS 50

#ifndef HTTPD_STACKSIZE
#define HTTPD_STACKSIZE 512
#endif
#ifndef HTTPD_PRIORITY
#define HTTPD_PRIORITY DEFAULT_THREAD_PRIO
#endif

#define uap_prov_e(...) wmlog_e("uap_prov", ##__VA_ARGS__)
#define uap_prov_w(...) wmlog_w("uap_prov", ##__VA_ARGS__)

#ifdef CONFIG_UAP_PROV_DEBUG
#define uap_prov_d(...) wmlog("uap_prov", ##__VA_ARGS__)
#else
#define uap_prov_d(...)
#endif /* ! CONFIG_UAP_PROV_DEBUG */

#define SM_EXIT 1

typedef enum _network_from
{
    NETWORK_FROM_CONFIG,
    NETWORK_FROM_HTTP,
    NETWORK_FROM_INVALID,
} network_from;

typedef enum _uap_prov_status
{
    STATUS_IDLE,
    STATUS_INIT_DONE,
    STATUS_DEACTIVATING,
    STATUS_DEACTIVATED,
    STATUS_INVALID,
} uap_prov_status;

typedef enum _uap_prov_wifi_states
{
    WIFI_STATE_IDLE,
    WIFI_STATE_STA_CONNECTING,
    WIFI_STATE_STA_CONNECTED,
    WIFI_STATE_STA_DISCONNECTING,
    WIFI_STATE_UAP_STARTING,
    WIFI_STATE_UAP_STARTED,
    WIFI_STATE_UAP_STA_SCANNING,
    WIFI_STATE_UAP_STA_SCAN_DONE,
    WIFI_STATE_UAP_STA_CONNECTING,
    WIFI_STATE_UAP_STA_DISCONNECTING,
    WIFI_STATE_UAP_STA_UAP_STOPPING,
    WIFI_STATE_UAP_STA_CONNECTED,
    WIFI_STATE_INVALID,
} uap_prov_wifi_states;

typedef struct _uap_prov_vars
{
    uap_prov_status status;
    uap_prov_wifi_states wifiState;
    char staSsid[IEEEtypes_SSID_SIZE + 1];
    char staPass[WLAN_PSK_MAX_LENGTH];
    /* uap prov thread */
    OSA_TASK_HANDLE_DEFINE(thread);
} uap_prov_vars;

typedef struct _uap_prov_uap_config
{
    char uapSsid[IEEEtypes_SSID_SIZE + 1];
    char uapPass[WLAN_PASSWORD_MAX_LENGTH + 1];
    uint32_t uapSec;
} uap_prov_uap_config;

typedef struct _uap_prov_queues
{
    /* uap prov queue */
    OSA_MSGQ_HANDLE_DEFINE(queue, MAX_EVENTS, sizeof(struct q_message));
    /* http server session queue */
    OSA_MSGQ_HANDLE_DEFINE(httpsrv_ses_queue, MAX_EVENTS, sizeof(struct q_message));
} uap_prov_queues;

#define MSG_TYPE_CMD 0
#define MSG_TYPE_EVT 1

#define CMD_REASON_UAP_PROV_START    0
#define CMD_REASON_UAP_PROV_RESET    1
#define CMD_REASON_UAP_PROV_STA_SCAN 2
#define CMD_REASON_UAP_PROV_STA_CONN 3

typedef struct _post_ap_info
{
    char posted_ssid[IEEEtypes_SSID_SIZE + 1];
    char posted_pass[WLAN_PSK_MAX_LENGTH];
} post_ap_info;

#define MAX_EVENTS 20

struct q_message
{
    /* type for cmd/event */
    uint16_t type;
    /* reason for cmd/event */
    uint16_t reason;
    /* data for cmd/event */
    void *data;
};

int uap_prov_cli_init(void);
int uap_prov_deinit(void);
void uap_prov_drain_queue(void);
void uap_prov_cleanup(void);

int uap_prov_start();
int uap_prov_reset();
int uap_prov_set_uapcfg(char *ssid, uint32_t sec, char *pass);

uint8_t check_valid_event_for_uap_prov(uint16_t reason);
uint8_t check_valid_status_for_uap_prov();
int send_msg_to_uap_prov(uint16_t type, uint16_t reason, int data);

#endif /* _UAP_PROV_H_ */
