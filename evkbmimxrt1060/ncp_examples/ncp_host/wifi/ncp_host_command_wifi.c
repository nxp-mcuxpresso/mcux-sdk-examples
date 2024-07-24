/** @file ncp_host_command_wifi.c
 *
 *  @brief  This file provides API functions to build tlv commands and process tlv responses.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#include "ncp_host_command.h"
#include "ncp_host_command_wifi.h"

static uint8_t broadcast_mac[NCP_WLAN_MAC_ADDR_LENGTH] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

int cli_optind   = 0;
char *cli_optarg = NULL;
ping_msg_t ping_msg;
extern int ping_sock_handle;

static int mdns_result_num;

static bool socket_receive_res  = false;
static bool socket_recvfrom_res = false;
extern power_cfg_t global_power_config;

#if CONFIG_NCP_SPI
extern AT_NONCACHEABLE_SECTION_INIT(uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN]);
#else
extern uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN];
#endif

extern int mcu_get_command_lock();
extern int ncp_host_cli_register_commands(const struct ncp_host_cli_command *commands, int num_commands);
NCP_WLAN_NET_MONITOR_PARA g_net_monitor_param;
wlan_csi_config_params_t g_csi_params;


MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_wifi()
{
    return (MCU_NCPCmd_DS_COMMAND *)(mcu_tlv_command_buff);
}

/**
 * @brief         Convert IP string to hex IP
 *
 * @param number  A pointer to int
 * @return        If IPstr can be converted hex IP: hex ip value, else : -WM_FAIL
 */
int strip_to_hex(int *number, int len)
{
    int ip_value = 0;
    for (int jk = 0; jk < len; jk++)
    {
        int temp = 1;
        for (int ji = 1; ji < len - jk; ji++)
            temp *= 10;
        ip_value += number[jk] * temp;
    }
    if (ip_value > NCP_HOST_IP_VALID)
        return -WM_FAIL;

    return ip_value;
}

/**
 * @brief         Convert IP string to hex IP
 *
 * @param IPstr   A pointer to char
 * @param hex     A pointer to uint8_t
 * @return        If IPstr can be converted hex IP: WM_SUCCESS, else : -WM_FAIL
 */
int IP_to_hex(char *IPstr, uint8_t *hex)
{
    int len          = strlen(IPstr);
    int ip_number[3] = {0};
    int j = 0, k = 0, dot_number = 0, hex_numer = 0;
    for (int i = 0; i < len; i++)
    {
        if (IPstr[i] == '.')
        {
            if (j > 0)
            {
                int hex_ip = strip_to_hex(ip_number, j);
                if (hex_ip == -WM_FAIL)
                {
                    PRINTF("Please input the correct IP address!\r\n");
                    return -WM_FAIL;
                }
                hex[k++] = hex_ip;
                j        = 0;
                hex_numer++;
            }
            dot_number++;
        }
        else if (IPstr[i] >= '0' && IPstr[i] <= '9')
        {
            if (j >= 3)
            {
                PRINTF("Please input the correct IP address!\r\n");
                return -WM_FAIL;
            }
            ip_number[j] = IPstr[i] - '0';
            j++;
        }
        else
        {
            PRINTF("Please input the correct IP address!\r\n");
            return -WM_FAIL;
        }
    }
    /* String IP address check*/
    if (dot_number != 3) // the number of '.' should be 3
    {
        PRINTF("Please input the correct IP address!\r\n");
        return -WM_FAIL;
    }
    if (dot_number == 3 && j > 0)
        hex_numer++;
    if (hex_numer != 4) // the number of ip number should be 4
    {
        PRINTF("Please input the correct IP address!\r\n");
        return -WM_FAIL;
    }

    int hex_ip = strip_to_hex(ip_number, j);
    if (hex_ip == -WM_FAIL)
    {
        PRINTF("Please input the correct IP address!\r\n");
        return -WM_FAIL;
    }
    hex[k] = hex_ip;

    return WM_SUCCESS;
}

extern ping_res_t ping_res;

/* Handle the ICMP echo response and extract required parameters */
static void ping_recv(NCP_CMD_SOCKET_RECVFROM_CFG *recv)
{
    int ret = -WM_FAIL;
    struct ip_hdr *iphdr;
    struct icmp_echo_hdr *iecho;

    /* Received length should be greater than size of IP header and
     * size of ICMP header */
    if (recv->recv_size >= (int)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
    {
        iphdr = (struct ip_hdr *)recv->recv_data;
        /* Calculate the offset of ICMP header */
        iecho = (struct icmp_echo_hdr *)(recv->recv_data + ((iphdr->_v_hl & 0x0f) * 4));

        /* Verify that the echo response is for the echo request
         * we sent by checking PING_ID and sequence number */
        if ((iecho->id == PING_ID) && (iecho->seqno == PP_HTONS(ping_res.seq_no)))
        {
            /* Increment the receive counter */
            ping_res.recvd++;
            /* To display successful ping stats, destination
             * IP address is required */
            (void)memcpy(ping_res.ip_addr, recv->peer_ip, sizeof(recv->peer_ip));

            /* Extract TTL and send back so that it can be
             * displayed in ping statistics */
            ping_res.ttl = iphdr->_ttl;
            ret          = WM_SUCCESS;
        }
        else
        {
            ret = -WM_FAIL;
        }

        ping_res.echo_resp = ret;
        ping_res.size      = ping_msg.size;
    }
}

/**
 * @brief This function prepares ncp iperf command
 *
 * @return Status returned
 */
iperf_msg_t iperf_msg;
int wlan_ncp_iperf_command(int argc, char **argv)
{
    unsigned int handle      = 0;
    unsigned int type        = -1;
    unsigned int direction   = -1;
    enum ncp_iperf_item item = FALSE_ITEM;
    if (argc < 4)
    {
        (void)PRINTF("Usage: %s handle [tcp|udp] [tx|rx]\r\n", __func__);
        return -WM_FAIL;
    }
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle [tcp|udp] [tx|rx]\r\n", __func__);
        return -WM_FAIL;
    }
    iperf_msg.handle = handle;
    if (!strncmp(argv[2], "tcp", 3))
    {
        type = 0;
        if (argc == 5)
        {
            if (get_uint(argv[4], (unsigned int *)&iperf_msg.iperf_set.iperf_count, strlen(argv[4])))
            {
                (void)PRINTF("tcp packet number format is error\r\n");
                return -WM_FAIL;
            }
        }
        else
            iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;
    }
    else if (!strncmp(argv[2], "udp", 3))
    {
        type = 1;
        if (argc < 5)
        {
            (void)PRINTF("udp want ip and port, Usage: %s handle udp [tx|rx] ip port\r\n", __func__);
            return -WM_FAIL;
        }
        memcpy(iperf_msg.ip_addr, argv[4], strlen(argv[4]) + 1);

        if (argc >= 6)
        {
            if (get_uint(argv[5], (unsigned int *)&iperf_msg.port, strlen(argv[5])))
            {
                (void)PRINTF("udp port format is error\r\n");
                return -WM_FAIL;
            }
        }
        else
            iperf_msg.port = NCP_IPERF_UDP_SERVER_PORT_DEFAULT;

        if (argc >= 7)
        {
            if (get_uint(argv[6], (unsigned int *)&iperf_msg.iperf_set.iperf_count, strlen(argv[6])))
            {
                (void)PRINTF("tcp packet number format is error\r\n");
                return -WM_FAIL;
            }
        }
        else
            iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;

        if (argc >= 8)
        {
            if (get_uint(argv[7], (unsigned int *)&iperf_msg.iperf_set.iperf_udp_rate, strlen(argv[7])))
            {
                (void)PRINTF("udp rate format is error\r\n");
                return -WM_FAIL;
            }
        }
        else
            iperf_msg.iperf_set.iperf_udp_rate = NCP_IPERF_UDP_RATE;

        if (argc >= 9)
        {
            if (get_uint(argv[8], (unsigned int *)&iperf_msg.iperf_set.iperf_udp_time, strlen(argv[8])))
            {
                (void)PRINTF("udp time format is error\r\n");
                return -WM_FAIL;
            }
        }
        else
            iperf_msg.iperf_set.iperf_udp_time = NCP_IPERF_UDP_TIME;
    }
    else
    {
        (void)PRINTF("Usage: %s handle [tcp|udp] [tx|rx]\r\n", __func__);
        return -WM_FAIL;
    }
    if (!strncmp(argv[3], "tx", 3))
        direction = 0;
    else if (!strncmp(argv[3], "rx", 3))
        direction = 1;
    else
    {
        (void)PRINTF("Usage: %s handle [tcp|udp] [tx|rx]\r\n", __func__);
        return -WM_FAIL;
    }

    if (!type && direction == 0)
        item = NCP_IPERF_TCP_TX;
    else if (!type && direction == 1)
        item = NCP_IPERF_TCP_RX;
    else if (type && direction == 0)
        item = NCP_IPERF_UDP_TX;
    else if (type && direction == 1)
        item = NCP_IPERF_UDP_RX;

    switch (item)
    {
        case NCP_IPERF_TCP_TX:
            iperf_msg.iperf_set.iperf_type = NCP_IPERF_TCP_TX;
            iperf_msg.per_size             = NCP_IPERF_PER_TCP_PKG_SIZE;
            if (iperf_msg.iperf_set.iperf_count == 0)
                iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;
            (void)iperf_tx_set_event(IPERF_TX_START);
            break;
        case NCP_IPERF_TCP_RX:
            iperf_msg.iperf_set.iperf_type = NCP_IPERF_TCP_RX;
            iperf_msg.per_size             = NCP_IPERF_PER_TCP_PKG_SIZE;
            if (iperf_msg.iperf_set.iperf_count == 0)
                iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;
            (void)iperf_rx_set_event(IPERF_RX_START);
            break;
        case NCP_IPERF_UDP_TX:
            iperf_msg.iperf_set.iperf_type = NCP_IPERF_UDP_TX;
            iperf_msg.per_size             = NCP_IPERF_PER_UDP_PKG_SIZE;
            if (iperf_msg.iperf_set.iperf_count == 0)
                iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;
            (void)iperf_tx_set_event(IPERF_TX_START);
            break;
        case NCP_IPERF_UDP_RX:
            iperf_msg.iperf_set.iperf_type = NCP_IPERF_UDP_RX;
            iperf_msg.per_size             = NCP_IPERF_PER_UDP_PKG_SIZE;
            if (iperf_msg.iperf_set.iperf_count == 0)
                iperf_msg.iperf_set.iperf_count = NCP_IPERF_PKG_COUNT;
            (void)iperf_rx_set_event(IPERF_RX_START);
            break;
        default:
            return -WM_FAIL;
    }
    return WM_SUCCESS;
}

/**
 * @brief This function prepares scan command
 *
 * @return Status returned
 */
int wlan_scan_command(int argc, char **argv)
{
    mcu_get_command_lock();
    MCU_NCPCmd_DS_COMMAND *scan_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)scan_command, 0, NCP_HOST_COMMAND_LEN);

    scan_command->header.cmd      = NCP_CMD_WLAN_STA_SCAN;
    scan_command->header.size     = NCP_CMD_HEADER_LEN;
    scan_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares connect command
 *
 * @return Status returned
 */
int wlan_connect_command(int argc, char **argv)
{
    mcu_get_command_lock();
    MCU_NCPCmd_DS_COMMAND *connect_command = ncp_host_get_cmd_buffer_wifi();

    if (argc > 2)
    {
        (void)PRINTF("invalid argument\r\n");
        return -WM_FAIL;
    }

    (void)memset((uint8_t *)connect_command, 0, NCP_HOST_COMMAND_LEN);
    connect_command->header.cmd      = NCP_CMD_WLAN_STA_CONNECT;
    connect_command->header.size     = NCP_CMD_HEADER_LEN;
    connect_command->header.result   = NCP_CMD_RESULT_OK;

    if (argc == 2)
    {
        NCP_CMD_WLAN_CONN *conn = (NCP_CMD_WLAN_CONN *)&connect_command->params.wlan_connect;
        (void)memcpy(conn->name, argv[1], strlen(argv[1]) + 1);
        connect_command->header.size += sizeof(NCP_CMD_WLAN_CONN);
    }

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares disconnect command
 *
 * @return Status returned
 */
int wlan_disconnect_command(int argc, char **argv)
{
    mcu_get_command_lock();
    MCU_NCPCmd_DS_COMMAND *disconnect_command = ncp_host_get_cmd_buffer_wifi();
    disconnect_command->header.cmd            = NCP_CMD_WLAN_STA_DISCONNECT;
    disconnect_command->header.size           = NCP_CMD_HEADER_LEN;
    disconnect_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_version_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *fw_ver_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)fw_ver_command, 0, NCP_HOST_COMMAND_LEN);

    fw_ver_command->header.cmd      = NCP_CMD_WLAN_STA_VERSION;
    fw_ver_command->header.size     = NCP_CMD_HEADER_LEN;
    fw_ver_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_stat_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *conn_stat_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)conn_stat_command, 0, NCP_HOST_COMMAND_LEN);

    conn_stat_command->header.cmd      = NCP_CMD_WLAN_STA_CONNECT_STAT;
    conn_stat_command->header.size     = NCP_CMD_HEADER_LEN;
    conn_stat_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_reset_command(int argc, char **argv)
{
    int option;
    option = atoi(argv[1]);
    if (argc != 2 || (option != 0 && option != 1 && option != 2))
    {
        (void)PRINTF("Usage: %s <options>\r\n", argv[0]);
        (void)PRINTF("0 to Disable WiFi\r\n");
        (void)PRINTF("1 to Enable WiFi\r\n");
        (void)PRINTF("2 to Reset WiFi\r\n");
        return -WM_FAIL;
    }

    if (option == 2)
    {
        (void)memset((void*)&g_csi_params, 0, sizeof(g_csi_params));
        (void)memset((void*)&g_net_monitor_param, 0, sizeof(g_net_monitor_param));
        (void)memset((void*)&global_power_config, 0, sizeof(global_power_config));
    }
    MCU_NCPCmd_DS_COMMAND *conn_reset_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)conn_reset_command, 0, NCP_HOST_COMMAND_LEN);
    conn_reset_command->header.cmd      = NCP_CMD_WLAN_BASIC_WLAN_RESET;
    conn_reset_command->header.size     = NCP_CMD_HEADER_LEN;
    conn_reset_command->header.result   = NCP_CMD_RESULT_OK;

    WLAN_RESET_data *reset_cfg = (WLAN_RESET_data *)&conn_reset_command->params.reset_config;
    reset_cfg->option          = option;
    conn_reset_command->header.size += sizeof(WLAN_RESET_data);

    return WM_SUCCESS;
}

static void dump_wlan_roaming_command(const char *str)
{
    (void)PRINTF("Usage: %s <enable> <rssi_threshold>\r\n", str);
    (void)PRINTF("      <enable>         : 0 - disable\r\n");
    (void)PRINTF("                         1 - enable\r\n");
    (void)PRINTF("      <rssi_threshold> : weak RSSI threshold in dBm (absolute value)\r\n");
    (void)PRINTF("                         default = 70\r\n");
    return;
}

int wlan_roaming_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *roaming_command = ncp_host_get_cmd_buffer_wifi();
    int enable                             = 0;

    if (argc < 2 || argc > 3)
    {
        dump_wlan_roaming_command(argv[0]);
        return -WM_FAIL;
    }

    enable = atoi(argv[1]);
    if (enable != 0 && enable != 1)
    {
        dump_wlan_roaming_command(argv[0]);
        return -WM_FAIL;
    }

    roaming_command->header.cmd      = NCP_CMD_WLAN_STA_ROAMING;
    roaming_command->header.size     = NCP_CMD_HEADER_LEN;
    roaming_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ROAMING *roaming = (NCP_CMD_ROAMING *)&roaming_command->params.roaming_cfg;
    roaming->enable          = enable;
    if (argc == 3)
        roaming->rssi_threshold = atoi(argv[2]);
    else
        roaming->rssi_threshold = NCP_WLAN_DEFAULT_RSSI_THRESHOLD;
    roaming_command->header.size += sizeof(NCP_CMD_ROAMING);

    return WM_SUCCESS;
}

int wlan_uap_prov_start_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *uap_prov_start_command = ncp_host_get_cmd_buffer_wifi();
    uap_prov_start_command->header.cmd            = NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_START;
    uap_prov_start_command->header.size           = NCP_CMD_HEADER_LEN;
    uap_prov_start_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_uap_prov_reset_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *uap_prov_reset_command = ncp_host_get_cmd_buffer_wifi();
    uap_prov_reset_command->header.cmd            = NCP_CMD_WLAN_BASIC_WLAN_UAP_PROV_RESET;
    uap_prov_reset_command->header.size           = NCP_CMD_HEADER_LEN;
    uap_prov_reset_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan-uap-prov-start response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_uap_prov_start_result_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("wlan-uap-prov-start is ok!\r\n");
    else
        (void)PRINTF("wlan-uap-prov-start is fail!\r\n");
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan-uap-prov-reset response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_uap_prov_reset_result_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("wlan-uap-prov-reset is ok!\r\n");
    else
        (void)PRINTF("wlan-uap-prov-reset is fail!\r\n");
    return WM_SUCCESS;
}

static void dump_wlan_multi_mef_command(const char *str)
{
    (void)PRINTF("Usage: %s <type> <action>\r\n", str);
    (void)PRINTF("      <type>   : ping/arp/multicast/ns\r\n");
    (void)PRINTF("                     - MEF entry type, will add one mef entry at a time\r\n");
    (void)PRINTF("                 del - Delete all previous MEF entries\r\n");
    (void)PRINTF("                       <action> is not needed for this type\r\n");
    (void)PRINTF("      <action> : 0 - discard and not wake host\r\n");
    (void)PRINTF("                 1 - discard and wake host\r\n");
    (void)PRINTF("                 3 - allow and wake host\r\n");
    return;
}

int wlan_multi_mef_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *mef_command = ncp_host_get_cmd_buffer_wifi();
    int type                           = MEF_TYPE_END;
    uint8_t action                     = 0;

    (void)memset((uint8_t *)mef_command, 0, NCP_HOST_COMMAND_LEN);
    if (argc < 2 || argc > 3)
    {
        dump_wlan_multi_mef_command(argv[0]);
        return -WM_FAIL;
    }

    if (argc == 2)
    {
        if (string_equal("del", argv[1]))
            type = MEF_TYPE_DELETE;
        else
        {
            (void)PRINTF("Invalid type!\r\n");
            dump_wlan_multi_mef_command(argv[0]);
            return -WM_FAIL;
        }
    }
    else if (argc == 3)
    {
        if (string_equal("ping", argv[1]))
            type = MEF_TYPE_PING;
        else if (string_equal("arp", argv[1]))
            type = MEF_TYPE_ARP;
        else if (string_equal("multicast", argv[1]))
            type = MEF_TYPE_MULTICAST;

        else if (string_equal("ns", argv[1]))
            type = MEF_TYPE_IPV6_NS;
        else
        {
            (void)PRINTF("Invalid type!\r\n");
            dump_wlan_multi_mef_command(argv[0]);
            return -WM_FAIL;
        }
        action = (uint8_t)atoi(argv[2]);
        if (action != 0 && action != 1 && action != 3)
        {
            (void)PRINTF("Invalid action!\r\n");
            dump_wlan_multi_mef_command(argv[0]);
            return -WM_FAIL;
        }
    }

    mef_command->header.cmd      = NCP_CMD_WLAN_POWERMGMT_MEF;
    mef_command->header.size     = NCP_CMD_HEADER_LEN;
    mef_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_MEF *mef_config = (NCP_CMD_POWERMGMT_MEF *)&mef_command->params.mef_config;
    mef_config->type                  = type;
    if (argc == 3)
        mef_config->action = action;
    mef_command->header.size += sizeof(NCP_CMD_POWERMGMT_MEF);

    return WM_SUCCESS;
}

int wlan_wakeup_condition_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wowlan_cfg_cmd = ncp_host_get_cmd_buffer_wifi();
    uint8_t is_mef                        = false;
    uint32_t wake_up_conds                = 0;

    if (argc < 2 || argc > 3)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    %s <wowlan [wake_up_conds]>/<mef>\r\n", argv[0]);
        (void)PRINTF("    wowolan -- wowlan conditions\r\n");
        (void)PRINTF("    [wake_up_conds] -- value for default wowlan conditions only\r\n");
        (void)PRINTF("              bit 0: WAKE_ON_ALL_BROADCAST\r\n");
        (void)PRINTF("              bit 1: WAKE_ON_UNICAST\r\n");
        (void)PRINTF("              bit 2: WAKE_ON_MAC_EVENT\r\n");
        (void)PRINTF("              bit 3: WAKE_ON_MULTICAST\r\n");
        (void)PRINTF("              bit 4: WAKE_ON_ARP_BROADCAST\r\n");
        (void)PRINTF("              bit 6: WAKE_ON_MGMT_FRAME\r\n");
        (void)PRINTF("              All bit 0 discard and not wakeup host\r\n");
        (void)PRINTF("    mef     -- MEF wowlan condition\r\n");
        (void)PRINTF("Example:\r\n");
        (void)PRINTF("    %s mef\r\n", argv[0]);
        (void)PRINTF("    %s wowlan 0x1e\r\n", argv[0]);
        return -WM_FAIL;
    }
    if (string_equal("mef", argv[1]))
        is_mef = true;
    else if (string_equal("wowlan", argv[1]))
    {
        if (argc < 3)
        {
            (void)PRINTF("wake_up_conds need be specified\r\n");
            return -WM_FAIL;
        }
        wake_up_conds = a2hex_or_atoi(argv[2]);
    }
    else
    {
        (void)PRINTF("Invalid wakeup condition.\r\n");
        return -WM_FAIL;
    }

    wowlan_cfg_cmd->header.cmd                  = NCP_CMD_WLAN_POWERMGMT_WOWLAN_CFG;
    wowlan_cfg_cmd->header.size                 = NCP_CMD_HEADER_LEN;
    wowlan_cfg_cmd->header.result               = NCP_CMD_RESULT_OK;
    NCP_CMD_POWERMGMT_WOWLAN_CFG *wowlan_config = (NCP_CMD_POWERMGMT_WOWLAN_CFG *)&wowlan_cfg_cmd->params.wowlan_config;
    wowlan_config->is_mef                       = is_mef;
    wowlan_config->wake_up_conds                = wake_up_conds;
    wowlan_cfg_cmd->header.size += sizeof(NCP_CMD_POWERMGMT_WOWLAN_CFG);

    global_power_config.is_mef        = is_mef;
    global_power_config.wake_up_conds = wake_up_conds;
    return WM_SUCCESS;
}

int wlan_suspend_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *suspend_command = ncp_host_get_cmd_buffer_wifi();
    int mode                               = 0;

    if (!global_power_config.is_manual)
    {
        (void)PRINTF("Suspend command is not allowed because manual method is not selected\r\n");
        return -WM_FAIL;
    }
    if (argc != 2)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("    wlan-suspend <power mode>\r\n");
        (void)PRINTF("    1:PM1 2:PM2 3:PM3 4:PM4\r\n");
        (void)PRINTF("Example:\r\n");
        (void)PRINTF("    wlan-suspend 3\r\n");
        return -WM_FAIL;
    }
    mode = atoi(argv[1]);
    if (mode < 1 || mode > 4)
    {
        (void)PRINTF("Invalid low power mode\r\n");
        (void)PRINTF("Only PM1/PM2/PM3/PM4 supported here\r\n");
        return -WM_FAIL;
    }
    if (((global_power_config.wake_mode == WAKE_MODE_INTF) && (mode > 2)) ||
        ((global_power_config.wake_mode == WAKE_MODE_GPIO) && (mode > 3)))
    {
        (void)PRINTF("Invalid power mode %d!\r\n", mode);
        (void)PRINTF("Only PM1/2 is allowed with wake mode INTF\r\n");
        (void)PRINTF("Only PM1/2/3 is allowed with wake mode GPIO\r\n");
        return -WM_FAIL;
    }

    suspend_command->header.cmd      = NCP_CMD_WLAN_POWERMGMT_SUSPEND;
    suspend_command->header.size     = NCP_CMD_HEADER_LEN;
    suspend_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_SUSPEND *suspend_config = (NCP_CMD_POWERMGMT_SUSPEND *)&suspend_command->params.suspend_config;
    suspend_config->mode                      = mode;
    suspend_command->header.size += sizeof(NCP_CMD_POWERMGMT_SUSPEND);

    return WM_SUCCESS;
}

int wlan_deep_sleep_ps_command(int argc, char **argv)
{
    int deep_sleep_enable;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <0/1> < 0--disable deep sleep; 1---enable deep sleep>\r\n", argv[0]);
        return -WM_FAIL;
    }

    deep_sleep_enable = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *wlan_deep_sleep_ps_command = ncp_host_get_cmd_buffer_wifi();
    wlan_deep_sleep_ps_command->header.cmd            = NCP_CMD_WLAN_POWERMGMT_DEEP_SLEEP_PS;
    wlan_deep_sleep_ps_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_deep_sleep_ps_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_DEEP_SLEEP_PS *wlan_deep_sleep_ps =
        (NCP_CMD_DEEP_SLEEP_PS *)&wlan_deep_sleep_ps_command->params.wlan_deep_sleep_ps;
    wlan_deep_sleep_ps->enable = deep_sleep_enable;
    wlan_deep_sleep_ps_command->header.size += sizeof(NCP_CMD_DEEP_SLEEP_PS);

    return WM_SUCCESS;
}

int wlan_ieee_ps_command(int argc, char **argv)
{
    int ieee_enable;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <0/1> < 0--disable ieee ps; 1---enable ieee ps>\r\n", argv[0]);
        return -WM_FAIL;
    }

    ieee_enable = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *wlan_ieee_ps_command = ncp_host_get_cmd_buffer_wifi();
    wlan_ieee_ps_command->header.cmd            = NCP_CMD_WLAN_POWERMGMT_IEEE_PS;
    wlan_ieee_ps_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_ieee_ps_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_IEEE_PS *wlan_ieee_ps = (NCP_CMD_IEEE_PS *)&wlan_ieee_ps_command->params.wlan_ieee_ps;
    wlan_ieee_ps->enable          = ieee_enable;
    wlan_ieee_ps_command->header.size += sizeof(NCP_CMD_IEEE_PS);

    return WM_SUCCESS;
}

int wlan_set_wmm_uapsd_command(int argc, char **argv)
{
    int enable_uapsd;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <0/1> < 0--disable UAPSD; 1---enable UAPSD>\r\n", argv[0]);
        return -WM_FAIL;
    }

    enable_uapsd = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *uapsd_command = ncp_host_get_cmd_buffer_wifi();
    uapsd_command->header.cmd            = NCP_CMD_WLAN_POWERMGMT_UAPSD;
    uapsd_command->header.size           = NCP_CMD_HEADER_LEN;
    uapsd_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_UAPSD *wlan_uapsd_cfg = (NCP_CMD_POWERMGMT_UAPSD *)&uapsd_command->params.uapsd_cfg;
    wlan_uapsd_cfg->enable                  = enable_uapsd;
    uapsd_command->header.size += sizeof(NCP_CMD_POWERMGMT_UAPSD);

    return WM_SUCCESS;
}

int wlan_process_wmm_uapsd_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("UAPSD cfg is success!\r\n");
    else
        (void)PRINTF("UAPSD cfg is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_wmm_uapsd_qosinfo_command(int argc, char **argv)
{
    uint8_t qos_info = 0;

    if (argc != 1 && argc != 2)
    {
        (void)PRINTF("Usage: %s <null | qos_info>\r\n", argv[0]);
        (void)PRINTF("qos_info: bit0:VO; bit1:VI; bit2:BK; bit3:BE\r\n");
        return -WM_FAIL;
    }

    if (argc == 2)
        qos_info = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *qosinfo_command = ncp_host_get_cmd_buffer_wifi();
    qosinfo_command->header.cmd            = NCP_CMD_WLAN_POWERMGMT_QOSINFO;
    qosinfo_command->header.size           = NCP_CMD_HEADER_LEN;
    qosinfo_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_QOSINFO *qosinfo_cfg = (NCP_CMD_POWERMGMT_QOSINFO *)&qosinfo_command->params.qosinfo_cfg;
    qosinfo_cfg->qos_info                  = qos_info;
    if (argc == 1)
        qosinfo_cfg->action = 0;
    else
        qosinfo_cfg->action = 1;

    qosinfo_command->header.size += sizeof(NCP_CMD_POWERMGMT_QOSINFO);

    return WM_SUCCESS;
}

int wlan_process_uapsd_qosinfo_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res         = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_POWERMGMT_QOSINFO *qosinfo_cfg = (NCP_CMD_POWERMGMT_QOSINFO *)&cmd_res->params.qosinfo_cfg;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("qosinfo is %u\r\n", qosinfo_cfg->qos_info);
    else
        (void)PRINTF("qosinfo cfg is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_uapsd_sleep_period_command(int argc, char **argv)
{
    uint32_t period = 0;

    if (argc != 1 && argc != 2)
    {
        (void)PRINTF("Usage: %s <null | period(ms)>\r\n", argv[0]);
        return -WM_FAIL;
    }

    if (argc == 2)
        period = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *sleep_period_command = ncp_host_get_cmd_buffer_wifi();
    sleep_period_command->header.cmd            = NCP_CMD_WLAN_POWERMGMT_SLEEP_PERIOD;
    sleep_period_command->header.size           = NCP_CMD_HEADER_LEN;
    sleep_period_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_POWERMGMT_SLEEP_PERIOD *sleep_period_cfg =
        (NCP_CMD_POWERMGMT_SLEEP_PERIOD *)&sleep_period_command->params.sleep_period_cfg;
    sleep_period_cfg->period = period;
    if (argc == 1)
        sleep_period_cfg->action = 0;
    else
        sleep_period_cfg->action = 1;

    sleep_period_command->header.size += sizeof(NCP_CMD_POWERMGMT_SLEEP_PERIOD);

    return WM_SUCCESS;
}

int wlan_process_uapsd_sleep_period_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_POWERMGMT_SLEEP_PERIOD *sleep_period_cfg =
        (NCP_CMD_POWERMGMT_SLEEP_PERIOD *)&cmd_res->params.sleep_period_cfg;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("sleep period is %u\r\n", sleep_period_cfg->period);
    else
        (void)PRINTF("sleep period cfg is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_process_wakeup_condition_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    uint16_t result                = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_OK)
        (void)PRINTF("Wakeup condition set is successful!\r\n");
    else
    {
        (void)PRINTF("Wakeup condition set is failed!\r\n");
        /* Clear corresponding setting if failed */
        global_power_config.is_mef        = 0;
        global_power_config.wake_up_conds = 0;
    }

    return WM_SUCCESS;
}

int wlan_process_suspend_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    uint16_t result                = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_ERROR)
        (void)PRINTF("suspend command is failed\r\n");
    else
        (void)PRINTF("suspend command is successful\r\n");
    return WM_SUCCESS;
}

static void dump_wlan_eu_crypto_ccmp_128(void)
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("Algorithm AES-CCMP-128 encryption and decryption verification\r\n");
    (void)PRINTF("wlan-eu-crypto-ccmp-128 <EncDec>\r\n");
    (void)PRINTF("EncDec: 0-Decrypt, 1-Encrypt\r\n");
}

int wlan_eu_crypto_ccmp128_command(int argc, char **argv)
{
    unsigned int EncDec = 0;

    if (argc != 2)
    {
        dump_wlan_eu_crypto_ccmp_128();
        (void)PRINTF("Error: invalid number of arguments\r\n");
        return -WM_FAIL;
    }

    EncDec = a2hex_or_atoi(argv[1]);
    if (EncDec != 0U && EncDec != 1U)
    {
        dump_wlan_eu_crypto_ccmp_128();
        (void)PRINTF("Error: invalid EncDec\r\n");
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_CCMP_128;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_EU_CRYPRTO *eu_crypto = (NCP_CMD_EU_CRYPRTO *)&command->params.eu_crypto;
    eu_crypto->enc = EncDec;
    command->header.size += sizeof(NCP_CMD_EU_CRYPRTO);

    return WM_SUCCESS;
}

int wlan_process_eu_crypto_ccmp128_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res       = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("EU Crypto CCMP 128 verified successfully.\r\n");
    }
    else
    {
        (void)PRINTF("EU Crypto CCMP 128 verified failed.\r\n");
    }

    return WM_SUCCESS;
}



static void dump_wlan_eu_crypto_gcmp_128(void)
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("Algorithm AES-GCMP-128 encryption and decryption verification\r\n");
    (void)PRINTF("wlan-eu-crypto-gcmp-128 <EncDec>\r\n");
    (void)PRINTF("EncDec: 0-Decrypt, 1-Encrypt\r\n");
}

int wlan_eu_crypto_gcmp128_command(int argc, char **argv)
{
    unsigned int EncDec = 0;

    if (argc != 2)
    {
        dump_wlan_eu_crypto_gcmp_128();
        (void)PRINTF("Error: invalid number of arguments\r\n");
        return -WM_FAIL;
    }

    EncDec = a2hex_or_atoi(argv[1]);
    if (EncDec != 0U && EncDec != 1U)
    {
        dump_wlan_eu_crypto_gcmp_128();
        (void)PRINTF("Error: invalid EncDec\r\n");
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_REGULATORY_EU_CRYPTO_GCMP_128;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_EU_CRYPRTO *eu_crypto = (NCP_CMD_EU_CRYPRTO *)&command->params.eu_crypto;
    eu_crypto->enc = EncDec;
    command->header.size += sizeof(NCP_CMD_EU_CRYPRTO);

    return WM_SUCCESS;
}

int wlan_process_eu_crypto_gcmp128_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res       = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("EU Crypto GCMP 128 verified successfully.\r\n");
    }
    else
    {
        (void)PRINTF("EU Crypto GCMP 128 verified failed.\r\n");
    }

    return WM_SUCCESS;
}

int wlan_set_mac_address_command(int argc, char **argv)
{
    int ret;
    uint8_t raw_mac[NCP_WLAN_MAC_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *mac_addr_command = ncp_host_get_cmd_buffer_wifi();

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s MAC_Address\r\n", argv[0]);
        return -WM_FAIL;
    }

    ret = get_mac(argv[1], (char *)raw_mac, ':');
    if (ret != 0)
    {
        (void)PRINTF("Error: invalid MAC argument\r\n");
        return -WM_FAIL;
    }

    if ((memcmp(&raw_mac[0], broadcast_mac, NCP_WLAN_MAC_ADDR_LENGTH) == 0) || (raw_mac[0] & 0x01))
    {
        (void)PRINTF("Error: only support unicast mac\r\n");
        return -WM_FAIL;
    }

    mac_addr_command->header.cmd      = NCP_CMD_WLAN_STA_SET_MAC;
    mac_addr_command->header.size     = NCP_CMD_HEADER_LEN;
    mac_addr_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MAC_ADDRESS *mac_address = (NCP_CMD_MAC_ADDRESS *)&mac_addr_command->params.mac_addr;
    memcpy(mac_address->mac_addr, raw_mac, NCP_WLAN_MAC_ADDR_LENGTH);
    mac_addr_command->header.size += sizeof(NCP_CMD_MAC_ADDRESS);

    return WM_SUCCESS;
}

int wlan_get_mac_address_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *get_mac_command = ncp_host_get_cmd_buffer_wifi();
    get_mac_command->header.cmd            = NCP_CMD_WLAN_STA_GET_MAC;
    get_mac_command->header.size           = NCP_CMD_HEADER_LEN;
    get_mac_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_info_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *network_info_command = ncp_host_get_cmd_buffer_wifi();
    network_info_command->header.cmd            = NCP_CMD_WLAN_NETWORK_INFO;
    network_info_command->header.size           = NCP_CMD_HEADER_LEN;
    network_info_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}
int wlan_address_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *network_address_command = ncp_host_get_cmd_buffer_wifi();
    network_address_command->header.cmd            = NCP_CMD_WLAN_NETWORK_ADDRESS;
    network_address_command->header.size           = NCP_CMD_HEADER_LEN;
    network_address_command->header.result         = NCP_CMD_RESULT_OK;
    
    return WM_SUCCESS;
}
static void dump_wlan_add_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("For Station interface\r\n");
    (void)PRINTF("  For DHCP IP Address assignment:\r\n");
    (void)PRINTF(
        "    wlan-add <profile_name> ssid <ssid> [wpa2 <psk> <secret>]"
        "\r\n");
    (void)PRINTF("      If using WPA2 security, set the PMF configuration if required.\r\n");
    (void)PRINTF(
        "    wlan-add <profile_name> ssid <ssid> [wpa3 sae <secret> mfpc <1> mfpr <0/1>]"
        "\r\n");
    (void)PRINTF("      If using WPA3 SAE security, always set the PMF configuration.\r\n");

    (void)PRINTF("  For static IP address assignment:\r\n");
    (void)PRINTF(
        "    wlan-add <profile_name> ssid <ssid>\r\n"
        "    ip:<ip_addr>,<gateway_ip>,<netmask>\r\n");
    (void)PRINTF(
        "    [bssid <bssid>] [channel <channel number>]\r\n"
        "    [wpa2 <psk> <secret>]"
        "\r\n");

    (void)PRINTF("For Micro-AP interface\r\n");
    (void)PRINTF(
        "    wlan-add <profile_name> ssid <ssid>\r\n"
        "    ip:<ip_addr>,<gateway_ip>,<netmask>\r\n");
    (void)PRINTF(
        "    role uap [bssid <bssid>]\r\n"
        "    [channel <channelnumber>]\r\n");
    (void)PRINTF(
        "    [wpa2 <psk> <secret>]/[wpa <secret> wpa2 <psk> <secret>]/[wpa3 sae <secret> [pwe <0/1/2>] [tr <0/1>]]/[wpa2 <secret> wpa3 sae "
        "<secret>]");
#if CONFIG_NCP_EAP_TLS
    (void)PRINTF(
        "/[eap-tls aid <aid> key_passwd <key_passwd>]");
#endif
#if CONFIG_NCP_WIFI_CAPA
    (void)PRINTF("\r\n");
    (void)PRINTF("    [capa <11ax/11ac/11n/legacy>]");
#endif
    (void)PRINTF("\r\n");
    (void)PRINTF("    [mfpc <0/1>] [mfpr <0/1>]\r\n");
#if CONFIG_NCP_WIFI_DTIM_PERIOD
    (void)PRINTF("If seting dtim\r\n");
    (void)PRINTF(
        "    The value of dtim is an integer. The default value is 10.\r\n"
        "    The range of dtim is [1,255].\r\n");
#endif
    (void)PRINTF("If Set channel to 0, set acs_band to 0 1.\r\n");
    (void)PRINTF("0: 2.4GHz channel   1: 5GHz channel  Not support to select dual band automatically.\r\n");
}

/* Parse the 'arg' string as "ip:ipaddr,gwaddr,netmask,[dns1,dns2]" into
 * a wlan_ip_config data structure */
static int get_address(char *arg, IP_ParamSet_t *ip)
{
    char *ipaddr = NULL, *gwaddr = NULL, *netmask = NULL;
    char *dns1 = NULL, *dns2 = NULL;
    struct in_addr ip_s, gw_s, nm_s, dns1_s, dns2_s;

    ipaddr = strstr(arg, "ip:");
    if (ipaddr == NULL)
        return -1;
    ipaddr += 3;

    gwaddr = strstr(ipaddr, ",");
    if (gwaddr == NULL)
        return -1;
    *gwaddr++ = 0;

    netmask = strstr(gwaddr, ",");
    if (netmask == NULL)
        return -1;
    *netmask++ = 0;

    dns1 = strstr(netmask, ",");
    if (dns1 != NULL)
    {
        *dns1++ = 0;
        dns2    = strstr(dns1, ",");
    }

    ip_s.s_addr = 0U;
    inet_aton(ipaddr, &ip_s);
    ip->address = ip_s.s_addr;
    gw_s.s_addr = 0U;
    inet_aton(gwaddr, &gw_s);
    ip->gateway = gw_s.s_addr;
    nm_s.s_addr = 0U;
    inet_aton(netmask, &nm_s);
    ip->netmask = nm_s.s_addr;
    if (dns1 != NULL)
    {
        dns1_s.s_addr = 0U;
        inet_aton(dns1, &dns1_s);
        ip->dns1 = dns1_s.s_addr;
    }
    else
        ip->dns1 = 0x0;

    if (dns2 != NULL)
    {
        dns2_s.s_addr = 0U;
        inet_aton(dns2, &dns2_s);
        ip->dns2 = dns2_s.s_addr;
    }
    else
        ip->dns2 = 0x0;

    return 0;
}

static int get_security(int argc, char **argv, enum wlan_security_type type, Security_ParamSet_t *sec)
{
    if (argc < 1)
        return 1;

    switch (type)
    {
        case WLAN_SECURITY_WPA:
        case WLAN_SECURITY_WPA2:
            if (argc < 1)
                return 1;
            /* copy the PSK phrase */
            sec->password_len = strlen(argv[0]);
            if (sec->password_len < 65)
                strcpy(sec->password, argv[0]);
            else
                return 1;
            sec->type = type;
            break;
        default:
            return 1;
    }

    return 0;
}

int wlan_add_command(int argc, char **argv)
{
    int ret = 0;
    int arg = 1;
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
#if CONFIG_NCP_EAP_TLS
        unsigned aid : 1;
        unsigned key_passwd : 1;
#endif
        unsigned pwe: 1;
        unsigned tr : 1;
#if CONFIG_NCP_WIFI_DTIM_PERIOD
        unsigned dtim : 1;
#endif
        unsigned acs_band : 1;
#if CONFIG_NCP_WIFI_CAPA
        unsigned wlan_capa : 1;
#endif
    } info;

    MCU_NCPCmd_DS_COMMAND *network_add_command = ncp_host_get_cmd_buffer_wifi();
    NCP_CMD_NETWORK_ADD *network_add_tlv       = (NCP_CMD_NETWORK_ADD *)&network_add_command->params.network_add;
    uint8_t *ptlv_pos                          = network_add_tlv->tlv_buf;
    uint32_t tlv_buf_len                       = 0;
    SSID_ParamSet_t *ssid_tlv                  = NULL;
    BSSID_ParamSet_t *bssid_tlv                = NULL;
    Channel_ParamSet_t *channel_tlv            = NULL;
    Pwe_Derivation_ParamSet_t *pwe_tlv         = NULL;
    Tr_Disable_ParamSet_t *tr_tlv              = NULL;	    
    IP_ParamSet_t *ip_tlv                      = NULL;
    Security_ParamSet_t *security_wpa_tlv = NULL, *security_wpa2_tlv = NULL, *security_wpa3_tlv = NULL;
    PMF_ParamSet_t *pmf_tlv      = NULL;
#if CONFIG_NCP_EAP_TLS
    EAP_ParamSet_t *eap_tlv      = NULL;
#endif
    BSSRole_ParamSet_t *role_tlv = NULL;
#if CONFIG_NCP_WIFI_DTIM_PERIOD
    DTIM_ParamSet_t *dtim_tlv = NULL;
#endif
    ACSBand_ParamSet_t *acs_band_tlv = NULL;
#if CONFIG_NCP_WIFI_CAPA
    CAPA_ParamSet_t *capa_tlv = NULL;
#endif
    (void)memset(&info, 0, sizeof(info));

    if (argc < 4)
    {
        dump_wlan_add_usage();
        (void)PRINTF("Error: invalid number of arguments\r\n");
        return -WM_FAIL;
    }

    if (strlen(argv[arg]) >= WLAN_NETWORK_NAME_MAX_LENGTH)
    {
        (void)PRINTF("Error: network name too long\r\n");
        return -WM_FAIL;
    }

    (void)memcpy(network_add_tlv->name, argv[arg],
                 (strlen(argv[arg]) > WLAN_NETWORK_NAME_MAX_LENGTH - 1) ? (WLAN_NETWORK_NAME_MAX_LENGTH - 1) :
                                                                          strlen(argv[arg]) + 1);

    arg++;
    info.address = ADDR_TYPE_DHCP;
    do
    {
        if (!info.ssid && string_equal("ssid", argv[arg]))
        {
            if (strlen(argv[arg + 1]) > 32)
            {
                (void)PRINTF("Error: SSID is too long\r\n");
                return -WM_FAIL;
            }
            ssid_tlv = (SSID_ParamSet_t *)ptlv_pos;
            (void)memcpy(ssid_tlv->ssid, argv[arg + 1], strlen(argv[arg + 1]) + 1);
            ssid_tlv->header.type = NCP_CMD_NETWORK_SSID_TLV;
            ssid_tlv->header.size = sizeof(ssid_tlv->ssid);
            ptlv_pos += sizeof(SSID_ParamSet_t);
            tlv_buf_len += sizeof(SSID_ParamSet_t);
            arg += 2;
            info.ssid = 1;
        }
        else if (!info.bssid && string_equal("bssid", argv[arg]))
        {
            bssid_tlv = (BSSID_ParamSet_t *)ptlv_pos;
            ret       = get_mac(argv[arg + 1], bssid_tlv->bssid, ':');
            if (ret != 0)
            {
                (void)PRINTF(
                    "Error: invalid BSSID argument"
                    "\r\n");
                return -WM_FAIL;
            }
            bssid_tlv->header.type = NCP_CMD_NETWORK_BSSID_TLV;
            bssid_tlv->header.size = sizeof(bssid_tlv->bssid);
            ptlv_pos += sizeof(BSSID_ParamSet_t);
            tlv_buf_len += sizeof(BSSID_ParamSet_t);
            arg += 2;
            info.bssid = 1;
        }
        else if (!info.channel && string_equal("channel", argv[arg]))
        {
            channel_tlv = (Channel_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid channel"
                    " argument\n");
                return -WM_FAIL;
            }
            channel_tlv->channel     = atoi(argv[arg + 1]);
            channel_tlv->header.type = NCP_CMD_NETWORK_CHANNEL_TLV;
            channel_tlv->header.size = sizeof(channel_tlv->channel);
            ptlv_pos += sizeof(Channel_ParamSet_t);
            tlv_buf_len += sizeof(Channel_ParamSet_t);
            arg += 2;
            info.channel = 1;
        }
        else if (!strncmp(argv[arg], "ip:", 3))
        {
            if (ip_tlv == NULL)
            {
                ip_tlv              = (IP_ParamSet_t *)ptlv_pos;
                ret                 = get_address(argv[arg], ip_tlv);
                ip_tlv->header.type = NCP_CMD_NETWORK_IP_TLV;
                ip_tlv->header.size = sizeof(IP_ParamSet_t) - NCP_TLV_HEADER_LEN;
                ptlv_pos += sizeof(IP_ParamSet_t);
                tlv_buf_len += sizeof(IP_ParamSet_t);
            }
            else
            {
                ret = get_address(argv[arg], ip_tlv);
            }
            ip_tlv->is_autoip = 0;
            if (ret != 0)
            {
                (void)PRINTF(
                    "Error: invalid address"
                    " argument\n");
                return -WM_FAIL;
            }
            arg++;
            info.address = ADDR_TYPE_STATIC;
        }
        else if (!strncmp(argv[arg], "autoip", 6))
        {
            if (ip_tlv != NULL)
            {
                ip_tlv->is_autoip = 1;
            }
            else
            {
                ip_tlv              = (IP_ParamSet_t *)ptlv_pos;
                ip_tlv->is_autoip   = 1;
                ip_tlv->header.type = NCP_CMD_NETWORK_IP_TLV;
                ip_tlv->header.size = sizeof(IP_ParamSet_t) - NCP_TLV_HEADER_LEN;
                ptlv_pos += sizeof(IP_ParamSet_t);
                tlv_buf_len += sizeof(IP_ParamSet_t);
            }
            arg++;
            info.address = ADDR_TYPE_LLA;
        }
        else if (!info.security && string_equal("wpa", argv[arg]))
        {
            security_wpa_tlv = (Security_ParamSet_t *)ptlv_pos;
            ret              = get_security(argc - arg - 1, argv + arg + 1, WLAN_SECURITY_WPA, security_wpa_tlv);
            if (ret != 0)
            {
                (void)PRINTF(
                    "Error: invalid WPA security"
                    " argument\r\n");
                return -WM_FAIL;
            }
            security_wpa_tlv->header.type = NCP_CMD_NETWORK_SECURITY_TLV;
            security_wpa_tlv->header.size = sizeof(security_wpa_tlv->type) + sizeof(security_wpa_tlv->password_len) +
                                            security_wpa_tlv->password_len;
            ptlv_pos += NCP_TLV_HEADER_LEN + security_wpa_tlv->header.size;
            tlv_buf_len += NCP_TLV_HEADER_LEN + security_wpa_tlv->header.size;
            arg += 2;
            info.security++;
        }
        else if (!info.security2 && string_equal("wpa2", argv[arg]))
        {
            if (string_equal(argv[arg + 1], "psk") == 0)
            {
                (void)PRINTF("Error: invalid WPA2 security argument, lack of psk.\r\n");
                return -WM_FAIL;
            }
            security_wpa2_tlv = (Security_ParamSet_t *)ptlv_pos;
            security_wpa2_tlv->type = WLAN_SECURITY_WPA2;
            security_wpa2_tlv->password_len = strlen(argv[arg + 2]);
            /* copy the PSK phrase */            
            if (security_wpa2_tlv->password_len < WLAN_PSK_MIN_LENGTH || security_wpa2_tlv->password_len >= WLAN_PSK_MAX_LENGTH)
            {
                (void)PRINTF("Error: Invalid passphrase length %d (expected ASCII characters: 8..63)\r\n", strlen(argv[arg + 2]));
                return -WM_FAIL;
            }
            else
            {
                strncpy(security_wpa2_tlv->password, argv[arg + 2], strlen(argv[arg + 2]));
            }
			
            security_wpa2_tlv->header.type = NCP_CMD_NETWORK_SECURITY_TLV;
            security_wpa2_tlv->header.size = sizeof(security_wpa2_tlv->type) + sizeof(security_wpa2_tlv->password_len) +
                                             security_wpa2_tlv->password_len;
            ptlv_pos += NCP_TLV_HEADER_LEN + security_wpa2_tlv->header.size;
            tlv_buf_len += NCP_TLV_HEADER_LEN + security_wpa2_tlv->header.size;
            arg += 3;
            info.security2++;
        }
        else if (!info.security3 && string_equal("wpa3", argv[arg]))
        {
            if (string_equal(argv[arg + 1], "sae") != 0)
            {
                security_wpa3_tlv = (Security_ParamSet_t *)ptlv_pos;

                security_wpa3_tlv->type = WLAN_SECURITY_WPA3_SAE;
                /* copy the PSK phrase */
                security_wpa3_tlv->password_len = strlen(argv[arg + 2]);
                if (!security_wpa3_tlv->password_len)
                {
                    (void)PRINTF(
                        "Error: invalid WPA3 security"
                        " argument\r\n");
                    return -WM_FAIL;
                }
                if (security_wpa3_tlv->password_len < 255)
                    strcpy(security_wpa3_tlv->password, argv[arg + 2]);
                else
                {
                    (void)PRINTF(
                        "Error: invalid WPA3 security"
                        " argument\r\n");
                    return -WM_FAIL;
                }

                security_wpa3_tlv->header.type = NCP_CMD_NETWORK_SECURITY_TLV;
                security_wpa3_tlv->header.size = sizeof(security_wpa3_tlv->type) +
                                                 sizeof(security_wpa3_tlv->password_len) +
                                                 security_wpa3_tlv->password_len;
                ptlv_pos += NCP_TLV_HEADER_LEN + security_wpa3_tlv->header.size;
                tlv_buf_len += NCP_TLV_HEADER_LEN + security_wpa3_tlv->header.size;
                arg += 3;
            }
            else
            {
                (void)PRINTF(
                    "Error: invalid WPA3 security"
                    " argument\r\n");
                return -WM_FAIL;
            }
            info.security3++;
        }
#if CONFIG_NCP_EAP_TLS
        else if ((info.security2 == 0U) && (string_equal("eap-tls", argv[arg])))
        {
            security_wpa2_tlv = (Security_ParamSet_t *)ptlv_pos;
            security_wpa2_tlv->type = WLAN_SECURITY_EAP_TLS;
            security_wpa2_tlv->header.type = NCP_CMD_NETWORK_SECURITY_TLV;
            security_wpa2_tlv->header.size = sizeof(security_wpa2_tlv->type);
            ptlv_pos += NCP_TLV_HEADER_LEN + security_wpa2_tlv->header.size;
            tlv_buf_len += NCP_TLV_HEADER_LEN + security_wpa2_tlv->header.size;
            arg += 1;
            info.security2++;
        }
        else if ((info.aid == 0U) && (string_equal("aid", argv[arg])))
        {
            if (eap_tlv == NULL)
            {
                eap_tlv = (EAP_ParamSet_t *)ptlv_pos;
                eap_tlv->header.type = NCP_CMD_NETWORK_EAP_TLV;
                eap_tlv->header.size = sizeof(eap_tlv->anonymous_identity) + sizeof(eap_tlv->client_key_passwd);
                ptlv_pos += NCP_TLV_HEADER_LEN + eap_tlv->header.size;
                tlv_buf_len += NCP_TLV_HEADER_LEN + eap_tlv->header.size;
            }
            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid aid"
                    " argument\r\n");
                return -WM_FAIL;
            }
            /* Set Client Anonymous Identity */
            strcpy(eap_tlv->anonymous_identity, argv[arg + 1]);
            arg += 2;
            info.aid++;
        }
        else if ((info.key_passwd == 0U) && (string_equal("key_passwd", argv[arg])))
        {
            if (eap_tlv == NULL)
            {
                eap_tlv = (EAP_ParamSet_t *)ptlv_pos;
                eap_tlv->header.type = NCP_CMD_NETWORK_EAP_TLV;
                eap_tlv->header.size = sizeof(eap_tlv->anonymous_identity) + sizeof(eap_tlv->client_key_passwd);
                ptlv_pos += NCP_TLV_HEADER_LEN + eap_tlv->header.size;
                tlv_buf_len += NCP_TLV_HEADER_LEN + eap_tlv->header.size;
            }
            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid aid"
                    " argument\r\n");
                return -WM_FAIL;
            }
            /* Set Client/Server Key password */
            strcpy(eap_tlv->client_key_passwd, argv[arg + 1]);
            arg += 2;
            info.key_passwd++;
        }
#endif
        else if (!info.role && string_equal("role", argv[arg]))
        {
            role_tlv = (BSSRole_ParamSet_t *)ptlv_pos;

            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " network role\r\n");
                return -WM_FAIL;
            }

            if (strcmp(argv[arg + 1], "sta") == 0)
                role_tlv->role = WLAN_BSS_ROLE_STA;
            else if (strcmp(argv[arg + 1], "uap") == 0)
                role_tlv->role = WLAN_BSS_ROLE_UAP;
            else
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " network role\r\n");
                return -WM_FAIL;
            }

            role_tlv->header.type = NCP_CMD_NETWORK_ROLE_TLV;
            role_tlv->header.size = sizeof(role_tlv->role);
            ptlv_pos += sizeof(BSSRole_ParamSet_t);
            tlv_buf_len += sizeof(BSSRole_ParamSet_t);
            arg += 2;
            info.role++;
        }
        else if (!info.mfpc && string_equal("mfpc", argv[arg]))
        {
            if (pmf_tlv == NULL)
            {
                pmf_tlv              = (PMF_ParamSet_t *)ptlv_pos;
                pmf_tlv->header.type = NCP_CMD_NETWORK_PMF_TLV;
                pmf_tlv->header.size = sizeof(pmf_tlv->mfpc) + sizeof(pmf_tlv->mfpr);
                ptlv_pos += sizeof(PMF_ParamSet_t);
                tlv_buf_len += sizeof(PMF_ParamSet_t);
            }

            pmf_tlv->mfpc = atoi(argv[arg + 1]);
            if (arg + 1 >= argc || (pmf_tlv->mfpc != 0 && pmf_tlv->mfpc != 1))
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " network mfpc\r\n");
                return -WM_FAIL;
            }
            arg += 2;
            info.mfpc++;
        }
        else if (!info.mfpr && string_equal("mfpr", argv[arg]))
        {
            if (pmf_tlv == NULL)
            {
                pmf_tlv              = (PMF_ParamSet_t *)ptlv_pos;
                pmf_tlv->header.type = NCP_CMD_NETWORK_PMF_TLV;
                pmf_tlv->header.size = sizeof(pmf_tlv->mfpc) + sizeof(pmf_tlv->mfpr);
                ptlv_pos += sizeof(PMF_ParamSet_t);
                tlv_buf_len += sizeof(PMF_ParamSet_t);
            }

            pmf_tlv->mfpr = atoi(argv[arg + 1]);
            if (arg + 1 >= argc || (pmf_tlv->mfpr != 0 && pmf_tlv->mfpr != 1))
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " network mfpr\r\n");
                return -WM_FAIL;
            }
            arg += 2;
            info.mfpr++;
        }
        else if (!info.pwe && string_equal("pwe", argv[arg]))
        {
            pwe_tlv = (Pwe_Derivation_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF("Error: invalid pwe argument\n");
                return -WM_FAIL;
            }
            pwe_tlv->pwe_derivation     = atoi(argv[arg + 1]);
            if(pwe_tlv->pwe_derivation > 2U)
            {
                // [pwe <0/1/2>]
                (void)PRINTF("Error: invalid pwe value %u\n", pwe_tlv->pwe_derivation);
                 return -WM_FAIL;
            }
            pwe_tlv->header.type = NCP_CMD_NETWORK_PWE_TLV;
            pwe_tlv->header.size = sizeof(pwe_tlv->pwe_derivation);
            ptlv_pos += sizeof(Pwe_Derivation_ParamSet_t);
            tlv_buf_len += sizeof(Pwe_Derivation_ParamSet_t);
            arg += 2;
            info.pwe = 1;
        }
        else if (!info.tr && string_equal("tr", argv[arg]))
        {
            tr_tlv = (Tr_Disable_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF("Error: invalid tr argument\n");
                return -WM_FAIL;
            }
            tr_tlv->transition_disable     = atoi(argv[arg + 1]);
            if(tr_tlv->transition_disable > 1U)
            {   
                 // [tr <0/1>]]
                (void)PRINTF("Error: invalid tr value %u\n", tr_tlv->transition_disable);
                return -WM_FAIL;
            }
            tr_tlv->header.type = NCP_CMD_NETWORK_TR_TLV;
            tr_tlv->header.size = sizeof(tr_tlv->transition_disable);
            ptlv_pos += sizeof(Tr_Disable_ParamSet_t);
            tlv_buf_len += sizeof(Tr_Disable_ParamSet_t);
            arg += 2;
            info.tr = 1;
        }
#if CONFIG_NCP_WIFI_DTIM_PERIOD
        else if (!info.dtim && string_equal("dtim", argv[arg]))
        {
            dtim_tlv = (DTIM_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid dtim"
                    " argument\n");
                return -WM_FAIL;
            }

            dtim_tlv->dtim_period = atoi(argv[arg + 1]);
            dtim_tlv->header.type = NCP_CMD_NETWORK_DTIM_TLV;
            dtim_tlv->header.size = sizeof(dtim_tlv->dtim_period);
            ptlv_pos += sizeof(DTIM_ParamSet_t);
            tlv_buf_len += sizeof(DTIM_ParamSet_t);
            arg += 2;
            info.dtim = 1;
        }
#endif
        else if (!info.acs_band && string_equal("acs_band", argv[arg]))
        {
            acs_band_tlv = (ACSBand_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF("Error: invalid acs_band\r\n");
                return -WM_FAIL;
            }

            acs_band_tlv->acs_band = atoi(argv[arg + 1]);
            if (acs_band_tlv->acs_band != 0 && acs_band_tlv->acs_band != 1)
            {
                (void)PRINTF("Pls Set acs_band to 0 or 1.\r\n");
                (void)PRINTF(
                    "0: 2.4GHz channel   1: 5GHz channel\r\n"
                    "Not support to select dual band automatically.\r\n");
                return -WM_FAIL;
            }

            acs_band_tlv->header.type = NCP_CMD_NETWORK_ACSBAND_TLV;
            acs_band_tlv->header.size = sizeof(acs_band_tlv->acs_band);
            ptlv_pos += sizeof(ACSBand_ParamSet_t);
            tlv_buf_len += sizeof(ACSBand_ParamSet_t);
            arg += 2;
            info.acs_band = 1;
        }
#if CONFIG_NCP_WIFI_CAPA
        else if (!info.wlan_capa && role_tlv->role == WLAN_BSS_ROLE_UAP && string_equal("capa", argv[arg]))
        {
            capa_tlv = (CAPA_ParamSet_t *)ptlv_pos;
            if (arg + 1 >= argc)
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " capability\r\n");
                return -WM_FAIL;
            }

            if (strcmp(argv[arg + 1], "11ax") == 0)
                capa_tlv->capa = WIFI_SUPPORT_11AX | WIFI_SUPPORT_11AC | WIFI_SUPPORT_11N | WIFI_SUPPORT_LEGACY;
            else if (strcmp(argv[arg + 1], "11ac") == 0)
                capa_tlv->capa = WIFI_SUPPORT_11AC | WIFI_SUPPORT_11N | WIFI_SUPPORT_LEGACY;
            else if (strcmp(argv[arg + 1], "11n") == 0)
                capa_tlv->capa = WIFI_SUPPORT_11N | WIFI_SUPPORT_LEGACY;
            else if (strcmp(argv[arg + 1], "legacy") == 0)
                capa_tlv->capa = WIFI_SUPPORT_LEGACY;
            else
            {
                (void)PRINTF(
                    "Error: invalid wireless"
                    " capability\r\n");
                return -WM_FAIL;
            }

            capa_tlv->header.type = NCP_CMD_NETWORK_CAPA_TLV;
            capa_tlv->header.size = sizeof(capa_tlv->capa);
            ptlv_pos += sizeof(CAPA_ParamSet_t);
            tlv_buf_len += sizeof(CAPA_ParamSet_t);
            arg += 2;
            info.wlan_capa++;
        }
#endif
        else
        {
            dump_wlan_add_usage();
            (void)PRINTF("Error: argument %d is invalid\r\n", arg);
            return -WM_FAIL;
        }
    } while (arg < argc);

    network_add_tlv->tlv_buf_len = tlv_buf_len;

    if (!info.ssid && !info.bssid)
    {
        dump_wlan_add_usage();
        (void)PRINTF("Error: specify at least the SSID or BSSID\r\n");
        return -WM_FAIL;
    }

    if ((info.security && info.security2 && info.security3) ||
        ((security_wpa_tlv != NULL) &&
         ((security_wpa_tlv->type == WLAN_SECURITY_WPA) && info.security && !info.security2)))
    {
        dump_wlan_add_usage();
        (void)PRINTF("Error: not support WPA or WPA/WPA2/WPA3 Mixed\r\n");
        return -WM_FAIL;
    }

    if (((info.security2 == 0 && info.security3 == 0) || (info.mfpc == 0 && info.mfpr == 0)) && (info.pwe != 0))
    {
        dump_wlan_add_usage();
        (void)PRINTF("Error: pwe is only configurable for wpa2/wpa3 and pmf, pwe =%d\r\n", info.pwe);
        return -WM_FAIL;
    }
	
    if (info.security3 == 0 && info.tr != 0)
    {
        dump_wlan_add_usage();
        (void)PRINTF("Error: tr is only configurable for wpa3, tr= %d\r\n", info.tr);
        return -WM_FAIL;
    }

    network_add_command->header.cmd = NCP_CMD_WLAN_NETWORK_ADD;
    network_add_command->header.size =
        NCP_CMD_HEADER_LEN + sizeof(network_add_tlv->name) + sizeof(network_add_tlv->tlv_buf_len) + tlv_buf_len;
    network_add_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan start network command
 *
 * @return Status returned
 */
int wlan_start_network_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *network_start_command = ncp_host_get_cmd_buffer_wifi();

    int arg = 1;

    if (argc > 2)
    {
        (void)PRINTF("invalid argument\r\n");
        return -WM_FAIL;
    }

    network_start_command->header.cmd      = NCP_CMD_WLAN_NETWORK_START;
    network_start_command->header.size     = NCP_CMD_HEADER_LEN;
    network_start_command->header.result   = NCP_CMD_RESULT_OK;

    if (argc == 2)
    {
        if (strlen(argv[1]) >= WLAN_NETWORK_NAME_MAX_LENGTH)
        {
            (void)PRINTF("Error: network name too long\r\n");
            return -WM_FAIL;
        }

        NCP_CMD_NETWORK_START *network_start = (NCP_CMD_NETWORK_START *)&network_start_command->params.network_start;
        (void)memcpy(network_start->name, argv[arg],
                     (strlen(argv[arg]) > WLAN_NETWORK_NAME_MAX_LENGTH - 1) ? (WLAN_NETWORK_NAME_MAX_LENGTH - 1) :
                                                                              strlen(argv[arg]));
        network_start_command->header.size += sizeof(NCP_CMD_NETWORK_START);
    }

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan stop network command
 *
 * @return Status returned
 */
int wlan_stop_network_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *network_stop_command = ncp_host_get_cmd_buffer_wifi();
    network_stop_command->header.cmd            = NCP_CMD_WLAN_NETWORK_STOP;
    network_stop_command->header.size           = NCP_CMD_HEADER_LEN;
    network_stop_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares get wlan uap sta list command
 *
 * @return Status returned
 */
int wlan_get_uap_sta_list_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *conn_stat_command = ncp_host_get_cmd_buffer_wifi();
    conn_stat_command->header.cmd            = NCP_CMD_WLAN_NETWORK_GET_UAP_STA_LIST;
    conn_stat_command->header.size           = NCP_CMD_HEADER_LEN;
    conn_stat_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

static void dump_wlan_set_monitor_filter_usage()
{
    (void)PRINTF("Error : invalid arguments\r\n");
    (void)PRINTF("Usage : wlan-set-monitor-filter <opt> <macaddr>\r\n");
    (void)PRINTF("opt   : add/delete/clear/dump \r\n");
    (void)PRINTF("add   : All options need to be filled in \r\n");
    (void)PRINTF("delete: Delete recent mac addr \r\n");
    (void)PRINTF("clear : Clear all mac addr \r\n");
    (void)PRINTF("dump  : Dump monitor cfg information \r\n");

    (void)PRINTF("\r\nUsage example : \r\n");
    (void)PRINTF("wlan-set-monitor-filter add 64:64:4A:D6:FA:7B \r\n");
    (void)PRINTF("wlan-set-monitor-filter delete \r\n");
    (void)PRINTF("wlan-set-monitor-filter clear  \r\n");
    (void)PRINTF("wlan-set-monitor-filter dump   \r\n");
}

static void dump_monitor_param()
{
    int i = 0;

    (void)PRINTF("\r\n");
    (void)PRINTF("current parameters: \r\n");
    (void)PRINTF("action            : %d \r\n", g_net_monitor_param.action);
    (void)PRINTF("monitor_activity  : %d \r\n", g_net_monitor_param.monitor_activity);
    (void)PRINTF("filter_flags      : %d \r\n", g_net_monitor_param.filter_flags);
    (void)PRINTF("radio_type        : %d \r\n", g_net_monitor_param.radio_type);
    (void)PRINTF("chan_number       : %d \r\n", g_net_monitor_param.chan_number);
    (void)PRINTF("filter_num        : %d \r\n", g_net_monitor_param.filter_num);
    (void)PRINTF("\r\n");

    for (i = 0; i < g_net_monitor_param.filter_num; i++)
    {
        (void)PRINTF("mac_addr      : %02X:%02X:%02X:%02X:%02X:%02X \r\n", g_net_monitor_param.mac_addr[i][0],
                     g_net_monitor_param.mac_addr[i][1], g_net_monitor_param.mac_addr[i][2],
                     g_net_monitor_param.mac_addr[i][3], g_net_monitor_param.mac_addr[i][4],
                     g_net_monitor_param.mac_addr[i][5]);
    }
}

int set_monitor_filter(int op_index, uint8_t *mac)
{
    uint8_t temp_filter_num = g_net_monitor_param.filter_num;

    switch (op_index)
    {
        case MONITOR_FILTER_OPT_ADD_MAC:
            if (temp_filter_num < MAX_MONIT_MAC_FILTER_NUM)
            {
                (void)memcpy(&g_net_monitor_param.mac_addr[temp_filter_num], mac, NCP_WLAN_MAC_ADDR_LENGTH);
                g_net_monitor_param.filter_num++;
            }
            else
            {
                (void)PRINTF("Max filter num is 3 \r\n");
                return -WM_FAIL;
            }
            break;

        case MONITOR_FILTER_OPT_DELETE_MAC:
            if (temp_filter_num > 0)
            {
                memset(&g_net_monitor_param.mac_addr[temp_filter_num], 0, NCP_WLAN_MAC_ADDR_LENGTH);
                g_net_monitor_param.filter_num--;
            }
            else
            {
                (void)PRINTF("Monitor filter num is 0 \r\n");
                return -WM_FAIL;
            }
            break;

        case MONITOR_FILTER_OPT_CLEAR_MAC:
            memset(&g_net_monitor_param.mac_addr[0], 0, MAX_MONIT_MAC_FILTER_NUM * NCP_WLAN_MAC_ADDR_LENGTH);
            g_net_monitor_param.filter_num = 0;
            break;

        case MONITOR_FILTER_OPT_DUMP:
            dump_monitor_param();
            break;

        default:
            (void)PRINTF("unknown argument!\r\n");
            return -WM_FAIL;
            break;
    }

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares net monitor filter
 *
 * @return Status returned
 */
int wlan_set_monitor_filter_command(int argc, char **argv)
{
    int ret = 0;
    uint8_t raw_mac[NCP_WLAN_MAC_ADDR_LENGTH];
    int op_index = 0;

    if (3 == argc)
    {
        if (string_equal("add", argv[1]))
        {
            ret = get_mac(argv[2], (char *)raw_mac, ':');
            if (ret != 0)
            {
                (void)PRINTF("Error: invalid MAC argument\r\n");
                return -WM_FAIL;
            }
            if ((memcmp(&raw_mac[0], broadcast_mac, NCP_WLAN_MAC_ADDR_LENGTH) == 0) || (raw_mac[0] & 0x01))
            {
                (void)PRINTF("Error: only support unicast mac\r\n");
                return -WM_FAIL;
            }
            op_index = MONITOR_FILTER_OPT_ADD_MAC;
        }
        else
        {
            dump_wlan_set_monitor_filter_usage();
            return -WM_FAIL;
        }
    }
    else if (2 == argc)
    {
        if (string_equal("delete", argv[1]))
            op_index = MONITOR_FILTER_OPT_DELETE_MAC;
        else if (string_equal("clear", argv[1]))
            op_index = MONITOR_FILTER_OPT_CLEAR_MAC;
        else if (string_equal("dump", argv[1]))
            op_index = MONITOR_FILTER_OPT_DUMP;
        else
        {
            (void)PRINTF("Unknown argument!\r\n\r\n");
            dump_wlan_set_monitor_filter_usage();
            return -WM_FAIL;
        }
    }
    else
    {
        dump_wlan_set_monitor_filter_usage();
        return -WM_FAIL;
    }

    ret = set_monitor_filter(op_index, raw_mac);

    return ret;
}

/**
 * @brief  This function set net monitor cfg parameters
 *
 * @return Status returned
 */
int wlan_set_monitor_param_command(int argc, char **argv)
{
    if (argc != 6)
    {
        (void)PRINTF("Error             : invalid number of arguments\r\n");
        (void)PRINTF("Usage             : %s <action> <monitor_activity> <filter_flags> <radio_type> <chan_number>\r\n",
                     argv[0]);
        (void)PRINTF("action            : 0/1 to Action Get/Set \r\n");
        (void)PRINTF("monitor_activity  : 1 to enable and other parameters to disable monitor activity \r\n");
        (void)PRINTF("filter_flags      : network monitor fitler flag \r\n");
        (void)PRINTF("chan_number       : channel to monitor \r\n");

        (void)PRINTF("\r\nUsage example ：\r\n");
        (void)PRINTF("wlan-set-monitor-param 1 1 7 0 1 \r\n");

        dump_monitor_param();
        return -WM_FAIL;
    }

    g_net_monitor_param.action           = (uint16_t)atoi(argv[1]);
    g_net_monitor_param.monitor_activity = (uint16_t)atoi(argv[2]);

    /*
     * filter_flags:
     * bit 0: (1/0) enable/disable management frame
     * bit 1: (1/0) enable/disable control frame
     * bit 2: (1/0) enable/disable data frame
     */
    g_net_monitor_param.filter_flags = (uint16_t)atoi(argv[3]);

    /*
     * radio_type:
     * Band Info - (00)=2.4GHz, (01)=5GHz
     * t_u8  chanBand    : 2;
     * Channel Width - (00)=20MHz, (10)=40MHz, (11)=80MHz
     * t_u8  chanWidth   : 2;
     * Secondary Channel Offset - (00)=None, (01)=Above, (11)=Below
     * t_u8  chan2Offset : 2;
     * Channel Selection Mode - (00)=manual, (01)=ACS, (02)=Adoption mode
     * t_u8  scanMode    : 2;
     */
    g_net_monitor_param.radio_type  = (uint8_t)atoi(argv[4]);
    g_net_monitor_param.chan_number = (uint8_t)atoi(argv[5]);

    dump_monitor_param();

    return WM_SUCCESS;
}

/**
 * @brief  This function prepares net monitor cfg parameters
 *
 * @return Status returned
 */
int wlan_net_monitor_cfg_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *monitor_command = ncp_host_get_cmd_buffer_wifi();
    monitor_command->header.cmd            = NCP_CMD_WLAN_NETWORK_MONITOR;
    monitor_command->header.size           = NCP_CMD_HEADER_LEN;
    monitor_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_NET_MONITOR *monitor_cfg = (NCP_CMD_NET_MONITOR *)&monitor_command->params.monitor_cfg;
    memcpy(&monitor_cfg->monitor_para, &g_net_monitor_param, sizeof(g_net_monitor_param));
    monitor_command->header.size += sizeof(NCP_CMD_NET_MONITOR);

    return WM_SUCCESS;
}

static void dump_wlan_csi_filter_usage()
{
    (void)PRINTF("Error: invalid number of arguments\r\n");
    (void)PRINTF("Usage : wlan-set-csi-filter <opt> <macaddr> <pkt_type> <type> <flag>\r\n");
    (void)PRINTF("opt   : add/delete/clear/dump \r\n");
    (void)PRINTF("add   : All options need to be filled in \r\n");
    (void)PRINTF("delete: Delete recent filter information \r\n");
    (void)PRINTF("clear : Clear all filter information \r\n");
    (void)PRINTF("dump  : Dump csi cfg information \r\n");

    (void)PRINTF("\r\nUsage example : \r\n");
    (void)PRINTF("wlan-set-csi-filter add 00:18:E7:ED:2D:C1 255 255 0 \r\n");
    (void)PRINTF("wlan-set-csi-filter delete \r\n");
    (void)PRINTF("wlan-set-csi-filter clear \r\n");
    (void)PRINTF("wlan-set-csi-filter dump \r\n");
}

void dump_csi_param_header()
{
    (void)PRINTF("\r\nThe current csi_param is: \r\n");
    if(g_csi_params.bss_type == 0)
        (void)PRINTF("bss_type      : sta\r\n");
    else
        (void)PRINTF("bss_type      : uap\r\n");
    (void)PRINTF("csi_enable    : %d \r\n", g_csi_params.csi_enable);
    (void)PRINTF("head_id       : %d \r\n", g_csi_params.head_id);
    (void)PRINTF("tail_id       : %d \r\n", g_csi_params.tail_id);
    (void)PRINTF("csi_filter_cnt: %d \r\n", g_csi_params.csi_filter_cnt);
    (void)PRINTF("chip_id       : %d \r\n", g_csi_params.chip_id);
    (void)PRINTF("band_config   : %d \r\n", g_csi_params.band_config);
    (void)PRINTF("channel       : %d \r\n", g_csi_params.channel);
    (void)PRINTF("csi_monitor_enable : %d \r\n", g_csi_params.csi_monitor_enable);
    (void)PRINTF("ra4us         : %d \r\n", g_csi_params.ra4us);

    (void)PRINTF("\r\n");
}

void set_csi_param_header(uint8_t bss_type,
                          uint16_t csi_enable,
                          uint32_t head_id,
                          uint32_t tail_id,
                          uint8_t chip_id,
                          uint8_t band_config,
                          uint8_t channel,
                          uint8_t csi_monitor_enable,
                          uint8_t ra4us)
{
    g_csi_params.bss_type           = bss_type;
    g_csi_params.csi_enable         = csi_enable;
    g_csi_params.head_id            = head_id;
    g_csi_params.tail_id            = tail_id;
    g_csi_params.chip_id            = chip_id;
    g_csi_params.band_config        = band_config;
    g_csi_params.channel            = channel;
    g_csi_params.csi_monitor_enable = csi_monitor_enable;
    g_csi_params.ra4us              = ra4us;

    dump_csi_param_header();
}

int set_csi_filter(uint8_t pkt_type, uint8_t subtype, uint8_t flags, int op_index, uint8_t *mac)
{
    uint8_t temp_filter_cnt = g_csi_params.csi_filter_cnt;
    int i                   = 0;

    switch (op_index)
    {
        case CSI_FILTER_OPT_ADD:
            if (temp_filter_cnt < CSI_FILTER_MAX)
            {
                (void)memcpy(&g_csi_params.csi_filter[temp_filter_cnt].mac_addr[0], mac, NCP_WLAN_MAC_ADDR_LENGTH);
                g_csi_params.csi_filter[temp_filter_cnt].pkt_type = pkt_type;
                g_csi_params.csi_filter[temp_filter_cnt].subtype  = subtype;
                g_csi_params.csi_filter[temp_filter_cnt].flags    = flags;
                g_csi_params.csi_filter_cnt++;
            }
            else
            {
                (void)PRINTF("max csi filter cnt is 16 \r\n");
                return -WM_FAIL;
            }
            break;

        case CSI_FILTER_OPT_DELETE:
            if (temp_filter_cnt > 0)
            {
                memset(&g_csi_params.csi_filter[temp_filter_cnt], 0, sizeof(wifi_csi_filter_t));
                g_csi_params.csi_filter_cnt--;
            }
            else
            {
                (void)PRINTF("csi filter cnt is 0 \r\n");
                return -WM_FAIL;
            }
            break;

        case CSI_FILTER_OPT_CLEAR:
            for (i = 0; i < temp_filter_cnt; i++)
            {
                memset(&g_csi_params.csi_filter[i], 0, sizeof(wifi_csi_filter_t));
            }
            g_csi_params.csi_filter_cnt = 0;
            break;

        case CSI_FILTER_OPT_DUMP:
            dump_csi_param_header();

            for (i = 0; i < temp_filter_cnt; i++)
            {
                (void)PRINTF("mac_addr      : %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                             g_csi_params.csi_filter[i].mac_addr[0], g_csi_params.csi_filter[i].mac_addr[1],
                             g_csi_params.csi_filter[i].mac_addr[2], g_csi_params.csi_filter[i].mac_addr[3],
                             g_csi_params.csi_filter[i].mac_addr[4], g_csi_params.csi_filter[i].mac_addr[5]);

                (void)PRINTF("pkt_type      : %d \r\n", g_csi_params.csi_filter[i].pkt_type);
                (void)PRINTF("subtype       : %d \r\n", g_csi_params.csi_filter[i].subtype);
                (void)PRINTF("flags         : %d \r\n", g_csi_params.csi_filter[i].flags);
                (void)PRINTF("\r\n");
            }
            break;

        default:
            (void)PRINTF("unknown argument!\r\n");
            break;
    }

    return WM_SUCCESS;
}

int wlan_set_csi_param_header_command(int argc, char **argv)
{
    uint8_t bss_type           = 0;
    uint16_t csi_enable        = 0;
    uint32_t head_id           = 0;
    uint32_t tail_id           = 0;
    uint8_t chip_id            = 0;
    uint8_t band_config        = 0;
    uint8_t channel            = 0;
    uint8_t csi_monitor_enable = 0;
    uint8_t ra4us              = 0;

    if (argc != 10)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF(
            "Usage: %s <sta/uap> <csi_enable> <head_id> <tail_id> <chip_id> <band_config> <channel> <csi_monitor_enable> "
            "<ra4us>\r\n\r\n",
            argv[0]);

        (void)PRINTF("[csi_enable] :1/2 to Enable/Disable CSI\r\n");
        (void)PRINTF("[head_id, head_id, chip_id] are used to seperate CSI event records received from FW\r\n");
        (void)PRINTF(
            "[Bandcfg] defined as below: \r\n"
            "    Band Info - (00)=2.4GHz, (01)=5GHz \r\n"
            "    uint8_t  chanBand    : 2;\r\n"
            "    Channel Width - (00)=20MHz, (10)=40MHz, (11)=80MHz\r\n"
            "    uint8_t  chanWidth   : 2;\r\n"
            "    Secondary Channel Offset - (00)=None, (01)=Above, (11)=Below\r\n"
            "    uint8_t  chan2Offset : 2;\r\n"
            "    Channel Selection Mode - (00)=manual, (01)=ACS, (02)=Adoption mode\r\n"
            "    uint8_t  scanMode    : 2;\r\n");
        (void)PRINTF("[channel] : monitor channel number\r\n");
        (void)PRINTF("[csi_monitor_enable] : 1-csi_monitor enable, 0-MAC filter enable\r\n");
        (void)PRINTF(
            "[ra4us] : 1/0 to Enable/Disable CSI data received in cfg channel with mac addr filter, not only RA is "
            "us or other\r\n");

        (void)PRINTF("\r\nUsage example : \r\n");
        (void)PRINTF("wlan-set-csi-param-header sta 1 66051 66051 170 0 11 1 1\r\n");

        dump_csi_param_header();

        return -WM_FAIL;
    }

    /*
     * csi param header headid, tailid, chipid are used to seperate CSI event records received from FW.
     * FW adds user configured headid, chipid and tailid for each CSI event record.
     * User could configure these fields and used these fields to parse CSI event buffer and do verification.
     * All the CSI filters share the same CSI param header.
     */
    if (string_equal("sta", argv[1]))
        bss_type = 0;
    else if (string_equal("uap", argv[1]))
        bss_type = 1;
    else
    {
        PRINTF("Please put sta or uap\r\n");
        return -WM_FAIL;
    }
    csi_enable         = (uint16_t)atoi(argv[2]);
    head_id            = (uint32_t)atoi(argv[3]);
    tail_id            = (uint32_t)atoi(argv[4]);
    chip_id            = (uint8_t)atoi(argv[5]);
    band_config        = (uint8_t)atoi(argv[6]);
    channel            = (uint8_t)atoi(argv[7]);
    csi_monitor_enable = (uint8_t)atoi(argv[8]);
    ra4us              = (uint8_t)atoi(argv[9]);

    set_csi_param_header(bss_type, csi_enable, head_id, tail_id, chip_id, band_config, channel, csi_monitor_enable, ra4us);

    return WM_SUCCESS;
}

int wlan_set_csi_filter_command(int argc, char **argv)
{
    int ret = WM_SUCCESS;
    uint8_t raw_mac[NCP_WLAN_MAC_ADDR_LENGTH];
    uint8_t pkt_type = 0;
    uint8_t subtype  = 0;
    uint8_t flags    = 0;
    int op_index     = 0;

    if (argc < 2)
    {
        dump_wlan_csi_filter_usage();
        return -WM_FAIL;
    }

    if (string_equal("add", argv[1]))
    {
        if (6 == argc)
        {
            ret = get_mac(argv[2], (char *)raw_mac, ':');
            if (ret != 0)
            {
                (void)PRINTF("Error: invalid MAC argument\r\n");
                return -WM_FAIL;
            }
            if ((memcmp(&raw_mac[0], broadcast_mac, NCP_WLAN_MAC_ADDR_LENGTH) == 0) || (raw_mac[0] & 0x01))
            {
                (void)PRINTF("Error: only support unicast mac\r\n");
                return -WM_FAIL;
            }

            /*
             * pkt_type and subtype are the 802.11 framecontrol pkttype and subtype
             * flags:
             * bit0 reserved, must be 0
             * bit1 set to 1: wait for trigger
             * bit2 set to 1: send csi error event when timeout
             */
            pkt_type = (uint8_t)atoi(argv[3]);
            subtype  = (uint8_t)atoi(argv[4]);
            flags    = (uint8_t)atoi(argv[5]);

            op_index = CSI_FILTER_OPT_ADD;
        }
        else
        {
            dump_wlan_csi_filter_usage();
            return -WM_FAIL;
        }
    }
    else if (string_equal("delete", argv[1]))
        op_index = CSI_FILTER_OPT_DELETE;
    else if (string_equal("clear", argv[1]))
        op_index = CSI_FILTER_OPT_CLEAR;
    else if (string_equal("dump", argv[1]))
        op_index = CSI_FILTER_OPT_DUMP;
    else
    {
        (void)PRINTF("Unknown argument!\r\n");
        return -WM_FAIL;
    }

    ret = set_csi_filter(pkt_type, subtype, flags, op_index, raw_mac);

    return ret;
}

int wlan_csi_cfg_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *csi_command = ncp_host_get_cmd_buffer_wifi();
    csi_command->header.cmd            = NCP_CMD_WLAN_STA_CSI;
    csi_command->header.size           = NCP_CMD_HEADER_LEN;
    csi_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_CSI *csi_cfg = (NCP_CMD_CSI *)&csi_command->params.csi_cfg;
    memcpy(&csi_cfg->csi_para, &g_csi_params, sizeof(g_csi_params));
    csi_command->header.size += sizeof(NCP_CMD_CSI);

    return WM_SUCCESS;
}

int wlan_11k_cfg_command(int argc, char **argv)
{
    int enable_11k;

    if (argc != 2)
    {
        PRINTF("Usage: %s <0/1> < 0--disable 11k; 1---enable 11k>\r\n", argv[0]);
        return -WM_FAIL;
    }

    enable_11k = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *wlan_11k_cfg_command = ncp_host_get_cmd_buffer_wifi();
    wlan_11k_cfg_command->header.cmd            = NCP_CMD_WLAN_STA_11K_CFG;
    wlan_11k_cfg_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_11k_cfg_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_11K_CFG *wlan_11k_cfg = (NCP_CMD_11K_CFG *)&wlan_11k_cfg_command->params.wlan_11k_cfg;
    wlan_11k_cfg->enable          = enable_11k;
    wlan_11k_cfg_command->header.size += sizeof(NCP_CMD_11K_CFG);

    return WM_SUCCESS;
}

int wlan_11k_neighbor_req_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *neighbor_req_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)neighbor_req_command, 0, NCP_HOST_COMMAND_LEN);
    neighbor_req_command->header.cmd      = NCP_CMD_WLAN_STA_NEIGHBOR_REQ;
    neighbor_req_command->header.size     = NCP_CMD_HEADER_LEN;
    neighbor_req_command->header.result   = NCP_CMD_RESULT_OK;

    if ((argc != 1 && argc != 3) || (argc == 3 && !string_equal("ssid", argv[1])))
    {
        PRINTF("Usage: %s\r\n", argv[0]);
        PRINTF("or     %s ssid <ssid>\r\n", argv[0]);
        return -WM_FAIL;
    }

    if (argc == 1)
    {
        return WM_SUCCESS;
    }
    else if (argc == 3)
    {
        if (strlen(argv[2]) > 32)
        {
            PRINTF("Error: ssid too long\r\n");
            return -WM_FAIL;
        }
        else
        {
            NCP_CMD_NEIGHBOR_REQ *neighbor_req = (NCP_CMD_NEIGHBOR_REQ *)&neighbor_req_command->params.neighbor_req;
            neighbor_req->ssid_tlv.header.type = NCP_CMD_NETWORK_SSID_TLV;
            neighbor_req->ssid_tlv.header.size = strlen(argv[2]);

            neighbor_req_command->header.size += strlen(argv[2]) + NCP_TLV_HEADER_LEN;
            (void)memcpy(neighbor_req->ssid_tlv.ssid, argv[2], strlen(argv[2]));
        }
    }

    return WM_SUCCESS;
}

int wlan_process_wlan_reset_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to config wlan-reset\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("wlan-reset succeeded!\r\n");
    return WM_SUCCESS;
}

/**
 * @brief       This function processes connect response from ncp device
 *
 * @param res   A pointer to uint8_t
 * @return      Status returned
 */
int wlan_process_con_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    struct in_addr ip;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        PRINTF("Failed to get correct AP info!\r\n");
        PRINTF("Please input 'wlan-connect' to connect an AP or wait a few moments for the AP information.\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_WLAN_CONN *connect_res_tlv = (NCP_CMD_WLAN_CONN *)&cmd_res->params.wlan_connect;
    ip.s_addr                          = connect_res_tlv->ip;
    PRINTF("STA connected:\r\n");
    PRINTF("SSID = [%s]\r\n", connect_res_tlv->ssid);
    PRINTF("IPv4 Address: [%s]\r\n", inet_ntoa(ip));

    return WM_SUCCESS;
}

/**
 * @brief      This function processes disconnect response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_discon_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("STA disconnect.\r\n");
    else
        (void)PRINTF("Failed to disconnect to network.\r\n");

    return WM_SUCCESS;
}

/**
 * @brief      This function processes scan response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_scan_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        NCP_CMD_SCAN_NETWORK_INFO *scan_res_tlv = (NCP_CMD_SCAN_NETWORK_INFO *)&cmd_res->params.scan_network_info;
        uint8_t count                           = scan_res_tlv->res_cnt;
        if (count == 0)
        {
            (void)PRINTF("no networks found\r\n");
        }
        else
        {
            PRINTF("%d networks found\r\n", count);
            for (int i = 0; i < count; i++)
            {
                print_mac(scan_res_tlv->res[i].bssid);
                (void)PRINTF(" [%s]\r\n", scan_res_tlv->res[i].ssid);
                (void)PRINTF("mode: ");
#if CONFIG_NCP_11AX
                if (scan_res_tlv->res[i].dot11ax != 0U)
                {
                    (void)PRINTF("802.11AX ");
                }
                else
#endif
#if CONFIG_NCP_11AC
                if (scan_res_tlv->res[i].dot11ac != 0U)
                {
                    (void)PRINTF("802.11AC ");
                }
                else
#endif
                if (scan_res_tlv->res[i].dot11n != 0U)
                {
                    (void)PRINTF("802.11N ");
                }
                else
                {
                    (void)PRINTF("802.11BG ");
                }
                (void)PRINTF("\r\n");
                (void)PRINTF("channel: %d\r\n", scan_res_tlv->res[i].channel);
                (void)PRINTF("rssi: -%d dBm\r\n", scan_res_tlv->res[i].rssi);

                (void)PRINTF("security: ");
                if (scan_res_tlv->res[i].wep != 0U)
                    (void)PRINTF("WEP ");
                if (scan_res_tlv->res[i].wpa && scan_res_tlv->res[i].wpa2)
                    (void)PRINTF("WPA/WPA2 Mixed ");
                else if (scan_res_tlv->res[i].wpa2 && scan_res_tlv->res[i].wpa3_sae)
                    (void)PRINTF("WPA2/WPA3 SAE Mixed ");
                else
                {
                    if (scan_res_tlv->res[i].wpa != 0U)
                        (void)PRINTF("WPA ");
                    if (scan_res_tlv->res[i].wpa2 != 0U)
                        (void)PRINTF("WPA2 ");
                    if (scan_res_tlv->res[i].wpa3_sae != 0U)
                        (void)PRINTF("WPA3 SAE ");
                    if (scan_res_tlv->res[i].wpa2_entp != 0U)
                        (void)PRINTF("WPA2 Enterprise");
                }
                if (!(scan_res_tlv->res[i].wep || scan_res_tlv->res[i].wpa || scan_res_tlv->res[i].wpa2 ||
                      scan_res_tlv->res[i].wpa3_sae || scan_res_tlv->res[i].wpa2_entp))
                {
                    (void)PRINTF("OPEN ");
                }
                (void)PRINTF("\r\n");
                (void)PRINTF("WMM: %s\r\n", scan_res_tlv->res[i].wmm ? "YES" : "NO");
            }
        }
    }
    else
    {
        (void)PRINTF("Failed to scan.\r\n");
    }
    (void)PRINTF("\r\n");
    return WM_SUCCESS;
}

int wlan_get_signal_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *get_signal_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)get_signal_command, 0, NCP_HOST_COMMAND_LEN);

    get_signal_command->header.cmd      = NCP_CMD_WLAN_STA_SIGNAL;
    get_signal_command->header.size     = NCP_CMD_HEADER_LEN;
    get_signal_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_process_rssi_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        PRINTF("Cann't get RSSI information in disconnect state.\r\n");
        return WM_SUCCESS;
    }

    NCP_CMD_RSSI *signal_rssi = (NCP_CMD_RSSI *)&cmd_res->params.signal_rssi;
    (void)PRINTF("\tBeaconLast\tBeacon Average\tData Last\tData Average\n");
    (void)PRINTF("RSSI\t%-10d \t%-10d \t%-10d \t%-10d\n", (int)signal_rssi->rssi_info.bcn_rssi_last,
                 (int)signal_rssi->rssi_info.bcn_rssi_avg, (int)signal_rssi->rssi_info.data_rssi_last,
                 (int)signal_rssi->rssi_info.data_rssi_avg);
    (void)PRINTF("SNR \t%-10d \t%-10d \t%-10d \t%-10d\n", (int)signal_rssi->rssi_info.bcn_snr_last,
                 (int)signal_rssi->rssi_info.bcn_snr_avg, (int)signal_rssi->rssi_info.data_snr_last,
                 (int)signal_rssi->rssi_info.data_snr_avg);
    (void)PRINTF("NF  \t%-10d \t%-10d \t%-10d \t%-10d\n", (int)signal_rssi->rssi_info.bcn_nf_last,
                 (int)signal_rssi->rssi_info.bcn_nf_avg, (int)signal_rssi->rssi_info.data_nf_last,
                 (int)signal_rssi->rssi_info.data_nf_avg);
    (void)PRINTF("\r\n");

    return WM_SUCCESS;
}

/**
 * @brief      This function processes fw version response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_version_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        NCP_CMD_FW_VERSION *fw_ver = (NCP_CMD_FW_VERSION *)&cmd_res->params.fw_version;
        (void)PRINTF("WLAN Driver Version   :%s \r\n", fw_ver->driver_ver_str);
        (void)PRINTF("WLAN Firmware Version :%s \r\n", fw_ver->fw_ver_str);
    }
    else
    {
        (void)PRINTF("failed to get firmware version\r\n");
    }

    return WM_SUCCESS;
}

/**
 * @brief      This function processes roaming response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_roaming_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("Set roaming successfully!\r\n");
    else
        (void)PRINTF("Failed to set roaming!\r\n");
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan connection state response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_stat_response(uint8_t *res)
{
    char ps_mode_str[25];
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to get wlan connection state\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_CONNECT_STAT *conn_stat = (NCP_CMD_CONNECT_STAT *)&cmd_res->params.conn_stat;

    switch (conn_stat->ps_mode)
    {
        case WLAN_IEEE:
            strcpy(ps_mode_str, "IEEE ps");
            break;
        case WLAN_DEEP_SLEEP:
            strcpy(ps_mode_str, "Deep sleep");
            break;
        case WLAN_IEEE_DEEP_SLEEP:
            strcpy(ps_mode_str, "IEEE ps and Deep sleep");
            break;
        case WLAN_WNM:
            strcpy(ps_mode_str, "WNM ps");
            break;
        case WLAN_WNM_DEEP_SLEEP:
            strcpy(ps_mode_str, "WNM ps and Deep sleep");
            break;
        case WLAN_ACTIVE:
        default:
            strcpy(ps_mode_str, "Active");
            break;
    }

    switch (conn_stat->sta_conn_stat)
    {
        case WLAN_DISCONNECTED:
            (void)PRINTF("Station disconnected (%s)\r\n", ps_mode_str);
            break;
        case WLAN_SCANNING:
            (void)PRINTF("Station scanning (%s)\r\n", ps_mode_str);
            break;
        case WLAN_ASSOCIATING:
            (void)PRINTF("Station associating (%s)\r\n", ps_mode_str);
            break;
        case WLAN_ASSOCIATED:
            (void)PRINTF("Station associated (%s)\r\n", ps_mode_str);
            break;
        case WLAN_CONNECTING:
            (void)PRINTF("Station connecting (%s)\r\n", ps_mode_str);
            break;
        case WLAN_CONNECTED:
            (void)PRINTF("Station connected (%s)\r\n", ps_mode_str);
            break;
        default:
            (void)PRINTF(
                "Error: invalid STA state"
                " %d\r\n",
                conn_stat->sta_conn_stat);
            break;
    }

    switch (conn_stat->uap_conn_stat)
    {
        case WLAN_UAP_STARTED:
            strcpy(ps_mode_str, "Active");
            (void)PRINTF("uAP started (%s)\r\n", ps_mode_str);
            break;
        case WLAN_UAP_STOPPED:
            (void)PRINTF("uAP stopped\r\n");
            break;
        default:
            (void)PRINTF(
                "Error: invalid uAP state"
                " %d\r\n",
                conn_stat->uap_conn_stat);
            break;
    }

    return WM_SUCCESS;
}

/*WLAN HTTP commamd*/
/**
 * @brief      This function processes wlan http connect from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_http_con_response(uint8_t *res)
{
    int handle                     = -1;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to connect!\r\n");
        return -WM_FAIL;
    }
    else
    {
        NCP_CMD_HTTP_CON_CFG *wlan_http_connect = (NCP_CMD_HTTP_CON_CFG *)&cmd_res->params.wlan_http_connect;
        handle                                  = wlan_http_connect->opened_handle;
        (void)PRINTF("Handle: %d\n", handle);
        return WM_SUCCESS;
    }
}

/**
 * @brief  This function prepares wlan http connect command
 *
 * @return Status returned
 */
int wlan_http_connect_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_CON;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s host\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_CON_CFG *wlan_http_tlv = (NCP_CMD_HTTP_CON_CFG *)&wlan_http_command->params.wlan_http_connect;
    if (strlen(argv[1]) + 1 > HTTP_URI_LEN)
        return -WM_FAIL;
    memcpy(wlan_http_tlv->host, argv[1], strlen(argv[1]) + 1);
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_CON_CFG);
    wlan_http_command->header.size += strlen(wlan_http_tlv->host) + 1;
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan http disconnect from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_http_discon_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to disconnect!\r\n");
        return -WM_FAIL;
    }
    else
    {
        (void)PRINTF("disconnect success!\r\n");
        return WM_SUCCESS;
    }
}

/**
 * @brief  This function prepares wlan http connect command
 *
 * @return Status returned
 */
int wlan_http_disconnect_command(int argc, char **argv)
{
    unsigned int handle = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_DISCON;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_DISCON_CFG *wlan_http_tlv = (NCP_CMD_HTTP_DISCON_CFG *)&wlan_http_command->params.wlan_http_disconnect;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_DISCON_CFG);
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan http req from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
typedef struct
{
    char *name;
    char *value;
} http_header_pair_t;
int wlan_process_wlan_http_req_response(uint8_t *res)
{
    unsigned int header_size       = 0;
    char *recv_header              = 0;
    int header_count               = 0;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to req!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_HTTP_REQ_RESP_CFG *wlan_http_req_resp = (NCP_CMD_HTTP_REQ_RESP_CFG *)&cmd_res->params.wlan_http_req_resp;
    header_size                                   = wlan_http_req_resp->header_size;
    ncp_dump_hex(wlan_http_req_resp->recv_header, header_size);
    recv_header = wlan_http_req_resp->recv_header;
    while (strlen(recv_header))
    {
        header_count++;
        http_header_pair_t header_pair;
        header_pair.name = recv_header;
        recv_header += (strlen(recv_header) + 1);
        header_pair.value = recv_header;
        recv_header += (strlen(recv_header) + 1);
        (void)PRINTF("%s:%s\n", header_pair.name, header_pair.value);
    }
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan http req command
 *
 * @return Status returned
 */
int wlan_http_req_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned req_size   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)wlan_http_command, 0, NCP_HOST_COMMAND_LEN);
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_REQ;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc < 3 || argc > 6)
    {
        (void)PRINTF("Usage: %s handle method [uri] [req_data] [req_size]\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_REQ_CFG *wlan_http_tlv = (NCP_CMD_HTTP_REQ_CFG *)&wlan_http_command->params.wlan_http_req;

    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle method [uri] [req_data] [req_size]\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    if (strlen(argv[2]) + 1 > HTTP_PARA_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }

    memcpy(wlan_http_tlv->method, argv[2], strlen(argv[2]) + 1);
    if (argv[3])
    {
        if (strlen(argv[3]) + 1 > HTTP_URI_LEN)
        {
            (void)PRINTF("over buffer size\r\n");
            return -WM_FAIL;
        }
        memcpy(wlan_http_tlv->uri, argv[3], strlen(argv[3]) + 1);
    }

    if (argv[4])
    {
        if (!argv[5])
            wlan_http_tlv->req_size = strlen(argv[4]) + 1;
        else
        {
            if (get_uint(argv[5], &req_size, strlen(argv[5])))
            {
                (void)PRINTF("Usage: %s handle method [uri] [req_data] [req_size]\r\n", __func__);
                return -WM_FAIL;
            }
            wlan_http_tlv->req_size = req_size;
        }
        wlan_http_command->header.size += wlan_http_tlv->req_size;
    }
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_REQ_CFG);
    if (argv[4])
    {
        if (wlan_http_command->header.size > NCP_HOST_COMMAND_LEN)
        {
            (void)PRINTF("over buffer size\r\n");
            return -WM_FAIL;
        }
        memcpy(wlan_http_tlv->req_data, argv[4], wlan_http_tlv->req_size);
    }
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan http recv from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_http_recv_response(uint8_t *res)
{
    unsigned int recv_size         = 0;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to receive data!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_HTTP_RECV_CFG *wlan_http_receive = (NCP_CMD_HTTP_RECV_CFG *)&cmd_res->params.wlan_http_recv;
    recv_size                                = wlan_http_receive->recv_size;
    ncp_dump_hex(wlan_http_receive->recv_data, recv_size);
    (void)PRINTF("receive data success, %s\r\n", wlan_http_receive->recv_data);
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan http recv command
 *
 * @return Status returned
 */
int wlan_http_recv_command(int argc, char **argv)
{
    unsigned int handle  = 0;
    unsigned int size    = 0;
    unsigned int timeout = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_RECV;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_RECV_CFG *wlan_http_tlv = (NCP_CMD_HTTP_RECV_CFG *)&wlan_http_command->params.wlan_http_recv;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    if (get_uint(argv[2], &size, strlen(argv[2])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->recv_size = size;
    if (get_uint(argv[3], &timeout, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->timeout = timeout;

    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_RECV_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan http seth from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_http_unseth_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to remove http header-name!\r\n");
        return -WM_FAIL;
    }
    (void)PRINTF("success to remove http header-name\n");
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan http seth command
 *
 * @return Status returned
 */
int wlan_http_unseth_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_UNSETH;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s header-name\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_UNSETH_CFG *wlan_http_tlv = (NCP_CMD_HTTP_UNSETH_CFG *)&wlan_http_command->params.wlan_http_unseth;
    if (strlen(argv[1]) + 1 > SETH_NAME_LENGTH)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }

    memcpy(wlan_http_tlv->name, argv[1], strlen(argv[1]) + 1);
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_UNSETH_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan http unseth from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_http_seth_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to set http header-name!\r\n");
        return -WM_FAIL;
    }
    (void)PRINTF("success to set http header-name\n");
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan http unseth command
 *
 * @return Status returned
 */
int wlan_http_seth_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_HTTP_SETH;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 3)
    {
        (void)PRINTF("Usage: %s header-name header-value\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_SETH_CFG *wlan_http_tlv = (NCP_CMD_HTTP_SETH_CFG *)&wlan_http_command->params.wlan_http_seth;
    if (strlen(argv[1]) + 1 > SETH_NAME_LENGTH || strlen(argv[2]) + 1 > SETH_VALUE_LENGTH)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }

    memcpy(wlan_http_tlv->name, argv[1], strlen(argv[1]) + 1);
    memcpy(wlan_http_tlv->value, argv[2], strlen(argv[2]) + 1);
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_SETH_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan websocket upgrade from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_websocket_upg_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to http upgrade!\r\n");
        return -WM_FAIL;
    }
    (void)PRINTF("success to http upgrade\n");
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan websocket upgrade command
 *
 * @return Status returned
 */
int wlan_websocket_upg_command(int argc, char **argv)
{
    unsigned int handle = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_WEBSOCKET_UPG;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_HTTP_UPG_CFG *wlan_http_tlv = (NCP_CMD_HTTP_UPG_CFG *)&wlan_http_command->params.wlan_http_upg;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    if (strlen(argv[2]) + 1 > HTTP_URI_LEN || strlen(argv[3]) + 1 > HTTP_PARA_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_http_tlv->uri, argv[2], strlen(argv[2]) + 1);
    memcpy(wlan_http_tlv->protocol, argv[3], strlen(argv[3]) + 1);
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_HTTP_UPG_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan websocket send from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_websocket_send_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to send data!\r\n");
        return -WM_FAIL;
    }
    (void)PRINTF("send data success!\r\n");
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan websocket send command
 *
 * @return Status returned
 */
int wlan_websocket_send_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int size   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_WEBSOCKET_SEND;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4 && argc != 5)
    {
        (void)PRINTF("Usage: %s handle type send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }

    if (!argv[3])
        return -WM_FAIL;

    NCP_CMD_WEBSOCKET_SEND_CFG *wlan_http_tlv =
        (NCP_CMD_WEBSOCKET_SEND_CFG *)&wlan_http_command->params.wlan_websocket_send;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle type send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    if (strlen(argv[2]) + 1 > HTTP_PARA_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_http_tlv->type, argv[2], strlen(argv[2]) + 1);
    if (!argv[4])
        wlan_http_tlv->size = strlen(argv[3]) + 1;
    else
    {
        if (get_uint(argv[4], &size, strlen(argv[4])))
        {
            (void)PRINTF("Usage: %s handle type send_data [send_size]\r\n", __func__);
            return -WM_FAIL;
        }
        wlan_http_tlv->size = size;
    }
    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_WEBSOCKET_SEND_CFG);
    wlan_http_command->header.size += wlan_http_tlv->size;
    if (wlan_http_command->header.size > NCP_HOST_COMMAND_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_http_tlv->send_data, argv[3], wlan_http_tlv->size);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan websocket recv from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_websocket_recv_response(uint8_t *res)
{
    unsigned int recv_size         = 0;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to receive data!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_WEBSOCKET_RECV_CFG *wlan_websocket_receive =
        (NCP_CMD_WEBSOCKET_RECV_CFG *)&cmd_res->params.wlan_websocket_recv;

    recv_size = wlan_websocket_receive->recv_size;
    ncp_dump_hex(wlan_websocket_receive->recv_data, recv_size);
    (void)PRINTF("receive data success, %s, fin = %d\r\n", wlan_websocket_receive->recv_data,
                 wlan_websocket_receive->fin);
    return WM_SUCCESS;
}

/**
 * @brief  This function prepares wlan websocket recv command
 *
 * @return Status returned
 */
int wlan_websocket_recv_command(int argc, char **argv)
{
    unsigned int handle  = 0;
    unsigned int size    = 0;
    unsigned int timeout = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_http_command = ncp_host_get_cmd_buffer_wifi();
    wlan_http_command->header.cmd            = NCP_CMD_WLAN_WEBSOCKET_RECV;
    wlan_http_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_http_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_WEBSOCKET_RECV_CFG *wlan_http_tlv =
        (NCP_CMD_WEBSOCKET_RECV_CFG *)&wlan_http_command->params.wlan_websocket_recv;

    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->handle = handle;
    if (get_uint(argv[2], &size, strlen(argv[2])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->recv_size = size;
    if (get_uint(argv[3], &timeout, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_http_tlv->timeout = timeout;

    /*cmd size*/
    wlan_http_command->header.size += sizeof(NCP_CMD_WEBSOCKET_RECV_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket open response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_open_response(uint8_t *res)
{
    int handle                     = -1;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to open!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_SOCKET_OPEN_CFG *wlan_socket_open = (NCP_CMD_SOCKET_OPEN_CFG *)&cmd_res->params.wlan_socket_open;
    handle                                    = wlan_socket_open->opened_handle;
    if (!strcmp(wlan_socket_open->protocol, "icmp"))
        ping_sock_handle = handle;
    (void)PRINTF("Handle: %d\n", handle);
    return WM_SUCCESS;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket open command
 *
 * @return Status returned
 */
int wlan_socket_open_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_OPEN;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc < 2 || argc > 4)
    {
        (void)PRINTF("Usage: %s tcp/udp/raw [domain] [protocol]\r\n", __func__);
        return -WM_FAIL;
    }

    if (!strcmp(argv[1], "tcp") && !strcmp(argv[1], "udp") && !strcmp(argv[1], "raw"))
    {
        (void)PRINTF("Usage: %s tcp/udp/raw [domain] [protocol]\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_OPEN_CFG *wlan_socket_tlv = (NCP_CMD_SOCKET_OPEN_CFG *)&wlan_socket_command->params.wlan_socket_open;
    if (strlen(argv[1]) + 1 > HTTP_PARA_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->socket_type, argv[1], HTTP_PARA_LEN);
    if (argv[2])
        memcpy(wlan_socket_tlv->domain_type, argv[2], sizeof(wlan_socket_tlv->domain_type));
    else
        memset(wlan_socket_tlv->domain_type, '\0', sizeof(wlan_socket_tlv->domain_type));
    if (argv[3])
        memcpy(wlan_socket_tlv->protocol, argv[3], sizeof(wlan_socket_tlv->protocol));
    else
        memset(wlan_socket_tlv->protocol, '\0', sizeof(wlan_socket_tlv->protocol));

    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_OPEN_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket connect from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_con_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to connect!\r\n");
        return -WM_FAIL;
    }
    else
    {
        (void)PRINTF("connect success!\r\n");
        return WM_SUCCESS;
    }
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket open command
 *
 * @return Status returned
 */
int wlan_socket_con_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int port   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_CON;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_CON_CFG *wlan_socket_tlv = (NCP_CMD_SOCKET_CON_CFG *)&wlan_socket_command->params.wlan_socket_con;

    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[3], &port, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->port = port;
    if (strlen(argv[2]) + 1 > IP_ADDR_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->ip_addr, argv[2], strlen(argv[2]) + 1);
    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_CON_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket bind from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_bind_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to bind!\r\n");
        return -WM_FAIL;
    }
    else
    {
        (void)PRINTF("bind success!\r\n");
        return WM_SUCCESS;
    }
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket bind command
 *
 * @return Status returned
 */
int wlan_socket_bind_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int port   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_BIND;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_BIND_CFG *wlan_socket_tlv = (NCP_CMD_SOCKET_BIND_CFG *)&wlan_socket_command->params.wlan_socket_bind;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[3], &port, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->port = port;
    if (strlen(argv[2]) + 1 > IP_ADDR_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->ip_addr, argv[2], strlen(argv[2]) + 1);
    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_BIND_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket close from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_close_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to close handle!\r\n");
        return -WM_FAIL;
    }
    else
    {
        (void)PRINTF("close handle success!\r\n");
        return WM_SUCCESS;
    }
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket close command
 *
 * @return Status returned
 */
int wlan_socket_close_command(int argc, char **argv)
{
    unsigned int handle = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_CLOSE;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_CLOSE_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_CLOSE_CFG *)&wlan_socket_command->params.wlan_socket_close;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;

    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_CLOSE_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket listen from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_listen_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to listen handle!\r\n");
        return -WM_FAIL;
    }
    else
    {
        (void)PRINTF("listen handle success!\r\n");
        return WM_SUCCESS;
    }
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket listen command
 *
 * @return Status returned
 */
int wlan_socket_listen_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int number = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_LISTEN;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 3)
    {
        (void)PRINTF("Usage: %s handle number\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_LISTEN_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_LISTEN_CFG *)&wlan_socket_command->params.wlan_socket_listen;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle number\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[2], &number, strlen(argv[2])))
    {
        (void)PRINTF("Usage: %s handle number\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->number = number;

    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_LISTEN_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket accept from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_accept_response(uint8_t *res)
{
    int handle                     = -1;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to accept handle!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_SOCKET_ACCEPT_CFG *wlan_socket_accept = (NCP_CMD_SOCKET_ACCEPT_CFG *)&cmd_res->params.wlan_socket_accept;
    handle                                        = wlan_socket_accept->accepted_handle;
    (void)PRINTF("accept handle %d!\r\n", handle);
    return WM_SUCCESS;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket close command
 *
 * @return Status returned
 */
int wlan_socket_accept_command(int argc, char **argv)
{
    unsigned int handle = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_ACCEPT;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_ACCEPT_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_ACCEPT_CFG *)&wlan_socket_command->params.wlan_socket_accept;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_ACCEPT_CFG);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket send from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_send_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to send data!\r\n");
        return -WM_FAIL;
    }

    return WM_SUCCESS;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket send command
 *
 * @return Status returned
 */
int wlan_socket_send_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int size   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SEND;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 3 && argc != 4)
    {
        (void)PRINTF("Usage: %s handle send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }

    if (!argv[2])
        return -WM_FAIL;

    NCP_CMD_SOCKET_SEND_CFG *wlan_socket_tlv = (NCP_CMD_SOCKET_SEND_CFG *)&wlan_socket_command->params.wlan_socket_send;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;

    if (!argv[3])
        wlan_socket_tlv->size = (strlen(argv[2]) + 1);
    else
    {
        if (get_uint(argv[3], &size, strlen(argv[3])))
        {
            (void)PRINTF("Usage: %s handle send_data [send_size]\r\n", __func__);
            return -WM_FAIL;
        }
        wlan_socket_tlv->size = size;
    }
    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_SEND_CFG);
    wlan_socket_command->header.size += wlan_socket_tlv->size;
    if (wlan_socket_command->header.size > NCP_HOST_COMMAND_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->send_data, argv[2], wlan_socket_tlv->size);
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket sendto from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_sendto_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        if (ping_res.seq_no < 0)
        {
            (void)PRINTF("failed to sendto data!\r\n");
        }
        else
        {
            /* Send ping cmd response to ping_sock_task */
            ping_set_event(PING_EVENTS_SENDTO_RESP);
        }
        return -WM_FAIL;
    }

    if (ping_res.seq_no >= 0)
    {
        /* Send ping cmd response to ping_sock_task */
        ping_set_event(PING_EVENTS_SENDTO_RESP);
    }

    return WM_SUCCESS;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket send command
 *
 * @return Status returned
 */
int wlan_socket_sendto_command(int argc, char **argv)
{
    unsigned int handle = 0;
    unsigned int size   = 0;
    unsigned int port   = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_SENDTO;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 5 && argc != 6)
    {
        (void)PRINTF("Usage: %s handle ip_addr port send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_SENDTO_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_SENDTO_CFG *)&wlan_socket_command->params.wlan_socket_sendto;

    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[3], &port, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle ip_addr port send_data [send_size]\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->port = port;

    if (!argv[5])
        wlan_socket_tlv->size = strlen(argv[4]) + 1;
    else
    {
        if (get_uint(argv[5], &size, strlen(argv[5])))
        {
            (void)PRINTF("Usage: %s handle ip_addr port send_data [send_size]\r\n", __func__);
            return -WM_FAIL;
        }
        wlan_socket_tlv->size = size;
    }
    if (strlen(argv[2]) + 1 > IP_ADDR_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->ip_addr, argv[2], strlen(argv[2]) + 1);
    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_SENDTO_CFG);
    wlan_socket_command->header.size += wlan_socket_tlv->size;
    if (wlan_socket_command->header.size > NCP_HOST_COMMAND_LEN)
    {
        (void)PRINTF("over buffer size\r\n");
        return -WM_FAIL;
    }
    memcpy(wlan_socket_tlv->send_data, argv[4], wlan_socket_tlv->size);
    return WM_SUCCESS;
}
extern char lwiperf_end_token[NCP_IPERF_END_TOKEN_SIZE]; 
/**
 * @brief      This function processes wlan socket receive from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_receive_response(uint8_t *res)
{
    unsigned int recv_size         = 0;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        if (socket_receive_res == true)
        {
            socket_receive_res = false;
        }
        (void)PRINTF("failed to receive data!\r\n");
        return -WM_FAIL;
    }
    NCP_CMD_SOCKET_RECEIVE_CFG *wlan_socket_receive =
        (NCP_CMD_SOCKET_RECEIVE_CFG *)&cmd_res->params.wlan_socket_receive;
    recv_size = wlan_socket_receive->recv_size;

    if (socket_receive_res == true)
    {
        socket_receive_res = false;
        recv_size = wlan_socket_receive->recv_size;
        (void)PRINTF("receive data success\r\n");
        ncp_dump_hex(wlan_socket_receive->recv_data, recv_size);
    }
    else
    {
#if CONFIG_NCP_HOST_IO_DUMP
        (void)PRINTF("receive data success\r\n");
        ncp_dump_hex(wlan_socket_receive->recv_data, recv_size);
#endif
        if(memcmp(wlan_socket_receive->recv_data, &lwiperf_end_token[0], sizeof(lwiperf_end_token)) == 0)
        {
            (void)PRINTF("recved end token!\r\n");
            return -WM_FAIL;
        }
    }

    return recv_size;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket receive command
 *
 * @return Status returned
 */
int wlan_socket_receive_command(int argc, char **argv)
{
    unsigned int handle  = 0;
    unsigned int size    = 0;
    unsigned int timeout = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_RECV;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_RECEIVE_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_RECEIVE_CFG *)&wlan_socket_command->params.wlan_socket_receive;
    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[2], &size, strlen(argv[2])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->recv_size = size;
    if (get_uint(argv[3], &timeout, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->timeout = timeout;

    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_RECEIVE_CFG);

    socket_receive_res = true;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan socket recvfrom from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_wlan_socket_recvfrom_response(uint8_t *res)
{
    unsigned int recv_size         = 0;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        if (socket_recvfrom_res == true)
        {
            socket_recvfrom_res = false;
            (void)PRINTF("failed to receive data!\r\n");
        }
        else if (ping_res.seq_no < 0)
        {
            (void)PRINTF("failed to receive data!\r\n");
        }
        else
        {
            ping_res.echo_resp = -WM_FAIL;
            (void)ping_set_event(PING_EVENTS_RECVFROM_RESP);
            return WM_SUCCESS;
        }

        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_RECVFROM_CFG *wlan_socket_recvfrom =
        (NCP_CMD_SOCKET_RECVFROM_CFG *)&cmd_res->params.wlan_socket_recvfrom;

    if (socket_recvfrom_res == true)
    {
        socket_recvfrom_res = false;
        recv_size = wlan_socket_recvfrom->recv_size;
        (void)PRINTF("recvfrom data success, peer_ip = %s, peer_port = %d\r\n", wlan_socket_recvfrom->peer_ip,
                     wlan_socket_recvfrom->peer_port);
        ncp_dump_hex(wlan_socket_recvfrom->recv_data, recv_size);
    }
    else if (ping_res.seq_no < 0)
    {
        recv_size = wlan_socket_recvfrom->recv_size;
#if CONFIG_NCP_HOST_IO_DUMP
        (void)PRINTF("recvfrom data success, %s, peer_ip = %s, peer_port = %d\r\n", wlan_socket_recvfrom->recv_data,
                     wlan_socket_recvfrom->peer_ip, wlan_socket_recvfrom->peer_port);

        ncp_dump_hex(wlan_socket_recvfrom->recv_data, recv_size);
#endif
        /*Recved end token*/
        if(memcmp(wlan_socket_recvfrom->recv_data, &lwiperf_end_token[0], sizeof(lwiperf_end_token)) == 0)
        {
          (void)PRINTF("recved end token!\r\n");
          return -WM_FAIL;
        }
    }
    else
    {
        ping_recv(wlan_socket_recvfrom);
        (void)ping_set_event(PING_EVENTS_RECVFROM_RESP);
    }

    return recv_size;
}

/*WLAN SOCKET commamd*/
/**
 * @brief  This function prepares wlan socket recvfrom command
 *
 * @return Status returned
 */
int wlan_socket_recvfrom_command(int argc, char **argv)
{
    unsigned int handle  = 0;
    unsigned int size    = 0;
    unsigned int timeout = 0;

    MCU_NCPCmd_DS_COMMAND *wlan_socket_command = ncp_host_get_cmd_buffer_wifi();
    wlan_socket_command->header.cmd            = NCP_CMD_WLAN_SOCKET_RECVFROM;
    wlan_socket_command->header.size           = NCP_CMD_HEADER_LEN;
    wlan_socket_command->header.result         = NCP_CMD_RESULT_OK;

    if (argc != 4)
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }

    NCP_CMD_SOCKET_RECVFROM_CFG *wlan_socket_tlv =
        (NCP_CMD_SOCKET_RECVFROM_CFG *)&wlan_socket_command->params.wlan_socket_recvfrom;

    if (get_uint(argv[1], &handle, strlen(argv[1])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->handle = handle;
    if (get_uint(argv[2], &size, strlen(argv[2])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->recv_size = size;
    if (get_uint(argv[3], &timeout, strlen(argv[3])))
    {
        (void)PRINTF("Usage: %s handle recv_size timeout\r\n", __func__);
        return -WM_FAIL;
    }
    wlan_socket_tlv->timeout = timeout;

    /*cmd size*/
    wlan_socket_command->header.size += sizeof(NCP_CMD_SOCKET_RECVFROM_CFG);

    socket_recvfrom_res = true;

    return WM_SUCCESS;
}

int wlan_process_multi_mef_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    uint16_t result                = cmd_res->header.result;

    if (result == NCP_CMD_RESULT_OK)
        (void)PRINTF("multi MEF cfg is success!\r\n");
    else if (result == WM_E_PERM)
        (void)PRINTF("Failed to get IPv4 address!\r\n");
    else if (result == WM_E_2BIG)
        (void)PRINTF("Number of MEF entries exceeds limit(8)\r\n");
    else
        (void)PRINTF("multi MEF cfg is fail!\r\n");
    return WM_SUCCESS;
}

NCP_CMD_11AX_CFG_INFO g_11axcfg_params = {
    /* band */
    0x03,
    {/* tlv header */
     {0x00ff, 0x0018},
     /* extension id */
     0x23,
     /* he mac cap */
     {0x03, 0x08, 0x00, 0x82, 0x00, 0x00},
     /* he phy cap */
     {0x40, 0x50, 0x42, 0x49, 0x0d, 0x00, 0x20, 0x1e, 0x17, 0x31, 0x00},
     /* he txrx mcs support */
     {0xfd, 0xff, 0xfd, 0xff},
     /* val for txrx mcs 160Mhz or 80+80, and PPE thresholds */
     {0x88, 0x1f}}};

NCP_CMD_BTWT_CFG_INFO g_btwt_params = {.action          = 0x0001,
                                  .sub_id          = 0x0125,
                                  .nominal_wake    = 0x40,
                                  .max_sta_support = 0x04,
                                  .twt_mantissa    = 0x0063,
                                  .twt_offset      = 0x0270,
                                  .twt_exponent    = 0x0a,
                                  .sp_gap          = 0x05};

NCP_CMD_TWT_SETUP_CFG g_twt_setup_params = {.implicit            = 0x01,
                                        .announced           = 0x00,
                                        .trigger_enabled     = 0x00,
                                        .twt_info_disabled   = 0x01,
                                        .negotiation_type    = 0x00,
                                        .twt_wakeup_duration = 0x40,
                                        .flow_identifier     = 0x00,
                                        .hard_constraint     = 0x01,
                                        .twt_exponent        = 0x0a,
                                        .twt_mantissa        = 0x0200,
                                        .twt_request         = 0x00};

NCP_CMD_TWT_TEARDOWN_CFG g_twt_teardown_params = {
    .flow_identifier = 0x00, .negotiation_type = 0x00, .teardown_all_twt = 0x00};
/* index enum of cfgs */
enum
{
    TEST_WLAN_11AX_CFG,
    TEST_WLAN_BCAST_TWT,
    TEST_WLAN_TWT_SETUP,
    TEST_WLAN_TWT_TEARDOWN,
};

/*
 *  Structs for mutiple config data in freeRTOS, split cfg to various param modules.
 *  Modify cfg data by param index
 *  test_cfg_param_t param module of cfg
 *  test_cfg_table_t cfg table for all the param modules of a cfg
 */
typedef struct
{
    /* name of param */
    const char *name;
    /* offset in cfg data */
    int offset;
    int len;
    const char *notes;
} test_cfg_param_t;

typedef struct
{
    /* name of cfg */
    const char *name;
    /* point of stored data for sending cmd, stored in Little-Endian */
    uint8_t *data;
    /* len of data */
    int len;
    /* point of list for all the params */
    const test_cfg_param_t *param_list;
    /* total number of params */
    int param_num;
} test_cfg_table_t;

const static test_cfg_param_t g_11ax_cfg_param[] = {
    /* name                 offset  len     notes */
    {"band", 0, 1, NULL},
    {"cap_id", 1, 2, NULL},
    {"cap_len", 3, 2, NULL},
    {"he_cap_id", 5, 1, NULL},
    {"he_mac_cap_info", 6, 6, NULL},
    {"he_phy_cap_info", 12, 11, NULL},
    {"he_mcs_nss_support", 23, 4, NULL},
    {"pe", 27, 2, NULL},
};

const static test_cfg_param_t g_btwt_cfg_param[] = {
    /* name             offset  len   notes */
    {"action", 0, 2, "only support 1: Set"},
    {"sub_id", 2, 2, "Broadcast TWT AP config"},
    {"nominal_wake", 4, 1, "range 64-255"},
    {"max_sta_support", 5, 1, "Max STA Support"},
    {"twt_mantissa", 6, 2, NULL},
    {"twt_offset", 8, 2, NULL},
    {"twt_exponent", 10, 1, NULL},
    {"sp_gap", 11, 1, NULL},
};

static test_cfg_param_t g_twt_setup_cfg_param[] = {
    /* name                 offset  len  notes */
    {"implicit", 0, 1, "0: TWT session is explicit, 1: Session is implicit"},
    {"announced", 1, 1, "0: Unannounced, 1: Announced TWT"},
    {"trigger_enabled", 2, 1, "0: Non-Trigger enabled, 1: Trigger enabled TWT"},
    {"twt_info_disabled", 3, 1, "0: TWT info enabled, 1: TWT info disabled"},
    {"negotiation_type", 4, 1, "0: Individual TWT, 3: Broadcast TWT"},
    {"twt_wakeup_duration", 5, 1, "time after which the TWT requesting STA can transition to doze state"},
    {"flow_identifier", 6, 1, "Range: [0-7]"},
    {"hard_constraint", 7, 1,
     "0: FW can tweak the TWT setup parameters if it is rejected by AP, 1: FW should not tweak any parameters"},
    {"twt_exponent", 8, 1, "Range: [0-63]"},
    {"twt_mantissa", 9, 2, "Range: [0-sizeof(UINT16)]"},
    {"twt_request", 11, 1, "Type, 0: REQUEST_TWT, 1: SUGGEST_TWT"},
};

static test_cfg_param_t g_twt_teardown_cfg_param[] = {
    /* name             offset  len  notes */
    {"FlowIdentifier", 0, 1, "Range: [0-7]"},
    {"NegotiationType", 1, 1, "0: Future Individual TWT SP start time, 1: Next Wake TBTT tim"},
    {"TearDownAllTWT", 2, 1, "1: To teardown all TWT, 0 otherwise"},
};

/*
 *  Cfg table for mutiple params commands in freeRTOS.
 *  name:          cfg name
 *  data:          cfg data stored and prepared to send
 *  total_len:     len of cfg data
 *  param_list:    param list of cfg data
 *  param_num:     number of cfg param list
 */
static test_cfg_table_t g_test_cfg_table_list[] = {
    /*  name         data                          total_len param_list          param_num*/
    {"11axcfg", (uint8_t *)&g_11axcfg_params, 29, g_11ax_cfg_param, 8},
    {"twt_bcast", (uint8_t *)&g_btwt_params, 12, g_btwt_cfg_param, 8},
    {"twt_setup", (uint8_t *)&g_twt_setup_params, 12, g_twt_setup_cfg_param, 11},
    {"twt_teardown", (uint8_t *)&g_twt_teardown_params, 3, g_twt_teardown_cfg_param, 3},
    {NULL}};

static int wlan_send_11axcfg_command(void)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_11AX_CFG;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    (void)memcpy((uint8_t *)&command->params.he_cfg, (uint8_t *)&g_11axcfg_params, sizeof(g_11axcfg_params));
    command->header.size += sizeof(g_11axcfg_params);

    return WM_SUCCESS;
}

int wlan_process_11axcfg_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    (void)PRINTF("11ax cfg set ret %hu\r\n", cmd_res->header.result);
    return WM_SUCCESS;
}

static int wlan_send_btwt_command(void)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_BTWT_CFG;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    (void)memcpy((uint8_t *)&command->params.btwt_cfg, (uint8_t *)&g_btwt_params, sizeof(g_btwt_params));
    command->header.size += sizeof(g_btwt_params);

    return WM_SUCCESS;
}

int wlan_process_btwt_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    (void)PRINTF("btwt cfg set ret %hu\r\n", cmd_res->header.result);
    return WM_SUCCESS;
}

static int wlan_send_twt_setup_command(void)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_TWT_SETUP;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    (void)memcpy((uint8_t *)&command->params.twt_setup, (uint8_t *)&g_twt_setup_params, sizeof(g_twt_setup_params));
    command->header.size += sizeof(g_twt_setup_params);

    return WM_SUCCESS;
}

int wlan_process_twt_setup_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    (void)PRINTF("twt setup set ret %hu\r\n", cmd_res->header.result);
    return WM_SUCCESS;
}

static int wlan_send_twt_teardown_command(void)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_TWT_TEARDOWN;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    (void)memcpy((uint8_t *)&command->params.he_cfg, (uint8_t *)&g_twt_teardown_params, sizeof(g_twt_teardown_params));
    command->header.size += sizeof(g_twt_teardown_params);

    return WM_SUCCESS;
}

int wlan_process_twt_teardown_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    (void)PRINTF("11ax cfg set ret %hu\r\n", cmd_res->header.result);
    return WM_SUCCESS;
}

int wlan_get_twt_report_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_TWT_GET_REPORT;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_process_twt_report_response(uint8_t *res)
{
    int i;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_TWT_REPORT twt_report  = {0};

    if (cmd_res->header.result == NCP_CMD_RESULT_OK &&
        cmd_res->header.size >= NCP_CMD_HEADER_LEN + sizeof(twt_report))
    {
        /* TODO: sensable var */
        (void)memcpy((uint8_t *)&twt_report, (uint8_t *)&cmd_res->params.twt_report, sizeof(twt_report));

        (void)PRINTF("get twt report:\r\n");
        for (i = 0; i < 4; i++)
        {
            (void)PRINTF(
                "twt id[%d]: type[%d] len[%d] request_type[0x%x] target_wake_time[%d]"
                " nominal_min_wake_duration[%d] wake_interval_mantissa[%d] twt_info[0x%x]\r\n",
                i, twt_report.type, twt_report.length, twt_report.info[i].request_type,
                twt_report.info[i].target_wake_time, twt_report.info[i].nominal_min_wake_duration,
                twt_report.info[i].wake_interval_mantissa, twt_report.info[i].twt_info);
        }
    }
    else
    {
        (void)PRINTF("get twt report fail\r\n");
    }
    return WM_SUCCESS;
}

static void dump_cfg_data_param(int param_id, uint8_t *data, const test_cfg_param_t *param_cfg)
{
    int i;

    (void)PRINTF("%s ", param_cfg->name);
    if (param_cfg->notes != NULL)
        (void)PRINTF("#### %s\r\n", param_cfg->notes);
    else
        (void)PRINTF("\r\n");

    (void)PRINTF("[%d]: ", param_id);
    for (i = 0; i < param_cfg->len; i++)
    {
        (void)PRINTF("0x%02x ", data[param_cfg->offset + i]);
    }
    (void)PRINTF("\r\n");
}

static void set_cfg_data_param(uint8_t *data, const test_cfg_param_t *param_cfg, char **argv)
{
    int i;

    for (i = 0; i < param_cfg->len; i++)
    {
        data[param_cfg->offset + i] = a2hex(argv[3 + i]);
    }
}

static void dump_cfg_data(test_cfg_table_t *cfg)
{
    int i;
    uint8_t *data = cfg->data;

    (void)PRINTF("cfg[%s] len[%d] param_num[%d]: \r\n", cfg->name, cfg->len, cfg->param_num);
    for (i = 0; i < cfg->param_num; i++)
    {
        dump_cfg_data_param(i, data, &cfg->param_list[i]);
    }
}

static void dump_cfg_help(test_cfg_table_t *cfg)
{
    dump_cfg_data(cfg);
}

/*
 *  match param name and set data by input
 *  argv[0] "wlan-xxxx"
 *  argv[1] "set"
 *  argv[2] param_id
 *  argv[3] param_data_set
 */
static void set_cfg_data(test_cfg_table_t *cfg, int argc, char **argv)
{
    uint8_t *data                     = cfg->data;
    const test_cfg_param_t *param_cfg = NULL;
    int param_id                      = atoi(argv[2]);
    /* input data starts from argv[3] */
    int input_data_num = argc - 3;

    if (param_id < 0 || param_id >= cfg->param_num)
    {
        (void)PRINTF("invalid param index %d\r\n", param_id);
        return;
    }

    param_cfg = &cfg->param_list[param_id];
    if (param_cfg->len != input_data_num)
    {
        (void)PRINTF("invalid input number %d, param has %d u8 arguments\r\n", input_data_num, param_cfg->len);
        return;
    }

    set_cfg_data_param(data, param_cfg, argv);
    dump_cfg_data_param(param_id, data, param_cfg);
}

static void send_cfg_msg(test_cfg_table_t *cfg, uint32_t index)
{
    int ret;

    switch (index)
    {
        case TEST_WLAN_11AX_CFG:
            ret = wlan_send_11axcfg_command();
            break;
        case TEST_WLAN_BCAST_TWT:
            ret = wlan_send_btwt_command();
            break;
        case TEST_WLAN_TWT_SETUP:
            ret = wlan_send_twt_setup_command();
            break;
        case TEST_WLAN_TWT_TEARDOWN:
            ret = wlan_send_twt_teardown_command();
            break;
        default:
            ret = -1;
            break;
    }

    (void)PRINTF("send config [%s] ret %d\r\n", cfg->name, ret);
}

static void test_wlan_cfg_process(uint32_t index, int argc, char **argv)
{
    test_cfg_table_t *cfg = NULL;

    /* last cfg table is invalid */
    if (index >= (sizeof(g_test_cfg_table_list) / sizeof(test_cfg_table_t) - 1))
    {
        (void)PRINTF("cfg table too large index %u\r\n", index);
        return;
    }

    cfg = &g_test_cfg_table_list[index];

    if (argc < 2)
    {
        dump_cfg_help(cfg);
        return;
    }

    if (string_equal("help", argv[1]))
        dump_cfg_help(cfg);
    else if (string_equal("dump", argv[1]))
        dump_cfg_data(cfg);
    else if (string_equal("set", argv[1]))
        set_cfg_data(cfg, argc, argv);
    else if (string_equal("done", argv[1]))
        send_cfg_msg(cfg, index);
    else
        (void)PRINTF("unknown argument\r\n");
}

int wlan_set_11axcfg_command(int argc, char **argv)
{
    test_wlan_cfg_process(TEST_WLAN_11AX_CFG, argc, argv);
    return WM_SUCCESS;
}

int wlan_set_btwt_command(int argc, char **argv)
{
    test_wlan_cfg_process(TEST_WLAN_BCAST_TWT, argc, argv);
    return WM_SUCCESS;
}

int wlan_twt_setup_command(int argc, char **argv)
{
    test_wlan_cfg_process(TEST_WLAN_TWT_SETUP, argc, argv);
    return WM_SUCCESS;
}

int wlan_twt_teardown_command(int argc, char **argv)
{
    test_wlan_cfg_process(TEST_WLAN_TWT_TEARDOWN, argc, argv);
    return WM_SUCCESS;
}

int wlan_set_11d_enable_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    if (argc != 3)
    {
        (void)PRINTF("set 11d invalid argument\r\n");
        return -WM_FAIL;
    }

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_11D_ENABLE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    if (string_equal(argv[1], "sta"))
    {
        command->params.wlan_11d_cfg.role = 0;
    }
    else if (string_equal(argv[1], "uap"))
    {
        command->params.wlan_11d_cfg.role = 1;
    }
    else
    {
        (void)PRINTF("set 11d invalid argument, please input sta/uap\r\n");
        return -WM_FAIL;
    }
    command->params.wlan_11d_cfg.state = atoi(argv[2]);
    command->header.size += sizeof(NCP_CMD_11D_ENABLE_CFG);

    return WM_SUCCESS;
}

int wlan_process_11d_enable_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if(cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Config successfully!\r\n");
    }
    else
    {
        (void)PRINTF("config failed!\r\n");
    }
    return WM_SUCCESS;
}

int wlan_region_code_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    if (argc < 2 || argc > 3)
    {
        (void)PRINTF("region code argument\r\n");
        return -WM_FAIL;
    }

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_REGION_CODE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    if (string_equal(argv[1], "get"))
    {
        command->params.region_cfg.action = 0;
    }
    else if (string_equal(argv[1], "set"))
    {
        command->params.region_cfg.action      = 1;
        command->params.region_cfg.region_code = strtol(argv[2], NULL, 0);
    }
    else
    {
        (void)PRINTF("region code invalid argument, please input set/get\r\n");
        return -WM_FAIL;
    }
    command->header.size += sizeof(NCP_CMD_REGION_CODE_CFG);

    return WM_SUCCESS;
}

int wlan_process_region_code_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_REGION_CODE_CFG *region    = &cmd_res->params.region_cfg;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        if (region->action == 1)
            (void)PRINTF("region code set 0x%x success\r\n", region->region_code);
        else
            (void)PRINTF("region code get 0x%x\r\n", region->region_code);
    }
    else
    {
        (void)PRINTF("region code get/set fail\r\n");
    }
    return WM_SUCCESS;
}

int wlan_set_max_clients_count_command(int argc, char **argv)
{
    if (argc != 2)
    {
        (void)PRINTF("Usage: %s  max_clients_count\r\n", argv[0]);
        return -WM_FAIL;
    }

    uint16_t max_sta_count = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_client_cnt_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)set_client_cnt_command, 0, NCP_HOST_COMMAND_LEN);

    set_client_cnt_command->header.cmd      = NCP_CMD_WLAN_UAP_MAX_CLIENT_CNT;
    set_client_cnt_command->header.size     = NCP_CMD_HEADER_LEN;
    set_client_cnt_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_CLIENT_CNT *sta_count = (NCP_CMD_CLIENT_CNT *)&set_client_cnt_command->params.max_client_count;
    sta_count->max_sta_count      = max_sta_count;
    sta_count->set_status         = WLAN_SET_MAX_CLIENT_CNT_SUCCESS;
    sta_count->support_count      = 0;

    set_client_cnt_command->header.size += sizeof(NCP_CMD_CLIENT_CNT);

    return WM_SUCCESS;
}

int wlan_process_client_count_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_CLIENT_CNT *sta_count  = (NCP_CMD_CLIENT_CNT *)&cmd_res->params.max_client_count;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        if (sta_count->set_status == WLAN_SET_MAX_CLIENT_CNT_START)
            (void)PRINTF("Failed to set max client count, already started an UAP.\r\n");
        else if (sta_count->set_status == WLAN_SET_MAX_CLIENT_CNT_EXCEED)
            (void)PRINTF("Failed to set max client count, the maxmium supported value is %d\r\n",
                         sta_count->support_count);
        else
            (void)PRINTF("Failed to set max client count, wifidriver set this config failed.\r\n");
        return WM_SUCCESS;
    }

    (void)PRINTF("Success to set max client count.\r\n");
    return WM_SUCCESS;
}

static void dump_wlan_set_antenna_cfg_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-antenna-cfg <ant mode> [evaluate_time] \r\n");
    (void)PRINTF("\r\n");
    (void)PRINTF("\t<ant mode>: \r\n");
    (void)PRINTF("\t           Bit 0   -- Tx/Rx antenna 1\r\n");
    (void)PRINTF("\t           Bit 1   -- Tx/Rx antenna 2\r\n");
    (void)PRINTF("\t           15  -- Tx/Rx antenna diversity\r\n");
    (void)PRINTF("\t[evaluate_time]: \r\n");
    (void)PRINTF("\t           if ant mode = 15, SAD evaluate time interval,\r\n");
    (void)PRINTF("\t           default value is 6s(6000)\r\n");
}

int wlan_set_antenna_cfg_command(int argc, char **argv)
{
    unsigned int value;
    uint32_t ant_mode;
    uint16_t evaluate_time = 0;
    if (argc != 2 && argc != 3)
    {
        dump_wlan_set_antenna_cfg_usage();
        return -WM_FAIL;
    }

    if (get_uint(argv[1], &value, strlen(argv[1])) || (value != 1 && value != 2 && value != 0xF))
    {
        dump_wlan_set_antenna_cfg_usage();
        return -WM_FAIL;
    }

    ant_mode = value;
    if (argc == 3 && ant_mode != 0xF)
    {
        dump_wlan_set_antenna_cfg_usage();
        return -WM_FAIL;
    }

    if (ant_mode == 0xF)
    {
        if (get_uint(argv[2], &value, strlen(argv[2])))
        {
            dump_wlan_set_antenna_cfg_usage();
            return -WM_FAIL;
        }
        evaluate_time = value & 0XFF;
    }

    MCU_NCPCmd_DS_COMMAND *set_antenna_cfg_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)set_antenna_cfg_command, 0, NCP_HOST_COMMAND_LEN);

    set_antenna_cfg_command->header.cmd      = NCP_CMD_WLAN_STA_ANTENNA;
    set_antenna_cfg_command->header.size     = NCP_CMD_HEADER_LEN;
    set_antenna_cfg_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ANTENNA_CFG *antenna_cfg = (NCP_CMD_ANTENNA_CFG *)&set_antenna_cfg_command->params.antenna_cfg;
    antenna_cfg->action              = ACTION_SET;
    if (ant_mode != 0xF)
        antenna_cfg->antenna_mode = ant_mode;
    else
        antenna_cfg->antenna_mode = 0xFFFF;
    antenna_cfg->evaluate_time = evaluate_time;

    set_antenna_cfg_command->header.size += sizeof(NCP_CMD_ANTENNA_CFG);

    return WM_SUCCESS;
}

int wlan_get_antenna_cfg_command(int argc, char **argv)
{
    if (argc != 1)
    {
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("wlan-get-antcfg \r\n");
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_antenna_cfg_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)get_antenna_cfg_command, 0, NCP_HOST_COMMAND_LEN);

    get_antenna_cfg_command->header.cmd      = NCP_CMD_WLAN_STA_ANTENNA;
    get_antenna_cfg_command->header.size     = NCP_CMD_HEADER_LEN;
    get_antenna_cfg_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ANTENNA_CFG *antenna_cfg = (NCP_CMD_ANTENNA_CFG *)&get_antenna_cfg_command->params.antenna_cfg;
    antenna_cfg->action              = ACTION_GET;
    antenna_cfg->antenna_mode        = 0;
    antenna_cfg->evaluate_time       = 0;
    antenna_cfg->current_antenna     = 0;

    get_antenna_cfg_command->header.size += sizeof(NCP_CMD_ANTENNA_CFG);

    return WM_SUCCESS;
}

int wlan_process_antenna_cfg_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res   = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_ANTENNA_CFG *antenna_cfg = (NCP_CMD_ANTENNA_CFG *)&cmd_res->params.antenna_cfg;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        if (antenna_cfg->action == ACTION_SET)
            PRINTF("Failed to set antenna config.\r\n");
        else
            PRINTF("Failed to get antenna config.\r\n");
    }
    else
    {
        if (antenna_cfg->action == ACTION_SET)
            PRINTF("Sucess to set antenna config.\r\n");
        else
        {
            PRINTF("Mode of Tx/Rx path is : %x\r\n", antenna_cfg->antenna_mode);
            if (antenna_cfg->antenna_mode == 0xFFFF)
                PRINTF("Evaluate time : %x\r\n", antenna_cfg->evaluate_time);
            if (antenna_cfg->current_antenna > 0)
                PRINTF("Current antenna is %d\n", antenna_cfg->current_antenna);
        }
    }
    return WM_SUCCESS;
}

int wlan_process_deep_sleep_ps_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Deep sleep ps is success!\r\n");
    }
    else
    {
        (void)PRINTF("Deep sleep ps is fail!\r\n");
    }
    return WM_SUCCESS;
}

int wlan_process_ieee_ps_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("IEEE ps is success!\r\n");
    }
    else
    {
        (void)PRINTF("IEEE ps is fail!\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_reg_access_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("Read the register:\r\n");
    (void)PRINTF("    wlan-reg-access <type> <offset>\r\n");
    (void)PRINTF("Write the regiset:\r\n");
    (void)PRINTF("    wlan-reg-access <type> <offset> <value>\r\n");
    (void)PRINTF("Options: \r\n");
    (void)PRINTF("    <type>  : 1:MAC, 2:BBP, 3:RF, 4:CAU\r\n");
    (void)PRINTF("    <offset>: offset of register\r\n");
    (void)PRINTF("For example:\r\n");
    (void)PRINTF("    wlan-reg-access 1 0x9b8             : Read the MAC register\r\n");
    (void)PRINTF("    wlan-reg-access 1 0x9b8 0x80000000 : Write 0x80000000 to MAC register\r\n");
}

int wlan_register_access_command(int argc, char **argv)
{
    uint16_t action;
    uint32_t type   = 0;
    uint32_t value  = 0;
    uint32_t offset = 0;
    if (argc < 3 || argc > 4)
    {
        dump_wlan_reg_access_usage();
        return -WM_FAIL;
    }

    if ((a2hex_or_atoi(argv[1]) != 1 && a2hex_or_atoi(argv[1]) != 2 && a2hex_or_atoi(argv[1]) != 3 &&
         a2hex_or_atoi(argv[1]) != 4))
    {
        dump_wlan_reg_access_usage();
        (void)PRINTF("Error: Illegal register type %s. Must be either '1','2','3' or '4'.\r\n", argv[1]);
        return -WM_FAIL;
    }

    type = a2hex_or_atoi(argv[1]);
    if (argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X'))
        offset = a2hex_or_atoi(argv[2]);
    else
    {
        dump_wlan_reg_access_usage();
        (void)PRINTF("Error: invalid offset argument\r\n");
        return -WM_FAIL;
    }

    if (argc == 3)
        action = ACTION_GET;
    else
    {
        action = ACTION_SET;
        if (argv[3][0] == '0' && (argv[3][1] == 'x' || argv[3][1] == 'X'))
            value = a2hex_or_atoi(argv[3]);
        else
        {
            dump_wlan_reg_access_usage();
            (void)PRINTF("Error: invalid value argument\r\n");
            return -WM_FAIL;
        }
    }

    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_DEBUG_REGISTER_ACCESS;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_REGISTER_ACCESS *register_access = (NCP_CMD_REGISTER_ACCESS *)&command->params.register_access;

    register_access->action = action;
    register_access->type   = type & 0xFF;
    register_access->offset = offset;
    register_access->value  = value;

    command->header.size += sizeof(NCP_CMD_REGISTER_ACCESS);

    return WM_SUCCESS;
}

int wlan_process_register_access_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res           = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_REGISTER_ACCESS *register_access = (NCP_CMD_REGISTER_ACCESS *)&cmd_res->params.register_access;

    char type[4][4] = {"MAC", "BBP", "RF", "CAU"};
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        if (register_access->action == ACTION_GET)
            (void)PRINTF("Register: %s  offset = 0x%08x    value = 0x%08x\r\n", type[register_access->type - 1],
                         register_access->offset, register_access->value);
        else
            (void)PRINTF("Set the register successfully\r\n");
    }
    else
    {
        if (register_access->action == ACTION_GET)
            (void)PRINTF("Read Register failed\r\n");
        else
            (void)PRINTF("Write Register failed\r\n");
    }

    (void)PRINTF("\r\n");

    return WM_SUCCESS;
}

#if CONFIG_NCP_MEM_MONITOR_DEBUG
int wlan_memory_state_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_MEMORY_HEAP_SIZE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_process_memory_state_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_MEM_STAT *mem_state    = (NCP_CMD_MEM_STAT *)&cmd_res->params.mem_stat;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("FreeHeapSize    : %u \r\n", mem_state->free_heap_size);
        (void)PRINTF("MinFreeHeapSize : %u \r\n\r\n", mem_state->minimun_ever_free_heap_size);
    }
    else
    {
        (void)PRINTF("Failed to get heap size.\r\n");
    }

    (void)PRINTF("\r\n");

    return WM_SUCCESS;
}
#endif

static void dump_wlan_set_ed_mac_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
#if CONFIG_NCP_5GHz_SUPPORT
    (void)PRINTF("wlan-set-ed-mac-mode <ed_ctrl_2g> <ed_offset_2g> <ed_ctrl_5g> <ed_offset_5g>\r\n");
#else
    (void)PRINTF("wlan-set-ed-mac-mode <ed_ctrl_2g> <ed_offset_2g>\r\n");
#endif
    (void)PRINTF("\r\n");
    (void)PRINTF("\ted_ctrl_2g \r\n");
    (void)PRINTF("\t    # 0       - disable EU adaptivity for 2.4GHz band\r\n");
    (void)PRINTF("\t    # 1       - enable EU adaptivity for 2.4GHz band\r\n");
    (void)PRINTF("\ted_offset_2g \r\n");
    (void)PRINTF("\t    # 0       - Default Energy Detect threshold\r\n");
    (void)PRINTF("\t    #offset value range: 0x80 to 0x7F\r\n");
#if CONFIG_NCP_5GHz_SUPPORT
    (void)PRINTF("\ted_ctrl_5g \r\n");
    (void)PRINTF("\t    # 0       - disable EU adaptivity for 5GHz band\r\n");
    (void)PRINTF("\t    # 1       - enable EU adaptivity for 5GHz band\r\n");
    (void)PRINTF("\ted_offset_5g \r\n");
    (void)PRINTF("\t    # 0       - Default Energy Detect threshold\r\n");
    (void)PRINTF("\t    #offset value range: 0x80 to 0x7F\r\n");
#endif
}

int wlan_ed_mac_mode_set_command(int argc, char **argv)
{
    unsigned int value;
#if CONFIG_NCP_5GHz_SUPPORT
    if (argc != 5)
#else
    if (argc != 3)
#endif
    {
        (void)PRINTF("Invalid argument numbers\r\n");
        dump_wlan_set_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_REGULATORY_ED_MAC_MODE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ED_MAC *ed_mac_mode = (NCP_CMD_ED_MAC *)&command->params.ed_mac_mode;
    ed_mac_mode->action         = ACTION_SET;

    if (get_uint(argv[1], &value, strlen(argv[1])) || (value != 0 && value != 1))
    {
        (void)PRINTF("Error: invalid ed_ctrl_2g value\r\n");
        dump_wlan_set_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    ed_mac_mode->ed_ctrl_2g = value & 0xFF;

    if (argv[2][0] == '0' && (argv[2][1] == 'x' || argv[2][1] == 'X'))
        value = a2hex_or_atoi(argv[2]);
    else
    {
        (void)PRINTF("Error: invalid ed_offset_2g value\r\n");
        dump_wlan_set_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    ed_mac_mode->ed_offset_2g = value & 0xFF;

#if CONFIG_NCP_5GHz_SUPPORT
    if (get_uint(argv[3], &value, strlen(argv[3])) || (value != 0 && value != 1))
    {
        (void)PRINTF("Error: invalid ed_ctrl_5g value\r\n");
        dump_wlan_set_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    ed_mac_mode->ed_ctrl_5g = value & 0xFF;

    if (argv[4][0] == '0' && (argv[4][1] == 'x' || argv[4][1] == 'X'))
        value = a2hex_or_atoi(argv[4]);
    else
    {
        (void)PRINTF("Error: invalid ed_offset_5g value\r\n");
        dump_wlan_set_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    ed_mac_mode->ed_offset_5g = value & 0xFF;
#endif

    command->header.size += sizeof(NCP_CMD_ED_MAC);

    return WM_SUCCESS;
}

static void dump_wlan_get_ed_mac_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-ed-mac-mode \r\n");
}

int wlan_ed_mac_mode_get_command(int argc, char **argv)
{
    if (argc != 1)
    {
        dump_wlan_get_ed_mac_mode_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_WLAN_REGULATORY_ED_MAC_MODE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_ED_MAC *ed_mac_mode = (NCP_CMD_ED_MAC *)&command->params.ed_mac_mode;
    ed_mac_mode->action         = ACTION_GET;

    command->header.size += sizeof(NCP_CMD_ED_MAC);

    return WM_SUCCESS;
}

int wlan_process_ed_mac_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_ED_MAC *ed_mac_mode    = (NCP_CMD_ED_MAC *)&cmd_res->params.ed_mac_mode;

    if (ed_mac_mode->action == ACTION_SET)
    {
        if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        {
            (void)PRINTF("ED MAC MODE settings configuration successful\r\n");
        }
        else
        {
            (void)PRINTF("ED MAC MODE settings configuration failed\r\n");
        }
    }
    else
    {
        if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        {
            (void)PRINTF("EU adaptivity for 2.4GHz band : %s\r\n",
                         ed_mac_mode->ed_ctrl_2g == 1 ? "Enabled" : "Disabled");
            if (ed_mac_mode->ed_offset_2g != 0)
            {
                (void)PRINTF("Energy Detect threshold offset : 0X%x\r\n", ed_mac_mode->ed_offset_2g);
            }
#if CONFIG_NCP_5GHz_SUPPORT
            (void)PRINTF("EU adaptivity for 5GHz band : %s\r\n", ed_mac_mode->ed_ctrl_5g == 1 ? "Enabled" : "Disabled");
            if (ed_mac_mode->ed_offset_5g != 0)
            {
                (void)PRINTF("Energy Detect threshold offset : 0X%x\r\n", ed_mac_mode->ed_offset_5g);
            }
#endif
        }
        else
        {
            (void)PRINTF("ED MAC MODE read failed\r\n");
        }
    }
    (void)PRINTF("\r\n");

    return WM_SUCCESS;
}

#if CONFIG_NCP_RF_TEST_MODE
static bool ncp_rf_test_mode = false;

static void dump_wlan_set_rf_test_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-test-mode \r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf test mode command
 *
 * @return Status returned
 */
int wlan_set_rf_test_mode_command(int argc, char **argv)
{
    if (argc != 1)
    {
        dump_wlan_set_rf_test_mode_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *set_rf_test_mode_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_test_mode_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_TEST_MODE;
    set_rf_test_mode_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_test_mode_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf test mode response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_test_mode_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        ncp_rf_test_mode = true;
        (void)PRINTF("RF Test Mode configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF Test Mode configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_set_rf_test_mode()
{
    (void)PRINTF("RF Test Mode is not set\r\n");
    dump_wlan_set_rf_test_mode_usage();
}

static void dump_wlan_set_rf_tx_antenna_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-tx-antenna <antenna> \r\n");
    (void)PRINTF("antenna: 1=Main, 2=Aux \r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf tx antenna command
 *
 * @return Status returned
 */
int wlan_set_rf_tx_antenna_command(int argc, char **argv)
{
    uint8_t ant;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_tx_antenna_usage();
        return -WM_FAIL;
    }

    ant = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_tx_antenna_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_tx_antenna_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_TX_ANTENNA;
    set_rf_tx_antenna_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_tx_antenna_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_TX_ANTENNA *tx_antenna = (NCP_CMD_RF_TX_ANTENNA *)&set_rf_tx_antenna_command->params.rf_tx_antenna;
    tx_antenna->ant                   = ant;
    set_rf_tx_antenna_command->header.size += sizeof(NCP_CMD_RF_TX_ANTENNA);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf tx antenna response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_tx_antenna_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Tx Antenna configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF Tx Antenna configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_tx_antenna_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-tx-antenna \r\n");
}

/**
 * @brief      This function prepares wlan get rf tx antenna command
 *
 * @return Status returned
 */
int wlan_get_rf_tx_antenna_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_tx_antenna_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_tx_antenna_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_tx_antenna_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_TX_ANTENNA;
    get_rf_tx_antenna_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_tx_antenna_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf tx antenna response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_tx_antenna_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Tx Antenna configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_TX_ANTENNA *tx_antenna = (NCP_CMD_RF_TX_ANTENNA *)&cmd_res->params.rf_tx_antenna;
    (void)PRINTF("Configured RF Tx Antenna is: %s\r\n", tx_antenna->ant == 1 ? "Main" : "Aux");

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_rx_antenna_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-rx-antenna <antenna> \r\n");
    (void)PRINTF("antenna: 1=Main, 2=Aux \r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf rx antenna command
 *
 * @return Status returned
 */
int wlan_set_rf_rx_antenna_command(int argc, char **argv)
{
    uint8_t ant;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_rx_antenna_usage();
        return -WM_FAIL;
    }

    ant = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_rx_antenna_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_rx_antenna_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_RX_ANTENNA;
    set_rf_rx_antenna_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_rx_antenna_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_RX_ANTENNA *rx_antenna = (NCP_CMD_RF_RX_ANTENNA *)&set_rf_rx_antenna_command->params.rf_rx_antenna;
    rx_antenna->ant                   = ant;
    set_rf_rx_antenna_command->header.size += sizeof(NCP_CMD_RF_RX_ANTENNA);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf rx antenna response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_rx_antenna_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Rx Antenna configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF Rx Antenna configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_rx_antenna_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-rx-antenna \r\n");
}

/**
 * @brief      This function prepares wlan get rf rx antenna command
 *
 * @return Status returned
 */
int wlan_get_rf_rx_antenna_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_rx_antenna_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_rx_antenna_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_rx_antenna_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_RX_ANTENNA;
    get_rf_rx_antenna_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_rx_antenna_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf rx antenna response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_rx_antenna_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Rx Antenna configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_RX_ANTENNA *rx_antenna = (NCP_CMD_RF_RX_ANTENNA *)&cmd_res->params.rf_rx_antenna;
    (void)PRINTF("Configured RF Rx Antenna is: %s\r\n", rx_antenna->ant == 1 ? "Main" : "Aux");

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_band_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-band <band> \r\n");
#if CONFIG_NCP_5GHz_SUPPORT
    (void)PRINTF("band: 0=2.4G, 1=5G \r\n");
#else
    (void)PRINTF("band: 0=2.4G \r\n");
#endif
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf band command
 *
 * @return Status returned
 */
int wlan_set_rf_band_command(int argc, char **argv)
{
    uint8_t band;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_band_usage();
        return -WM_FAIL;
    }

    band = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_band_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_band_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_BAND;
    set_rf_band_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_band_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_BAND *rf_band = (NCP_CMD_RF_BAND *)&set_rf_band_command->params.rf_band;
    rf_band->band            = band;
    set_rf_band_command->header.size += sizeof(NCP_CMD_RF_BAND);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf band response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_band_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Band configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF Band configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_band_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-band \r\n");
}

/**
 * @brief      This function prepares wlan get rf band command
 *
 * @return Status returned
 */
int wlan_get_rf_band_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_band_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_band_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_band_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_BAND;
    get_rf_band_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_band_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf band response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_band_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Band configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_BAND *rf_band = (NCP_CMD_RF_BAND *)&cmd_res->params.rf_band;
    (void)PRINTF("Configured RF Band is: %s\r\n", rf_band->band ? "5G" : "2.4G");

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_bandwidth_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-bandwidth <bandwidth> \r\n");
    (void)PRINTF("\r\n");
    (void)PRINTF("\t<bandwidth>: \r\n");
    (void)PRINTF("\t		0: 20MHz\r\n");
#if CONFIG_NCP_5GHz_SUPPORT
    (void)PRINTF("\t		1: 40MHz\r\n");
#endif
#if CONFIG_NCP_11AC
    (void)PRINTF("\t		4: 80MHz\r\n");
#endif
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf bandwidth command
 *
 * @return Status returned
 */
int wlan_set_rf_bandwidth_command(int argc, char **argv)
{
    uint8_t bandwidth;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_bandwidth_usage();
        return -WM_FAIL;
    }

    bandwidth = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_bandwidth_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_bandwidth_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_BANDWIDTH;
    set_rf_bandwidth_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_bandwidth_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_BANDWIDTH *rf_bandwidth = (NCP_CMD_RF_BANDWIDTH *)&set_rf_bandwidth_command->params.rf_bandwidth;
    rf_bandwidth->bandwidth            = bandwidth;
    set_rf_bandwidth_command->header.size += sizeof(NCP_CMD_RF_BANDWIDTH);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf bandwidth response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_bandwidth_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Bandwidth configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF Bandwidth configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_bandwidth_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-bandwidth \r\n");
}

/**
 * @brief      This function prepares wlan get rf bandwidth command
 *
 * @return Status returned
 */
int wlan_get_rf_bandwidth_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_bandwidth_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_bandwidth_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_bandwidth_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_BANDWIDTH;
    get_rf_bandwidth_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_bandwidth_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf bandwidth response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_bandwidth_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF Bandwidth configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_BANDWIDTH *rf_bandwidth = (NCP_CMD_RF_BANDWIDTH *)&cmd_res->params.rf_bandwidth;
    (void)PRINTF("Configured RF bandwidth is: %s\r\n",
                 rf_bandwidth->bandwidth == 0 ? "20MHz" : rf_bandwidth->bandwidth == 1 ? "40MHz" : "80MHz");

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_channel_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-channel <channel> \r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf channel command
 *
 * @return Status returned
 */
int wlan_set_rf_channel_command(int argc, char **argv)
{
    uint8_t channel;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_channel_usage();
        return -WM_FAIL;
    }

    channel = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_channel_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_channel_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_CHANNEL;
    set_rf_channel_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_channel_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_CHANNEL *rf_channel = (NCP_CMD_RF_CHANNEL *)&set_rf_channel_command->params.rf_channel;
    rf_channel->channel            = channel;
    set_rf_channel_command->header.size += sizeof(NCP_CMD_RF_CHANNEL);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf channel response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_channel_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF channel configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("RF channel configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_channel_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-channel \r\n");
}

/**
 * @brief      This function prepares wlan get rf channel command
 *
 * @return Status returned
 */
int wlan_get_rf_channel_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_channel_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_channel_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_channel_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_CHANNEL;
    get_rf_channel_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_channel_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf channel response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_channel_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF channel configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_CHANNEL *rf_channel = (NCP_CMD_RF_CHANNEL *)&cmd_res->params.rf_channel;
    (void)PRINTF("Configured channel is: %d\r\n\r\n", rf_channel->channel);

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_radio_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-radio-mode <radio_mode> \r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf radio mode command
 *
 * @return Status returned
 */
int wlan_set_rf_radio_mode_command(int argc, char **argv)
{
    uint8_t radio_mode;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 2)
    {
        dump_wlan_set_rf_radio_mode_usage();
        return -WM_FAIL;
    }

    radio_mode = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *set_rf_radio_mode_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_radio_mode_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_RADIO_MODE;
    set_rf_radio_mode_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_radio_mode_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_RADIO_MODE *rf_radio_mode = (NCP_CMD_RF_RADIO_MODE *)&set_rf_radio_mode_command->params.rf_radio_mode;
    rf_radio_mode->radio_mode            = radio_mode;
    set_rf_radio_mode_command->header.size += sizeof(NCP_CMD_RF_RADIO_MODE);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf radio mode response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_radio_mode_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Set RF radio mode successful\r\n");
    }
    else
    {
        (void)PRINTF("Set RF radio mode failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_rf_radio_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-rf-radio-mode \r\n");
}

/**
 * @brief      This function prepares wlan get rf radio mode command
 *
 * @return Status returned
 */
int wlan_get_rf_radio_mode_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_rf_radio_mode_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_rf_radio_mode_command = ncp_host_get_cmd_buffer_wifi();
    get_rf_radio_mode_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_RF_RADIO_MODE;
    get_rf_radio_mode_command->header.size           = NCP_CMD_HEADER_LEN;
    get_rf_radio_mode_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get rf radio mode response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_rf_radio_mode_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF radio mode configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_RADIO_MODE *rf_radio_mode = (NCP_CMD_RF_RADIO_MODE *)&cmd_res->params.rf_radio_mode;
    (void)PRINTF("Configured RF radio mode is: %d\r\n\r\n\r\n", rf_radio_mode->radio_mode);

    return WM_SUCCESS;
}

static void dump_wlan_set_rf_tx_power_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-set-rf-tx-power <tx_power> <modulation> <path_id> \r\n");
    (void)PRINTF("Power       (0 to 24 dBm)\r\n");
    (void)PRINTF("Modulation  (0: CCK, 1:OFDM, 2:MCS)\r\n");
    (void)PRINTF("Path ID     (0: PathA, 1:PathB, 2:PathA+B)\r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf tx power command
 *
 * @return Status returned
 */
static int wlan_ncp_set_rf_tx_power_command(int argc, char **argv)
{
    uint8_t power;
    uint8_t mod;
    uint8_t path_id;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 4)
    {
        dump_wlan_set_rf_tx_power_usage();
        return -WM_FAIL;
    }

    power   = atoi(argv[1]);
    mod     = atoi(argv[2]);
    path_id = atoi(argv[3]);

    if (power > 24)
    {
        dump_wlan_set_rf_tx_power_usage();
        return -WM_FAIL;
    }

    if (mod != 0 && mod != 1 && mod != 2)
    {
        dump_wlan_set_rf_tx_power_usage();
        return -WM_FAIL;
    }

    if (path_id != 0 && path_id != 1 && path_id != 2)
    {
        dump_wlan_set_rf_tx_power_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *set_rf_tx_power_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_tx_power_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_TX_POWER;
    set_rf_tx_power_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_tx_power_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_TX_POWER *rf_tx_power = (NCP_CMD_RF_TX_POWER *)&set_rf_tx_power_command->params.rf_tx_power;
    rf_tx_power->power               = power;
    rf_tx_power->mod                 = mod;
    rf_tx_power->path_id             = path_id;
    set_rf_tx_power_command->header.size += sizeof(NCP_CMD_RF_TX_POWER);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf tx power response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_tx_power_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Set RF tx power configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("Set RF tx power configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_set_tx_cont_mode_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF(
        "wlan-set-rf-tx-cont-mode <enable_tx> <cw_mode> <payload_pattern> <cs_mode> <act_sub_ch> <tx_rate> \r\n");
    (void)PRINTF("Enable                (0:disable, 1:enable)\r\n");
    (void)PRINTF("Continuous Wave Mode  (0:disable, 1:enable)\r\n");
    (void)PRINTF("Payload Pattern       (0 to 0xFFFFFFFF) (Enter hexadecimal value)\r\n");
    (void)PRINTF("CS Mode               (Applicable only when continuous wave is disabled) (0:disable, 1:enable)\r\n");
    (void)PRINTF("Active SubChannel     (0:low, 1:upper, 3:both)\r\n");
    (void)PRINTF("Tx Data Rate          (Rate Index corresponding to legacy/HT/VHT rates)\r\n");
    (void)PRINTF("\r\n");
    (void)PRINTF("To Disable:\r\n");
    (void)PRINTF("wlan-set-rf-tx-cont-mode 0\r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf tx cont mode command
 *
 * @return Status returned
 */
static int wlan_ncp_set_rf_tx_cont_mode_command(int argc, char **argv)
{
    uint32_t enable_tx, cw_mode, payload_pattern, cs_mode, act_sub_ch, tx_rate;

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc == 2 && atoi(argv[1]) == 0)
    {
        enable_tx       = 0;
        cw_mode         = 0;
        payload_pattern = 0;
        cs_mode         = 0;
        act_sub_ch      = 0;
        tx_rate         = 0;
    }
    else if (argc != 7)
    {
        dump_wlan_set_tx_cont_mode_usage();
        return -WM_FAIL;
    }
    else
    {
        enable_tx       = atoi(argv[1]);
        cw_mode         = atoi(argv[2]);
        payload_pattern = strtol(argv[3], NULL, 16);
        cs_mode         = atoi(argv[4]);
        act_sub_ch      = atoi(argv[5]);
        tx_rate         = atoi(argv[6]);
    }

    MCU_NCPCmd_DS_COMMAND *set_rf_tx_cont_mode_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_tx_cont_mode_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_TX_CONT_MODE;
    set_rf_tx_cont_mode_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_tx_cont_mode_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_TX_CONT_MODE *rf_tx_power =
        (NCP_CMD_RF_TX_CONT_MODE *)&set_rf_tx_cont_mode_command->params.rf_tx_cont_mode;
    rf_tx_power->enable_tx       = enable_tx;
    rf_tx_power->cw_mode         = cw_mode;
    rf_tx_power->payload_pattern = payload_pattern;
    rf_tx_power->cs_mode         = cs_mode;
    rf_tx_power->act_sub_ch      = act_sub_ch;
    rf_tx_power->tx_rate         = tx_rate;
    set_rf_tx_cont_mode_command->header.size += sizeof(NCP_CMD_RF_TX_CONT_MODE);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf tx cont mode response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_tx_cont_mode_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Set RF tx continuous configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("Set RF tx continuous configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_set_tx_frame_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF(
        "wlan-set-rf-tx-frame <start> <data_rate> <frame_pattern> <frame_len> <adjust_burst_sifs> <burst_sifs_in_us> "
        "<short_preamble> <act_sub_ch> <short_gi> <adv_coding> <tx_bf> <gf_mode> <stbc> <bssid>\r\n");
    (void)PRINTF("Enable                 (0:disable, 1:enable)\r\n");
    (void)PRINTF("Tx Data Rate           (Rate Index corresponding to legacy/HT/VHT rates)\r\n");
    (void)PRINTF("Payload Pattern        (0 to 0xFFFFFFFF) (Enter hexadecimal value)\r\n");
    (void)PRINTF("Payload Length         (1 to 0x400) (Enter hexadecimal value)\r\n");
    (void)PRINTF("Adjust Burst SIFS3 Gap (0:disable, 1:enable)\r\n");
    (void)PRINTF("Burst SIFS in us       (0 to 255us)\r\n");
    (void)PRINTF("Short Preamble         (0:disable, 1:enable)\r\n");
    (void)PRINTF("Active SubChannel      (0:low, 1:upper, 3:both)\r\n");
    (void)PRINTF("Short GI               (0:disable, 1:enable)\r\n");
    (void)PRINTF("Adv Coding             (0:disable, 1:enable)\r\n");
    (void)PRINTF("Beamforming            (0:disable, 1:enable)\r\n");
    (void)PRINTF("GreenField Mode        (0:disable, 1:enable)\r\n");
    (void)PRINTF("STBC                   (0:disable, 1:enable)\r\n");
    (void)PRINTF("BSSID                  (xx:xx:xx:xx:xx:xx)\r\n");
    (void)PRINTF("\r\n");
    (void)PRINTF("To Disable:\r\n");
    (void)PRINTF("wlan-set-rf-tx-frame 0\r\n");
    (void)PRINTF("\r\n");
}

/**
 * @brief  This function prepares wlan set rf tx frame command
 *
 * @return Status returned
 */
static int wlan_ncp_set_rf_tx_frame_command(int argc, char **argv)
{
    int ret;
    uint32_t enable;
    uint32_t data_rate;
    uint32_t frame_pattern;
    uint32_t frame_length;
    uint32_t adjust_burst_sifs;
    uint32_t burst_sifs_in_us;
    uint32_t short_preamble;
    uint32_t act_sub_ch;
    uint32_t short_gi;
    uint32_t adv_coding;
    uint32_t tx_bf;
    uint32_t gf_mode;
    uint32_t stbc;
    uint8_t bssid[NCP_WLAN_MAC_ADDR_LENGTH];

    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc == 2 && atoi(argv[1]) == 0)
    {
        enable            = 0;
        data_rate         = 0;
        frame_pattern     = 0;
        frame_length      = 1;
        adjust_burst_sifs = 0;
        burst_sifs_in_us  = 0;
        short_preamble    = 0;
        act_sub_ch        = 0;
        short_gi          = 0;
        adv_coding        = 0;
        tx_bf             = 0;
        gf_mode           = 0;
        stbc              = 0;
        (void)memset(bssid, 0, sizeof(bssid));
    }
    else if (argc != 15)
    {
        dump_wlan_set_tx_frame_usage();
        return -WM_FAIL;
    }
    else
    {
        enable            = atoi(argv[1]);
        data_rate         = atoi(argv[2]);
        frame_pattern     = strtol(argv[3], NULL, 16);
        frame_length      = strtol(argv[4], NULL, 16);
        adjust_burst_sifs = atoi(argv[5]);
        burst_sifs_in_us  = atoi(argv[6]);
        short_preamble    = atoi(argv[7]);
        act_sub_ch        = atoi(argv[8]);
        short_gi          = atoi(argv[9]);
        adv_coding        = atoi(argv[10]);
        tx_bf             = atoi(argv[11]);
        gf_mode           = atoi(argv[12]);
        stbc              = atoi(argv[13]);
        ret               = get_mac((const char *)argv[14], (char *)bssid, ':');
        if (ret)
        {
            dump_wlan_set_tx_frame_usage();
            return -WM_FAIL;
        }

        if (enable > 1 || frame_length < 1 || frame_length > 0x400 || burst_sifs_in_us > 255 || short_preamble > 1 ||
            act_sub_ch == 2 || act_sub_ch > 3 || short_gi > 1 || adv_coding > 1 || tx_bf > 1 || gf_mode > 1 || stbc > 1)
        {
            dump_wlan_set_tx_frame_usage();
            return -WM_FAIL;
        }
    }

    MCU_NCPCmd_DS_COMMAND *set_rf_tx_frame_command = ncp_host_get_cmd_buffer_wifi();
    set_rf_tx_frame_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_SET_RF_TX_FRAME;
    set_rf_tx_frame_command->header.size           = NCP_CMD_HEADER_LEN;
    set_rf_tx_frame_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_RF_TX_FRAME *rf_tx_frame = (NCP_CMD_RF_TX_FRAME *)&set_rf_tx_frame_command->params.rf_tx_frame;
    rf_tx_frame->enable              = enable;
    rf_tx_frame->data_rate           = data_rate;
    rf_tx_frame->frame_pattern       = frame_pattern;
    rf_tx_frame->frame_length        = frame_length;
    rf_tx_frame->adjust_burst_sifs   = adjust_burst_sifs;
    rf_tx_frame->burst_sifs_in_us    = burst_sifs_in_us;
    rf_tx_frame->short_preamble      = short_preamble;
    rf_tx_frame->act_sub_ch          = act_sub_ch;
    rf_tx_frame->short_gi            = short_gi;
    rf_tx_frame->adv_coding          = adv_coding;
    rf_tx_frame->tx_bf               = tx_bf;
    rf_tx_frame->gf_mode             = gf_mode;
    rf_tx_frame->stbc                = stbc;
    memcpy(rf_tx_frame->bssid, bssid, sizeof(bssid));
    set_rf_tx_frame_command->header.size += sizeof(NCP_CMD_RF_TX_FRAME);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan set rf tx frame response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_tx_frame_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Set RF tx frame configuration successful\r\n");
    }
    else
    {
        (void)PRINTF("Set RF tx frame configuration failed\r\n");
    }
    return WM_SUCCESS;
}

static void dump_wlan_get_and_reset_rf_per_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("wlan-get-and-reset-rf-per \r\n");
}

/**
 * @brief  This function prepares wlan get and reset rf per command
 *
 * @return Status returned
 */
static int wlan_ncp_set_rf_get_and_reset_rf_per_command(int argc, char **argv)
{
    if (!ncp_rf_test_mode)
    {
        dump_wlan_set_rf_test_mode();
        return -WM_FAIL;
    }

    if (argc != 1)
    {
        dump_wlan_get_and_reset_rf_per_usage();
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *get_and_reset_rf_per_command = ncp_host_get_cmd_buffer_wifi();
    get_and_reset_rf_per_command->header.cmd            = NCP_CMD_WLAN_REGULATORY_GET_AND_RESET_RF_PER;
    get_and_reset_rf_per_command->header.size           = NCP_CMD_HEADER_LEN;
    get_and_reset_rf_per_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get and reset rf per response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_set_rf_get_and_reset_rf_per_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("RF PER configuration read failed\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_RF_PER *rf_per = (NCP_CMD_RF_PER *)&cmd_res->params.rf_per;
    (void)PRINTF("PER is as below: \r\n");
    (void)PRINTF("Total Rx Packet Count: %d\r\n", rf_per->rx_tot_pkt_count);
    (void)PRINTF("Total Rx Multicast/Broadcast Packet Count: %d\r\n", rf_per->rx_mcast_bcast_count);
    (void)PRINTF("Total Rx Packets with FCS error: %d\r\n", rf_per->rx_pkt_fcs_error);

    return WM_SUCCESS;
}
#endif

int wlan_process_set_mac_address(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to set mac address\r\n");
    }

    return WM_SUCCESS;
}

int wlan_process_get_mac_address(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        NCP_CMD_GET_MAC_ADDRESS *get_mac = (NCP_CMD_GET_MAC_ADDRESS *)&cmd_res->params.get_mac_addr;
        (void)PRINTF("MAC Address\r\n");
        (void)PRINTF("STA MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", MAC2STR((unsigned char)get_mac->sta_mac));
        (void)PRINTF("UAP MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", MAC2STR((unsigned char)get_mac->uap_mac));
    }
    else
    {
        (void)PRINTF("failed to get mac address\r\n");
    }

    return WM_SUCCESS;
}

static const char *print_role(uint8_t role)
{
    switch (role)
    {
        case WLAN_BSS_ROLE_STA:
            return "Infra";
        case WLAN_BSS_ROLE_UAP:
            return "uAP";
        case WLAN_BSS_ROLE_ANY:
            return "any";
    }
    return "unknown";
}

char *ipv6_addr_addr_to_desc(void *addr)
{
    ip6_addr_t ip6_addr;

    (void)memcpy((void *)ip6_addr.addr, addr, sizeof(ip6_addr.addr));

    return inet6_ntoa(ip6_addr);
}

static void print_address(NCP_WLAN_NETWORK *network, uint8_t role)
{
    struct in_addr ip, gw, nm, dns1, dns2;
    char addr_type[40] = {0};

    if (role == WLAN_BSS_ROLE_STA && !network->is_sta_ipv4_connected)
        goto out;

    ip.s_addr   = network->ipv4.address;
    gw.s_addr   = network->ipv4.gw;
    nm.s_addr   = network->ipv4.netmask;
    dns1.s_addr = network->ipv4.dns1;
    dns2.s_addr = network->ipv4.dns2;
    if (network->ipv4.addr_type == ADDR_TYPE_STATIC)
        strcpy(addr_type, "STATIC");
    else if (network->ipv4.addr_type == ADDR_TYPE_STATIC)
        strcpy(addr_type, "AUTO IP");
    else
        strcpy(addr_type, "DHCP");

    (void)PRINTF("\r\n\tIPv4 Address\r\n");
    (void)PRINTF("\taddress: %s", addr_type);
    (void)PRINTF("\r\n\t\tIP:\t\t%s", inet_ntoa(ip));
    (void)PRINTF("\r\n\t\tgateway:\t%s", inet_ntoa(gw));
    (void)PRINTF("\r\n\t\tnetmask:\t%s", inet_ntoa(nm));
    (void)PRINTF("\r\n\t\tdns1:\t\t%s", inet_ntoa(dns1));
    (void)PRINTF("\r\n\t\tdns2:\t\t%s", inet_ntoa(dns2));
    (void)PRINTF("\r\n");
out:
#if CONFIG_NCP_IPV6
    if (role == WLAN_BSS_ROLE_STA || role == WLAN_BSS_ROLE_UAP)
    {
        int i;
        (void)PRINTF("\r\n\tIPv6 Addresses\r\n");
        for (i = 0; i < CONFIG_MAX_IPV6_ADDRESSES; i++)
        {
            if (network->ipv6[i].addr_state_str[0] != '\0')
            {
                if (strcmp((char *)network->ipv6[i].addr_state_str, "Invalid"))
                {
                    (void)PRINTF("\t%-13s:\t%s (%s)\r\n", network->ipv6[i].addr_type_str,
                                 ipv6_addr_addr_to_desc((void *)network->ipv6[i].address),
                                 network->ipv6[i].addr_state_str);
                }
            }
        }
        (void)PRINTF("\r\n");
    }
#endif
    return;
}

static void print_network(NCP_WLAN_NETWORK *network)
{
    char *pssid = "(hidden)";
    if (network->ssid[0])
        pssid = network->ssid;
    (void)PRINTF("\"%s\"\r\n\tSSID: %s\r\n\tBSSID: ", network->name, pssid);
    print_mac((const char *)network->bssid);
    if (network->channel != 0U)
        (void)PRINTF("\r\n\tchannel: %d", network->channel);
    else
        (void)PRINTF("\r\n\tchannel: %s", "(Auto)");
    (void)PRINTF("\r\n\trole: %s\r\n", print_role(network->role));

    char *sec_tag = "\tsecurity";
    if (!network->security_specific)
    {
        sec_tag = "\tsecurity [Wildcard]";
    }
    switch (network->security_type)
    {
        case WLAN_SECURITY_NONE:
            (void)PRINTF("%s: none\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WEP_OPEN:
            (void)PRINTF("%s: WEP (open)\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WEP_SHARED:
            (void)PRINTF("%s: WEP (shared)\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WPA:
            (void)PRINTF("%s: WPA\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WPA2:
            (void)PRINTF("%s: WPA2\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WPA_WPA2_MIXED:
            (void)PRINTF("%s: WPA/WPA2 Mixed\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WPA3_SAE:
            (void)PRINTF("%s: WPA3 SAE\r\n", sec_tag);
            break;
        case WLAN_SECURITY_WPA2_WPA3_SAE_MIXED:
            (void)PRINTF("%s: WPA2/WPA3 SAE Mixed\r\n", sec_tag);
            break;
        default:
            break;
    }
#if CONFIG_NCP_WIFI_CAPA
    if (network->role == WLAN_BSS_ROLE_UAP)
    {
        if (network->wlan_capa & WIFI_SUPPORT_11AX)
        {
            if (!network->enable_11ax)
            {
                if (network->enable_11ac)
                    (void)PRINTF("\twifi capability: 11ac\r\n");
                else
                    (void)PRINTF("\twifi capability: 11n\r\n");
            }
            else
                (void)PRINTF("\twifi capability: 11ax\r\n");
            (void)PRINTF("\tuser configure: 11ax\r\n");
        }
        else if (network->wlan_capa & WIFI_SUPPORT_11AC)
        {
            if (!network->enable_11ac)
                (void)PRINTF("\twifi capability: 11n\r\n");
            else
                (void)PRINTF("\twifi capability: 11ac\r\n");
            (void)PRINTF("\tuser configure: 11ac\r\n");
        }
        else if (network->wlan_capa & WIFI_SUPPORT_11N)
        {
            if (!network->enable_11n)
                (void)PRINTF("\twifi capability: legacy\r\n");
            else
                (void)PRINTF("\twifi capability: 11n\r\n");
            (void)PRINTF("\tuser configure: 11n\r\n");
        }
        else
        {
            (void)PRINTF("\twifi capability: legacy\r\n");
            (void)PRINTF("\tuser configure: legacy\r\n");
        }
    }
#endif
    print_address(network, network->role);
#if CONFIG_NCP_SCAN_WITH_RSSIFILTER
    (void)PRINTF("\r\n\trssi threshold: %d \r\n", network->rssi_threshold);
#endif
}

int wlan_process_info(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to get wlan info\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_INFO *network_info = (NCP_CMD_NETWORK_INFO *)&cmd_res->params.network_info;
    if (network_info->sta_conn_stat == WLAN_CONNECTED)
    {
        (void)PRINTF("Station connected to:\r\n");
        print_network(&network_info->sta_network);
    }
    else
    {
        (void)PRINTF("Station not connected\r\n");
    }

    if (network_info->uap_conn_stat == WLAN_UAP_STARTED)
    {
        (void)PRINTF("uAP started as:\r\n");
        print_network(&network_info->uap_network);
    }
    else
    {
        (void)PRINTF("uAP not started\r\n");
    }

    return WM_SUCCESS;
}

int wlan_process_address(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to get wlan address\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_ADDRESS *network_address = (NCP_CMD_NETWORK_ADDRESS *)&cmd_res->params.network_address;
    if (network_address->sta_conn_stat == WLAN_CONNECTED)
    {
        print_address(&network_address->sta_network, network_address->sta_network.role);
    }
    else
    {
        (void)PRINTF("Station not connected\r\n");
    }

    return WM_SUCCESS;
}

int wlan_process_add_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    switch (ret)
    {
        case WM_SUCCESS:
            (void)PRINTF("Added network successfully\r\n");
            break;
        case -WM_E_INVAL:
            (void)PRINTF("Error: network already exists or invalid arguments\r\n");
            break;
        case -WM_E_NOMEM:
            (void)PRINTF("Error: network list is full\r\n");
            break;
        case WLAN_ERROR_STATE:
            (void)PRINTF("Error: can't add networks in this state\r\n");
            break;
        default:
            (void)PRINTF(
                "Error: unable to add network for unknown"
                " reason\r\n");
            break;
    }

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan start network response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_start_network_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Error: unable to start network\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_START *start_res_tlv = (NCP_CMD_NETWORK_START *)&cmd_res->params.network_start;
    (void)PRINTF("UAP started\r\n");
    (void)PRINTF("Soft AP \"%s\" started successfully\r\n", start_res_tlv->ssid);

    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan stop network response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_stop_network_response(uint8_t *res)
{
    int ret;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    ret                            = cmd_res->header.result;

    if (ret == NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Stop network successfully\r\n");
    }
    else
    {
        (void)PRINTF("Error: unable to stop network\r\n");
    }
    return WM_SUCCESS;
}

/**
 * @brief      This function processes wlan get uap sta list response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_get_uap_sta_list(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    int i;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to get wlan uap sta list\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_UAP_STA_LIST *uap_sta_list = (NCP_CMD_NETWORK_UAP_STA_LIST *)&cmd_res->params.uap_sta_list;

    (void)PRINTF("Number of STA = %d \r\n\r\n", uap_sta_list->sta_count);
    for (i = 0; i < uap_sta_list->sta_count; i++)
    {
        (void)PRINTF("STA %d information:\r\n", i + 1);
        (void)PRINTF("=====================\r\n");
        (void)PRINTF("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\r\n", uap_sta_list->info[i].mac[0],
                     uap_sta_list->info[i].mac[1], uap_sta_list->info[i].mac[2], uap_sta_list->info[i].mac[3],
                     uap_sta_list->info[i].mac[4], uap_sta_list->info[i].mac[5]);
        (void)PRINTF("Power mfg status: %s\r\n",
                     (uap_sta_list->info[i].power_mgmt_status == 0) ? "active" : "power save");
        (void)PRINTF("Rssi : %d dBm\r\n\r\n", (signed char)uap_sta_list->info[i].rssi);
    }

    return WM_SUCCESS;
}

int wlan_set_time_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();
    NCP_CMD_DATE_TIME_CFG *time        = &command->params.date_time;

    if (argc != 7)
    {
        (void)PRINTF("set time invalid argument, please input <year> <month> <day> <hour> <minute> <second>\r\n");
        return -WM_FAIL;
    }

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_DATE_TIME;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    time->action           = 1;
    time->date_time.year   = atoi(argv[1]);
    time->date_time.month  = atoi(argv[2]);
    time->date_time.day    = atoi(argv[3]);
    time->date_time.hour   = atoi(argv[4]);
    time->date_time.minute = atoi(argv[5]);
    time->date_time.second = atoi(argv[6]);

    command->header.size += sizeof(NCP_CMD_DATE_TIME_CFG);

    return WM_SUCCESS;
}

int wlan_get_time_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_DATE_TIME;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    command->params.date_time.action = 0;
    command->header.size += sizeof(NCP_CMD_DATE_TIME_CFG);

    return WM_SUCCESS;
}

int wlan_process_time_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_DATE_TIME_CFG *time        = &cmd_res->params.date_time;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        if (time->action == 1)
        {
            (void)PRINTF("time set success\r\n");
        }
        else
        {
            wlan_date_time_t *t = &time->date_time;
            (void)PRINTF("time: %d:%d:%d %d:%d:%d\r\n", t->year, t->month, t->day, t->hour, t->minute, t->second);
        }
    }
    else
    {
        (void)PRINTF("time get/set fail\r\n");
    }
    return WM_SUCCESS;
}

int wlan_get_temperature_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *command = ncp_host_get_cmd_buffer_wifi();

    (void)memset((uint8_t *)command, 0, NCP_HOST_COMMAND_LEN);
    command->header.cmd      = NCP_CMD_GET_TEMPERATUE;
    command->header.size     = NCP_CMD_HEADER_LEN;
    command->header.result   = NCP_CMD_RESULT_OK;

    command->header.size += sizeof(NCP_CMD_TEMPERATURE);

    return WM_SUCCESS;
}

int wlan_process_get_temperature_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("temperature %dC\r\n", cmd_res->params.temperature.temp);
    else
        (void)PRINTF("temperature get fail\r\n");
    return WM_SUCCESS;
}

/**
 * @brief      This function processes monitor response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_monitor_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        PRINTF("Monitor cfg is success!\r\n");
        return WM_SUCCESS;
    }
    else
        PRINTF("Monitor cfg is fail!\r\n");
    return WM_SUCCESS;
}

/**
 * @brief      This function processes csi response from ncp device
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
int wlan_process_csi_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        PRINTF("CSI cfg is success!\r\n");
        return WM_SUCCESS;
    }
    else
        PRINTF("CSI cfg is fail!\r\n");
    return WM_SUCCESS;
}

int wlan_process_11k_cfg_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        PRINTF("11k cfg is success!\r\n");
        return WM_SUCCESS;
    }
    else
        PRINTF("11k cfg is fail!\r\n");
    return WM_SUCCESS;
}

int wlan_process_neighbor_req_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        PRINTF("neighbor_req is success!\r\n");
        return WM_SUCCESS;
    }
    else
        PRINTF("neighbor_req is fail!\r\n");
    return WM_SUCCESS;
}

/* Display the usage of ping */
static void display_ping_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF(
        "\tping [-s <packet_size>] [-c <packet_count>] "
        "[-W <timeout in sec>] <ipv4 address>\r\n");
    (void)PRINTF("Default values:\r\n");
    (void)PRINTF(
        "\tpacket_size: %u\r\n\tpacket_count: %u"
        "\r\n\ttimeout: %u sec\r\n",
        PING_DEFAULT_SIZE, PING_DEFAULT_COUNT, PING_DEFAULT_TIMEOUT_SEC);
}

int ncp_ping_command(int argc, char **argv)
{
    int c;
    int ret = WM_SUCCESS, errno = 0;
    uint16_t size    = PING_DEFAULT_SIZE;
    uint32_t count   = PING_DEFAULT_COUNT, temp;
    uint32_t timeout = PING_DEFAULT_TIMEOUT_SEC;

    /* If number of arguments is not even then print error */
    if ((argc % 2) != 0)
    {
        ret = -WM_FAIL;
        goto end;
    }

    cli_optind = 1;
    while ((c = cli_getopt(argc, argv, "c:s:W:")) != -1)
    {
        errno = 0;
        switch (c)
        {
            case 'c':
                count = strtoul(cli_optarg, NULL, 10);
                break;
            case 's':
                temp = strtoul(cli_optarg, NULL, 10);
                if (temp > PING_MAX_SIZE)
                {
                    if (errno != 0)
                        (void)PRINTF("Error during strtoul errno:%d", errno);
                    (void)PRINTF(
                        "ping: packet size too large: %u."
                        " Maximum is %u\r\n",
                        temp, PING_MAX_SIZE);
                    return -WM_FAIL;
                }
                size = temp;
                break;
            case 'W':
                timeout = strtoul(cli_optarg, NULL, 10);
                break;
            default:
                goto end;
        }
        if (errno != 0)
            (void)PRINTF("Error during strtoul errno:%d", errno);
    }
    if (cli_optind == argc)
        goto end;

    (void)memset(&ping_msg, 0, sizeof(ping_msg_t));
    ping_msg.count   = count;
    ping_msg.size    = size;
    ping_msg.timeout = timeout;
    ping_msg.port    = 0;
    strcpy(ping_msg.ip_addr, argv[cli_optind++]);

    /* Notify ping_sock_task to handle ping command*/
    (void)ping_set_event(PING_EVENTS_START);

    return WM_SUCCESS;

end:
    (void)PRINTF("Incorrect usage\r\n");
    display_ping_usage();

    return ret;
}

int wlan_start_wps_pbc_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wpspbc_command = ncp_host_get_cmd_buffer_wifi();
    wpspbc_command->header.cmd            = NCP_CMD_WLAN_STA_WPS_PBC;
    wpspbc_command->header.size           = NCP_CMD_HEADER_LEN;
    wpspbc_command->header.result         = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_process_wps_pbc_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("start wps pbc is successful!\r\n");
    else
        (void)PRINTF("start wps pbc is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_wps_generate_pin_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *wps_gen_pin_command = ncp_host_get_cmd_buffer_wifi();
    wps_gen_pin_command->header.cmd            = NCP_CMD_WLAN_STA_GEN_WPS_PIN;
    wps_gen_pin_command->header.size           = NCP_CMD_HEADER_LEN;
    wps_gen_pin_command->header.result         = NCP_CMD_RESULT_OK;
    wps_gen_pin_command->header.size += sizeof(NCP_CMD_WPS_GEN_PIN);

    return WM_SUCCESS;
}

int wlan_process_wps_generate_pin_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CMD_WPS_GEN_PIN *pin_info  = (NCP_CMD_WPS_GEN_PIN *)&cmd_res->params.wps_gen_pin_info;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("WPS PIN is %08u\r\n", pin_info->pin);
    else
        (void)PRINTF("WPS PIN generation is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_start_wps_pin_command(int argc, char **argv)
{
    uint32_t pin = 0;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <8 digit pin>\r\n", argv[0]);
        return -WM_FAIL;
    }

    pin = atoi(argv[1]);

    MCU_NCPCmd_DS_COMMAND *wps_pin_command = ncp_host_get_cmd_buffer_wifi();
    wps_pin_command->header.cmd            = NCP_CMD_WLAN_STA_WPS_PIN;
    wps_pin_command->header.size           = NCP_CMD_HEADER_LEN;
    wps_pin_command->header.result         = NCP_CMD_RESULT_OK;

    NCP_CMD_WPS_PIN *wps_pin_cfg = (NCP_CMD_WPS_PIN *)&wps_pin_command->params.wps_pin_cfg;
    wps_pin_cfg->pin             = pin;
    wps_pin_command->header.size += sizeof(NCP_CMD_WPS_PIN);

    return WM_SUCCESS;
}

int wlan_process_wps_pin_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
        (void)PRINTF("start wps pin is successful!\r\n");
    else
        (void)PRINTF("start wps pin is fail!\r\n");

    return WM_SUCCESS;
}

int wlan_mdns_query_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *mdns_command = ncp_host_get_cmd_buffer_wifi();

    if (!(argc == 3 || argc == 2))
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("      wlan-mdns-query <service> <protocol>\r\n");
        (void)PRINTF("For example:\r\n");
        (void)PRINTF("      wlan-mdns-query _http tcp\r\n");
        return -WM_FAIL;
    }

    mdns_result_num = 0;

    mdns_command->header.cmd      = NCP_CMD_WLAN_NETWORK_MDNS_QUERY;
    mdns_command->header.size     = NCP_CMD_HEADER_LEN;
    mdns_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MDNS_QUERY *mdns_query = (NCP_CMD_MDNS_QUERY *)&mdns_command->params.mdns_query;

    if (argc == 3)
    {
        mdns_query->qtype = DNS_RRTYPE_PTR;
        memcpy(mdns_query->Q.ptr_cfg.service, argv[1], strlen(argv[1]) + 1);
        if (!strcmp(argv[2], "udp"))
        {
            mdns_query->Q.ptr_cfg.proto = DNSSD_PROTO_UDP;
        }
        else if (!strcmp(argv[2], "tcp"))
        {
            mdns_query->Q.ptr_cfg.proto = DNSSD_PROTO_TCP;
        }
        else
        {
            (void)PRINTF("Invalid protocol value\r\n");
            return -WM_FAIL;
        }
    }

    if (argc == 2)
    {
        mdns_query->qtype = DNS_RRTYPE_A;
        memcpy(mdns_query->Q.a_cfg.name, argv[1], strlen(argv[1]) + 1);
    }

    mdns_command->header.size += sizeof(NCP_CMD_MDNS_QUERY);

    return WM_SUCCESS;
}

int wlan_process_mdns_query_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("failed to mDNS query!\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("mDNS query succeeded!\r\n");
    return WM_SUCCESS;
}

int wlan_process_mdns_query_result_event(uint8_t *res)
{
    uint32_t tlv_buf_len                 = 0;
    uint8_t *ptlv_pos                    = NULL;
    NCP_MCU_HOST_TLV_HEADER *ptlv_header = NULL;
    PTR_ParamSet_t *ptr_tlv              = NULL;
    SRV_ParamSet_t *srv_tlv              = NULL;
    TXT_ParamSet_t *txt_tlv              = NULL;
    IP_ADDR_ParamSet_t *ip_addr_tlv      = NULL;

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (evt_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Unknown mDNS result!\r\n");
        return -WM_FAIL;
    }

    NCP_EVT_MDNS_RESULT *mdns_result_tlv = (NCP_EVT_MDNS_RESULT *)&evt_res->params.mdns_result;

    (void)PRINTF("Answers: %d\r\n", mdns_result_num++);
    ptlv_pos    = mdns_result_tlv->tlv_buf;
    tlv_buf_len = mdns_result_tlv->tlv_buf_len;

    do
    {
        ptlv_header = (NCP_MCU_HOST_TLV_HEADER *)ptlv_pos;

        switch (ptlv_header->type)
        {
            case NCP_CMD_NETWORK_MDNS_RESULT_PTR:
                ptr_tlv = (PTR_ParamSet_t *)ptlv_pos;
                if (ptr_tlv->instance_name[0] != '\0')
                    (void)PRINTF("PTR : %s\r\n", ptr_tlv->instance_name);
                else
                    (void)PRINTF("PTR : %s.%s.local\r\n", ptr_tlv->service_type, ptr_tlv->proto);
                break;
            case NCP_CMD_NETWORK_MDNS_RESULT_SRV:
                srv_tlv = (SRV_ParamSet_t *)ptlv_pos;
                (void)PRINTF("SRV : %s:%d\r\n", srv_tlv->target, srv_tlv->port);
                break;
            case NCP_CMD_NETWORK_MDNS_RESULT_TXT:
                txt_tlv = (TXT_ParamSet_t *)ptlv_pos;
                (void)PRINTF("TXT : %s\r\n", txt_tlv->txt);
                break;
            case NCP_CMD_NETWORK_MDNS_RESULT_IP_ADDR:
                ip_addr_tlv = (IP_ADDR_ParamSet_t *)ptlv_pos;
                if (ip_addr_tlv->addr_type == 4)
                {
                    struct in_addr ip;
                    ip.s_addr = ip_addr_tlv->ip.ip_v4;
                    (void)PRINTF("A   : %s\r\n", inet_ntoa(ip));
                }
                else
                {
                    (void)PRINTF("AAAA: %s\r\n", ipv6_addr_addr_to_desc((void *)ip_addr_tlv->ip.ip_v6));
                }
                break;
            default:
                (void)PRINTF("Invaild TLV\r\n");
                return -WM_FAIL;
        }
        ptlv_pos += NCP_TLV_HEADER_LEN + ptlv_header->size;
        tlv_buf_len -= NCP_TLV_HEADER_LEN + ptlv_header->size;
    } while (tlv_buf_len > 0);

    (void)PRINTF("TTL : %d\r\n", mdns_result_tlv->ttl);

    return WM_SUCCESS;
}

int wlan_process_mdns_resolve_domain_event(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res         = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_EVT_MDNS_RESOLVE *mdns_resolve_tlv = (NCP_EVT_MDNS_RESOLVE *)&evt_res->params.mdns_resolve;
    struct in_addr ip;

    if (evt_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Unknown IP address!\r\n");
        return -WM_FAIL;
    }

    switch (mdns_resolve_tlv->ip_type)
    {
        case MDNS_ADDRTYPE_IPV4:
            ip.s_addr = mdns_resolve_tlv->u_addr.ip4_addr;
            (void)PRINTF("IPv4 address: %s\r\n", inet_ntoa(ip));
            break;
        case MDNS_ADDRTYPE_IPV6:
            (void)PRINTF("IPv6 address: %s\r\n", ipv6_addr_addr_to_desc((void *)mdns_resolve_tlv->u_addr.ip6_addr));
            break;
        default:
            (void)PRINTF("Not found ip address\r\n");
            break;
    }

    return WM_SUCCESS;
}

int wlan_process_csi_data_event(uint8_t *res)
{
    //MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    //NCP_EVT_CSI_DATA *p_csi_data = (NCP_EVT_CSI_DATA *)&evt_res->params.csi_data;
    //PRINTF("CSI user callback: Event CSI data\r\n");
    // The real CSI data length is p_csi_data->Len*4 bytes,
    // print 1/4 to avoid USB rx buffer overflow.
    //ncp_dump_hex((void *)p_csi_data, p_csi_data->Len);
    return WM_SUCCESS;
}

int wlan_process_con_event(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    struct in_addr ip;

    if (evt_res->header.result != NCP_CMD_RESULT_OK)
    {
        PRINTF("Failed to get correct AP info!\r\n");
        PRINTF("Please input 'wlan-connect' to connect an AP or wait a few moments for the AP information.\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_WLAN_CONN *connect_res_tlv = (NCP_CMD_WLAN_CONN *)&evt_res->params.wlan_connect;
    ip.s_addr                          = connect_res_tlv->ip;
    PRINTF("STA connected:\r\n");
    PRINTF("SSID = [%s]\r\n", connect_res_tlv->ssid);
    PRINTF("IPv4 Address: [%s]\r\n", inet_ntoa(ip));

    return WM_SUCCESS;
}

int wlan_process_discon_event(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    (void)PRINTF("STA disconnect, result %d\r\n",evt_res->header.result);

    return WM_SUCCESS;
}

int wlan_process_stop_network_event(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    (void)PRINTF("Stop network, result %d\r\n",evt_res->header.result);

    return WM_SUCCESS;
}

int wlan_list_command(int argc, char **argv)
{
    mcu_get_command_lock();
    MCU_NCPCmd_DS_COMMAND *network_list_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)network_list_command, 0, NCP_HOST_COMMAND_LEN);

    if (argc != 1)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("wlan-list\r\n");
        return -WM_FAIL;
    }

    network_list_command->header.cmd      = NCP_CMD_WLAN_NETWORK_LIST;
    network_list_command->header.size     = NCP_CMD_HEADER_LEN;
    network_list_command->header.result   = NCP_CMD_RESULT_OK;

    return WM_SUCCESS;
}

int wlan_process_network_list_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to get network list!\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_LIST *network_list = (NCP_CMD_NETWORK_LIST *)&cmd_res->params.network_list;

    (void)PRINTF(" %d networks %s\r\n", network_list->count, network_list->count == 0 ? "" : ":");

    for (int i = 0; i < network_list->count; i++)
    {
        print_network(&network_list->net_list[i]);
    }

    return WM_SUCCESS;
}

int wlan_remove_command(int argc, char **argv)
{
    mcu_get_command_lock();
    MCU_NCPCmd_DS_COMMAND *network_remove_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)network_remove_command, 0, NCP_HOST_COMMAND_LEN);

    if (argc != 2)
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("wlan-remove <profile_name>\r\n");
        return -WM_FAIL;
    }

    if (strlen(argv[1]) > WLAN_NETWORK_NAME_MAX_LENGTH)
    {
        (void)PRINTF("Error: The length of profile_name is too log.\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF("wlan-remove <profile_name>\r\n");
        return -WM_FAIL;
    }

    network_remove_command->header.cmd      = NCP_CMD_WLAN_NETWORK_REMOVE;
    network_remove_command->header.size     = NCP_CMD_HEADER_LEN;
    network_remove_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_NETWORK_REMOVE *network_remove = (NCP_CMD_NETWORK_REMOVE *)&network_remove_command->params.network_remove;

    (void)memcpy(network_remove->name, argv[1], strlen(argv[1]));
    network_remove->remove_state = WM_SUCCESS;

    network_remove_command->header.size += sizeof(NCP_CMD_NETWORK_REMOVE);

    return WM_SUCCESS;
}

int wlan_process_network_remove_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to remove network!\r\n");
        return -WM_FAIL;
    }

    NCP_CMD_NETWORK_REMOVE *network_remove = (NCP_CMD_NETWORK_REMOVE *)&cmd_res->params.network_remove;

    switch (network_remove->remove_state)
    {
        case WM_SUCCESS:
            (void)PRINTF("Removed \"%s\"\r\n", network_remove->name);
            break;
        case -WM_E_INVAL:
            (void)PRINTF("Error: network not found\r\n");
            break;
        case WLAN_ERROR_STATE:
            (void)PRINTF("Error: can't remove network in this state\r\n");
            break;
        default:
            (void)PRINTF("Error: unable to remove network\r\n");
            break;
    }

    return WM_SUCCESS;
}

int wlan_mbo_enable_command(int argc, char **argv)
{
    int enable_mbo;
    int errno = 0;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <0/1> < 0--disable MBO; 1---enable MBO>\r\n", argv[0]);
        return -WM_FAIL;
    }

    enable_mbo = (int)strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        (void)PRINTF("Error during strtol:wlan mbo cfg:%d\r\n", errno);
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *wlan_mbo_cfg_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)wlan_mbo_cfg_command, 0, NCP_HOST_COMMAND_LEN);

    wlan_mbo_cfg_command->header.cmd      = NCP_CMD_WLAN_MBO_ENABLE;
    wlan_mbo_cfg_command->header.size     = NCP_CMD_HEADER_LEN;
    wlan_mbo_cfg_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MBO_ENABLE *wlan_mbo_enable = (NCP_CMD_MBO_ENABLE *)&wlan_mbo_cfg_command->params.wlan_mbo_enable;
    wlan_mbo_enable->enable             = enable_mbo;
    wlan_mbo_cfg_command->header.size += sizeof(NCP_CMD_MBO_ENABLE);

    return WM_SUCCESS;
}

int wlan_mbo_nonprefer_ch(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *mbo_nonprefer_ch_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)mbo_nonprefer_ch_command, 0, NCP_HOST_COMMAND_LEN);

    if (!(argc == 2 || argc == 5))
    {
        (void)PRINTF("Error: invalid number of arguments\r\n");
        (void)PRINTF("Usage:\r\n");
        (void)PRINTF(
            "      For ncp: wlan-mbo-nonprefer-ch <ch0> <Preference0: 0/1/255> <ch1> <Preference1: "
            "0/1/255>\r\n");
        (void)PRINTF(
            "      For ncp_supplicant: wlan-mbo-nonprefer-ch \"<oper_class>:<chan>:<preference>:<reason> "
            "<oper_class>:<chan>:<preference>:<reason>\"\r\n");
        return -WM_FAIL;
    }

    mbo_nonprefer_ch_command->header.cmd      = NCP_CMD_WLAN_MBO_NONPREFER_CH;
    mbo_nonprefer_ch_command->header.size     = NCP_CMD_HEADER_LEN;
    mbo_nonprefer_ch_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MBO_NONPREFER_CH *mbo_nonprefer_ch =
        (NCP_CMD_MBO_NONPREFER_CH *)&mbo_nonprefer_ch_command->params.mbo_nonprefer_ch;

    if (argc == 2)
    {
        memcpy(mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_supp_cfg.mbo_nonprefer_ch_params, argv[1],
               strlen(argv[1]) + 1);
    }

    if (argc == 5)
    {
        uint8_t ch0;
        uint8_t ch1;
        uint8_t preference0;
        uint8_t preference1;

        ch0         = atoi(argv[1]);
        ch1         = atoi(argv[2]);
        preference0 = atoi(argv[3]);
        preference1 = atoi(argv[4]);

        mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.ch0         = ch0;
        mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.ch1         = ch1;
        mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.preference0 = preference0;
        mbo_nonprefer_ch->NONPREFER_CH_CFG.mbo_nonprefer_ch_cfg.preference0 = preference1;
    }

    mbo_nonprefer_ch_command->header.size += sizeof(NCP_CMD_MBO_NONPREFER_CH);

    return WM_SUCCESS;
}

int wlan_mbo_set_cell_capa(int argc, char **argv)
{
    int errno = 0;
    ;
    uint8_t cell_capa;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <cell capa: 1/2/3(default)>\r\n", argv[0]);
        (void)PRINTF(
            "\tMBO Cellular Data Capabilities\r\n"
            "\t# 1 = Cellular data connection available\r\n"
            "\t# 2 = Cellular data connection not available\r\n"
            "\t# 3 = Not cellular capable (default)\r\n");
        return -WM_FAIL;
    }

    cell_capa = (uint8_t)strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        (void)PRINTF("Error during strtol:wlan mbo cell_capa:%d\r\n", errno);
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *wlan_mbo_set_cell_capa_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)wlan_mbo_set_cell_capa_command, 0, NCP_HOST_COMMAND_LEN);

    wlan_mbo_set_cell_capa_command->header.cmd      = NCP_CMD_WLAN_MBO_SET_CELL_CAPA;
    wlan_mbo_set_cell_capa_command->header.size     = NCP_CMD_HEADER_LEN;
    wlan_mbo_set_cell_capa_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MBO_SET_CELL_CAPA *wlan_mbo_set_cell_capa =
        (NCP_CMD_MBO_SET_CELL_CAPA *)&wlan_mbo_set_cell_capa_command->params.wlan_mbo_set_cell_capa;
    wlan_mbo_set_cell_capa->cell_capa = cell_capa;
    wlan_mbo_set_cell_capa_command->header.size += sizeof(NCP_CMD_MBO_SET_CELL_CAPA);

    return WM_SUCCESS;
}

int wlan_mbo_set_oce(int argc, char **argv)
{
    int errno = 0;
    ;
    uint8_t oce;

    if (argc != 2)
    {
        (void)PRINTF("Usage: %s <oce: 1(default)/2>\r\n", argv[0]);
        (void)PRINTF(
            "\tOptimized Connectivity Experience (OCE)\r\n"
            "\t# oce: Enable OCE features\r\n"
            "\t# 1 = Enable OCE in non-AP STA mode (default;\r\n"
            "\tdisabled if the driver does not indicate support for OCE in STA mode)\r\n"
            "\t# 2 = Enable OCE in STA-CFON mode\r\n");
        return -WM_FAIL;
    }

    oce = (uint8_t)strtol(argv[1], NULL, 10);
    if (errno != 0)
    {
        (void)PRINTF("Error during strtol:wlan mbo oce:%d\r\n", errno);
        return -WM_FAIL;
    }

    MCU_NCPCmd_DS_COMMAND *wlan_mbo_set_oce_command = ncp_host_get_cmd_buffer_wifi();
    (void)memset((uint8_t *)wlan_mbo_set_oce_command, 0, NCP_HOST_COMMAND_LEN);

    wlan_mbo_set_oce_command->header.cmd      = NCP_CMD_WLAN_MBO_SET_OCE;
    wlan_mbo_set_oce_command->header.size     = NCP_CMD_HEADER_LEN;
    wlan_mbo_set_oce_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_MBO_SET_OCE *wlan_mbo_set_oce = (NCP_CMD_MBO_SET_OCE *)&wlan_mbo_set_oce_command->params.wlan_mbo_set_oce;
    wlan_mbo_set_oce->oce                 = oce;
    wlan_mbo_set_oce_command->header.size += sizeof(NCP_CMD_MBO_SET_OCE);

    return WM_SUCCESS;
}

int wlan_process_mbo_enable_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to config MBO\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("Config MBO succeeded!\r\n");
    return WM_SUCCESS;
}

int wlan_process_mbo_nonprefer_ch_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to add pefer or non-pefer channels.\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("Add pefer or non-pefer channels succeeded!\r\n");
    return WM_SUCCESS;
}

int wlan_process_mbo_set_cell_capa_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to set mbo cell capa.\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("Set mbo cell capa succeeded!\r\n");

    return WM_SUCCESS;
}

int wlan_process_mbo_set_oce_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result != NCP_CMD_RESULT_OK)
    {
        (void)PRINTF("Failed to set mbo oce.\r\n");
        return -WM_FAIL;
    }

    (void)PRINTF("Set mbo oce succeeded!\r\n");

    return WM_SUCCESS;
}

int wlan_process_ncp_event(uint8_t *res)
{
    int ret                    = -WM_FAIL;
    MCU_NCPCmd_DS_COMMAND *evt = (MCU_NCPCmd_DS_COMMAND *)res;

    switch (evt->header.cmd)
    {
        case NCP_EVENT_MDNS_QUERY_RESULT:
            ret = wlan_process_mdns_query_result_event(res);
            break;
        case NCP_EVENT_MDNS_RESOLVE_DOMAIN:
            ret = wlan_process_mdns_resolve_domain_event(res);
            break;
        case NCP_EVENT_CSI_DATA:
            ret = wlan_process_csi_data_event(res);
            break;
        case NCP_EVENT_WLAN_STA_CONNECT:
            ret = wlan_process_con_event(res);
            break;
        case NCP_EVENT_WLAN_STA_DISCONNECT:
            ret = wlan_process_discon_event(res);
            break;
        case NCP_EVENT_WLAN_STOP_NETWORK:
            ret = wlan_process_stop_network_event(res);
            break;
        default:
            PRINTF("Invaild event!\r\n");
            break;
    }
    return ret;
}

/**
 * @brief       This function processes response from ncp device
 *
 * @param res   A pointer to uint8_t
 * @return      Status returned
 */
int wlan_process_response(uint8_t *res)
{
    int ret                        = -WM_FAIL;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    switch (cmd_res->header.cmd)
    {
        case NCP_RSP_WLAN_STA_SCAN:
            ret = wlan_process_scan_response(res);
            break;
        case NCP_RSP_WLAN_STA_SIGNAL:
            ret = wlan_process_rssi_response(res);
            break;
        case NCP_RSP_WLAN_STA_CONNECT:
            ret = wlan_process_con_response(res);
            break;
        case NCP_RSP_WLAN_STA_DISCONNECT:
            ret = wlan_process_discon_response(res);
            break;
        case NCP_RSP_WLAN_STA_VERSION:
            ret = wlan_process_version_response(res);
            break;
        case NCP_RSP_WLAN_STA_CONNECT_STAT:
            ret = wlan_process_stat_response(res);
            break;
        case NCP_RSP_WLAN_STA_ROAMING:
            ret = wlan_process_roaming_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_MEF:
            ret = wlan_process_multi_mef_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_UAPSD:
            ret = wlan_process_wmm_uapsd_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_QOSINFO:
            ret = wlan_process_uapsd_qosinfo_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_SLEEP_PERIOD:
            ret = wlan_process_uapsd_sleep_period_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_WOWLAN_CFG:
            ret = wlan_process_wakeup_condition_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_SUSPEND:
            ret = wlan_process_suspend_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_CON:
            ret = wlan_process_wlan_http_con_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_DISCON:
            ret = wlan_process_wlan_http_discon_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_REQ:
            ret = wlan_process_wlan_http_req_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_RECV:
            ret = wlan_process_wlan_http_recv_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_SETH:
            ret = wlan_process_wlan_http_seth_response(res);
            break;
        case NCP_RSP_WLAN_HTTP_UNSETH:
            ret = wlan_process_wlan_http_unseth_response(res);
            break;
        case NCP_RSP_WLAN_WEBSOCKET_UPG:
            ret = wlan_process_wlan_websocket_upg_response(res);
            break;
        case NCP_RSP_WLAN_WEBSOCKET_SEND:
            ret = wlan_process_wlan_websocket_send_response(res);
            break;
        case NCP_RSP_WLAN_WEBSOCKET_RECV:
            ret = wlan_process_wlan_websocket_recv_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_OPEN:
            ret = wlan_process_wlan_socket_open_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_CON:
            ret = wlan_process_wlan_socket_con_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_BIND:
            ret = wlan_process_wlan_socket_bind_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_CLOSE:
            ret = wlan_process_wlan_socket_close_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_LISTEN:
            ret = wlan_process_wlan_socket_listen_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_ACCEPT:
            ret = wlan_process_wlan_socket_accept_response(res);
            break;
        case NCP_RSP_WLAN_SOCKET_SEND:
            ret                 = wlan_process_wlan_socket_send_response(res);
            iperf_msg.status[0] = ret;
            break;
        case NCP_RSP_WLAN_SOCKET_SENDTO:
            ret                 = wlan_process_wlan_socket_sendto_response(res);
            iperf_msg.status[0] = ret;
            break;
        case NCP_RSP_WLAN_SOCKET_RECV:
            ret                 = wlan_process_wlan_socket_receive_response(res);
            iperf_msg.status[1] = ret;
            break;
        case NCP_RSP_WLAN_SOCKET_RECVFROM:
            ret                 = wlan_process_wlan_socket_recvfrom_response(res);
            iperf_msg.status[1] = ret;
            break;
        case NCP_RSP_11AX_CFG:
            ret = wlan_process_11axcfg_response(res);
            break;
        case NCP_RSP_BTWT_CFG:
            ret = wlan_process_btwt_response(res);
            break;
        case NCP_RSP_TWT_SETUP:
            ret = wlan_process_twt_setup_response(res);
            break;
        case NCP_RSP_TWT_TEARDOWN:
            ret = wlan_process_twt_teardown_response(res);
            break;
        case NCP_RSP_TWT_GET_REPORT:
            ret = wlan_process_twt_report_response(res);
            break;
        case NCP_RSP_11D_ENABLE:
            ret = wlan_process_11d_enable_response(res);
            break;
        case NCP_RSP_REGION_CODE:
            ret = wlan_process_region_code_response(res);
            break;
        case NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_START:
            ret = wlan_process_wlan_uap_prov_start_result_response(res);
            break;
        case NCP_RSP_WLAN_BASIC_WLAN_UAP_PROV_RESET:
            ret = wlan_process_wlan_uap_prov_reset_result_response(res);
            break;
        case NCP_RSP_WLAN_UAP_MAX_CLIENT_CNT:
            ret = wlan_process_client_count_response(res);
            break;
        case NCP_RSP_WLAN_STA_ANTENNA:
            ret = wlan_process_antenna_cfg_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_DEEP_SLEEP_PS:
            ret = wlan_process_deep_sleep_ps_response(res);
            break;
        case NCP_RSP_WLAN_POWERMGMT_IEEE_PS:
            ret = wlan_process_ieee_ps_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_ED_MAC_MODE:
            ret = wlan_process_ed_mac_response(res);
            break;
#if CONFIG_NCP_RF_TEST_MODE
        case NCP_RSP_WLAN_REGULATORY_SET_RF_TEST_MODE:
            ret = wlan_process_set_rf_test_mode_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_TX_ANTENNA:
            ret = wlan_process_set_rf_tx_antenna_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_TX_ANTENNA:
            ret = wlan_process_get_rf_tx_antenna_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_RX_ANTENNA:
            ret = wlan_process_set_rf_rx_antenna_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_RX_ANTENNA:
            ret = wlan_process_get_rf_rx_antenna_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_BAND:
            ret = wlan_process_set_rf_band_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_BAND:
            ret = wlan_process_get_rf_band_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_BANDWIDTH:
            ret = wlan_process_set_rf_bandwidth_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_BANDWIDTH:
            ret = wlan_process_get_rf_bandwidth_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_CHANNEL:
            ret = wlan_process_set_rf_channel_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_CHANNEL:
            ret = wlan_process_get_rf_channel_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_RADIO_MODE:
            ret = wlan_process_set_rf_radio_mode_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_RF_RADIO_MODE:
            ret = wlan_process_get_rf_radio_mode_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_TX_POWER:
            ret = wlan_process_set_rf_tx_power_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_TX_CONT_MODE:
            ret = wlan_process_set_rf_tx_cont_mode_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_SET_RF_TX_FRAME:
            ret = wlan_process_set_rf_tx_frame_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_GET_AND_RESET_RF_PER:
            ret = wlan_process_set_rf_get_and_reset_rf_per_response(res);
            break;
#endif
        case NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_CCMP_128:
            ret = wlan_process_eu_crypto_ccmp128_response(res);
            break;
        case NCP_RSP_WLAN_REGULATORY_EU_CRYPTO_GCMP_128:
            ret = wlan_process_eu_crypto_gcmp128_response(res);
            break;
        case NCP_RSP_WLAN_DEBUG_REGISTER_ACCESS:
            ret = wlan_process_register_access_response(res);
            break;
#if CONFIG_NCP_MEM_MONITOR_DEBUG
        case NCP_RSP_WLAN_MEMORY_HEAP_SIZE:
            ret = wlan_process_memory_state_response(res);
            break;
#endif
        case NCP_RSP_WLAN_STA_SET_MAC:
            ret = wlan_process_set_mac_address(res);
            break;
        case NCP_RSP_WLAN_STA_GET_MAC:
            ret = wlan_process_get_mac_address(res);
            break;
        case NCP_RSP_WLAN_NETWORK_INFO:
            ret = wlan_process_info(res);
            break;
        case NCP_RSP_WLAN_NETWORK_ADDRESS:
            ret = wlan_process_address(res);
            break;
        case NCP_RSP_WLAN_NETWORK_ADD:
            ret = wlan_process_add_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_START:
            ret = wlan_process_start_network_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_STOP:
            ret = wlan_process_stop_network_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_GET_UAP_STA_LIST:
            ret = wlan_process_get_uap_sta_list(res);
            break;
        case NCP_RSP_DATE_TIME:
            ret = wlan_process_time_response(res);
            break;
        case NCP_RSP_GET_TEMPERATUE:
            ret = wlan_process_get_temperature_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_MONITOR:
            ret = wlan_process_monitor_response(res);
            break;
        case NCP_RSP_WLAN_STA_CSI:
            ret = wlan_process_csi_response(res);
            break;
        case NCP_RSP_WLAN_STA_11K_CFG:
            ret = wlan_process_11k_cfg_response(res);
            break;
        case NCP_RSP_WLAN_STA_NEIGHBOR_REQ:
            ret = wlan_process_neighbor_req_response(res);
            break;
        case NCP_RSP_WLAN_STA_WPS_PBC:
            ret = wlan_process_wps_pbc_response(res);
            break;
        case NCP_RSP_WLAN_STA_GEN_WPS_PIN:
            ret = wlan_process_wps_generate_pin_response(res);
            break;
        case NCP_RSP_WLAN_STA_WPS_PIN:
            ret = wlan_process_wps_pin_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_MDNS_QUERY:
            ret = wlan_process_mdns_query_response(res);
            break;
        case NCP_RSP_INVALID_CMD:
            ret = WM_SUCCESS;
            break;
        case NCP_RSP_WLAN_NETWORK_LIST:
            ret = wlan_process_network_list_response(res);
            break;
        case NCP_RSP_WLAN_NETWORK_REMOVE:
            ret = wlan_process_network_remove_response(res);
            break;
        case NCP_RSP_WLAN_MBO_ENABLE:
            ret = wlan_process_mbo_enable_response(res);
            break;
        case NCP_RSP_WLAN_MBO_NONPREFER_CH:
            ret = wlan_process_mbo_nonprefer_ch_response(res);
            break;
        case NCP_RSP_WLAN_MBO_SET_CELL_CAPA:
            ret = wlan_process_mbo_set_cell_capa_response(res);
            break;
        case NCP_RSP_WLAN_MBO_SET_OCE:
            ret = wlan_process_mbo_set_oce_response(res);
            break;
        case NCP_RSP_WLAN_BASIC_WLAN_RESET:
            ret = wlan_process_wlan_reset_response(res);
            break;
        default:
            PRINTF("Invaild response cmd!\r\n");
            break;
    }
    return ret;
}

#if !COFNIG_NCP_SDIO_TEST_LOOPBACK
static struct ncp_host_cli_command ncp_host_app_cli_commands[] = {
    {"wlan-ncp-iperf", NULL, wlan_ncp_iperf_command},
    {"wlan-scan", NULL, wlan_scan_command},
    {"wlan-connect", NULL, wlan_connect_command},
    {"wlan-disconnect", NULL, wlan_disconnect_command},
    {"wlan-get-signal", NULL, wlan_get_signal_command},
    {"wlan-version", NULL, wlan_version_command},
    {"wlan-stat", NULL, wlan_stat_command},
    {"wlan-reset", NULL, wlan_reset_command},
    {"wlan-roaming", NULL, wlan_roaming_command},
    {"wlan-socket-open", NULL, wlan_socket_open_command},
    {"wlan-socket-connect", NULL, wlan_socket_con_command},
    {"wlan-socket-bind", NULL, wlan_socket_bind_command},
    {"wlan-socket-close", NULL, wlan_socket_close_command},
    {"wlan-socket-listen", NULL, wlan_socket_listen_command},
    {"wlan-socket-accept", NULL, wlan_socket_accept_command},
    {"wlan-socket-send", NULL, wlan_socket_send_command},
    {"wlan-socket-sendto", NULL, wlan_socket_sendto_command},
    {"wlan-socket-receive", NULL, wlan_socket_receive_command},
    {"wlan-socket-recvfrom", NULL, wlan_socket_recvfrom_command},
    {"wlan-http-connect", NULL, wlan_http_connect_command},
    {"wlan-http-disconnect", NULL, wlan_http_disconnect_command},
    {"wlan-http-req", NULL, wlan_http_req_command},
    {"wlan-http-recv", NULL, wlan_http_recv_command},
    {"wlan-http-seth", NULL, wlan_http_seth_command},
    {"wlan-http-unseth", NULL, wlan_http_unseth_command},
    {"wlan-websocket-upg", NULL, wlan_websocket_upg_command},
    {"wlan-websocket-send", NULL, wlan_websocket_send_command},
    {"wlan-websocket-recv", NULL, wlan_websocket_recv_command},
    {"wlan-multi-mef", NULL, wlan_multi_mef_command},
    {"wlan-uapsd-enable", NULL, wlan_set_wmm_uapsd_command},
    {"wlan-uapsd-qosinfo", NULL, wlan_wmm_uapsd_qosinfo_command},
    {"wlan-uapsd-sleep-period", NULL, wlan_uapsd_sleep_period_command},
    {"wlan-wakeup-condition", NULL, wlan_wakeup_condition_command},
    {"wlan-suspend", NULL, wlan_suspend_command},
    {"wlan-set-11axcfg", NULL, wlan_set_11axcfg_command},
    {"wlan-uap-prov-start", NULL, wlan_uap_prov_start_command},
    {"wlan-uap-prov-reset", NULL, wlan_uap_prov_reset_command},
    {"wlan-set-btwt-cfg", NULL, wlan_set_btwt_command},
    {"wlan-twt-setup", NULL, wlan_twt_setup_command},
    {"wlan-twt-teardown", NULL, wlan_twt_teardown_command},
    {"wlan-get-twt-report", NULL, wlan_get_twt_report_command},
    {"wlan-set-11d-enable", "sta/uap <state>", wlan_set_11d_enable_command},
    {"wlan-region-code", "get/set <region_code hex>", wlan_region_code_command},
    {"wlan-set-max-clients-count", "<max clients count>", wlan_set_max_clients_count_command},
    {"wlan-set-antenna-cfg", "<antenna mode> <evaluate_time>", wlan_set_antenna_cfg_command},
    {"wlan-get-antenna-cfg", NULL, wlan_get_antenna_cfg_command},
    {"wlan-deep-sleep-ps", "<0/1>", wlan_deep_sleep_ps_command},
    {"wlan-ieee-ps", "<0/1>", wlan_ieee_ps_command},
    {"wlan-reg-access", "<type> <offset> <value>", wlan_register_access_command},
#if CONFIG_NCP_MEM_MONITOR_DEBUG
    {"wlan-mem-stat", NULL, wlan_memory_state_command},
#endif
#if CONFIG_NCP_5GHz_SUPPORT
    {"wlan-set-ed-mac-mode", "<ed_ctrl_2g> <ed_offset_2g> <ed_ctrl_5g> <ed_offset_5g>", wlan_ed_mac_mode_set_command},
#else
    {"wlan-set-ed-mac-mode", "<ed_ctrl_2g> <ed_offset_2g>", wlan_ed_mac_mode_set_command},
#endif
    {"wlan-get-ed-mac-mode", NULL, wlan_ed_mac_mode_get_command},
#if CONFIG_NCP_RF_TEST_MODE
    {"wlan-set-rf-test-mode", NULL, wlan_set_rf_test_mode_command},
    {"wlan-set-rf-tx-antenna", "<antenna>", wlan_set_rf_tx_antenna_command},
    {"wlan-get-rf-tx-antenna", NULL, wlan_get_rf_tx_antenna_command},
    {"wlan-set-rf-rx-antenna", "<antenna>", wlan_set_rf_rx_antenna_command},
    {"wlan-get-rf-rx-antenna", NULL, wlan_get_rf_rx_antenna_command},
    {"wlan-set-rf-band", "<band>", wlan_set_rf_band_command},
    {"wlan-get-rf-band", NULL, wlan_get_rf_band_command},
    {"wlan-set-rf-bandwidth", "<bandwidth>", wlan_set_rf_bandwidth_command},
    {"wlan-get-rf-bandwidth", NULL, wlan_get_rf_bandwidth_command},
    {"wlan-set-rf-channel", "<channel>", wlan_set_rf_channel_command},
    {"wlan-get-rf-channel", NULL, wlan_get_rf_channel_command},
    {"wlan-set-rf-radio-mode", "<radio_mode>", wlan_set_rf_radio_mode_command},
    {"wlan-get-rf-radio-mode", NULL, wlan_get_rf_radio_mode_command},
    {"wlan-set-rf-tx-power", "<tx_power> <modulation> <path_id>", wlan_ncp_set_rf_tx_power_command},
    {"wlan-set-rf-tx-cont-mode", "<enable_tx> <cw_mode> <payload_pattern> <cs_mode> <act_sub_ch> <tx_rate>",
     wlan_ncp_set_rf_tx_cont_mode_command},
    {"wlan-set-rf-tx-frame",
     "<start> <data_rate> <frame_pattern> <frame_len> <adjust_burst_sifs> <burst_sifs_in_us> <short_preamble> "
     "<act_sub_ch> <short_gi> <adv_coding> <tx_bf> <gf_mode> <stbc> <bssid>",
     wlan_ncp_set_rf_tx_frame_command},
    {"wlan-get-and-reset-rf-per", NULL, wlan_ncp_set_rf_get_and_reset_rf_per_command},
#endif
    {"wlan-eu-crypto-ccmp-128", "<EncDec>", wlan_eu_crypto_ccmp128_command},
    {"wlan-eu-crypto-gcmp-128", "<EncDec>", wlan_eu_crypto_gcmp128_command},
    {"wlan-set-mac", "<mac_address>", wlan_set_mac_address_command},
    {"wlan-get-mac", NULL, wlan_get_mac_address_command},
    {"wlan-info", NULL, wlan_info_command},
    {"wlan-address", NULL, wlan_address_command},
    {"wlan-add", NULL, wlan_add_command},
    {"wlan-start-network", "<profile_name>", wlan_start_network_command},
    {"wlan-stop-network", NULL, wlan_stop_network_command},
    {"wlan-get-uap-sta-list", NULL, wlan_get_uap_sta_list_command},
    {"wlan-get-time", NULL, wlan_get_time_command},
    {"wlan-set-time", "<year> <month> <day> <hour> <minute> <second>", wlan_set_time_command},
    {"wlan-get-temp", NULL, wlan_get_temperature_command},
    {"ping", "[-s <packet_size>] [-c <packet_count>] [-W <timeout in sec>] <ipv4 address>", ncp_ping_command},
    {"wlan-net-monitor-cfg", NULL, wlan_net_monitor_cfg_command},
    {"wlan-set-monitor-filter", "<opt> <macaddr>", wlan_set_monitor_filter_command},
    {"wlan-set-monitor-param", "<action> <monitor_activity> <filter_flags> <radio_type> <chan_number>",
     wlan_set_monitor_param_command},
    {"wlan-csi-cfg", NULL, wlan_csi_cfg_command},
    {"wlan-set-csi-param-header",
     " <sta/uap> <csi_enable> <head_id> <tail_id> <chip_id> <band_config> <channel> <csi_monitor_enable> <ra4us>",
     wlan_set_csi_param_header_command},
    {"wlan-set-csi-filter", "<opt> <macaddr> <pkt_type> <type> <flag>", wlan_set_csi_filter_command},
    {"wlan-11k-enable", "<0/1>", wlan_11k_cfg_command},
    {"wlan-11k-neigbor-req", "[ssid <ssid>]", wlan_11k_neighbor_req_command},
    {"wlan-start-wps-pbc", NULL, wlan_start_wps_pbc_command},
    {"wlan-generate-wps-pin", NULL, wlan_wps_generate_pin_command},
    {"wlan-start-wps-pin", "<8 digit pin>", wlan_start_wps_pin_command},
    {"wlan-mdns-query", "<service> <protocol>", wlan_mdns_query_command},
    {"wlan-list", NULL, wlan_list_command},
    {"wlan-remove", "<profile_name>", wlan_remove_command},
#if !(CONFIG_NCP_SUPP)
    {"wlan-mbo-enable", "<0/1>", wlan_mbo_enable_command},
#else
    {"wlan-mbo-set-cell-capa", "<cell capa: 1/2/3(default)>", wlan_mbo_set_cell_capa},
    {"wlan-mbo-set-oce", "<oce: 1(default)/2>", wlan_mbo_set_oce},
#endif
    {"wlan-mbo-nonprefer-ch", "<ch0> <Preference0: 0/1/255> <ch1> <Preference1: 0/1/255>", wlan_mbo_nonprefer_ch},
};
#endif

/**
 * @brief      Register ncp_host_cli commands
 *
 * @return     Status returned
 */
int ncp_host_wifi_command_init()
{
#if !(COFNIG_NCP_SDIO_TEST_LOOPBACK)
    if (ncp_host_cli_register_commands(ncp_host_app_cli_commands,
                                       sizeof(ncp_host_app_cli_commands) / sizeof(struct ncp_host_cli_command)) != 0)
        return -WM_FAIL;
#endif

    return WM_SUCCESS;
}
