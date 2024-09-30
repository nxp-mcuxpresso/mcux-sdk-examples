/* @file ncp_glue_wifi.h
 *
 *  @brief This file contains ncp API functions definitions
 *
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_GLUE_WIFI_H__
#define __NCP_GLUE_WIFI_H__

#include "wlan.h"
#include "wifi.h"
#include "mlan_api.h"
#include <wm_net.h>
#include <osa.h>
#include "ncp_cmd_wifi.h"
#include "mdns_service.h"

int wlan_ncp_prepare_status(uint32_t cmd, uint16_t result);

uint8_t *wlan_ncp_evt_status(uint32_t evt_id, void *msg);

int wlan_ncp_prepare_scan_result(NCP_CMD_SCAN_NETWORK_INFO *scan_res);

int wlan_ncp_prepare_connect_result(NCP_CMD_WLAN_CONN *conn_res);

int wlan_ncp_prepare_start_network_result(NCP_CMD_NETWORK_START *start_res);

int wlan_ncp_prepare_mac_address(void *mac_addr, uint8_t bss_type);

int wlan_ncp_prepare_mdns_result(mdns_result_ring_buffer_t *mdns_res);

uint8_t *wlan_ncp_prepare_mdns_resolve_result(ip_addr_t *ipaddr);

NCPCmd_DS_COMMAND *wlan_ncp_get_response_buffer();

int ncp_mdns_init(void);

void ncp_PostPowerSwitch(uint32_t mode, void *param);

void ncp_get_wifi_resp_buf_lock();
void ncp_put_wifi_resp_buf_lock();

#if CONFIG_NCP_DEBUG
void print_ncp_debug_time(void);
void add_ncp_debug_time_item(const char *func);
#endif

#if CONFIG_SCHED_SWITCH_TRACE
extern int ncp_debug_task_switch_start;
void trace_task_switch(int in, const char *func_name);
void trace_task_switch_print();
#endif

#if CONFIG_NCP_SOCKET_SEND_FIFO
#define SOCKET_SEND_COMMAND_NUM 64
/* app notify event queue message */
typedef struct
{
    uint32_t send_type;
    void *data;
} socket_send_msg_t;
#endif
#endif /* __NCP_GLUE_WIFI_H__ */
