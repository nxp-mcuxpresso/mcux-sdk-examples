/*!\file ncp_cmd_wifi.h
 *\brief This file provides NCP Wi-Fi command/response definitions.
 */
/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_CMD_WIFI_H__
#define __NCP_CMD_WIFI_H__

#include "ncp_cmd_common.h"

/** NCP Wi-Fi subclass typse */
/** subclass type for Wi-Fi station commands, such as scan, connect,version, set mac and so on. */
#define NCP_CMD_WLAN_STA         0x00000000
/** subclass type for basic command such as reset, reset UAP provisioning and start UAP provisioning. */
#define NCP_CMD_WLAN_BASIC       0x00100000
/** subclass type for regulatory commands, such as set RF TX, RX antenna mode and RF band. */
#define NCP_CMD_WLAN_REGULATORY  0x00200000
/** subclass type for power managerment commands, such as enable/disable deep sleep power save mode. */
#define NCP_CMD_WLAN_POWERMGMT   0x00300000
/** subclass type for debug commands. */
#define NCP_CMD_WLAN_DEBUG       0x00400000
/** subclass type for TWT (Target wake-up time), 802.11ax and region code commands. */
#define NCP_CMD_WLAN_OTHER       0x00500000
/** subclass type for memory commands. */
#define NCP_CMD_WLAN_MEMORY      0x00600000
/** subclass type for network commands, such as add, start, stop and remove network command. */
#define NCP_CMD_WLAN_NETWORK     0x00700000
/** subclass type for socket command, such as socket open, connect, close and send. */
#define NCP_CMD_WLAN_SOCKET      0x00900000
/** subclass type for UAP commands */
#define NCP_CMD_WLAN_UAP         0x00a00000
/** subclass type for http commands */
#define NCP_CMD_WLAN_HTTP        0x00b00000
/** subclass type for async events. */
#define NCP_CMD_WLAN_ASYNC_EVENT 0x00f00000


/** Maximum size of the network list */
#define NCP_WLAN_KNOWN_NETWORKS 5

/** Wi-Fi STA scan command ID */
#define NCP_CMD_WLAN_STA_SCAN          (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-scan */
/** Wi-Fi STA scan command response ID */
#define NCP_RSP_WLAN_STA_SCAN          (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi STA connect command ID */
#define NCP_CMD_WLAN_STA_CONNECT       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-connect */
/** Wi-Fi STA connect command response ID */
#define NCP_RSP_WLAN_STA_CONNECT       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP |0x00000002)
/** Wi-Fi STA disconnect command ID */
#define NCP_CMD_WLAN_STA_DISCONNECT    (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-disconnect */
/** Wi-Fi STA disconnect command response ID */
#define NCP_RSP_WLAN_STA_DISCONNECT    (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi STA version command ID */
#define NCP_CMD_WLAN_STA_VERSION       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-version */
/** Wi-Fi STA version command response ID */
#define NCP_RSP_WLAN_STA_VERSION       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi STA set MAC address command ID */
#define NCP_CMD_WLAN_STA_SET_MAC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-set-mac */
/** Wi-Fi STA set MAC address command response ID */
#define NCP_RSP_WLAN_STA_SET_MAC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi STA get MAC address command ID */
#define NCP_CMD_WLAN_STA_GET_MAC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-get-mac */
/** Wi-Fi STA get MAC address command response ID */
#define NCP_RSP_WLAN_STA_GET_MAC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi STA connect state command ID */
#define NCP_CMD_WLAN_STA_CONNECT_STAT  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-stat */
/** Wi-Fi STA connect state command response ID */
#define NCP_RSP_WLAN_STA_CONNECT_STAT  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi STA roaming command ID */
#define NCP_CMD_WLAN_STA_ROAMING       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-roaming */
/** Wi-Fi STA roaming command response ID */
#define NCP_RSP_WLAN_STA_ROAMING       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi STA set/get antenna command ID */
#define NCP_CMD_WLAN_STA_ANTENNA       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-set-antenna / wlan-get-antenna*/
/** Wi-Fi STA set/get antenna command response ID */
#define NCP_RSP_WLAN_STA_ANTENNA       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000009)
/** Wi-Fi STA signal command ID */
#define NCP_CMD_WLAN_STA_SIGNAL        (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000012) /* wlan-get-signal */
/** Wi-Fi STA signal command response ID */
#define NCP_RSP_WLAN_STA_SIGNAL        (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000012)
/** Wi-Fi STA csi command ID */
#define NCP_CMD_WLAN_STA_CSI           (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000020) /* wlan-csi */
/** Wi-Fi STA csi command response ID */
#define NCP_RSP_WLAN_STA_CSI           (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000020)
/** Wi-Fi STA enable 11K command ID */
#define NCP_CMD_WLAN_STA_11K_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000021) /* wlan-11k-enable */
/** Wi-Fi STA enable 11K command response ID */
#define NCP_RSP_WLAN_STA_11K_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000021)
/** Wi-Fi STA neigbor request command ID */
#define NCP_CMD_WLAN_STA_NEIGHBOR_REQ  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000022) /* wlan-11k-neighbor-req */
/** Wi-Fi STA neigbor request command response ID */
#define NCP_RSP_WLAN_STA_NEIGHBOR_REQ  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000022)
/** Wi-Fi STA MBO enable command ID */
#define NCP_CMD_WLAN_MBO_ENABLE        (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000023) /*wlan-mbo-enable*/
/** Wi-Fi STA MBO enable command response ID */
#define NCP_RSP_WLAN_MBO_ENABLE        (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000023)
/** Wi-Fi STA MBO nonprefer channel command ID */
#define NCP_CMD_WLAN_MBO_NONPREFER_CH  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000024) /*wlan-mbo-nonprefer-ch*/
/** Wi-Fi STA MBO nonprefer channel command response ID */
#define NCP_RSP_WLAN_MBO_NONPREFER_CH  (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000024)
/** Wi-Fi STA MBO set cell capacity command ID */
#define NCP_CMD_WLAN_MBO_SET_CELL_CAPA (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000025) /*wlan-mbo-set-cell-capa*/
/** Wi-Fi STA MBO set cell capacity command response ID */
#define NCP_RSP_WLAN_MBO_SET_CELL_CAPA (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000025)
/** Wi-Fi STA MBO set OCE command ID */
#define NCP_CMD_WLAN_MBO_SET_OCE       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000026) /*wlan-mbo-set-oce*/
/** Wi-Fi STA MBO set OCE command response ID */
#define NCP_RSP_WLAN_MBO_SET_OCE       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000026)
/** Wi-Fi STA start WPS PBC command ID */
#define NCP_CMD_WLAN_STA_WPS_PBC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000031) /* wlan-start-wps-pbc */
/** Wi-Fi STA start WPS PBC command response ID */
#define NCP_RSP_WLAN_STA_WPS_PBC       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000031)
/** Wi-Fi STA generate WPS pin code command ID */
#define NCP_CMD_WLAN_STA_GEN_WPS_PIN   (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD | 0x00000032) /* wlan-generate-wps-pin */
/** Wi-Fi STA generate WPS pin code command response ID */
#define NCP_RSP_WLAN_STA_GEN_WPS_PIN   (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000032)
/** Wi-Fi STA start WPS pin command ID */
#define NCP_CMD_WLAN_STA_WPS_PIN       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_CMD |0x00000033) /* wlan-start-wps-pin */
/** Wi-Fi STA start WPS pin command response ID */
#define NCP_RSP_WLAN_STA_WPS_PIN       (NCP_CMD_WLAN | NCP_CMD_WLAN_STA | NCP_MSG_TYPE_RESP | 0x00000033)

/** WLAN Basic command/response */
/** Wi-Fi reset command ID */
#define NCP_CMD_WLAN_BASIC_WLAN_RESET           (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-reset */
/** Wi-Fi reset command response ID */
#define NCP_RSP_WLAN_BASIC_WLAN_RESET           (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi start UAP provisioning command ID */
#define NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_START  (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-uap-prov-start */
/** Wi-Fi start UAP provisioning command response ID */
#define NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_START  (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi reset UAP provisioning command ID */
#define NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_RESET  (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-uap-prov-reset */
/** Wi-Fi reset UAP provisioning command response ID */
#define NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_RESET  (NCP_CMD_WLAN | NCP_CMD_WLAN_BASIC | NCP_MSG_TYPE_RESP | 0x00000003)

/** WLAN Http command and response */
/** Wi-Fi http connect command ID */
#define NCP_CMD_WLAN_HTTP_CON         (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-http-connect */
/** Wi-Fi http connect command response ID */
#define NCP_RSP_WLAN_HTTP_CON         (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi http disconnect command ID */
#define NCP_CMD_WLAN_HTTP_DISCON      (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-http-disconnect */
/** Wi-Fi http disconnect command response ID */
#define NCP_RSP_WLAN_HTTP_DISCON      (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi http request command ID */
#define NCP_CMD_WLAN_HTTP_REQ         (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-http-req */
/** Wi-Fi http request command response ID */
#define NCP_RSP_WLAN_HTTP_REQ         (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi http receive command ID */
#define NCP_CMD_WLAN_HTTP_RECV        (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-http-recv */
/** Wi-Fi http request command response ID */
#define NCP_RSP_WLAN_HTTP_RECV        (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi http seth command ID */
#define NCP_CMD_WLAN_HTTP_SETH        (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-http-seth */
/** Wi-Fi http seth command response ID */
#define NCP_RSP_WLAN_HTTP_SETH        (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi http unseth command ID */
#define NCP_CMD_WLAN_HTTP_UNSETH      (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-http-unseth */
/** Wi-Fi http unseth command response ID */
#define NCP_RSP_WLAN_HTTP_UNSETH      (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi web socket update command ID */
#define NCP_CMD_WLAN_WEBSOCKET_UPG    (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-websocket-upg */
/** Wi-Fi web socket update command response ID */
#define NCP_RSP_WLAN_WEBSOCKET_UPG    (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi web socket send command ID */
#define NCP_CMD_WLAN_WEBSOCKET_SEND   (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-websocket-send */
/** Wi-Fi web socket send command response ID */
#define NCP_RSP_WLAN_WEBSOCKET_SEND   (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi web socket receive command ID */
#define NCP_CMD_WLAN_WEBSOCKET_RECV   (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-websocket-recv */
/** Wi-Fi web socket receive command response ID */
#define NCP_RSP_WLAN_WEBSOCKET_RECV   (NCP_CMD_WLAN | NCP_CMD_WLAN_HTTP | NCP_MSG_TYPE_RESP | 0x00000009)

/** WLAN Socket command */
/** Wi-Fi socket open command ID */
#define NCP_CMD_WLAN_SOCKET_OPEN      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-socket-open */
/** Wi-Fi socket open command response ID */
#define NCP_RSP_WLAN_SOCKET_OPEN      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi socket connect command ID */
#define NCP_CMD_WLAN_SOCKET_CON       (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-socket-connect */
/** Wi-Fi socket connect command response ID */
#define NCP_RSP_WLAN_SOCKET_CON       (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi socket receive command ID */
#define NCP_CMD_WLAN_SOCKET_RECV      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-socket-receive */
/** Wi-Fi socket receive command response ID */
#define NCP_RSP_WLAN_SOCKET_RECV      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi socket send command ID */
#define NCP_CMD_WLAN_SOCKET_SEND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-socket-send */
/** Wi-Fi socket send command response ID */
#define NCP_RSP_WLAN_SOCKET_SEND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi socket sendto command ID */
#define NCP_CMD_WLAN_SOCKET_SENDTO    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-socket-sendto */
/** Wi-Fi socket sendto command response ID */
#define NCP_RSP_WLAN_SOCKET_SENDTO    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi socket bind command ID */
#define NCP_CMD_WLAN_SOCKET_BIND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-socket-bind */
/** Wi-Fi socket bind command response ID */
#define NCP_RSP_WLAN_SOCKET_BIND      (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi socket listen command ID */
#define NCP_CMD_WLAN_SOCKET_LISTEN    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-socket-listen */
/** Wi-Fi socket listen command response ID */
#define NCP_RSP_WLAN_SOCKET_LISTEN    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi socket accept command ID */
#define NCP_CMD_WLAN_SOCKET_ACCEPT    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-socket-accept */
/** Wi-Fi socket accept command response ID */
#define NCP_RSP_WLAN_SOCKET_ACCEPT    (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi socket close command ID */
#define NCP_CMD_WLAN_SOCKET_CLOSE     (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-socket-close */
/** Wi-Fi socket close command response ID */
#define NCP_RSP_WLAN_SOCKET_CLOSE     (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x00000009)
/** Wi-Fi socket receive from command ID */
#define NCP_CMD_WLAN_SOCKET_RECVFROM  (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_CMD | 0x0000000a) /* wlan-socket-recvfrom */
/** Wi-Fi socket receive from command response ID */
#define NCP_RSP_WLAN_SOCKET_RECVFROM  (NCP_CMD_WLAN | NCP_CMD_WLAN_SOCKET | NCP_MSG_TYPE_RESP | 0x0000000a)

/** Wi-Fi Debug command/response */
/** Wi-Fi register access command ID */
#define  NCP_CMD_WLAN_DEBUG_REGISTER_ACCESS  (NCP_CMD_WLAN | NCP_CMD_WLAN_DEBUG | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-reg-access */
/** Wi-Fi register access command response ID */
#define  NCP_RSP_WLAN_DEBUG_REGISTER_ACCESS  (NCP_CMD_WLAN | NCP_CMD_WLAN_DEBUG | NCP_MSG_TYPE_RESP | 0x00000001)

/** WLAN Memory command */
/** Wi-Fi memory state command ID */
#define NCP_CMD_WLAN_MEMORY_HEAP_SIZE        (NCP_CMD_WLAN | NCP_CMD_WLAN_MEMORY | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-mem-stat */
/** Wi-Fi memory state command response ID */
#define NCP_RSP_WLAN_MEMORY_HEAP_SIZE        (NCP_CMD_WLAN | NCP_CMD_WLAN_MEMORY | NCP_MSG_TYPE_RESP | 0x00000001)

/** WLAN Network command/response */
/** Wi-Fi network information command ID */ 
#define NCP_CMD_WLAN_NETWORK_INFO             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-info */
/** Wi-Fi network information command response ID  */
#define NCP_RSP_WLAN_NETWORK_INFO             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi network monitor command ID */
#define NCP_CMD_WLAN_NETWORK_MONITOR          (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-monitor */
/** Wi-Fi network monitor command response ID */
#define NCP_RSP_WLAN_NETWORK_MONITOR          (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi add network command ID */
#define NCP_CMD_WLAN_NETWORK_ADD              (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-add */
/** Wi-Fi add network command response ID */
#define NCP_RSP_WLAN_NETWORK_ADD              (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi start network command ID */
#define NCP_CMD_WLAN_NETWORK_START            (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-start-network */
/** Wi-Fi start network command response ID */
#define NCP_RSP_WLAN_NETWORK_START            (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi stop network command ID */
#define NCP_CMD_WLAN_NETWORK_STOP             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-stop-network */
/** Wi-Fi stop network command response ID */
#define NCP_RSP_WLAN_NETWORK_STOP             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi get number of STAs connected to UAP command ID */
#define NCP_CMD_WLAN_NETWORK_GET_UAP_STA_LIST (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-get-uap-sta-list */
/** Wi-Fi get number of STAs connected to UAP command response ID */
#define NCP_RSP_WLAN_NETWORK_GET_UAP_STA_LIST (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi MDNS query command ID */
#define NCP_CMD_WLAN_NETWORK_MDNS_QUERY       (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-mdns-query */
/** Wi-Fi MDNS query command response ID */
#define NCP_RSP_WLAN_NETWORK_MDNS_QUERY       (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi get added network list command ID */
#define NCP_CMD_WLAN_NETWORK_LIST             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-list */
/** Wi-Fi get added network list command response ID */
#define NCP_RSP_WLAN_NETWORK_LIST             (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi remove already added network command ID */
#define NCP_CMD_WLAN_NETWORK_REMOVE           (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-remove */
/** Wi-Fi remove already added network command response ID */
#define NCP_RSP_WLAN_NETWORK_REMOVE           (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x00000009)
/** Wi-Fi get STA/UAP network IP address command ID */
#define NCP_CMD_WLAN_NETWORK_ADDRESS          (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_CMD | 0x0000000A) /* wlan-address */
/** Wi-Fi get STA/UAP network IP address command response ID */
#define NCP_RSP_WLAN_NETWORK_ADDRESS          (NCP_CMD_WLAN | NCP_CMD_WLAN_NETWORK | NCP_MSG_TYPE_RESP | 0x0000000A)

/*WLAN Power mgmt command*/
/** Wi-Fi configure multiple MEF command ID */
#define NCP_CMD_WLAN_POWERMGMT_MEF           (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-multi-mef */
/** Wi-Fi configure multiple MEF command response ID */
#define NCP_RSP_WLAN_POWERMGMT_MEF           (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi enable/disable deep sleep power save mode command ID */
#define NCP_CMD_WLAN_POWERMGMT_DEEP_SLEEP_PS (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-deep-sleep-ps */
/** Wi-Fi enable/disable deep sleep power save mode command response ID */
#define NCP_RSP_WLAN_POWERMGMT_DEEP_SLEEP_PS (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi enable/disable ieee power save mode command ID */
#define NCP_CMD_WLAN_POWERMGMT_IEEE_PS       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-ieee-ps */
/** Wi-Fi enable/disable ieee power save mode command response ID */
#define NCP_RSP_WLAN_POWERMGMT_IEEE_PS       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi enable/disable WMM power save mode command ID */
#define NCP_CMD_WLAN_POWERMGMT_UAPSD         (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000004)  /* wlan-uapsd-enable */
/** Wi-Fi enable/disable WMM power save mode command response ID */
#define NCP_RSP_WLAN_POWERMGMT_UAPSD         (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi set/get WMM QOS information command ID */
#define NCP_CMD_WLAN_POWERMGMT_QOSINFO       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-uapsd-qosinfo */
/** Wi-Fi set/get WMM QOS information command response ID */
#define NCP_RSP_WLAN_POWERMGMT_QOSINFO       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi set/get sleep period command ID */
#define NCP_CMD_WLAN_POWERMGMT_SLEEP_PERIOD  (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-uapsd-sleep-period */
/** Wi-Fi set/get sleep period command response ID */
#define NCP_RSP_WLAN_POWERMGMT_SLEEP_PERIOD  (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi set wakeup conditions command ID */
#define NCP_CMD_WLAN_POWERMGMT_WOWLAN_CFG    (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-wowlan-cfg */
/** Wi-Fi set wakeup conditions command response ID */
#define NCP_RSP_WLAN_POWERMGMT_WOWLAN_CFG    (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi device enter power save mode command ID */
#define NCP_CMD_WLAN_POWERMGMT_SUSPEND       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_CMD | 0x0000000a) /* wlan-suspend */
/** Wi-Fi device enter power save mode command response ID */
#define NCP_RSP_WLAN_POWERMGMT_SUSPEND       (NCP_CMD_WLAN | NCP_CMD_WLAN_POWERMGMT | NCP_MSG_TYPE_RESP | 0x0000000a)

/** WLAN UAP command*/
/** Wi-Fi set maximum client count command ID */
#define NCP_CMD_WLAN_UAP_MAX_CLIENT_CNT   (NCP_CMD_WLAN | NCP_CMD_WLAN_UAP | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-set-max-clients-count */
/** Wi-Fi set maximum client count command response ID */
#define NCP_RSP_WLAN_UAP_MAX_CLIENT_CNT   (NCP_CMD_WLAN | NCP_CMD_WLAN_UAP | NCP_MSG_TYPE_RESP | 0x00000001)

/** WLAN Other command */
/** Wi-Fi configure 802.11ax features command ID */
#define NCP_CMD_11AX_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-set-11axcfg */
/** Wi-Fi configure 802.11ax features command response ID */
#define NCP_RSP_11AX_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000001)
/** Wi-Fi configure broadcast TWT command ID */
#define NCP_CMD_BTWT_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-set-11axcfg */
/** Wi-Fi configure broadcast TWT command response ID */
#define NCP_RSP_BTWT_CFG       (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi setup TWT command ID */
#define NCP_CMD_TWT_SETUP      (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-set-btwt-cfg */ 
/** Wi-Fi setup TWT command response ID */
#define NCP_RSP_TWT_SETUP      (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi disable TWT command ID */
#define NCP_CMD_TWT_TEARDOWN   (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-twt-teardown */
/** Wi-Fi disable TWT command response ID */
#define NCP_RSP_TWT_TEARDOWN   (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi get TWT information report command ID */
#define NCP_CMD_TWT_GET_REPORT (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-get-twt-report */
/** Wi-Fi get TWT information report command response ID */
#define NCP_RSP_TWT_GET_REPORT (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000005)
/** ID of Wi-Fi enable/disable STA/UAP 802.11d features command */
#define NCP_CMD_11D_ENABLE     (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-set-11d-enable */
/** Wi-Fi enable/disable STA/UAP 802.11d features command response ID */
#define NCP_RSP_11D_ENABLE     (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000006)
/** ID of Wi-Fi set/get region code command */
#define NCP_CMD_REGION_CODE    (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-region-code */
/** ID of Wi-Fi set/get region code command response */
#define NCP_RSP_REGION_CODE    (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi set/get time date command ID */
#define NCP_CMD_DATE_TIME      (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-set/get-time */
/** Wi-Fi set/get time date command response ID */
#define NCP_RSP_DATE_TIME      (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi get chip temperature command ID */
#define NCP_CMD_GET_TEMPERATUE (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-get-temp */
/** Wi-Fi get chip temperature command response ID */
#define NCP_RSP_GET_TEMPERATUE (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x00000009)
/** Wi-Fi invalid command ID */
#define NCP_CMD_INVALID_CMD    (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_CMD | 0x0000000a)
/** Wi-Fi invalid command response ID */
#define NCP_RSP_INVALID_CMD    (NCP_CMD_WLAN | NCP_CMD_WLAN_OTHER | NCP_MSG_TYPE_RESP | 0x0000000a)
      
/*WLAN Regulatory command*/
/** Wi-Fi enable/disable EU adaptivity command ID */
#define NCP_CMD_WLAN_REGULATORY_ED_MAC_MODE    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000001) /* wlan-set-ed-mac-mode */
/** Wi-Fi enable/disable EU adaptivity command response ID */
#define NCP_RSP_WLAN_REGULATORY_ED_MAC_MODE    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000001)

#if CONFIG_NCP_RF_TEST_MODE
/** Wi-Fi set RF test mode command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_TEST_MODE      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000002) /* wlan-set-rf-test-mode */
/** Wi-Fi set RF test mode command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_TEST_MODE      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000002)
/** Wi-Fi set RF transmit antenna command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_TX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000003) /* wlan-set-rf-tx-antenna */
/** Wi-Fi set RF transmit antenna command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_TX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000003)
/** Wi-Fi get RF transmit antenna command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_TX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000004) /* wlan-get-rf-tx-antenna */
/** Wi-Fi get RF transmit antenna command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_TX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000004)
/** Wi-Fi set RF receive antenna command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_RX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000005) /* wlan-set-rf-rx-antenna */
/** Wi-Fi set RF receive antenna command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_RX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000005)
/** Wi-Fi get RF receive antenna command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_RX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000006) /* wlan-get-rf-rx-antenna */
/** Wi-Fi get RF receive antenna command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_RX_ANTENNA     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000006)
/** Wi-Fi set RF band command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_BAND           (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000007) /* wlan-set-rf-band */
/** Wi-Fi set RF band command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_BAND           (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000007)
/** Wi-Fi get RF band command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_BAND           (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000008) /* wlan-get-rf-band */
/** Wi-Fi get RF band command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_BAND           (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000008)
/** Wi-Fi set RF bandwidth command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_BANDWIDTH      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000009) /* wlan-set-rf-bandwidth */
/** Wi-Fi set RF bandwidth command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_BANDWIDTH      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000009)
/** Wi-Fi get RF bandwidth command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_BANDWIDTH      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000a) /* wlan-get-rf-bandwidth */
/** Wi-Fi get RF bandwidth command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_BANDWIDTH      (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000a)
/** Wi-Fi set RF channel number command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_CHANNEL        (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000b) /* wlan-set-rf-channel */
/** Wi-Fi set RF channel number command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_CHANNEL        (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000b)
/** Wi-Fi get RF channel number command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_CHANNEL        (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000c) /* wlan-get-rf-channel */
/** Wi-Fi get RF channel number command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_CHANNEL        (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000c)
/** Wi-Fi set RF radio mode command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_RADIO_MODE     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000d) /* wlan-set-rf-radio-mode */
/** Wi-Fi set RF radio mode command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_RADIO_MODE     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000d)
/** Wi-Fi get RF radio mode command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_RF_RADIO_MODE     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000e) /* wlan-get-rf-radio-mode */
/** Wi-Fi get RF radio mode command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_RF_RADIO_MODE     (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000e)
/** Wi-Fi set RF transmit power command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_TX_POWER       (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x0000000f) /* wlan-set-rf-tx-power */
/** Wi-Fi set RF transmit power command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_TX_POWER       (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x0000000f)
/** Wi-Fi set RF transmit continuous configuration command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_TX_CONT_MODE   (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000010) /* wlan-set-rf-tx-cont-mode */
/** Wi-Fi set RF transmit continuous configuration command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_TX_CONT_MODE   (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000010)
/** Wi-Fi set RF transmit frame command ID */
#define NCP_CMD_WLAN_REGULATORY_SET_RF_TX_FRAME       (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000011) /* wlan-set-rf-tx-frame */
/** Wi-Fi set RF transmit frame command response ID */
#define NCP_RSP_WLAN_REGULATORY_SET_RF_TX_FRAME       (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000011)
/** Wi-Fi get and reset RF per command ID */
#define NCP_CMD_WLAN_REGULATORY_GET_AND_RESET_RF_PER  (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000012) /* wlan-get-and-reset-rf-per */
/** Wi-Fi get and reset RF per command response ID */
#define NCP_RSP_WLAN_REGULATORY_GET_AND_RESET_RF_PER  (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000012)
#endif
/** Wi-Fi verify Algorithm AES-CCMP-128 encryption decryption command ID */
#define NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_CCMP_128    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000013) /* wlan-eu-crypto-ccmp-128 */
/** Wi-Fi verify Algorithm AES-CCMP-128 encryption decryption command response ID */
#define NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_CCMP_128    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000013)
/** Wi-Fi verify Algorithm AES-GCMP-128 encryption decryption command ID */
#define NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_GCMP_128    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_CMD | 0x00000014) /* wlan-eu-crypto-gcmp-128 */
/** Wi-Fi verify Algorithm AES-GCMP-128 encryption decryption command response ID */
#define NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_GCMP_128    (NCP_CMD_WLAN | NCP_CMD_WLAN_REGULATORY | NCP_MSG_TYPE_RESP | 0x00000014)

/** WLAN events */
/** Wi-Fi MDNS query result event ID */
#define NCP_EVENT_MDNS_QUERY_RESULT   (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000003) /* mdns-query-result */
/** Wi-Fi MDNS resolve domain event ID */
#define NCP_EVENT_MDNS_RESOLVE_DOMAIN (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000004) /* mdns-resolve-domain-name */
/** Wi-Fi MDNS csi data event ID */
#define NCP_EVENT_CSI_DATA            (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000005) /* csi data */
/** Wi-Fi station connect AP event ID */
#define NCP_EVENT_WLAN_STA_CONNECT    (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000006) /* wlan sta connect */
/** Wi-Fi station disconnect AP event ID */
#define NCP_EVENT_WLAN_STA_DISCONNECT (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000007) /* wlan sta disconnect */
/** Wi-Fi UAP stop network event ID */
#define NCP_EVENT_WLAN_STOP_NETWORK   (NCP_CMD_WLAN | NCP_CMD_WLAN_ASYNC_EVENT | NCP_MSG_TYPE_EVENT | 0x00000008) /* wlan stop network */

/** NCP WLAN TLV */
/** NCP network ssid tlv type */
#define NCP_CMD_NETWORK_SSID_TLV         0x0001
/** NCP network bssid tlv type */
#define NCP_CMD_NETWORK_BSSID_TLV        0x0002
/** NCP network channel tlv type */
#define NCP_CMD_NETWORK_CHANNEL_TLV      0x0003
/** NCP network IP tlv type */
#define NCP_CMD_NETWORK_IP_TLV           0x0004
/** NCP network security tlv type */
#define NCP_CMD_NETWORK_SECURITY_TLV     0x0005
/** NCP network role tlv type */
#define NCP_CMD_NETWORK_ROLE_TLV         0x0006
/** NCP network DTIM tlv type */
#define NCP_CMD_NETWORK_DTIM_TLV         0x0007
/** NCP network capacity tlv type */
#define NCP_CMD_NETWORK_CAPA_TLV         0x0008
/** NCP auto change band tlv type */
#define NCP_CMD_NETWORK_ACSBAND_TLV      0x0009
/** NCP management frame protection tlv type */
#define NCP_CMD_NETWORK_PMF_TLV          0x000A
/** NCP network PWE (password element) tlv type */
#define NCP_CMD_NETWORK_PWE_TLV          0x000B
/** NCP transition_Disable tlv type */
#define NCP_CMD_NETWORK_TR_TLV           0x000C
/** NCP network EAP tlv type */
#define NCP_CMD_NETWORK_EAP_TLV          0x000D
/** NCP network high efficiency tlv type */
#define NCP_CMD_WLAN_HE_CAP_TLV 0x00FF

/** NCP MDNS result TLV */
/** NCP PTR (pointer records) resource record tlv type */
#define NCP_CMD_NETWORK_MDNS_RESULT_PTR 0x0011
/** NCP SRV (service records) resource record tlv type */
#define NCP_CMD_NETWORK_MDNS_RESULT_SRV 0x0012
/** NCP TXT (text) resource record tlv type */
#define NCP_CMD_NETWORK_MDNS_RESULT_TXT 0x0013
/** NCP IPv4 and IPv6 resource record tlv type */
#define NCP_CMD_NETWORK_MDNS_RESULT_IP_ADDR 0x0014

/** Set UAP max client count status */
#define WLAN_SET_MAX_CLIENT_CNT_SUCCESS          0
#define WLAN_SET_MAX_CLIENT_CNT_FAIL             1
#define WLAN_SET_MAX_CLIENT_CNT_START            2
#define WLAN_SET_MAX_CLIENT_CNT_EXCEED           3

#define MDNS_PROTO_UDP "_udp"
#define MDNS_PROTO_TCP "_tcp"

#define MDNS_RRTYPE_A   "A"
#define MDNS_RRTYPE_PTR "PTR"

/** This structure is used to set the configuration of the scanned AP. */
typedef NCP_TLV_PACK_START struct _ncp_wlan_scan_result
{
    /** Network SSID, represented as a NULL-terminated C string of 0 to 32
     *  characters. If the network has a hidden SSID, this will be the empty
     *  string.
     */
    char ssid[33];
    /** SSID length. */
    unsigned int ssid_len;
    /** Network BSSID, represented as a 6-byte array. */
    char bssid[6];
    /** Network channel. */
    unsigned int channel;

    /* network features */
    /** Network supports 802.11n.  This is set to 0 if the network does not
     *  support 802.11N or if the system does not have 802.11N support enabled. */
    unsigned dot11n : 1;
#if CONFIG_NCP_11AC
    /** Network supports 802.11ac.  This is set to 0 if the network does not
     *  support 802.11ac or if the system does not have 802.11ac support enabled. */
    unsigned dot11ac : 1;
#endif
#if CONFIG_NCP_11AX
    /** Network supports 802.11ax.  This is set to 0 if the network does not
     *  support 802.11ax or if the system does not have 802.11ax support enabled. */
    unsigned dot11ax : 1;
#endif
    unsigned wmm : 1;
#if CONFIG_NCP_SUPP_WPS
    /** Network supports WPS.  This is set to 0 if the network does not
     *  support WPS or if the system does not have WPS support enabled. */
    unsigned wps : 1;
    /** WPS Type PBC/PIN */
    unsigned int wps_session;
#endif
    /** WPA2 enterprise security */
    unsigned wpa2_entp : 1;
    /** Network uses WEP security. */
    unsigned wep : 1;
    /** Network uses WPA security. */
    unsigned wpa : 1;
    /** Network uses WPA2 security. */
    unsigned wpa2 : 1;
    /** Network uses WPA3 SAE security. */
    unsigned wpa3_sae : 1;

    /** Signal strength of the beacon. */
    unsigned char rssi;
    /** Network SSID, represented as a NULL-terminated C string of 0 to 32
     *  characters.  If the network has a hidden SSID, this will be the empty
     *  string.
     */
    char trans_ssid[33];
    /** SSID length */
    unsigned int trans_ssid_len;
    /** Network BSSID, represented as a 6-byte array. */
    char trans_bssid[6];

    /** Beacon period. */
    uint16_t beacon_period;

    /** DTIM period. */
    uint8_t dtim_period;
} NCP_TLV_PACK_END NCP_WLAN_SCAN_RESULT;

/** This structure is used to configure the information of all the scanned APs. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SCAN_NETWORK_INFO
{
    /** Number of scanned APs. */
    uint32_t res_cnt;
    /** Configuration information of each scanned AP. */
    NCP_WLAN_SCAN_RESULT res[CONFIG_MAX_AP_ENTRIES];
} NCP_TLV_PACK_END NCP_CMD_SCAN_NETWORK_INFO;

/** This structure is used to configure the Wi-Fi driver and the firmware version information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_FW_VERSION
{
    /** Driver version string. */
    char driver_ver_str[16];
    /** Firmware version string. */
    char fw_ver_str[MLAN_MAX_VER_STR_LEN];
} NCP_TLV_PACK_END NCP_CMD_FW_VERSION;

/** This structure is used to store the MAC address of the STA and UAP. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_GET_MAC_ADDRESS
{
    /** MAC address of UAP. */
    uint8_t uap_mac[MLAN_MAC_ADDR_LENGTH];
    /** MAC address of STA. */
    uint8_t sta_mac[MLAN_MAC_ADDR_LENGTH];
} NCP_TLV_PACK_END NCP_CMD_GET_MAC_ADDRESS;

/** This structure is used to set the Wi-Fi connection status. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CONNECT_STAT
{ 
    /** Power save mode.
     * 0: active mode,
     * 1: IEEE power save mode,
     * 2: deep sleep power save mode,
     * 3: IEEE power save and deep sleep mode,
     * 4: WNM power save mode,
     * 5: WNM power save and deep sleep mode.
     */
    uint8_t ps_mode;
    /** UAP connect status. */
    uint8_t uap_conn_stat;
    /** STA connect status. */
    uint8_t sta_conn_stat;
} NCP_TLV_PACK_END NCP_CMD_CONNECT_STAT;

#if CONFIG_NCP_IPV6
/** This structure is used to set the IPv6 configuration */
typedef NCP_TLV_PACK_START struct _ncp_wlan_ipv6_config
{
    /** System's IPv6 address in network order. */
    unsigned address[4];
    /** Address type string: linklocal, site-local or global. */
    unsigned char addr_type_str[16];
    /** State string of IPv6 address (Tentative, Preferred, etc). */
    unsigned char addr_state_str[32];
} NCP_TLV_PACK_END NCP_WLAN_IPV6_CONFIG;
#endif

/** This structure is used to store the IPv4 configuration. */
typedef NCP_TLV_PACK_START struct _ncp_wlan_ipv4_config
{
    /** Set to \ref ADDR_TYPE_DHCP to use DHCP to obtain the IP address or
     *  \ref ADDR_TYPE_STATIC to use a static IP. In case of static IP
     *  address ip, gateway, netmask and dns members must be specified.  When
     *  using DHCP, the IP, gateway, netmask and DNS are overwritten by the
     *  values obtained from the DHCP server. They should be zeroed out if
     *  not used. */
    uint32_t addr_type : 2;
    /** IP address in network order of the system. */
    uint32_t address;
    /** Default gateway in network order of the system. */
    uint32_t gw;
    /** Subnet mask in network order of the system. */
    uint32_t netmask;
    /** Primary dns server in network order of the system. */
    uint32_t dns1;
    /** Secondary DNS server in network order of the system. */
    uint32_t dns2;
} NCP_TLV_PACK_END NCP_WLAN_IPV4_CONFIG;

/** This structure represents a Wi-Fi network profile. It consists of an
 *  arbitrary name, Wi-Fi configuration, and IP address configuration.
 */
typedef NCP_TLV_PACK_START struct _ncp_wlan_network
{
    /** Name of this network profile. */
    char name[WLAN_NETWORK_NAME_MAX_LENGTH + 1];
    /** Network SSID, represented as a C string of up to 32 characters
     *  in length.
     *  If this profile is used in the UAP mode, this field is
     *  used as the SSID of the network.
     *  If this profile is used in the station mode, this field is
     *  used to identify the network. Set the first byte of the SSID to NULL
     *  (a 0-length string) to use only the BSSID to find the network.
     */
    char ssid[IEEEtypes_SSID_SIZE + 1];
    /** Network BSSID, represented as a 6-byte array.
     *  If this profile is used in the UAP mode, this field is
     *  ignored.
     *  If this profile is used in the station mode, this field is
     *  used to identify the network. Set all 6 bytes to 0 to use any BSSID,
     *  in which case only the SSID will be used to find the network.
     */
    char bssid[IEEEtypes_ADDRESS_SIZE];
    /** Channel for this network.
     *  If this profile is used in UAP mode, this field
     *  specifies the channel to start the UAP interface on. Set this
     *  to 0 for auto channel selection.
     *  If this profile is used in the station mode, this constrains the
     *  channel on which the network to connect should be present. Set this
     *  to 0 to allow the network to be found on any channel. */
    unsigned int channel;
    /** If set channel to 0, enable auto channel select. */
    uint16_t acs_band;
    /** Rssi threshold is used to scan with RSSI filter. */
    short rssi_threshold;
    /** BSS type. */
    uint8_t type;
    /** Network role type, set this to specify what type of Wi-Fi network mode to use.
     *  This can either be \ref WLAN_BSS_ROLE_STA for use in
     *  the station mode, or it can be \ref WLAN_BSS_ROLE_UAP
     *  for use in the UAP mode.
     */
    uint8_t role;

    /** Network's security type. Use specified by enum wlan_security_type. */
    uint8_t security_type;

    /** Enable 802.11ax flage, 1: enable; 0: disable. */
    uint8_t enable_11ax : 1;
    /** Enable 802.11ac flag, 1: enable; 0: disable. */
    uint8_t enable_11ac : 1;
    /** Enable 802.11n flag, 1: enable; 0: disable */
    uint8_t enable_11n : 1;

    /** Configuration of network is IPv6 address. */
    NCP_WLAN_IPV6_CONFIG ipv6[CONFIG_MAX_IPV6_ADDRESSES];
    /** Configuration of network is IPv4 address. */
    NCP_WLAN_IPV4_CONFIG ipv4;
    /** STA connected AP with IPV4 flag, TRUE means STA is connected to AP,
    FALSE means STA is disconnected to AP. */
    uint8_t is_sta_ipv4_connected;
    /** Identity string. */
    char identity[IDENTITY_MAX_LENGTH];
    /* Private Fields */
    /** If set to 1, the ssid field contains the specific SSID for this
     * network.*/
    unsigned ssid_specific : 1;
    /** If set to 1, the BSSID field contains the specific BSSID for this
     *  network. */
    unsigned bssid_specific : 1;
    /** If set to 1, the channel field contains the specific channel for this
     * network. */
    unsigned channel_specific : 1;
    /** Set to 0, any security that matches is used. */
    unsigned security_specific : 1;
    /** This indicates this network is used as an internal network for
     * WPS */
    unsigned wps_specific : 1;
    /** Beacon period of associated BSS. */
    uint16_t beacon_period;
    /** DTIM period of associated BSS. */
    uint8_t dtim_period;
    /** Wi-Fi network capability */
    uint8_t wlan_capa;
} NCP_TLV_PACK_END NCP_WLAN_NETWORK;

/** This structure is used to configure the network information of the STA and UAP. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_INFO
{
    /** UAP connection status */
    uint8_t uap_conn_stat;
    /** Station connection status */
    uint8_t sta_conn_stat;
    /** Network information for the UAP */
    NCP_WLAN_NETWORK uap_network;
    /** Network information for the station */
    NCP_WLAN_NETWORK sta_network;
} NCP_TLV_PACK_END NCP_CMD_NETWORK_INFO;
    
/** This structure is used to configure the network information of the STA. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_ADDRESS
{
    /** Station connection status */
    uint8_t sta_conn_stat;
    /** Network information for the station */
    NCP_WLAN_NETWORK sta_network;
} NCP_TLV_PACK_END NCP_CMD_NETWORK_ADDRESS;

/** This structure is used to configure the network list.
*/
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_LIST
{   
    /** Network number */
    uint8_t count;
    /** Network infomation list, the max size of the list is 5. */
    NCP_WLAN_NETWORK net_list[NCP_WLAN_KNOWN_NETWORKS];
} NCP_TLV_PACK_END NCP_CMD_NETWORK_LIST;

/** This structure is used to configure the network to be removed. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_REMOVE
{   
    /** Removed network name. */
    char name[WLAN_NETWORK_NAME_MAX_LENGTH + 1];
    /** Remove status, WM_SUCCESS means removed successfully, otherwise removed failed */
    int8_t remove_state;
} NCP_TLV_PACK_END NCP_CMD_NETWORK_REMOVE;

/** This structure is used to configure the MAC address. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_MAC_ADDRESS
{
    /** MAC address. */
    uint8_t mac_addr[MLAN_MAC_ADDR_LENGTH];
} NCP_TLV_PACK_END NCP_CMD_MAC_ADDRESS;

/** This structure is used to configure the SSID. */
typedef NCP_TLV_PACK_START struct _SSID_ParamSet_t
{   
    /** Header, include type and size. */
    TypeHeader_t header;
    /** SSID (service set identifier) */
    char ssid[IEEEtypes_SSID_SIZE+1];
} NCP_TLV_PACK_END SSID_ParamSet_t;

/** This structure is used to configure the BSSID. */
typedef NCP_TLV_PACK_START struct _BSSID_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** BSSId (basic service set identifier). */
    char bssid[IEEEtypes_ADDRESS_SIZE];
} NCP_TLV_PACK_END BSSID_ParamSet_t;

/** This structure is used to configure the BSS role. */
typedef NCP_TLV_PACK_START struct _BSSRole_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** network BSS role, 0: STA, 1: UAP */
    uint8_t role;
} NCP_TLV_PACK_END BSSRole_ParamSet_t;

/** This structure is used to configure the channel infomation. */
typedef NCP_TLV_PACK_START struct _Channel_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** network channel */
    uint8_t channel;
} NCP_TLV_PACK_END Channel_ParamSet_t;

/** This structure is used to configure the PWE (password element) derivation infomation. */
typedef NCP_TLV_PACK_START struct _Pwe_Derivation_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /**
     * sae_pwe - SAE mechanism for PWE derivation
     * 0 = hunting-and-pecking loop only
     * 1 = hash-to-element only
     * 2 = both hunting-and-pecking loop and hash-to-element enabled
     */
    uint8_t pwe_derivation;
} NCP_TLV_PACK_END Pwe_Derivation_ParamSet_t;

/** This structure is used to configure the transition disable flag. */
typedef NCP_TLV_PACK_START struct _Transition_Disable_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** transition disable flag. */
    uint8_t transition_disable;
} NCP_TLV_PACK_END Tr_Disable_ParamSet_t;

/** This structure is to configure the ACS (auto channel select) band. */
typedef NCP_TLV_PACK_START struct _ACSBand_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Auto select band, only if set channel to 0. */
    uint16_t acs_band;
} NCP_TLV_PACK_END ACSBand_ParamSet_t;

/** This structure is used to store the network IP configuration information. */
typedef NCP_TLV_PACK_START struct _IP_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Auto ip flag, 0: no auto IP, 1: auto IP. */
    uint8_t is_autoip;
    /** Network IP address. */
    uint32_t address;
    /** Gateway. */
    uint32_t gateway;
    /** Netmask. */
    uint32_t netmask;
    /** dns1. */
    uint32_t dns1;
    /** dns2. */
    uint32_t dns2;
} NCP_TLV_PACK_END IP_ParamSet_t;

/** This structure is used to configure the security passsword information. */
typedef NCP_TLV_PACK_START struct _Security_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Wi-Fi security type. */
    uint8_t type;
    /** Wi-Fi security password length. */
    uint8_t password_len;
    /** Wi-Fi security password string. */
    char password[1];
} NCP_TLV_PACK_END Security_ParamSet_t;

/** This structure is used to configure the MFPC (management frame protection capable) 
and MFPR (management frame protection required) flag.
*/
typedef NCP_TLV_PACK_START struct _PMF_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** MFPC (management frame protection capable) flag, 1: enable, 0: disable. */
    uint8_t mfpc;
    /** MFPR (management frame protection required) flag, 1: enable, 0: disable. */
    uint8_t mfpr;
} NCP_TLV_PACK_END PMF_ParamSet_t;

/** This structure is used to store the EAP (extensible authentication protocol) information. */
typedef NCP_TLV_PACK_START struct _EAP_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** anonymous identity string for EAP */
    char anonymous_identity[IDENTITY_MAX_LENGTH];
    /** client key password string */
    char client_key_passwd[PASSWORD_MAX_LENGTH];
} NCP_TLV_PACK_END EAP_ParamSet_t;

#if CONFIG_NCP_WIFI_DTIM_PERIOD
/** This structure is used to configure the DTIM (delivery traffic indication message) period. */
typedef NCP_TLV_PACK_START struct _DTIM_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** DTIM (delivery traffic indication message) period */
    uint8_t dtim_period;
} NCP_TLV_PACK_END DTIM_ParamSet_t;
#endif

#if CONFIG_NCP_WIFI_CAPA
/** This structure is used to set the Wi-Fi capabilities. */
typedef NCP_TLV_PACK_START struct _CAPA_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Wi-Fi capabilities bitmap, such as 802.11n, 802.11ac, 802.11ax and legacy. */
    uint8_t capa;
} NCP_TLV_PACK_END CAPA_ParamSet_t;
#endif

/** This structure is used to store the "wlan-add" command configuration information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_ADD
{
    /** Network name string */
    char name[WLAN_NETWORK_NAME_MAX_LENGTH];
    /** Length of TLVs sent in command starting at tlvBuffer */
    uint32_t tlv_buf_len;
    /** Payload of "wlan-add" command, includes:
     * SSID TLV, SSID_ParamSet_t
     * BSSID TLV, BSSID_ParamSet_t
     * BSS role TLV, BSSRole_ParamSet_t
     * Channel TLV, Channel_ParamSet_t
     * ACS band TLV, ACSBand_ParamSet_t
     * IP address TLV, IP_ParamSet_t
     * Security TLV, Security_ParamSet_t
     * DTIM period TLV, DTIM_ParamSet_t
     * CAPA TLV, CAPA_ParamSet_t
     * PMF TLV, PMF_ParamSet_t
     */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_CMD_NETWORK_ADD;

/** This structure is used to store the network name and SSID of the UAP to be started. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_START
{ 
    /** Network name string. */
    char name[32];
    /** SSID (service set identifier). */
    char ssid[32 + 1];
} NCP_TLV_PACK_END NCP_CMD_NETWORK_START;

/** This structure is used to store a collection of STAs information, those are connected to the UAP. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NETWORK_UAP_STA_LIST
{
    /** Number of STAs connected to the UAP. */
    uint16_t sta_count;
    /** Infomation on the STAs connected to the UAP. */
    wifi_sta_info_t info[MAX_NUM_CLIENTS];
} NCP_TLV_PACK_END NCP_CMD_NETWORK_UAP_STA_LIST;

/** This structure is used to store the roaming configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ROAMING
{
    /** STA roaming enable flag, 1: enable roaming, 0: disable roaming. */
    uint32_t enable;
    /** When the RSSI of the AP currently connected to the STA is lower than this threshold,
     * the STA begins to scan the environment to find APs with better signals for roaming.
     */
    uint8_t rssi_threshold;
} NCP_TLV_PACK_END NCP_CMD_ROAMING;

/** This structure is used to configure the Wi-Fi reset option. */
typedef NCP_TLV_PACK_START struct WLAN_RESET_ParaSet
{   
    /** Wi-Fi reset option, there are 3 modes,
     *  0: disable Wi-Fi;
     *  1: enable Wi-Fi;
     *  2: reset Wi-Fi.
     */
    uint8_t option;
} NCP_TLV_PACK_END WLAN_RESET_data;

/*NCP WLAN SOCKET*/
/*NCP Wlan Socket Open*/
#define HTTP_PARA_LEN 16
/** This structure is used to store the socket open configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_OPEN_CFG
{
    /** Socket type: raw/UDP/TCP */
    char socket_type[HTTP_PARA_LEN];
    /** Domain type */
    char domain_type[HTTP_PARA_LEN];
    /** Protocol. */
    char protocol[HTTP_PARA_LEN];
    /** Handle to open. */
    uint32_t  opened_handle;
} NCP_TLV_PACK_END NCP_CMD_SOCKET_OPEN_CFG;

/** NCP Wlan socket connect */
#define IP_ADDR_LEN 16

/** This structure is used to store the socket connect configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_CON_CFGN
{
    /** Socket handle index number */
    uint32_t handle;
    /** Port number */
    uint32_t port;
    /** IP address */
    char ip_addr[IP_ADDR_LEN];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_CON_CFG;

/** This structure is used to configure the socket bind configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_BIND_CFG
{
    /** Socket handle index number */
    uint32_t handle;
    /** socket port number to be bound. */
    uint32_t port;
    /** IP address to be bound. */
    char ip_addr[IP_ADDR_LEN];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_BIND_CFG;

/** This structure is used to configure the close socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_CLOSE_CFG
{
    /** Socket handle index number */
    uint32_t handle;
} NCP_TLV_PACK_END NCP_CMD_SOCKET_CLOSE_CFG;

/** This structure is used to set the listening socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_LISTEN_CFG
{
    /** Socket handle index number */
    uint32_t handle;
    /** Maximum number of sockets to listen to */
    uint32_t number;
} NCP_TLV_PACK_END NCP_CMD_SOCKET_LISTEN_CFG;

/** This structure is used to store the socket accept configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_ACCEPT_CFG
{
    /** Socket handle index number. */
    uint32_t handle;
    /** Accept socket handle */
    int  accepted_handle;
} NCP_TLV_PACK_END NCP_CMD_SOCKET_ACCEPT_CFG;

/** This structure is used to set the send socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_SEND_CFG
{
    /** Socket handle index number. */
    uint32_t handle;
    /** Size of send data */
    uint32_t size;
    /** Data buffer to send. */
    char send_data[1];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_SEND_CFG;

/** This structure is used to set the sendto socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_SENDTO_CFG
{
    /** Socket handle index number */
    uint32_t handle;
    /** Send data size. */
    uint32_t size;
    /** Peer device IP address, the max length is 16. */
    char ip_addr[IP_ADDR_LEN];
    /** Peer port number */
    uint32_t port;
    /** Data buffer to send. */
    char send_data[1];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_SENDTO_CFG;

/** This structure is used to set the receive socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_RECEIVE_CFG
{
    /** Peer socket handle */
    uint32_t handle;
    /** Receive buffer size, the buffer max length is 4072 */
    uint32_t recv_size;
    /** Wait time */
    uint32_t timeout;
    /** Reveived data */
    char recv_data[1];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_RECEIVE_CFG;

/** This structure is used to set the receive from socket configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_SOCKET_RECVFROM_CFG
{   
    /** Peer socket handle. */
    uint32_t handle;
    /** Size of received buffer, the buffer maximum length is 4072. */
    uint32_t recv_size;
    /** Wait time. */
    uint32_t timeout;
    /** Peer IP address. */
    char peer_ip[IP_ADDR_LEN];
    /** Peer port number. */
    uint32_t peer_port;
    /** Received data. */
    char recv_data[1];
} NCP_TLV_PACK_END NCP_CMD_SOCKET_RECVFROM_CFG;

/** This structure is used to configure the HTTP connect information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_CONNECT_CFG
{   
    /** Created HTTP handle. */
    int opened_handle;
    /** HTTP host, the maximum string length is 512. */
    char host[1];
} NCP_TLV_PACK_END  NCP_CMD_HTTP_CON_CFG;

/** This structure is used to configure the HTTP disconnect information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_DISCONNECT_CFG
{
    /** Socket handle index number. */
    uint32_t handle;
} NCP_TLV_PACK_END  NCP_CMD_HTTP_DISCON_CFG;

/** NCP Wi-Fi Http Seth */
#define SETH_NAME_LENGTH  64
#define SETH_VALUE_LENGTH 128
/** This structure is used to configure HTTP header. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_SETH_CFG
{
    /** HTTP header segment name. */
    char name[SETH_NAME_LENGTH];
    /** HTTP header segment value */
    char value[SETH_VALUE_LENGTH];
} NCP_TLV_PACK_END NCP_CMD_HTTP_SETH_CFG;

/** This structure is used to configure the unset HTTP header segment. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_UNSETH_CFG
{
    /** HTTP header segment name, the max length is 64. */
    char name[SETH_NAME_LENGTH];
} NCP_TLV_PACK_END NCP_CMD_HTTP_UNSETH_CFG;

/** NCP Wlan Http Request */
#define HTTP_URI_LEN 512
/** This structure is used to configure the HTTP request information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_REQ_CFG
{
    /** socket handle index number */
    uint32_t handle;
    /** Support method is get/delete/put/options. The max length is 16. */
    char method[HTTP_PARA_LEN];
    /** HTTP URL, the string max length is 512 */
    char uri[HTTP_URI_LEN];
    /** Size of the request data */
    uint32_t req_size;
    /** Request data. */
    char req_data[1];
} NCP_TLV_PACK_END NCP_CMD_HTTP_REQ_CFG;

/** This structure is used to configure the HTTP request response. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_REQ_RESP_CFG
{
    /** Received header size */
    uint32_t header_size;
    /** Received header */
    char recv_header[1];
} NCP_TLV_PACK_END NCP_CMD_HTTP_REQ_RESP_CFG;

/** This structure is used to set the HTTP receive data configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_RECV_CFG
{
    /** Socket handle index number. */
    uint32_t handle;
    /** Size of HTTP received data */
    uint32_t recv_size;
    /** HTTP receive data wait time */
    uint32_t timeout;
    /** Received data buffer. */
    char recv_data[1];
} NCP_TLV_PACK_END NCP_CMD_HTTP_RECV_CFG;

/** This structure is used to set the HTTP upgrade configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_HTTP_UPG_CFG
{
    /** Socket handle index number */
    uint32_t handle;
    /** Web socket URI */
    char uri[HTTP_URI_LEN];
    /** Protocol */
    char protocol[HTTP_PARA_LEN];
} NCP_TLV_PACK_END NCP_CMD_HTTP_UPG_CFG;

/** This structure is used to set the web socket send configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_WEBSOCKET_SEND_CFG
{
    /** Socket handle index number */
    uint32_t handle;
    /** Data type. */
    char     type[HTTP_PARA_LEN];
    /** Size of send data. */
    uint32_t size;
    /** Data to be send. */
    char send_data[1];
} NCP_TLV_PACK_END NCP_CMD_WEBSOCKET_SEND_CFG;

/** This structure is used to set the web socket receive information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_WEBSOCKET_RECV_CFG
{
    /** Web socket handle. */
    uint32_t handle;
    /** Size of received data, the maximum length is 4068. */
    uint32_t recv_size;
    /** Wait time, timeout if no data is received from the peer device within a period of time. */
    uint32_t timeout;
    /** FIN flag is used to terminate the TCP connection, If the value is 1, the TCP connection is disconnected. */
    uint32_t fin;
    /** Data buffer used to store the packets received from the web socket. */
    char recv_data[1];
} NCP_TLV_PACK_END NCP_CMD_WEBSOCKET_RECV_CFG;

/** This structure is used to store the generated 8 pin code. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_WPS_GEN_PIN
{   
    /** WPS (Wi-Fi protected setup) pin code. */
    uint32_t pin;
} NCP_TLV_PACK_END NCP_CMD_WPS_GEN_PIN;

/** This structure is used to configure the pin code. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_WPS_PIN
{  
    /** Pin code of WPS (Wi-Fi protected setup). */
    uint32_t pin;
} NCP_TLV_PACK_END NCP_CMD_WPS_PIN;

/** This structure is used to configure the network monitor information. */
typedef NCP_TLV_PACK_START struct
{
    /** Operations, 
    Add: Add mac addr as filter element
    Delete: Delete mac addr
    Clear: Clear mac addr
    Dump: Dump monitor parameter.
    */
    uint16_t action;
    /** Monitor activity. */
    uint16_t monitor_activity;
    /** Filter flags. */
    uint16_t filter_flags;
    /** Channel scan parameter: radio type. */
    uint8_t radio_type;
    /** Channel number. */
    uint8_t chan_number;
    /** mac num of filter. */
    uint8_t filter_num;
    /** Source address of the packet to receive. */
    uint8_t mac_addr[MAX_MONIT_MAC_FILTER_NUM][MLAN_MAC_ADDR_LENGTH];
} NCP_TLV_PACK_START NCP_WLAN_NET_MONITOR_PARA;

/** This structure is used to configure the network monitor information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NET_MONITOR
{   
    /** Monitor configuration. */
    NCP_WLAN_NET_MONITOR_PARA monitor_para;
} NCP_TLV_PACK_END NCP_CMD_NET_MONITOR;

/** This structure is used to configure the register access. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_REGISTER_ACCESS
{  
    /** Operation 0:get, 1:set. */
    uint8_t action;
    /** 1:MAC, 2:BBP, 3:RF, 4:CAU */
    uint8_t type;
    /** Offset of register. */
    uint32_t offset;
    /** value. */
    uint32_t value;
} NCP_TLV_PACK_END NCP_CMD_REGISTER_ACCESS;

/** This structure is used to store the memory status. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_MEM_STAT
{
    /** Size of free heap. */
    uint32_t free_heap_size;
    /** Size of minimun free heap. */
    uint32_t minimum_ever_free_heap_size;
} NCP_TLV_PACK_END NCP_CMD_MEM_STAT;

/** This structure is used to set the CSI configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CSI
{   
    /** CSI configuration. */
    wlan_csi_config_params_t csi_para;
} NCP_TLV_PACK_END NCP_CMD_CSI;

/** This structure is used to configure the enable/disable 802.11k flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_11K_CFG
{   
    /** Enable/disable 802.11k flag, 0: disable, 1: enable. */
    int enable;
} NCP_TLV_PACK_END NCP_CMD_11K_CFG;

/** This structure is used to configure the neighbor request. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_NEIGHBOR_REQ
{
    /** SSID parameter set */
    SSID_ParamSet_t ssid_tlv;
} NCP_TLV_PACK_END NCP_CMD_NEIGHBOR_REQ;

/** This structure is used to configure the enable/disable MBO (multi band operation) flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_MBO_ENABLE
{   
    /** Enable/disable MBO flag, 0: disable, 1: enable. */
    int enable;
} NCP_TLV_PACK_END NCP_CMD_MBO_ENABLE;

typedef NCP_TLV_PACK_START struct _MBO_NONPREFER_CH
{   
    /** channel0 number */
    uint8_t ch0;
    /** channel1 number */
    uint8_t ch1;
    /** Preference0: 0/1/255 */
    uint8_t preference0;
    /** Preference1: 0/1/255 */
    uint8_t preference1;
} NCP_TLV_PACK_END MBO_NONPREFER_CH;

typedef NCP_TLV_PACK_START struct _MBO_NONPREFER_CH_SUPP
{
    char mbo_nonprefer_ch_params[32];
} NCP_TLV_PACK_END MBO_NONPREFER_CH_SUPP;

typedef NCP_TLV_PACK_START struct _NCP_CMD_MBO_NONPREFER_CH
{
    union{
        MBO_NONPREFER_CH mbo_nonprefer_ch_cfg;
        MBO_NONPREFER_CH_SUPP mbo_nonprefer_ch_supp_cfg;
    } NONPREFER_CH_CFG;
} NCP_TLV_PACK_END NCP_CMD_MBO_NONPREFER_CH;

typedef NCP_TLV_PACK_START struct _NCP_CMD_MBO_SET_CELL_CAPA
{
    uint8_t cell_capa;
} NCP_TLV_PACK_END NCP_CMD_MBO_SET_CELL_CAPA;

typedef NCP_TLV_PACK_START struct _NCP_CMD_MBO_SET_OCE
{
    uint8_t oce;
} NCP_TLV_PACK_END NCP_CMD_MBO_SET_OCE;

/** This structure is used to configure the RSSI information. */
typedef NCP_TLV_PACK_START struct
{
    /** Value of last RSSI. */
    int16_t data_rssi_last;
    /** Value of last NF (noise floor). */
    int16_t data_nf_last;
    /** Average value of RSSI. */
    int16_t data_rssi_avg;
    /** Average value of NF (noise floor). */
    int16_t data_nf_avg;
    /** SNR (signal noise ratio) of last beacon. */
    int16_t bcn_snr_last;
	/** Average SNR (signal noise ratio) of beacon. */
    int16_t bcn_snr_avg;
	/** SNR (signal noise ratio) of last data. */
    int16_t data_snr_last;
	/** Average SNR (signal noise ratio) of data. */
    int16_t data_snr_avg;
    /** RSSI of last beacon. */
    int16_t bcn_rssi_last;
    /** NF (noise floor) of last beacon. */
    int16_t bcn_nf_last;
	/** Average RSSI of beacon. */
    int16_t bcn_rssi_avg;
	/** Average NF of beacon. */
    int16_t bcn_nf_avg;
} NCP_TLV_PACK_END NCP_WLAN_RSSI_INFO_T;

/** This structure is used to configure the RSSI information. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RSSI
{
    /** RSSI info */
    NCP_WLAN_RSSI_INFO_T rssi_info;
} NCP_TLV_PACK_END NCP_CMD_RSSI;

/** This structure is used to configure the power manager for the MEF (memory efficient filtering). */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_MEF
{
    /** Type,
     0: delete
     1: ping
     2: arp
     3: multicast
     4: Ipv6 NS
    */
    int type;
    /** Operation,
     0: discard and do not wake up the host.
     1: discard and wake up the host.
     3: allow and wake up the host.
    */
    uint8_t action;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_MEF;

/** This strucure is use to configure the enable/disable UAPSD (unscheduled automatic power save delivery) flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_UAPSD
{   
    /** Enable flag,
    0: disable UAPSD,
    1: enable UAPSD.
    */
    int enable;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_UAPSD;

/** This structure is used to configure the set/get QOS (quality of service) opertaion. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_QOSINFO
{   
    /** QOS information. */
    uint8_t qos_info;
    /** Operation,
    0 - get,
    1 - set
    */
    uint8_t action;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_QOSINFO;

/** This structure is used for set or get the power managerment sleep period. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_SLEEP_PERIOD
{   
    /** Power managerment sleep period */
    uint32_t period;
    /** Operation,
    0 - get,
    1 - set
    */
    uint8_t action;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_SLEEP_PERIOD;

/** This sructure is used to configure the WOWLAN (wake on wireless LAN) of power managerment. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_WOWLAN_CFG
{   
    /** Type,
    0x0: wowlan
    0x1: MEF
    */
    uint8_t is_mef;
    /**
    Wakeup condition bitmap, value for default wowlan conditions only
    bit 0: WAKE_ON_ALL_BROADCAST
    bit 1: WAKE_ON_UNICAST
    bit 2: WAKE_ON_MAC_EVENT
    bit 3: WAKE_ON_MULTICAST
    bit 4: WAKE_ON_ARP_BROADCAST
    bit 6: WAKE_ON_MGMT_FRAME
    All bit 0 discard and not wakeup host
    */
    uint8_t wake_up_conds;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_WOWLAN_CFG;

/** This sructure is used to configure the suspend of power managerment. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_POWERMGMT_SUSPEND
{   
    /** Suspend mode,
    1: PM1
    2: PM2
    3: PM3  
    */
    int mode;
} NCP_TLV_PACK_END NCP_CMD_POWERMGMT_SUSPEND;

/** This structure is used to configure the HE (802.11ax high efficient) capability. */
typedef NCP_TLV_PACK_START struct _HE_CAP_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Element ID extension. */
    uint8_t ext_id;
    /** HE MAC capability info */
    uint8_t he_mac_cap[6];
    /** HE physical capability info */
    uint8_t he_phy_cap[11];
    /** HE txrx MCS support for 80MHz */
    uint8_t he_txrx_mcs_support[4];
    /** value for txrx mcs 160MHz or 80+80, and PPE thresholds */
    uint8_t val[28];
} NCP_TLV_PACK_END HE_CAP_ParamSet_t;

/** This structure is used to configure the band type and HE (802.11ax high efficient) capability. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_11AX_CFG
{
    /** Band, 
     * 0: 2.4GHz, 
     * 2: 5GHz, 
     * 3: 2.4GHz and 5GHz 
     * */
    uint8_t band;
    /** HE (802.11ax high efficient) capability. */
    HE_CAP_ParamSet_t he_cap_tlv;
} NCP_TLV_PACK_END NCP_CMD_11AX_CFG_INFO;

/** This structure is used to configure the BTWT (broadcast TWT).
 * The BTWT (broadcast TWT) is a working mechanism that is managed
 * by the AP. In this mechanism, the TWT time period is announced by the AP,
 * and usually the AP announces the TWT time period of the current round
 * in each beacon frame. In some special cases, the AP will also announce in
 * other management frames, such as association frames, reassociation frames,
 * probe response frames, etc.The endpoint must apply to the AP to add a group
 * before it can run broadcast TWT. After the STA completes the group adding,
 * the STA works according to the latest TWT time period, and the STA wakes
 * up after the TWT time period is reached and exchanges information.
 */
typedef NCP_TLV_PACK_START struct _NCP_CMD_BTWT_CFG
{
    /** Operation, only support 1: set. */
    uint16_t action;
    /** Subcommand ID, the subcommand ID of BTWT is 0x125. */
    uint16_t sub_id;
    /** Nominal minimum TWT wake duration. */
    uint8_t nominal_wake;
    /** Maximum number of STAs supported. */
    uint8_t max_sta_support;
    /** TWT wake interval mantissa. */
    uint16_t twt_mantissa;
    /** TWT wake interval offset. */
    uint16_t twt_offset;
    /** TWT exponent, range: [0-63] */
    uint8_t twt_exponent;
    /** Service period gap. */
    uint8_t sp_gap;
} NCP_TLV_PACK_END NCP_CMD_BTWT_CFG_INFO;

/** This structure is used to configure the BTWT. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_TWT_SETUP
{
    /** Implicit enable flag, 0: TWT session is explicit, 1: session is implicit. */
    uint8_t implicit;
    /** Announced enable flag, 0: unannounced, 1: announced TWT. */
    uint8_t announced;
    /** Trigger enable flag, 0: Non-Trigger enabled, 1: trigger enabled TWT. */
    uint8_t trigger_enabled;
    /** TWT information disable flag, 0: TWT info enabled, 1: TWT info disabled. */
    uint8_t twt_info_disabled;
    /** Negotiation type, 0: individual TWT, 3: broadcast TWT. */
    uint8_t negotiation_type;
    /** TWT wakeup duration, time after which the TWT requesting STA can
     * transition to doze state. */
    uint8_t twt_wakeup_duration;
    /** Flow identifier. range: [0-7]. */
    uint8_t flow_identifier;
    /** Hard constraint, 0: FW can tweak the TWT setup parameters if it is
     *rejected by AP.
     ** 1: firmware should not tweak any parameters. */
    uint8_t hard_constraint;
    /** TWT exponent, range: [0-63]. */
    uint8_t twt_exponent;
    /** TWT wake interval mantissa, range: [0-sizeof(UINT16)]. */
    uint16_t twt_mantissa;
    /** TWT request type, 0: REQUEST_TWT, 1: SUGGEST_TWT. */
    uint8_t twt_request;
} NCP_TLV_PACK_END NCP_CMD_TWT_SETUP_CFG;

/** This structure is used to configure the TWT teardown. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_TWT_TEARDOWN
{
    /** TWT flow identifier, range from 0 to 7. */
    uint8_t flow_identifier;
    /** Negotiation type. 0: future individual TWT service period start time,
    1: Next wake TBTT (target beacon transmission time).
    */
    uint8_t negotiation_type;
    /** Tear down all TWT enable flag. 1: to teardown all TWT, 0: none to do. */
    uint8_t teardown_all_twt;
} NCP_TLV_PACK_END NCP_CMD_TWT_TEARDOWN_CFG;

/** This structure is used to configure the IEEE BTWT. */
typedef NCP_TLV_PACK_START struct _IEEE_BTWT_ParamSet_t
{
    /**
     *  [Bit 0]     request
     *  [Bit 1-3]   setup_cmd
     *  [Bit 4]     trigger
     *  [Bit 5]     last_broadcast_parameter_set
     *  [Bit 6]     flow_type
     *  [Bit 7-9]   btwt_recommendation
     *  [Bit 10-14] wake_interval_exponent
     *  [Bit 15]    reserved.
     */
    uint16_t request_type;
    /** Wake up time agreed by the device and host. */
    uint16_t target_wake_time;
    /** Nominal minimum TWT wake duration. */
    uint8_t nominal_min_wake_duration;
    /** TWT wake interval mantissa . */
    uint16_t wake_interval_mantissa;
    /**
     *  [Bit 0-2]   reserved
     *  [Bit 3-7]   btwt_id
     *  [Bit 8-15]  btwt_persistence.
     */
    uint16_t twt_info;
} NCP_TLV_PACK_END IEEE_BTWT_ParamSet_t;

/** This structure is used to configure the TWT report informaiton.  */
typedef NCP_TLV_PACK_START struct _NCP_CMD_TWT_REPORT
{
    /** TWT report type, 0: BTWT id. */
    uint8_t type;
    /** TWT report length of value in data. */
    uint8_t length;
    uint8_t reserve[2];
    /** TWT report payload for FW response to fill, 4 * 9bytes. */
    IEEE_BTWT_ParamSet_t info[4];
} NCP_TLV_PACK_END NCP_CMD_TWT_REPORT;

/** This structure is used to set the enable/disable 802.11d flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_11D_ENABLE
{
    /** 0: STA, 
     * 1: UAP.
     *  */
    uint32_t role;
    /** 
     * 0: disable 802.11d,
     * 1: enable 802.11d.
     */
    uint32_t state;
} NCP_TLV_PACK_END NCP_CMD_11D_ENABLE_CFG;

/** This structure is used to configure the region code. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_REGION_CODE
{
    /** 0 - get, 1 - set */
    uint32_t action;
    /** Region code, 0x00 for world wide safe, 0x50 for CN, etc. */
    uint32_t region_code;
} NCP_TLV_PACK_END NCP_CMD_REGION_CODE_CFG;

/** This structure is used to set the maximum number of STAs that a UAP can support. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_CLIENT_CNT
{   
    /** Maximum number of STAs set in the command. */
    uint16_t max_sta_count;
    /** Set status. 0: success, 1: fail. */
    uint8_t set_status;
    /** Maximum number of STAs supported by the device side. */
    uint8_t support_count;
} NCP_TLV_PACK_END NCP_CMD_CLIENT_CNT;

/** This structure is used to configure the antenna. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ANTENNA_CFG
{   
    /** Operation type, 0:get, 1:set */
    uint8_t action;
    /** Antenna mode, 0: 1ANT, 1: 2ANT, 15: ANT diversity. */
    uint32_t antenna_mode;
    /** Evaluate time. */
    uint16_t evaluate_time;
    /** Evaluate mode. */
    uint8_t evaluate_mode;
    /** Current antenna configuration. */
    uint16_t current_antenna;
} NCP_TLV_PACK_END NCP_CMD_ANTENNA_CFG;

/** This structure is used to set the enable/disable deep sleep power save mode flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_DEEP_SLEEP_PS
{
    /** 
    0:Disable deep sleep power save mode.
    1:Enable deep sleep power save mode.
    */
    int enable;
} NCP_TLV_PACK_END NCP_CMD_DEEP_SLEEP_PS;

/** This structure is used to set the enable/disable IEEE power save mode flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_IEEE_PS
{
    /** 
    0:Disable ieee power save mode
    1:Enable ieee power save mode
    */
    int enable;
} NCP_TLV_PACK_END NCP_CMD_IEEE_PS;

/** This structure is used to set the enable/disable MAC mode flag. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_ED_MAC
{   
    /** Option, 0: get, 1: set. */
    uint8_t action;
    /** Enable/disable EU adaptivity for 2.4GHz band. */
    uint16_t ed_ctrl_2g;
    /** Energy detect threshold for 2.4GHz band. */
    uint16_t ed_offset_2g;
#if CONFIG_NCP_5GHz_SUPPORT
    /** Enable/disable EU adaptivity for 5GHz band. */
    uint16_t ed_ctrl_5g;
    /** Energy detect threshold for 5GHz band. */
    uint16_t ed_offset_5g;
#endif
} NCP_TLV_PACK_END NCP_CMD_ED_MAC;

#if CONFIG_NCP_RF_TEST_MODE
/** This structure is used to configure the RF TX antenna. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_TX_ANTENNA
{   
    /** 1: main antenna, 2: aux antenna. */
    uint8_t ant;
} NCP_TLV_PACK_END NCP_CMD_RF_TX_ANTENNA;

/** This structure is used to set the RF RX antenna. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_RX_ANTENNA
{   
    /** 1: main antenna, 2: aux antenna. */
    uint8_t ant;
} NCP_TLV_PACK_END NCP_CMD_RF_RX_ANTENNA;

/** This structure is used to configure the RF RX band. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_BAND
{   
    /** Band: 0: 2.4GHz, 1: 5GHz. */
    uint8_t band;
} NCP_TLV_PACK_END NCP_CMD_RF_BAND;

/** This structure is used to set the RF bandwidth. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_BANDWIDTH
{   
    /** 
    0: 20MHz;
    1: 40MHz;
    4: 80MHz
    */
    uint8_t bandwidth;
} NCP_TLV_PACK_END NCP_CMD_RF_BANDWIDTH;

/** This structure is used to set the RF channel number. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_CHANNEL
{   
    /** 2.4GHz channel numbers or 5GHz channel numbers. */
    uint8_t channel;
} NCP_TLV_PACK_END NCP_CMD_RF_CHANNEL;

/** This structure is used to set the RF radio mode. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_RADIO_MODE
{  
    /** 
    Radio mode,
    0: set radio in power down mode,
    3: sets radio in 5GHz band, 1X1 mode(path A),
    11: sets radio in 2.4GHz band, 1X1 mode(path A).
    */
    uint8_t radio_mode;
} NCP_TLV_PACK_END NCP_CMD_RF_RADIO_MODE;

/** This structure is used to set the RF TX power configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_TX_POWER
{
    /** Power value: 0~24 dBm. */
	uint8_t power;
	/** Modulation mode, (0: CCK, 1:OFDM, 2:MCS). */
    uint8_t mod;
	/**
	0: PathA
	1: PathB
	2: PathA+B
     */
    uint8_t path_id;
} NCP_TLV_PACK_END NCP_CMD_RF_TX_POWER;

/** This stucture is used to set the RF TX continuous configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_TX_CONT_MODE
{   
    /** Enable TX flag, 0: disable TX, 1: enable TX. */
    uint32_t enable_tx;
	/** Enable continuous wave mode flag, 0: disable, 1: enable.  */
    uint32_t cw_mode;
	/** Payload pattern, 0 to 0xFFFFFFF (enter hexadecimal value). */
    uint32_t payload_pattern;
	/** Enable CS mode flag, applicable only when continuous wave is disabled,
	0:disable, 1:enable.
	*/
    uint32_t cs_mode;
	/** Active sub channel, 0: low, 1: upper, 3: both. */
    uint32_t act_sub_ch;
	/** Transmit data rate, rate index corresponding to legacy/HT/VHT rates. */
    uint32_t tx_rate;
} NCP_TLV_PACK_END NCP_CMD_RF_TX_CONT_MODE;

/** This stucture is used to set the RF TX frame configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_TX_FRAME
{   
    /** Enable flag, 0: disable, 1: enable. */
    uint32_t enable;
    /** Transmit data rate, rate index corresponding to legacy/HT/VHT rates. */
    uint32_t data_rate;
    /** Payload pattern, (0 to 0xFFFFFFFF) (Enter hexadecimal value) */
    uint32_t frame_pattern;
    /** Payload length, (1 to 0x400) (Enter hexadecimal value) */
    uint32_t frame_length;
    /** Adjust burst SIFS3 gap enable flag (0:disable, 1:enable) */
    uint32_t adjust_burst_sifs;
    /** Burst SIFS in us (0 to 255us) */
    uint32_t burst_sifs_in_us;
    /** Enable short preamble flag (0: disable, 1: enable). */
    uint32_t short_preamble;
    /** Enable active sub channel flag (0: low, 1: upper, 3: both). */
    uint32_t act_sub_ch;
    /** Enable short GI flag (0: disable, 1: enable). */
    uint32_t short_gi;
    /** Eable advanced coding flag (0: disable, 1: enable). */
    uint32_t adv_coding;
    /** Enable beamforming flag (0: disable, 1: enable). */
    uint32_t tx_bf;
    /** Enable green field mode flag (0: disable, 1: enable). */
    uint32_t gf_mode;
    /** Enable STBC (space time block coding) flag (0: disable, 1: enable). */
    uint32_t stbc;
    /** Bssid string. */
    uint8_t bssid[MLAN_MAC_ADDR_LENGTH];
} NCP_TLV_PACK_END NCP_CMD_RF_TX_FRAME;

/** This structure is used to configure the RF per command. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_RF_PER
{   
    /** Sum of RX packet count. */
    uint32_t rx_tot_pkt_count;
    /** Sum of RX multicast/broadcast packet count. */
    uint32_t rx_mcast_bcast_count;
    /** Sum of RX packets with FCS (frame check sequence) error count. */
    uint32_t rx_pkt_fcs_error;
} NCP_TLV_PACK_END NCP_CMD_RF_PER;
#endif

/** This structure is used to set the EU crypto. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_EU_CRYPRTO
{   
    /** 0: decrypt, 1: encrypt. */
    uint8_t enc;
} NCP_TLV_PACK_END NCP_CMD_EU_CRYPRTO;

/** This structure is used to configure the date time. */
typedef NCP_TLV_PACK_START struct _wlan_date_time_t
{   
    /** Operation type, 0: get, 1: set. */
    uint32_t action;
    /** Year: range from 1970 to 2099. */
    uint16_t year;  
    /** Month: range from 1 to 12. */
    uint8_t month; 
    /** Day: range from 1 to 31 (depending on month). */
    uint8_t day;   
    /** Hour: range from 0 to 23. */
    uint8_t hour;   
    /** Minute: range from 0 to 59. */
    uint8_t minute;
    /** Minute: range from 0 to 59. */
    uint8_t second;
} NCP_TLV_PACK_END wlan_date_time_t;

/** This structure is used to set or get the date time. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_DATE_TIME
{   
    /** Operation type, 0: get, 1: set. */
    uint32_t action;
    /** Date time structure. */
    wlan_date_time_t date_time;
} NCP_TLV_PACK_END NCP_CMD_DATE_TIME_CFG;

/** This structure is used for the temperature of the device. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_TEMPERATURE
{   
    /** Temperature (celsius) value. */
    uint32_t temp;
} NCP_TLV_PACK_END NCP_CMD_TEMPERATURE;

/** This structure is used to set the Wi-Fi connection configuration. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_WLAN_CONN
{   
    /** Wi-Fi network profile name. */
    char name[WLAN_NETWORK_NAME_MAX_LENGTH];
    /** IP address of STA after the connection is successful. */
    uint32_t ip;
    /** SSID of the connected Wi-Fi network. */
    char ssid[IEEEtypes_SSID_SIZE + 1];
} NCP_TLV_PACK_END NCP_CMD_WLAN_CONN;

/** This structure is used to configure the service protocol. */
typedef NCP_TLV_PACK_START struct _QUERY_PTR_CFG
{
    /** Type of service, like "http". */
    char service[63 + 1];
    /** Protocol type, 0: UDP, 1: TCP. */
    uint16_t proto;
} NCP_TLV_PACK_END QUERY_PTR_CFG;

/** This structure is used to configure the domain name. */
typedef NCP_TLV_PACK_START struct _QUERY_A_CFG
{
    /** Domain name, like "wifi-http.local". */
    char name[63 + 1];
} NCP_TLV_PACK_END QUERY_A_CFG;

/** This structure is used to configure the MDNS query command. */
typedef NCP_TLV_PACK_START struct _NCP_CMD_MDNS_QUERY
{
    /** Query type, PTR (pointer records), SRV (service records), A (Ipv4), AAAA (IPv6). */
    uint8_t qtype;
    union
    {
        QUERY_PTR_CFG ptr_cfg;
        QUERY_A_CFG a_cfg;
    } Q;
} NCP_TLV_PACK_END NCP_CMD_MDNS_QUERY;

/** This structure is used to configure the pointer records for MDNS. */
typedef NCP_TLV_PACK_START struct _PTR_ParamSet_t
{   
    /** Header, include type and size. */
    TypeHeader_t header;
    /* Instance name. */
    char instance_name[63 + 1];
    /* Service type. */
    char service_type[63 + 1];
    /* Service protocol. */
    char proto[8];
} NCP_TLV_PACK_END PTR_ParamSet_t;

/** This structure is used to configure the service records for MDNS. */
typedef NCP_TLV_PACK_START struct _SRV_ParamSet_t
{    
    /** Header, include type and size. */
    TypeHeader_t header;
    /** Host name. */
    char host_name[63 + 1];
    /** Service port. */
    uint16_t port;
    /** Target name. */
    char target[63 + 1];
} NCP_TLV_PACK_END SRV_ParamSet_t;

/** This structure is used to configure the txt resource for MDNS. */
typedef NCP_TLV_PACK_START struct _TXT_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** txt value length. */
    uint8_t txt_len;
    /** txt string. */
    char txt[63 + 1];
} NCP_TLV_PACK_END TXT_ParamSet_t;

/** This structure is used to configure the IP address. */
typedef NCP_TLV_PACK_START struct _IP_ADDR_ParamSet_t
{
    /** Header, include type and size. */
    TypeHeader_t header;
    /** 
    IP address type. 
    1: ADDR_TYPE_DHCP, use DHCP to obtain the IP address.
    0: ADDR_TYPE_STATIC, use a static IP address.
    */
    uint8_t addr_type;
    /** IPv4/IPv6 address. */
    union {
        uint32_t ip_v4;
        uint32_t ip_v6[4];
    } ip;
} NCP_TLV_PACK_END IP_ADDR_ParamSet_t;

#define PTR_TLV_LEN (sizeof(PTR_ParamSet_t))
#define SRV_TLV_LEN (sizeof(SRV_ParamSet_t))
#define TXT_TLV_LEN (sizeof(TXT_ParamSet_t))
#define IP_ADDR_TLV_LEN (sizeof(IP_ADDR_ParamSet_t))

/** This structure is used to configure the MDNS query result. */
typedef NCP_TLV_PACK_START struct _NCP_EVT_MDNS_RESULT
{
    /** Time to live. */
    uint32_t ttl;
    /** Length of TLVs sent in command starting at buffer. */
    uint32_t tlv_buf_len;
    /** Payload buffer of MDNS query command. */
    uint8_t tlv_buf[1];
} NCP_TLV_PACK_END NCP_EVT_MDNS_RESULT;

/** This structure is used to configure the CSI (channel state information) data. */
typedef NCP_TLV_PACK_START struct _NCP_EVT_CSI_DATA
{
    /** Length in dwords, including header. */
    uint16_t Len;
    /** CSI signature. 0xABCD fixed. */
    uint16_t CSI_Sign;
    /** User defined header ID. */
    uint32_t CSI_HeaderID;
    /** Packet info field. */
    uint16_t PKT_info;
    /** Frame control field for the received packet. */
    uint16_t FCF;
    /** Timestamp when packet received */
    uint64_t TSF;
    /** Destination MAC address of received packet. */
    uint8_t Dst_MAC[6];
    /** Source MAC address of received packet. */
    uint8_t Src_MAC[6];
    /** RX RSSI for antenna A. */
    uint8_t Rx_RSSI_A;
    /** RX RSSI for antenna B. */
    uint8_t Rx_RSSI_B;
    /** Noise floor for antenna A. */
    uint8_t Rx_NF_A;
    /** Noise floor for antenna B. */
    uint8_t Rx_NF_B;
    /** RX signal strength above noise floor. */
    uint8_t Rx_SINR;
    /** Channel. */
    uint8_t channel;
    /** User defined chip ID. */
    uint16_t chip_id;
    /** Reserved. */
    uint32_t rsvd;
    /** CSI data length in DWORDs. */
    uint32_t CSI_Data_Length;
    /** Start of CSI data. */
    uint8_t CSI_Data[0];
    /** At the end of CSI raw data, user defined tail ID of 4 bytes. */
} NCP_TLV_PACK_END NCP_EVT_CSI_DATA;

/** This structure is for MDNS domain event. */
typedef NCP_TLV_PACK_START struct _NCP_EVT_MDNS_RESOLVE
{   
    /** IP type, 
    0: IPv4, 1: IPv6.
    */
    uint8_t ip_type;
	/** IPv4/IPv6 address. */
    union {
      uint32_t ip6_addr[4];
      uint32_t ip4_addr;
    } u_addr;
} NCP_TLV_PACK_END NCP_EVT_MDNS_RESOLVE;

/** NCP command structure. */
typedef NCP_TLV_PACK_START struct _NCPCmd_DS_COMMAND
{
    /** Command header. */
    NCP_COMMAND header;
    /** Command body. */
    union
    {
        /** Scan result. */
        NCP_CMD_SCAN_NETWORK_INFO scan_network_info;
        /** Driver and firmware version */
        NCP_CMD_FW_VERSION fw_version;
        /** MAC address */
        NCP_CMD_MAC_ADDRESS mac_addr;
        /** Get MAC address */
        NCP_CMD_GET_MAC_ADDRESS get_mac_addr;
        /** wlan connnection state */
        NCP_CMD_CONNECT_STAT conn_stat;
        /** wlan network info*/
        NCP_CMD_NETWORK_INFO network_info;
        /** wlan network address*/
        NCP_CMD_NETWORK_ADDRESS network_address;
        NCP_CMD_NET_MONITOR monitor_cfg;
        /** wlan add network */
        NCP_CMD_NETWORK_ADD network_add;
        /** wlan start network*/
        NCP_CMD_NETWORK_START network_start;
        /** wlan uap sta list */
        NCP_CMD_NETWORK_UAP_STA_LIST uap_sta_list;
		/** CSI configuration */
        NCP_CMD_CSI csi_cfg;
		/** 802.11k enabale configuration */
        NCP_CMD_11K_CFG wlan_11k_cfg;
		/** neighbor report request */
        NCP_CMD_NEIGHBOR_REQ neighbor_req;
        /** MBO enable configure */
        NCP_CMD_MBO_ENABLE wlan_mbo_enable;
		/** NCP MBO non prefer channel structure */
        NCP_CMD_MBO_NONPREFER_CH mbo_nonprefer_ch;
        /** MBO cell capacity */
		NCP_CMD_MBO_SET_CELL_CAPA wlan_mbo_set_cell_capa;
        /** NCP MBO set OCE */
		NCP_CMD_MBO_SET_OCE wlan_mbo_set_oce;

        /** RSSI information*/
        NCP_CMD_RSSI signal_rssi;
        /** Roaming configurations */
        NCP_CMD_ROAMING roaming_cfg;
        /** MAX client count*/
        NCP_CMD_CLIENT_CNT max_client_count;
        /** Antenna config*/
        NCP_CMD_ANTENNA_CFG antenna_cfg;
		/** Generated WPS pin code */
        NCP_CMD_WPS_GEN_PIN wps_gen_pin_info;
		/** wps pin code */
        NCP_CMD_WPS_PIN wps_pin_cfg;
        /** 802.11ax configure information */
        NCP_CMD_11AX_CFG_INFO he_cfg;
		/** TWT broadcast configuration information */
        NCP_CMD_BTWT_CFG_INFO btwt_cfg;
		/** TWT setup configuration */
        NCP_CMD_TWT_SETUP_CFG twt_setup;
		/** TWT teardown configuration */
        NCP_CMD_TWT_TEARDOWN_CFG twt_teardown;
		/** 802.11d enable configuration */
        NCP_CMD_11D_ENABLE_CFG wlan_11d_cfg;
        /** socket open configuration */
        NCP_CMD_SOCKET_OPEN_CFG wlan_socket_open;
		/** socket connect configuration */
        NCP_CMD_SOCKET_CON_CFG wlan_socket_con;
		/** socket bind configuration */
        NCP_CMD_SOCKET_BIND_CFG wlan_socket_bind;
        /** socket close configuration */
        NCP_CMD_SOCKET_CLOSE_CFG wlan_socket_close;
        /** socket listen configuration */
        NCP_CMD_SOCKET_LISTEN_CFG wlan_socket_listen;
        /** socket accept configuration */
        NCP_CMD_SOCKET_ACCEPT_CFG wlan_socket_accept;
		/** socket send configuration */
        NCP_CMD_SOCKET_SEND_CFG wlan_socket_send;
		/** socket sendto configuration */
        NCP_CMD_SOCKET_SENDTO_CFG wlan_socket_sendto;
		/** socket receive configuration */
        NCP_CMD_SOCKET_RECEIVE_CFG wlan_socket_receive;
		/** socket recvfrom configuration */
        NCP_CMD_SOCKET_RECVFROM_CFG wlan_socket_recvfrom;
        /** http connect configuration */
        NCP_CMD_HTTP_CON_CFG wlan_http_connect;
		/** http disconnect configuration */
        NCP_CMD_HTTP_DISCON_CFG wlan_http_disconnect;
        /** http seth. */
        NCP_CMD_HTTP_SETH_CFG wlan_http_seth;
        /** http unseth. */
        NCP_CMD_HTTP_UNSETH_CFG wlan_http_unseth;
        /** http request. */
        NCP_CMD_HTTP_REQ_CFG wlan_http_req;
        /** http request response */
        NCP_CMD_HTTP_REQ_RESP_CFG wlan_http_req_resp;
        /** http receive configuration. */
        NCP_CMD_HTTP_RECV_CFG wlan_http_recv;
        /** Http upgrade */
        NCP_CMD_HTTP_UPG_CFG wlan_http_upg;
        /** NCP web socket send configuration. */
        NCP_CMD_WEBSOCKET_SEND_CFG wlan_websocket_send;
        /** NCP web socket receive configuration. */
        NCP_CMD_WEBSOCKET_RECV_CFG wlan_websocket_recv;

        /** MEF configurations. */
        NCP_CMD_POWERMGMT_MEF mef_config;
        /** Wi-Fi deep sleep power save mode. */
        NCP_CMD_DEEP_SLEEP_PS wlan_deep_sleep_ps;
        /** Wlan IEEE power save mode configure. */
        NCP_CMD_IEEE_PS wlan_ieee_ps;
		/** NCP enable/disable UAPSD configuration*/
        NCP_CMD_POWERMGMT_UAPSD uapsd_cfg;
		/** NCP set/get WMM Qos configuration  */
        NCP_CMD_POWERMGMT_QOSINFO qosinfo_cfg;
		/** NCP set/get sleep period */
        NCP_CMD_POWERMGMT_SLEEP_PERIOD sleep_period_cfg;
		/** Wakeup configuration. */
        NCP_CMD_POWERMGMT_WOWLAN_CFG wowlan_config;
        /** Enter suspend configuration. */
        NCP_CMD_POWERMGMT_SUSPEND suspend_config;
        /** Wi-Fi reset config. **/
        WLAN_RESET_data reset_config;
        /** TWT report. */
        NCP_CMD_TWT_REPORT twt_report;
        /** Region code configuration structure. */
        NCP_CMD_REGION_CODE_CFG region_cfg;

        /** Regulatory commands. */
        NCP_CMD_ED_MAC ed_mac_mode;
#if CONFIG_NCP_RF_TEST_MODE
        /** NCP RF TX antenna. */
        NCP_CMD_RF_TX_ANTENNA rf_tx_antenna;
        /** NCP RF RX antenna. */
        NCP_CMD_RF_RX_ANTENNA rf_rx_antenna;
		/** NCP RF band. */
        NCP_CMD_RF_BAND rf_band;
        /** NCP RF bandwidth. */
        NCP_CMD_RF_BANDWIDTH rf_bandwidth;
        /** NCP RF channel. */
        NCP_CMD_RF_CHANNEL rf_channel;
        /** NCP RF radio mode. */
        NCP_CMD_RF_RADIO_MODE rf_radio_mode;
        /** NCP RF TX power. */
        NCP_CMD_RF_TX_POWER rf_tx_power;
        /** NCP RF TX cont mode. */
        NCP_CMD_RF_TX_CONT_MODE rf_tx_cont_mode;
        /** NCP RF TX frame. */
        NCP_CMD_RF_TX_FRAME rf_tx_frame;
        /** NCP RF per. */
        NCP_CMD_RF_PER rf_per;
#endif
        /** NCP EU cryprto. */
        NCP_CMD_EU_CRYPRTO eu_crypto;

        /** Debug commands. */
        NCP_CMD_REGISTER_ACCESS register_access;
        /** Memory commands. */
        NCP_CMD_MEM_STAT mem_stat;
        /** Data time. */
        NCP_CMD_DATE_TIME_CFG date_time;
        /** Chip temperature. */
        NCP_CMD_TEMPERATURE temperature;

        /** Wi-Fi connect. */
        NCP_CMD_WLAN_CONN wlan_connect;
        /** MDNS query. */
        NCP_CMD_MDNS_QUERY mdns_query;
        /** MDNS reuslt. */
        NCP_EVT_MDNS_RESULT mdns_result;
        /** MDNS resolve. */
        NCP_EVT_MDNS_RESOLVE mdns_resolve;
        /** CSI data. */
        NCP_EVT_CSI_DATA csi_data;
        /** Added network list. */
        NCP_CMD_NETWORK_LIST network_list;
        /** Network to remove. */
        NCP_CMD_NETWORK_REMOVE network_remove;
    } params;
} NCP_TLV_PACK_END NCPCmd_DS_COMMAND, MCU_NCPCmd_DS_COMMAND;

#endif /* __NCP_CMD_WIFI_H__ */
