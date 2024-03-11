/* @file ncp_bridge_glue.h
 *
 *  @brief This file contains ncp bridge API functions definitions
 *
 *  Copyright 2008-2023 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#ifndef __NCP_BRIDGE_GLUE_H__
#define __NCP_BRIDGE_GLUE_H__

#include "wlan.h"
#include "wifi.h"
#include "mlan_api.h"
#include <wm_net.h>
#include <wm_os.h>
#include "ncp_cmd_wifi.h"
#include "mdns_service.h"

int wlan_bridge_prepare_status(uint32_t cmd, uint16_t result);

uint8_t *wlan_bridge_evt_status(uint32_t evt_id, void *msg);

int wlan_bridge_prepare_scan_result(NCP_CMD_SCAN_NETWORK_INFO *scan_res);

int wlan_bridge_prepare_connect_result(NCP_CMD_WLAN_CONN *conn_res);

int wlan_bridge_prepare_start_network_result(NCP_CMD_NETWORK_START *start_res);

int wlan_bridge_prepare_mac_address(void *mac_addr, uint8_t bss_type);

int wlan_bridge_prepare_mdns_result(mdns_result_ring_buffer_t *mdns_res);

uint8_t *wlan_bridge_prepare_mdns_resolve_result(ip_addr_t *ipaddr);

NCPCmd_DS_COMMAND *ncp_bridge_get_response_buffer();

int ncp_bridge_mdns_init(void);

void bridge_PostPowerSwitch(uint32_t mode, void *param);

#ifdef CONFIG_NCP_BRIDGE_DEBUG
void print_ncp_debug_time(void);
void add_ncp_debug_time_item(const char *func);
#endif

#ifdef CONFIG_SCHED_SWITCH_TRACE
extern int ncp_debug_task_switch_start;
void trace_task_switch(int in, const char *func_name);
void trace_task_switch_print();
#endif

#ifdef CONFIG_NCP_SOCKET_SEND_FIFO
#define SOCKET_SEND_COMMAND_NUM 64
/* app notify event queue message */
typedef struct
{
    uint32_t send_type;
    void *data;
} socket_send_msg_t;
#endif
#endif /* __NCP_BRIDGE_GLUE_H__ */
