/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 * The BSD-3-Clause license can be found at https://spdx.org/licenses/BSD-3-Clause.html
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/tcpip.h"
#include "timers.h"
#include "httpsrv.h"
#include "http_server.h"

#include "fsl_debug_console.h"
#include "wlan.h"
#include "uap_prov.h"
#include "app_notify.h"
#include "ncp_config.h"

#include <stdio.h>
#include <cli.h>
#include <wmerrno.h>
#include <osa.h>
#include <wm_net.h>

#include "FreeRTOS.h"

#define MAX_JSON_NETWORK_RECORD_LENGTH 185

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static int CGI_HandleGet(HTTPSRV_CGI_REQ_STRUCT *param);
static int CGI_HandlePost(HTTPSRV_CGI_REQ_STRUCT *param);
static int CGI_HandleReset(HTTPSRV_CGI_REQ_STRUCT *param);
static int CGI_HandleStatus(HTTPSRV_CGI_REQ_STRUCT *param);

static int config_set_sta_network();
static int config_reset_sta_network();
static int config_set_uapcfg(char *ssid, uint32_t sec, char *pass);
static int config_reset_uapcfg();
static int check_valid_uapcfg(char *ssid, uint32_t sec, char *pass);
static int validate_and_copy_uapcfg(char *ssid, uint32_t sec, char *pass);

static int do_get_ip(char *ip, int client);
static int do_sta_add_network(network_from nw_from, char *ssid, char *pass, char *label);
static int do_sta_connect(char *label);
static int do_fall_to_sta(network_from nw_from);
static int do_uap_add_network(uap_prov_uap_config *uapcfg, char *label, uint32_t channel);
static int do_uap_start(char *label);
static int do_fall_to_uap();
static int do_uap_stop();

static char *uap_prov_get_status_str(uap_prov_status status);
static char *uap_prov_get_sm_state_str(uap_prov_wifi_states wifi_state);
static int uap_prov_scan_cb(unsigned int count);
static int send_msg_to_httpsrv_ses(uint16_t type, uint16_t reason, int data);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
const HTTPSRV_CGI_LINK_STRUCT cgi_lnk_tbl[] = {
    {"reset", CGI_HandleReset},
    {"get", CGI_HandleGet},
    {"post", CGI_HandlePost},
    {"status", CGI_HandleStatus},
    {0, 0} // DO NOT REMOVE - last item - end of table
};

/*******************************************************************************
 * Variables
 ******************************************************************************/
static void uap_prov_main(void *data);
OSA_TASK_DEFINE(uap_prov_main, OSA_PRIORITY_NORMAL, 1, 4096, 0);

uap_prov_vars uap_prov;
uap_prov_uap_config uap_prov_uapcfg;
uap_prov_queues uap_prov_q;

post_ap_info g_post_ap_info;

extern wifi_bss_config wifi_lfs_bss_config[5];

static char uap_prov_sm_state_str[][32] = {
    "IDLE",
    "STA_CONNECTING",
    "STA_CONNECTED",
    "STA_DISCONNECTING",
    "UAP_STARTING",
    "UAP_STARTED",
    "UAP_STA_SCANNING",
    "UAP_STA_SCAN_DONE",
    "UAP_STA_CONNECTING",
    "UAP_STA_DISCONNECTING",
    "UAP_STA_UAP_STOPPING",
    "UAP_STA_CONNECTED",
    "INVALID",
};

static char uap_prov_status_str[][32] = {
    "STATUS_IDLE", "STATUS_INIT_DONE", "STATUS_DEACTIVATING", "STATUS_DEACTIVATED", "STATUS_INVALID",
};

/*******************************************************************************
 * Code
 ******************************************************************************/
/*CGI*/
/* Example Common Gateway Interface callback. */
/* These callbacks are called from the session tasks according to the Link struct above */
/* The get.cgi request triggers a scan and responds with a list of the SSIDs */
static int CGI_HandleGet(HTTPSRV_CGI_REQ_STRUCT *param)
{
    int ret = WM_SUCCESS;
    struct q_message msg;
    osa_status_t status;
    /* Buffer for hodling response JSON data */
    char buffer[32]                     = {0};
    HTTPSRV_CGI_RES_STRUCT response     = {0};
    struct wlan_scan_result scan_result = {0};
    char *ssids_json                    = NULL;
    uint32_t ssids_json_len             = 0;
    int count                           = 0;
    uint32_t ssids_json_idx             = 0;
    int i                               = 0;

    if (uap_prov.status != STATUS_INIT_DONE)
    {
        uap_prov_e("CGI_HandleGet: Not allowed in status %d", uap_prov.status);
        strcpy(buffer, "{\"networks\":\"false\"}");
        goto send_rsp;
    }
    if ((uap_prov.wifiState != WIFI_STATE_UAP_STARTED) && (uap_prov.wifiState != WIFI_STATE_UAP_STA_SCAN_DONE))
    {
        uap_prov_e("CGI_HandleGet: Not allowed in wifiState %d", uap_prov.wifiState);
        strcpy(buffer, "{\"networks\":\"false\"}");
        goto send_rsp;
    }

    response.ses_handle   = param->ses_handle;
    response.status_code  = HTTPSRV_CODE_OK;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.data         = buffer;

    /* Initiate Scan */
    PRINTF("Initiating scan...\r\n");
    ret = send_msg_to_uap_prov(MSG_TYPE_CMD, CMD_REASON_UAP_PROV_STA_SCAN, 0);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("CGI_HandleGet: Failed to send_msg_to_uap_prov: %d", ret);
        strcpy(buffer, "{\"networks\":\"error\"}");
        goto send_rsp;
    }

    status = OSA_MsgQGet((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue, &msg, osaWaitForever_c);
    if (status != KOSA_StatusSuccess)
    {
        uap_prov_e("CGI_HandleGet: Failed to recv on httpsrv_ses_queue: %d", status);
        strcpy(buffer, "{\"networks\":\"error\"}");
        goto send_rsp;
    }

    if ((msg.type == MSG_TYPE_EVT) && (msg.reason == WLAN_REASON_SCAN_DONE))
    {
        count = (int)(msg.data);
    }
    else
    {
        uap_prov_e("CGI_HandleGet: Recv invalid msg: %u %u %d", msg.type, msg.reason, msg.data);
        strcpy(buffer, "{\"networks\":\"error\"}");
        goto send_rsp;
    }

    if (count < 0)
    {
        uap_prov_e("CGI_HandleGet: Recv scan fail msg: %u %u %d", msg.type, msg.reason, msg.data);
        strcpy(buffer, "{\"networks\":\"error\"}");
        goto send_rsp;
    }
    else if (count == 0)
    {
        strcpy(buffer, "{\"networks\":\"null\"}");
        // strcpy(ssids_json, "{\"networks\":[]}");???
        goto send_rsp;
    }

    /* Add length of "{"networks":[]}" */
    ssids_json_len = 15;
    ssids_json_len += count * MAX_JSON_NETWORK_RECORD_LENGTH;
    ssids_json = (char *)OSA_MemoryAllocate(ssids_json_len);
    if (ssids_json == NULL)
    {
        uap_prov_e("CGI_HandleGet: ssids_json allocation failed len=%u", ssids_json_len);
        strcpy(buffer, "{\"networks\":\"error\"}");
        goto send_rsp;
    }

    /* Start building JSON */
    strcpy(ssids_json, "{\"networks\":[");
    ssids_json_idx = strlen(ssids_json);

    for (i = 0; i < count; i++)
    {
        char security[40] = {0};
        wlan_get_scan_result(i, &scan_result);
        PRINTF("%s\r\n", scan_result.ssid);
        PRINTF("	 BSSID		   : %02X:%02X:%02X:%02X:%02X:%02X\r\n", (unsigned int)scan_result.bssid[0],
               (unsigned int)scan_result.bssid[1], (unsigned int)scan_result.bssid[2],
               (unsigned int)scan_result.bssid[3], (unsigned int)scan_result.bssid[4],
               (unsigned int)scan_result.bssid[5]);
        PRINTF("	 RSSI		   : %ddBm\r\n", -(int)scan_result.rssi);
        PRINTF("	 Channel	   : %d\r\n", (int)scan_result.channel);
        security[0] = '\0';
        if (scan_result.wpa2_entp)
        {
            strcat(security, "WPA2_ENTP ");
        }
        if (scan_result.wep)
        {
            strcat(security, "WEP ");
        }
        if (scan_result.wpa)
        {
            strcat(security, "WPA ");
        }
        if (scan_result.wpa2)
        {
            strcat(security, "WPA2 ");
        }
        if (scan_result.wpa3_sae)
        {
            strcat(security, "WPA3_SAE ");
        }
        if (i != 0)
        {
            /* Add ',' separator before next entry */
            ssids_json[ssids_json_idx++] = ',';
        }
        ret = snprintf(
            ssids_json + ssids_json_idx, ssids_json_len - ssids_json_idx - 1,
            "{\"ssid\":\"%s\",\"bssid\":\"%02X:%02X:%02X:%02X:%02X:%02X\",\"signal\":\"%ddBm\",\"channel\":%d,"
            "\"security\":\"%s\"}",
            scan_result.ssid, (unsigned int)scan_result.bssid[0], (unsigned int)scan_result.bssid[1],
            (unsigned int)scan_result.bssid[2], (unsigned int)scan_result.bssid[3], (unsigned int)scan_result.bssid[4],
            (unsigned int)scan_result.bssid[5], -(int)scan_result.rssi, (int)scan_result.channel, security);
        if (ret <= 0)
        {
            uap_prov_e("CGI_HandleGet: JSON creation failed");
            strcpy(buffer, "{\"networks\":\"error\"}");
            goto send_rsp;
        }
        ssids_json_idx += ret;
    }
    /* End of JSON "]}" */
    strcpy(ssids_json + ssids_json_idx, "]}");
    response.data = ssids_json;

send_rsp:
    /* Send the response back to browser */
    response.data_length    = strlen(response.data);
    response.content_length = response.data_length;
    HTTPSRV_cgi_write(&response);

    if (ssids_json != NULL)
    {
        OSA_MemoryFree(ssids_json);
        ssids_json = NULL;
    }

    return (response.content_length);
}

/* The post.cgi request is used for triggering a connection to an external AP */
static int CGI_HandlePost(HTTPSRV_CGI_REQ_STRUCT *param)
{
    int ret                         = -WM_FAIL;
    HTTPSRV_CGI_RES_STRUCT response = {0};
    char buffer[256]                = {0};
    uint32_t length                 = 0;
    uint32_t read                   = 0;
    char posted_ssid[IEEEtypes_SSID_SIZE + 1];
    char posted_passphrase[WLAN_PSK_MAX_LENGTH];
    struct q_message msg = {0};
    osa_status_t status;

    if (uap_prov.status != STATUS_INIT_DONE)
    {
        uap_prov_e("CGI_HandlePost: Not allowed in status %d", uap_prov.status);
        goto send_rsp;
    }
    if (uap_prov.wifiState != WIFI_STATE_UAP_STA_SCAN_DONE)
    {
        uap_prov_e("CGI_HandlePost: Not allowed in wifiState %u", uap_prov.wifiState);
        goto send_rsp;
    }

    response.ses_handle   = param->ses_handle;
    response.status_code  = HTTPSRV_CODE_OK;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;

    length = MIN(param->content_length, (sizeof(buffer) - 1));
    read   = HTTPSRV_cgi_read(param->ses_handle, buffer, length);
    if (read == 0)
    {
        uap_prov_e("CGI_HandlePost: HTTPSRV_cgi_read fail %u");
        goto send_rsp;
    }
    buffer[length] = '\0';
    cgi_get_varval(buffer, "post_ssid", posted_ssid, sizeof(posted_ssid));
    cgi_get_varval(buffer, "post_passphrase", posted_passphrase, sizeof(posted_passphrase));
    cgi_urldecode(posted_ssid);
    cgi_urldecode(posted_passphrase);
    /* Any post processing of the posted data (sanitation, validation) */
    format_post_data(posted_ssid);
    format_post_data(posted_passphrase);
    PRINTF("[i] Chosen ssid: %s\r\n[i] Chosen passphrase: \"%s\" \r\n", posted_ssid, posted_passphrase);

    memset(&g_post_ap_info, 0x00, sizeof(g_post_ap_info));
    strcpy(g_post_ap_info.posted_ssid, posted_ssid);
    strcpy(g_post_ap_info.posted_pass, posted_passphrase);
    ret = send_msg_to_uap_prov(MSG_TYPE_CMD, CMD_REASON_UAP_PROV_STA_CONN, (int)&g_post_ap_info);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("CGI_HandlePost: Failed to send_msg_to_uap_prov: %d", ret);
        goto send_rsp;
    }

    status = OSA_MsgQGet((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue, &msg, osaWaitForever_c);
    if (status != KOSA_StatusSuccess)
    {
        uap_prov_e("CGI_HandlePost: Failed to recv on uap_prov_q.httpsrv_ses_queue: %d", status);
        goto send_rsp;
    }

    ret = -WM_FAIL;
    if ((msg.type == MSG_TYPE_EVT) && (msg.reason == WLAN_REASON_SUCCESS))
    {
        ret = WM_SUCCESS;
    }
    else
    {
        uap_prov_e("CGI_HandlePost: Recv invalid msg: %u %u %d", msg.type, msg.reason, msg.data);
    }

send_rsp:
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("[!] Cannot connect to wifi\r\n[!] ssid: %s\r\n[!] passphrase: %s\r\n", posted_ssid,
                   posted_passphrase);
        /* Respond with a failure to the browser */
        response.data           = "{\"status\":\"failed\"}";
        response.data_length    = strlen(response.data);
        response.content_length = response.data_length;
        HTTPSRV_cgi_write(&response);
    }
    else
    {
        char ip[32];
        /* We have successfully connected however the old AP is still running.
         * This session is still active and will try replying to the browser with a success message.
         * This message will also hold the new IP address under which the board will be reachable */
        PRINTF("[i] Successfully joined: %s\r\n", posted_ssid);
        /* Get new client address to be sent back to the old browser session */
        do_get_ip(ip, 1);
        PRINTF(" Now join that network on your device and connect to this IP: %s\r\n", ip);

        snprintf(buffer, sizeof(buffer), "{\"status\":\"success\",\"new_ip\":\"%s\"}", ip);

        response.data           = buffer;
        response.data_length    = strlen(response.data);
        response.content_length = response.data_length;
        HTTPSRV_cgi_write(&response);

        /* Since the Joining was successful, we can save the credentials to the Flash */
        config_set_sta_network();
    }

    return (response.content_length);
}

/* The reset.cgi is used to clear the Flash memory and reset the board back to AP mode */
static int CGI_HandleReset(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response;

    response.ses_handle   = param->ses_handle;
    response.status_code  = HTTPSRV_CODE_OK;
    response.content_type = HTTPSRV_CONTENT_TYPE_PLAIN;
    char str_buffer[64];

    /* Try to clear the flash memory */
    if (config_reset_sta_network() != 0)
    {
        uap_prov_e("[!] Error occured during resetting of saved credentials!");
        response.data        = "{\"status\":\"failed\"}";
        response.data_length = strlen(response.data);
    }
    else
    {
        /* The new ip will be the static ip configured for the local AP */
        snprintf(str_buffer, sizeof(str_buffer), "{\"status\":\"success\",\"new_ip\":\"%s\"}", DEF_UAP_IP_ADDR);

        response.data        = str_buffer;
        response.data_length = strlen(str_buffer);
    }

    response.content_length = response.data_length;
    HTTPSRV_cgi_write(&response);

    return 0;
}

/*CGI*/
/* Example Common Gateway Interface callback. */
/* These callbacks are called from the session tasks according to the Link struct above */
/* The status  status.cgi request returns status */
static int CGI_HandleStatus(HTTPSRV_CGI_REQ_STRUCT *param)
{
    HTTPSRV_CGI_RES_STRUCT response = {0};

    response.ses_handle  = param->ses_handle;
    response.status_code = HTTPSRV_CODE_OK;

    /* Buffer for hodling response JSON data */
    char buffer[256] = {0};

    // Build the response JSON
    snprintf(buffer, sizeof(buffer), "{\"info\":{\"ap\":\"%s\",\"status\":\"%s\"}}", uap_prov_uapcfg.uapSsid,
             uap_prov_get_sm_state_str(uap_prov.wifiState));

    // Send the response back to browser
    response.content_type   = HTTPSRV_CONTENT_TYPE_PLAIN;
    response.data           = buffer;
    response.data_length    = strlen(buffer);
    response.content_length = response.data_length;
    HTTPSRV_cgi_write(&response);

    return (response.content_length);
}

static int config_set_sta_network()
{
    struct wlan_network *sta_nw = NULL;
    int ret                     = -WM_FAIL;

    if (!is_nvm_enabled())
    {
        uap_prov_d("config_set_sta_network: nvm disabled.");
        goto done;
    }

    sta_nw = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (!sta_nw)
    {
        uap_prov_d("config_set_sta_network: sta_nw malloc fail.");
        goto done;
    }

    ret = wlan_get_current_network(sta_nw);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("config_set_sta_network: wlan_get_current_network fail");
        goto done;
    }

    sta_nw->channel = 0;
    ret             = wifi_set_network(sta_nw);

done:
    if (sta_nw)
    {
        OSA_MemoryFree(sta_nw);
        sta_nw = NULL;
    }
    return ret;
}

static int config_reset_sta_network()
{
    struct wlan_network *sta_nw = NULL;
    int ret                     = -WM_FAIL;

    if (!is_nvm_enabled())
    {
        uap_prov_d("config_reset_sta_network: nvm disabled.");
        goto done;
    }

    sta_nw = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (!sta_nw)
    {
        uap_prov_d("config_reset_sta_network: sta_nw malloc fail.");
        goto done;
    }

    sta_nw->role = WLAN_BSS_ROLE_STA;

    ret = wifi_set_network(sta_nw);

done:
    if (sta_nw)
    {
        OSA_MemoryFree(sta_nw);
        sta_nw = NULL;
    }
    return ret;
}

static int config_get_uapcfg()
{
    int ret                                    = WM_SUCCESS;
    char uapSsid[IEEEtypes_SSID_SIZE + 1]      = {0};
    char uapPass[WLAN_PASSWORD_MAX_LENGTH + 1] = {0};
    uint32_t uapSec                            = 0;
    char security_str[PROV_SECURITY_MAX_LEN]   = {0};

    if (!is_nvm_enabled())
    {
        uap_prov_d("config_get_uapcfg: nvm disabled.");
        ret = -WM_FAIL;
        return ret;
    }
    ret = ncp_get_conf("prov", "ssid", uapSsid, sizeof(uapSsid));
    ret += ncp_get_conf("prov", "passphrase", uapPass, sizeof(uapPass));
    ret += ncp_get_conf("prov", "security", security_str, sizeof(security_str));
    if (ret != 0)
    {
        uap_prov_e("config_get_uapcfg: get uapcfg from /etc/prov_config fail: %d\n", ret);
        ret = -WM_FAIL;
        return ret;
    }
    uapSec = atoi(security_str);

    ret = validate_and_copy_uapcfg(uapSsid, uapSec, uapPass);

    return ret;
}

static int config_set_uapcfg(char *ssid, uint32_t sec, char *pass)
{
    int ret                                  = WM_SUCCESS;
    char security_str[PROV_SECURITY_MAX_LEN] = {0};
    uint32_t security                        = 0;
    int i                                    = 0;

    ret = validate_and_copy_uapcfg(ssid, sec, pass);
    if (ret != WM_SUCCESS)
    {
        (void)uap_prov_e("config_set_uapcfg: invalid input.");
        return ret;
    }

    if (!is_nvm_enabled())
    {
        uap_prov_d("config_get_uapcfg: nvm disabled.");
        return ret;
    }
    ret = ncp_set_conf("prov", "ssid", uap_prov_uapcfg.uapSsid);
    ret += ncp_set_conf("prov", "passphrase", uap_prov_uapcfg.uapPass);
    security = uap_prov_uapcfg.uapSec;
    if (security >= 10)
    {
        security_str[i] = '0' + (uap_prov_uapcfg.uapSec / 10);
        i++;
    }
    security_str[i] = '0' + (uap_prov_uapcfg.uapSec % 10);
    ret += ncp_set_conf("prov", "security", security_str);
    if (ret != 0)
    {
        uap_prov_e("config_set_uapcfg: set uapcfg to /etc/prov_config fail: %d\n", ret);
        ret = -WM_FAIL;
        return ret;
    }

    return ret;
}

static int config_reset_uapcfg()
{
    int ret                                    = WM_SUCCESS;
    char uapSsid[IEEEtypes_SSID_SIZE + 1]      = {0};
    char uapPass[WLAN_PASSWORD_MAX_LENGTH + 1] = {0};
    uint32_t uapSec                            = 0;
    uint8_t uap_mac[MLAN_MAC_ADDR_LENGTH]      = {0};

    wlan_get_mac_address(uap_mac);
    snprintf(uapSsid, sizeof(uapSsid), "%s%02X%02X", DEF_UAP_SSID, uap_mac[4], uap_mac[5]);
    strcpy(uapPass, DEF_UAP_PASS);
    uapSec = DEF_UAP_SEC;

    ret = config_set_uapcfg(uapSsid, uapSec, uapPass);

    return ret;
}

static char *uap_prov_get_status_str(uap_prov_status status)
{
    if (status > STATUS_INVALID)
        status = STATUS_INVALID;

    return uap_prov_status_str[status];
}

static char *uap_prov_get_sm_state_str(uap_prov_wifi_states wifi_state)
{
    if (wifi_state > WIFI_STATE_INVALID)
        wifi_state = WIFI_STATE_INVALID;

    return uap_prov_sm_state_str[wifi_state];
}

static int uap_prov_set_status(uap_prov_status new_status)
{
    if (new_status >= STATUS_INVALID)
    {
        uap_prov_e("Invalid status: %u", new_status);
        return -WM_FAIL;
    }

    uap_prov_d("status: %u(%s) => %u(%s)", uap_prov.status, uap_prov_get_status_str(uap_prov.status), new_status,
               uap_prov_get_status_str(new_status));
    uap_prov.status = new_status;

    return WM_SUCCESS;
}

static int uap_prov_set_sm_state(uap_prov_wifi_states new_state)
{
    if (new_state >= WIFI_STATE_INVALID)
    {
        uap_prov_e("Invalid wifiState: %u", new_state);
        return -WM_FAIL;
    }

    uap_prov_d("%u(%s) => %u(%s)", uap_prov.wifiState, uap_prov_get_sm_state_str(uap_prov.wifiState), new_state,
               uap_prov_get_sm_state_str(new_state));
    uap_prov.wifiState = new_state;

    return WM_SUCCESS;
}

static int uap_prov_sm(struct q_message *msg)
{
    int ret             = WM_SUCCESS;
    uint8_t msg_handled = 0;

    if (!msg)
    {
        uap_prov_e("uap_prov_sm: msg null");
        goto done;
    }

    uap_prov_d("=== %u(%s) + msg(%u %u 0x%x)", uap_prov.wifiState, uap_prov_get_sm_state_str(uap_prov.wifiState),
               msg->type, msg->reason, msg->data);

    if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_RESET))
    {
        msg_handled = 1;
        uap_prov_set_status(STATUS_DEACTIVATING);
        wlan_stop_all_networks();
        OSA_TimeDelay(1000);

        send_msg_to_httpsrv_ses(MSG_TYPE_CMD, CMD_REASON_UAP_PROV_RESET, 0);

        ret = SM_EXIT;
        goto done;
    }

    switch (uap_prov.wifiState)
    {
        case WIFI_STATE_IDLE:
            if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_START))
            {
                msg_handled = 1;
                wlan_stop_all_networks();
                OSA_TimeDelay(1000);
                /* When the App starts up, it will first read the mflash to check if any
                 * credentials have been saved from previous runs.
                 * If the mflash is empty, the board starts and AP allowing the user to configure
                 * the desired Wi-Fi network.
                 * Otherwise the stored credentials will be used to connect to the Wi-Fi network.*/
                ret = do_fall_to_sta(NETWORK_FROM_CONFIG);
                if (ret == WM_SUCCESS)
                {
                    uap_prov_set_sm_state(WIFI_STATE_STA_CONNECTING);
                    break;
                }
                ret = do_fall_to_uap();
                if (ret == WM_SUCCESS)
                {
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STARTING);
                }
            }
            break;
        case WIFI_STATE_STA_CONNECTING:
        case WIFI_STATE_UAP_STA_CONNECTING:
            if (msg->type == MSG_TYPE_EVT)
            {
                if (msg->reason == WLAN_REASON_SUCCESS)
                {
                    msg_handled = 1;
                    if (uap_prov.wifiState == WIFI_STATE_STA_CONNECTING)
                    {
                        uap_prov_set_sm_state(WIFI_STATE_STA_CONNECTED);
                        app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_SUCCESS, NULL, 0);
                    }
                    else
                    {
                        uap_prov_set_sm_state(WIFI_STATE_UAP_STA_CONNECTED);
                        ret = do_uap_stop();
                        if (ret == WM_SUCCESS)
                        {
                            uap_prov_set_sm_state(WIFI_STATE_UAP_STA_UAP_STOPPING);
                        }
                        send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_SUCCESS, 0);
                    }
                }
                else if ((msg->reason == WLAN_REASON_CONNECT_FAILED) ||
                         (msg->reason == WLAN_REASON_NETWORK_NOT_FOUND) ||
                         (msg->reason == WLAN_REASON_NETWORK_AUTH_FAILED) ||
                         (msg->reason == WLAN_REASON_ADDRESS_FAILED))
                {
                    msg_handled = 1;
                    ret         = wlan_disconnect();
                    if (ret == WM_SUCCESS)
                    {
                        if (uap_prov.wifiState == WIFI_STATE_STA_CONNECTING)
                        {
                            uap_prov_set_sm_state(WIFI_STATE_STA_DISCONNECTING);
                        }
                        else
                        {
                            uap_prov_set_sm_state(WIFI_STATE_UAP_STA_DISCONNECTING);
                            send_msg_to_httpsrv_ses(MSG_TYPE_EVT, msg->reason, 0);
                        }
                    }
                    else
                    {
                        uap_prov_e("uap_prov_sm: %u wlan_disconnect fail %d", uap_prov.wifiState, ret);
                        wlan_remove_network(DEF_STA_NETWORK_LABEL);
                        if (uap_prov.wifiState == WIFI_STATE_STA_CONNECTING)
                        {
                            ret = do_fall_to_uap();
                            if (ret != WM_SUCCESS)
                            {
                                uap_prov_set_sm_state(WIFI_STATE_IDLE);
                                app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
                            }
                            else
                            {
                                uap_prov_set_sm_state(WIFI_STATE_UAP_STARTING);
                            }
                        }
                        else
                        {
                            uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCAN_DONE);
                            send_msg_to_httpsrv_ses(MSG_TYPE_EVT, msg->reason, 0);
                        }
                    }
                }
            }
            break;
        case WIFI_STATE_STA_DISCONNECTING:
        case WIFI_STATE_UAP_STA_DISCONNECTING:
            if ((msg->type == MSG_TYPE_EVT) && (msg->reason == WLAN_REASON_USER_DISCONNECT))
            {
                msg_handled = 1;
                wlan_remove_network(DEF_STA_NETWORK_LABEL);
                if (uap_prov.wifiState == WIFI_STATE_STA_DISCONNECTING)
                {
                    ret = do_fall_to_uap();
                    if (ret != WM_SUCCESS)
                    {
                        uap_prov_set_sm_state(WIFI_STATE_IDLE);
                        app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
                    }
                    else
                    {
                        uap_prov_set_sm_state(WIFI_STATE_UAP_STARTING);
                    }
                }
                else
                {
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCAN_DONE);
                }
            }
            break;
        case WIFI_STATE_UAP_STARTING:
            if (msg->type == MSG_TYPE_EVT)
            {
                if (msg->reason == WLAN_REASON_UAP_SUCCESS)
                {
                    msg_handled = 1;
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STARTED);
                    app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_SUCCESS, NULL, 0);
                }
                else if (msg->reason == WLAN_REASON_UAP_START_FAILED)
                {
                    msg_handled = 1;
                    uap_prov_set_sm_state(WIFI_STATE_IDLE);
                    app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
                }
            }
            break;
        case WIFI_STATE_UAP_STARTED:
            if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_STA_SCAN))
            {
                msg_handled = 1;
                ret         = wlan_scan(uap_prov_scan_cb);
                if (ret != WM_SUCCESS)
                {
                    send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_SCAN_DONE, ret);
                }
                else
                {
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCANNING);
                }
            }
            break;
        case WIFI_STATE_UAP_STA_SCANNING:
            if ((msg->type == MSG_TYPE_EVT) && (msg->reason == WLAN_REASON_SCAN_DONE))
            {
                msg_handled = 1;
                uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCAN_DONE);
                send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_SCAN_DONE, (int)(msg->data));
            }
            break;
        case WIFI_STATE_UAP_STA_SCAN_DONE:
            if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_STA_SCAN))
            {
                msg_handled = 1;
                ret         = wlan_scan(uap_prov_scan_cb);
                if (ret != WM_SUCCESS)
                {
                    send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_SCAN_DONE, ret);
                }
                else
                {
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCANNING);
                }
            }
            else if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_STA_CONN))
            {
                post_ap_info *ap_info = (post_ap_info *)(msg->data);
                msg_handled           = 1;
                if (!ap_info || !strlen(ap_info->posted_ssid) ||
                    ((strlen(ap_info->posted_pass) != 0) && (strlen(ap_info->posted_pass) < WLAN_PSK_MIN_LENGTH)) ||
                    (strlen(ap_info->posted_pass) >= WLAN_PSK_MAX_LENGTH))
                {
                    uap_prov_e("uap_prov_sm: invalid msg data");
                    send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_CONNECT_FAILED, ret);
                    break;
                }
                strcpy(uap_prov.staSsid, ap_info->posted_ssid);
                strcpy(uap_prov.staPass, ap_info->posted_pass);
                ret = do_fall_to_sta(NETWORK_FROM_HTTP);
                if (ret != WM_SUCCESS)
                {
                    uap_prov_set_sm_state(WIFI_STATE_UAP_STA_SCAN_DONE);
                    send_msg_to_httpsrv_ses(MSG_TYPE_EVT, WLAN_REASON_CONNECT_FAILED, ret);
                    break;
                }
                uap_prov_set_sm_state(WIFI_STATE_UAP_STA_CONNECTING);
            }
            break;
        case WIFI_STATE_UAP_STA_UAP_STOPPING:
            if (msg->type == MSG_TYPE_EVT)
            {
                if (msg->reason == WLAN_REASON_UAP_STOPPED)
                {
                    msg_handled = 1;
                    wlan_remove_network(DEF_UAP_NETWORK_LABEL);
                    uap_prov_set_sm_state(WIFI_STATE_STA_CONNECTED);
                }
                else if (msg->reason == WLAN_REASON_UAP_STOP_FAILED)
                {
                    msg_handled = 1;
                }
            }
            break;
        default:
            break;
    }

done:

    if (!msg_handled)
    {
        uap_prov_d("### msg not handled.");
        if ((msg->type == MSG_TYPE_CMD) && (msg->reason == CMD_REASON_UAP_PROV_START))
        {
            app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
        }
        ret = -WM_FAIL;
    }

    return ret;
}

static void uap_prov_main(void *data)
{
    int ret              = WM_SUCCESS;
    struct q_message msg = {0};
    osa_status_t status;

    while (true)
    {
        status = OSA_MsgQGet((osa_msgq_handle_t)uap_prov_q.queue, &msg, osaWaitForever_c);
        if (status != KOSA_StatusSuccess)
        {
            uap_prov_e("Error: Failed to recv on uap_prov_q.queue: %d", status);
            continue;
        }

        ret = uap_prov_sm(&msg);
        if (ret == SM_EXIT)
        {
            break;
        }
    }

    uap_prov_set_status(STATUS_DEACTIVATED);

    vTaskDelete(NULL);
}

int uap_prov_init(void)
{
    int ret = WM_SUCCESS;
    osa_status_t status;

    wlan_register_uap_prov_deinit_cb(uap_prov_deinit);
    wlan_register_uap_prov_cleanup_cb(uap_prov_cleanup);

    if (uap_prov.status != STATUS_IDLE)
    {
        (void)uap_prov_w("uap_prov_init: uap prov may already inited.");
        return WM_SUCCESS;
    }

    memset(&uap_prov, 0x00, sizeof(uap_prov));

    /* Create uap_prov related queue and thread */
    status = OSA_MsgQCreate((osa_msgq_handle_t)uap_prov_q.queue, MAX_EVENTS, sizeof(struct q_message));
    if (status != KOSA_StatusSuccess)
    {
        uap_prov_e("Error: Failed to create uap_prov_queue: %d", status);
        ret = -WM_FAIL;
        goto done;
    }

    status = OSA_TaskCreate((osa_task_handle_t)uap_prov.thread, OSA_TASK(uap_prov_main), NULL);
    if (status != KOSA_StatusSuccess)
    {
        uap_prov_e("Error: Failed to create uap_prov_thread: %d", status);
        ret = -WM_FAIL;
        goto done;
    }

    /* Create httpsrv related queue and thread */
    status = OSA_MsgQCreate((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue, MAX_EVENTS, sizeof(struct q_message));
    if (status != KOSA_StatusSuccess)
    {
        uap_prov_e("Error: Failed to create httpsrv_ses_queue: %d", status);
        ret = -WM_FAIL;
        goto done;
    }

    ret = http_srv_init();

done:
    if (ret != WM_SUCCESS)
    {
        OSA_MsgQDestroy(uap_prov_q.queue);
        OSA_TaskDestroy(uap_prov.thread);
        OSA_MsgQDestroy(uap_prov_q.httpsrv_ses_queue);
    }
    else
    {
        uap_prov_drain_queue();
        uap_prov_set_status(STATUS_INIT_DONE);
    }

    return ret;
}

int uap_prov_deinit(void)
{
    int ret = WM_SUCCESS;

    if (uap_prov.status == STATUS_IDLE)
    {
        return ret;
    }

    ret = send_msg_to_uap_prov(MSG_TYPE_CMD, CMD_REASON_UAP_PROV_RESET, 0);
    if (ret != WM_SUCCESS)
    {
        (void)uap_prov_e("uap_prov_deinit: send_msg_to_uap_prov fail.");
        return ret;
    }

    while (uap_prov.status != STATUS_DEACTIVATED) /* Wait for task completition.*/
    {
        OSA_TimeDelay(1);
    }

    http_srv_deinit();

    uap_prov_set_status(STATUS_IDLE);

    uap_prov_drain_queue();

    return ret;
}

void uap_prov_drain_queue(void)
{
    struct q_message msg = {0};

    while (OSA_MsgQWaiting((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue))
    {
        memset(&msg, 0, sizeof(msg));
        OSA_MsgQGet((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue, &msg, osaWaitNone_c);
    }

    while (OSA_MsgQWaiting((osa_msgq_handle_t)uap_prov_q.queue))
    {
        memset(&msg, 0, sizeof(msg));
        OSA_MsgQGet((osa_msgq_handle_t)uap_prov_q.queue, &msg, osaWaitNone_c);
    }
}

void uap_prov_cleanup(void)
{
    OSA_MsgQDestroy((osa_msgq_handle_t)uap_prov_q.queue);
    OSA_MsgQDestroy((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue);
}

static int do_get_ip(char *ip, int client)
{
    int ret = WM_SUCCESS;
    struct wlan_ip_config addr;

    if (ip == NULL)
    {
        return -WM_FAIL;
    }

    if (client)
    {
        ret = wlan_get_address(&addr);
    }
    else
    {
        ret = wlan_get_uap_address(&addr);
    }

    if (ret != WM_SUCCESS)
    {
        return ret;
    }

    net_inet_ntoa(addr.ipv4.address, ip);

    return ret;
}

static int do_sta_add_network(network_from nw_from, char *ssid, char *pass, char *label)
{
    int ret = WM_SUCCESS;
    struct wlan_network *sta_network = NULL;
    enum wlan_security_type security = WLAN_SECURITY_NONE;

    if (!label || (strlen(label) == 0) || (strlen(label) >= WLAN_NETWORK_NAME_MAX_LENGTH))
    {
        uap_prov_e("do_sta_add_network: Invalid label.");
        ret = -WM_E_INVAL;
        return ret;
    }
    
    sta_network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (sta_network == NULL)
    {
        wlcm_e("wlan_pscan: fail to malloc memory! \r\n");
        return -WM_FAIL;
    }
    (void)memset(sta_network, 0, sizeof(struct wlan_network));
    
    if (nw_from == NETWORK_FROM_CONFIG)
    {
        if (!is_nvm_enabled())
        {
            uap_prov_d("do_sta_add_network: nvm disabled.");
            ret = -WM_FAIL;
            goto done;
        }
        ret = wifi_get_network(sta_network, WLAN_BSS_ROLE_STA, DEF_STA_NETWORK_LABEL);
        if (ret || (strlen(sta_network->ssid) == 0))
        {
            uap_prov_e("do_sta_add_network: read fail %d or invalid ssid %s.", ret, sta_network->ssid);
            ret = -WM_FAIL;
            goto done;
        }
    }
    else if (nw_from == NETWORK_FROM_HTTP)
    {
        if (!ssid || !pass)
        {
            uap_prov_e("do_sta_add_network: Invalid null param.");
            ret = -WM_E_INVAL;
            goto done;
        }
        if ((strlen(ssid) == 0) || (strlen(ssid) > IEEEtypes_SSID_SIZE))
        {
            uap_prov_e("do_sta_add_network: Invalid ssid.");
            ret = -WM_E_INVAL;
            goto done;
        }
        if (strlen(pass) == 0)
        {
            security = WLAN_SECURITY_NONE;
        }
        else if ((strlen(pass) >= WLAN_PSK_MIN_LENGTH) && (strlen(pass) < WLAN_PSK_MAX_LENGTH))
        {
            security = WLAN_SECURITY_WILDCARD;
        }
        else
        {
            uap_prov_e("do_sta_add_network: Invalid pass.");
            ret = -WM_E_INVAL;
            goto done;
        }
        memset(sta_network->ssid, '\0', sizeof(sta_network->ssid));

        memcpy(sta_network->ssid, (const char *)ssid, strlen(ssid));
        sta_network->ip.ipv4.addr_type = ADDR_TYPE_DHCP;
        sta_network->ssid_specific     = 1;
        sta_network->security.type     = security;

        if (strlen(pass))
        {
            unsigned int count            = 0;
            int i                         = 0;
            struct wifi_scan_result2 *res = NULL;
            ret                           = wifi_get_scan_result_count(&count);
            if (ret != 0)
                count = 0;
            for (i = 0; i < count; i++)
            {
                ret = wifi_get_scan_result(i, &res);
                if (ret == WM_SUCCESS && (memcmp(sta_network->ssid, (char *)res->ssid, strlen(sta_network->ssid)) == 0) &&
                    (res->ssid_len == strlen(sta_network->ssid)))
                {
                    if (res->WPA_WPA2_WEP.wepStatic || res->WPA_WPA2_WEP.wpa ||
                        res->WPA_WPA2_WEP.wpa2 || res->WPA_WPA2_WEP.wpa2_sha256 ||
                        res->WPA_WPA2_WEP.wpa3_sae)
                        break;
                }
            }
            if (i == count)
            {
                uap_prov_e("do_sta_add_network: Could not find a proper AP with secure mode.");
                ret = -WM_FAIL;
                goto done;
            }
            if (res->WPA_WPA2_WEP.wepStatic || res->WPA_WPA2_WEP.wpa || res->WPA_WPA2_WEP.wpa2 || res->WPA_WPA2_WEP.wpa2_sha256)
            {
                sta_network->security.psk_len = strlen(pass);
                strncpy(sta_network->security.psk, pass, strlen(pass));
            }
            if (res->WPA_WPA2_WEP.wpa3_sae)
            {
                sta_network->security.password_len = strlen(pass);
                strncpy(sta_network->security.password, pass, strlen(pass));
            }
#if CONFIG_WPA_SUPP
            if (sta_network->security.type == WLAN_SECURITY_WILDCARD)
            {
                /* Wildcard: If wildcard security is specified, copy the highest
                 * security available in the scan result to the configuration
                 * structure
                 */
                enum wlan_security_type t;
                if ((res->WPA_WPA2_WEP.wpa3_sae != 0U) && (res->WPA_WPA2_WEP.wpa2 != 0U))
                    t = WLAN_SECURITY_WPA2_WPA3_SAE_MIXED;
                else if (res->WPA_WPA2_WEP.wpa3_sae != 0U)
                    t = WLAN_SECURITY_WPA3_SAE;
                else if (res->WPA_WPA2_WEP.wpa2 != 0U)
                    t = WLAN_SECURITY_WPA2;
                /* Delete temporary
                            else if (res->WPA_WPA2_WEP.wpa2_sha256!= 0U)
                                t = WLAN_SECURITY_WPA2_SHA256;
                */
                else if (res->WPA_WPA2_WEP.wpa != 0U)
                    t = WLAN_SECURITY_WPA_WPA2_MIXED;
                else if (res->WPA_WPA2_WEP.wepStatic != 0U)
                    t = WLAN_SECURITY_WEP_OPEN;
#if CONFIG_DRIVER_OWE
                else if (res->WPA_WPA2_WEP.wpa2 && res->WPA_WPA2_WEP.owe)
                    t = WLAN_SECURITY_OWE_ONLY;
#endif
                else
                    t = WLAN_SECURITY_NONE;
                sta_network->security.type = t;
            }
            if (res->wpa_mcstCipher.tkip || res->rsn_mcstCipher.tkip)
                sta_network->security.group_cipher |= BIT(3); /*WPA_CIPHER_TKIP*/
            if (res->wpa_mcstCipher.ccmp || res->rsn_mcstCipher.ccmp)
                sta_network->security.group_cipher |= BIT(4); /*WPA_CIPHER_CCMP*/
            if (res->wpa_ucstCipher.tkip || res->rsn_ucstCipher.tkip)
                sta_network->security.pairwise_cipher |= BIT(3); /*WPA_CIPHER_TKIP*/
            if (res->wpa_ucstCipher.ccmp || res->rsn_ucstCipher.ccmp)
                sta_network->security.pairwise_cipher |= BIT(4); /*WPA_CIPHER_CCMP*/
#endif
        }
    }
    else
    {
        uap_prov_e("do_sta_add_network: Invalid nw_from.");
        ret = -WM_FAIL;
        goto done;
    }

    strcpy(sta_network->name, label);
    wlan_remove_network(label);

    ret = wlan_remove_network(wifi_lfs_bss_config[1].network_name);
    if(is_nvm_enabled())
    {
        ret = wifi_overwrite_network(wifi_lfs_bss_config[1].network_name);
        if (ret != WM_SUCCESS)
        {
            uap_prov_e("do_sta_add_network: remove %s network fail %d.",wifi_lfs_bss_config[1].config_path, ret);
            goto done;
        }
    }

    ret = wlan_add_network(sta_network);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_sta_add_network: add network fail %d.", ret);
        wlan_remove_network(label);
        goto done;
    }
    
    if(is_nvm_enabled())
    {
        ret = wifi_set_network(sta_network);
        if (ret != WM_SUCCESS)
        {
            uap_prov_e("do_sta_add_network: add %s network fail %d.",wifi_lfs_bss_config[1].config_path, ret);
            goto done;
        }
    }
    
done:
    if (sta_network)
    {   
        OSA_MemoryFree((void *)sta_network);
    }

    return ret;
}

static int do_sta_connect(char *label)
{
    int ret = WM_SUCCESS;

    ret = wlan_connect(label);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_sta_connect: connect network fail %d.", ret);
        wlan_remove_network(label);
        return ret;
    }

    return ret;
}

static int do_fall_to_sta(network_from nw_from)
{
    int ret = WM_SUCCESS;

    PRINTF("uap_prov: try sta...\r\n");

    if (nw_from >= NETWORK_FROM_INVALID)
    {
        uap_prov_e("do_fall_to_sta: Invalid nw_from.");
        ret = -WM_FAIL;
        return ret;
    }

    ret = do_sta_add_network(nw_from, uap_prov.staSsid, uap_prov.staPass, DEF_STA_NETWORK_LABEL);
    if (ret != WM_SUCCESS)
    {
        return ret;
    }

    ret = do_sta_connect(DEF_STA_NETWORK_LABEL);

    return ret;
}

static int do_uap_add_network(uap_prov_uap_config *uapcfg, char *label, uint32_t channel)
{
    int ret = WM_SUCCESS;
    struct wlan_network *uap_network = NULL;

    if (!label || (strlen(label) == 0) || (strlen(label) >= WLAN_NETWORK_NAME_MAX_LENGTH))
    {
        uap_prov_e("do_sta_add_network: Invalid label.");
        ret = -WM_E_INVAL;
        return ret;
    }

    if (!uapcfg)
    {
        uap_prov_e("do_uap_add_network: null uapcfg.");
        ret = -WM_E_INVAL;
        return ret;
    }
    ret = check_valid_uapcfg(uapcfg->uapSsid, uapcfg->uapSec, uapcfg->uapPass);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_uap_add_network: Invalid uapcfg.");
        ret = -WM_E_INVAL;
        return ret;
    }
    uap_network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (uap_network == NULL)
    {
        wlcm_e("do_uap_add_network: fail to malloc memory! \r\n");
        return -WM_FAIL;
    }
    
    wlan_initialize_uap_network(uap_network);
    memset(uap_network->name, 0x00, sizeof(uap_network->name));
    strcpy(uap_network->name, label);
    memcpy(uap_network->ssid, uapcfg->uapSsid, strlen(uapcfg->uapSsid));
    uap_network->ip.ipv4.address = ipaddr_addr(DEF_UAP_IP_ADDR);
    uap_network->ip.ipv4.gw      = ipaddr_addr(DEF_UAP_IP_ADDR);
    uap_network->channel         = channel;
    uap_network->security.type   = (enum wlan_security_type)(uapcfg->uapSec);
    if ((uap_network->security.type != WLAN_SECURITY_NONE) && strlen(uapcfg->uapPass))
    {
        if ((uap_network->security.type == WLAN_SECURITY_WPA) || (uap_network->security.type == WLAN_SECURITY_WPA2) ||
            (uap_network->security.type == WLAN_SECURITY_WPA_WPA2_MIXED))
        {
            uap_network->security.psk_len = strlen(uapcfg->uapPass);
            memset(uap_network->security.psk, '\0', sizeof(uap_network->security.psk));
            strncpy(uap_network->security.psk, uapcfg->uapPass, MIN(strlen(uapcfg->uapPass), sizeof(uap_network->security.psk)));
        }
        if ((uap_network->security.type == WLAN_SECURITY_WPA3_SAE) ||
            (uap_network->security.type == WLAN_SECURITY_WPA2_WPA3_SAE_MIXED))
        {
            uap_network->security.password_len = strlen(uapcfg->uapPass);
            strncpy(uap_network->security.password, uapcfg->uapPass, strlen(uapcfg->uapPass));
            uap_network->security.mfpc = 1;
            uap_network->security.mfpr = 1;
        }
    }

    wlan_remove_network(label);
    ret = wlan_add_network(uap_network);
    if (ret != WM_SUCCESS)
    {
        if(ret != -WM_E_NOMEM)
        {
            uap_prov_e("do_uap_add_network: add network fail %d.", ret);
            goto done;
        }
        else
        {
            ret = wlan_remove_network(wifi_lfs_bss_config[0].network_name);
            if(ret != WM_SUCCESS && ret != WM_E_INVAL)
            {
                uap_prov_e("do_uap_add_network: remove network fail %d.", ret);
                goto done;
            }

            ret = wlan_add_network(uap_network);
            if (ret != WM_SUCCESS)
            {
                if(ret != -WM_E_NOMEM)
                {
                    uap_prov_e("do_uap_add_network: add network fail %d.", ret);
                    goto done;
                }
            }
        }
    }
    
done:
    if (uap_network)
    {
        OSA_MemoryFree((void *)uap_network);
    }
     return ret;
}

static int do_uap_start(char *label)
{
    int ret = WM_SUCCESS;

    ret = wlan_start_network(label);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_uap_start: start network fail %d.", ret);
        wlan_remove_network(label);
        return ret;
    }

    return ret;
}

static int do_fall_to_uap()
{
    int ret = WM_SUCCESS;

    PRINTF("uap_prov: try uap...\r\n");

    ret = config_get_uapcfg();
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_fall_to_uap: config_get_uapcfg fail %d, try reset uapcfg.\n", ret);
        config_reset_uapcfg();
    }

    ret = do_uap_add_network(&uap_prov_uapcfg, DEF_UAP_NETWORK_LABEL, DEF_UAP_CH);
    if (ret != WM_SUCCESS)
        return ret;

    ret = do_uap_start(DEF_UAP_NETWORK_LABEL);

    return ret;
}

static int do_uap_stop()
{
    int ret = WM_SUCCESS;

    ret = wlan_stop_network(DEF_UAP_NETWORK_LABEL);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("do_uap_stop: stop network fail %d.", ret);
        return ret;
    }

    return ret;
}

static int uap_prov_scan_cb(unsigned int count)
{
    int ret = WM_SUCCESS;

    send_msg_to_uap_prov(MSG_TYPE_EVT, WLAN_REASON_SCAN_DONE, count);

    return ret;
}

uint8_t check_valid_event_for_uap_prov(uint16_t reason)
{
    if ((reason == WLAN_REASON_SUCCESS) || (reason == WLAN_REASON_CONNECT_FAILED) ||
        (reason == WLAN_REASON_NETWORK_NOT_FOUND) || (reason == WLAN_REASON_NETWORK_AUTH_FAILED) ||
        (reason == WLAN_REASON_ADDRESS_FAILED) || (reason == WLAN_REASON_USER_DISCONNECT) ||
        (reason == WLAN_REASON_UAP_SUCCESS) || (reason == WLAN_REASON_UAP_START_FAILED) ||
        (reason == WLAN_REASON_UAP_STOPPED))
    {
        return 1;
    }
    return 0;
}

uint8_t check_valid_status_for_uap_prov()
{
    if (uap_prov.status == STATUS_INIT_DONE)
    {
        return 1;
    }
    return 0;
}

int send_msg_to_uap_prov(uint16_t type, uint16_t reason, int data)
{
    struct q_message msg = {0};

    msg.type   = type;
    msg.reason = reason;
    msg.data   = (void *)data;

    if ((OSA_MsgQPut((osa_msgq_handle_t)uap_prov_q.queue, &msg) != KOSA_StatusSuccess))
    {
        uap_prov_e("send_msg_to_uap_prov: send msg(%u %u %d) to %p fail", msg.type, msg.reason, msg.data,
                   uap_prov_q.queue);
        return -WM_FAIL;
    }

    uap_prov_d("send_msg_to_uap_prov: send msg(%u %u %d)to uap_prov", msg.type, msg.reason, msg.data);
    return WM_SUCCESS;
}

static int send_msg_to_httpsrv_ses(uint16_t type, uint16_t reason, int data)
{
    struct q_message msg = {0};

    msg.type   = type;
    msg.reason = reason;
    msg.data   = (void *)data;

    if ((OSA_MsgQPut((osa_msgq_handle_t)uap_prov_q.httpsrv_ses_queue, &msg) != KOSA_StatusSuccess))
    {
        uap_prov_e("send_msg_to_httpsrv_ses: send msg(%u %u %d) to %p fail", msg.type, msg.reason, msg.data,
                   uap_prov_q.httpsrv_ses_queue);
        return -WM_FAIL;
    }

    uap_prov_d("send_msg_to_httpsrv_ses: send msg(%u %u %d)", msg.type, msg.reason, msg.data);
    return WM_SUCCESS;
}

int uap_prov_start()
{
    int ret = WM_SUCCESS;

    ret = uap_prov_init();
    if (ret != WM_SUCCESS)
    {
        (void)uap_prov_e("uap_prov_start: uap prov init fail.");
        app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
        return ret;
    }

    ret = send_msg_to_uap_prov(MSG_TYPE_CMD, CMD_REASON_UAP_PROV_START, 0);
    if (ret != WM_SUCCESS)
    {
        (void)uap_prov_e("uap_prov_start: send_msg_to_uap_prov fail.");
        app_notify_event(APP_EVT_UAP_PROV_START, APP_EVT_REASON_FAILURE, NULL, 0);
    }
    return ret;
}

int uap_prov_reset()
{
    int ret = WM_SUCCESS;
    // We don't reset uapcfg for flash has default value and user might also configured
    // config_reset_uapcfg();

    ret = uap_prov_deinit();
    if (ret != WM_SUCCESS)
    {
        (void)uap_prov_e("uap_prov_reset: uap_prov_deinit fail.");
    }

    if(ncp_network_is_added(DEF_STA_NETWORK_LABEL))
    {
        wlan_remove_network(DEF_STA_NETWORK_LABEL);
        ret = wifi_overwrite_network(DEF_STA_NETWORK_LABEL);
        if (ret != WM_SUCCESS)
        {
            (void)uap_prov_e("uap_prov_reset: fail to overwrite lfs %s network.",DEF_STA_NETWORK_LABEL);
        }
    }

    return ret;
}

static void dump_uap_prov_set_uapcfg_usage(void)
{
    (void)PRINTF("Usage: wlan-uap-prov-set-uapcfg <ssid> <security> <password>\r\n");
    (void)PRINTF("Value for <security>: \r\n");
    (void)PRINTF("    %d - WLAN_SECURITY_NONE\r\n", WLAN_SECURITY_NONE);
    (void)PRINTF("    %d - WLAN_SECURITY_WPA\r\n", WLAN_SECURITY_WPA);
    (void)PRINTF("    %d - WLAN_SECURITY_WPA2\r\n", WLAN_SECURITY_WPA2);
    (void)PRINTF("    %d - WLAN_SECURITY_WPA_WPA2_MIXED\r\n", WLAN_SECURITY_WPA_WPA2_MIXED);
    (void)PRINTF("    %d - WLAN_SECURITY_WPA3_SAE\r\n", WLAN_SECURITY_WPA3_SAE);
    (void)PRINTF("    %d - WLAN_SECURITY_WPA2_WPA3_SAE_MIXED\r\n", WLAN_SECURITY_WPA2_WPA3_SAE_MIXED);
    (void)PRINTF("Usage example : \r\n");
    (void)PRINTF("wlan-uap-prov-set-uapcfg nxp_uap\r\n");
    (void)PRINTF("wlan-uap-prov-set-uapcfg nxp_uap 0\r\n");
    (void)PRINTF("wlan-uap-prov-set-uapcfg nxp_uap 4 12345678\r\n");
}

static int check_valid_uapcfg(char *ssid, uint32_t sec, char *pass)
{
    if (!ssid)
    {
        (void)uap_prov_e("check_valid_uapcfg: ssid is null.");
        return -WM_E_INVAL;
    }

    if (!strlen(ssid) || strlen(ssid) > IEEEtypes_SSID_SIZE)
    {
        (void)uap_prov_e("check_valid_uapcfg: Invalid uAP ssid len, expect len<=%d.", IEEEtypes_SSID_SIZE);
        return -WM_E_INVAL;
    }

    if ((sec != WLAN_SECURITY_NONE) && (sec != WLAN_SECURITY_WPA) && (sec != WLAN_SECURITY_WPA2) &&
        (sec != WLAN_SECURITY_WPA_WPA2_MIXED) && (sec != WLAN_SECURITY_WPA3_SAE) &&
        (sec != WLAN_SECURITY_WPA2_WPA3_SAE_MIXED))
    {
        (void)uap_prov_e("check_valid_uapcfg: Invalid security=%d.", sec);
        return -WM_E_INVAL;
    }

    if (sec != WLAN_SECURITY_NONE)
    {
        uint32_t max_pass_len = WLAN_PSK_MAX_LENGTH;
        if ((sec == WLAN_SECURITY_WPA3_SAE) || (sec == WLAN_SECURITY_WPA2_WPA3_SAE_MIXED))
            max_pass_len = WLAN_PASSWORD_MAX_LENGTH + 1;
        if (!pass || ((strlen(pass) < WLAN_PSK_MIN_LENGTH) || (strlen(pass) > (max_pass_len - 1))))
        {
            (void)uap_prov_e("check_valid_uapcfg: security mode %d with invalid passphrase.", sec);
            return -WM_E_INVAL;
        }
    }

    return WM_SUCCESS;
}

static int validate_and_copy_uapcfg(char *ssid, uint32_t sec, char *pass)
{
    int ret = WM_SUCCESS;

    ret = check_valid_uapcfg(ssid, sec, pass);
    if (ret != WM_SUCCESS)
    {
        uap_prov_e("validate_and_copy_uapcfg: check_valid_uapcfg fail: %d\n", ret);
        return ret;
    }

    memset(&uap_prov_uapcfg, 0x00, sizeof(uap_prov_uapcfg));
    if (strlen(ssid))
        strcpy(uap_prov_uapcfg.uapSsid, ssid);
    if ((sec != WLAN_SECURITY_NONE) && pass && strlen(pass))
        strcpy(uap_prov_uapcfg.uapPass, pass);
    uap_prov_uapcfg.uapSec = sec;

    return ret;
}

int uap_prov_set_uapcfg(char *ssid, uint32_t sec, char *pass)
{
    int ret = -WM_FAIL;

    ret = config_set_uapcfg(ssid, sec, pass);

    return ret;
}

static void test_uap_prov_start(int argc, char **argv)
{
    uap_prov_start();
}

static void test_uap_prov_reset(int argc, char **argv)
{
    uap_prov_reset();
}

static void test_uap_prov_set_uapcfg(int argc, char **argv)
{
    int ret      = -WM_FAIL;
    char *ssid   = NULL;
    uint32_t sec = WLAN_SECURITY_NONE;
    char *pass   = NULL;

    if ((argc < 2) || (argc > 4))
    {
        ret = -WM_E_INVAL;
        (void)uap_prov_e("test_uap_prov_setuapcfg: invalid number of arguments");
        goto done;
    }

    if (argc >= 2)
        ssid = argv[1];
    if (argc >= 3)
        sec = atoi(argv[2]);
    if (argc >= 4)
        pass = argv[3];

    ret = uap_prov_set_uapcfg(ssid, sec, pass);

done:
    if (ret == (-WM_E_INVAL))
    {
        dump_uap_prov_set_uapcfg_usage();
    }
}

static struct cli_command uap_prov_cmds[] = {
    {"wlan-uap-prov-start", NULL, test_uap_prov_start},
    {"wlan-uap-prov-reset", NULL, test_uap_prov_reset},
    {"wlan-uap-prov-set-uapcfg", NULL, test_uap_prov_set_uapcfg},
};

int uap_prov_cli_init(void)
{
    int i;

    for (i = 0; i < sizeof(uap_prov_cmds) / sizeof(struct cli_command); i++)
        if (cli_register_command(&uap_prov_cmds[i]) != 0)
            return -WM_FAIL;

    return WM_SUCCESS;
}
