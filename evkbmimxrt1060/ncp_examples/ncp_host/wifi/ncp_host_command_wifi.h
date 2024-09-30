/*!\file ncp_host_command_wifi.h
 *\brief This file provides NCP host command interfaces.
 */
/*
 *  Copyright 2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __NCP_HOST_COMMAND_WIFI_H_
#define __NCP_HOST_COMMAND_WIFI_H_

#include "ncp_adapter.h"
#include "ncp_host_utils_wifi.h"

/** Define macros which are used in ncp_cmd_wifi.h but not defined in mcu project. */
#define MLAN_MAX_VER_STR_LEN         128
#define MLAN_MAC_ADDR_LENGTH         6
#define WLAN_NETWORK_NAME_MAX_LENGTH 32
#define IDENTITY_MAX_LENGTH          64
#define PASSWORD_MAX_LENGTH          128
#define MAX_NUM_CLIENTS              16
#define MAX_MONIT_MAC_FILTER_NUM     3
#define IEEEtypes_ADDRESS_SIZE       6
#define CSI_FILTER_MAX               16

#define NCP_IPERF_END_TOKEN_SIZE 11

/** Structures which are used by ncp_cmd_wifi.h but not defined in mcu project. */
/** This structure is used to set station configuration. */
typedef struct
{
    /** MAC address buffer. */
    uint8_t mac[MLAN_MAC_ADDR_LENGTH];
    /**
     * Power management status
     * 0 = active (not in power save)
     * 1 = in power save status
     */
    uint8_t power_mgmt_status;
    /** Value of RSSI: dBm */
    char rssi;
} wifi_sta_info_t;

/** This structure is used to set the Wi-Fi CSI (channel state information) filter. */
typedef NCP_TLV_PACK_START struct _wifi_csi_filter_t
{
    /** Source address of the packet to receive */
    uint8_t mac_addr[MLAN_MAC_ADDR_LENGTH];
    /** Packet type of the interested CSI */
    uint8_t pkt_type;
    /** Packet subtype of the interested CSI */
    uint8_t subtype;
    /** Other filter flags */
    uint8_t flags;
} NCP_TLV_PACK_END wifi_csi_filter_t;

/** This structure is used to set the Wi-Fi CSI configuration. */
typedef NCP_TLV_PACK_START struct _wifi_csi_config_params_t
{   
   /** 0: STA; 1: UAP */
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
	
/** WLAN network security type */
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

/** NCP BSSID tlv */

#define NCP_WLAN_DEFAULT_RSSI_THRESHOLD 70
#define PING_INTERVAL                   1000
#define PING_DEFAULT_TIMEOUT_SEC        2
#define PING_DEFAULT_COUNT              10
#define PING_DEFAULT_SIZE               56
#define PING_MAX_SIZE                   65507

#define PING_ID 0xAFAF

#define IP_ADDR_LEN 16

/** This structure is used to set the ICMP (internet control message protocol) echo header. */
NCP_TLV_PACK_START struct icmp_echo_hdr
{   
    /** Echo type */ 
    uint8_t type;
    /** Code */
    uint8_t code;
    /** Checksum */
    uint16_t chksum;
    /** Identify */
    uint16_t id;
    /** Sequence number. */
    uint16_t seqno;
} NCP_TLV_PACK_END;

/** This structure is used to configure the IP header. */
NCP_TLV_PACK_START struct ip_hdr
{
    /** version / header length */
    uint8_t _v_hl;
    /** Type of service */
    uint8_t _tos;
    /** Total length */
    uint16_t _len;
    /** Identification */
    uint16_t _id;
    /** fragment offset field */
    uint16_t _offset;
#define IP_RF      0x8000U /* reserved fragment flag */
#define IP_DF      0x4000U /* don't fragment flag */
#define IP_MF      0x2000U /* more fragments flag */
#define IP_OFFMASK 0x1fffU /* mask for fragmenting bits */
    /** Time to live. */
    uint8_t _ttl;
    /** Protocol*/
    uint8_t _proto;
    /** Checksum */
    uint16_t _chksum;
    /** Source IP addresses */
    in_addr_t src;
    /** Destination IP addresses */
    in_addr_t dest;
} NCP_TLV_PACK_END;

/** This structure is used to set the ping configuration. */
typedef NCP_TLV_PACK_START struct _ping_msg_t
{   
    /** Packet size */
    uint16_t size;
    /** Packet count */
    uint32_t count;
    /** Timeout in seconds */
    uint32_t timeout;
    /** Socket handle index number */
    uint32_t handle;
    /** peer ID address */
    char ip_addr[IP_ADDR_LEN];
    /** peer port number */
    uint32_t port;
} NCP_TLV_PACK_END ping_msg_t;

/** This structure is used to set the ping response message. */
typedef NCP_TLV_PACK_START struct _ping_res
{    
    /** Sequence number. */
    int seq_no;
    /** Echo response. */
    int echo_resp;
    /** Time interval. */
    uint32_t time;
    /** Count of received packet. */
    uint32_t recvd;
    /** Time to live */
    int ttl;
    /** IP address of the peer device. */
    char ip_addr[IP_ADDR_LEN];
    /** Packet size. */
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

/** This structure is used to set the iperf configuration. */
typedef struct _iperf_set_t
{   
    /** 
    0: TCP TX,
    1: TCP RX,
    2: UDP TX,
    3: UDP RX,
    */
    uint32_t iperf_type;
    /** iperf package count */
    uint32_t iperf_count;
    /** UDP rate */
    uint32_t iperf_udp_rate;
    /** UDP time */
    uint32_t iperf_udp_time;
} iperf_set_t;

/** This structure is used to configure the iperf message. */
typedef struct _iperf_msg_t
{   
    /** Status. */
    int16_t status[2];
    /** Count. */
    uint32_t count;
    /** Timeout. */
    uint32_t timeout;
    /** Socket handle index number. */
    uint32_t handle;
    /** Port number. */
    uint32_t port;
    /** Size of per UDP package. */
    uint16_t per_size;
    /** IP address. */
    char ip_addr[IP_ADDR_LEN];
    /** Iperf configuration. */
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
    /** IEEE deep sleep power save mode */
    WLAN_IEEE_DEEP_SLEEP,
    /** WNM power save mode */
    WLAN_WNM,
    /** WNM deep sleep power save mode */
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

/** 
* Get NCP host TLV command buffer.
*
* \return pointer to the host TLV command buffer.
*/
MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_wifi();

#define ICMP_ECHO             8 /* echo */
#define IP_HEADER_LEN         20
#define PING_RECVFROM_TIMEOUT 2000

#define PING_EVENTS_START         0x01
#define PING_EVENTS_SENDTO_RESP   0x02
#define PING_EVENTS_RECVFROM_RESP 0x03

/** 
* This API is used for ping task to wait for user input ping command from console.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void ping_wait_event(osa_event_flags_t flagsToWait);

/** 
* This API is used to send ping command response to ping_sock_task.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void ping_set_event(osa_event_flags_t flagsToWait);

#define IPERF_TX_START    0x01
#define IPERF_RX_START    0x02

/** 
* This API is used to wait for transmit event.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void iperf_tx_wait_event(osa_event_flags_t flagsToWait);

/** 
* This API is used to set transmit event flags that to wait.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void iperf_tx_set_event(osa_event_flags_t flagsToWait);

/** 
* This API is used to wait for receive event IPERF_RX_START.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void iperf_rx_wait_event(osa_event_flags_t flagsToWait);

/** 
* This API is used to set receive event flags.
*
* \param[in] flagsToWait Event flags that to wait
*
* \return void
*/
void iperf_rx_set_event(osa_event_flags_t flagsToWait);

/**
 * This API is used to start the UAP provisioning.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-uap-prov-start
 *
 * \return WM_SUCCESS if success.
 *
 */
int wlan_uap_prov_start_command(int argc, char **argv);

/**
 * This API is used to process start the UAP provisioning response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wlan_uap_prov_start_result_response(uint8_t *res);

/**
 * This API is used to reset the UAP provisioning.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-uap-prov-reset
 *
 * \return WM_SUCCESS if success.
 *
 */
int wlan_uap_prov_reset_command(int argc, char **argv);

/**
 * This API is used to process reset the UAP provisioning response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wlan_uap_prov_reset_result_response(uint8_t *res);

/**
 * This API is used to get the RSSI information.
 * 
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-signal
 *
 * \return WM_SUCCESS.
 */
int wlan_get_signal_command(int argc, char **argv);

/**
 * This API is used to process the RSSI information response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_RSSI.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_rssi_response(uint8_t *res);

/** 
 * This API is used to set the mode of TX/RX antenna.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2 or 3.
 * \param[in] argv    Argument vector\n
 *                    argv[0]: wlan-set-antenna-cfg\n
 *                    argv[1]: string of antenna mode (Required)\n
 *                             0: TX/RX antenna 1\n
 *                             1: TX/RX antenna 2\n
 *                             15: TX/RX antenna diversity.\n
 *                    argv[2]: string of evaluate_time (Optional)\n
 *                             if ant mode = 15, SAD (slow antenna diversity) evaluate time interval.\n
 *                             default value is 6s(6000).
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_antenna_cfg_command(int argc, char **argv);

/** 
 * This API is used to get the mode of TX/RX antenna.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-antenna-cfg
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_get_antenna_cfg_command(int argc, char **argv);

/**
 * This API is used to process set/get antenna configuration response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_ANTENNA_CFG.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_antenna_cfg_response(uint8_t *res);

/**
 * This API is used to set maximum number of the stations that can be allowed to connect to the UAP.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-max-clients-count\n
 *                    argv[1]: string value of STA count, maximum supported STA count is 8.
 *
 * \return WM_SUCCESS if successful.
 * \return -WM_FAIL if unsuccessful.
 *
 * \note Set operation in not allowed in \ref WLAN_UAP_STARTED state.
 */
int wlan_set_max_clients_count_command(int argc, char **argv);

/**
 * This API is used to process set the maximum number of stations response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_CLIENT_CNT.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_client_count_response(uint8_t *res);

/** 
 * This API is used to verify the algorithm AES-CCMP-128 encryption and decryption.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-eu-crypto-ccmp-128\n
 *                    argv[1]: string value of decrypt or encypt option.\n
 *                             0: decrypt\n
 *                             1: encrypt
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_eu_crypto_ccmp128_command(int argc, char **argv);

/**
 * This API is used to process the algorithm AES-CCMP-128 encryption and decryption response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_eu_crypto_ccmp128_response(uint8_t *res);

/** 
 * This API is used to verify the algorithm AES-GCMP-128 encryption and decryption.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-eu-crypto-gcmp-128\n
 *                    argv[1]: string value of decrypt or encypt option.\n
 *                             0: decrypt\n
 *                             1: encrypt
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_eu_crypto_gcmp128_command(int argc, char **argv);

/**
 * This API is used to process the algorithm AES-GCMP-128 encryption and decryption response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_eu_crypto_gcmp128_response(uint8_t *res);

/**
 * This API is used to configure the ED (energy detect) MAC mode for station in Wi-Fi firmware.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    If enable CONFIG_NCP_5GHz_SUPPORT:\n
 *                              argc should be 5.\n
 *                    If disable CONFIG_NCP_5GHz_SUPPORT:\n
 *                              argc should be 3.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-ed-mac-mode\n
 *                    argv[1]: string of ed_ctrl_2g\n
 *                             0: disable EU adaptivity for 2.4GHz band.\n
 *                             1: enable EU adaptivity for 2.4GHz band.\n
 *                    argv[2]: string of ed_offset_2g\n
 *                             0: default dnergy detect threshold.\n
 *                             ed_threshold = ed_base - ed_offset_2g\n
 *                             e.g., if ed_base default is -62dBm, ed_offset_2g is 0x8, then ed_threshold is -70dBm.\n
 *             #if CONFIG_NCP_5GHz_SUPPORT\n
 *                    argv[3]: string of ed_ctrl_5g\n
 *                             0: disable EU adaptivity for 5GHz band.\n
 *                             1: enable EU adaptivity for 5GHz band.\n
 *                    argv[4]: string of ed_offset_5g\n
 *                             0: default energy detect threshold.\n
 *                             ed_threshold = ed_base - ed_offset_5g\n
 *                             e.g., if ed_base default is -62dBm, ed_offset_5g is 0x8, then ed_threshold is -70dBm.\n
 *             #endif
 * 
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_ed_mac_mode_set_command(int argc, char **argv);

/**
 * This API is used to get the current ED MAC mode configuration for station.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-ed-mac-mode
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_ed_mac_mode_get_command(int argc, char **argv);

/**
 * This API is used to process the response for the set/get ED (energy detect) MAC mode command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_ED_MAC.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_ed_mac_response(uint8_t *res);

#if CONFIG_NCP_RF_TEST_MODE
/**
 * This API is used to enable the RF test mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-test-mode
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note If you test with RF test mode, don't use wlan-reset 2, it is not supported.
 */
int wlan_set_rf_test_mode_command(int argc, char **argv);

/**
 * This API is used to process Wi-Fi set the RF test mode response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_test_mode_response(uint8_t *res);

/**
 * This API is used to set the RF TX antenna mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-tx-antenna\n
 *                    argv[1]: antenna\n
 *                             1 -- Main\n
 *                             2 -- Aux
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_tx_antenna_command(int argc, char **argv);

/**
 * This API is used to process the Wi-Fi set RF TX antenna response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_tx_antenna_response(uint8_t *res);

/**
 * This API is used to get the RF TX antenna mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-tx-antenna
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set RF TX antenna before get it.
 */
int wlan_get_rf_tx_antenna_command(int argc, char **argv);

/**
 * This API is used to process the get RF TX antenna response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_TX_ANTENNA.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_tx_antenna_response(uint8_t *res);

/**
 * This API is used to set the RF RX antenna mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-rx-antenna\n
 *                    argv[1]: antenna\n
 *                             1 -- Main\n
 *                             2 -- Aux
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_rx_antenna_command(int argc, char **argv);

/**
 * This API is used to process the response of the Wi-Fi set RF RX antenna mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_rx_antenna_response(uint8_t *res);

/**
 * This API is used to get the RF RX antenna mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-rx-antenna
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set RF RX antenna before get it.
 */
int wlan_get_rf_rx_antenna_command(int argc, char **argv);

/**
 * This API is used to process the response of the get RF RX antenna mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_RX_ANTENNA.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_rx_antenna_response(uint8_t *res);

/**
 * This API is used to set the RF band.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-band\n
 *                    argv[1]: band\n
 *                             0 -- 2.4G\n
 *                             1 -- 5G
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_band_command(int argc, char **argv);

/**
 * This API is used to process the response of the configure RF band command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_band_response(uint8_t *res);

/**
 * This API is used to get the RF band.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-band
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set the RF band before get it.
 */
int wlan_get_rf_band_command(int argc, char **argv);

/**
 * This API is used to process the response of the getting RF band command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_BAND.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_band_response(uint8_t *res);

/**
 * This API is used to set the RF bandwidth.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-bandwidth\n
 *                    argv[1]: bandwidth\n
 *                             0 -- 20MHz\n
 *                             1 -- 40MHz\n
 *                             4 -- 80MHz
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_bandwidth_command(int argc, char **argv);

/**
 * This API is used to process the response of the set RF bandwidth command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_bandwidth_response(uint8_t *res);

/**
 * This API is used to get the RF bandwidth.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-bandwidth
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set the RF bandwidth before get it.
 */
int wlan_get_rf_bandwidth_command(int argc, char **argv);

/**
 * This API is used to process the response of the get RF bandwidth command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_BANDWIDTH.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_bandwidth_response(uint8_t *res);

/**
 * This API is used to set the RF channel.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-channel\n
 *                    argv[1]: channel, 2.4G channel numbers or 5G channel numbers
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_channel_command(int argc, char **argv);

/**
 * This API is used to process the response of the set RF channel command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_channel_response(uint8_t *res);

/**
 * This API is used to get the RF channel.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-channel
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set the RF channel before get it.
 */
int wlan_get_rf_channel_command(int argc, char **argv);

/**
 * This API is used to process the response of the get RF channel command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_CHANNEL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_channel_response(uint8_t *res);

/**
 * This API is used to set the RF radio mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-radio-mode\n
 *                    argv[1]: radio_mode\n
 *                             0 -- set the radio in power down mode\n
 *                             3 -- set the radio in 5GHz band, 1X1 mode(path A)\n
 *                             11 -- set the radio in 2.4GHz band, 1X1 mode(path A)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_rf_radio_mode_command(int argc, char **argv);

/**
 * This API is used to process the reponse of setting RF radio mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_radio_mode_response(uint8_t *res);

/**
 * This API is used to get the RF radio mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-rf-radio-mode
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Please set RF radio mode before get it.
 */
int wlan_get_rf_radio_mode_command(int argc, char **argv);

/**
 * This API is used to process the response of getting RF radio mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_RADIO_MODE.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_get_rf_radio_mode_response(uint8_t *res);

/**
 * This API is used to set the RF TX power.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 4.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-tx-power\n
 *                    argv[1]: power\n
 *                             0 to 24 (dBm)\n
 *                    argv[2]: modulation\n
 *                             0 -- CCK\n
 *                             1 -- OFDM\n
 *                             2 -- MCS\n
 *                    argv[3]: path ID\n
 *                             0 -- PathA\n
 *                             1 -- PathB\n
 *                             2 -- PathA+B\n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_ncp_set_rf_tx_power_command(int argc, char **argv);

/**
 * This API is used to process the response for the setting RF TX power command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_tx_power_response(uint8_t *res);

/**
 * This API is used to set the RF TX continuous mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2 or 6.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-tx-cont-mode\n
 *                    argv[1]: enable/disable RF TX cont mode (Required)\n
 *                             0 -- disable RF TX cont mode\n
 *                             1 -- enable RF TX cont mode\n
 *                    argv[2]: continuous wave Mode (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable continuous Wave Mode\n
 *                             1 -- enable continuous Wave Mode\n
 *                    argv[3]: payload Pattern (Optional)\n
 *                             Required when argv[1] is 1\n
 *                             0 to 0xFFFFFFFF (Enter hexadecimal value)\n
 *                    argv[4]: CS mode (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             Applicable only when continuous wave is disabled.\n
 *                             0 -- disable CS mode\n
 *                             1 -- enable CS mode\n
 *                    argv[5]: Active SubChannel (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- low\n
 *                             1 -- upper\n
 *                             3 -- both\n
 *                    argv[6]: TX Data Rate (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             Rate index corresponding to legacy/HT/VHT rates.\n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_ncp_set_rf_tx_cont_mode_command(int argc, char **argv);

/**
 * This API is used to process the response message for the setting RF TX continuous mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_tx_cont_mode_response(uint8_t *res);

/**
 * This API is used to set the RF TX frame.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 4.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-set-rf-tx-frame\n
 *                    argv[1]: enable/disable RF TX frame (Required)\n
 *                             0 -- disable RF TX frame\n
 *                             1 -- enable RF TX frame\n
 *                    argv[2]: TX data rate (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             Rate index corresponding to legacy/HT/VHT rates).\n
 *                    argv[3]: Payload Pattern (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 to 0xFFFFFFFF (Enter hexadecimal value)\n
 *                    argv[4]: Payload Length (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             1 to 0x400 (Enter hexadecimal value)\n
 *                    argv[5]: Adjust burst SIFS3 gap (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[6]: Burst SIFS in us (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 to 255 (us)\n
 *                    argv[7]: Short preamble (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[8]: active subchannel (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- low\n
 *                             1 -- upper\n
 *                             3 -- both\n
 *                    argv[9]: short GI (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[10]: adv coding (Optional).\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[11]: Beamforming (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[12]: GreenField Mode (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[13]: STBC (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[14]: BSSID (Optional)\n
 *                             Required when argv[1] is 1.\n
 *                             xx:xx:xx:xx:xx:xx
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_ncp_set_rf_tx_frame_command(int argc, char **argv);

/**
 * This API is used to process set rf tx frame response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_set_rf_tx_frame_response(uint8_t *res);

/**
 * This API is used to get and reset RF per.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-get-and-reset-rf-per
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_ncp_set_rf_get_and_reset_rf_per_command(int argc, char **argv);

/**
 * This API is used to process the response for the get and reset RF per command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                   Response body refer to \ref NCP_CMD_RF_PER.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_process_set_rf_get_and_reset_rf_per_response(uint8_t *res);
#endif

/** This API is used to reads/writes adapter registers value.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv.
 *                    argc should be 3 or 4.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-reg-access\n
 *                    argv[1]: type (Required)\n
 *                             1: MAC\n
 *                             2: BBP\n
 *                             3: RF\n
 *                             4: CAU\n
 *                    argv[2]: offset (Required)\n
 *                             offset value of register.\n
 *                    argv[3]: value  (Optional)\n
 *                             Set register value.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_register_access_command(int argc, char **argv);

/**
 * This API is used to process the response message for the reads/writes adapter registers value command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_REGISTER_ACCESS.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_register_access_response(uint8_t *res);

#if CONFIG_NCP_MEM_MONITOR_DEBUG
/** This API is used to get the OS memory allocate and free info.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-mem-stat
 *
 * \return WM_SUCCESS if success.
 */
int wlan_memory_state_command(int argc, char **argv);

/**
 * This API is used to process the response message for the get OS memory allocate and free info command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_MEM_STAT.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_memory_state_response(uint8_t *res);
#endif

/**
 * This API is used to scan for Wi-Fi networks.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-scan \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_scan_command(int argc, char **argv);

/**
 * This API is used to process the response message for the scan Wi-Fi network command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_SCAN_NETWORK_INFO.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_scan_response(uint8_t *res);

/**
 * This API is used to connect to a Wi-Fi network (access point).
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-connect \n
 *                    argv[1]: string value of name (Required) \n
 *                             A string representing the name of the network to connect to.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_connect_command(int argc, char **argv);

/**
 * This API is used to process the response for the connect command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                   Response body refer to \ref NCP_CMD_WLAN_CONN.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_con_response(uint8_t *res);

/**
 * This API is used to disconnect from the current Wi-Fi network (access point).
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-disconnect
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_disconnect_command(int argc, char **argv);

/**
 * This API is used to process the response for the disconnect command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_discon_response(uint8_t *res);

/**
 * This API is used to get the Wi-Fi driver and firmware extended version.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-version
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_version_command(int argc, char **argv);

/**
 * This API is used to process the response for the wlan-version command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                   Response body refer to \ref NCP_CMD_FW_VERSION.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_version_response(uint8_t *res);

/**
 * This API is used to retrieve the connection state of the station and UAP.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-stat
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_stat_command(int argc, char **argv);

/**
 * This API is used to process the response of the Wi-Fi connection state command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_CONNECT_STAT.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_stat_response(uint8_t *res);

/**
 * This API is used to reset the Wi-Fi driver.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-reset \n
 *                    argv[1]: action (Required) \n
 *                             0: disable Wi-Fi  \n
 *                             1: enable Wi-Fi   \n
 *                             2: reset Wi-Fi    \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_reset_command(int argc, char **argv);

/**
 * This API is used to process the reponse for the Wi-Fi reset command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_reset_response(uint8_t *res);

/**
 * This API is used to set Wi-Fi MAC Address in Wi-Fi firmware.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-set-mac \n
 *                    argv[1]: string value of MAC (Required) \n
 *                             The MAC address format like "xx:xx:xx:xx:xx:xx".
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_set_mac_address_command(int argc, char **argv);

/**
 * This API is used to process the response for the set Wi-Fi MAC address command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_set_mac_address(uint8_t *res);

/**
 * This API is used to get Wi-Fi MAC Address in Wi-Fi firmware.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-get-mac
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_get_mac_address_command(int argc, char **argv);

/**
 * This API is used to process the response for the get MAC address command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_GET_MAC_ADDRESS.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_get_mac_address(uint8_t *res);

/**
 * This API is used to Get the configured Wi-Fi network information.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-info
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_info_command(int argc, char **argv);

/**
 * This API is used to process the response for the get Wi-Fi network information command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_NETWORK_INFO.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_info(uint8_t *res);
/**
 * This API is used to get the list of discovered service on the local network.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 3.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-mdns-query \n
 *                    argv[1]: string value of service types (Required) \n
 *                             The type of service to be discovered. \n
 *                             The service types can be found at http://www.dns-sd.org/ServiceTypes.html. \n
 *                    argv[2]: string value of protocol (Required) \n
 *                             e.g. TCP or UDP
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_mdns_query_command(int argc, char **argv);

/**
 * This API is used to process the response for the MDNS query command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_mdns_query_response(uint8_t *res);

/**
 * This API is used to process the MDNS query event.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Event body refer to \ref NCP_EVT_MDNS_RESULT.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_mdns_query_result_event(uint8_t *res);

/**
 * This API is used to send an ICMP echo request, receive its response and 
 * print its statistics and result.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should range from 2 to 5.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: ping \n
 *                    argv[1]: value of -s <packet_size> (Optional)     \n
 *                    argv[2]: value of -c <packet_count> (Optional)    \n
 *                    argv[3]: value of -W <timeout in sec> (Optional)  \n
 *                    argv[4]: value of <ipv4 address> (Required)       \n
 *                             The ipv4 address of target device.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int ncp_ping_command(int argc, char **argv);

/**
 * This API is used to add a network profile to the list of known networks.
 *
 * The network's 'name' field is unique and between \ref WLAN_NETWORK_NAME_MIN_LENGTH and
 * \ref WLAN_NETWORK_NAME_MAX_LENGTH characters.
 *
 * \note The network must specify at least an SSID or BSSID.
 *
 * \note This API is used to add profiles for station or UAP interfaces.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should range from 3 to 14.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-add \n
 *                    argv[1]: string value of profile name (Required) \n
 *                             The name of network profile. \n
 *                    argv[2]: string value of ssid (Optional) \n
 *                             The network SSID, represented as a C string of up to 32 characters in length. \n
 *                    argv[3]: string value of ip address (Optional) \n
 *                             The ip address format like "ip:<ip_addr>,<gateway_ip>,<netmask>". \n
 *                             The network IP address configuration specified by struct \n
 *                             NCP_WLAN_IPV4_CONFIG that should be associated with this interface. \n
 *                             If this profile is used in the UAP mode, this field is mandatory. \n
 *                             If this profile is used in the station mode, this field is mandatory \n
 *                             if using static IP, and is optional if using DHCP. \n
 *                    argv[4]: string value of bssid (Optional) \n
 *                             The network BSSID, represented as a 6-byte array. \n
 *                             If this profile is used in the UAP mode, this field is ignored. \n
 *                             If this profile is used in the station mode, this field is used to \n
 *                             identify the network. Set all 6 bytes to 0 to use any BSSID, in which \n
 *                             case only the SSID is used to find the network. \n
 *                    argv[5]: string value of role (Required) \n
 *                             The network Wi-Fi mode enum wlan_bss_role. \n
 *                             Set this to specify what type of Wi-Fi network mode to use. \n
 *                             This can either be \ref WLAN_BSS_ROLE_STA for use in the station mode, \n
 *                             or it can be \ref WLAN_BSS_ROLE_UAP for use in the UAP mode. \n
 *                    argv[6]: string value of security (Optional) \n
 *                             The network security configuration specified for the network. \n
 *                    argv[7]: channel (Optional) \n
 *                             The channel for this network. \n
 *                             If this profile is used in UAP mode, this field specifies the channel to \n
 *                             start the UAP interface on. Set this to 0 for auto channel selection. \n
 *                             If this profile is used in the station mode, this constrains the channel on \n
 *                             which the network to connect should be present. Set this to 0 to allow the \n
 *                             network to be found on any channel. \n
 *                    argv[8]: capa (Optional) \n
 *                             Wi-Fi capabilities of UAP network 802.11n, 802.11ac or/and 802.11ax. \n
 *                    argv[9]: mfpc (Optional) \n
 *                             Management frame protection capable (MFPC) \n
 *                    argv[10]: mfpr (Optional) \n
 *                              Management frame protection required (MFPR) \n
 *                    argv[11]: dtim (Optional) \n
 *                              DTIM period of associated BSS \n
 *                    argv[12]: aid (Optional) \n
 *                              Client anonymous identity \n
 *                    argv[13]: string value of key_passwd (Optional) \n
 *                              Client Key password \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_add_command(int argc, char **argv);

/**
 * This API is used to process the response for the add network command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_add_response(uint8_t *res);

/**
 * This API is used to set the mode of UAPSD (unscheduled automatic power save delivery).
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-uapsd-enable \n
 *                    argv[1]: string of UAPSD mode (Required) \n
 *                             0  -- disable UAPSD \n
 *                             1  -- enable UAPSD. \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_wmm_uapsd_command(int argc, char **argv);

/**
 * This API is used to process the response for the set UAPSD command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                    Response body refer to \ref NCP_CMD_POWERMGMT_UAPSD.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wmm_uapsd_response(uint8_t *res);

/** 
 * This API is used to set/get the QoS (Quality of service) information of UAPSD.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1 or 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-uapsd-qosinfo \n
 *                    argv[1]: string of UAPSD QoS Info (Optional) \n
 *                             if is NULL, get the QoS information of UAPSD, \n
 *                             else, set the QoS information of UAPSD. \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_wmm_uapsd_qosinfo_command(int argc, char **argv);

/**
 * This API is used to process the response for the set/get UAPSD QoS information command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                    Response body refer to \ref NCP_CMD_POWERMGMT_QOSINFO.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_uapsd_qosinfo_response(uint8_t *res);

/**
 * This API is used to set/get the sleep period of UAPSD.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1 or 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-uapsd-sleep-period \n
 *                    argv[1]: string of UAPSD sleep period (Optional) \n
 *                             if is NULL, get the sleep period of UAPSD, \n
 *                             else, set the sleep period of UAPSD. \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_uapsd_sleep_period_command(int argc, char **argv);

/**
 * This API is used to process the response for the set/get UAPSD sleep period command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                    Response body refer to \ref NCP_CMD_POWERMGMT_SLEEP_PERIOD.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_uapsd_sleep_period_response(uint8_t *res);

/**
 * This API is used to start WPS (Wi-Fi protected setup) PBC (push-button configuration) 
 * to quickly connect to the AP.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-start-wps-pbc \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_start_wps_pbc_command(int argc, char **argv);

/**
 * This API is used to process WPS PBC response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wps_pbc_response(uint8_t *res);

/**
 * This API is used to generate WPS PIN (personal identification number).
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-generate-wps-pin \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_wps_generate_pin_command(int argc, char **argv);

/**
 * This API is used to process the response for the generate WPS PIN command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                    Response body refer to \ref NCP_CMD_WPS_GEN_PIN.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wps_generate_pin_response(uint8_t *res);

/**
 * This API is used to start WPS PIN.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-start-wps-pin \n
 *                    argv[1]: string of WPS PIN (Required) \n
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_start_wps_pin_command(int argc, char **argv);

/**
 * This API is used to process the response for the start WPS PIN command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_wps_pin_response(uint8_t *res);

/**
 * This API is used to modify and set Wi-Fi 802.11ax config.
 *
 * \note Implemented as global variable arrays with default config.
 *       This API can get and change one parameter and
 *       it will be restored until reboot.
 *       Then this config data can be sent to Wi-Fi.
 *       Refer to \ref NCP_CMD_11AX_CFG_INFO for config parameter usage and length.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be at least 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-set-11axcfg \n
 *                    argv[1]: action in string (Required)
 *                             help: show helper
 *                             dump: dump parameter data currently restored
 *                             set: set one parameter
 *                             done: configure current data to Wi-Fi \n
 *                    argv[2]: parameter name (Required when action is "set") \n
 *                    argv[3]: parameter value in hexadecimal,
 *                             have more argvs when value is more than one byte,
 *                             like a byte array or uint16_t, uint32_t,
 *                             uint16_t and uint32_t is ordered in little-endian
 *                             (Required when action is "set") \n
 *                    ... \n
 *                    argv[x]: parameter value in hexadecimal,
 *                             uint16_t and uint32_t is ordered in little-endian
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_set_11axcfg_command(int argc, char **argv);

/**
 * This API is used to process Wi-Fi 802.11ax config command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body is MCU_NCPCmd_DS_COMMAND with result.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_11axcfg_response(uint8_t *res);

/**
 * This API is used to modify and set Wi-Fi BTWT (broadcast target awake time) config.
 *
 * \note Implemented as global variable arrays with default config.
 *       This API can get and change one parameter and
 *       it will be restored until reboot.
 *       Then this config data can be sent to Wi-Fi.
 *       Refer to \ref NCP_CMD_BTWT_CFG_INFO for config parameter usage and length.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be at least 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-set-btwt-cfg \n
 *                    argv[1]: action in string (Required)
 *                             help: show helper
 *                             dump: dump parameter data currently restored
 *                             set: set one parameter
 *                             done: configure current data to Wi-Fi \n
 *                    argv[2]: parameter name (Required when action is "set") \n
 *                    argv[3]: parameter value in hexadecimal,
 *                             have more argvs when value is more than one byte,
 *                             like a byte array or uint16_t, uint32_t,
 *                             uint16_t and uint32_t is ordered in little-endian
 *                             (Required when action is "set") \n
 *                    ... \n
 *                    argv[x]: parameter value in hexadecimal,
 *                             uint16_t and uint32_t is ordered in little-endian
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_set_btwt_command(int argc, char **argv);

/**
 * This API is used to process BTWT command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body is MCU_NCPCmd_DS_COMMAND with result.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_btwt_response(uint8_t *res);

/**
 * This API is used to modify and set TWT (target awake time) config.
 *
 * \note Implemented as global variable arrays with default config.
 *       This API can get and change one parameter and
 *       it will be restored until reboot.
 *       Then this config data can be sent to Wi-Fi.
 *       Refer to \ref NCP_CMD_TWT_SETUP_CFG for config parameter usage and length.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be at least 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-twt-setup \n
 *                    argv[1]: action in string (Required)
 *                             help: show helper
 *                             dump: dump parameter data currently restored
 *                             set: set one parameter
 *                             done: configure current data to Wi-Fi \n
 *                    argv[2]: parameter name (Required when action is "set") \n
 *                    argv[3]: parameter value in hexadecimal,
 *                             have more argvs when value is more than one byte,
 *                             like a byte array or uint16_t, uint32_t,
 *                             uint16_t and uint32_t is ordered in little-endian
 *                             (Required when action is "set") \n
 *                    ... \n
 *                    argv[x]: parameter value in hexadecimal,
 *                             uint16_t and uint32_t is ordered in little-endian
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_twt_setup_command(int argc, char **argv);

/**
 * This API is used to process the response for the TWT setup command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body is MCU_NCPCmd_DS_COMMAND with result.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_twt_setup_response(uint8_t *res);

/**
 * This API is used to modify and set TWT teardown config.
 *
 * \note Implemented as global variable arrays with default config.
 *       This API can get and change one parameter and
 *       it will be restored until reboot.
 *       Then this config data can be sent to Wi-Fi.
 *       Refer to \ref NCP_CMD_TWT_TEARDOWN_CFG for config parameter usage and length.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be at least 2.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-twt-teardown \n
 *                    argv[1]: action in string (Required)
 *                             help: show helper
 *                             dump: dump parameter data currently restored
 *                             set: set one parameter
 *                             done: configure current data to Wi-Fi \n
 *                    argv[2]: parameter name (Required when action is "set") \n
 *                    argv[3]: parameter value in hexadecimal,
 *                             have more argvs when value is more than one byte,
 *                             like a byte array or uint16_t, uint32_t,
 *                             uint16_t and uint32_t is ordered in little-endian
 *                             (Required when action is "set") \n
 *                    ... \n
 *                    argv[x]: parameter value in hexadecimal,
 *                             uint16_t and uint32_t is ordered in little-endian
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_twt_teardown_command(int argc, char **argv);

/**
 * This API is used to process the response for the set TWT teardown command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body is MCU_NCPCmd_DS_COMMAND with result.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_twt_teardown_response(uint8_t *res);

/**
 * This API is used to get TWT report information.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-get-twt-report
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_get_twt_report_command(int argc, char **argv);

/**
 * This API is used to process the TWT report command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body refer to \ref NCP_CMD_TWT_REPORT.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_twt_report_response(uint8_t *res);

/**
 * This API is used to enable or disable Wi-Fi 802.11d feature.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 3.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-set-11d-enable \n
 *                    argv[1]: mode in string, sta: staion, uap: soft AP \n
 *                    argv[2]: state, 0: disable, 1: enable
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_set_11d_enable_command(int argc, char **argv);

/**
 * This API is used to process the set 802.11d feature state command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body is MCU_NCPCmd_DS_COMMAND with result.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_11d_enable_response(uint8_t *res);

/**
 * This API is used to set or get the Wi-Fi region code.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 2 or 3.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-region-code \n
 *                    argv[1]: action in string, get/set (Required) \n
 *                    argv[2]: region code value to set (Required when action is "set")
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_region_code_command(int argc, char **argv);

/**
 * This API is used to process the response for the get/set region code command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body refer to \ref NCP_CMD_REGION_CODE_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_region_code_response(uint8_t *res);

/**
 * This API is used to get the device temperature.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-get-temp
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_get_temperature_command(int argc, char **argv);

/**
 * This API is used to process the response for the get temperature command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body refer to \ref NCP_CMD_TEMPERATURE.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_get_temperature_response(uint8_t *res);

/**
 * This API is used to set device time, the time includes year,
 * month, day, hour, minute and second.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 7.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-set-time \n
 *                    argv[1]: year \n
 *                    argv[2]: month \n
 *                    argv[3]: day \n
 *                    argv[4]: hour \n
 *                    argv[5]: minute \n
 *                    argv[6]: second
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_set_time_command(int argc, char **argv);

/**
 * This API is used to get device time in date.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-get-time
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_get_time_command(int argc, char **argv);

/**
 * This API is used to process the response for the get time command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                    Response body refer to \ref NCP_CMD_DATE_TIME_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_time_response(uint8_t *res);

/** This API is used to set soft roaming configuration.
 *
 * Call this API to enable/disable soft roaming
 * by specifying the RSSI threshold.
 *
 * \note <b>RSSI Threshold setting for soft roaming</b>:
 * The provided RSSI low threshold value is used to subscribe
 * RSSI low event from firmware. On reception of this event,
 * background scan is started in firmware with same RSSI
 * threshold, to find out APs with better signal strength than
 * RSSI threshold.
 *
 * \note If AP is found then roam attempt is initiated, otherwise
 * the background scan is started again until limit reaches to \ref BG_SCAN_LIMIT.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should range from 2 to 3.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-roaming\n
 *                    argv[1]: string value of enable/disable roaming (Required)\n
 *                             0 -- disable\n
 *                             1 -- enable\n
 *                    argv[2]: string value of RSSI low threshold (Optional)\n
 *                             Default value is 70
 *
 * \return WM_SUCCESS if the call was successful.
 * \return -WM_FAIL if failed.
 */
int wlan_roaming_command(int argc, char **argv);

/**
 * This API is used to process the response for the roaming command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_roaming_response(uint8_t *res);

/**
 * This API is used to set/delete the MEF (memory efficient filtering) entries configuration.
 *
 * \note Use this API with command wlan-mcu-sleep with wakeup method MEF.
 * Make sure to have connection on STA interface or start uAP.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should range from 2 to 3.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-multi-mef\n
 *                    argv[1]: Packet type string (Required)\n
 *                             ping      -– Add MEF entry for ping packets\n
 *                             arp       -– Add MEF entry for ARP packets\n
 *                             multicast -– Add MEF entry for multicast packets\n
 *                             ns        -– Add MEF entry for IPV6 NS packets\n
 *                             del       -– Delete all MEF entries\n
 *                    argv[2]: Sting value of action (Required)\n
 *                             0 –- Discard and not wakeup host\n
 *                             1 –- Discard and wakeup host\n
 *                             3 –- Allow and wakeup host
 *
 * \note argv[2] is not needed if argv[1] is del.
 *
 * \return WM_SUCCESS if the call was successful.
 * \return -WM_FAIL if failed.
 */
int wlan_multi_mef_command(int argc, char **argv);

/**
 * This API is used to process multi MEF response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_multi_mef_response(uint8_t *res);

/**
 * This API is used to configure Wi-Fi wakeup conditions.
 * Once any condition is meet, NCP device will be woken up by Wi-Fi.
 * Some of the wakeup conditions needs connection on station or uAP.
 *
 * \note Use this API with \ref wlan_multi_mef_command
 * if MEF mode is used.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should range from 2 to 3.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-wakeup-condition\n
 *                    argv[1]: String of wakeup mode (Required)\n
 *                             wowlan –- Default wakeup conditions\n
 *                             mef    –- MEF conditions\n
 *                    argv[2]: String of wakeup bitmap for mode wowlan\n
 *                             bit[0] -- WAKE_ON_ALL_BROADCAST\n
 *                             bit[1] -- WAKE_ON_UNICAST\n
 *                             bit[2] -- WAKE_ON_MAC_EVENT\n
 *                             bit[3] -- WAKE_ON_MULTICAST\n
 *                             bit[4] -- WAKE_ON_ARP_BROADCAST\n
 *                             bit[6] -- WAKE_ON_MGMT_FRAME
 *
 * \note The argv[2] is only required when argv[1] is wowlan.
 *
 * \note If argv[1] is wowlan and all bits of argv[2] are 0, NCP device won't be woken up by Wi-Fi.
 *
 * \return WM_SUCCESS if the call was successful.
 * \return -WM_FAIL if failed.
 */
int wlan_wakeup_condition_command(int argc, char **argv);

/**
 * This API is used to process the response for the configure Wi-Fi wakeup condition command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_wakeup_condition_response(uint8_t *res);

/**
 * This API is used to request NCP device enter specific low power mode.
 *
 * \note Should be used after API \ref ncp_mcu_sleep_command with manual mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-suspend\n
 *                    argv[1]: String value of low power mode\n
 *                             1 -- PM1\n
 *                             2 -- PM2\n
 *                             3 -- PM3\n
 *
 * \note The allowed low power mode is controlled by configuration of \ref ncp_wake_cfg_command.\n
 * For INTF wake mode, the allowed <power_mode> are PM1 and PM2.\n
 * For GPIO wake mode and RDRW612 as NCP device, the allowed <power_mode> are PM1, PM2 and PM3.\n
 * For GPIO wake mode and FRDMRW612 as NCP device, the allowed <power_mode> are PM1 and PM2.\n
 * For WIFI-NB wake mode and FRDRW612 as NCP device, the allowed <power_mode> are PM1, PM2 and PM3.
 *
 * \return WM_SUCCESS if the call was successful.
 * \return -WM_FAIL if failed.
 */
int wlan_suspend_command(int argc, char **argv);

/**
 * This API is used to process suspend response.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_suspend_response(uint8_t *res);

/**
 * This API is used to enable/disable deep sleep power save mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-deep-sleep-ps\n
 *                    argv[1]: enable/disable deep sleep power save mode.\n
 *                             0 -- disable deep sleep\n
 *                             1 -- enable deep sleep
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Deep sleep power save is enabled by default.
 */
int wlan_deep_sleep_ps_command(int argc, char **argv);

/**
 * This API is used to process the response for the deep sleep power save mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_deep_sleep_ps_response(uint8_t *res);

/**
 * This API is used to enable/disable ieee power save mode.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: wlan-ieee-ps\n
 *                    argv[1]: enable/disable ieee power save mode.\n
 *                             0 -- disable ieee power save mode\n
 *                             1 -- enable ieee power save mode
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 * \note Ieee power save is enabled by default.
 */
int wlan_ieee_ps_command(int argc, char **argv);

/**
 * This API is used to process the response for the enable/disable ieee power save mode command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS
 */
int wlan_process_ieee_ps_response(uint8_t *res);

/**
 * This API is used to control Wi-Fi system enable, disable and reset.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 2.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: should be "wlan-reset"\n
 *                    argv[1]: string value of wlan reset type (Required)\n
 *                             0 -- disable WiFi\n
 *                             1 -- enable WiFi\n
 *                             2 -- reset WiFi
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_reset_command(int argc, char **argv);

/**
 * This API is used to create a socket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 2 to 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-open\n
 *                    argv[1]: string value of socket_type (Required)\n
 *                             tcp -- tcp socket\n
 *                             udp -- udp socket\n
 *                             raw -- raw socket\n
 *                    argv[2]: string value of socket domain (Optional)\n
 *                             ipv4 -- default is ipv4 domain\n
 *                             ipv6 -- ipv6 domain\n
 *                    argv[3]: string value of socket procotol (Optional)\n
 *                             icmp   -- default is icmp protocol\n
 *                             icmpv6 -- icmpv6 protocol
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_open_command(int argc, char **argv);

/**
 * This API is used to process the socket open command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body refer to \ref NCP_CMD_SOCKET_OPEN_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_open_response(uint8_t *res);

/**
 * This API is used to connect peer socket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 2 to 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-connect\n
 *                    argv[1]: string value of peer socket fd to connect (Required)\n
 *                    argv[2]: string value of peer socket ipv4 address to connect, so far only support IPV4 address (Required)\n
 *                    argv[3]: string value of peer socket port (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_con_command(int argc, char **argv);

/**
 * This API is used to process the socket connect command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_con_response(uint8_t *res);

/**
 * This API is used to bind socket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 2 to 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-bind\n
 *                    argv[1]: string value of socket fd to bind (Required)\n
 *                    argv[2]: string value of socket ipv4 address to connect, so far only support IPV4 address (Required)\n
 *                    argv[3]: string value of socket port (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_bind_command(int argc, char **argv);

/**
 * This API is used to process socket bind command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_bind_response(uint8_t *res);

/**
 * This API is used to close the opened socket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 2.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-close\n
 *                    argv[1]: string value of opened socket fd to close (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_close_command(int argc, char **argv);

/**
 * This API is used to process the response for the socket close command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_close_response(uint8_t *res);

/**
 * This API is used to listen TCP client's connection.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 3.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-listen\n
 *                    argv[1]: string value of opened socket fd to listen (Required)\n
 *                    argv[2]: string value of listen maximum number (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_listen_command(int argc, char **argv);

/**
 * This API is used to process the response for the socket listen command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_listen_response(uint8_t *res);

/**
 * This API is used to accpet the TCP sever's connection.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 2,
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-accept\n
 *                    argv[1]: string value of tcp server listen socket fd (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_accept_command(int argc, char **argv);

/**
 * This API is used to process the response for the socket accept command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_SOCKET_ACCEPT_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_accept_response(uint8_t *res);

/**
 * This API is used to send data.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 3 to 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-send\n
 *                    argv[1]: string value of socket fd, use the socket to send data (Required)\n
 *                    argv[2]: string value of send data , the max send data buffer is 4076. (Required)\n
 *                    argv[3]: string value of send data size, we can specify send data size (Optional)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_send_command(int argc, char **argv);

/**
 * This API is used to process socket send command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_send_response(uint8_t *res);

/**
 * This API is used to send data to specified ip and port.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 6 to 7.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-sendto\n
 *                    argv[1]: int value of socket fd, use the socket to send data (Required)\n
 *                    argv[2]: string value of peer ip addr, the max length is 16 (Required)\n
 *                    argv[3]: string value of peer socket port (Required)\n
 *                    argv[4]: string value of send data, the max length is 4056 (Required)\n
 *                    argv[5]: string value of send data size, we can specify send data size (Optional)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_sendto_command(int argc, char **argv);

/**
 * This API is used to process the socket send command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response,\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_sendto_response(uint8_t *res);

/**
 * This API is used to receive data from socket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-receive\n
 *                    argv[1]: string value of socket fd, receive data from the socket fd (Required)\n
 *                    argv[2]: string value of receive size, the buffer max length is 4072 (Required)\n
 *                    argv[3]: string value of wait time, return fail when timemout (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_receive_command(int argc, char **argv);

/**
 * This API is used to process the socket receive command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_SOCKET_RECEIVE_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_receive_response(uint8_t *res);

/**
 * This API is used to receive data from socket and return peer socket ip and port.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-socket-recvfrom\n
 *                    argv[1]: string value of socket fd, receive data from the socket fd (Required)\n
 *                    argv[2]: string value of receive size, the buffer max length is 4072 (Required)\n
 *                    argv[3]: string value of wait time, return fail when timemout (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_socket_recvfrom_command(int argc, char **argv);

/**
 * This API is used to process the socket recvfrom command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_SOCKET_RECVFROM_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_socket_recvfrom_response(uint8_t *res);

/**
 * This API is used to connect the HTTP server.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 3.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-connect\n
 *                    argv[1]: string value of http handle to connect, (Required)\n
 *                    argv[2]: string value of http server host address, the string max length is 512 (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_http_connect_command(int argc, char **argv);

/**
 * This API is used to process the HTTP connect command response.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_HTTP_CON_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_con_response(uint8_t *res);

/**
 * This API is used to disconnect the HTTP server.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 2.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-disconnect\n
 *                    argv[1]: string value of http handle to disconnect, (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_http_disconnect_command(int argc, char **argv);

/**
 * This API is used to process the response for the HTTP disconnect command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_discon_response(uint8_t *res);

/**
 * This API is used to send request to the HTTP server.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 4 to 6.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-req\n
 *                    argv[1]: int value of http handle to send http request (Required)\n
 *                    argv[2]: string value of http method, the max length is 16 (Required)\n
 *                             get     -- http request get method\n
 *                             delete  -- http request delete method\n
 *                             put     -- http request put method\n
 *                             options -- http request options method\n
 *                    argv[3]: string value of http URL, the string max length is 512 (Required)\n
 *                    argv[4]: string value of http send data, the buffer max length is 3560 (Optional)\n
 *                    argv[5]: string value of http send data size (Optional)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_http_req_command(int argc, char **argv);

/**
 * This API is used to process the response for the HTTP request.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_HTTP_REQ_RESP_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_req_response(uint8_t *res);

/**
 * This API is used to receive the HTTP server data.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-receive\n
 *                    argv[1]: string value of http handle to receive http data (Required)\n
 *                    argv[2]: string value of http received data, the buffer max length is 4072. (Required)\n
 *                    argv[3]: string value of http wait time, return fail when timemout (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure
 *
 */
int wlan_http_recv_command(int argc, char **argv);

/**
 * This API is used to process the response for the HTTP receive command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_HTTP_RECV_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_recv_response(uint8_t *res);

/**
 * This API is used to unset the HTTP request header.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-unseth\n
 *                    argv[1]: string http header segment name, the max length is 64 (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_http_unseth_command(int argc, char **argv);

/**
 * This API is used to process the response for the HTTP unset header command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_unseth_response(uint8_t *res);

/**
 * This API is used to set the HTTP request header.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-http-seth\n
 *                    argv[1]: string value of segment name of the header, the max length is 64 (Required)\n
 *                    argv[2]: string value of segment value of the header, the max length is 128 (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_http_seth_command(int argc, char **argv);

/**
 * This API is used to process the response for the set HTTP header command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_http_seth_response(uint8_t *res);

/**
 * This API is used to upgrade the HTTP to websocket.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-websocket-upg\n
 *                    argv[1]: string value of http handle (Required)\n
 *                    argv[2]: string value of websocket URI (Required)\n
 *                             /ws/ -- websocket uri is /ws/\n
 *                    argv[3]: string value of websocket type (Required)\n
 *                             echo -- websocket type is echo
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_websocket_upg_command(int argc, char **argv);

/**
 * This API is used to process the response for the websocket upgrade command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success otherwise return -WM_FAIL.
 */
int wlan_process_wlan_websocket_upg_response(uint8_t *res);

/**
 * This API is used to send data to the websocket server.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 4 to 5.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-websocket-send\n
 *                    argv[1]: string value of http handle upgraded to websocket (Required)\n
 *                    argv[2]: string value of websocket type (Required)\n
 *                             text -- websocket text type data\n
 *                             bin  -- websocket bin type data\n
 *                    argv[3]: string value of send data, the buffer max length is 4060. (Required)\n
 *                    argv[4]: string value of send data size (Optional)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_websocket_send_command(int argc, char **argv);

/**
 * This API is used to process the response for the websocket send data command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is NULL.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_websocket_send_response(uint8_t *res);

/**
 * This API is used to receive data from the websocket server.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should be 4.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-websocket-recv\n
 *                    argv[1]: string value of http handle to receive websocket data (Required)\n
 *                    argv[2]: string value of websocket received data, the buffer max length is 4068. (Required)\n
 *                    argv[3]: string value of websocket wait time, return fail when timemout (Required)
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_websocket_recv_command(int argc, char **argv);

/**
 * This API is used to process the response for the websocket receive data command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body is \ref NCP_CMD_WEBSOCKET_RECV_CFG.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_wlan_websocket_recv_response(uint8_t *res);

/**
 * This API is used to the Wi-Fi receive commission infomation from Bluetooth LE.
 *
 * \param[in] ncp_commission_cfg     A pointer to \ref ncp_commission_cfg_t from Bluetooth LE.\n
 *
 */
void ncp_network_commissioning(ncp_commission_cfg_t *ncp_commission_cfg);

/**
 * This API is used to send the neighbor report request.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1 or 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-11k-neigbor-req".\n
 *                    argv[1]: string value of the SSID for neighbor report (Required).
 *
 * \note ssid parameter is optional
 *
 * \return WM_SUCCESS if successful.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_11k_neighbor_req_command(int argc, char **argv);

/**
 * This API is used to process the response of the send neighbor report request.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 *
 */
int wlan_process_neighbor_req_response(uint8_t *res);

/**
 * This API is used to enable or disable the 802.11k feature in Wi-Fi firmware.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-11k-enable".\n
 *                    argv[1]: string value of enable or disable 802.11k feature.\n
 *                             0: disable 802.11k feature.\n
 *                             1: enable 802.11k feature.
 *
 * \note 802.11k is disabled by default.
 *
 * \return WM_SUCCESS if successful.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_11k_cfg_command(int argc, char **argv);

/**
 * This API is used to process the command response of enable or disable 802.11k feature command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 */
int wlan_process_11k_cfg_response(uint8_t *res);

/** 
 * This API is used to set the CSI configuration.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-csi-cfg".
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_csi_cfg_command(int argc, char **argv);

/**
 * This API is used to process the response for the set CSI configuration command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 *
 */
int wlan_process_csi_response(uint8_t *res);

/** 
 * This API is used to set CSI header part of parameters.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 10.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-set-csi-param-header".\n
 *                    argv[1]: string value of bss type\n
 *                             "sta": bss type is station\n
 *                             "uap": bss type is micro ap.\n
 *                    argv[2]: string value of enable or disable csi.\n
 *                             1: enable csi\n
 *                             0: disable csi\n
 *                    argv[3]: string value of head id\n
 *                             head id is symbol placed at start of csi data\n
 *                             used to seperate CSI event records received from FW, could be set as any value.\n
 *                    argv[4]: string value of tail id\n
 *                             tail id is symbol placed at end of csi data\n
 *                             used to seperate CSI event records received from FW, could be set as any value.\n
 *                    argv[5]: string value of chip id\n
 *                             used to seperate CSI event records received from FW, could be set as any value.\n
 *                    argv[6]: band configure\n
 *                             properties of csi channel\n
 *                             bit[0:1]: Band Info\n
 *                             00: 2.4GHz\n
 *                             01: 5GHz\n
 *                             bit[2:3]: chan Width\n                    
 *                             00: 20MHz\n
 *                             10: 40MHz\n
 *                             11: 80MHz\n
 *                             bit[4:5]: chan to Offset\n                     
 *                             00: None\n
 *                             10: Above\n
 *                             11: Below\n
 *                             bit[6:7]: scan Mode  \n                   
 *                             00: manual\n
 *                             10: ACS.\n
 *                    argv[7]: string value of channel number.\n
 *                    argv[8]: string value of enable or disable csi monitor feature\n
 *                             1: enable csi monitor\n
 *                             0: disable csi monitor.\n
 *                    argv[9]: string value of enable or disable csi ra for us feature\n
 *                             1: enable csi ra for us\n
 *                             0: disable csi ra for us.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_csi_param_header_command(int argc, char **argv);

/** 
 * This API is used to set the filter for CSI.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 2 or 6.\n
 *                    2: action for filter is delet, clear and dump\n
 *                    6: action for filter is add.\n
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-set-csi-filter".\n
 *                    argv[1]: string value of action for filter\n
 *                             "add": add one filter to csi filter\n
 *                             "delet": delete last filter entry of csi filter\n
 *                             "clear": clear all entries of csi filter\n
 *                             "dump": dump all entries of csi filter.\n
 *                    argv[2]: string value of mac address, format like 11:22:33:44:55:66.\n
 *                    argv[3]: string value of 802.11 frame control filed type.\n
 *                    argv[4]: string value of 802.11 frame control filed subtype.\n
 *                    argv[5]: string value of flag\n
 *                             bit[0]: reserved, must be set to 0\n
 *                             bit[1]: wait for trigger, not implement currently, must be set to 0\n
 *                             bit[2]: send csi error event when timeout, not implement currently, must be set to 0.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_csi_filter_command(int argc, char **argv);

/** 
 * This API is used to set the monitor configuration.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1.\n
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-net-monitor-cfg"
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_net_monitor_cfg_command(int argc, char **argv);

/**
 * This API is used to process the response of the set monitor configuration command.
 *
 * \param[in] res     A pointer to \ref MCU_NCPCmd_DS_COMMAND response.\n
 *                    Response body: None.
 *
 * \return WM_SUCCESS if success.
 *
 */
int wlan_process_monitor_response(uint8_t *res);

/** 
 * This API is used to set the filter for monitor.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 2 or 3.\n
 *                    2: action for filter is delet, clear and dump\n
 *                    3: action for filter is add.\n
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-set-monitor-filter".\n
 *                    argv[1]: string value of action for filter\n
 *                             "add": add one filter to monitor filter\n
 *                             "delet": delete last filter entry of monitor filter\n
 *                             "clear": clear all entries of monitor filter\n
 *                             "dump": dump all entries of monitor filter.\n
 *                    argv[2]: string value of mac address, format like 11:22:33:44:55:66
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_monitor_filter_command(int argc, char **argv);

/** 
 * This API is used to set the monitor parameters.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 6.\n
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-set-monitor-param".\n
 *                    argv[1]: string value of action\n
 *                             1: action set.\n
 *                    argv[2]: string value of enable or disable monitor\n
 *                             1: enable monitor\n
 *                             other values: disable monitor.\n
 *                    argv[3]: string value of filter flags\n
 *                             bit[0]: enable or disable management frame\n
 *                             1: enable management frame\n
 *                             0: disable management frame\n
 *                             bit[1]: enable or disable control frame\n
 *                             1: enable control frame\n
 *                             0: disable control frame\n
 *                             bit[2]: enable or disable data frame\n
 *                             1: enable data frame\n
 *                             0: disable data frame.\n
 *                    argv[4]: string value of radio type\n
 *                             properties of monitor channel\n
 *                             bit[0:1]: Band Info\n
 *                             00: 2.4GHz\n
 *                             01: 5GHz\n
 *                             bit[2:3]: chan Width   \n                  
 *                             00: 20MHz\n
 *                             10: 40MHz\n
 *                             11: 80MHz\n
 *                             bit[4:5]: chan to Offset\n                  
 *                             00: None\n
 *                             10: Above\n
 *                             11: Below\n
 *                             bit[6:7]: scan Mode\n             
 *                             00: manual\n
 *                             10: ACS.\n
 *                    argv[5]: string value of channel number
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_set_monitor_param_command(int argc, char **argv);

/**
 * This API is used to get IP address of the station.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv, \n
 *                    argc should be 1.
 * \param[in] argv    Argument vector, \n
 *                    argv[0]: wlan-address
 *
 * \return WM_SUCCESS if success.
 */
int wlan_address_command(int argc, char **argv);

/**
 * This API is used to process the response for the get STA IP address command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_NETWORK_ADDRESS.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_address(uint8_t *res);

/**
 * This API is used to start a UAP.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-start-network".\n
 *                    argv[1]: string value of network name.
 *
 * \return WM_SUCCESS if successful.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_start_network_command(int argc, char **argv);

/**
 * This API is used to process the response for the start UAP network command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response. \n
 *                   Response body refer to \ref NCP_CMD_NETWORK_START.
 *
 * \return WM_SUCCESS if UAP starts successfully.
 * \return -WM_FAIL if failure.
 */
int wlan_process_start_network_response(uint8_t *res);

/**
 * This API is used to stop a UAP network.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-stop-netwok".
 *
 * \return WM_SUCCESS.
 *
 */
int wlan_stop_network_command(int argc, char **argv);

/**
 * This API is used to process the response for the stop UAP network command.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_stop_network_response(uint8_t *res);

/**
 * This API is used to get the information for all STAs those connected to an AP.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-get-uap-sta-list".
 *
 * \return WM_SUCCESS if UAP starts successfully.
 * \return -WM_FAIL if failure.
 */
int wlan_get_uap_sta_list_command(int argc, char **argv);

/**
 * This API is used to process the information for all STAs those connected to an AP.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                   Response body refer to \ref NCP_CMD_NETWORK_UAP_STA_LIST.
 *
 * \return WM_SUCCESS.
 */
int wlan_process_get_uap_sta_list(uint8_t *res);

/**
 * This API is used to get all the networks information those added locally.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 1.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-list".
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_list_command(int argc, char **argv);

/**
 * This API is used to process information of all the network those added locally.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                   Response body refer to \ref NCP_CMD_NETWORK_LIST.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_network_list_response(uint8_t *res);

/**
 * This API is used to remove an AP from the local network list.
 *
 * \param[in] argc    Argument count, the number of string pointed to by argv,
 *                    argc should be 2.
 * \param[in] argv    Argument vector.\n
 *                    argv[0]: should be "wlan-remove".\n
 *                    argv[1]: string value of network name.
 *
 * \return WM_SUCCESS if successful.
 * \return -WM_FAIL if failure.
 *
 */
int wlan_remove_command(int argc, char **argv);

/**
 * This API is used to process the removed AP information.
 *
 * \param[in] res    A pointer to \ref MCU_NCPCmd_DS_COMMAND response.
 *                   Response body refer to \ref NCP_CMD_NETWORK_REMOVE.
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure.
 */
int wlan_process_network_remove_response(uint8_t *res);

/**
 * This is a private NCP throughput test API that needs to play with sample TCP/UDP server/client, 
 * and could not play with open source iperf.
 *
 * \param[in] argc    Argument count, the number of strings pointed to by argv,\n
 *                    argc should range from 5 to 9.
 * \param[in] argv    Argument vector,\n
 *                    argv[0]: wlan-ncp-iperf\n
 *                    argv[1]: string value of socket handle index opended (Required)\n
 *                    argv[2]: string value of protocol type, "tcp" or "udp". (Required)\n
 *                    argv[3]: string value of data direction, "tx"(send) or "rx"(receive) (Required)\n
 *                    argv[4]: string value of packet count for TCP or string value of peer IP address for UDP
 *                    (Required)\n
 *                    argv[5]: string value of peer port number for UDP (Required)\n
 *                    argv[6]: string value of packet count except to receive or send for UDP (Required)\n
 *                    argv[7]: string value of data rate for UDP (Required)\n
 *                    argv[8]: string value of duration for UDP (Optional).
 *
 * \return WM_SUCCESS if success.
 * \return -WM_FAIL if failure
 *
 */
int wlan_ncp_iperf_command(int argc, char **argv);
#endif /*__NCP_HOST_COMMAND_WIFI_H_*/
