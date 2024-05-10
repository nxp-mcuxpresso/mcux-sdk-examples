/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "fsl_os_abstraction.h"
#include "serial_httpc.h"
#include "websockets.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"

#ifndef RW610
#include "wifi_bt_config.h"
#endif
#ifdef CONFIG_WIFI_USB_FILE_ACCESS
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_api.h"
#endif /* CONFIG_WIFI_USB_FILE_ACCESS */

#include "cli_utils.h"

#include "ncp_glue_wifi.h"
#include "ncp_wifi.h"
#include "ncp_config.h"
#include "uap_prov.h"
#include "app_notify.h"

#ifdef CONFIG_NCP_BRIDGE_DEBUG
#include "cli.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef struct ncp_cmd_t wifi_ncp_command_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef RW610
extern const unsigned char *wlan_fw_bin;
extern const unsigned int wlan_fw_bin_len;
#endif

extern int network_services;
extern uint16_t g_cmd_seqno;
extern uint8_t cmd_buf[NCP_BRIDGE_INBUF_SIZE];
extern uint8_t res_buf[NCP_BRIDGE_INBUF_SIZE];

/*WIFI NCP COMMAND TASK*/
#define WIFI_NCP_COMMAND_QUEUE_NUM 160
static osa_msgq_handle_t wifi_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(wifi_ncp_command_queue_buff, WIFI_NCP_COMMAND_QUEUE_NUM,  sizeof(wifi_ncp_command_t));

OSA_SEMAPHORE_HANDLE_DEFINE(wifi_ncp_lock);

os_thread_t wifi_ncp_thread;                         /* ncp bridge  task */
static os_thread_stack_define(wifi_ncp_stack, 6144); /* ncp bridge task stack*/

static struct wlan_network sta_network;
static struct wlan_network uap_network;

extern uint32_t current_cmd;

/*******************************************************************************
 * Code
 ******************************************************************************/

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

/* wifi_ncp_send_response() handles the response from the wifi driver.
 * This involves
 * 1) sending cmd response out to interface
 * 2) computation of the crc of the cmd resp
 * 3) reset cmd_buf & res_buf
 * 4) release bridge lock
 */
int wifi_ncp_send_response(uint8_t *pbuf)
{
    int ret                = WM_SUCCESS;
    uint16_t transfer_len = 0;
    NCP_BRIDGE_COMMAND *res = (NCP_BRIDGE_COMMAND *)pbuf;

    /* set cmd seqno */
    res->seqnum = g_cmd_seqno;
    transfer_len        = res->size;
    if (transfer_len >= NCP_BRIDGE_CMD_HEADER_LEN)
    {
        /* write response to host */
        ncp_tlv_send(pbuf, transfer_len);
        if (ret != WM_SUCCESS)
        {
            ncp_e("failed to write response");
            ret = -WM_FAIL;
        }
    }
    else
    {
        ncp_e("command length is less than 12, cmd_len = %d", transfer_len);
        ret = -WM_FAIL;
    }

    if (res->msg_type != NCP_BRIDGE_MSG_TYPE_EVENT)
    {
        /* Reset cmd_buf */
        memset(cmd_buf, 0, sizeof(cmd_buf));
        /* Reset res_buf */
        memset(res_buf, 0, sizeof(res_buf));
        OSA_SemaphorePost(wifi_ncp_lock);
        ncp_d("put bridge lock");
    }

    return ret;
}

static int wifi_ncp_command_handle_input(uint8_t *cmd)
{
    NCP_BRIDGE_COMMAND *input_cmd = (NCP_BRIDGE_COMMAND *)cmd;
    struct cmd_t *command         = NULL;
    int ret                       = WM_SUCCESS;

    uint32_t cmd_class    = GET_CMD_CLASS(input_cmd->cmd);
    uint32_t cmd_subclass = GET_CMD_SUBCLASS(input_cmd->cmd);
    uint32_t cmd_id       = GET_CMD_ID(input_cmd->cmd);
    void *cmd_tlv         = GET_CMD_TLV(input_cmd);
    current_cmd = cmd_id;

    command = lookup_class(cmd_class, cmd_subclass, cmd_id);
    if (NULL == command)
    {
        ncp_d("ncp wifi lookup cmd failed\r\n");
        return -WM_FAIL;
    }
    ncp_d("ncp wifi got command: <%s>", command->help);
    ret = command->handler(cmd_tlv);

    if (command->async == CMD_SYNC)
    {
         wifi_ncp_send_response(res_buf);
    }
    else
    {
        /* Wait for cmd to execute, then
         * 1) send cmd response
         * 2) reset cmd_buf & res_buf
         * 3) release wifi_ncp_lock */
    }

    return ret;
}

static void wifi_ncp_task(void *pvParameters)
{
    int ret = 0;
    wifi_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(wifi_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != WM_SUCCESS)
        {
            ncp_e("wifi ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            if(((NCP_BRIDGE_COMMAND *)cmd_buf)->cmd != NCP_BRIDGE_CMD_WLAN_POWERMGMT_MCU_SLEEP_CFM)
                OSA_SemaphoreWait(wifi_ncp_lock, osaWaitForever_c);
            wifi_ncp_command_handle_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

static void wifi_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    wifi_ncp_command_t cmd_item;

    cmd_item.block_type = 0;
    cmd_item.command_sz = tlv_sz;
    cmd_item.cmd_buff = (ncp_tlv_qelem_t *)OSA_MemoryAllocate(tlv_sz);
    if (!cmd_item.cmd_buff)
    {
        NCP_TLV_STATS_INC(drop);
        ncp_adap_d("%s: failed to allocate memory for tlv queue element", __FUNCTION__);
        return ;
    }
    memcpy(cmd_item.cmd_buff, tlv, tlv_sz);

    ret = OSA_MsgQPut(wifi_ncp_command_queue, &cmd_item);
    if (ret != kStatus_Success)
    {
        if (cmd_item.cmd_buff)
        {
            OSA_MemoryFree(cmd_item.cmd_buff);
            cmd_item.cmd_buff = NULL;
        }
        ncp_e("send to wifi ncp cmd queue failed");
    }
    else
        ncp_d("success to send ncp command on queue");
}

/* Callback Function passed to WLAN Connection Manager. The callback function
 * gets called when there are WLAN Events that need to be handled by the
 * application.
 */
int wlan_event_callback(enum wlan_event_reason reason, void *data)
{
#ifdef CONFIG_NCP_WIFI
    int ret;
    static int auth_fail = 0;
    struct wlan_ip_config addr;
    char ip[16];

    printSeparator();
    PRINTF("app_cb: WLAN: received event %d\r\n", reason);
    printSeparator();

    if (check_valid_status_for_uap_prov() && check_valid_event_for_uap_prov(reason))
    {
        send_msg_to_uap_prov(MSG_TYPE_EVT, reason, (int)data);
    }

    switch (reason)
    {
        case WLAN_REASON_INITIALIZED:
            PRINTF("app_cb: WLAN initialized\r\n");
            printSeparator();

#ifdef CONFIG_NCP_BRIDGE_DEBUG
            ret = wlan_basic_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize BASIC WLAN CLIs\r\n");
                return 0;
            }

            ret = wlan_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("WLAN CLIs are initialized\r\n");
            printSeparator();

#ifdef CONFIG_HOST_SLEEP
            ret = host_sleep_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN host sleep CLIs\r\n");
                return 0;
            }
            PRINTF("HOST SLEEP CLIs are initialized\r\n");
            printSeparator();
#endif

            ret = wlan_enhanced_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize WLAN CLIs\r\n");
                return 0;
            }
            PRINTF("ENHANCED WLAN CLIs are initialized\r\n");
            printSeparator();

            ret = dhcpd_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize DHCP Server CLI\r\n");
                return 0;
            }
#endif
            ret = uap_prov_cli_init();
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to initialize UAP PROV CLI\r\n");
                return 0;
            }

            ret = ncp_bridge_mdns_init();
            if (ret != WM_SUCCESS)
            {
                (void)PRINTF("Failed to initialize mDNS\r\n");
                return 0;
            }
            (void)PRINTF("mDNS are initialized\r\n");
            printSeparator();
            break;
        case WLAN_REASON_INITIALIZATION_FAILED:
            PRINTF("app_cb: WLAN: initialization failed\r\n");
            break;
        case WLAN_REASON_SUCCESS:
            PRINTF("app_cb: WLAN: connected to network\r\n");
            ret = wlan_get_address(&addr);
            if (ret != WM_SUCCESS)
            {
                PRINTF("failed to get IP address\r\n");
                return 0;
            }

            net_inet_ntoa(addr.ipv4.address, ip);

            ret = wlan_get_current_network(&sta_network);
            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get External AP network\r\n");
                return 0;
            }
            PRINTF("Connected to following BSS:\r\n");
            PRINTF("SSID = [%s]\r\n", sta_network.ssid);
            if (addr.ipv4.address != 0U)
            {
                PRINTF("IPv4 Address: [%s]\r\n", ip);
            }
#ifdef CONFIG_IPV6
            int i;
            for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
            {
                if (ip6_addr_isvalid(addr.ipv6[i].addr_state))
                {
                    (void)PRINTF("IPv6 Address: %-13s:\t%s (%s)\r\n",
                                 ipv6_addr_type_to_desc((struct net_ipv6_config *)&addr.ipv6[i]),
                                 inet6_ntoa(addr.ipv6[i].address), ipv6_addr_state_to_desc(addr.ipv6[i].addr_state));
                }
            }
            (void)PRINTF("\r\n");
#endif
#ifdef MDNS_STA_INTERFACE
            if (!network_services)
            {
                ret = app_mdns_register_iface(net_get_sta_handle());
                if (ret != WM_SUCCESS)
                    (void)PRINTF("Error in registering mDNS STA interface\r\n");
                else
                {
                    (void)PRINTF("mDNS STA Interface successfully registered\r\n");
                    network_services = 1;
                }
            }
            else
            {
                app_mdns_resp_restart(net_get_sta_handle());
            }
#endif
            NCP_CMD_WLAN_CONN *conn_res = (NCP_CMD_WLAN_CONN *)os_mem_alloc(sizeof(NCP_CMD_WLAN_CONN));
            if (conn_res == NULL)
            {
                app_notify_event(APP_EVT_USER_CONNECT, APP_EVT_REASON_FAILURE, NULL, 0);
            }
            else
            {
                conn_res->ip = addr.ipv4.address;
                (void)memcpy(conn_res->ssid, sta_network.ssid, sizeof(sta_network.ssid));
                app_notify_event(APP_EVT_USER_CONNECT, APP_EVT_REASON_SUCCESS, conn_res, sizeof(NCP_CMD_WLAN_CONN));
            }
            auth_fail = 0;
            break;
        case WLAN_REASON_CONNECT_FAILED:
            PRINTF("app_cb: WLAN: connect failed\r\n");
            app_notify_event(APP_EVT_USER_CONNECT, APP_EVT_REASON_FAILURE, NULL, 0);
            break;
        case WLAN_REASON_NETWORK_NOT_FOUND:
            PRINTF("app_cb: WLAN: network not found\r\n");
            break;
        case WLAN_REASON_NETWORK_AUTH_FAILED:
            PRINTF("app_cb: WLAN: network authentication failed\r\n");
            auth_fail++;
            if (auth_fail >= 3)
            {
                PRINTF("Authentication Failed. Disconnecting ... \r\n");
                wlan_disconnect();
                auth_fail = 0;
            }
#ifdef MDNS_STA_INTERFACE
            ret = app_mdns_deregister_iface(net_get_sta_handle());
            if (ret != WM_SUCCESS)
                (void)PRINTF("Error in deregistering mDNS STA interface\r\n");
            else
                (void)PRINTF("mDNS STA Interface successfully deregistered\r\n");
#endif
            break;
        case WLAN_REASON_ADDRESS_SUCCESS:
            PRINTF("network mgr: DHCP new lease\r\n");
#ifdef MDNS_STA_INTERFACE
            app_mdns_resp_restart(net_get_sta_handle());
#endif
            break;
        case WLAN_REASON_ADDRESS_FAILED:
            PRINTF("app_cb: failed to obtain an IP address\r\n");
            break;
        case WLAN_REASON_USER_DISCONNECT:
            PRINTF("app_cb: disconnected\r\n");
#ifdef MDNS_STA_INTERFACE
            ret = app_mdns_deregister_iface(net_get_sta_handle());
            if (ret != WM_SUCCESS)
                (void)PRINTF("Error in deregistering mDNS STA interface\r\n");
            else
            {
                network_services = 0;
                (void)PRINTF("mDNS STA Interface successfully deregistered\r\n");
            }
#endif
            if (!data)
                app_notify_event(APP_EVT_USER_DISCONNECT, APP_EVT_REASON_SUCCESS, NULL, 0);
            else
                app_notify_event(APP_EVT_USER_DISCONNECT, (int)data ? APP_EVT_REASON_FAILURE : APP_EVT_REASON_SUCCESS,
                                 NULL, 0);
            auth_fail = 0;
            break;
        case WLAN_REASON_LINK_LOST:
            PRINTF("app_cb: WLAN: link lost\r\n");
            break;
        case WLAN_REASON_CHAN_SWITCH:
            PRINTF("app_cb: WLAN: channel switch\r\n");
            break;
        case WLAN_REASON_UAP_SUCCESS:
            PRINTF("app_cb: WLAN: UAP Started\r\n");
            ret = wlan_get_current_uap_network(&uap_network);

            if (ret != WM_SUCCESS)
            {
                PRINTF("Failed to get Soft AP network\r\n");
                return 0;
            }
            printSeparator();
            PRINTF("Soft AP \"%s\" started successfully\r\n", uap_network.ssid);
            printSeparator();
            if (dhcp_server_start(net_get_uap_handle()))
                PRINTF("Error in starting dhcp server\r\n");

            PRINTF("DHCP Server started successfully\r\n");
            printSeparator();
            NCP_CMD_NETWORK_START *start_res = (NCP_CMD_NETWORK_START *)os_mem_alloc(sizeof(NCP_CMD_NETWORK_START));
            (void)memcpy(start_res->ssid, uap_network.ssid, sizeof(uap_network.ssid));
            app_notify_event(APP_EVT_USER_START_NETWORK, APP_EVT_REASON_SUCCESS, start_res,
                             sizeof(NCP_CMD_NETWORK_START));

            ret = app_mdns_register_iface(net_get_uap_handle());
            if (ret != WM_SUCCESS)
                (void)PRINTF("Error in registering mDNS uAP interface\r\n");
            else
                (void)PRINTF("mDNS uAP Interface successfully registered\r\n");
            printSeparator();

            break;
        case WLAN_REASON_UAP_CLIENT_ASSOC:
            PRINTF("app_cb: WLAN: UAP a Client Associated\r\n");
            printSeparator();
            PRINTF("Client => ");
            print_mac((const char *)data);
            PRINTF("Associated with Soft AP\r\n");
            printSeparator();
            break;
        case WLAN_REASON_UAP_START_FAILED:
            PRINTF("app_cb: WLAN: UAP start failed\r\n");
            app_notify_event(APP_EVT_USER_START_NETWORK, APP_EVT_REASON_FAILURE, NULL, 0);
            break;
        case WLAN_REASON_UAP_STOP_FAILED:
            PRINTF("app_cb: WLAN: UAP stop failed\r\n");
            app_notify_event(APP_EVT_USER_STOP_NETWORK, APP_EVT_REASON_FAILURE, NULL, 0);
            break;
        case WLAN_REASON_UAP_STOPPED:
            PRINTF("app_cb: WLAN: UAP Stopped\r\n");
            printSeparator();
            PRINTF("Soft AP \"%s\" stopped successfully\r\n", uap_network.ssid);
            printSeparator();

            dhcp_server_stop();

            PRINTF("DHCP Server stopped successfully\r\n");
            printSeparator();
            app_notify_event(APP_EVT_USER_STOP_NETWORK, APP_EVT_REASON_SUCCESS, NULL, 0);

            ret = app_mdns_deregister_iface(net_get_uap_handle());
            if (ret != WM_SUCCESS)
                (void)PRINTF("Error in deregistering mDNS uAP interface\r\n");
            else
                (void)PRINTF("mDNS uAP Interface successfully deregistered\r\n");
            printSeparator();
            break;
        case WLAN_REASON_PS_ENTER:
            PRINTF("app_cb: WLAN: PS_ENTER\r\n");
            break;
        case WLAN_REASON_PS_EXIT:
            PRINTF("app_cb: WLAN: PS EXIT\r\n");
            break;
        case WLAN_REASON_WPS_SESSION_DONE:
            PRINTF("app_cb: WLAN: WPS session done\r\n");
            app_notify_event(APP_EVT_WPS_DONE, APP_EVT_REASON_SUCCESS, data, sizeof(struct wlan_network));
            break;
        default:
            PRINTF("app_cb: WLAN: Unknown Event: %d\r\n", reason);
    }
#endif
    return 0;
}

#ifdef CONFIG_NCP_BRIDGE_DEBUG
static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

int wifi_ncp_cli_init(void)
{
    int32_t result = 0;

    PRINTF("Initialize CLI\r\n");
    printSeparator();
    result = cli_init();

    return result;
}
#endif

int wifi_ncp_init(void)
{
    int ret;
    wifi_ncp_command_queue = (osa_msgq_handle_t)wifi_ncp_command_queue_buff;

#ifdef CONFIG_NCP_BRIDGE_DEBUG
    ret = wifi_ncp_cli_init();
    if (ret != WM_SUCCESS)
    {
        app_e("failed to init wifi cli: %d", ret);
        return -WM_FAIL;
    }
#endif

    ret = OSA_MsgQCreate(wifi_ncp_command_queue, WIFI_NCP_COMMAND_QUEUE_NUM,  sizeof(wifi_ncp_command_t));
    if (ret != WM_SUCCESS)
    {
        app_e("failed to create wifi ncp command queue: %d", ret);
        return -WM_FAIL;
    }

    ret = OSA_SemaphoreCreateBinary(wifi_ncp_lock);
    if (ret != kStatus_Success)
    {
        ncp_e("failed to create wifi_ncp_lock: %d", ret);
        return ret;
    }
    else
    {
        OSA_SemaphorePost(wifi_ncp_lock);
    }

    ncp_tlv_install_handler(GET_CMD_CLASS(NCP_BRIDGE_CMD_WLAN), (void *)wifi_ncp_callback);
    ret = os_thread_create(&wifi_ncp_thread, "wifi_ncp_task", wifi_ncp_task, 0, &wifi_ncp_stack, OS_PRIO_3);
    if (ret != WM_SUCCESS)
    {
        ncp_e("failed to create ncp wifi task: %d", ret);
        return -WM_FAIL;
    }

    ret = app_notify_init();
    if (ret != WM_SUCCESS)
    {
        ncp_e("app notify failed to initialize: %d", ret);
        return -WM_FAIL;
    }

    PRINTF("Initialize WLAN Driver\r\n");

    /* Initialize WIFI Driver */
    ret = wlan_init(wlan_fw_bin, wlan_fw_bin_len);
    if (ret != WM_SUCCESS)
    {
        ncp_e("wlan_init fail: %d", ret);
        return -WM_FAIL;
    }

    ret = wlan_start(wlan_event_callback);
    if (ret != WM_SUCCESS)
    {
        ncp_e("wlan_start fail: %d", ret);
        return -WM_FAIL;
    }
    return WM_SUCCESS;
}
