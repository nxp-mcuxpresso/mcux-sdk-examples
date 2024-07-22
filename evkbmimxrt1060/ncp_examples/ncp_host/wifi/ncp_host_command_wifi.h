/**@file ncp_host_command_wifi.h
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */

#ifndef __NCP_HOST_COMMAND_WIFI_H_
#define __NCP_HOST_COMMAND_WIFI_H_

#include "ncp_adapter.h"
#include "ncp_host_utils_wifi.h"

/*Define macros which are used in ncp_cmd_wifi.h but not defined in mcu project.*/
#define MLAN_MAX_VER_STR_LEN         128
#define MLAN_MAC_ADDR_LENGTH         6
#define WLAN_NETWORK_NAME_MAX_LENGTH 32
#define IEEEtypes_SSID_SIZE          32
#define IDENTITY_MAX_LENGTH          64
#define PASSWORD_MAX_LENGTH          128
#define MAX_NUM_CLIENTS              16
#define MAX_MONIT_MAC_FILTER_NUM     3
#define IEEEtypes_ADDRESS_SIZE       6
#define CSI_FILTER_MAX               16

#define NCP_IPERF_END_TOKEN_SIZE 11
/* Min WPA2 passphrase can be upto 8 ASCII chars */
#define WLAN_PSK_MIN_LENGTH 8U
/** Max WPA2 passphrase can be upto 63 ASCII chars or 64 hexadecimal digits*/
#define WLAN_PSK_MAX_LENGTH 65U

/*Structures which are used by ncp_cmd_wifi.h but not defined in mcu project.*/
/** Station information structure */
typedef struct
{
    /** MAC address buffer */
    uint8_t mac[MLAN_MAC_ADDR_LENGTH];
    /**
     * Power management status
     * 0 = active (not in power save)
     * 1 = in power save status
     */
    uint8_t power_mgmt_status;
    /** RSSI: dBm */
    char rssi;
} wifi_sta_info_t;

typedef NCP_TLV_PACK_START struct _wifi_csi_filter_t
{
    /** Source address of the packet to receive */
    uint8_t mac_addr[MLAN_MAC_ADDR_LENGTH];
    /** Pakcet type of the interested CSI */
    uint8_t pkt_type;
    /** Packet subtype of the interested CSI */
    uint8_t subtype;
    /** Other filter flags */
    uint8_t flags;
} NCP_TLV_PACK_END wifi_csi_filter_t;

typedef NCP_TLV_PACK_START struct _wifi_csi_config_params_t
{
    uint8_t bss_type;
    /** CSI enable flag. 1: enable, 2: disable */
    uint16_t csi_enable;
    /** Header ID*/
    uint32_t head_id;
    /** Tail ID */
    uint32_t tail_id;
    /** Number of CSI filters */
    uint8_t csi_filter_cnt;
    /** Chip ID */
    uint8_t chip_id;
    /** band config */
    uint8_t band_config;
    /** Channel num */
    uint8_t channel;
    /** Enable getting CSI data on special channel */
    uint8_t csi_monitor_enable;
    /** CSI data received in cfg channel with mac addr filter, not only RA is us or other*/
    uint8_t ra4us;
    /** CSI filters */
    wifi_csi_filter_t csi_filter[CSI_FILTER_MAX];
} NCP_TLV_PACK_END wifi_csi_config_params_t;

typedef wifi_csi_config_params_t wlan_csi_config_params_t;

#include "ncp_cmd_wifi.h"

/*mcu macros*/
#define MAC2STR(a)              a[0], a[1], a[2], a[3], a[4], a[5]
#define NCP_HOST_IP_LENGTH      4
#define NCP_HOST_IP_VALID       255
#define NCP_HOST_MAX_AP_ENTRIES 30

#define ACTION_GET 0
#define ACTION_SET 1

#define NCP_WLAN_MAC_ADDR_LENGTH 6
#define MAX_MONIT_MAC_FILTER_NUM 3

enum wlan_monitor_opt
{
    MONITOR_FILTER_OPT_ADD_MAC = 0,
    MONITOR_FILTER_OPT_DELETE_MAC,
    MONITOR_FILTER_OPT_CLEAR_MAC,
    MONITOR_FILTER_OPT_DUMP,
};

enum wlan_csi_opt
{
    CSI_FILTER_OPT_ADD = 0,
    CSI_FILTER_OPT_DELETE,
    CSI_FILTER_OPT_CLEAR,
    CSI_FILTER_OPT_DUMP,
};

/** The space reserved for storing network names */
#define WLAN_NETWORK_NAME_MAX_LENGTH 32

#define WLAN_SSID_MAX_LENGTH 32

/** The operation could not be performed in the current system state. */
#define WLAN_ERROR_STATE 3

#define DNS_RRTYPE_A   1  /* a host address */
#define DNS_RRTYPE_PTR 12 /* a domain name pointer */

enum mdns_sd_proto
{
    DNSSD_PROTO_UDP = 0,
    DNSSD_PROTO_TCP = 1
};

#define MDNS_ADDRTYPE_IPV4 0
#define MDNS_ADDRTYPE_IPV6 1

enum wlan_mef_type
{
    MEF_TYPE_DELETE = 0,
    MEF_TYPE_PING,
    MEF_TYPE_ARP,
    MEF_TYPE_MULTICAST,
    MEF_TYPE_IPV6_NS,
    MEF_TYPE_END,
};

#if CONFIG_NCP_WIFI_CAPA
#define WIFI_SUPPORT_11AX   (1 << 3)
#define WIFI_SUPPORT_11AC   (1 << 2)
#define WIFI_SUPPORT_11N    (1 << 1)
#define WIFI_SUPPORT_LEGACY (1 << 0)
#endif

/** Network wireless BSS Role */
enum wlan_bss_role
{
    /** Infrastructure network. The system will act as a station connected
     *  to an Access Point. */
    WLAN_BSS_ROLE_STA = 0,
    /** uAP (micro-AP) network.  The system will act as an uAP node to
     * which other Wireless clients can connect. */
    WLAN_BSS_ROLE_UAP = 1,
    /** Either Infrastructure network or micro-AP network */
    WLAN_BSS_ROLE_ANY = 0xff,
};

enum wlan_security_type
{
    /** The network does not use security. */
    WLAN_SECURITY_NONE,
    /** The network uses WEP security with open key. */
    WLAN_SECURITY_WEP_OPEN,
    /** The network uses WEP security with shared key. */
    WLAN_SECURITY_WEP_SHARED,
    /** The network uses WPA security with PSK. */
    WLAN_SECURITY_WPA,
    /** The network uses WPA2 security with PSK. */
    WLAN_SECURITY_WPA2,
    /** The network uses WPA/WPA2 mixed security with PSK */
    WLAN_SECURITY_WPA_WPA2_MIXED,
#if CONFIG_NCP_11R
    /** The network uses WPA2 security with PSK FT. */
    WLAN_SECURITY_WPA2_FT,
#endif
    /** The network uses WPA3 security with SAE. */
    WLAN_SECURITY_WPA3_SAE,
#if CONFIG_NCP_WPA_SUPP
#if CONFIG_NCP_11R
    /** The network uses WPA3 security with SAE FT. */
    WLAN_SECURITY_WPA3_FT_SAE,
#endif
#endif
    /** The network uses WPA2/WPA3 SAE mixed security with PSK. This security mode
     * is specific to uAP or SoftAP only */
    WLAN_SECURITY_WPA2_WPA3_SAE_MIXED,
#if CONFIG_NCP_OWE
    /** The network uses OWE only security without Transition mode support. */
    WLAN_SECURITY_OWE_ONLY,
#endif
#if CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE
    /** The network uses WPA2 Enterprise EAP-TLS security
     * The identity field in \ref wlan_network structure is used */
    WLAN_SECURITY_EAP_TLS,
#endif
#if CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE
#if CONFIG_NCP_EAP_TLS
    /** The network uses WPA2 Enterprise EAP-TLS SHA256 security
     * The identity field in \ref wlan_network structure is used */
    WLAN_SECURITY_EAP_TLS_SHA256,
#if CONFIG_NCP_11R
    /** The network uses WPA2 Enterprise EAP-TLS FT security
     * The identity field in \ref wlan_network structure is used */
    WLAN_SECURITY_EAP_TLS_FT,
    /** The network uses WPA2 Enterprise EAP-TLS FT SHA384 security
     * The identity field in \ref wlan_network structure is used */
    WLAN_SECURITY_EAP_TLS_FT_SHA384,
#endif
#endif
#if CONFIG_NCP_EAP_TTLS
    /** The network uses WPA2 Enterprise EAP-TTLS security
     * The identity field in \ref wlan_network structure is used */
    WLAN_SECURITY_EAP_TTLS,
#if CONFIG_NCP_EAP_MSCHAPV2
    /** The network uses WPA2 Enterprise EAP-TTLS-MSCHAPV2 security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_TTLS_MSCHAPV2,
#endif
#endif
#endif
#if (CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE) || (CONFIG_NCP_PEAP_MSCHAPV2)
    /** The network uses WPA2 Enterprise EAP-PEAP-MSCHAPV2 security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_PEAP_MSCHAPV2,
#endif
#if CONFIG_NCP_WPA_SUPP_CRYPTO_ENTERPRISE
#if CONFIG_NCP_EAP_PEAP
#if CONFIG_NCP_EAP_TLS
    /** The network uses WPA2 Enterprise EAP-PEAP-TLS security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_PEAP_TLS,
#endif
#if CONFIG_NCP_EAP_GTC
    /** The network uses WPA2 Enterprise EAP-PEAP-GTC security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_PEAP_GTC,
#endif
#endif
#if CONFIG_NCP_EAP_FAST
#if CONFIG_NCP_EAP_MSCHAPV2
    /** The network uses WPA2 Enterprise EAP-FAST-MSCHAPV2 security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_FAST_MSCHAPV2,
#endif
#if CONFIG_NCP_EAP_GTC
    /** The network uses WPA2 Enterprise EAP-FAST-GTC security
     * The anonymous identity, identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_FAST_GTC,
#endif
#endif
#if CONFIG_NCP_EAP_SIM
    /** The network uses WPA2 Enterprise EAP-SIM security
     * The identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_SIM,
#endif
#if CONFIG_NCP_EAP_AKA
    /** The network uses WPA2 Enterprise EAP-AKA security
     * The identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_AKA,
#endif
#if CONFIG_NCP_EAP_AKA_PRIME
    /** The network uses WPA2 Enterprise EAP-AKA-PRIME security
     * The identity and password fields in
     * \ref wlan_network structure are used */
    WLAN_SECURITY_EAP_AKA_PRIME,
#endif
#endif
#if CONFIG_NCP_WPA_SUPP_DPP
    /** The network uses DPP security with NAK(Net Access Key) */
    WLAN_SECURITY_DPP,
#endif
    /** The network can use any security method. This is often used when
     * the user only knows the name and passphrase but not the security
     * type.  */
    WLAN_SECURITY_WILDCARD,
};

enum
{
    /** static IP address */
    ADDR_TYPE_STATIC = 0,
    /** Dynamic  IP address*/
    ADDR_TYPE_DHCP = 1,
    /** Link level address */
    ADDR_TYPE_LLA = 2,
};

/*NCP BSSID tlv*/

#define NCP_WLAN_DEFAULT_RSSI_THRESHOLD 70
#define PING_INTERVAL                   1000
#define PING_DEFAULT_TIMEOUT_SEC        2
#define PING_DEFAULT_COUNT              10
#define PING_DEFAULT_SIZE               56
#define PING_MAX_SIZE                   65507

#define PING_ID 0xAFAF

#define IP_ADDR_LEN 16

NCP_TLV_PACK_START struct icmp_echo_hdr
{
    uint8_t type;
    uint8_t code;
    uint16_t chksum;
    uint16_t id;
    uint16_t seqno;
} NCP_TLV_PACK_END;

NCP_TLV_PACK_START struct ip_hdr
{
    /* version / header length */
    uint8_t _v_hl;
    /* type of service */
    uint8_t _tos;
    /* total length */
    uint16_t _len;
    /* identification */
    uint16_t _id;
    /* fragment offset field */
    uint16_t _offset;
#define IP_RF      0x8000U /* reserved fragment flag */
#define IP_DF      0x4000U /* don't fragment flag */
#define IP_MF      0x2000U /* more fragments flag */
#define IP_OFFMASK 0x1fffU /* mask for fragmenting bits */
    /* time to live */
    uint8_t _ttl;
    /* protocol*/
    uint8_t _proto;
    /* checksum */
    uint16_t _chksum;
    /* source and destination IP addresses */
    in_addr_t src;
    in_addr_t dest;
} NCP_TLV_PACK_END;

typedef NCP_TLV_PACK_START struct _ping_msg_t
{
    uint16_t size;
    uint32_t count;
    uint32_t timeout;
    uint32_t handle;
    char ip_addr[IP_ADDR_LEN];
    uint32_t port;
} NCP_TLV_PACK_END ping_msg_t;

typedef NCP_TLV_PACK_START struct _ping_res
{
    int seq_no;
    int echo_resp;
    uint32_t time;
    uint32_t recvd;
    int ttl;
    char ip_addr[IP_ADDR_LEN];
    uint16_t size;
} NCP_TLV_PACK_END ping_res_t;

#define NCP_IPERF_TCP_SERVER_PORT_DEFAULT 5001
#define NCP_IPERF_UDP_SERVER_PORT_DEFAULT NCP_IPERF_TCP_SERVER_PORT_DEFAULT + 2
#define NCP_IPERF_UDP_RATE                30 * 1024
#define NCP_IPERF_UDP_TIME                100
#define NCP_IPERF_PKG_COUNT               1000
#define NCP_IPERF_PER_TCP_PKG_SIZE        1448
#define NCP_IPERF_PER_UDP_PKG_SIZE        1472

#define IPERF_TCP_RECV_TIMEOUT 1000
#define IPERF_UDP_RECV_TIMEOUT 1000

enum ncp_iperf_item
{
    NCP_IPERF_TCP_TX,
    NCP_IPERF_TCP_RX,
    NCP_IPERF_UDP_TX,
    NCP_IPERF_UDP_RX,
    FALSE_ITEM,
};

typedef struct _iperf_set_t
{
    uint32_t iperf_type;
    uint32_t iperf_count;
    uint32_t iperf_udp_rate;
    uint32_t iperf_udp_time;
} iperf_set_t;

typedef struct _iperf_msg_t
{
    int16_t status[2];
    uint32_t count;
    uint32_t timeout;
    uint32_t handle;
    uint32_t port;
    uint16_t per_size;
    char ip_addr[IP_ADDR_LEN];
    iperf_set_t iperf_set;
} iperf_msg_t;

/** Station Power save mode */
enum wlan_ps_mode
{
    /** Active mode */
    WLAN_ACTIVE = 0,
    /** IEEE power save mode */
    WLAN_IEEE,
    /** Deep sleep power save mode */
    WLAN_DEEP_SLEEP,
    WLAN_IEEE_DEEP_SLEEP,
    WLAN_WNM,
    WLAN_WNM_DEEP_SLEEP,
};

/** WLAN station/micro-AP/Wi-Fi Direct Connection/Status state */
enum wlan_connection_state
{
    /** The WLAN Connection Manager is not connected and no connection attempt
     *  is in progress.  It is possible to connect to a network or scan. */
    WLAN_DISCONNECTED,
    /** The WLAN Connection Manager is not connected but it is currently
     *  attempting to connect to a network.  It is not possible to scan at this
     *  time.  It is possible to connect to a different network. */
    WLAN_CONNECTING,
    /** The WLAN Connection Manager is not connected but associated. */
    WLAN_ASSOCIATED,
    /** The WLAN Connection Manager is not connected but authenticated. */
    WLAN_AUTHENTICATED,
    /** The WLAN Connection Manager is connected.  It is possible to scan and
     *  connect to another network at this time.  Information about the current
     *  network configuration is available. */
    WLAN_CONNECTED,
    /** The WLAN Connection Manager has started uAP */
    WLAN_UAP_STARTED,
    /** The WLAN Connection Manager has stopped uAP */
    WLAN_UAP_STOPPED,
    /** The WLAN Connection Manager is not connected and network scan
     * is in progress. */
    WLAN_SCANNING,
    /** The WLAN Connection Manager is not connected and network association
     * is in progress. */
    WLAN_ASSOCIATING,
};

MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_wifi();

#define ICMP_ECHO             8 /* echo */
#define IP_HEADER_LEN         20
#define PING_RECVFROM_TIMEOUT 2000

#define PING_EVENTS_START         0x01
#define PING_EVENTS_SENDTO_RESP   0x02
#define PING_EVENTS_RECVFROM_RESP 0x03
void ping_wait_event(osa_event_flags_t flagsToWait);
void ping_set_event(osa_event_flags_t flagsToWait);

#define IPERF_TX_START    0x01
#define IPERF_RX_START    0x02
void iperf_tx_wait_event(osa_event_flags_t flagsToWait);
void iperf_tx_set_event(osa_event_flags_t flagsToWait);
void iperf_rx_wait_event(osa_event_flags_t flagsToWait);
void iperf_rx_set_event(osa_event_flags_t flagsToWait);

#endif /*__NCP_HOST_COMMAND_WIFI_H_*/
