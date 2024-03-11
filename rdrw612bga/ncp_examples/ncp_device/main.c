/** @file main.c
 *
 *  @brief main file
 *
 *  Copyright 2020 NXP
 *  All rights reserved.
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////

// SDK Included Files
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app.h"

#include "ncp_glue_wifi.h"
#include "ncp_config.h"

#ifdef CONFIG_NCP_WIFI
#include "serial_httpc.h"
#include "websockets.h"
#include "wlan_bt_fw.h"
#include "wlan.h"
#include "wifi.h"
#include "wm_net.h"
#include <wm_os.h>
#include "dhcp-server.h"
#include "uap_prov.h"
#include "app_notify.h"

#ifndef RW610
#include "wifi_bt_config.h"
#endif
#ifdef CONFIG_WIFI_USB_FILE_ACCESS
#include "usb_host_config.h"
#include "usb_host.h"
#include "usb_api.h"
#endif /* CONFIG_WIFI_USB_FILE_ACCESS */
#include "cli_utils.h"
#endif

#ifdef CONFIG_NCP_BRIDGE_DEBUG
#include "cli.h"
#endif
#include "crc.h"
#include "fsl_rtc.h"
#include "fsl_power.h"
#ifdef CONFIG_HOST_SLEEP
#include "host_sleep.h"
#endif

#include "ncp_tlv_adapter.h"
#include "fsl_os_abstraction.h"

#include "fsl_device_registers.h"
#ifdef CONFIG_WIFI_USB_FILE_ACCESS
#include "usb_support.h"
#endif
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define NCP_BRIDGE_INBUF_SIZE     4096
#define WM_SUCCESS 0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
#ifdef CONFIG_WIFI_USB_FILE_ACCESS
extern usb_host_handle g_HostHandle;
#endif
extern int network_services;

uint32_t current_cmd = 0;

#ifdef RW610
extern const unsigned char *wlan_fw_bin;
extern const unsigned int wlan_fw_bin_len;
#endif

typedef struct
{
    uint32_t block_type;
    uint32_t command_sz;
    void     *cmd_buff;
} system_ncp_command_t, wifi_ncp_command_t;

#define SYSTEM_TASK_PRIO        3
#define SYSTEM_NCP_STACK_SIZE   2048

static OSA_TASK_HANDLE_DEFINE(system_ncp_handle);

static void system_ncp_task(void *pvParameters);
OSA_TASK_DEFINE(system_ncp_task, SYSTEM_TASK_PRIO, 1, SYSTEM_NCP_STACK_SIZE, 0);

OSA_SEMAPHORE_HANDLE_DEFINE(system_ncp_lock);

#define SYSTEM_NCP_COMMAND_QUEUE_NUM 8
static osa_msgq_handle_t system_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(system_ncp_command_queue_buff, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));

#if defined (CONFIG_NCP_WIFI) && !defined (CONFIG_NCP_BLE)
#define TASK_MAIN_PRIO         configMAX_PRIORITIES - 4
#else
#define TASK_MAIN_PRIO         OSA_TASK_PRIORITY_MIN - 2
#endif
#define TASK_MAIN_STACK_SIZE   2048
void task_main(osa_task_param_t arg);
OSA_TASK_DEFINE(task_main, TASK_MAIN_PRIO, 1, TASK_MAIN_STACK_SIZE, 0);
void ncp_tlv_process(osa_task_param_t arg);

OSA_SEMAPHORE_HANDLE_DEFINE(wifi_ncp_lock);
#ifdef CONFIG_NCP_WIFI
os_thread_t wifi_ncp_thread;                         /* ncp bridge  task */
static os_thread_stack_define(wifi_ncp_stack, 6144); /* ncp bridge task stack*/

static struct wlan_network sta_network;
static struct wlan_network uap_network;
#endif

uint16_t g_cmd_seqno = 0;

uint8_t cmd_buf[NCP_BRIDGE_INBUF_SIZE];
uint8_t res_buf[NCP_BRIDGE_INBUF_SIZE];

#ifndef CONFIG_CRC32_HW_ACCELERATE
static unsigned int crc32_table[256];
#endif
extern bool usart_suspend_flag;
#ifdef CONFIG_NCP_WIFI
extern power_cfg_t global_power_config;
#endif
os_mutex_t resp_buf_mutex;

#ifdef CONFIG_NCP_WIFI
/*WIFI NCP COMMAND TASK*/
#define WIFI_NCP_COMMAND_QUEUE_NUM 160
static osa_msgq_handle_t wifi_ncp_command_queue; /* ncp adapter TX msgq */
OSA_MSGQ_HANDLE_DEFINE(wifi_ncp_command_queue_buff, WIFI_NCP_COMMAND_QUEUE_NUM,  sizeof(wifi_ncp_command_t));
#endif

#ifdef CONFIG_NCP_BLE
extern int ble_ncp_init(void);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

#ifdef CONFIG_WIFI_USB_FILE_ACCESS

void USBHS_IRQHandler(void)
{
    USB_HostEhciIsrFunction(g_HostHandle);
}

void USB_HostClockInit(void)
{
    /* reset USB */
    RESET_PeripheralReset(kUSB_RST_SHIFT_RSTn);
    /* enable usb clock */
    CLOCK_EnableClock(kCLOCK_Usb);
    /* enable usb phy clock */
    CLOCK_EnableUsbhsPhyClock();
}

void USB_HostIsrEnable(void)
{
    uint8_t irqNumber;

    uint8_t usbHOSTEhciIrq[] = USBHS_IRQS;
    irqNumber                = usbHOSTEhciIrq[CONTROLLER_ID - kUSB_ControllerEhci0];
    /* USB_HOST_CONFIG_EHCI */

    /* Install isr, set priority, and enable IRQ. */
    NVIC_SetPriority((IRQn_Type)irqNumber, USB_HOST_INTERRUPT_PRIORITY);
    EnableIRQ((IRQn_Type)irqNumber);
}

void USB_HostTaskFn(void *param)
{
    USB_HostEhciTaskFunction(param);
}
#endif

static void printSeparator(void)
{
    PRINTF("========================================\r\n");
}

/* system_ncp_send_response() handles the response from the wifi driver.
 * This involves
 * 1) sending cmd response out to interface
 * 2) computation of the crc of the cmd resp
 * 3) reset cmd_buf & res_buf
 * 4) release bridge lock
 */
int system_ncp_send_response(uint8_t *pbuf)
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
        OSA_SemaphorePost(system_ncp_lock);
        ncp_d("put bridge lock");
    }

    return ret;
}

static int system_ncp_command_handle_input(uint8_t *cmd)
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
        ncp_d("ncp system lookup cmd failed\r\n");
        return -WM_FAIL;
    }
    ncp_d("ncp system got command: <%s>", command->help);
    ret = command->handler(cmd_tlv);

    if (command->async == CMD_SYNC)
    {
         system_ncp_send_response(res_buf);
    }
    else
    {
        /* Wait for cmd to execute, then
         * 1) send cmd response
         * 2) reset cmd_buf & res_buf
         * 3) release system_ncp_lock */
    }

    return ret;
}

static void system_ncp_task(void *pvParameters)
{
    int ret = 0;
    system_ncp_command_t cmd_item;
    uint8_t *cmd_buf = NULL;
    while (1)
    {
        ret = OSA_MsgQGet(system_ncp_command_queue, &cmd_item, osaWaitForever_c);
        if (ret != WM_SUCCESS)
        {
            ncp_e("system ncp command queue receive failed");
            continue;
        }
        else
        {
            cmd_buf = cmd_item.cmd_buff;
            system_ncp_command_handle_input(cmd_buf);
            OSA_MemoryFree(cmd_buf);
            cmd_buf = NULL;
        }
    }
}

static void system_ncp_callback(void *tlv, size_t tlv_sz, int status)
{
    int ret = 0;
    system_ncp_command_t cmd_item;

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

    ret = OSA_MsgQPut(system_ncp_command_queue, &cmd_item);
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

static int system_ncp_init(void)
{
    int ret;
    system_ncp_command_queue = (osa_msgq_handle_t)system_ncp_command_queue_buff;

    ret = ncp_config_init();
    assert(WM_SUCCESS == ret);

    ret = OSA_MsgQCreate(system_ncp_command_queue, SYSTEM_NCP_COMMAND_QUEUE_NUM,  sizeof(system_ncp_command_t));
    if (ret != WM_SUCCESS)
    {
        ncp_e("failed to create system ncp command queue: %d", ret);
        return -WM_FAIL;
    }

    ret = OSA_SemaphoreCreateBinary(system_ncp_lock);
    if (ret != kStatus_Success)
    {
        ncp_e("failed to create system_ncp_lock: %d", ret);
        return ret;
    }
    ncp_tlv_install_handler(GET_CMD_CLASS(NCP_BRIDGE_CMD_SYSTEM), (void *)system_ncp_callback);
    ret = OSA_TaskCreate((osa_task_handle_t) system_ncp_handle, OSA_TASK(system_ncp_task), NULL);
    if (ret != KOSA_StatusSuccess)
    {
        ncp_e("failed to create ncp system task: %d", ret);
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

#ifdef CONFIG_NCP_WIFI
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
#endif

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

#ifdef CONFIG_NCP_WIFI
static int wifi_ncp_init(void)
{
    int ret;
    wifi_ncp_command_queue = (osa_msgq_handle_t)wifi_ncp_command_queue_buff;

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
#endif

void task_main(void *param)
{
    int32_t result = 0;
#ifdef CONFIG_NCP_BRIDGE_DEBUG
    PRINTF("Initialize CLI\r\n");
    printSeparator();
    result = cli_init();

    assert(WM_SUCCESS == result);

#endif

    PRINTF("Initialize NCP config littlefs CLIs\r\n");
    printSeparator();
    result = ncp_adapter_init();
    assert(WM_SUCCESS == result);
    result = system_ncp_init();
    assert(WM_SUCCESS == result);
#ifdef CONFIG_NCP_WIFI
    result = wifi_ncp_init();
#elif defined(CONFIG_NCP_BLE)
    result = ble_ncp_init();
#endif
    assert(WM_SUCCESS == result);

    result = ncp_cmd_list_init();
    assert(WM_SUCCESS == result);

    printSeparator();
#ifdef CONFIG_HOST_SLEEP
    hostsleep_init();
#endif

    while (1)
    {
        /* wait for interface up */
        OSA_TimeDelay(5000);
    }
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static OSA_TASK_HANDLE_DEFINE(main_task_handle);
int main(void)
{
    BaseType_t result = 0;
    (void)result;
    BOARD_InitBootPins();
    if (BOARD_IS_XIP())
    {
        BOARD_BootClockLPR();
        CLOCK_EnableClock(kCLOCK_Otp);
        CLOCK_EnableClock(kCLOCK_Els);
        CLOCK_EnableClock(kCLOCK_ElsApb);
        RESET_PeripheralReset(kOTP_RST_SHIFT_RSTn);
        RESET_PeripheralReset(kELS_APB_RST_SHIFT_RSTn);
    }
    else
    {
        BOARD_InitBootClocks();
    }
    BOARD_InitDebugConsole();
    /* Reset GMDA */
    RESET_PeripheralReset(kGDMA_RST_SHIFT_RSTn);
    /* Keep CAU sleep clock here. */
    /* CPU1 uses Internal clock when in low power mode. */
    POWER_ConfigCauInSleep(false);
    BOARD_InitSleepPinConfig();

    RTC_Init(RTC);

    printSeparator();
    PRINTF("wifi supplicant demo\r\n");
    printSeparator();

#ifdef CONFIG_WIFI_USB_FILE_ACCESS
    usb_init();
#endif
    (void)OSA_TaskCreate((osa_task_handle_t)main_task_handle, OSA_TASK(task_main), NULL);

    OSA_Start();
    for (;;)
        ;
}
