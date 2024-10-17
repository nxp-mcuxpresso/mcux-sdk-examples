/* @file ncp_glue_wifi.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "fsl_debug_console.h"
#include "dhcp-server.h"
#include "ncp_glue_wifi.h"
#include "ncp_config.h"
#include "app_notify.h"

#include "uap_prov.h"
#include "serial_httpc.h"
#include "serial_socket.h"

#include "cli_utils.h"
#include "fsl_rtc.h"
#include "fsl_power.h"
#include "fsl_pm_core.h"
#include "fsl_pm_device.h"
#include "host_sleep.h"
#include "ncp_adapter.h"
#include "ncp_wifi.h"
#include "serial_network.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define WLAN_CAU_ENABLE_ADDR      (0x45004008U)
#define WLAN_CAU_TEMPERATURE_ADDR (0x4500400CU)

#define UNUSED(x) (void)(x)

#define NCP_WLAN_MAX_KNOWN_NETWORKS 5

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern int net_wlan_get_mac_address(unsigned char *mac);
extern int wifi_ncp_send_response(uint8_t *pbuf);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t wifi_res_buf[NCP_INBUF_SIZE];
extern osa_task_handle_t  wlan_suspend_thread;
static uint32_t reg_access_cnt = 0;
extern power_cfg_t global_power_config;
extern int suspend_mode;
extern nw_conn_t nw_handles[MAX_HANDLES];

/*******************************************************************************
 * Code
 ******************************************************************************/

#if CONFIG_NCP_DEBUG
#define NCP_DEBUG_TIME_COUNT 512
#define NCP_DEBUG_TIME_FUNC  128
int ncp_debug_time_num                                             = 0;
unsigned long ncp_debug_time[NCP_DEBUG_TIME_COUNT]                 = {0};
char ncp_debug_time_pos[NCP_DEBUG_TIME_COUNT][NCP_DEBUG_TIME_FUNC] = {0};
extern unsigned int os_get_timestamp(void);
void print_ncp_debug_time(void)
{
    for (int i = 0; i < ncp_debug_time_num; i++)
        (void)PRINTF("%d-%s-%lu\r\n", i, ncp_debug_time_pos[i], ncp_debug_time[i]);
    for (int i = 1; i < ncp_debug_time_num; i++)
        (void)PRINTF("[%d-%lu]\r\n", i, ncp_debug_time[i] - ncp_debug_time[i - 1]);
    ncp_debug_time_num = 0;
}
void add_ncp_debug_time_item(const char *func)
{
    int func_len = strlen(func) + 1 <= NCP_DEBUG_TIME_FUNC ? strlen(func) + 1 : NCP_DEBUG_TIME_FUNC;
    if (ncp_debug_time_num >= NCP_DEBUG_TIME_COUNT)
    {
        (void)PRINTF("the array is full, please increase NCP_DEBUG_TIME_COUNT\r\n");
        return;
    }
    ncp_debug_time[ncp_debug_time_num] = os_get_timestamp();
    memcpy(ncp_debug_time_pos[ncp_debug_time_num++], func, func_len);
}
#endif

/*scan fuctions*/
static int scan_cb(unsigned int count)
{
    struct wlan_scan_result res;
    NCP_CMD_SCAN_NETWORK_INFO *scan_result = NULL;
    int i, j;
    int err;

    scan_result = (NCP_CMD_SCAN_NETWORK_INFO *)OSA_MemoryAllocate(sizeof(NCP_CMD_SCAN_NETWORK_INFO));
    if (scan_result == NULL)
    {
        ncp_e("failed to allocate memory for scan result");
        app_notify_event(APP_EVT_SCAN_RESULT, APP_EVT_REASON_FAILURE, NULL, 0);
        return 0;
    }

    for (i = 0, j = 0; i < count && j < CONFIG_MAX_AP_ENTRIES; i++, j++)
    {
        err = wlan_get_scan_result(i, &res);
        if (err)
        {
            ncp_e("Error: can't get scan res %d", j);
            j--;
            continue;
        }

        memcpy((void *)&scan_result->res[i].ssid[0], (const void *)&res.ssid[0], WLAN_SSID_MAX_LEN);
        memcpy((void *)&scan_result->res[i].bssid[0], (const void *)&res.bssid[0], WLAN_BSSID_NAME_LEN);
        memcpy((void *)&scan_result->res[i].trans_ssid[0], (const void *)&res.trans_ssid[0], WLAN_SSID_MAX_LEN);
        memcpy((void *)&scan_result->res[i].trans_bssid[0], (const void *)&res.trans_bssid[0], WLAN_BSSID_NAME_LEN);
#if CONFIG_NCP_SUPP_WPS
        scan_result->res[i].wps         = res.wps;
        scan_result->res[i].wps_session = res.wps_session;
#endif
        scan_result->res[i].ssid_len       = res.ssid_len;
        scan_result->res[i].channel        = res.channel;
        scan_result->res[i].rssi           = res.rssi;
        scan_result->res[i].dot11n         = res.dot11n;
#if CONFIG_NCP_11AC
        scan_result->res[i].dot11ac        = res.dot11ac;
#endif
#if CONFIG_NCP_11AX
        scan_result->res[i].dot11ax        = res.dot11ax;
#endif
        scan_result->res[i].wmm            = res.wmm;
        scan_result->res[i].wpa2_entp      = res.wpa2_entp;
        scan_result->res[i].wep            = res.wep;
        scan_result->res[i].wpa            = res.wpa;
        scan_result->res[i].wpa2           = res.wpa2;
        scan_result->res[i].wpa3_sae       = res.wpa3_sae;
        scan_result->res[i].trans_ssid_len = res.trans_ssid_len;
        scan_result->res[i].beacon_period  = res.beacon_period;
        scan_result->res[i].dtim_period    = res.dtim_period;
    }
    scan_result->res_cnt = j;

    if (app_notify_event(APP_EVT_SCAN_RESULT, APP_EVT_REASON_SUCCESS, scan_result, sizeof(NCP_CMD_SCAN_NETWORK_INFO)) !=
        WM_SUCCESS)
    {
        /* If fail to send message on queue, free allocated memory ! */
        OSA_MemoryFree((void *)scan_result);
    }
    return 0;
}

/*CMD handle functions*/
static int wlan_ncp_scan(void *tlv)
{
    int ret;
    UNUSED(tlv);

    ret = wlan_scan(scan_cb);
    if (ret != WM_SUCCESS)
    {
        ncp_e("scan failed");
        app_notify_event(APP_EVT_SCAN_RESULT, ret, NULL, 0);
    }
    return ret;
}

static int wlan_ncp_connect(void *tlv)
{
    int ret = -WM_FAIL;

    struct wlan_network *network = NULL;
    char *net_name = (char *)tlv;

    network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (network == NULL)
    {
        ncp_e("failed to allocate memory for network list");
        ret = -WM_FAIL;
        goto done;
    }
    (void)memset(network, 0, sizeof(struct wlan_network));

    /* Check whether nvm is enabled, and if so, read from LittleFS. */
    if (is_nvm_enabled())
    {
        ret = wifi_get_network(network, WLAN_BSS_ROLE_STA, net_name);
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to get station network info from LittleFS");
            goto done;
        }

        ret = wlan_remove_network(network->name);
        switch (ret)
        {
            case WM_SUCCESS:
                ncp_d("Removed existing \"%s\"", network->name);
                break;
            case -WM_E_INVAL:
                ncp_d("\"%s\" network not found", network->name);
                break;
            case WLAN_ERROR_STATE:
                ncp_e("Error: can't remove network in this state");
                goto done;
            default:
                ncp_e("Error: unable to remove network \"%s\"", network->name);
                goto done;
        }

        ret = wlan_add_network(network);
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to add station network");
            goto done;
        }

        ret = wlan_connect(network->name);
    }
    else
    {
        if (tlv == NULL)
        {
            ncp_e("specify a network to connect");
            goto done;
        }
        NCP_CMD_WLAN_CONN *conn = (NCP_CMD_WLAN_CONN *)tlv;
        ret                     = wlan_connect(conn->name);
    }

    if (ret != WM_SUCCESS)
    {
        ncp_e("unable to connect");
        goto done;
    }

    if (network != NULL)
        OSA_MemoryFree(network);

    return WM_SUCCESS;

done:
    app_notify_event(APP_EVT_USER_CONNECT, ret, NULL, 0);
    if (network != NULL)
        OSA_MemoryFree(network);

    return ret;
}

static int wlan_ncp_disconnect(void *tlv)
{
    int ret;

    UNUSED(tlv);

    ret = wlan_disconnect();
    if (ret != WM_SUCCESS)
    {
        ncp_e("unable to disconnect");
        app_notify_event(APP_EVT_USER_DISCONNECT, ret, NULL, 0);
    }
    return ret;
}

static int wlan_ncp_version(void *tlv)
{
    char *version_str;

    UNUSED(tlv);

    version_str = wlan_get_firmware_version_ext();

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_VERSION;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_FW_VERSION *fw_ver = (NCP_CMD_FW_VERSION *)&cmd_res->params.fw_version;
    memcpy(fw_ver->driver_ver_str, WLAN_DRV_VERSION, sizeof(WLAN_DRV_VERSION));
    memcpy(fw_ver->fw_ver_str, version_str, sizeof(fw_ver->fw_ver_str));

    cmd_res->header.size += sizeof(NCP_CMD_FW_VERSION);

    return WM_SUCCESS;
}

static int wlan_ncp_set_mac_address(void *tlv)
{
    NCP_CMD_MAC_ADDRESS *mac_addr = (NCP_CMD_MAC_ADDRESS *)tlv;

    wlan_set_mac_addr(mac_addr->mac_addr);

    wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_SET_MAC, NCP_CMD_RESULT_OK);

    return WM_SUCCESS;
}

static int wlan_ncp_get_mac_address(void *tlv)
{
    uint8_t uap_mac[6], sta_mac[6];

    UNUSED(tlv);

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_GET_MAC;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    if (wlan_get_mac_address(sta_mac) || wlan_get_mac_address_uap(uap_mac))
    {
        cmd_res->header.result     = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        NCP_CMD_GET_MAC_ADDRESS *get_mac = (NCP_CMD_GET_MAC_ADDRESS *)&cmd_res->params.get_mac_addr;
        memcpy(get_mac->uap_mac, uap_mac, sizeof(uap_mac));
        memcpy(get_mac->sta_mac, sta_mac, sizeof(sta_mac));

        cmd_res->header.size += sizeof(NCP_CMD_GET_MAC_ADDRESS);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_stat(void *tlv)
{
    enum wlan_ps_mode ps_mode;
    enum wlan_connection_state uap_state;
    enum wlan_connection_state sta_state;

    UNUSED(tlv);

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_CONNECT_STAT;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    if (wlan_get_ps_mode(&ps_mode) != 0 || wlan_get_connection_state(&sta_state) != 0 ||
        wlan_get_uap_connection_state(&uap_state) != 0)
    {
        cmd_res->header.result     = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        NCP_CMD_CONNECT_STAT *conn_stat = (NCP_CMD_CONNECT_STAT *)&cmd_res->params.conn_stat;
        conn_stat->ps_mode       = ps_mode;
        conn_stat->uap_conn_stat = uap_state;
        conn_stat->sta_conn_stat = sta_state;

        cmd_res->header.size += sizeof(NCP_CMD_CONNECT_STAT);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_roaming(void *tlv)
{
    NCP_CMD_ROAMING *roaming_cmd = (NCP_CMD_ROAMING *)tlv;
    int ret                      = 0;

    ret = wlan_set_roaming(roaming_cmd->enable, roaming_cmd->rssi_threshold);
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_ROAMING, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_ROAMING, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static void wlan_network_info_copy(NCP_WLAN_NETWORK *network_info, struct wlan_network *network)
{
    char *addr_str = NULL;

    memcpy(network_info->name, network->name, sizeof(network->name));
    memcpy(network_info->ssid, network->ssid, sizeof(network->ssid));
    memcpy(network_info->bssid, network->bssid, sizeof(network->bssid));
    network_info->channel  = network->channel;
    network_info->acs_band = network->acs_band;
#if CONFIG_SCAN_WITH_RSSIFILTER
    network_info->rssi_threshold = network->rssi_threshold;
#endif
    network_info->type = network->type;
    network_info->role = network->role;

    network_info->security_type     = network->security.type;
    network_info->security_specific = network->security_specific;

#if CONFIG_WIFI_CAPA
    if (network->role == WLAN_BSS_ROLE_UAP)
    {
        network_info->enable_11ac = wlan_check_11ac_capa(network->channel);
        network_info->enable_11ax = wlan_check_11ax_capa(network->channel);
        network_info->enable_11n  = wlan_check_11n_capa(network->channel);
        network_info->wlan_capa   = network->wlan_capa;
    }
#endif

    network_info->is_sta_ipv4_connected = is_sta_ipv4_connected();
    if (!(network->role == WLAN_BSS_ROLE_STA && !network_info->is_sta_ipv4_connected))
    {
        network_info->ipv4.addr_type = network->ip.ipv4.addr_type;
        network_info->ipv4.address   = network->ip.ipv4.address;
        network_info->ipv4.gw        = network->ip.ipv4.gw;
        network_info->ipv4.netmask   = network->ip.ipv4.netmask;
        network_info->ipv4.dns1      = network->ip.ipv4.dns1;
        network_info->ipv4.dns2      = network->ip.ipv4.dns2;
    }
#if CONFIG_IPV6
    if (network->role == WLAN_BSS_ROLE_STA || network->role == WLAN_BSS_ROLE_UAP)
    {
        for (int i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
        {
            memcpy(network_info->ipv6[i].address, network->ip.ipv6[i].address, sizeof(network->ip.ipv6[i].address));
            addr_str = ipv6_addr_type_to_desc((struct net_ipv6_config *)&network->ip.ipv6[i]);
            memcpy(network_info->ipv6[i].addr_type_str, addr_str, strlen(addr_str));
            addr_str = ipv6_addr_state_to_desc(network->ip.ipv6[i].addr_state);
            memcpy(network_info->ipv6[i].addr_state_str, addr_str, strlen(addr_str));
        }
    }
#endif
#if CONFIG_SCAN_WITH_RSSIFILTER
    network_info->rssi_threshold = network->rssi_threshold;
#endif
}

static int wlan_ncp_info(void *tlv)
{
    int ret = WM_SUCCESS;
    enum wlan_connection_state state;
    struct wlan_network *sta_network = NULL;
    struct wlan_network *uap_network = NULL;
    int sta_found                    = 0;

    NCPCmd_DS_COMMAND *cmd_res       = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd              = NCP_RSP_WLAN_NETWORK_INFO;
    cmd_res->header.size             = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum           = 0x00;
    cmd_res->header.result           = NCP_CMD_RESULT_OK;

    sta_network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (sta_network == NULL)
    {
        ncp_e("failed to allocate memory for STA network");
        ret = -WM_FAIL;
        goto done;
    }
    uap_network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (uap_network == NULL)
    {
        ncp_e("failed to allocate memory for uAP network");
        ret = -WM_FAIL;
        goto done;
    }

    NCP_CMD_NETWORK_INFO *network_info = (NCP_CMD_NETWORK_INFO *)&cmd_res->params.network_info;

    if (wlan_get_connection_state(&state) != 0)
    {
        network_info->sta_conn_stat = WLAN_DISCONNECTED;
        ncp_e("unable to get STA connection state");
    }
    else
    {
        switch (state)
        {
            case WLAN_CONNECTED:
                if (!wlan_get_current_network(sta_network))
                {
                    ncp_d("Station connected");
                    wlan_network_info_copy(&network_info->sta_network, sta_network);
                    network_info->sta_conn_stat = state;
                    sta_found                   = 1;
                }
                else
                {
                    network_info->sta_conn_stat = WLAN_DISCONNECTED;
                    ncp_d("Station not connected");
                }
                break;
            default:
                network_info->sta_conn_stat = state;
                ncp_d("Station not connected");
                break;
        }
    }

    if (wlan_get_current_uap_network(uap_network) != WM_SUCCESS)
    {
        network_info->uap_conn_stat = WLAN_UAP_STOPPED;
        ncp_d("uAP not started");
    }
    else
    {
        /* Since uAP automatically changes the channel to the one that
         * STA is on */
        if (sta_found == 1)
            uap_network->channel = sta_network->channel;
        if (uap_network->role == WLAN_BSS_ROLE_UAP)
            ncp_d("uAP started");

        wlan_network_info_copy(&network_info->uap_network, uap_network);
        network_info->uap_conn_stat = WLAN_UAP_STARTED;
    }

done:
    if (sta_network != NULL)
        OSA_MemoryFree(sta_network);
    if (uap_network != NULL)
        OSA_MemoryFree(uap_network);

    if (ret == -WM_FAIL)
    {
        cmd_res->header.result   = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        cmd_res->header.size     += sizeof(NCP_CMD_NETWORK_INFO);
    }

    return ret;
}

static int wlan_ncp_address(void *tlv)
{
    int ret = WM_SUCCESS;
    enum wlan_connection_state state;
    struct wlan_network *sta_network = NULL;

    NCPCmd_DS_COMMAND *cmd_res       = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd              = NCP_RSP_WLAN_NETWORK_ADDRESS;
    cmd_res->header.size             = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum           = 0x00;
    cmd_res->header.result           = NCP_CMD_RESULT_OK;

    sta_network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (sta_network == NULL)
    {
        ncp_e("failed to allocate memory for STA network");
        ret = -WM_FAIL;
        goto done;
    }

    NCP_CMD_NETWORK_ADDRESS *network_address = (NCP_CMD_NETWORK_ADDRESS *)&cmd_res->params.network_address;

    if (wlan_get_connection_state(&state) != 0)
    {
        network_address->sta_conn_stat = WLAN_DISCONNECTED;
        ncp_e("unable to get STA connection state");
    }
    else
    {
        switch (state)
        {
            case WLAN_CONNECTED:
                if (!wlan_get_current_network(sta_network))
                {
                    ncp_d("Station connected");
                    wlan_network_info_copy(&network_address->sta_network, sta_network);
                    network_address->sta_conn_stat = state;
                }
                else
                {
                    network_address->sta_conn_stat = WLAN_DISCONNECTED;
                    ncp_d("Station not connected");
                }
                break;
            default:
                network_address->sta_conn_stat = state;
                ncp_d("Station not connected");
                break;
        }
    }

done:
    if (sta_network != NULL)
        OSA_MemoryFree(sta_network);

    if (ret == -WM_FAIL)
    {
        cmd_res->header.result   = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        cmd_res->header.size     += sizeof(NCP_CMD_NETWORK_ADDRESS);
    }


    return ret;
}

static int wlan_ncp_network_list(void *tlv)
{
    struct wlan_network *network = NULL;
    int ret                      = WM_SUCCESS;
    int index                    = 0;
    unsigned int count;

    NCPCmd_DS_COMMAND *cmd_res   = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd          = NCP_RSP_WLAN_NETWORK_LIST;
    cmd_res->header.size         = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum       = 0x00;
    cmd_res->header.result       = NCP_CMD_RESULT_OK;

    if (wlan_get_network_count(&count) != 0)
    {
        ret = -WM_FAIL;
        goto done;
    }

    network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (network == NULL)
    {
        ncp_e("failed to allocate memory for network list");
        ret = -WM_FAIL;
        goto done;
    }

    NCP_CMD_NETWORK_LIST *network_list = (NCP_CMD_NETWORK_LIST *)&cmd_res->params.network_list;
    network_list->count = count & 0xff;

    for (int i = 0; i < NCP_WLAN_MAX_KNOWN_NETWORKS; i++)
    {
        if (wlan_get_network(i, network) == WM_SUCCESS)
            wlan_network_info_copy(&network_list->net_list[index++], network);
    }
done:
    if (network != NULL)
        OSA_MemoryFree(network);
    
    if(ret == -WM_FAIL)
    {
        cmd_res->header.result       = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        cmd_res->header.size         += sizeof(NCP_CMD_NETWORK_LIST);
    }

    return ret;
}

static int wlan_ncp_network_remove(void *tlv)
{
    int state;
    char name[WLAN_NETWORK_NAME_MAX_LENGTH + 1] = {0};

    NCP_CMD_NETWORK_REMOVE *network_remove = (NCP_CMD_NETWORK_REMOVE *)tlv;

    (void)memcpy(name, network_remove->name, strlen(network_remove->name));

    state = wlan_remove_network(name);

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    network_remove             = (NCP_CMD_NETWORK_REMOVE *)&cmd_res->params.network_remove;

    cmd_res->header.cmd      = NCP_RSP_WLAN_NETWORK_REMOVE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum   = 0x00;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;

    cmd_res->header.size     += sizeof(NCP_CMD_NETWORK_REMOVE);

    network_remove->remove_state = state;
    (void)memcpy(network_remove->name, name, strlen(name));
    network_remove->name[strlen(name)] = '\0';

    /* Check whether overwrite this network in LittleFS.*/
    if (is_nvm_enabled())
    {
        if(state == WM_SUCCESS)
        {
            state= wifi_overwrite_network(network_remove->name);
            if(state != WM_SUCCESS)
            {
                flash_log_e("Failed to set overwrite flag: %s", name);
            }
        }
    }

    return WM_SUCCESS;
}

static int wlan_ncp_add(void *tlv)
{
    struct wlan_network *network = NULL;
    int ret = 0, tlv_buf_len = 0;
    uint8_t *ptlv_pos                  = NULL;
    NCP_TLV_HEADER *ptlv_header = NULL;

    NCP_CMD_NETWORK_ADD *network_cmd = (NCP_CMD_NETWORK_ADD *)tlv;

    struct
    {
        unsigned ssid : 1;
        unsigned bssid : 1;
        unsigned channel : 1;
        unsigned address : 2;
        unsigned security : 1;
        unsigned security2 : 1;
        unsigned security3 : 1;
        unsigned role : 1;
        unsigned mfpc : 1;
        unsigned mfpr : 1;
#if CONFIG_EAP_TLS
        unsigned aid : 1;
        unsigned key_passwd : 1;
#endif
        unsigned pwe: 1;
        unsigned tr : 1;
#if CONFIG_WIFI_DTIM_PERIOD
        unsigned dtim : 1;
#endif
        unsigned acs_band : 1;
#if CONFIG_WIFI_CAPA
        unsigned wlan_capa : 1;
#endif
    } info;

    (void)memset(&info, 0, sizeof(info));

    network = (struct wlan_network *)OSA_MemoryAllocate(sizeof(struct wlan_network));
    if (network == NULL)
    {
        ncp_e("failed to allocate memory for network list");
        ret = -WM_FAIL;
        goto done;
    }

    (void)memset(network, 0, sizeof(struct wlan_network));

    if(!memcmp(network_cmd->name, DEF_STA_NETWORK_LABEL, sizeof(DEF_STA_NETWORK_LABEL)))
    {
        ncp_e("'%s' is default uap_prov sta name, pls use another name!",DEF_STA_NETWORK_LABEL);
        ret = -WM_FAIL;
        goto done;
    }
    memcpy(network->name, network_cmd->name, sizeof(network_cmd->name));

    info.address = ADDR_TYPE_DHCP;

    ptlv_pos    = network_cmd->tlv_buf;
    tlv_buf_len = network_cmd->tlv_buf_len;

    do
    {
        ptlv_header = (NCP_TLV_HEADER *)ptlv_pos;

        switch (ptlv_header->type)
        {
            case NCP_CMD_NETWORK_SSID_TLV:
                if (!info.ssid)
                {
                    SSID_ParamSet_t *ssid_tlv = (SSID_ParamSet_t *)ptlv_pos;
                    memcpy(network->ssid, ssid_tlv->ssid, sizeof(ssid_tlv->ssid));
                    info.ssid = 1;
                }
                break;
            case NCP_CMD_NETWORK_BSSID_TLV:
                if (!info.bssid)
                {
                    BSSID_ParamSet_t *bssid_tlv = (BSSID_ParamSet_t *)ptlv_pos;
                    memcpy(network->bssid, bssid_tlv->bssid, sizeof(bssid_tlv->bssid));
                    info.bssid = 1;
                }
                break;
            case NCP_CMD_NETWORK_CHANNEL_TLV:
                if (!info.channel)
                {
                    Channel_ParamSet_t *channel_tlv = (Channel_ParamSet_t *)ptlv_pos;
                    network->channel                = channel_tlv->channel;
                    info.channel                    = 1;
                }
                break;
             case NCP_CMD_NETWORK_PWE_TLV:
                if (!info.pwe)
                {
                    Pwe_Derivation_ParamSet_t *pwe_tlv = (Pwe_Derivation_ParamSet_t *)ptlv_pos;
                    network->security.pwe_derivation   = pwe_tlv->pwe_derivation;
                    info.pwe                    = 1;
                }
                break;
             case NCP_CMD_NETWORK_TR_TLV:
                if (!info.tr)
                {
                    Tr_Disable_ParamSet_t *tr_tlv = (Tr_Disable_ParamSet_t *)ptlv_pos;
                    network->security.transition_disable   = tr_tlv->transition_disable;
                    info.tr                    = 1;
                }
                break;
            case NCP_CMD_NETWORK_IP_TLV:
            {
                IP_ParamSet_t *ip_tlv = (IP_ParamSet_t *)ptlv_pos;
                if (ip_tlv->is_autoip)
                    info.address = ADDR_TYPE_LLA;
                else
                {
                    network->ip.ipv4.address = ip_tlv->address;
                    network->ip.ipv4.gw      = ip_tlv->gateway;
                    network->ip.ipv4.netmask = ip_tlv->netmask;
                    network->ip.ipv4.dns1    = ip_tlv->dns1;
                    network->ip.ipv4.dns2    = ip_tlv->dns2;
                    info.address             = ADDR_TYPE_STATIC;
                }
            }
            break;
            case NCP_CMD_NETWORK_SECURITY_TLV:
            {
                Security_ParamSet_t *security_tlv = (Security_ParamSet_t *)ptlv_pos;

                switch (security_tlv->type)
                {
                    case WLAN_SECURITY_WPA:
                        if (!info.security)
                        {
                            network->security.type = WLAN_SECURITY_WPA;
                            /** copy the PSK phrase */
                            network->security.psk_len = security_tlv->password_len;
                            if (network->security.psk_len < sizeof(network->security.psk))
                                strncpy(network->security.psk, security_tlv->password, security_tlv->password_len);
                            else
                                ncp_e("invalid WPA security argument");
                            info.security++;
                        }
                        break;
                    case WLAN_SECURITY_WPA2:
                        if (!info.security2)
                        {
                            network->security.type = WLAN_SECURITY_WPA2;
                            /** copy the PSK phrase */
                            network->security.psk_len = security_tlv->password_len;
                            if (network->security.psk_len < sizeof(network->security.psk))
                                strncpy(network->security.psk, security_tlv->password, security_tlv->password_len);
                            else
                                ncp_e("invalid WPA2 security argument");
                            info.security2++;
                        }
                        break;
                    case WLAN_SECURITY_WPA3_SAE:
                        if (!info.security3)
                        {
                            network->security.type = WLAN_SECURITY_WPA3_SAE;
                            /** copy the PSK phrase */
                            network->security.password_len = security_tlv->password_len;
                            if (network->security.password_len < sizeof(network->security.password))
                                strncpy(network->security.password, security_tlv->password, security_tlv->password_len);
                            else
                                ncp_e("invalid WPA3 security argument");
                            info.security3++;
                        }
                        break;
#if CONFIG_EAP_TLS
                    case WLAN_SECURITY_EAP_TLS:
                        if (!info.security2)
                        {
                            network->security.type = WLAN_SECURITY_EAP_TLS;
                            info.security2++;
                        }
                        break;
#endif
#if CONFIG_WPA2_ENTP
                    case WLAN_SECURITY_EAP_TLS:
                        if (!info.security2)
                        {
                            u8_t *data   = NULL;
                            int data_len = 0;

                            network->security.type = WLAN_SECURITY_EAP_TLS;
                            /** Set Client Identity */
                            strcpy(network->identity, (const char *)"client1");
                            /** Set SSID specific network search */
                            network->ssid_specific = 1;
                            /** Specify CA certificate */
                            data_len = wlan_get_entp_cert_files(FILE_TYPE_ENTP_CA_CERT, &data);
                            if (data_len == 0)
                            {
                                wlan_free_entp_cert_files();
                                ncp_e("invalid ca cert file");
                                break;
                            }
                            network->security.tls_cert.ca_chain =
                                wm_mbedtls_parse_cert((const unsigned char *)data, (size_t)data_len);
                            /** Specify Client certificate */
                            data_len = wlan_get_entp_cert_files(FILE_TYPE_ENTP_CLIENT_CERT, &data);
                            if (data_len == 0)
                            {
                                wlan_free_entp_cert_files();
                                ncp_e("invalid client cert file");
                                break;
                            }
                            network->security.tls_cert.own_cert =
                                wm_mbedtls_parse_cert((const unsigned char *)data, (size_t)data_len);
                            /** Specify Client key */
                            data_len = wlan_get_entp_cert_files(FILE_TYPE_ENTP_CLIENT_KEY, &data);
                            if (data_len == 0)
                            {
                                wlan_free_entp_cert_files();
                                ncp_e("invalid client key file");
                                break;
                            }
                            network->security.tls_cert.own_key =
                                wm_mbedtls_parse_key((const unsigned char *)data, (size_t)data_len, NULL, 0);
                            /** Specify address type as DHCP */
                            wlan_free_entp_cert_files();
                            info.security2++;
                        }
                        break;
#endif
                    default:
                        break;
                }
            }
            break;
            case NCP_CMD_NETWORK_PMF_TLV:
            {
                PMF_ParamSet_t *pmf_tlv = (PMF_ParamSet_t *)ptlv_pos;
                if (!info.mfpr)
                {
                    network->security.mfpr = pmf_tlv->mfpr;
                    info.mfpr++;
                }

                if (!info.mfpc)
                {
                    network->security.mfpc = pmf_tlv->mfpc;
                    info.mfpc++;
                }
            }
            break;
#if CONFIG_EAP_TLS
            case NCP_CMD_NETWORK_EAP_TLV:
            {
                EAP_ParamSet_t *eap_tlv = (EAP_ParamSet_t *)ptlv_pos;
                if (!info.aid)
                {
                    /* Set Client Anonymous Identity */
                    strcpy(network->security.anonymous_identity, eap_tlv->anonymous_identity);
                    info.aid++;
                }
                if (!info.key_passwd)
                {
                    /* Set Client/Server Key password */
                    strcpy(network->security.client_key_passwd, eap_tlv->client_key_passwd);
                    info.key_passwd++;
                }
            }
            break;
#endif
            case NCP_CMD_NETWORK_ROLE_TLV:
                if (!info.role)
                {
                    BSSRole_ParamSet_t *role_tlv = (BSSRole_ParamSet_t *)ptlv_pos;
                    network->role                = (enum wlan_bss_role)role_tlv->role;
                    info.role                    = 1;
                }
                break;
#if CONFIG_NCP_WIFI_DTIM_PERIOD
            case NCP_CMD_NETWORK_DTIM_TLV:
                if (!info.dtim)
                {
                    DTIM_ParamSet_t *dtim_tlv = (DTIM_ParamSet_t *)ptlv_pos;
                    network->dtim_period      = dtim_tlv->dtim_period;
                    info.dtim                 = 1;
                }
                break;
#endif
            case NCP_CMD_NETWORK_ACSBAND_TLV:
                if (!info.acs_band)
                {
                    ACSBand_ParamSet_t *acs_band_tlv = (ACSBand_ParamSet_t *)ptlv_pos;
                    network->acs_band                = acs_band_tlv->acs_band;
                    info.acs_band                    = 1;
                }
                break;
#if CONFIG_NCP_WIFI_CAPA
            case NCP_CMD_NETWORK_CAPA_TLV:
                if (!info.wlan_capa && network->role == WLAN_BSS_ROLE_UAP)
                {
                    CAPA_ParamSet_t *capa_tlv = (CAPA_ParamSet_t *)ptlv_pos;
                    network->wlan_capa        = capa_tlv->capa;
                    info.wlan_capa            = 1;
                }
                break;
#endif
            default:
                break;
        }

        ptlv_pos += NCP_TLV_HEADER_LEN + ptlv_header->size;
        tlv_buf_len -= NCP_TLV_HEADER_LEN + ptlv_header->size;
    } while (tlv_buf_len > 0);

    if ((network->security.type == WLAN_SECURITY_WPA) || (network->security.type == WLAN_SECURITY_WPA2))
    {
        if (network->security.psk_len && info.security && info.security2)
            network->security.type = WLAN_SECURITY_WPA_WPA2_MIXED;
    }

    if ((network->security.type == WLAN_SECURITY_WPA2) || (network->security.type == WLAN_SECURITY_WPA3_SAE))
    {
        if (network->security.psk_len && network->security.password_len)
            network->security.type = WLAN_SECURITY_WPA2_WPA3_SAE_MIXED;
    }

    network->ip.ipv4.addr_type         = (enum address_types)info.address;
    network->ssid[IEEEtypes_SSID_SIZE] = '\0';
    ret                                = wlan_add_network(network);
    /* Check whether it needs to be stored in LittleFS.*/
    if (is_nvm_enabled())
    {
        if(ret == WM_SUCCESS)
        {
            ret = wifi_set_network(network);
            if (ret != WM_SUCCESS)
            {
                ncp_e("failed to save wlan network info!");
            }
        }
    }

done:
    wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_ADD, ret);

    if (network != NULL)
        OSA_MemoryFree(network);

    return ret;
}

static int wlan_ncp_start_network(void *tlv)
{
    int ret = -WM_FAIL;
    char *net_name = (char *)tlv;

    /* Check whether nvm is enabled, and if so, read from LittleFS. */
    if (is_nvm_enabled())
    {
        struct wlan_network network;

        (void)memset(&network, 0, sizeof(struct wlan_network));

        ret = wifi_get_network(&network, WLAN_BSS_ROLE_UAP, net_name);
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to get uap network info from LittleFS");
            goto done;
        }

        ret = wlan_remove_network(network.name);
        switch (ret)
        {
            case WM_SUCCESS:
                ncp_d("Removed existing \"%s\"", network.name);
                break;
            case -WM_E_INVAL:
                ncp_d("\"%s\" network not found", network.name);
                break;
            case WLAN_ERROR_STATE:
                ncp_e("Error: can't remove network in this state");
                goto done;
            default:
                ncp_e("Error: unable to remove network \"%s\"", network.name);
                goto done;
        }

        ret = wlan_add_network(&network);
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to add uap network");
            goto done;
        }
        ret = wlan_start_network(network.name);
    }
    else
    {
        if (tlv == NULL)
        {
            ncp_e("specify a network to connect");
            goto done;
        }
        NCP_CMD_NETWORK_START *network_start = (NCP_CMD_NETWORK_START *)tlv;

        ret = wlan_start_network(network_start->name);
    }

    if (ret != WM_SUCCESS)
    {
        ncp_e("unable to start network");
        goto done;
    }

    return WM_SUCCESS;

done:
    app_notify_event(APP_EVT_USER_START_NETWORK, ret, NULL, 0);

    return ret;
}

static int wlan_ncp_stop_network(void *tlv)
{
    int ret = -WM_FAIL;
    struct wlan_network network;
    (void)memset((void *)&network, 0, sizeof(network));

    wlan_get_current_uap_network(&network);
    ret = wlan_stop_network(network.name);

    if (ret != WM_SUCCESS)
    {
        ncp_e("stop network failed");
        app_notify_event(APP_EVT_USER_STOP_NETWORK, ret, NULL, 0);
    }
    return ret;
}

static int wlan_ncp_get_uap_sta_list(void *tlv)
{
    int ret = WM_SUCCESS;
    int i;
    wifi_sta_list_t *sl = NULL;

    NCPCmd_DS_COMMAND *cmd_res      = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd             = NCP_RSP_WLAN_NETWORK_GET_UAP_STA_LIST;
    cmd_res->header.size            = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum          = 0x00;
    cmd_res->header.result          = NCP_CMD_RESULT_OK;

    wifi_uap_bss_sta_list(&sl);

    if (!sl)
    {
        ncp_e("Failed to get sta list\n\r");
        ret = -WM_FAIL;
        goto done;
    }

    wifi_sta_info_t *si = (wifi_sta_info_t *)(&sl->count + 1);

    NCP_CMD_NETWORK_UAP_STA_LIST *uap_sta_list = (NCP_CMD_NETWORK_UAP_STA_LIST *)&cmd_res->params.uap_sta_list;

    uap_sta_list->sta_count = sl->count;
    for (i = 0; i < sl->count; i++)
    {
        (void)memcpy(uap_sta_list->info[i].mac, si[i].mac, MLAN_MAC_ADDR_LENGTH);
        uap_sta_list->info[i].power_mgmt_status = si[i].power_mgmt_status;
        uap_sta_list->info[i].rssi              = si[i].rssi;
    }

done:
    if(sl)
        OSA_MemoryFree(sl);

    if (ret == -WM_FAIL)
    {
        cmd_res->header.result          = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        cmd_res->header.size            += sizeof(NCP_CMD_NETWORK_UAP_STA_LIST);
    }

    return WM_SUCCESS;
}

/** Prepare TLV command response */
// extern os_mutex_t resp_buf_mutex;
int wlan_ncp_prepare_status(uint32_t cmd, uint16_t result)
{
    // os_mutex_get(&resp_buf_mutex, OS_WAIT_FOREVER);
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = cmd;

    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum   = 0x00;
    cmd_res->header.result   = result;
    // os_mutex_put(&resp_buf_mutex);
    return WM_SUCCESS;
}

uint8_t *wlan_ncp_evt_status(uint32_t evt_id, void *msg)
{
    uint8_t *event_buf        = NULL;
    app_notify_msg_t *message = (app_notify_msg_t *)msg;
    int total_len             = 0;

    total_len = message->data_len + NCP_CMD_HEADER_LEN;
    event_buf = (uint8_t *)OSA_MemoryAllocate(total_len);
    if (event_buf == NULL)
    {
        ncp_e("failed to allocate memory for event");
        return NULL;
    }

    NCP_COMMAND *evt_hdr = (NCP_COMMAND *)event_buf;
    evt_hdr->cmd                = evt_id;
    evt_hdr->size               = total_len;
    evt_hdr->seqnum             = 0x00;
    evt_hdr->result             = message->reason;
    if (message->data_len)
        memcpy(event_buf + NCP_CMD_HEADER_LEN, message->data, message->data_len);

    return event_buf;
}

int wlan_ncp_prepare_scan_result(NCP_CMD_SCAN_NETWORK_INFO *scan_res)
{
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_SCAN;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_SCAN_NETWORK_INFO *scan_result = (NCP_CMD_SCAN_NETWORK_INFO *)&cmd_res->params.scan_network_info;

    scan_result->res_cnt = scan_res->res_cnt;
    memcpy(scan_result->res, scan_res->res, sizeof(NCP_WLAN_SCAN_RESULT) * scan_res->res_cnt);

    cmd_res->header.size += scan_result->res_cnt * sizeof(NCP_WLAN_SCAN_RESULT) + sizeof(scan_result->res_cnt);

    return WM_SUCCESS;
}

int wlan_ncp_prepare_connect_result(NCP_CMD_WLAN_CONN *conn_res)
{
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_CONNECT;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_WLAN_CONN *conn_result = (NCP_CMD_WLAN_CONN *)&cmd_res->params.wlan_connect;

    conn_result->ip = conn_res->ip;
    memcpy(conn_result->ssid, conn_res->ssid, sizeof(conn_res->ssid));

    cmd_res->header.size += sizeof(NCP_CMD_WLAN_CONN);

    return WM_SUCCESS;
}

int wlan_ncp_prepare_start_network_result(NCP_CMD_NETWORK_START *start_res)
{
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_NETWORK_START;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_NETWORK_START *start_result = (NCP_CMD_NETWORK_START *)&cmd_res->params.network_start;

    (void)memcpy(start_result->ssid, start_res->ssid, sizeof(start_res->ssid));

    cmd_res->header.size += sizeof(NCP_CMD_NETWORK_START);

    return WM_SUCCESS;
}

int wlan_ncp_prepare_mac_address(void *mac_addr, uint8_t bss_type)
{
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_SET_MAC;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_MAC_ADDRESS *mac_address = (NCP_CMD_MAC_ADDRESS *)&cmd_res->params.mac_addr;
    memcpy(mac_address->mac_addr, (uint8_t *)mac_addr, sizeof(mac_address->mac_addr));

    cmd_res->header.size += sizeof(NCP_CMD_MAC_ADDRESS);

    return WM_SUCCESS;
}

static int wlan_ncp_monitor(void *tlv)
{
    int ret;
    NCP_CMD_NET_MONITOR *monitor_cfg = (NCP_CMD_NET_MONITOR *)tlv;

    ret = wlan_net_monitor_cfg((wlan_net_monitor_t *)&monitor_cfg->monitor_para);
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_MONITOR, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_MONITOR, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int ncp_csi_data_recv_user(void *buffer, size_t data_len)
{
    if (app_notify_event(APP_EVT_CSI_DATA, APP_EVT_REASON_SUCCESS, buffer, data_len) !=
        WM_SUCCESS)
    {
        return WM_FAIL;
    }
    PRINTF("CSI user callback: Event CSI data\r\n");
    dump_hex(buffer, data_len);
    return WM_SUCCESS;
}

static int wlan_ncp_csi(void *tlv)
{
    int ret;
    NCP_CMD_CSI *csi_cfg = (NCP_CMD_CSI *)tlv;
    if(csi_cfg->csi_para.csi_enable == 1)
    {
        ret = wlan_register_csi_user_callback(ncp_csi_data_recv_user);
        if (ret != WM_SUCCESS)
        {
            PRINTF("Error during register csi user callback\r\n");
        }
    }

    ret = wlan_csi_cfg(&csi_cfg->csi_para);
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_CSI, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_CSI, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_get_signal(void *tlv)
{
    int ret = WM_SUCCESS;
    wlan_rssi_info_t signal;

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_SIGNAL;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    if (!is_sta_connected())
    {
        ret = -WM_FAIL;
        goto done;
    }

    ret = wlan_get_signal_info(&signal);
    if (ret != WM_SUCCESS)
    {
        ret = -WM_FAIL;
        goto done;
    }

    NCP_CMD_RSSI *signal_rssi = (NCP_CMD_RSSI *)&cmd_res->params.signal_rssi;

    (void)memcpy(&signal_rssi->rssi_info, &signal, sizeof(signal));

done:
    if(ret == -WM_FAIL)
    {
        cmd_res->header.result     = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        cmd_res->header.size       += sizeof(NCP_CMD_RSSI);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_max_client_count(void *tlv)
{
    NCP_CMD_CLIENT_CNT *client_cnt = (NCP_CMD_CLIENT_CNT *)tlv;

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_UAP_MAX_CLIENT_CNT;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    unsigned uap_supported_max_sta_num   = wlan_get_uap_supported_max_clients();
    NCP_CMD_CLIENT_CNT *max_client_count = (NCP_CMD_CLIENT_CNT *)&cmd_res->params.max_client_count;

    int ret;
    cmd_res->header.size += sizeof(NCP_CMD_CLIENT_CNT);

    if (is_uap_started() != 0)
    {
        (void)PRINTF(
            "Cannot set the max station number "
            "as the uAP is already running\r\n");
        cmd_res->header.result       = NCP_CMD_RESULT_ERROR;
        max_client_count->set_status = WLAN_SET_MAX_CLIENT_CNT_START;
        return WM_SUCCESS;
    }
    else if (client_cnt->max_sta_count > uap_supported_max_sta_num)
    {
        (void)PRINTF(
            "Maximum supported station number "
            "limit is = %d\r\n",
            uap_supported_max_sta_num);
        cmd_res->header.result          = NCP_CMD_RESULT_ERROR;
        max_client_count->set_status    = WLAN_SET_MAX_CLIENT_CNT_EXCEED;
        max_client_count->support_count = uap_supported_max_sta_num;
        return WM_SUCCESS;
    }
    else
    {
        unsigned int max_sta_num = client_cnt->max_sta_count;
        ret                      = wifi_set_uap_max_clients(&max_sta_num);
    }

    if (ret == -WM_FAIL)
    {
        cmd_res->header.result       = NCP_CMD_RESULT_ERROR;
        max_client_count->set_status = WLAN_SET_MAX_CLIENT_CNT_FAIL;
        return WM_SUCCESS;
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_get_antenna_cfg(void *tlv)
{
    NCP_CMD_ANTENNA_CFG *antenna_cfg = (NCP_CMD_ANTENNA_CFG *)tlv;

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_ANTENNA;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_ANTENNA_CFG *antenna_cfg_res = (NCP_CMD_ANTENNA_CFG *)&cmd_res->params.antenna_cfg;
    antenna_cfg_res->action              = antenna_cfg->action;

    int ret;
    if (antenna_cfg->action == ACTION_SET)
    {
        ret = wlan_set_antcfg(antenna_cfg->antenna_mode, antenna_cfg->evaluate_time, antenna_cfg->evaluate_mode);
        if (ret != WM_SUCCESS)
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        uint32_t get_antenna_mode    = 0;
        uint16_t get_evaluate_time   = 0;
        uint8_t get_evaluate_mode    = 0;
        uint16_t get_current_antenna = 0;
        ret = wlan_get_antcfg(&get_antenna_mode, &get_evaluate_time, &get_evaluate_mode, &get_current_antenna);
        if (ret != WM_SUCCESS)
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;

        antenna_cfg_res->antenna_mode    = get_antenna_mode;
        antenna_cfg_res->evaluate_time   = get_evaluate_time;
        antenna_cfg_res->evaluate_mode   = get_evaluate_mode;
        antenna_cfg_res->current_antenna = get_current_antenna;
    }

    cmd_res->header.size += sizeof(NCP_CMD_ANTENNA_CFG);

    return WM_SUCCESS;
}

NCPCmd_DS_COMMAND *wlan_ncp_get_response_buffer()
{
    ncp_get_wifi_resp_buf_lock();
    return (NCPCmd_DS_COMMAND *)(wifi_res_buf);
}

#if CONFIG_NCP_SUPP_WPS
static int wlan_ncp_start_wps_pbc(void *tlv)
{
    int ret;

    ret = wlan_start_wps_pbc();
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_WPS_PBC, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_WPS_PBC, (uint16_t)ret);

    return WM_SUCCESS;
}

static int wlan_ncp_wps_generate_pin(void *tlv)
{
    uint32_t pin = 0;

#if CONFIG_WPA_SUPP_WPS
    wlan_wps_generate_pin((uint32_t *)&pin);
#else
    wlan_wps_generate_pin(&pin);
#endif

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_STA_GEN_WPS_PIN;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    NCP_CMD_WPS_GEN_PIN *gen_pin_resp = (NCP_CMD_WPS_GEN_PIN *)&cmd_res->params.wps_gen_pin_info;

    gen_pin_resp->pin = pin;
    cmd_res->header.size += sizeof(NCP_CMD_WPS_GEN_PIN);

    return WM_SUCCESS;
}

static int wlan_ncp_start_wps_pin(void *tlv)
{
    int ret;
    char pin_str[10]         = {0};
    NCP_CMD_WPS_PIN *pin_cfg = (NCP_CMD_WPS_PIN *)tlv;
    uint32_t pin             = pin_cfg->pin;
    (void)snprintf(pin_str, sizeof(pin_str), "%08d", pin);

#if CONFIG_WPA_SUPP_WPS
    ret = wlan_start_wps_pin(pin_str);
#else
    ret = wlan_start_wps_pin(pin);
#endif
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_WPS_PIN, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_WPS_PIN, (uint16_t)ret);

    return WM_SUCCESS;
}
#endif

int wlan_ncp_reset(void *data)
{
    int option = -1;
    ncp_d("NCP: run wlan_ncp_reset!\r\n");
    WLAN_RESET_data *wlan_reset_data = (WLAN_RESET_data *)data;
    option                           = wlan_reset_data->option;
    ncp_d("WLAN-RESET: option = %d\r\n", option);
    wlan_reset((cli_reset_option)option);
    if(is_nvm_enabled())
    {
        ncp_wifi_set_nvm_network();
    }

    for(int i = 0; i < MAX_HANDLES; i++)
    {
        if(nw_handles[i].type == SOCKET_HANDLE)
        {
            ncp_sock_close(i);
        }
    }

    wlan_ncp_prepare_status(NCP_RSP_WLAN_BASIC_WLAN_RESET, NCP_CMD_RESULT_OK);
    return WM_SUCCESS;
}

static int wlan_ncp_uap_prov_start(void *data)
{
    app_d("NCP: run %s!\r\n", __func__);
    uap_prov_start();
    return WM_SUCCESS;
}

static int wlan_ncp_uap_prov_reset(void *data)
{
    int ret = WM_SUCCESS;
    app_d("NCP: run %s!\r\n", __func__);
    ret = uap_prov_reset();
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_RESET, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_RESET, NCP_CMD_RESULT_ERROR);
    return ret;
}

/*WLAN HTTP COMMAND*/
/*http connect command*/
static int wlan_ncp_http_connect(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_CON_CFG *tlv = (NCP_CMD_HTTP_CON_CFG *)data;
    ret                       = ncp_http_connect(tlv->host);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail\r\n", __func__);
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_CON, NCP_CMD_RESULT_ERROR);
        return -WM_FAIL;
    }

    NCPCmd_DS_COMMAND *cmd_res    = wlan_ncp_get_response_buffer();
    NCP_CMD_HTTP_CON_CFG *tlv_res = (NCP_CMD_HTTP_CON_CFG *)&cmd_res->params.wlan_http_connect;
    tlv_res->opened_handle        = ret;
    cmd_res->header.cmd           = NCP_RSP_WLAN_HTTP_CON;
    cmd_res->header.size          = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum        = 0x00;
    cmd_res->header.result        = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_HTTP_CON_CFG);
    ncp_d("NCP: %s done!\r\n", __func__);
    return WM_SUCCESS;
}

/*http disconnect command*/
static int wlan_ncp_http_disconnect(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_DISCON_CFG *tlv = (NCP_CMD_HTTP_DISCON_CFG *)data;
    ret                          = ncp_http_disconnect(tlv->handle);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
        ret = WM_SUCCESS;
out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_DISCON, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_DISCON, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

static int wlan_ncp_http_req(void *data)
{
    int ret      = WM_SUCCESS;
    int ret_size = 0;
    ncp_d("NCP: run %s!\r\n", __func__);

    NCP_CMD_HTTP_REQ_CFG *tlv          = (NCP_CMD_HTTP_REQ_CFG *)data;

    NCPCmd_DS_COMMAND *cmd_res         = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd                = NCP_RSP_WLAN_HTTP_REQ;
    cmd_res->header.size               = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum             = 0x00;
    cmd_res->header.result             = NCP_CMD_RESULT_OK;

    NCP_CMD_HTTP_REQ_RESP_CFG *tlv_res = (NCP_CMD_HTTP_REQ_RESP_CFG *)&cmd_res->params.wlan_http_req;
    unsigned int header_len            = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_HTTP_REQ_CFG);

    if ((strlen(tlv->method) + 1) > HTTP_PARA_LEN || (strlen(tlv->uri) + 1) > HTTP_URI_LEN)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    /*the size is sent*/
    int req_size = tlv->req_size;
    if (req_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }

    ncp_d("%d, %s, %s, %d\n", tlv->handle, tlv->method, tlv->uri, tlv->req_size);
    ret_size = ncp_http_request(tlv->handle, tlv->method, tlv->uri, tlv->req_size, tlv->req_data, tlv_res->recv_header);
    if (ret_size < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:
    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        tlv_res->header_size = ret_size;
        cmd_res->header.size += sizeof(NCP_CMD_HTTP_REQ_RESP_CFG);
        cmd_res->header.size += ret_size;
    }
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*http receive command.
value[0]: handle
value[1]: recv data size
value[2]: timeout[ms], when there are no data, wait for timeout ms
value[3]: receive data buffer.
*/
static int wlan_ncp_http_recv(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_RECV_CFG *tlv = (NCP_CMD_HTTP_RECV_CFG *)data;
    unsigned int header_len    = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_HTTP_RECV_CFG);

    int recv_size = tlv->recv_size;
    if (recv_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }

    ncp_d("size = %s, timeout = %s\n", tlv->recv_size, tlv->timeout);

    NCPCmd_DS_COMMAND *cmd_res     = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd            = NCP_RSP_WLAN_HTTP_RECV;
    cmd_res->header.size           = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum         = 0x00;
    cmd_res->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_HTTP_RECV_CFG *tlv_res = (NCP_CMD_HTTP_RECV_CFG *)&cmd_res->params.wlan_http_recv;
    recv_size                      = ncp_http_recv(tlv->handle, tlv->recv_size, tlv->timeout, tlv_res->recv_data);

    ncp_d("NCP: recv_size = %d, recv_data = %s\r\n", recv_size, tlv_res->recv_data);
    if (recv_size < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
out:

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        tlv_res->recv_size   = recv_size;
        cmd_res->header.size += sizeof(NCP_CMD_SOCKET_RECEIVE_CFG);
        cmd_res->header.size += recv_size;
    }

    ncp_d("NCP: %s done!\r\n", __func__);
    return WM_SUCCESS;
}

/*http seth command*/
static int wlan_ncp_http_seth(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_SETH_CFG *tlv = (NCP_CMD_HTTP_SETH_CFG *)data;
    if (((strlen(tlv->name) + 1) > SETH_NAME_LENGTH) || ((strlen(tlv->value) + 1) > SETH_VALUE_LENGTH))
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

    ret = ncp_http_setheader(tlv->name, tlv->value);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_SETH, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_SETH, NCP_CMD_RESULT_OK);

    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*http unseth command*/
static int wlan_ncp_http_unseth(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_UNSETH_CFG *tlv = (NCP_CMD_HTTP_UNSETH_CFG *)data;
    if ((strlen(tlv->name) + 1) > SETH_NAME_LENGTH)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    ret = ncp_http_unsetheader(tlv->name);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_UNSETH, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_HTTP_UNSETH, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*http websocket upgrade command*/
static int wlan_ncp_websocket_upgrade(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_HTTP_UPG_CFG *tlv = (NCP_CMD_HTTP_UPG_CFG *)data;
    if ((strlen(tlv->protocol) + 1) > HTTP_PARA_LEN || (strlen(tlv->uri) + 1) > HTTP_URI_LEN)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    ret = ncp_ws_upg(tlv->handle, tlv->uri, tlv->protocol);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_WEBSOCKET_UPG, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_WEBSOCKET_UPG, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*http websocket send command*/
static int wlan_ncp_websocket_send(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_WEBSOCKET_SEND_CFG *tlv = (NCP_CMD_WEBSOCKET_SEND_CFG *)data;
    if ((strlen(tlv->type) + 1) > HTTP_PARA_LEN)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    unsigned int header_len = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_WEBSOCKET_SEND_CFG);
    if (tlv->size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }
    ncp_d("NCP: send_size = %d, recv_data = %s\r\n", tlv->size, tlv->send_data);
    ret = ncp_ws_send(tlv->handle, tlv->type, tlv->size, tlv->send_data);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:

    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_WEBSOCKET_SEND, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_WEBSOCKET_SEND, NCP_CMD_RESULT_OK);

    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

/*http websocket recv command*/
static int wlan_ncp_websocket_recv(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_WEBSOCKET_RECV_CFG *tlv = (NCP_CMD_WEBSOCKET_RECV_CFG *)data;

    NCPCmd_DS_COMMAND *cmd_res      = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd             = NCP_RSP_WLAN_WEBSOCKET_RECV;
    cmd_res->header.size            = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum          = 0x00;
    cmd_res->header.result          = NCP_CMD_RESULT_OK;

    NCP_CMD_WEBSOCKET_RECV_CFG *tlv_res = (NCP_CMD_WEBSOCKET_RECV_CFG *)&cmd_res->params.wlan_websocket_recv;

    unsigned int header_len         = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_WEBSOCKET_RECV_CFG);
    int recv_size                   = tlv->recv_size;
    if (recv_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }

    ncp_d("size = %s, timeout = %s\n", tlv->recv_size, tlv->timeout);

    uint32_t fin                        = tlv_res->fin;
    recv_size = ncp_ws_recv(tlv->handle, tlv->recv_size, tlv->timeout, &fin, tlv_res->recv_data);
    ncp_d("NCP: recv_size = %d, recv_data = %s\r\n", recv_size, tlv_res->recv_data);
    if (recv_size < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:
    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result          = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        tlv_res->recv_size   = recv_size;
        cmd_res->header.size += sizeof(NCP_CMD_WEBSOCKET_RECV_CFG);
        cmd_res->header.size += tlv_res->recv_size;
    }

    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

/*WLAN SOCKET COMMAND*/
static int wlan_ncp_socket_open(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_OPEN_CFG *tlv = (NCP_CMD_SOCKET_OPEN_CFG *)data;
    if ((strlen(tlv->socket_type) + 1) > HTTP_PARA_LEN || (strlen(tlv->domain_type) + 1) > HTTP_PARA_LEN ||
        (strlen(tlv->protocol) + 1) > HTTP_PARA_LEN)
    {
        ncp_e("NCP: ascii_sock_open %s!\r\n", tlv->socket_type);
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_OPEN, NCP_CMD_RESULT_ERROR);
        return -WM_FAIL;
    }
    ret = ncp_sock_open(tlv->socket_type, tlv->domain_type, tlv->protocol);
    if (ret < 0)
    {
        ncp_e("NCP: ascii_sock_open %s!\r\n", tlv->socket_type);
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_OPEN, NCP_CMD_RESULT_ERROR);
        return -WM_FAIL;
    }

    NCPCmd_DS_COMMAND *cmd_res       = wlan_ncp_get_response_buffer();
    NCP_CMD_SOCKET_OPEN_CFG *tlv_res = (NCP_CMD_SOCKET_OPEN_CFG *)&cmd_res->params.wlan_socket_open;
    tlv_res->opened_handle           = ret;
    strcpy(tlv_res->protocol, tlv->protocol);
    cmd_res->header.cmd      = NCP_RSP_WLAN_SOCKET_OPEN;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum   = 0x00;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_SOCKET_OPEN_CFG);
    ncp_d("NCP: %s done!\r\n", __func__);
    return WM_SUCCESS;
}

/*socket connect command*/
static int wlan_ncp_socket_connect(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_CON_CFG *tlv = (NCP_CMD_SOCKET_CON_CFG *)data;
    ret                         = ncp_sock_connect(tlv->handle, tlv->port, tlv->ip_addr);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
        ret = WM_SUCCESS;
out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_CON, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_CON, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*socket bind command*/
static int wlan_ncp_socket_bind(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_BIND_CFG *tlv = (NCP_CMD_SOCKET_BIND_CFG *)data;
    ret                          = ncp_sock_bind(tlv->handle, tlv->port, tlv->ip_addr);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
        ret = WM_SUCCESS;
out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_BIND, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_BIND, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*socket close command*/
static int wlan_nxp_socket_close(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_CLOSE_CFG *tlv = (NCP_CMD_SOCKET_CLOSE_CFG *)data;
    ret                           = ncp_sock_close(tlv->handle);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
        ret = WM_SUCCESS;

out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_CLOSE, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_CLOSE, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*socket listen command*/
static int wlan_ncp_socket_listen(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_LISTEN_CFG *tlv = (NCP_CMD_SOCKET_LISTEN_CFG *)data;
    ret                            = ncp_sock_listen(tlv->handle, tlv->number);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
        ret = WM_SUCCESS;

out:
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_LISTEN, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_LISTEN, NCP_CMD_RESULT_OK);
    ncp_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*socket accept command*/
static int wlan_ncp_socket_accept(void *data)
{
    int ret           = WM_SUCCESS;
    int accept_handle = -1;
    ncp_d("NCP: run %s!\r\n", __func__);

    NCPCmd_DS_COMMAND *cmd_res         = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd                = NCP_RSP_WLAN_SOCKET_ACCEPT;
    cmd_res->header.size               = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum             = 0x00;
    cmd_res->header.result             = NCP_CMD_RESULT_OK;

    NCP_CMD_SOCKET_ACCEPT_CFG *tlv = (NCP_CMD_SOCKET_ACCEPT_CFG *)data;
    accept_handle                  = ncp_sock_accept(tlv->handle);
    if (accept_handle < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
out:

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result             = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        NCP_CMD_SOCKET_ACCEPT_CFG *tlv_res = (NCP_CMD_SOCKET_ACCEPT_CFG *)&cmd_res->params.wlan_socket_accept;
        tlv_res->accepted_handle           = accept_handle;
        cmd_res->header.size += sizeof(NCP_CMD_SOCKET_ACCEPT_CFG);
        ncp_d("NCP: %s done!\r\n", __func__);
    }
    app_d("NCP: %s done!\r\n", __func__);
    return ret;
}

/*socket send command.
value[0]: handle
value[1]: send data buffer.
value[2]: send data size.
*/
static int wlan_ncp_socket_send(void *data)
{
    int ret                      = 0;
    NCP_CMD_SOCKET_SEND_CFG *tlv = (NCP_CMD_SOCKET_SEND_CFG *)data;
    unsigned int header_len      = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_SOCKET_SEND_CFG);
    int send_size                = tlv->size;
    ncp_d("NCP: hanele = %d, send_size = %d, send_data = %s\r\n", tlv->handle, tlv->size, tlv->send_data);
    if (send_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }
    ret = ncp_sock_send(tlv->handle, tlv->size, tlv->send_data);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
    {
        ncp_d("NCP: %s success!\r\n", __func__);
        ret = WM_SUCCESS;
    }
out:
#if !(CONFIG_NCP_SOCKET_SEND_FIFO)
    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_SEND, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_SEND, NCP_CMD_RESULT_OK);
#endif
    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

static int wlan_ncp_socket_sendto(void *data)
{
    int ret = 0;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_SENDTO_CFG *tlv = (NCP_CMD_SOCKET_SENDTO_CFG *)data;
    unsigned int header_len        = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_SOCKET_SENDTO_CFG);
    int send_size                  = tlv->size;
    if (send_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }
    ncp_d("NCP: handle = %d, ip_addr = %s, port = %d, size = %d, send_data = %s\r\n", tlv->handle, tlv->ip_addr,
          tlv->port, tlv->size, tlv->send_data);

    ret = ncp_sock_sendto(tlv->handle, tlv->ip_addr, tlv->port, tlv->size, tlv->send_data);
    if (ret < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
    {
        ret = WM_SUCCESS;
    }
out:

    if (ret != WM_SUCCESS)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_SENDTO, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_SOCKET_SENDTO, NCP_CMD_RESULT_OK);

    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

static int wlan_ncp_socket_receive(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_RECEIVE_CFG *tlv = (NCP_CMD_SOCKET_RECEIVE_CFG *)data;

    NCPCmd_DS_COMMAND *cmd_res      = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd             = NCP_RSP_WLAN_SOCKET_RECV;
    cmd_res->header.size            = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum          = 0x00;
    cmd_res->header.result          = NCP_CMD_RESULT_OK;

    unsigned int header_len = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_SOCKET_RECEIVE_CFG);
    int recv_size           = tlv->recv_size;
    if (recv_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }

    ncp_d("size = %u, timeout = %u\n", tlv->recv_size, tlv->timeout);

    NCP_CMD_SOCKET_RECEIVE_CFG *tlv_res = (NCP_CMD_SOCKET_RECEIVE_CFG *)&cmd_res->params.wlan_socket_receive;
    recv_size = ncp_sock_receive(tlv->handle, tlv->recv_size, tlv->timeout, tlv_res->recv_data);
    ncp_d("NCP: recv_size = %d, recv_data = %s\r\n", recv_size, tlv_res->recv_data);
    if (recv_size < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }
    else
    {
        ret = WM_SUCCESS;
    }
out:

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result   = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        tlv_res->recv_size       = recv_size;
        cmd_res->header.size     += sizeof(NCP_CMD_SOCKET_RECEIVE_CFG);
        cmd_res->header.size     += recv_size;
    }

    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

/*socket recvform command.
value[0]: handle
value[1]: recv data size
value[2]: timeout[ms], when there are no data, wait for timeout ms
value[3]: receive data buffer.
value[4]: get the peer ip addr
value[5]: get the peer port
*/
static int wlan_ncp_socket_recvfrom(void *data)
{
    int ret = WM_SUCCESS;
    ncp_d("NCP: run %s!\r\n", __func__);
    NCP_CMD_SOCKET_RECVFROM_CFG *tlv = (NCP_CMD_SOCKET_RECVFROM_CFG *)data;

    NCPCmd_DS_COMMAND *cmd_res       = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd              = NCP_RSP_WLAN_SOCKET_RECVFROM;
    cmd_res->header.size             = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum           = 0x00;
    cmd_res->header.result           = NCP_CMD_RESULT_OK;

    unsigned int header_len          = NCP_CMD_HEADER_LEN + sizeof(NCP_CMD_SOCKET_RECVFROM_CFG);
    int recv_size                    = tlv->recv_size;
    if (recv_size >= (NCP_INBUF_SIZE - header_len))
    {
        ncp_e("NCP: %s fail, the remain buffer is %d\r\n", __func__, NCP_INBUF_SIZE - header_len);
        ret = -WM_FAIL;
        goto out;
    }

    ncp_d("size = %u, timeout = %u\n", tlv->recv_size, tlv->timeout);

    NCP_CMD_SOCKET_RECVFROM_CFG *tlv_res = (NCP_CMD_SOCKET_RECVFROM_CFG *)&cmd_res->params.wlan_socket_recvfrom;
    uint32_t peer_port                   = tlv_res->peer_port;
    recv_size = ncp_sock_receivefrom(tlv->handle, tlv->recv_size, tlv->timeout, tlv_res->peer_ip, &peer_port,
                                     tlv_res->recv_data);
    tlv_res->peer_port                   = peer_port;
    ncp_d("NCP: recv_size = %d, recv_data = %s\r\n", recv_size, tlv_res->recv_data);
    if (recv_size < 0)
    {
        ncp_e("NCP: %s fail!\r\n", __func__);
        ret = -WM_FAIL;
        goto out;
    }

out:

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result   = NCP_CMD_RESULT_ERROR;
    }
    else
    {
        tlv_res->recv_size       = recv_size;
        cmd_res->header.size     += sizeof(NCP_CMD_SOCKET_RECVFROM_CFG);
        cmd_res->header.size     += recv_size;
    }

    ncp_d("NCP: %s done!\r\n", __func__);
    return 0;
}

static int wlan_ncp_11k_enable(void *tlv)
{
    int ret;
    NCP_CMD_11K_CFG *wlan_11k_enable = (NCP_CMD_11K_CFG *)tlv;

    ret = wlan_host_11k_cfg(wlan_11k_enable->enable);
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_11K_CFG, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_11K_CFG, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_11k_neighbor_req(void *tlv)
{
    int ret;
    t_u8 ssid[IEEEtypes_SSID_SIZE + 1] = {0};
    t_u16 tlv_len                      = 0;

    NCP_CMD_NEIGHBOR_REQ *neighbor_req = (NCP_CMD_NEIGHBOR_REQ *)tlv;

    tlv_len = neighbor_req->ssid_tlv.header.size;
    if ((tlv_len > 0) && (tlv_len <= 32))
    {
        (void)memcpy(ssid, neighbor_req->ssid_tlv.ssid, neighbor_req->ssid_tlv.header.size);
    }

    ret = wlan_host_11k_neighbor_req((const char *)ssid);
    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_NEIGHBOR_REQ, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_STA_NEIGHBOR_REQ, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

#if CONFIG_MEF_CFG
static int wlan_ncp_multi_mef(void *tlv)
{
    NCP_CMD_POWERMGMT_MEF *mef_config = (NCP_CMD_POWERMGMT_MEF *)tlv;
    int ret                           = 0;

    ret = wlan_config_mef(mef_config->type, mef_config->action);

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_MEF, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_MEF, (uint16_t)ret);

    return WM_SUCCESS;
}
#endif

static int wlan_ncp_wmm_uapsd(void *tlv)
{
    int ret                            = 0;
    NCP_CMD_POWERMGMT_UAPSD *uapsd_cfg = (NCP_CMD_POWERMGMT_UAPSD *)tlv;

    ret = wlan_set_wmm_uapsd(uapsd_cfg->enable);
    if (ret == WM_SUCCESS)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_UAPSD, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_UAPSD, (uint16_t)ret);
    }
    return WM_SUCCESS;
}

static int wlan_ncp_uapsd_qosinfo(void *tlv)
{
    int ret;
    NCP_CMD_POWERMGMT_QOSINFO *qosinfo_cfg = (NCP_CMD_POWERMGMT_QOSINFO *)tlv;
    t_u8 qosinfo                           = qosinfo_cfg->qos_info;

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_POWERMGMT_QOSINFO;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    ret = wlan_wmm_uapsd_qosinfo((t_u8 *)&qosinfo, qosinfo_cfg->action);

    if (!ret)
    {

        NCP_CMD_POWERMGMT_QOSINFO *qos_info_resp = (NCP_CMD_POWERMGMT_QOSINFO *)&cmd_res->params.qosinfo_cfg;

        qos_info_resp->qos_info = qosinfo;
        cmd_res->header.size += sizeof(NCP_CMD_POWERMGMT_QOSINFO);
    }
    else
    {
        cmd_res->header.result     = NCP_CMD_RESULT_ERROR;
    }

    return WM_SUCCESS;
}

static int wlan_ncp_uapsd_sleep_period(void *tlv)
{
    int ret;
    NCP_CMD_POWERMGMT_SLEEP_PERIOD *sleep_period_cfg = (NCP_CMD_POWERMGMT_SLEEP_PERIOD *)tlv;
    t_u32 period                                     = sleep_period_cfg->period;

    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();
    cmd_res->header.cmd        = NCP_RSP_WLAN_POWERMGMT_SLEEP_PERIOD;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = NCP_CMD_RESULT_OK;

    ret = wlan_sleep_period(&period, sleep_period_cfg->action);

    if (!ret)
    {
        NCP_CMD_POWERMGMT_SLEEP_PERIOD *period_resp =
            (NCP_CMD_POWERMGMT_SLEEP_PERIOD *)&cmd_res->params.sleep_period_cfg;

        period_resp->period = period;
        cmd_res->header.size += sizeof(NCP_CMD_POWERMGMT_SLEEP_PERIOD);
    }
    else
        cmd_res->header.result     = NCP_CMD_RESULT_ERROR;

    return WM_SUCCESS;
}

static int wlan_ncp_wakeup_condition(void *tlv)
{
    NCP_CMD_POWERMGMT_WOWLAN_CFG *wowlan_config = (NCP_CMD_POWERMGMT_WOWLAN_CFG *)tlv;
    uint8_t is_mef                              = 0;
    uint32_t wake_up_conds                      = 0;
    int ret                                     = 0;

    is_mef        = wowlan_config->is_mef;
    wake_up_conds = wowlan_config->wake_up_conds;
    ret           = wlan_wowlan_config(is_mef, wake_up_conds);
    if (!ret)
    {
        global_power_config.is_mef        = is_mef;
        global_power_config.wake_up_conds = wake_up_conds;
    }

    if (ret != 0)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_WOWLAN_CFG, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_WOWLAN_CFG, NCP_CMD_RESULT_OK);

    return WM_SUCCESS;
}

extern OSA_SEMAPHORE_HANDLE_DEFINE(ncp_suspend_event);
static int wlan_ncp_suspend(void *tlv)
{
    NCP_CMD_POWERMGMT_SUSPEND *suspend_cfg = (NCP_CMD_POWERMGMT_SUSPEND *)tlv;
    int ret                                = 0;

    if ((global_power_config.wake_mode == WAKE_MODE_INTF && suspend_cfg->mode > 2) ||
        (global_power_config.wake_mode == WAKE_MODE_GPIO && suspend_cfg->mode > 3) ||
        (global_power_config.wake_mode == WAKE_MODE_GPIO && !strcmp(BOARD_NAME, "FRDM-RW612") &&
          suspend_cfg->mode > 2) ||
        (global_power_config.wake_mode == WAKE_MODE_WIFI_NB && !strcmp(BOARD_NAME, "FRDM-RW612") &&
          suspend_cfg->mode > 3))
    {
        ncp_e("NCP: Invalid power mode %d!\r\n", suspend_cfg->mode);
        ncp_e("NCP: Only PM1/2 allowed with INTF mode\r\n");
        ncp_e("NCP: Only PM1/2/3 allowed with GPIO mode for RDRW612\r\n");
        ncp_e("NCP: Only PM1/2 allowed with GPIO mode for FRDMRW612\r\n");
        ncp_e("NCP: Only PM1/2/3 allowed with WIFI-NB mode for FRDMRW612\r\n");
        ret = -WM_FAIL;
        goto out;
    }
    if (!global_power_config.is_manual)
    {
        PRINTF("Error: Maunal mode is not selected!\r\n");
        ret = -WM_FAIL;
               goto out;
    }
    suspend_mode = suspend_cfg->mode;

    (void)OSA_EventSet((osa_event_handle_t)ncp_suspend_event, SUSPEND_EVENT_TRIGGERS);

out:
    if (ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_SUSPEND, NCP_CMD_RESULT_ERROR);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_SUSPEND, NCP_CMD_RESULT_OK);
    return WM_SUCCESS;
}

static int wlan_ncp_11ax_cfg(void *data)
{
    int ret;

    wlan_11ax_config_t ax_conf;
    (void)memcpy((void *)&ax_conf, data, sizeof(ax_conf));

    ret = wlan_set_11ax_cfg(&ax_conf);
    wlan_ncp_prepare_status(NCP_RSP_11AX_CFG, ret);
    return ret;
}

static int wlan_ncp_btwt_cfg(void *data)
{
    int ret;

    wlan_btwt_config_t btwt_config;
    (void)memcpy((void *)&btwt_config, data, sizeof(btwt_config));

    ret = wlan_set_btwt_cfg(&btwt_config);
    wlan_ncp_prepare_status(NCP_RSP_BTWT_CFG, ret);
    return ret;
}

static int wlan_ncp_twt_setup(void *data)
{
    int ret;

    wlan_twt_setup_config_t twt_setup_conf;
    (void)memcpy((void *)&twt_setup_conf, data, sizeof(twt_setup_conf));

    ret = wlan_set_twt_setup_cfg(&twt_setup_conf);
    wlan_ncp_prepare_status(NCP_RSP_TWT_SETUP, ret);
    return ret;
}

static int wlan_ncp_twt_teardown(void *data)
{
    int ret;

    wlan_twt_teardown_config_t teardown_conf;
    (void)memcpy((void *)&teardown_conf, (void *)data, sizeof(teardown_conf));

    ret = wlan_set_twt_teardown_cfg(&teardown_conf);
    wlan_ncp_prepare_status(NCP_RSP_TWT_TEARDOWN, ret);
    return ret;
}

static int wlan_ncp_twt_report(void *data)
{
    NCPCmd_DS_COMMAND *cmd_res    = wlan_ncp_get_response_buffer();
    NCP_CMD_TWT_REPORT twt_report = {0};
    cmd_res->header.cmd           = NCP_RSP_TWT_GET_REPORT;
    cmd_res->header.size          = NCP_CMD_HEADER_LEN;
    cmd_res->header.result        = NCP_CMD_RESULT_OK;
    cmd_res->header.seqnum        = 0x00;

    wlan_twt_report_t twt_info;
    (void)memcpy((void *)&twt_info, (void *)&twt_report, sizeof(wlan_twt_report_t));

    wlan_get_twt_report(&twt_info);
    memcpy(&cmd_res->params.twt_report, (uint8_t *)&twt_report, sizeof(twt_report));
    cmd_res->header.size += sizeof(NCP_CMD_TWT_REPORT);
    return WM_SUCCESS;
}

static int wlan_ncp_11d_enable(void *data)
{
    NCP_CMD_11D_ENABLE_CFG *cfg = (NCP_CMD_11D_ENABLE_CFG *)data;

    if (cfg->role == WLAN_BSS_ROLE_STA)
    {
        wlan_set_11d_state(WLAN_BSS_ROLE_STA, cfg->state);
    }
    else if (cfg->role == WLAN_BSS_ROLE_UAP)
    {
        wlan_set_11d_state(WLAN_BSS_ROLE_UAP, cfg->state);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_11D_ENABLE, NCP_CMD_RESULT_NOT_SUPPORT);
        return -WM_FAIL;
    }

    wlan_ncp_prepare_status(NCP_RSP_11D_ENABLE, NCP_CMD_RESULT_OK);
    return WM_SUCCESS;
}

static int wlan_ncp_region_code(void *data)
{
    int ret;
    t_u32 region_code;
    NCP_CMD_REGION_CODE_CFG *cfg   = (NCP_CMD_REGION_CODE_CFG *)data;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_REGION_CODE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;
    cmd_res->header.seqnum   = 0x00;

    if (cfg->action == ACTION_SET)
    {
        if (is_uap_started())
        {
            (void)PRINTF("Error: region code can not be set after uAP start!\r\n");
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
            return -WM_FAIL;
        }
        ret = wlan_set_region_code(cfg->region_code);
        if (ret != WM_SUCCESS)
        {
            ncp_e("set region code fail ret %d", ret);
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
            return -WM_FAIL;
        }

        cmd_res->params.region_cfg.action      = ACTION_SET;
        cmd_res->params.region_cfg.region_code = cfg->region_code;
        cmd_res->header.size += sizeof(NCP_CMD_REGION_CODE_CFG);
        /* TODO: set new power config for new region */
    }
    else if (cfg->action == ACTION_GET)
    {
        ret = wlan_get_region_code(&region_code);
        if (ret != WM_SUCCESS)
        {
            ncp_e("get region code fail ret %d", ret);
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
            return -WM_FAIL;
        }

        cmd_res->params.region_cfg.action      = ACTION_GET;
        cmd_res->params.region_cfg.region_code = region_code;
        cmd_res->header.size += sizeof(NCP_CMD_REGION_CODE_CFG);
    }
    else
    {
        cmd_res->header.result = NCP_CMD_RESULT_NOT_SUPPORT;
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

static int wlan_ncp_deep_sleep_ps(void *tlv)
{
    int ret                                       = -WM_FAIL;
    NCP_CMD_DEEP_SLEEP_PS *wlan_deep_sleep_enable = (NCP_CMD_DEEP_SLEEP_PS *)tlv;

    if (wlan_deep_sleep_enable->enable == MTRUE)
    {
        ret = wlan_deepsleepps_on();
    }
    else if (wlan_deep_sleep_enable->enable == MFALSE)
    {
        ret = wlan_deepsleepps_off();
    }

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_DEEP_SLEEP_PS, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_DEEP_SLEEP_PS, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_ieee_ps(void *tlv)
{
    int ret                           = -WM_FAIL;
    NCP_CMD_IEEE_PS *wlan_ieee_enable = (NCP_CMD_IEEE_PS *)tlv;
    unsigned int condition            = 0;

    if (wlan_ieee_enable->enable == MTRUE)
    {
        condition = WAKE_ON_ARP_BROADCAST | WAKE_ON_UNICAST | WAKE_ON_MULTICAST | WAKE_ON_MAC_EVENT;
        ret       = wlan_ieeeps_on(condition);
    }
    else if (wlan_ieee_enable->enable == MFALSE)
    {
        ret = wlan_ieeeps_off();
    }

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_IEEE_PS, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_POWERMGMT_IEEE_PS, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_register_access(void *data)
{
    NCP_CMD_REGISTER_ACCESS *register_access = (NCP_CMD_REGISTER_ACCESS *)data;
    NCPCmd_DS_COMMAND *cmd_res               = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_DEBUG_REGISTER_ACCESS;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_REGISTER_ACCESS *register_access_res = (NCP_CMD_REGISTER_ACCESS *)&cmd_res->params.register_access;
    register_access_res->action                  = register_access->action;
    register_access_res->type                    = register_access->type;
    register_access_res->offset                  = register_access->offset;

    int ret;
    uint32_t value = 0;
    ret = wlan_reg_access((wifi_reg_t)register_access->type, register_access->action, register_access->offset, &value);

    if (ret == WM_SUCCESS)
    {
        if (register_access->action == ACTION_GET)
            register_access_res->value = value;
    }
    else
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;

    cmd_res->header.size += sizeof(NCP_CMD_REGISTER_ACCESS);

    return WM_SUCCESS;
}

static int wlan_ncp_memory_state(void *data)
{
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_MEMORY_HEAP_SIZE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MEM_STAT *mem_stat_res            = (NCP_CMD_MEM_STAT *)&cmd_res->params.mem_stat;
    mem_stat_res->free_heap_size              = xPortGetFreeHeapSize();
    mem_stat_res->minimum_ever_free_heap_size = xPortGetMinimumEverFreeHeapSize();

    cmd_res->header.size += sizeof(NCP_CMD_MEM_STAT);

    return WM_SUCCESS;
}

static int wlan_ncp_set_get_eu_mac_mode(void *data)
{
    NCP_CMD_ED_MAC *ed_mac_mode = (NCP_CMD_ED_MAC *)data;
    NCPCmd_DS_COMMAND *cmd_res  = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_ED_MAC_MODE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ED_MAC *ed_mac_res = (NCP_CMD_ED_MAC *)&cmd_res->params.ed_mac_mode;
    ed_mac_res->action         = ed_mac_mode->action;

    int ret;
    wlan_ed_mac_ctrl_t wlan_ed_mac_ctrl;

    if (ed_mac_mode->action == ACTION_SET)
    {
        wlan_ed_mac_ctrl.ed_ctrl_2g   = ed_mac_mode->ed_ctrl_2g;
        wlan_ed_mac_ctrl.ed_offset_2g = ed_mac_mode->ed_offset_2g;
#if CONFIG_NCP_5GHz_SUPPORT
        wlan_ed_mac_ctrl.ed_ctrl_5g   = ed_mac_mode->ed_ctrl_5g;
        wlan_ed_mac_ctrl.ed_offset_5g = ed_mac_mode->ed_offset_5g;
#endif

        ret = wlan_set_ed_mac_mode(wlan_ed_mac_ctrl);
        if (ret != WM_SUCCESS)
        {
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        }
    }
    else
    {
        ret = wlan_get_ed_mac_mode(&wlan_ed_mac_ctrl);
        if (ret == WM_SUCCESS)
        {
            ed_mac_res->ed_ctrl_2g   = wlan_ed_mac_ctrl.ed_ctrl_2g;
            ed_mac_res->ed_offset_2g = wlan_ed_mac_ctrl.ed_offset_2g;
#if CONFIG_NCP_5GHz_SUPPORT
            ed_mac_res->ed_ctrl_5g   = wlan_ed_mac_ctrl.ed_ctrl_5g;
            ed_mac_res->ed_offset_5g = wlan_ed_mac_ctrl.ed_offset_5g;
#endif
        }
        else
        {
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        }
    }

    cmd_res->header.size += sizeof(NCP_CMD_ED_MAC);

    return WM_SUCCESS;
}

#if CONFIG_NCP_RF_TEST_MODE
static int wlan_ncp_set_rf_test_mode(void *tlv)
{
    int ret = -WM_FAIL;

    ret = wlan_set_rf_test_mode();

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TEST_MODE, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TEST_MODE, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_tx_antenna(void *tlv)
{
    int ret                       = -WM_FAIL;
    NCP_CMD_RF_TX_ANTENNA *tx_ant = (NCP_CMD_RF_TX_ANTENNA *)tlv;

    ret = wlan_set_rf_tx_antenna(tx_ant->ant);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_ANTENNA, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_ANTENNA, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_tx_antenna(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_TX_ANTENNA;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_TX_ANTENNA *tx_ant = (NCP_CMD_RF_TX_ANTENNA *)&cmd_res->params.rf_tx_antenna;
    uint8_t antenna;

    ret = wlan_get_rf_tx_antenna(&antenna);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    tx_ant->ant = antenna;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_TX_ANTENNA);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_rx_antenna(void *tlv)
{
    int ret                       = -WM_FAIL;
    NCP_CMD_RF_RX_ANTENNA *rx_ant = (NCP_CMD_RF_RX_ANTENNA *)tlv;

    ret = wlan_set_rf_rx_antenna(rx_ant->ant);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_RX_ANTENNA, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_RX_ANTENNA, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_rx_antenna(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_RX_ANTENNA;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_RX_ANTENNA *rx_ant = (NCP_CMD_RF_RX_ANTENNA *)&cmd_res->params.rf_rx_antenna;
    uint8_t antenna;

    ret = wlan_get_rf_rx_antenna(&antenna);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    rx_ant->ant = antenna;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_RX_ANTENNA);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_band(void *tlv)
{
    int ret                  = -WM_FAIL;
    NCP_CMD_RF_BAND *rf_band = (NCP_CMD_RF_BAND *)tlv;

    ret = wlan_set_rf_band(rf_band->band);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_BAND, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_BAND, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_band(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_BAND;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_BAND *rf_band = (NCP_CMD_RF_BAND *)&cmd_res->params.rf_band;
    uint8_t band;

    ret = wlan_get_rf_band(&band);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    rf_band->band = band;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_BAND);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_bandwidth(void *tlv)
{
    int ret                            = -WM_FAIL;
    NCP_CMD_RF_BANDWIDTH *rf_bandwidth = (NCP_CMD_RF_BANDWIDTH *)tlv;

    ret = wlan_set_rf_bandwidth(rf_bandwidth->bandwidth);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_BANDWIDTH, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_BANDWIDTH, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_bandwidth(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_BANDWIDTH;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_BANDWIDTH *rf_bandwidth = (NCP_CMD_RF_BANDWIDTH *)&cmd_res->params.rf_bandwidth;
    uint8_t bandwidth;

    ret = wlan_get_rf_bandwidth(&bandwidth);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    rf_bandwidth->bandwidth = bandwidth;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_BANDWIDTH);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_channel(void *tlv)
{
    int ret                        = -WM_FAIL;
    NCP_CMD_RF_CHANNEL *rf_channel = (NCP_CMD_RF_CHANNEL *)tlv;

    ret = wlan_set_rf_channel(rf_channel->channel);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_CHANNEL, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_CHANNEL, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_channel(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_CHANNEL;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_CHANNEL *rf_channel = (NCP_CMD_RF_CHANNEL *)&cmd_res->params.rf_channel;
    uint8_t channel;

    ret = wlan_get_rf_channel(&channel);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    rf_channel->channel = channel;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_CHANNEL);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_radio_mode(void *tlv)
{
    int ret                              = -WM_FAIL;
    NCP_CMD_RF_RADIO_MODE *rf_radio_mode = (NCP_CMD_RF_RADIO_MODE *)tlv;

    ret = wlan_set_rf_radio_mode(rf_radio_mode->radio_mode);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_RADIO_MODE, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_RADIO_MODE, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_rf_radio_mode(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_RF_RADIO_MODE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_RADIO_MODE *rf_radio_mode = (NCP_CMD_RF_RADIO_MODE *)&cmd_res->params.rf_radio_mode;
    uint8_t mode;
    ret = wlan_get_rf_radio_mode(&mode);
    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }
    rf_radio_mode->radio_mode = mode;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_RADIO_MODE);

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_tx_power(void *tlv)
{
    int ret                          = -WM_FAIL;
    NCP_CMD_RF_TX_POWER *rf_tx_power = (NCP_CMD_RF_TX_POWER *)tlv;

    ret = wlan_set_rf_tx_power(rf_tx_power->power, rf_tx_power->mod, rf_tx_power->path_id);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_POWER, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_POWER, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_tx_cont_mode(void *tlv)
{
    int ret                                  = -WM_FAIL;
    NCP_CMD_RF_TX_CONT_MODE *rf_tx_cont_mode = (NCP_CMD_RF_TX_CONT_MODE *)tlv;

    ret =
        wlan_set_rf_tx_cont_mode(rf_tx_cont_mode->enable_tx, rf_tx_cont_mode->cw_mode, rf_tx_cont_mode->payload_pattern,
                                 rf_tx_cont_mode->cs_mode, rf_tx_cont_mode->act_sub_ch, rf_tx_cont_mode->tx_rate);

    if (!ret)
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_CONT_MODE, NCP_CMD_RESULT_OK);
    }
    else
    {
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_CONT_MODE, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_set_rf_tx_frame(void *tlv)
{
    int ret                          = -WM_FAIL;
    NCP_CMD_RF_TX_FRAME *rf_tx_frame = (NCP_CMD_RF_TX_FRAME *)tlv;

    ret = wlan_set_rf_tx_frame(rf_tx_frame->enable, rf_tx_frame->data_rate, rf_tx_frame->frame_pattern,
                               rf_tx_frame->frame_length, rf_tx_frame->adjust_burst_sifs, rf_tx_frame->burst_sifs_in_us,
                               rf_tx_frame->short_preamble, rf_tx_frame->act_sub_ch, rf_tx_frame->short_gi,
                               rf_tx_frame->adv_coding, rf_tx_frame->tx_bf, rf_tx_frame->gf_mode, rf_tx_frame->stbc,
                               rf_tx_frame->bssid);

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_FRAME, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_SET_RF_TX_FRAME, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_get_and_reset_rf_per(void *tlv)
{
    int ret;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_WLAN_REGULATORY_GET_AND_RESET_RF_PER;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;

    NCP_CMD_RF_RADIO_MODE *rf_radio_mode = (NCP_CMD_RF_RADIO_MODE *)&cmd_res->params.rf_radio_mode;
    uint8_t radio_mode;

    ret = wlan_get_rf_radio_mode(&radio_mode);

    if (ret != WM_SUCCESS)
    {
        cmd_res->header.result = NCP_CMD_RESULT_ERROR;
        return WM_SUCCESS;
    }

    rf_radio_mode->radio_mode = radio_mode;

    cmd_res->header.result = NCP_CMD_RESULT_OK;
    cmd_res->header.size += sizeof(NCP_CMD_RF_RADIO_MODE);

    return WM_SUCCESS;
}
#endif

static int wlan_ncp_eu_crypto_ccmp_128(void *data)
{
    int ret;
    uint16_t Dec_DataLength;
    uint16_t Enc_DataLength;
    uint16_t KeyLength;
    uint16_t NonceLength;
    uint16_t AADLength;
    uint8_t DATA[80] = {0};
    uint16_t Length;
    NCP_CMD_EU_CRYPRTO *eu_crypto = (NCP_CMD_EU_CRYPRTO *)data;

    /*Algorithm: AES_CCMP_128*/
    t_u8 Key[16]     = {0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85, 0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f};
    KeyLength        = 16;
    t_u8 EncData[20] = {0xf8, 0xba, 0x1a, 0x55, 0xd0, 0x2f, 0x85, 0xae, 0x96, 0x7b,
                        0xb6, 0x2f, 0xb6, 0xcd, 0xa8, 0xeb, 0x7e, 0x78, 0xa0, 0x50};
    Enc_DataLength   = 20;
    t_u8 DecData[28] = {0xf3, 0xd0, 0xa2, 0xfe, 0x9a, 0x3d, 0xbf, 0x23, 0x42, 0xa6, 0x43, 0xe4, 0x32, 0x46,
                        0xe8, 0x0c, 0x3c, 0x04, 0xd0, 0x19, 0x78, 0x45, 0xce, 0x0b, 0x16, 0xf9, 0x76, 0x23};
    Dec_DataLength   = 28;
    t_u8 Nonce[13]   = {0x00, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0xb5, 0x03, 0x97, 0x76, 0xe7, 0x0c};
    NonceLength      = 13;
    t_u8 AAD[22]     = {0x08, 0x40, 0x0f, 0xd2, 0xe1, 0x28, 0xa5, 0x7c, 0x50, 0x30, 0xf1,
                        0x84, 0x44, 0x08, 0xab, 0xae, 0xa5, 0xb8, 0xfc, 0xba, 0x00, 0x00};
    AADLength        = 22;

    if (eu_crypto->enc == 0U)
    {
        (void)memcpy(DATA, DecData, Dec_DataLength);
        Length = Dec_DataLength;
        ret    = wlan_set_crypto_AES_CCMP_decrypt(Key, KeyLength, AAD, AADLength, Nonce, NonceLength, DATA, &Length);
    }
    else
    {
        (void)memcpy(DATA, EncData, Enc_DataLength);
        Length = Enc_DataLength;
        ret    = wlan_set_crypto_AES_CCMP_encrypt(Key, KeyLength, AAD, AADLength, Nonce, NonceLength, DATA, &Length);
    }

    if (ret == WM_SUCCESS)
    {
        (void)PRINTF("Raw Data:\r\n");
        if (eu_crypto->enc == 0U)
        {
            dump_hex((uint8_t *)DecData, Dec_DataLength);
            (void)PRINTF("Decrypted Data:\r\n");
            dump_hex((uint8_t *)DATA, Length);
        }
        else
        {
            dump_hex((uint8_t *)EncData, Enc_DataLength);
            (void)PRINTF("Encrypted Data:\r\n");
            dump_hex((uint8_t *)DATA, Length);
        }

        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_CCMP_128, NCP_CMD_RESULT_OK);
    }
    else
    {
        (void)PRINTF("Hostcmd failed error: %d \r\n", ret);

        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_CCMP_128, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}

static int wlan_ncp_eu_crypto_gcmp_128(void *data)
{
    int ret;
    uint16_t Dec_DataLength;
    uint16_t Dec_DataOnlyLength;
    uint16_t Dec_TagLength;
    uint16_t Enc_DataLength;
    uint16_t KeyLength;
    uint16_t NonceLength;
    uint16_t AADLength;
    uint8_t DATA[80] = {0};
    uint16_t Length;
    NCP_CMD_EU_CRYPRTO *eu_crypto = (NCP_CMD_EU_CRYPRTO *)data;

    /*Algorithm: AES_WRAP*/
    uint8_t Key[16]     = {0xc9, 0x7c, 0x1f, 0x67, 0xce, 0x37, 0x11, 0x85, 0x51, 0x4a, 0x8a, 0x19, 0xf2, 0xbd, 0xd5, 0x2f};
    KeyLength           = 16;
    uint8_t EncData[40] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
                        0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
                        0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
    Enc_DataLength   = 40;
    uint8_t DecDataOnly[40] = {0x60, 0xe9, 0x70, 0x0c, 0xc4, 0xd4, 0x0a, 0xc6, 0xd2, 0x88, 0xb2, 0x01, 0xc3, 0x8f,
                            0x5b, 0xf0, 0x8b, 0x80, 0x74, 0x42, 0x64, 0x0a, 0x15, 0x96, 0xe5, 0xdb, 0xda, 0xd4,
                            0x1d, 0x1f, 0x36, 0x23, 0xf4, 0x5d, 0x7a, 0x12, 0xdb, 0x7a, 0xfb, 0x23};
    Dec_DataOnlyLength   = 40;
#if defined(SD9177) || defined(RW610)
    uint8_t DecTag[16] = {0xde, 0xf6, 0x19, 0xc2, 0xa3, 0x74, 0xb6, 0xdf, 0x66, 0xff, 0xa5, 0x3b, 0x6c, 0x69, 0xd7, 0x9e};
#else
    uint8_t DecTag[16] = {0xe9, 0x04, 0x97, 0xa1, 0xec, 0x9c, 0x5e, 0x8b, 0x85, 0x5b, 0x9d, 0xc3, 0xa8, 0x16, 0x91, 0xa3};
#endif
    Dec_TagLength = 16;

    uint8_t DecData[56] = {0}; /*Dec-data-only + Tag*/
    (void)memcpy(DecData, DecDataOnly, Dec_DataOnlyLength);
    (void)memcpy(DecData + Dec_DataOnlyLength, DecTag, Dec_TagLength);
    Dec_DataLength = Dec_DataOnlyLength + Dec_TagLength;

    uint8_t Nonce[12] = {0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x00, 0x89, 0x5f, 0x5f, 0x2b, 0x08};
    NonceLength       = 12;
    uint8_t AAD[24]   = {0x88, 0x48, 0x0f, 0xd2, 0xe1, 0x28, 0xa5, 0x7c, 0x50, 0x30, 0xf1, 0x84,
                      0x44, 0x08, 0x50, 0x30, 0xf1, 0x84, 0x44, 0x08, 0x80, 0x33, 0x03, 0x00};
    AADLength         = 24;

    if (eu_crypto->enc == 0U)
    {
        (void)memcpy(DATA, DecData, Dec_DataLength);
        Length = Dec_DataLength;
        ret    = wlan_set_crypto_AES_GCMP_decrypt(Key, KeyLength, AAD, AADLength, Nonce, NonceLength, DATA, &Length);
    }
    else
    {
        (void)memcpy(DATA, EncData, Enc_DataLength);
        Length = Enc_DataLength;
        ret    = wlan_set_crypto_AES_GCMP_encrypt(Key, KeyLength, AAD, AADLength, Nonce, NonceLength, DATA, &Length);
    }

    if (ret == WM_SUCCESS)
    {
        (void)PRINTF("Raw Data:\r\n");
        if (eu_crypto->enc == 0U)
        {
            dump_hex((uint8_t *)DecData, Dec_DataLength);
            (void)PRINTF("Decrypted Data:\r\n");
            dump_hex((uint8_t *)DATA, Length);
        }
        else
        {
            dump_hex((uint8_t *)EncData, Enc_DataLength);
            (void)PRINTF("Encrypted Data:\r\n");
            dump_hex((uint8_t *)DATA, Length);
        }

        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_GCMP_128, NCP_CMD_RESULT_OK);
    }
    else
    {
        (void)PRINTF("Hostcmd failed error: %d \r\n", ret);

        wlan_ncp_prepare_status(NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_GCMP_128, NCP_CMD_RESULT_ERROR);
    }

    return WM_SUCCESS;
}


static int wlan_ncp_date_time(void *data)
{
    int ret;
    NCP_CMD_DATE_TIME_CFG *cfg = (NCP_CMD_DATE_TIME_CFG *)data;
    wlan_date_time_t *date;
    rtc_datetime_t rtc_date_time = {0};
    NCPCmd_DS_COMMAND *cmd_res   = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_DATE_TIME;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;
    cmd_res->header.seqnum   = 0x00;

    if (cfg->action == ACTION_SET)
    {
        date = &cfg->date_time;

        /* transfer avoid structure align difference */
        rtc_date_time.year   = date->year;
        rtc_date_time.month  = date->month;
        rtc_date_time.day    = date->day;
        rtc_date_time.hour   = date->hour;
        rtc_date_time.minute = date->minute;
        rtc_date_time.second = date->second;

        ret = RTC_SetDatetime(RTC, &rtc_date_time);
        if (ret != 0)
        {
            ncp_e("set RTC date time fail ret %d", ret);
            cmd_res->header.result = NCP_CMD_RESULT_ERROR;
            return -WM_FAIL;
        }

        cmd_res->params.date_time.action = ACTION_SET;
        cmd_res->header.size += sizeof(NCP_CMD_DATE_TIME_CFG);
    }
    else if (cfg->action == ACTION_GET)
    {
        date = &cmd_res->params.date_time.date_time;

        RTC_GetDatetime(RTC, &rtc_date_time);

        /* transfer avoid structure align difference */
        date->year   = rtc_date_time.year;
        date->month  = rtc_date_time.month;
        date->day    = rtc_date_time.day;
        date->hour   = rtc_date_time.hour;
        date->minute = rtc_date_time.minute;
        date->second = rtc_date_time.second;

        cmd_res->params.date_time.action = ACTION_GET;
        cmd_res->header.size += sizeof(NCP_CMD_DATE_TIME_CFG);
    }
    else
    {
        cmd_res->header.result = NCP_CMD_RESULT_NOT_SUPPORT;
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

static int wlan_ncp_get_temperature(void *data)
{
    uint32_t val;
    uint32_t temp;
    NCPCmd_DS_COMMAND *cmd_res = wlan_ncp_get_response_buffer();

    cmd_res->header.cmd      = NCP_RSP_GET_TEMPERATUE;
    cmd_res->header.size     = NCP_CMD_HEADER_LEN;
    cmd_res->header.result   = NCP_CMD_RESULT_OK;
    cmd_res->header.seqnum   = 0x00;

    if (reg_access_cnt == 0)
    {
        val = *((volatile uint32_t *)WLAN_CAU_ENABLE_ADDR);
        val &= ~(0xC);
        val |= (2 << 2);
        (*((volatile uint32_t *)WLAN_CAU_ENABLE_ADDR)) = val;
        reg_access_cnt++;
    }

    val  = *((volatile uint32_t *)WLAN_CAU_TEMPERATURE_ADDR);
    val  = (val & 0xFFC00) >> 10;
    temp = ((val * 478011) - 219082200) / 1000000;

    cmd_res->params.temperature.temp = temp;
    cmd_res->header.size += sizeof(NCP_CMD_TEMPERATURE);

    return WM_SUCCESS;
}

int wlan_ncp_prepare_mdns_result(mdns_result_ring_buffer_t *mdns_res)
{
    int i;
    uint8_t *event_buf = NULL;
    uint32_t total_len = 0;
    PTR_ParamSet_t *ptr_tlv         = NULL;
    SRV_ParamSet_t *srv_tlv         = NULL;
    TXT_ParamSet_t *txt_tlv         = NULL;
    IP_ADDR_ParamSet_t *ip_addr_tlv = NULL;

    NCPCmd_DS_COMMAND *evt_res           = NULL;
    NCP_EVT_MDNS_RESULT *mdns_result_tlv = NULL;

    uint8_t *ptlv_pos;
    uint32_t tlv_buf_len;
    uint32_t rd = mdns_res->ring_tail;
    uint32_t wr = mdns_res->ring_head;

    if (rd == wr)
    {
        return WM_SUCCESS;
    }

    while (rd != wr)
    {
        total_len = NCP_CMD_HEADER_LEN + sizeof(NCP_EVT_MDNS_RESULT) - sizeof(uint8_t) + PTR_TLV_LEN +
                    SRV_TLV_LEN + TXT_TLV_LEN + IP_ADDR_TLV_LEN * mdns_res->ring_buffer[rd].ip_count;
        /* Allocate memory for event */
        event_buf = (uint8_t *)OSA_MemoryAllocate(total_len);
        if (event_buf == NULL)
        {
            ncp_e("failed to allocate memory for event");
            return -WM_FAIL;
        }

        evt_res         = (NCPCmd_DS_COMMAND *)event_buf;
        mdns_result_tlv = (NCP_EVT_MDNS_RESULT *)&evt_res->params.mdns_result;
        ptlv_pos        = mdns_result_tlv->tlv_buf;
        tlv_buf_len     = 0;

        mdns_result_tlv->ttl = mdns_res->ring_buffer[rd].ttl;

        if (mdns_res->ring_buffer[rd].only_ptr == 1)
        {
            ptr_tlv = (PTR_ParamSet_t *)ptlv_pos;
            (void)memcpy(ptr_tlv->service_type, mdns_res->ring_buffer[rd].service_type,
                         strlen(mdns_res->ring_buffer[rd].service_type) + 1);
            (void)memcpy(ptr_tlv->proto, mdns_res->ring_buffer[rd].proto, strlen(mdns_res->ring_buffer[rd].proto) + 1);
            ptr_tlv->header.type = NCP_CMD_NETWORK_MDNS_RESULT_PTR;
            ptr_tlv->header.size =
                sizeof(ptr_tlv->instance_name) + sizeof(ptr_tlv->service_type) + sizeof(ptr_tlv->proto);
            ptlv_pos += sizeof(PTR_ParamSet_t);
            tlv_buf_len += sizeof(PTR_ParamSet_t);
            goto done;
        }

        if (mdns_res->ring_buffer[rd].instance_name != NULL)
        {
            ptr_tlv = (PTR_ParamSet_t *)ptlv_pos;
            (void)memcpy(ptr_tlv->instance_name, mdns_res->ring_buffer[rd].instance_name,
                         strlen(mdns_res->ring_buffer[rd].instance_name) + 1);
            (void)memcpy(ptr_tlv->service_type, mdns_res->ring_buffer[rd].service_type,
                         strlen(mdns_res->ring_buffer[rd].service_type) + 1);
            (void)memcpy(ptr_tlv->proto, mdns_res->ring_buffer[rd].proto, strlen(mdns_res->ring_buffer[rd].proto) + 1);
            ptr_tlv->header.type = NCP_CMD_NETWORK_MDNS_RESULT_PTR;
            ptr_tlv->header.size =
                sizeof(ptr_tlv->instance_name) + sizeof(ptr_tlv->service_type) + sizeof(ptr_tlv->proto);
            ptlv_pos += sizeof(PTR_ParamSet_t);
            tlv_buf_len += sizeof(PTR_ParamSet_t);
        }

        if (mdns_res->ring_buffer[rd].hostname != NULL)
        {
            srv_tlv = (SRV_ParamSet_t *)ptlv_pos;
            (void)memcpy(srv_tlv->host_name, mdns_res->ring_buffer[rd].hostname,
                         strlen(mdns_res->ring_buffer[rd].hostname) + 1);
            (void)memcpy(srv_tlv->target, mdns_res->ring_buffer[rd].target,
                         strlen(mdns_res->ring_buffer[rd].target) + 1);
            srv_tlv->port        = mdns_res->ring_buffer[rd].port;
            srv_tlv->header.type = NCP_CMD_NETWORK_MDNS_RESULT_SRV;
            srv_tlv->header.size = sizeof(srv_tlv->host_name) + sizeof(srv_tlv->target) + sizeof(srv_tlv->port);
            ptlv_pos += sizeof(SRV_ParamSet_t);
            tlv_buf_len += sizeof(SRV_ParamSet_t);
        }

        if (mdns_res->ring_buffer[rd].txt_len != 0)
        {
            txt_tlv = (TXT_ParamSet_t *)ptlv_pos;
            (void)memcpy(txt_tlv->txt, mdns_res->ring_buffer[rd].txt, strlen(mdns_res->ring_buffer[rd].txt) + 1);
            txt_tlv->txt_len     = mdns_res->ring_buffer[rd].txt_len;
            txt_tlv->header.type = NCP_CMD_NETWORK_MDNS_RESULT_TXT;
            txt_tlv->header.size = sizeof(txt_tlv->txt) + sizeof(txt_tlv->txt_len);
            ptlv_pos += sizeof(TXT_ParamSet_t);
            tlv_buf_len += sizeof(TXT_ParamSet_t);
        }

        mdns_ip_addr_t *tmp = mdns_res->ring_buffer[rd].ip_addr;
        while (tmp != NULL)
        {
            ip_addr_tlv            = (IP_ADDR_ParamSet_t *)ptlv_pos;
            ip_addr_tlv->addr_type = tmp->addr_type;
            if (tmp->addr_type == 4)
                ip_addr_tlv->ip.ip_v4 = tmp->ip.ip_v4;
            else
            {
                for (i = 0; i < 4; i++)
                    ip_addr_tlv->ip.ip_v6[i] = tmp->ip.ip_v6[i];
            }

            ip_addr_tlv->header.type = NCP_CMD_NETWORK_MDNS_RESULT_IP_ADDR;
            ip_addr_tlv->header.size = sizeof(ip_addr_tlv->addr_type) + sizeof(ip_addr_tlv->ip);
            ptlv_pos += sizeof(IP_ADDR_ParamSet_t);
            tlv_buf_len += sizeof(IP_ADDR_ParamSet_t);

            tmp = tmp->next;
        }

    done:
        mdns_result_tlv->tlv_buf_len = tlv_buf_len;

        evt_res->header.cmd  = NCP_EVENT_MDNS_QUERY_RESULT;
        evt_res->header.size = NCP_CMD_HEADER_LEN + sizeof(mdns_result_tlv->ttl) +
                               sizeof(mdns_result_tlv->tlv_buf_len) + tlv_buf_len;
        evt_res->header.seqnum   = 0x00;
        evt_res->header.result   = NCP_CMD_RESULT_OK;
        wifi_ncp_send_response(event_buf);

        /* Clear event buffer */
        (void)memset(event_buf, 0x00, total_len);

        ncp_d("Answers: instance name = %s", mdns_res->ring_buffer[rd].instance_name);

        /* Release the space of the sent result */
        app_mdns_result_ring_buffer_free(&mdns_res->ring_buffer[rd]);

        /* Update mdns result ring buffer tail */
        MDNS_BUFFER_UPDATE_HEAD_TAIL(mdns_res->ring_tail, 1);
        rd = mdns_res->ring_tail;

        /* Free event buffer */
        OSA_MemoryFree(event_buf);
        event_buf = NULL;
    }

    return WM_SUCCESS;
}

uint8_t *wlan_ncp_prepare_mdns_resolve_result(ip_addr_t *ipaddr)
{
    int i;
    uint8_t *event_buf         = NULL;
    uint32_t total_len         = NCP_CMD_HEADER_LEN + sizeof(NCP_EVT_MDNS_RESOLVE);
    NCPCmd_DS_COMMAND *evt_res = NULL;

    /* Allocate memory for event */
    event_buf = (uint8_t *)OSA_MemoryAllocate(total_len);
    if (event_buf == NULL)
    {
        ncp_e("failed to allocate memory for event");
        return NULL;
    }

    evt_res                                = (NCPCmd_DS_COMMAND *)event_buf;
    NCP_EVT_MDNS_RESOLVE *mdns_resolve_tlv = (NCP_EVT_MDNS_RESOLVE *)&evt_res->params.mdns_resolve;

    evt_res->header.cmd      = NCP_EVENT_MDNS_RESOLVE_DOMAIN;
    evt_res->header.size     = NCP_CMD_HEADER_LEN;
    evt_res->header.seqnum   = 0x00;
    evt_res->header.result   = NCP_CMD_RESULT_OK;

    if (ipaddr != NULL)
    {
        mdns_resolve_tlv->ip_type = ipaddr->type;

        if (IP_IS_V6(ipaddr))
        {
            for (i = 0; i < 4; i++)
                mdns_resolve_tlv->u_addr.ip6_addr[i] = ipaddr->u_addr.ip6.addr[i];
        }
        else
        {
            mdns_resolve_tlv->u_addr.ip4_addr = ipaddr->u_addr.ip4.addr;
        }
    }

    evt_res->header.size += sizeof(NCP_EVT_MDNS_RESOLVE);

    return event_buf;
}

/* Read mDNS configuration from LittleFS, and initialize mDNS*/
int ncp_mdns_init(void)
{
    int ret = -WM_FAIL;
    char mdns_enabled[PROV_MDNS_ENABLED_MAX_LEN];
    char mdns_hostname[PROV_HOST_NAME_MAX_LEN];

    ret = ncp_get_conf("prov", "mdns_enabled", mdns_enabled, sizeof(mdns_enabled));
    if (ret == WM_SUCCESS && (strcmp(mdns_enabled, "1") == 0))
    {
        ret = ncp_get_conf("prov", "hostname", mdns_hostname, sizeof(mdns_hostname));
        if (ret == WM_SUCCESS)
        {
            app_mdns_start(mdns_hostname);
        }
        else
        {
            ncp_e("failed to read variable 'hostname'");
        }
    }
    else
    {
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to read variable 'mdns_enabled'");
        }
        else if (strcmp(mdns_enabled, "0") == 0)
        {
            ncp_e("mDNS is not enabled");
        }
    }

    return ret;
}

static int wlan_ncp_mdns_query(void *tlv)
{
    int ret                        = WM_SUCCESS;
    NCP_CMD_MDNS_QUERY *mdns_query = (NCP_CMD_MDNS_QUERY *)tlv;

    switch (mdns_query->qtype)
    {
        case DNS_RRTYPE_A:
            ncp_d("Query A: %s", mdns_query->Q.a_cfg.name);
            ret = app_mdns_query_a(mdns_query->Q.a_cfg.name, MDNS_IFAC_ROLE_UAP);
            break;
        case DNS_RRTYPE_PTR:
            switch (mdns_query->Q.ptr_cfg.proto)
            {
                case DNSSD_PROTO_UDP:
                    ncp_d("Query PTR: %s.%s.local", mdns_query->Q.ptr_cfg.service, MDNS_PROTO_UDP);
                    break;
                case DNSSD_PROTO_TCP:
                    ncp_d("Query PTR: %s.%s.local", mdns_query->Q.ptr_cfg.service, MDNS_PROTO_TCP);
                    break;
                default:
                    ncp_e("Invaild protocol value");
                    ret = -WM_FAIL;
                    goto done;
            }
            ret = app_mdns_search_service(&mdns_query->Q.ptr_cfg, MDNS_IFAC_ROLE_UAP);
            break;
        default:
            ncp_e("Invaild query type");
            ret = -WM_FAIL;
            break;
    }

done:
    wlan_ncp_prepare_status(NCP_RSP_WLAN_NETWORK_MDNS_QUERY, ret);

    return WM_SUCCESS;
}

#if CONFIG_DRIVER_MBO
#if !CONFIG_WPA_SUPP
static int wlan_ncp_mbo_enable(void *tlv)
{
    int ret;
    NCP_CMD_MBO_ENABLE *wlan_mbo_enable = (NCP_CMD_MBO_ENABLE *)tlv;

    ret = wlan_host_mbo_cfg(wlan_mbo_enable->enable);

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_ENABLE, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_ENABLE, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}
#endif

static int wlan_ncp_mbo_nonprefer_ch(void *tlv)
{
    int ret;
    NCP_CMD_MBO_NONPREFER_CH *mbo_nonprefer_ch = (NCP_CMD_MBO_NONPREFER_CH *)tlv;

#if CONFIG_WPA_SUPP
    ret = wlan_mbo_peferch_cfg(mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_supp_cfg.mbo_nonprefer_ch_params);
#else
    ret = wlan_mbo_peferch_cfg(mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.ch0,
                               mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.preference0,
                               mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.ch1,
                               mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.preference1);
#endif

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_NONPREFER_CH, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_NONPREFER_CH, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

#if CONFIG_WPA_SUPP
static int wlan_ncp_mbo_set_cell_capa(void *tlv)
{
    int ret;
    NCP_CMD_MBO_SET_CELL_CAPA *mbo_set_cell_capa = (NCP_CMD_MBO_SET_CELL_CAPA *)tlv;

    ret = wlan_mbo_set_cell_capa(mbo_set_cell_capa->cell_capa);

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_SET_CELL_CAPA, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_SET_CELL_CAPA, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}

static int wlan_ncp_mbo_set_oce(void *tlv)
{
    int ret;
    NCP_CMD_MBO_SET_OCE *mbo_set_oce = (NCP_CMD_MBO_SET_OCE *)tlv;

    ret = wlan_mbo_set_oce(mbo_set_oce->oce);

    if (!ret)
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_SET_OCE, NCP_CMD_RESULT_OK);
    else
        wlan_ncp_prepare_status(NCP_RSP_WLAN_MBO_SET_OCE, NCP_CMD_RESULT_ERROR);

    return WM_SUCCESS;
}
#endif
#endif /* CONFIG_DRIVER_MBO */

static int wlan_ncp_error_ack(void *tlv)
{
    wlan_ncp_prepare_status(NCP_RSP_INVALID_CMD, NCP_CMD_RESULT_ERROR);
    return WM_SUCCESS;
}
struct cmd_t error_ack_cmd = {0, "lookup cmd fail", wlan_ncp_error_ack, CMD_SYNC};

struct cmd_t wlan_cmd_sta[] = {
    {NCP_CMD_WLAN_STA_SCAN, "wlan-scan", wlan_ncp_scan, CMD_ASYNC},
    {NCP_CMD_WLAN_STA_CONNECT, "wlan-connect", wlan_ncp_connect, CMD_ASYNC},
    {NCP_CMD_WLAN_STA_DISCONNECT, "wlan-disconnect", wlan_ncp_disconnect, CMD_ASYNC},
    {NCP_CMD_WLAN_STA_VERSION, "wlan-version", wlan_ncp_version, CMD_SYNC},
    {NCP_CMD_WLAN_STA_SET_MAC, "wlan-set-mac", wlan_ncp_set_mac_address, CMD_SYNC},
    {NCP_CMD_WLAN_STA_GET_MAC, "wlan-get-mac", wlan_ncp_get_mac_address, CMD_SYNC},
    {NCP_CMD_WLAN_STA_CONNECT_STAT, "wlan-stat", wlan_ncp_stat, CMD_SYNC},
    {NCP_CMD_WLAN_STA_ROAMING, "wlan-roaming", wlan_ncp_set_roaming, CMD_SYNC},
    {NCP_CMD_WLAN_STA_CSI, "wlan-csi", wlan_ncp_csi, CMD_SYNC},
    {NCP_CMD_WLAN_STA_11K_CFG, "wlan-11k-enable", wlan_ncp_11k_enable, CMD_SYNC},
    {NCP_CMD_WLAN_STA_NEIGHBOR_REQ, "wlan-11k-neighbor-req", wlan_ncp_11k_neighbor_req, CMD_SYNC},
    {NCP_CMD_WLAN_STA_SIGNAL, "wlan-get-signal", wlan_ncp_get_signal, CMD_SYNC},
    {NCP_CMD_WLAN_STA_ANTENNA, "wlan-set-antenna-cfg", wlan_ncp_set_get_antenna_cfg, CMD_SYNC},
    {NCP_CMD_WLAN_STA_ANTENNA, "wlan-get-antenna-cfg", wlan_ncp_set_get_antenna_cfg, CMD_SYNC},
#if CONFIG_NCP_SUPP_WPS
    {NCP_CMD_WLAN_STA_WPS_PBC, "wlan-start-wps-pbc", wlan_ncp_start_wps_pbc, CMD_SYNC},
    {NCP_CMD_WLAN_STA_GEN_WPS_PIN, "wlan-generate-wps-pin", wlan_ncp_wps_generate_pin, CMD_SYNC},
    {NCP_CMD_WLAN_STA_WPS_PIN, "wlan-start-wps-pin", wlan_ncp_start_wps_pin, CMD_SYNC},
#endif
#if CONFIG_DRIVER_MBO
#if !CONFIG_WPA_SUPP
    {NCP_CMD_WLAN_MBO_ENABLE, "wlan-mbo-enable", wlan_ncp_mbo_enable, CMD_SYNC},
#endif
    {NCP_CMD_WLAN_MBO_NONPREFER_CH, "wlan-mbo-nonprefer-ch", wlan_ncp_mbo_nonprefer_ch, CMD_SYNC},
#if CONFIG_WPA_SUPP
    {NCP_CMD_WLAN_MBO_SET_CELL_CAPA, "wlan-mbo-set-cell-capa", wlan_ncp_mbo_set_cell_capa, CMD_SYNC},
    {NCP_CMD_WLAN_MBO_SET_OCE, "wlan-mbo-set-oce", wlan_ncp_mbo_set_oce, CMD_SYNC},
#endif
#endif
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_basic[] = {
    {NCP_CMD_WLAN_BASIC_WLAN_RESET, "wlan-reset", wlan_ncp_reset, CMD_SYNC},
    {NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_START, "wlan-uap-prov-start", wlan_ncp_uap_prov_start, CMD_ASYNC},
    {NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_RESET, "wlan-uap-prov-reset", wlan_ncp_uap_prov_reset, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_socket[] = {
    {NCP_CMD_WLAN_SOCKET_OPEN, "wlan-socket-open", wlan_ncp_socket_open, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_CON, "wlan-socket-connect", wlan_ncp_socket_connect, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_BIND, "wlan-socket-bind", wlan_ncp_socket_bind, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_CLOSE, "wlan-socket-close", wlan_nxp_socket_close, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_LISTEN, "wlan-socket-listen", wlan_ncp_socket_listen, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_ACCEPT, "wlan-socket-accept", wlan_ncp_socket_accept, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_SEND, "wlan-socket-send", wlan_ncp_socket_send, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_SENDTO, "wlan-socket-sendto", wlan_ncp_socket_sendto, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_RECV, "wlan-socket-receive", wlan_ncp_socket_receive, CMD_SYNC},
    {NCP_CMD_WLAN_SOCKET_RECVFROM, "wlan-socket-recvfrom", wlan_ncp_socket_recvfrom, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_http[] = {
    {NCP_CMD_WLAN_HTTP_CON, "wlan-http-connect", wlan_ncp_http_connect, CMD_SYNC},
    {NCP_CMD_WLAN_HTTP_DISCON, "wlan-http-disconnect", wlan_ncp_http_disconnect, CMD_SYNC},
    {NCP_CMD_WLAN_HTTP_REQ, "wlan-http-req", wlan_ncp_http_req, CMD_SYNC},
    {NCP_CMD_WLAN_HTTP_RECV, "wlan-http-recv", wlan_ncp_http_recv, CMD_SYNC},
    {NCP_CMD_WLAN_HTTP_SETH, "wlan-http-seth", wlan_ncp_http_seth, CMD_SYNC},
    {NCP_CMD_WLAN_HTTP_UNSETH, "wlan-http-unseth", wlan_ncp_http_unseth, CMD_SYNC},
    {NCP_CMD_WLAN_WEBSOCKET_UPG, "wlan-websocket-upgrade", wlan_ncp_websocket_upgrade, CMD_SYNC},
    {NCP_CMD_WLAN_WEBSOCKET_SEND, "wlan-websocket-send", wlan_ncp_websocket_send, CMD_SYNC},
    {NCP_CMD_WLAN_WEBSOCKET_RECV, "wlan-websocket-recv", wlan_ncp_websocket_recv, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_network[] = {
    {NCP_CMD_WLAN_NETWORK_INFO, "wlan-info", wlan_ncp_info, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_MONITOR, "wlan-monitor", wlan_ncp_monitor, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_ADD, "wlan-add", wlan_ncp_add, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_START, "wlan-start-network", wlan_ncp_start_network, CMD_ASYNC},
    {NCP_CMD_WLAN_NETWORK_STOP, "wlan-stop-network", wlan_ncp_stop_network, CMD_ASYNC},
    {NCP_CMD_WLAN_NETWORK_GET_UAP_STA_LIST, "wlan-get-uap-sta-list", wlan_ncp_get_uap_sta_list, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_MDNS_QUERY, "wlan-mdns-query", wlan_ncp_mdns_query, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_LIST, "wlan-list", wlan_ncp_network_list, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_REMOVE, "wlan-remove", wlan_ncp_network_remove, CMD_SYNC},
    {NCP_CMD_WLAN_NETWORK_ADDRESS, "wlan-address", wlan_ncp_address, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_powermgmt[] = {
#if CONFIG_MEF_CFG
    {NCP_CMD_WLAN_POWERMGMT_MEF, "wlan-multi-mef", wlan_ncp_multi_mef, CMD_SYNC},
#endif
    {NCP_CMD_WLAN_POWERMGMT_DEEP_SLEEP_PS, "wlan-deep-sleep-ps", wlan_ncp_deep_sleep_ps, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_IEEE_PS, "wlan-ieee-ps", wlan_ncp_ieee_ps, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_UAPSD, "wlan-uapsd-enable", wlan_ncp_wmm_uapsd, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_QOSINFO, "wlan-uapsd-qosinfo", wlan_ncp_uapsd_qosinfo, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_SLEEP_PERIOD, "wlan-uapsd-sleep-period", wlan_ncp_uapsd_sleep_period, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_WOWLAN_CFG, "wlan-wakeup-condition", wlan_ncp_wakeup_condition, CMD_SYNC},
    {NCP_CMD_WLAN_POWERMGMT_SUSPEND, "wlan-suspend", wlan_ncp_suspend, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_uap[] = {
    {NCP_CMD_WLAN_UAP_MAX_CLIENT_CNT, "wlan-set-max-clients-count", wlan_ncp_set_max_client_count, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_debug[] = {
    {NCP_CMD_WLAN_DEBUG_REGISTER_ACCESS, "wlan-reg-access", wlan_ncp_register_access, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_memory[] = {
    {NCP_CMD_WLAN_MEMORY_HEAP_SIZE, "wlan-mem-stat", wlan_ncp_memory_state, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_other[] = {
    {NCP_CMD_11AX_CFG, "wlan-11axcfg", wlan_ncp_11ax_cfg, CMD_SYNC},
    {NCP_CMD_BTWT_CFG, "wlan-bcast-twt", wlan_ncp_btwt_cfg, CMD_SYNC},
    {NCP_CMD_TWT_SETUP, "wlan-twt-setup", wlan_ncp_twt_setup, CMD_SYNC},
    {NCP_CMD_TWT_TEARDOWN, "wlan-twt-teardown", wlan_ncp_twt_teardown, CMD_SYNC},
    {NCP_CMD_TWT_GET_REPORT, "wlan-twt-report", wlan_ncp_twt_report, CMD_SYNC},
    {NCP_CMD_11D_ENABLE, "wlan-11d-enable", wlan_ncp_11d_enable, CMD_SYNC},
    {NCP_CMD_REGION_CODE, "wlan-set/get-regioncode", wlan_ncp_region_code, CMD_SYNC},
    {NCP_CMD_DATE_TIME, "wlan-set/get-time", wlan_ncp_date_time, CMD_SYNC},
    {NCP_CMD_GET_TEMPERATUE, "wlan-get-temperature", wlan_ncp_get_temperature, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t wlan_cmd_regulatory[] = {
    {NCP_CMD_WLAN_REGULATORY_ED_MAC_MODE, "wlan-set-ed-mac-mode", wlan_ncp_set_get_eu_mac_mode, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_ED_MAC_MODE, "wlan-get-ed-mac-mode", wlan_ncp_set_get_eu_mac_mode, CMD_SYNC},
#if CONFIG_NCP_RF_TEST_MODE
    {NCP_CMD_WLAN_REGULATORY_SET_RF_TEST_MODE, "wlan-set-rf-test-mode", wlan_ncp_set_rf_test_mode, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_TX_ANTENNA, "wlan-set-rf-tx-antenna", wlan_ncp_set_rf_tx_antenna,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_TX_ANTENNA, "wlan-get-rf-tx-antenna", wlan_ncp_get_rf_tx_antenna,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_RX_ANTENNA, "wlan-set-rf-rx-antenna", wlan_ncp_set_rf_rx_antenna,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_RX_ANTENNA, "wlan-get-rf-rx-antenna", wlan_ncp_get_rf_rx_antenna,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_BAND, "wlan-set-rf-band", wlan_ncp_set_rf_band, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_BAND, "wlan-get-rf-band", wlan_ncp_get_rf_band, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_BANDWIDTH, "wlan-set-rf-bandwidth", wlan_ncp_set_rf_bandwidth, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_BANDWIDTH, "wlan-get-rf-bandwidth", wlan_ncp_get_rf_bandwidth, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_CHANNEL, "wlan-set-rf-channel", wlan_ncp_set_rf_channel, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_CHANNEL, "wlan-get-rf-channel", wlan_ncp_get_rf_channel, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_RADIO_MODE, "wlan-set-rf-radio-mode", wlan_ncp_set_rf_radio_mode,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_RF_RADIO_MODE, "wlan-get-rf-radio-mode", wlan_ncp_get_rf_radio_mode,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_TX_POWER, "wlan-set-rf-tx-power", wlan_ncp_set_rf_tx_power, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_TX_CONT_MODE, "wlan-set-rf-tx-cont-mode", wlan_ncp_set_rf_tx_cont_mode,
     CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_SET_RF_TX_FRAME, "wlan-set-rf-tx-frame", wlan_ncp_set_rf_tx_frame, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_GET_AND_RESET_RF_PER, "wlan-get-and-reset-rf-per", wlan_ncp_get_and_reset_rf_per,
     CMD_SYNC},
#endif
    {NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_CCMP_128, "wlan-eu-crypto-ccmp-128", wlan_ncp_eu_crypto_ccmp_128, CMD_SYNC},
    {NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_GCMP_128, "wlan-eu-crypto-gcmp-128", wlan_ncp_eu_crypto_gcmp_128, CMD_SYNC},
    {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_subclass_t cmd_subclass_wlan[] = {
    {NCP_CMD_WLAN_STA, wlan_cmd_sta},
    {NCP_CMD_WLAN_BASIC, wlan_cmd_basic},
    {NCP_CMD_WLAN_NETWORK, wlan_cmd_network},
    {NCP_CMD_WLAN_SOCKET, wlan_cmd_socket},
    {NCP_CMD_WLAN_HTTP, wlan_cmd_http},
    {NCP_CMD_WLAN_POWERMGMT, wlan_cmd_powermgmt},
    {NCP_CMD_WLAN_REGULATORY, wlan_cmd_regulatory},
    {NCP_CMD_WLAN_UAP, wlan_cmd_uap},
    {NCP_CMD_WLAN_DEBUG, wlan_cmd_debug},
    {NCP_CMD_WLAN_MEMORY, wlan_cmd_memory},
    {NCP_CMD_WLAN_OTHER, wlan_cmd_other},
    {NCP_CMD_INVALID, NULL},
};


