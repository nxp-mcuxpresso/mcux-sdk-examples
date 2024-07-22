/** @file ncp_mcu_host_command_ble.c
 *
 *  @brief  This file provides API functions to build tlv commands and process tlv responses.
 *
 *  Copyright 2008-2024 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 *
 */
#if CONFIG_NCP_BLE
#include "ncp_host_command.h"
#include "ncp_host_command_ble.h"
#include "ncp_host_utils_wifi.h"
#include "ncp_debug.h"

#include "ble_service/service.h"
#include "ble_service/ht.h"
#include "ble_service/hr.h"
#include "ble_service/bas.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern uint8_t mcu_tlv_command_buff[NCP_HOST_COMMAND_LEN];

static uint8_t reg_serv[MAX_SUPPORT_SERVICE] = {0};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void htc_central_notify(uint8_t *data);
static void hrc_central_notify(uint8_t *data);
static int ble_set_adv_data_command(int argc, char **argv);
static int ble_start_adv_command(int argc, char **argv);
static int ble_stop_adv_command(int argc, char **argv);
static int ble_set_scan_param_command(int argc, char **argv);
static int ble_start_scan_command(int argc, char **argv);
static int ble_stop_scan_command(int argc, char **argv);
static void ble_connect_command_local(NCP_CMD_CONNECT *param);
static int ble_connect_command(int argc, char **argv);
static int ble_disconnect_command(int argc, char **argv);
static int ble_set_data_len_command(int argc, char **argv);
static int ble_set_phy_command(int argc, char **argv);
static int ble_connect_paramter_update_command(int argc, char **argv);
static int ble_set_filter_list_command(int argc, char **argv);
static int ble_start_encryption_command(int argc, char **argv);
static int ble_read_characteristic_command(int argc, char **argv);
static int ble_register_service_command(int argc, char **argv);
static int ble_set_power_mode_command(int argc, char **argv);
static int ble_set_device_address_command(int argc, char **argv);
//static int ble_set_device_name_command(int argc, char **argv);
static int ble_host_service_add_command(int argc, char **argv);
static int ble_start_service_command(int argc, char **argv);
//static void ble_disc_prim_command_local(NCP_DISC_PRIM_UUID_CMD *param);
//static int ble_disc_prim_command(int argc, char **argv);
//int ble_host_service_discover(int argc, char **argv);
//static void ble_disc_chrc_command_local(NCP_DISC_CHRC_UUID_CMD *param);
//static int ble_disc_chrc_command(int argc, char **argv);
//static void ble_disc_desc_command_local(NCP_DISC_CHRC_UUID_CMD *param);
//static int ble_disc_desc_command(int argc, char **argv);
static int ble_cfg_subscribe_command(int argc, char **argv);
static int ble_l2cap_connection_command(int argc, char **argv);
static int ble_l2cap_disconnect_command(int argc, char **argv);
static int ble_l2cap_send_command(int argc, char **argv);
static int ble_l2cap_register_command(int argc, char **argv);
//static int ble_l2cap_metrics_command(int argc, char **argv);
//static int ble_l2cap_receive_command(int argc, char **argv);
static int ble_process_adv_report(uint8_t *res);
static int ble_process_device_connected(uint8_t *res);
static int ble_process_device_disconnected(uint8_t *res);
static int ble_process_passkey_display(uint8_t *res);
static int ble_process_conn_param_update(uint8_t *res);
static int ble_process_phy_update(uint8_t *res);
static int ble_process_data_len_update(uint8_t *res);
static int ble_process_idenity_resolved(uint8_t *res);
static int ble_process_security_level_changed(uint8_t *res);
static int ble_process_gatt_notification(uint8_t *res);
static int ble_process_attr_value_changed(uint8_t *res);
static int ble_process_gatt_ccc_cfg_changed(uint8_t *res);
static int ble_process_gatt_subscriptioned(uint8_t *res);
static int ble_process_l2cap_connected(uint8_t *res);
static int ble_process_l2cap_disconnected(uint8_t *res);
static int ble_process_l2cap_received(uint8_t *res);
static int ble_process_gatt_prim_discovered(uint8_t *res);
static int ble_process_gatt_chrc_discovered(uint8_t *res);
static int ble_process_gatt_desc_discovered(uint8_t *res);
static int ble_process_start_adv_response(uint8_t *res);
static int ble_process_stop_adv_response(uint8_t *res);
static int ble_process_set_adv_data_response(uint8_t *res);
static int ble_process_set_scan_param_response(uint8_t *res);
static int ble_process_start_scan_response(uint8_t *res);
static int ble_process_stop_scan_response(uint8_t *res);
static int ble_process_connect_response(uint8_t *res);
static int ble_process_disconnect_response(uint8_t *res);
static int ble_process_set_data_len_response(uint8_t *res);
static int ble_process_set_phy_response(uint8_t *res);
static int ble_process_conn_update_response(uint8_t *res);
static int ble_process_set_filter_list_response(uint8_t *res);
static int ble_process_ble_gap_pair_response(uint8_t *res);
static int ble_process_ble_gatt_read_char_response(uint8_t *res);
static int ble_process_set_dev_addr_response(uint8_t *res);
static int ble_process_set_dev_name_response(uint8_t *res);
static int ble_process_set_power_mode_response(uint8_t *res);
static int ble_process_host_service_add_response(uint8_t *res);
static int ble_process_start_service_response(uint8_t *res);
static int ble_process_write_charateristic_response(uint8_t *res);
static int ble_process_disc_prim_response(uint8_t *res);
static int ble_process_disc_chrc_response(uint8_t *res);
static int ble_process_disc_desc_response(uint8_t *res);
static int ble_process_cfg_notify_response(uint8_t *res);
static int ble_process_cfg_indicate_response(uint8_t *res);
static int ble_process_register_service_response(uint8_t *res);
static int ble_process_l2cap_connect_response(uint8_t *res);
static int ble_process_l2cap_disconnect_response(uint8_t *res);
static int ble_process_l2cap_send_response(uint8_t *res);
static int ble_process_l2cap_register_response(uint8_t *res);
static int ble_process_l2cap_metrics_response(uint8_t *res);
static int ble_process_l2cap_receive_response(uint8_t *res);
extern int ncp_host_cli_register_commands(const struct ncp_host_cli_command *commands, int num_commands);

/*******************************************************************************
 * Definitions
 ******************************************************************************/
struct _host_svc_list
{
    const char *name;
    void (*svc_start)(void);
};

struct _host_svc_list host_service_list[] = {
#if CONFIG_NCP_HTS
    {"hts", peripheral_hts_start},
#endif
#if CONFIG_NCP_HTC
    {"htc", central_htc_start},
#endif
#if CONFIG_NCP_HRS
    {"hrs", peripheral_hrs_start},
#endif
#if CONFIG_NCP_HRC
    {"hrc", central_hrc_start},
#endif
#if CONFIG_NCP_BAS
    {"bas", peripheral_bas_start},
#endif
    NULL,
};

/*******************************************************************************
 * Code
 ******************************************************************************/
MCU_NCPCmd_DS_COMMAND *ncp_host_get_cmd_buffer_ble()
{
    return (MCU_NCPCmd_DS_COMMAND *)(mcu_tlv_command_buff);
}


/**
 * @brief This function prepares ble start adv command
 *
 * @return Status returned
 */

static int ble_start_adv_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *start_adv_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)start_adv_command, 0, NCP_HOST_COMMAND_LEN);

    start_adv_command->header.cmd      = NCP_CMD_BLE_GAP_START_ADV;
    start_adv_command->header.size     = NCP_CMD_HEADER_LEN;
    start_adv_command->header.result   = NCP_CMD_RESULT_OK;

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set le adv data command
 *
 * @return Status returned
 */ 
static int ble_set_adv_data_command(int argc, char **argv)
{
    uint32_t i = 0;
    char *endptr;
    char conv_value[2]= {0};
    MCU_NCPCmd_DS_COMMAND *set_adv_data_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_adv_data_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc != 2 || (strlen(argv[1])%2 !=0))
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <adv_data>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    set_adv_data_command->header.cmd      = NCP_CMD_BLE_GAP_SET_ADV_DATA;
    set_adv_data_command->header.size     = NCP_CMD_HEADER_LEN;
    set_adv_data_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SET_ADV_DATA *set_adv_da = (NCP_CMD_SET_ADV_DATA *)&set_adv_data_command->params.set_adv_data;

    set_adv_da->adv_length = strlen(argv[1])/2;

    for (i = 0; i < set_adv_da->adv_length * 2; i = i+2)
    {
      memcpy(conv_value, (char *)&argv[1][i], 2);
      set_adv_da->adv_data[i/2] = strtol(conv_value, &endptr, 16);
    }
    
    set_adv_data_command->header.size += set_adv_da->adv_length + 1;

    return NCP_STATUS_SUCCESS;
}


/**
 * @brief This function prepares stop adv command
 *
 * @return Status returned
 */
static int ble_stop_adv_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *stop_adv_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)stop_adv_command, 0, NCP_HOST_COMMAND_LEN);

    stop_adv_command->header.cmd      = NCP_CMD_BLE_GAP_STOP_ADV;
    stop_adv_command->header.size     = NCP_CMD_HEADER_LEN;
    stop_adv_command->header.result   = NCP_CMD_RESULT_OK;

    return NCP_STATUS_SUCCESS;
}


/**
 * @brief This function prepares set le scan parameter command
 *
 * @return Status returned
 */
static int ble_set_scan_param_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *set_scan_para_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_scan_para_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc != 4)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <filter_option> <interval> <window>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    set_scan_para_command->header.cmd      = NCP_CMD_BLE_GAP_SET_SCAN_PARAM;
    set_scan_para_command->header.size     = NCP_CMD_HEADER_LEN;
    set_scan_para_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SET_SCAN_PARAM *set_scan_param = (NCP_CMD_SET_SCAN_PARAM *)&set_scan_para_command->params.set_scan_parameter;

    set_scan_param->options = atoi(argv[1]);
    set_scan_param->interval = atoi(argv[2]);
    set_scan_param->window = atoi(argv[3]);

    set_scan_para_command->header.size += sizeof(NCP_CMD_SET_SCAN_PARAM);

    return NCP_STATUS_SUCCESS;
}
/**
 * @brief This function prepares start scan command
 *
 * @return Status returned
 */
static int ble_start_scan_command(int argc, char **argv)
{
    int scan_type;
    MCU_NCPCmd_DS_COMMAND *start_scan_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)start_scan_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc != 2 || (atoi(argv[1]) != 0 && atoi(argv[1]) != 1))
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <0/1> <0 -- active scan; 1 -- passive scan>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    scan_type = atoi(argv[1]);
    start_scan_command->header.cmd      = NCP_CMD_BLE_GAP_START_SCAN;
    start_scan_command->header.size     = NCP_CMD_HEADER_LEN;
    start_scan_command->header.result   = NCP_CMD_RESULT_OK;
    
    NCP_CMD_SCAN_START *scan_start = (NCP_CMD_SCAN_START *)&start_scan_command->params.scan_start;
    if(scan_type == 0)
    {
        scan_start->type = 0x09; // active scan
    }
    else
    {
        scan_start->type = 0x01; // passive scan
    }

    start_scan_command->header.size += sizeof(NCP_CMD_SCAN_START);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares stop scan command
 *
 * @return Status returned
 */
static int ble_stop_scan_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *stop_scan_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)stop_scan_command, 0, NCP_HOST_COMMAND_LEN);

    stop_scan_command->header.cmd      = NCP_CMD_BLE_GAP_STOP_SCAN;
    stop_scan_command->header.size     = NCP_CMD_HEADER_LEN;
    stop_scan_command->header.result   = NCP_CMD_RESULT_OK;

    return NCP_STATUS_SUCCESS;
}

static void ble_connect_command_local(NCP_CMD_CONNECT *param)
{
    MCU_NCPCmd_DS_COMMAND *connect_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)connect_command, 0, NCP_HOST_COMMAND_LEN);
    connect_command->header.cmd      = NCP_CMD_BLE_GAP_CONNECT;
    connect_command->header.size     = NCP_CMD_HEADER_LEN;
    connect_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_CONNECT *connect = (NCP_CMD_CONNECT *)&connect_command->params.connect;
    memcpy(connect, param, sizeof(NCP_CMD_CONNECT));

    connect_command->header.size += sizeof(NCP_CMD_CONNECT);
}

/**
 * @brief This function prepares create a LE link command
 *
 * @return Status returned
 */
static int ble_connect_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];

    NCP_CMD_CONNECT connect;


    if(argc != 3)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
      printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    connect.type = type;
    memcpy(connect.addr, raw_addr, NCP_BLE_ADDR_LENGTH);

    ble_connect_command_local(&connect);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares terminated a LE link command
 *
 * @return Status returned
 */
static int ble_disconnect_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];

    MCU_NCPCmd_DS_COMMAND *disconnect_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)disconnect_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc != 3)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    disconnect_command->header.cmd      = NCP_CMD_BLE_GAP_DISCONNECT;
    disconnect_command->header.size     = NCP_CMD_HEADER_LEN;
    disconnect_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_CONNECT *connect = (NCP_CMD_CONNECT *)&disconnect_command->params.connect;

    connect->type = type;
    memcpy(connect->addr, raw_addr, NCP_BLE_ADDR_LENGTH);

    disconnect_command->header.size += sizeof(NCP_CMD_CONNECT);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set le data len command
 *
 * @return Status returned
 */
static int ble_set_data_len_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *set_data_len_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_data_len_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc < 4 || argc > 5)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <tx_max_len> [optional<tx_max_time>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    set_data_len_command->header.cmd      = NCP_CMD_BLE_GAP_SET_DATA_LEN;
    set_data_len_command->header.size     = NCP_CMD_HEADER_LEN;
    set_data_len_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SET_DATA_LEN *set_le_data_len = (NCP_CMD_SET_DATA_LEN *)&set_data_len_command->params.set_data_len;

    set_le_data_len->address_type = type;
    memcpy(set_le_data_len->address, raw_addr, NCP_BLE_ADDR_LENGTH);
    set_le_data_len->tx_max_len = atoi(argv[3]);
    if (argc == 5)
    {
      set_le_data_len->time_flag = 1;
      set_le_data_len->tx_max_time = atoi(argv[4]);
    }
    else
    {
      set_le_data_len->time_flag = 0;
      set_le_data_len->tx_max_time = 0;
    }

    set_data_len_command->header.size += sizeof(NCP_CMD_SET_DATA_LEN);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set le phy command
 *
 * @return Status returned
 */
static int ble_set_phy_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *set_phy_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_phy_command, 0, NCP_HOST_COMMAND_LEN);

    if(argc != 5)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <tx_phy> <rx_phy>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    set_phy_command->header.cmd      = NCP_CMD_BLE_GAP_SET_PHY;
    set_phy_command->header.size     = NCP_CMD_HEADER_LEN;
    set_phy_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_CMD_SET_PHY *set_le_phy = (NCP_CMD_SET_PHY *)&set_phy_command->params.set_phy;

    set_le_phy->address_type = type;
    memcpy(set_le_phy->address, raw_addr, NCP_BLE_ADDR_LENGTH);
    set_le_phy->pref_tx_phy = atoi(argv[3]);
    set_le_phy->pref_rx_phy = set_le_phy->pref_tx_phy;
    set_le_phy->options = 0;

    for (size_t argn = 4; argn < argc; argn++) {
        const char *arg = argv[argn];

        if (!strcmp(arg, "s2")) {
            set_le_phy->options |= 0x0001;
        } else if (!strcmp(arg, "s8")) {
            set_le_phy->options |= 0x0010;
        } else {
            set_le_phy->pref_rx_phy = atoi(argv[4]);
        }
    }

    set_phy_command->header.size += sizeof(NCP_CMD_SET_PHY);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares LE connect paramter update command
 *
 * @return Status returned
 */
static int ble_connect_paramter_update_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    uint16_t conn_interval_min;
    uint16_t conn_interval_max;
    uint16_t conn_latency;
    uint16_t conn_timeout;

    MCU_NCPCmd_DS_COMMAND *conn_param_update_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)conn_param_update_command, 0, NCP_HOST_COMMAND_LEN);

    conn_param_update_command->header.cmd      = NCP_CMD_BLE_GAP_CONN_PARAM_UPDATE;
    conn_param_update_command->header.size     = NCP_CMD_HEADER_LEN;
    conn_param_update_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 7)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] [interval_min<xxxx>] [interval_max<xxxx>] [latency<xxxx>] [timeout<xxxx>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid type argument\r\n");
        return -NCP_STATUS_ERROR;
    }
    conn_interval_min = atoi(argv[3]);
    conn_interval_max = atoi(argv[4]);
    conn_latency = atoi(argv[5]);
    conn_timeout = atoi(argv[6]);

    NCP_CMD_CONN_PARA_UPDATE *connect_param_update = (NCP_CMD_CONN_PARA_UPDATE *)&conn_param_update_command->params.conn_param_update;

    connect_param_update->type = type;
    memcpy(connect_param_update->addr, raw_addr, NCP_BLE_ADDR_LENGTH);
    connect_param_update->interval_min = conn_interval_min;
    connect_param_update->interval_max = conn_interval_max;
    connect_param_update->latency = conn_latency;
    connect_param_update->timeout = conn_timeout;

    conn_param_update_command->header.size += sizeof(NCP_CMD_CONN_PARA_UPDATE);
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set filter list command
 *
 * @return Status returned
 */
static int ble_set_filter_list_command(int argc, char **argv)
{
    int i = 0;
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *set_filter_list_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_filter_list_command, 0, NCP_HOST_COMMAND_LEN);

    set_filter_list_command->header.cmd      = NCP_CMD_BLE_GAP_SET_FILTER_LIST;
    set_filter_list_command->header.size     = NCP_CMD_HEADER_LEN;
    set_filter_list_command->header.result   = NCP_CMD_RESULT_OK;

    if((argc != (atoi(argv[1]) * 2 + 2)) || (atoi(argv[1]) <= 0))
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [filter_addr_num <xx>] [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] ... [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>], such as:  %s 2 public 00:11:22:33:44:55 public 00:11:22:33:44:66\r\n", argv[0], argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ((uint8_t *)set_filter_list_command)[sizeof(NCP_HOST_COMMAND)]= atoi(argv[1]);
    for (i = 0; i< atoi(argv[1]); i++)
    {
      ret = get_mac(argv[3 + 2 * i], (char *)raw_addr, ':');
      if (ret != 0)
      {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
      }

      if (!strncmp(argv[2 + 2 * i], "public", 7))
        type = 0x00;
      else if (!strncmp(argv[2 + 2 * i], "random", 7))
        type = 0x01;
      else
      {
        printf("Error: invalid type argument\r\n");
        return -NCP_STATUS_ERROR;
      }
      ((uint8_t *)set_filter_list_command)[sizeof(NCP_HOST_COMMAND) + 1 + (1 + 6) * i] = type;
      memcpy((uint8_t *)(&(((uint8_t *)set_filter_list_command)[sizeof(NCP_HOST_COMMAND) + 2 + (1 + 6) * i])), raw_addr, NCP_BLE_ADDR_LENGTH);
    }

    set_filter_list_command->header.size += (1 + atoi(argv[1]) * 7);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares start encryption command
 *
 * @return Status returned
 */
static int ble_start_encryption_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *start_encryption_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)start_encryption_command, 0, NCP_HOST_COMMAND_LEN);

    start_encryption_command->header.cmd      = NCP_CMD_BLE_GAP_PAIR;
    start_encryption_command->header.size     = NCP_CMD_HEADER_LEN;
    start_encryption_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 3)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    NCP_CMD_ENCRYPTION *connect_encryption = (NCP_CMD_ENCRYPTION *)&start_encryption_command->params.conn_encryption;

    connect_encryption->type = type;
    memcpy(connect_encryption->addr, raw_addr, NCP_BLE_ADDR_LENGTH);

    start_encryption_command->header.size += sizeof(NCP_CMD_ENCRYPTION);
    return NCP_STATUS_SUCCESS;
}


void write_charateristic_command_local(NCP_SET_VALUE_CMD *param)
{
    MCU_NCPCmd_DS_COMMAND *write_characteristic_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)write_characteristic_command, 0, NCP_HOST_COMMAND_LEN);

    write_characteristic_command->header.cmd      = NCP_CMD_BLE_GATT_SET_VALUE;
    write_characteristic_command->header.size     = NCP_CMD_HEADER_LEN;
    write_characteristic_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_SET_VALUE_CMD *gatt_set_value = (NCP_SET_VALUE_CMD *)&write_characteristic_command->params.gatt_set_value;

    memcpy(gatt_set_value, param, sizeof(NCP_SET_VALUE_CMD));

    write_characteristic_command->header.size += (gatt_set_value->len + 19);
}
/**
 * @brief This function prepares write characteristic command
 *
 * @return Status returned
 */
static int ble_write_characteristic_command(int argc, char **argv)
{
    uint8_t uuid_length = 0;
    uint8_t uuid[SERVER_MAX_UUID_LEN] = {0};

    NCP_SET_VALUE_CMD gatt_set_value;

    if (argc < 4)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage:\r\n");
        printf("      ble-set-value <uuid_len> <uuid> <value_len> <value ...>\r\n");
        printf("      uuid_len:   2 = UUID16, 10 = UUID128\r\n");
        printf("      uuid:       XXXX for UUID16\r\n");
        printf("                  XXXX-XX-XX-XX-XXXXXX for UUID128\r\n");
        printf("      value_len:   1 - 512\r\n");
        printf("      value:\r\n");
        return -NCP_STATUS_ERROR;
    }


    uuid_length = a2hex(argv[1]);
    if ((uuid_length == 2) || (uuid_length == 16))
    {
        uuid2arry(argv[2], uuid, uuid_length);
        memcpy(gatt_set_value.uuid, uuid, uuid_length);
    }
    else
    {
        printf("Error: invalid uuid_length value\r\n");
        return -NCP_STATUS_ERROR;
    }

    for(uint8_t i = 4; i < argc; i ++)
    {
        gatt_set_value.value[i-4] = a2hex(argv[i]);
    }

    gatt_set_value.len = a2hex(argv[3]);
    gatt_set_value.uuid_length = uuid_length;

    write_charateristic_command_local(&gatt_set_value);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares read characteristic command
 *
 * @return Status returned
 */
static int ble_read_characteristic_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *read_characteristic_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)read_characteristic_command, 0, NCP_HOST_COMMAND_LEN);

    read_characteristic_command->header.cmd      = NCP_CMD_BLE_GATT_READ;
    read_characteristic_command->header.size     = NCP_CMD_HEADER_LEN;
    read_characteristic_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 4)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <handle>\r\n", argv[0]);
        printf("      handle:     XXXX\r\n");
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    NCP_GATT_READ_CMD *read_characteristic = (NCP_GATT_READ_CMD *)&read_characteristic_command->params.gatt_read_char;

    read_characteristic->type = type;
    memcpy(read_characteristic->addr, raw_addr, NCP_BLE_ADDR_LENGTH);
    read_characteristic->handle= a2hex(argv[3]);

    read_characteristic_command->header.size += sizeof(NCP_GATT_READ_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares register service command
 *
 * @return Status returned
 */
static int ble_register_service_command(int argc, char **argv)
{
    uint8_t service_len = 0;
    uint8_t service_id[MAX_SUPPORT_SERVICE] = {0};

    MCU_NCPCmd_DS_COMMAND *register_service_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)register_service_command, 0, NCP_HOST_COMMAND_LEN);

    if (argc < 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage:\r\n");
        printf("      ble-register-service <num_of_service> <service id_1> <service id_2> ...\r\n");
        printf("      service id:  1  Peripheral_HTS\r\n");
        printf("                   2  Peripheral_HRS\r\n");
        printf("                   3  BAS\r\n");
        printf("                   4  Central_HTS\r\n");
        printf("                   5  Central_HRS\r\n");
        return -NCP_STATUS_ERROR;
    }

    register_service_command->header.cmd      = NCP_CMD_BLE_GATT_REGISTER_SERVICE;
    register_service_command->header.size     = NCP_CMD_HEADER_LEN;
    register_service_command->header.result   = NCP_CMD_RESULT_OK;

    NCP_REGISTER_SERVICE *register_service = (NCP_REGISTER_SERVICE *)&register_service_command->params.register_service;
    
    service_len = atoi(argv[1]);
    for(uint8_t i = 2; i < argc; i++)
    {
        service_id[i-2] = atoi(argv[i]);
        reg_serv[i-2]   = service_id[i-2];
    }

    memcpy(register_service->service, service_id, service_len);
    register_service->svc_length = service_len;

    register_service_command->header.size += (register_service->svc_length + 1);
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set power mode command
 *
 * @return Status returned
 */
static int ble_set_power_mode_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *set_power_mode_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_power_mode_command, 0, NCP_HOST_COMMAND_LEN);

    set_power_mode_command->header.cmd      = NCP_CMD_BLE_VENDOR_POWER_MODE;
    set_power_mode_command->header.size     = NCP_CMD_HEADER_LEN;
    set_power_mode_command->header.result   = NCP_CMD_RESULT_OK;

    if (argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: ble-set-power-mode <0/1>\r\n");
        return -NCP_STATUS_ERROR;
    }

    if ((atoi(argv[1]) != 0) && (atoi(argv[1]) != 1))
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: ble-set-power-mode <0/1>\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_CMD_SET_POWER_MODE *set_power_mode = (NCP_CMD_SET_POWER_MODE *)&set_power_mode_command->params.set_pw_mode;

    set_power_mode->mode = atoi(argv[1]);

    set_power_mode_command->header.size += sizeof(NCP_CMD_SET_POWER_MODE);
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares set device address command
 *
 * @return Status returned
 */
static int ble_set_device_address_command(int argc, char **argv)
{
    int ret;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *set_device_address_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_device_address_command, 0, NCP_HOST_COMMAND_LEN);

    set_device_address_command->header.cmd      = NCP_CMD_BLE_VENDOR_SET_DEVICE_ADDR;
    set_device_address_command->header.size     = NCP_CMD_HEADER_LEN;
    set_device_address_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[1], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_CMD_SET_ADDR *set_device_addr = (NCP_CMD_SET_ADDR *)&set_device_address_command->params.conn_encryption;

    memcpy(set_device_addr->addr, raw_addr, NCP_BLE_ADDR_LENGTH);

    set_device_address_command->header.size += sizeof(NCP_CMD_SET_ADDR);

    return NCP_STATUS_SUCCESS;
}

#if 0
/**
 * @brief This function prepares set device name command
 *
 * @return Status returned
 */
static int ble_set_device_name_command(int argc, char **argv)
{
    int len;
    MCU_NCPCmd_DS_COMMAND *set_device_name_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)set_device_name_command, 0, NCP_HOST_COMMAND_LEN);

    set_device_name_command->header.cmd      = NCP_CMD_BLE_VENDOR_SET_DEVICE_NAME;
    set_device_name_command->header.size     = NCP_CMD_HEADER_LEN;
    set_device_name_command->header.result   = NCP_CMD_RESULT_OK;
    set_device_name_command->header.msg_type = NCP_MSG_TYPE_CMD;

    if(argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <name string>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    len = strlen(argv[1]);

    if (len > NCP_BLE_DEVICE_NAME_MAX)
    {
        printf("Error: name greater than 32 bytes\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_CMD_SET_NAME *set_device_name = (NCP_CMD_SET_NAME *)&set_device_name_command->params.set_dev_name;

    memcpy(set_device_name->name,argv[1], len + 1);

    set_device_name_command->header.size += (len + 1);

    return NCP_STATUS_SUCCESS;
}
#endif

static void dump_ble_host_service_add_usage()
{
    (void)PRINTF("Usage:\r\n");
    (void)PRINTF("For add Primary Service, and Characteristic, Client Characteristic Configuration to Host Service: \r\n");
    (void)PRINTF(
        "    ble-host-svc-add prim <uuid> chrc <uuid> <Properties> <Permissions> [ccc <Permissions>]"
        "\r\n");
    (void)PRINTF(" If want auto start host Service after added: \r\n");
    (void)PRINTF(
        "    ble-host-svc-add prim <uuid> chrc <uuid> <Properties> <Permissions> [ccc <Permissions>] start"
        "\r\n");
    (void)PRINTF(" If want add mutli Characteristic: \r\n");
    (void)PRINTF(
        "    ble-host-svc-add prim <uuid> chrc <uuid> <Properties> <Permissions> chrc <uuid> <Properties> <Permissions> ..."
        "\r\n");
    (void)PRINTF(" For <Properties>: \r\n");
    (void)PRINTF("   BIT 1:	Broadcast\r\n");
    (void)PRINTF("   BIT 2:	Read\r\n");
    (void)PRINTF("   BIT 3:	Write Without Response\r\n");
    (void)PRINTF("   BIT 4:	Write\r\n");
    (void)PRINTF("   BIT 5:	Notify\r\n");
    (void)PRINTF("   BIT 6:	Indicate\r\n");
    (void)PRINTF("   BIT 7:	Authenticated Signed Writes\r\n");
    (void)PRINTF("   BIT 8:	Extended Properties\r\n");
    (void)PRINTF(" For <Permissions>: \r\n");
    (void)PRINTF("   0:	None\r\n");
    (void)PRINTF("   BIT 0: Read\r\n");
    (void)PRINTF("   BIT 1: Write\r\n");
    (void)PRINTF("   BIT 2: Read with Encryption\r\n");
    (void)PRINTF("   BIT 3: Write with Encryption\r\n");
    (void)PRINTF("   BIT 4: Read with Authentication\r\n");
    (void)PRINTF("   BIT 5: Write with Authentication\r\n");
    (void)PRINTF("   BIT 6: Prepare Write\r\n");
    (void)PRINTF("   BIT 7: Read with LE Secure Connection encryption\r\n");
    (void)PRINTF("   BIT 8: Write with LE Secure Connection encryption\r\n");

}

/**
 * @brief This function prepares add service attributes from host to device command
 *
 * @return Status returned
 */
static int ble_host_service_add_command(int argc, char **argv)
{
    int arg = 1;


    MCU_NCPCmd_DS_COMMAND *host_service_add_command = ncp_host_get_cmd_buffer_ble();
    NCP_CMD_SERVICE_ADD *host_service_add_tlv       = (NCP_CMD_SERVICE_ADD *)&host_service_add_command->params.host_svc_add;
    uint8_t *ptlv_pos                               = host_service_add_tlv->tlv_buf;
    uint32_t tlv_buf_len                            = 0;
    gatt_add_service_cmd_t *add_service_tlv         = NULL;
    gatt_add_characteristic_cmd_t *add_chrc_tlv     = NULL;
    gatt_add_descriptor_cmd_t *add_desc_tlv         = NULL;
    gatt_start_service_cmd_t *start_service_tlv     = NULL;

    if (argc < 3)
    {
        dump_ble_host_service_add_usage();
        (void)PRINTF("Error: invalid number of arguments\r\n");
        return NCP_STATUS_ERROR;
    }

    do
    {
        if (string_equal("prim", argv[arg]) || string_equal("primary", argv[arg]))
        {
            add_service_tlv = (gatt_add_service_cmd_t *)ptlv_pos;

            add_service_tlv->type = 0;// primary service
            add_service_tlv->uuid_length = strlen(argv[arg + 1])/2;

            if (uuid2arry(argv[arg + 1], add_service_tlv->uuid, add_service_tlv->uuid_length))
            {
                (void)PRINTF("Error: invalid uuid format\r\n");
                (void)PRINTF("For uuid format:\r\n");
                (void)PRINTF("  XXXX for UUID16\r\n");
                (void)PRINTF("  XXX-XX-XX-XX-XXXXXX for UUID128\r\n");
                return NCP_STATUS_ERROR;
            }
            add_service_tlv->header.type = NCP_CMD_GATT_ADD_SERVICE_TLV;

            add_service_tlv->header.size = sizeof(gatt_add_service_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_service_tlv->uuid_length);
            ptlv_pos += add_service_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_service_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg += 2;
        }
        else if (string_equal("chrc", argv[arg]) || string_equal("characteristic", argv[arg]))
        {
            add_chrc_tlv = (gatt_add_characteristic_cmd_t *)ptlv_pos;

            add_chrc_tlv->svc_id = 0;
            add_chrc_tlv->uuid_length = strlen(argv[arg + 1])/2;

            if (uuid2arry(argv[arg + 1], add_chrc_tlv->uuid, add_chrc_tlv->uuid_length))
            {
                (void)PRINTF("Error: invalid uuid format\r\n");
                (void)PRINTF("For uuid format:\r\n");
                (void)PRINTF("  XXXX for UUID16\r\n");
                (void)PRINTF("  XXX-XX-XX-XX-XXXXXX for UUID128\r\n");
                return NCP_STATUS_ERROR;
            }
            
            // if (a2hex(argv[arg + 2]) > (1 << 8))
            // {
            //     (void)PRINTF("Error: invalid properties\r\n");
            //     return NCP_STATUS_ERROR;
            // }
            add_chrc_tlv->properties = a2hex(argv[arg + 2]);

            // if (a2hex(argv[arg + 3]) > (1 << 8))
            // {
            //     (void)PRINTF("Error: invalid permissions\r\n");
            //     return NCP_STATUS_ERROR;
            // }
            add_chrc_tlv->permissions =  a2hex(argv[arg + 3]);

            add_chrc_tlv->header.type = NCP_CMD_GATT_ADD_CHRC_TLV;
            add_chrc_tlv->header.size = sizeof(gatt_add_characteristic_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_chrc_tlv->uuid_length);
            ptlv_pos += add_chrc_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_chrc_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg += 4;
        }
        else if (string_equal("ccc", argv[arg]))
        {
            add_desc_tlv = (gatt_add_descriptor_cmd_t *)ptlv_pos;

            add_desc_tlv->char_id = 0;
            add_desc_tlv->uuid_length = strlen(argv[arg + 1])/2;

            if (uuid2arry(argv[arg + 1], add_desc_tlv->uuid, add_desc_tlv->uuid_length))
            {
                (void)PRINTF("Error: invalid uuid format\r\n");
                (void)PRINTF("For uuid format:\r\n");
                (void)PRINTF("  XXXX for UUID16\r\n");
                (void)PRINTF("  XXX-XX-XX-XX-XXXXXX for UUID128\r\n");
                return NCP_STATUS_ERROR;
            }

            // if (a2hex(argv[arg + 2]) > (1 << 8))
            // {
            //     (void)PRINTF("Error: invalid permissions\r\n");
            //     return NCP_STATUS_ERROR;
            // }
            add_chrc_tlv->permissions =  a2hex(argv[arg + 2]);

            add_desc_tlv->header.type = NCP_CMD_GATT_ADD_DESC_TLV;
            add_desc_tlv->header.size = sizeof(gatt_add_descriptor_cmd_t) - NCP_TLV_HEADER_LEN - (SERVER_MAX_UUID_LEN - add_desc_tlv->uuid_length);
            ptlv_pos += add_desc_tlv->header.size + NCP_TLV_HEADER_LEN;
            tlv_buf_len += add_desc_tlv->header.size + NCP_TLV_HEADER_LEN;
            arg += 3;
        }
        else if (string_equal("start", argv[arg]))
        {
            start_service_tlv = (gatt_start_service_cmd_t *)ptlv_pos;

            start_service_tlv->started = 1;
            start_service_tlv->header.type = NCP_CMD_GATT_START_SVC_TLV;

            start_service_tlv->header.size = sizeof(gatt_start_service_cmd_t) - NCP_TLV_HEADER_LEN;
            ptlv_pos += sizeof(gatt_start_service_cmd_t);
            tlv_buf_len += sizeof(gatt_start_service_cmd_t);
            arg += 2;
        }
        else
        {
            dump_ble_host_service_add_usage();
            (void)PRINTF("Error: argument %d is invalid\r\n", arg);
            return NCP_STATUS_ERROR;
        }
    } while (arg < argc);
    
    host_service_add_tlv->tlv_buf_len = tlv_buf_len;

    host_service_add_command->header.cmd      = NCP_CMD_BLE_HOST_SERVICE_ADD;
    host_service_add_command->header.size     = NCP_CMD_HEADER_LEN + sizeof(host_service_add_tlv->tlv_buf_len) + tlv_buf_len;
    host_service_add_command->header.result   = NCP_CMD_RESULT_OK;

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares a command to start ble profile at host side
 *
 * @return Status returned
 */
static int ble_start_service_command(int argc, char **argv)
{

    MCU_NCPCmd_DS_COMMAND *start_service_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)start_service_command, 0, NCP_HOST_COMMAND_LEN);


    if (argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [<profile_name>]\r\n", argv[0]);
        printf("       profile name: hts\r\n");
        printf("                     htc\r\n");
        printf("                     hrs\r\n");
        printf("                     hrc\r\n");
        printf("                     bas\r\n");
    }

    for (uint8_t i = 0; i < ARRAY_SIZE(host_service_list); i++)
    {
        if (strcmp(host_service_list[i].name, argv[1]) == 0)
        {
            if (host_service_list[i].svc_start)
            {
                host_service_list[i].svc_start(); 
                return NCP_STATUS_SUCCESS;
            }
        }
    }

    printf("Error: Cannot find the profile\r\n");

    // {
    //     start_service_command->header.cmd      = NCP_CMD_BLE_GATT_START_SERVICE;
    //     start_service_command->header.size     = NCP_CMD_HEADER_LEN;
    //     start_service_command->header.result   = NCP_CMD_RESULT_OK;
    //     start_service_command->header.msg_type = NCP_MSG_TYPE_CMD;
    // }

    return NCP_STATUS_SUCCESS;
}

#if 0
static void ble_disc_prim_command_local(NCP_DISC_PRIM_UUID_CMD *param)
{
    MCU_NCPCmd_DS_COMMAND *ble_disc_prim_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)ble_disc_prim_command, 0, NCP_HOST_COMMAND_LEN);

    ble_disc_prim_command->header.cmd      = NCP_CMD_BLE_GATT_DISC_PRIM;
    ble_disc_prim_command->header.size     = NCP_CMD_HEADER_LEN;
    ble_disc_prim_command->header.result   = NCP_CMD_RESULT_OK;
    ble_disc_prim_command->header.msg_type = NCP_MSG_TYPE_CMD;

    NCP_DISC_CHRC_UUID_CMD *ble_disc_prim = (NCP_DISC_CHRC_UUID_CMD *)&ble_disc_prim_command->params.discover_prim;
    memcpy(ble_disc_prim, param, sizeof(NCP_DISC_CHRC_UUID_CMD));

    ble_disc_prim_command->header.size += sizeof(NCP_DISC_CHRC_UUID_CMD);
}

/**
 * @brief This function prepares discover primary service command
 *
 * @return Status returned
 */
static int ble_disc_prim_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    uint8_t uuid_length = 0;
    uint8_t uuid[SERVER_MAX_UUID_LEN] = {0};

    NCP_DISC_PRIM_UUID_CMD discover_prim;

    if(argc != 5)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <uuid_len> <uuid>\r\n", argv[0]);
        printf("      uuid_len: 2 = UUID16, 10 = UUID128\r\n");
        printf("      uuid:     XXXX for UUID16\r\n");
        printf("                XXXX-XX-XX-XX-XXXXXX for UUID128\r\n");
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    uuid_length = a2hex(argv[3]);

    if ((uuid_length == 2) || (uuid_length == 16))
    {
        uuid2arry(argv[4], uuid, uuid_length);
        memcpy(discover_prim.uuid, uuid, uuid_length);
    }
    else
    {
        printf("Error: invalid uuid_length value\r\n");
        return -NCP_STATUS_ERROR;
    }

    discover_prim.address_type = type;
    memcpy(discover_prim.address, raw_addr, NCP_BLE_ADDR_LENGTH);
    discover_prim.uuid_length = uuid_length;

    ble_disc_prim_command_local(&discover_prim);

    return NCP_STATUS_SUCCESS;
}

static void ble_disc_chrc_command_local(NCP_DISC_CHRC_UUID_CMD *param)
{
    MCU_NCPCmd_DS_COMMAND *ble_disc_chrc_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)ble_disc_chrc_command, 0, NCP_HOST_COMMAND_LEN);

    ble_disc_chrc_command->header.cmd      = NCP_CMD_BLE_GATT_DISC_CHRC;
    ble_disc_chrc_command->header.size     = NCP_CMD_HEADER_LEN;
    ble_disc_chrc_command->header.result   = NCP_CMD_RESULT_OK;
    ble_disc_chrc_command->header.msg_type = NCP_MSG_TYPE_CMD;

    NCP_DISC_CHRC_UUID_CMD *ble_disc_chrc = (NCP_DISC_CHRC_UUID_CMD *)&ble_disc_chrc_command->params.discover_chrc;
    memcpy(ble_disc_chrc, param, sizeof(NCP_DISC_CHRC_UUID_CMD));

    ble_disc_chrc_command->header.size += sizeof(NCP_DISC_CHRC_UUID_CMD);
}

/**
 * @brief This function prepares discover characteristics command
 *
 * @return Status returned
 */
static int ble_disc_chrc_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    uint8_t uuid_length = 0;
    uint8_t uuid[SERVER_MAX_UUID_LEN] = {0};
    uint16_t start_handle = 0;
    uint16_t end_handle = 0;

    NCP_DISC_CHRC_UUID_CMD discover_chrc;

    if(argc != 7)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <uuid_len> <uuid> <start_handle> <end_handle>\r\n", argv[0]);
        printf("      uuid_len: 2 = UUID16, 10 = UUID128\r\n");
        printf("      uuid:     XXXX for UUID16\r\n");
        printf("                XXXX-XX-XX-XX-XXXXXX for UUID128\r\n");
        printf("      start_handle:   XXXX\r\n");
        printf("      end_handle:     XXXX\r\n");
        return -NCP_STATUS_ERROR;
    }

    start_handle = a2hex(argv[5]);
    end_handle = a2hex(argv[6]);

    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    uuid_length = a2hex(argv[3]);

    if ((uuid_length == 2) || (uuid_length == 16))
    {
        uuid2arry(argv[4], uuid, uuid_length);
        memcpy(discover_chrc.uuid, uuid, uuid_length);
    }
    else
    {
        printf("Error: invalid uuid_length value\r\n");
        return -NCP_STATUS_ERROR;
    }

    discover_chrc.address_type = type;
    memcpy(discover_chrc.address, raw_addr, NCP_BLE_ADDR_LENGTH);
    discover_chrc.uuid_length = uuid_length;
    discover_chrc.start_handle = start_handle;
    discover_chrc.end_handle = end_handle;

    ble_disc_chrc_command_local(&discover_chrc);

    return NCP_STATUS_SUCCESS;
}

static void ble_disc_desc_command_local(NCP_DISC_CHRC_UUID_CMD *param)
{
    MCU_NCPCmd_DS_COMMAND *ble_disc_desc_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)ble_disc_desc_command, 0, NCP_HOST_COMMAND_LEN);

    ble_disc_desc_command->header.cmd      = NCP_CMD_BLE_GATT_DESC_CHRC;
    ble_disc_desc_command->header.size     = NCP_CMD_HEADER_LEN;
    ble_disc_desc_command->header.result   = NCP_CMD_RESULT_OK;
    ble_disc_desc_command->header.msg_type = NCP_MSG_TYPE_CMD;

    NCP_DISC_CHRC_UUID_CMD *ble_disc_desc = (NCP_DISC_CHRC_UUID_CMD *)&ble_disc_desc_command->params.discover_chrc;
    memcpy(ble_disc_desc, param, sizeof(NCP_DISC_CHRC_UUID_CMD));

    ble_disc_desc_command->header.size += sizeof(NCP_DISC_CHRC_UUID_CMD);
}

/**
 * @brief This function prepares discover Descriptors command
 *
 * @return Status returned
 */
static int ble_disc_desc_command(int argc, char **argv)
{
    int ret;

    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    uint8_t uuid_length = 0;
    uint8_t uuid[SERVER_MAX_UUID_LEN] = {0};
    uint16_t start_handle = 0;
    uint16_t end_handle = 0;

    NCP_DISC_CHRC_UUID_CMD discover_desc;

    if(argc != 7)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <uuid_len> <uuid> <start_handle> <end_handle>\r\n", argv[0]);
        printf("      uuid_len: 2 = UUID16, 10 = UUID128\r\n");
        printf("      uuid:     XXXX for UUID16\r\n");
        printf("                XXXX-XX-XX-XX-XXXXXX for UUID128\r\n");
        printf("      start_handle:   XXXX\r\n");
        printf("      end_handle:     XXXX\r\n");
        return -NCP_STATUS_ERROR;
    }

    start_handle = a2hex(argv[5]);
    end_handle = a2hex(argv[6]);

    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    uuid_length = a2hex(argv[3]);

    if ((uuid_length == 2) || (uuid_length == 16))
    {
        uuid2arry(argv[4], uuid, uuid_length);
        memcpy(discover_desc.uuid, uuid, uuid_length);
    }
    else
    {
        printf("Error: invalid uuid_length value\r\n");
        return -NCP_STATUS_ERROR;
    }

    discover_desc.address_type = type;
    memcpy(discover_desc.address, raw_addr, NCP_BLE_ADDR_LENGTH);
    discover_desc.uuid_length = uuid_length;
    discover_desc.start_handle = start_handle;
    discover_desc.end_handle = end_handle;

    ble_disc_desc_command_local(&discover_desc);

    return NCP_STATUS_SUCCESS;
}
#endif

/**
 * @brief This function prepares configure subscribe command
 *
 * @return Status returned
 */
static int ble_cfg_subscribe_command(int argc, char **argv)
{
    int ret;
    uint8_t addr_type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *ble_subcribe_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)ble_subcribe_command, 0, NCP_HOST_COMMAND_LEN);

    ble_subcribe_command->header.size     = NCP_CMD_HEADER_LEN;
    ble_subcribe_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 6)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [indicate/notif] [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] [enable<0/1>] <ccc_handle>\r\n", argv[0]);
        printf("      ccc_handle:     XXXX\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "indicate", 9))
        ble_subcribe_command->header.cmd      = NCP_CMD_BLE_GATT_CFG_INDICATE;
    else if (!strncmp(argv[1], "notify", 7))
        ble_subcribe_command->header.cmd      = NCP_CMD_BLE_GATT_CFG_NOTIFY;
    else
    {
        printf("Error: invalid sucbscribe type\r\n");
        return -NCP_STATUS_ERROR;
    }

    if ((atoi(argv[4]) != 0) && (atoi(argv[4]) != 1))
    {
        printf("Error: invalid value of arguments\r\n");
        printf("Usage: %s [indicate/notif] [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] [enable<0/1>] <ccc_handle>\r\n", argv[0]);
        printf("      type:          0 - indicate, 1 - notify\r\n");
        printf("      ccc_handle:     XXXX\r\n");
        return -NCP_STATUS_ERROR;
    }

    ret = get_mac(argv[3], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[2], "public", 7))
        addr_type = 0x00;
    else if (!strncmp(argv[2], "random", 7))
        addr_type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }
    NCP_CFG_SUBCRIBE_CMD *ble_subcribe = (NCP_CFG_SUBCRIBE_CMD *)&ble_subcribe_command->params.cfg_subcribe;

    ble_subcribe->address_type = addr_type;
    memcpy(ble_subcribe->address, raw_addr, NCP_BLE_ADDR_LENGTH);
    ble_subcribe->enable = atoi(argv[4]);
    ble_subcribe->ccc_handle = a2hex(argv[5]);

    ble_subcribe_command->header.size += sizeof(NCP_CFG_SUBCRIBE_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares le l2cap connection command
 *
 * @return Status returned
 */
static int ble_l2cap_connection_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *l2cap_connect_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_connect_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_connect_command->header.cmd      = NCP_CMD_BLE_L2CAP_CONNECT;
    l2cap_connect_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_connect_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 4)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <psm>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_CONNECT_CMD *set_l2cap_conn = (NCP_L2CAP_CONNECT_CMD *)&l2cap_connect_command->params.l2cap_connect;

    set_l2cap_conn->address_type = type;
    memcpy(set_l2cap_conn->address, raw_addr, NCP_BLE_ADDR_LENGTH);
    set_l2cap_conn->psm = atoi(argv[3]);
    if (argc == 5)
    {
      set_l2cap_conn->sec_flag = 1;
      set_l2cap_conn->sec = atoi(argv[4]);
    }
    else
    {
      set_l2cap_conn->sec_flag = 0;
      set_l2cap_conn->sec = 0;
    }
    l2cap_connect_command->header.size += sizeof(NCP_L2CAP_CONNECT_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares le l2cap disconnect command
 *
 * @return Status returned
 */
static int ble_l2cap_disconnect_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *l2cap_disconnect_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_disconnect_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_disconnect_command->header.cmd      = NCP_CMD_BLE_L2CAP_DISCONNECT;
    l2cap_disconnect_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_disconnect_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 3)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>]\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_DISCONNECT_CMD *set_l2cap_disconn = (NCP_L2CAP_DISCONNECT_CMD *)&l2cap_disconnect_command->params.l2cap_disconnect;

    set_l2cap_disconn->address_type = type;
    memcpy(set_l2cap_disconn->address, raw_addr, NCP_BLE_ADDR_LENGTH);

    l2cap_disconnect_command->header.size += sizeof(NCP_L2CAP_DISCONNECT_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares le l2cap send command
 *
 * @return Status returned
 */
static int ble_l2cap_send_command(int argc, char **argv)
{
    int ret;
    uint8_t type;
    uint8_t raw_addr[NCP_BLE_ADDR_LENGTH];
    MCU_NCPCmd_DS_COMMAND *l2cap_send_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_send_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_send_command->header.cmd      = NCP_CMD_BLE_L2CAP_SEND;
    l2cap_send_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_send_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 4)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s [addr_type<public|random>] [addr<XX:XX:XX:XX:XX:XX>] <times>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }
    ret = get_mac(argv[2], (char *)raw_addr, ':');

    if (ret != 0)
    {
        printf("Error: invalid ADDRESS argument\r\n");
        return -NCP_STATUS_ERROR;
    }

    if (!strncmp(argv[1], "public", 7))
        type = 0x00;
    else if (!strncmp(argv[1], "random", 7))
        type = 0x01;
    else
    {
        printf("Error: invalid ADDRESS type\r\n");
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_SEND_CMD *set_l2cap_send = (NCP_L2CAP_SEND_CMD *)&l2cap_send_command->params.l2cap_send;

    set_l2cap_send->address_type = type;
    memcpy(set_l2cap_send->address, raw_addr, NCP_BLE_ADDR_LENGTH);
    set_l2cap_send->times = atoi(argv[3]);

    l2cap_send_command->header.size += sizeof(NCP_L2CAP_SEND_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares le l2cap register command
 *
 * @return Status returned
 */
static int ble_l2cap_register_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *l2cap_register_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_register_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_register_command->header.cmd      = NCP_CMD_BLE_L2CAP_REGISTER;
    l2cap_register_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_register_command->header.result   = NCP_CMD_RESULT_OK;

    if(argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <psm>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_REGISTER_CMD *set_l2cap_register = (NCP_L2CAP_REGISTER_CMD *)&l2cap_register_command->params.l2cap_register;

    set_l2cap_register->psm = atoi(argv[1]);

    if (argc == 3 || argc == 4)
    {
      set_l2cap_register->sec_flag= 1;
      set_l2cap_register->sec_level = atoi(argv[2]);
    }
    else
    {
      set_l2cap_register->sec_flag= 0;
      set_l2cap_register->sec_level = 0;
    }

    if (argc == 4)
    {
      set_l2cap_register->policy_flag = 1;
      set_l2cap_register->policy = atoi(argv[3]);
    }
    else
    {
      set_l2cap_register->policy_flag = 0;
      set_l2cap_register->policy = 0;
    }

    l2cap_register_command->header.size += sizeof(NCP_L2CAP_REGISTER_CMD);

    return NCP_STATUS_SUCCESS;
}

#if 0
/**
 * @brief This function prepares le l2cap metrics command
 *
 * @return Status returned
 */
static int ble_l2cap_metrics_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *l2cap_rmetrics_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_rmetrics_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_rmetrics_command->header.cmd      = NCP_CMD_BLE_L2CAP_METRICS;
    l2cap_rmetrics_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_rmetrics_command->header.result   = NCP_CMD_RESULT_OK;
    l2cap_rmetrics_command->header.msg_type = NCP_MSG_TYPE_CMD;

    if(argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <metrics_flag>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_METRICS_CMD *set_l2cap_metrics = (NCP_L2CAP_METRICS_CMD *)&l2cap_rmetrics_command->params.l2cap_metrics;

    set_l2cap_metrics->metrics_flag = atoi(argv[1]);

    l2cap_rmetrics_command->header.size += sizeof(NCP_L2CAP_METRICS_CMD);

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief This function prepares le l2cap receive command
 *
 * @return Status returned
 */
static int ble_l2cap_receive_command(int argc, char **argv)
{
    MCU_NCPCmd_DS_COMMAND *l2cap_receive_command = ncp_host_get_cmd_buffer_ble();
    (void)memset((uint8_t *)l2cap_receive_command, 0, NCP_HOST_COMMAND_LEN);

    l2cap_receive_command->header.cmd      = NCP_CMD_BLE_L2CAP_RECEIVE;
    l2cap_receive_command->header.size     = NCP_CMD_HEADER_LEN;
    l2cap_receive_command->header.result   = NCP_CMD_RESULT_OK;
    l2cap_receive_command->header.msg_type = NCP_MSG_TYPE_CMD;

    if(argc != 2)
    {
        printf("Error: invalid number of arguments\r\n");
        printf("Usage: %s <delay_ms>\r\n", argv[0]);
        return -NCP_STATUS_ERROR;
    }

    NCP_L2CAP_RECEIVE_CMD *set_l2cap_receive = (NCP_L2CAP_RECEIVE_CMD *)&l2cap_receive_command->params.l2cap_receive;

    set_l2cap_receive->l2cap_recv_delay_ms = atoi(argv[1]);

    l2cap_receive_command->header.size += sizeof(NCP_L2CAP_RECEIVE_CMD);

    return NCP_STATUS_SUCCESS;
}
#endif

/**
 * @brief       This function processes response from ncp_ble
 *
 * @param res   A pointer to uint8_t
 * @return      Status returned
 */
int ble_process_ncp_event(uint8_t *res)
{
    int ret                    = -NCP_STATUS_ERROR;
    MCU_NCPCmd_DS_COMMAND *evt = (MCU_NCPCmd_DS_COMMAND *)res;

    switch (evt->header.cmd)
    {
        case NCP_EVENT_IUT_READY:
            ret = NCP_STATUS_SUCCESS;
            printf("IUT Device is ready!\r\n");
            break;
        case NCP_EVENT_ADV_REPORT:
            ret = ble_process_adv_report(res);
            break;
        case NCP_EVENT_DEVICE_CONNECTED:
            ret = ble_process_device_connected(res);
            break;
        case NCP_EVENT_DEVICE_DISCONNECT:
            ret = ble_process_device_disconnected(res);
            break;
        case NCP_EVENT_PASSKEY_DISPLAY:
            ret = ble_process_passkey_display(res);
            break;
        case NCP_EVENT_IDENITY_RESOLVED:
            ret = ble_process_idenity_resolved(res);
            break;
        case NCP_EVENT_CONN_PARAM_UPDATE:
            ret = ble_process_conn_param_update(res);
            break;
        case NCP_EVENT_PHY_UPDATED:
            ret = ble_process_phy_update(res);
            break;
        case NCP_EVENT_DATA_LEN_UPDATED:
            ret = ble_process_data_len_update(res);
            break;
        case NCP_EVENT_SEC_LEVEL_CHANGED:
            ret = ble_process_security_level_changed(res);
            break;

        case NCP_EVENT_GATT_NOTIFICATION:
            ret = ble_process_gatt_notification(res);
            break;
        case NCP_EVENT_ATTR_VALUE_CHANGED:
            ret = ble_process_attr_value_changed(res);
            break;
        case NCP_EVENT_GATT_CCC_CFG_CHANGED:
            ret = ble_process_gatt_ccc_cfg_changed(res);
            break;
        case NCP_EVENT_GATT_SUBSCRIPTIONED:
            ret = ble_process_gatt_subscriptioned(res);
            break;
        case NCP_EVENT_L2CAP_CONNECT:
            ret = ble_process_l2cap_connected(res);
            break;
        case NCP_EVENT_L2CAP_DISCONNECT:
            ret = ble_process_l2cap_disconnected(res);
            break;
        case NCP_EVENT_L2CAP_RECEIVE:
            ret = ble_process_l2cap_received(res);
            break;
        case NCP_EVENT_GATT_DISC_PRIM:
            ret = ble_process_gatt_prim_discovered(res);
            break;
        case NCP_EVENT_GATT_DISC_CHRC:
            ret = ble_process_gatt_chrc_discovered(res);
            break;
        case NCP_EVENT_GATT_DISC_DESC:
            ret = ble_process_gatt_desc_discovered(res);
            break;
        default:
            printf("Invaild event! Invalid event id is %08x\r\n", evt->header.cmd);
            break;
    }
    return ret;
}

static int ble_process_adv_report(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_DEVICE_ADV_REPORT_EV *adv_reported_tlv = (NCP_DEVICE_ADV_REPORT_EV *)&evt_res->params.adv_reported;

    memcpy(address,adv_reported_tlv->address, 6);
    printf("find device: %s address, %02X:%02X:%02X:%02X:%02X:%02X \r\n", adv_reported_tlv->address_type == 0 ? "public" : "random",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);
    return NCP_STATUS_SUCCESS;
}

static int ble_process_device_connected(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_DEVICE_CONNECTED_EV *device_connected_tlv = (NCP_DEVICE_CONNECTED_EV *)&evt_res->params.device_connected;

    memcpy(address,device_connected_tlv->address, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X ", 
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(device_connected_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("Connection Interval: %X\r\n",device_connected_tlv->interval);
    printf("Connection Latency: %X\r\n",device_connected_tlv->latency);
    printf("Supervation Timeout: %X\r\n",device_connected_tlv->timeout);

    return NCP_STATUS_SUCCESS;
}

static int ble_process_device_disconnected(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_DEVICE_DISCONNECTED_EV *device_disconnected_tlv = (NCP_DEVICE_DISCONNECTED_EV *)&evt_res->params.device_disconnected;

    memcpy(address,device_disconnected_tlv->address, 6);
    printf("Disonnected to: %02X:%02X:%02X:%02X:%02X:%02X ", 
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(device_disconnected_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }

    return NCP_STATUS_SUCCESS;
}

static int ble_process_passkey_display(uint8_t *res)
{
    uint32_t key = 0;
    
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_PASSKEY_DISPLAY_EV *passkey_display_tlv = (NCP_PASSKEY_DISPLAY_EV *)&evt_res->params.passkey_display;

    key = passkey_display_tlv->passkey;
    printf("Pass Key PIN: %06d\r\n", key);

    return NCP_STATUS_SUCCESS;
}

static int ble_process_idenity_resolved(uint8_t *res)
{
    uint8_t identity_address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_IDENTITY_RESOLVED_EV *idenitiy_resolved_tlv = (NCP_IDENTITY_RESOLVED_EV *)&evt_res->params.idenitiy_resolved;

    memcpy(identity_address,idenitiy_resolved_tlv->identity_address, 6);

    printf("Remote device Identity Address Resolved\r\n");
    printf("Identity Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", 
                    identity_address[0],  identity_address[1], identity_address[2], identity_address[3], 
                    identity_address[4], identity_address[5]);

    if(idenitiy_resolved_tlv->identity_address_type == 0) //pubilc address
    {
        printf("Identity Address type: public\r\n");

    }
    else
    {
        printf("Identity Address type: random\r\n");
    }

    return NCP_STATUS_SUCCESS;
}

static int ble_process_conn_param_update(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_CMD_CONN_PARA_UPDATE_EV *conn_update_tlv = (NCP_CMD_CONN_PARA_UPDATE_EV *)&evt_res->params.conn_param_update_ev;

    memcpy(address,conn_update_tlv->addr, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(conn_update_tlv->type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("Connection Interval: %X\r\n",conn_update_tlv->interval);
    printf("Connection Latency: %X\r\n",conn_update_tlv->latency);
    printf("Supervation Timeout: %X\r\n",conn_update_tlv->timeout);

    return NCP_STATUS_SUCCESS;
}

static int ble_process_phy_update(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_CMD_PHY_UPDATE_EV *phy_update_tlv = (NCP_CMD_PHY_UPDATE_EV *)&evt_res->params.phy_updated_ev;

    memcpy(address,phy_update_tlv->address, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(phy_update_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");
    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("rx_phy: %X\r\n",phy_update_tlv->rx_phy);
    printf("tx_phy: %X\r\n",phy_update_tlv->tx_phy);

    return NCP_STATUS_SUCCESS;
}

static int ble_process_data_len_update(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_CMD_DATA_LEN_UPDATE_EV *data_len_update_tlv = (NCP_CMD_DATA_LEN_UPDATE_EV *)&evt_res->params.data_len_updated_ev;

    memcpy(address,data_len_update_tlv->address, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(data_len_update_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("tx_max_len: %X\r\n",data_len_update_tlv->tx_max_len);
    printf("tx_max_time: %X\r\n",data_len_update_tlv->tx_max_time);
    printf("rx_max_len: %X\r\n",data_len_update_tlv->rx_max_len);
    printf("rx_max_time: %X\r\n",data_len_update_tlv->rx_max_time);
    
    return NCP_STATUS_SUCCESS;
}

static int ble_process_security_level_changed(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_SEC_LEVEL_CHANGED_EV *sec_level_changed_tlv = (NCP_SEC_LEVEL_CHANGED_EV *)&evt_res->params.idenitiy_resolved;

    memcpy(address,sec_level_changed_tlv->address, 6);

    printf("Security Level Changed: ");
    switch(sec_level_changed_tlv->sec_level)
    {
        case 4:
            printf("Level 4: Authenticated Secure Connections and 128-bit key.");
            break;
        case 3:
            printf("Level 3: Encryption and authentication (MITM).");
            break;
        case 2:
            printf("Level 2: Encryption and no authentication (no MITM).");
            break;
        case 1:
            printf("Level 1: No encryption and no authentication.");
            break;
        default:
        case 0:
            printf("Level 0: Only for BR/EDR special cases, like SDP. ");
            break;
    }

    printf(" Address: %02X:%02X:%02X:%02X:%02X:%02X ", 
                    address[0],  address[1], address[2], address[3], 
                    address[4], address[5]);

    if(sec_level_changed_tlv->address_type == 0) //pubilc address
    {
        printf(" type: public\r\n");

    }
    else
    {
        printf(" type: random\r\n");
    }

    return NCP_STATUS_SUCCESS;
}

static int ble_process_gatt_notification(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_NOTIFICATION_EV *gatt_notification_tlv = (NCP_NOTIFICATION_EV *)&evt_res->params.gatt_notification;

    uint8_t address[6]= {0};

    memcpy(address, gatt_notification_tlv->address, 6);
    printf("Recieve Notification form %02X:%02X:%02X:%02X:%02X:%02X ", 
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(gatt_notification_tlv->address_type == 0) //pubilc address
    {
        printf("type: public ");

    }
    else
    {
        printf("type: random ");
    }
    printf("Attribute handle: %04X Value: ", gatt_notification_tlv->handle);
    for(uint8_t i = 0; i < gatt_notification_tlv->data_length; i ++)
    {
        printf("%02d ", gatt_notification_tlv->data[i]);
    }
    printf("\r\n");
    
    // device central profile data print 
    switch (gatt_notification_tlv->svc_id)
    {
        case CENTRAL_HTC_SERVICE_ID:
            htc_central_notify(gatt_notification_tlv->data);
            break;
        case CENTRAL_HRC_SERVICE_ID:
            hrc_central_notify(gatt_notification_tlv->data);
            break;
  
        default:
            // PRINTF("unsupported profile service\n");
            break;
    }

    return NCP_STATUS_SUCCESS;
}

static int ble_process_attr_value_changed(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_ATTR_VALUE_CHANGED_EV *attr_value_changed_tlv = (NCP_ATTR_VALUE_CHANGED_EV *)&evt_res->params.attr_value_changed;

    printf("Attribute value changed. Attribute handle: %04X Value: ", attr_value_changed_tlv->handle);
    for(uint8_t i = 0; i < attr_value_changed_tlv->data_length; i ++)
    {
        printf("%02d ", attr_value_changed_tlv->data[i]);
    }
    printf("\r\n");
    return NCP_STATUS_SUCCESS;
}

static int ble_process_gatt_ccc_cfg_changed(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_CCC_CFG_CHANGED_EV *gatt_ccc_cfg_changed_tlv = (NCP_CCC_CFG_CHANGED_EV *)&evt_res->params.gatt_ccc_cfg_changed_ev;

    printf("UUID - %02X%02X: Client Characteristic Configuration changed: ", gatt_ccc_cfg_changed_tlv->uuid[1], gatt_ccc_cfg_changed_tlv->uuid[0]);
    if(gatt_ccc_cfg_changed_tlv->ccc_value == BT_GATT_CCC_NOTIFY)
    {
        printf("Notification\r\n");
    }
    else if(gatt_ccc_cfg_changed_tlv->ccc_value == BT_GATT_CCC_INDICATE)
    {
        printf("Indication\r\n");
    }
    else
    {
        printf("Disable\r\n");

    }
#if CONFIG_NCP_HTS
    peripheral_hts_indicate(gatt_ccc_cfg_changed_tlv->ccc_value);
#endif
#if CONFIG_NCP_HRS
    peripheral_hrs_indicate(gatt_ccc_cfg_changed_tlv->ccc_value);
#endif
#if CONFIG_NCP_BAS
    peripheral_bas_indicate(gatt_ccc_cfg_changed_tlv->ccc_value);
#endif
    return NCP_STATUS_SUCCESS;
}

static int ble_process_gatt_subscriptioned(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_SUBSCRIPTIONED_EV *gatt_subscriptioned_tlv = (NCP_SUBSCRIPTIONED_EV *)&evt_res->params.gatt_subscription_ev;
    printf("\n");
    printf("Subscription ");
    switch (gatt_subscriptioned_tlv->svc_id)
    {
    case CENTRAL_HTC_SERVICE_ID:
        printf("HTC ");
        break;
    
    case CENTRAL_HRC_SERVICE_ID:
        printf("HRC ");
        break;

    default:
        break;
    }

    if(!gatt_subscriptioned_tlv->status) {
        printf("success\r\n");
    }else {
        printf("failed\r\n");
    }
    printf("\n");
    
    return NCP_STATUS_SUCCESS;
}

static int ble_process_l2cap_connected(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_L2CAP_CONNECT_EV *l2cap_connected_tlv = (NCP_L2CAP_CONNECT_EV *)&evt_res->params.l2cap_connect_ev;

    memcpy(address,l2cap_connected_tlv->address, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(l2cap_connected_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("L2CAP psm: %u\r\n",l2cap_connected_tlv->psm);

    return NCP_STATUS_SUCCESS;
}

static int ble_process_l2cap_disconnected(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_L2CAP_DISCONNECT_EV *l2cap_disconnected_tlv = (NCP_L2CAP_DISCONNECT_EV *)&evt_res->params.l2cap_disconnect_ev;

    memcpy(address,l2cap_disconnected_tlv->address, 6);
    printf("Addr: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(l2cap_disconnected_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("L2CAP psm %u disconnect\r\n",l2cap_disconnected_tlv->psm);
    
    return NCP_STATUS_SUCCESS;
}

static int ble_process_gatt_prim_discovered(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_DISC_PRIM_EV *gatt_disc_prim_tlv = (NCP_DISC_PRIM_EV *)&evt_res->params.gatt_disc_prim_ev;
    GATT_SERVICE_T service;

    for( uint8_t i = 0; i < gatt_disc_prim_tlv->services_count; i++)
    {
        memcpy(&service, &gatt_disc_prim_tlv->services[i], sizeof(GATT_SERVICE_T));
        (void)PRINTF("Discovered Primary Service: Handle %04X, uuid: %02X%02X\r\n",service.start_handle,service.uuid[1],service.uuid[0]);

        switch (sys_get_le16(service.uuid))
        {
            case UUID_HTS:
                (void)PRINTF("Discovered Health Thermometer Service\r\n");
                break;
            case UUID_HRS:
                (void)PRINTF("Discovered Health Rate Service\r\n");
            default:
                break;
        }
    }

    return NCP_STATUS_SUCCESS;
}

static void print_chrc_props(uint8_t _props)
{
    if (_props & BT_GATT_CHRC_BROADCAST)
        (void)PRINTF("Characteristic property: broadcast\r\n");
    if (_props & BT_GATT_CHRC_READ)
        (void)PRINTF("Characteristic property: read\r\n");
    if (_props & BT_GATT_CHRC_WRITE_WITHOUT_RESP)
        (void)PRINTF("Characteristic property: write without response\r\n");
    if (_props & BT_GATT_CHRC_WRITE)
        (void)PRINTF("Characteristic property: write with response\r\n");
    if (_props & BT_GATT_CHRC_NOTIFY)
        (void)PRINTF("Characteristic property: notify\r\n");
    if (_props & BT_GATT_CHRC_INDICATE)
        (void)PRINTF("Characteristic property: indicate\r\n");
    if (_props & BT_GATT_CHRC_AUTH)
        (void)PRINTF("Characteristic property: Authenticated Signed Writes\r\n");
    if (_props & BT_GATT_CHRC_EXT_PROP)
        (void)PRINTF("Characteristic property: Extended Properties\r\n");
}

static int ble_process_gatt_chrc_discovered(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_DISC_CHRC_EV *gatt_disc_chrc_tlv = (NCP_DISC_CHRC_EV *)&evt_res->params.gatt_disc_chrc_ev;
    GATT_CHARACTERISTIC_T characteristics;

    for( uint8_t i = 0; i < gatt_disc_chrc_tlv->characteristics_count; i++)
    {
        memcpy(&characteristics, &gatt_disc_chrc_tlv->characteristics[i], sizeof(GATT_CHARACTERISTIC_T));
        (void)PRINTF("Discovered Characteristics: Handle %04X, Value Handle %04X, uuid: %02X%02X ", \
                characteristics.characteristic_handle, characteristics.value_handle, characteristics.uuid[1], characteristics.uuid[0]);
        
        print_chrc_props(characteristics.properties);

        switch (sys_get_le16(characteristics.uuid))
        {
            case UUID_HTS_MEASUREMENT:
                (void)PRINTF("Discovered HTS Characteristic Measurement\r\n");
                break;
            case UUID_HRS_MEASUREMENT:
                (void)PRINTF("Discovered HRS Characteristic Measurement Interval\r\n");
                break;
            case UUID_HRS_BODY_SENSOR:
                (void)PRINTF("Discovered HRS Characteristic Body Sensor Location\r\n");
                break;
            case UUID_HRS_CONTROL_POINT:
                (void)PRINTF("Discovered  HRS Characteristic Control Point\r\n");
                break;
            default:
                break;
        }
    }

    return NCP_STATUS_SUCCESS;
}

int ble_process_l2cap_received(uint8_t *res)
{
    uint8_t address[6]= {0};

    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;

    NCP_L2CAP_RECEIVE_EV *l2cap_received_tlv = (NCP_L2CAP_RECEIVE_EV *)&evt_res->params.l2cap_receive_ev;

    memcpy(address,l2cap_received_tlv->address, 6);
    printf("Connected to: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
                    address[0],  address[1], address[2], address[3], address[4], address[5]);

    if(l2cap_received_tlv->address_type == 0) //pubilc address
    {
        printf("Address type: public\r\n");

    }
    else
    {
        printf("Address type: random\r\n");
    }
    printf("L2CAP psm: %u\r\n",l2cap_received_tlv->psm);

    ncp_dump_hex(l2cap_received_tlv->data, l2cap_received_tlv->len);

    return NCP_STATUS_SUCCESS;
}

int ble_process_gatt_desc_discovered(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *evt_res = (MCU_NCPCmd_DS_COMMAND *)res;
    NCP_DISC_ALL_DESC_EV *gatt_disc_desc_tlv = (NCP_DISC_ALL_DESC_EV *)&evt_res->params.gatt_disc_desc_ev;
    GATT_DESCRIPTOR_T descriptor;

    for( uint8_t i = 0; i < gatt_disc_desc_tlv->descriptors_count; i++)
    {
        memcpy(&descriptor, &gatt_disc_desc_tlv->descriptors[i], sizeof(GATT_SERVICE_T));
        (void)PRINTF("Discovered Client Characteristic Configuration: Handle %04X, uuid: %02X%02X\r\n", \
                    descriptor.descriptor_handle,descriptor.uuid[1],descriptor.uuid[0]);
    }

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief       This function processes response from ncp_ble
 *
 * @param res   A pointer to uint8_t
 * @return      Status returned
 */
int ble_process_response(uint8_t *res)
{
    int ret                        = -NCP_STATUS_ERROR;
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    switch (cmd_res->header.cmd)
    {
        case NCP_RSP_BLE_GAP_START_ADV:
            ret = ble_process_start_adv_response(res);
            break;
        case NCP_RSP_BLE_GAP_STOP_ADV:
            ret = ble_process_stop_adv_response(res);
            break;
        case NCP_RSP_BLE_GAP_SET_ADV_DATA:
            ret = ble_process_set_adv_data_response(res);
            break;
        case NCP_RSP_BLE_GAP_SET_SCAN_PARAM:
            ret = ble_process_set_scan_param_response(res);
            break;
        case NCP_RSP_BLE_GAP_START_SCAN:
            ret = ble_process_start_scan_response(res);
            break;
        case NCP_RSP_BLE_GAP_STOP_SCAN:
            ret = ble_process_stop_scan_response(res);
            break;
        case NCP_RSP_BLE_GAP_CONNECT:
            ret = ble_process_connect_response(res);
            break;
        case NCP_RSP_BLE_GAP_DISCONNECT:
            ret = ble_process_disconnect_response(res);
            break;
        case NCP_RSP_BLE_GAP_SET_DATA_LEN:
            ret = ble_process_set_data_len_response(res);
            break;
        case NCP_RSP_BLE_GAP_SET_PHY:
            ret = ble_process_set_phy_response(res);
            break;
        case NCP_RSP_BLE_GAP_CONN_PARAM_UPDATE:
            ret = ble_process_conn_update_response(res);
            break;
        case NCP_RSP_BLE_GAP_SET_FILTER_LIST:
            ret = ble_process_set_filter_list_response(res);
            break;
        case NCP_RSP_BLE_GAP_PAIR:
            ret = ble_process_ble_gap_pair_response(res);
            break;
        case NCP_RSP_BLE_GATT_READ:
            ret = ble_process_ble_gatt_read_char_response(res);
            break;
        case NCP_RSP_BLE_VENDOR_SET_DEVICE_ADDR:
            ret = ble_process_set_dev_addr_response(res);
            break;
        case NCP_RSP_BLE_VENDOR_SET_DEVICE_NAME:
            ret = ble_process_set_dev_name_response(res);
            break;
        case NCP_RSP_BLE_HOST_SERVICE_ADD:
            ret = ble_process_host_service_add_response(res);
            break;
        case NCP_RSP_BLE_GATT_START_SERVICE:
            ret = ble_process_start_service_response(res);
            break;
        case NCP_RSP_BLE_GATT_SET_VALUE:
            ret = ble_process_write_charateristic_response(res);
            break;
        case NCP_RSP_BLE_VENDOR_POWER_MODE:
            ret = ble_process_set_power_mode_response(res);
            break;
        case NCP_RSP_BLE_GATT_DISC_PRIM:
            ret = ble_process_disc_prim_response(res);
            break;
        case NCP_RSP_BLE_GATT_DISC_CHRC:
            ret = ble_process_disc_chrc_response(res);
            break;
        case NCP_RSP_BLE_GATT_DESC_CHRC:
            ret = ble_process_disc_desc_response(res);
            break;
        case NCP_RSP_BLE_GATT_CFG_INDICATE:
            ret = ble_process_cfg_indicate_response(res);
            break;
        case NCP_RSP_BLE_GATT_CFG_NOTIFY:
            ret = ble_process_cfg_notify_response(res);
            break;
        case NCP_RSP_BLE_GATT_REGISTER_SERVICE:
            ret = ble_process_register_service_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_CONNECT:
            ret = ble_process_l2cap_connect_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_DISCONNECT:
            ret = ble_process_l2cap_disconnect_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_SEND:
            ret = ble_process_l2cap_send_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_REGISTER:
            ret = ble_process_l2cap_register_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_METRICS:
            ret = ble_process_l2cap_metrics_response(res);
            break;
        case NCP_RSP_BLE_L2CAP_RECEIVE:
            ret = ble_process_l2cap_receive_response(res);
            break;

        default:
            printf("Invaild response cmd!\r\n");
            break;
    }
    return ret;
}

/**
 * @brief      This function processes start adv resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_start_adv_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;
    if (cmd_res->header.result == NCP_CMD_RESULT_OK) {
        printf("Start advertising successfully.\r\n");
    }
    else
        printf("Error: Failed to start advertising.\r\n");

    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes stop adv resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_stop_adv_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Stop advertising successfully\r\n");
    }
    else
    {
        printf("Error: unable to stop advertising\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes set adv data response from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_set_adv_data_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Set advertising data successfully\r\n");
    }
    else
    {
        printf("Error: unable to set advertising data\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes set scan parameter resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_set_scan_param_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Set scan parameter successfully\r\n");
    }
    else
    {
        printf("Error: unable to set scan parameter\r\n");
    }
    return NCP_STATUS_SUCCESS;
}
/**
 * @brief      This function processes start scan resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_start_scan_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Start scan successfully\r\n");
    }
    else
    {
        printf("Error: unable to start scan\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes stop scan resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_stop_scan_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Stop scanning successfully\r\n");
    }
    else
    {
        printf("Error: unable to stop scanning\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes connect resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_connect_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("le start to connect\r\n");
    }
    else
    {
        printf("Error: unable to start to connect\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes disconnect resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_disconnect_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("le start to disconnect\r\n");
    }
    else
    {
        printf("Error: unable to start to disconnect\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes set data len response from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_set_data_len_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Set data len initiated successfully\r\n");
    }
    else
    {
        printf("Error: set data len failed\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes set phy response from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_set_phy_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Set phy initiated successfully\r\n");
    }
    else
    {
        printf("Error: set phy failed\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes connection param update resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_conn_update_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("le start to update connection parameter\r\n");
    }
    else
    {
        printf("Error: unable to update connection paramete \r\n");
        printf("Make sure you have entered valid interval, interval_min and interval_max should apply to : interval_min > current interval or interval_max < current interval \r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes set filter list resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_set_filter_list_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("set filter list successfully\r\n");
    }
    else
    {
        printf("Error: unable to set filter list\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_ble_gap_pair_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("pair successfully\r\n");
    }
    else
    {
        printf("Error: unable to pair\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_ble_gatt_read_char_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("read successfully\r\n");
    }
    else
    {
        printf("Error: unable to read\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_set_dev_addr_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("set successfully\r\n");
    }
    else
    {
        printf("Error: unable to set\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_set_dev_name_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("set successfully\r\n");
    }
    else
    {
        printf("Error: unable to set\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_host_service_add_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Host service add successfully\r\n");
    }
    else
    {
        printf("Error: unable to add host service\r\n");  
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_start_service_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Start service successfully\r\n");
    }
    else
    {
        printf("Error: unable to start service\r\n");  
    }
    return NCP_STATUS_SUCCESS;
}

int ble_process_write_charateristic_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Write GATT value successfully\r\n");
    }
    else
    {
        printf("Error: unable to write GATT value\r\n");
    }
#if CONFIG_NCP_HTS
    peripheral_hts_event_put(HTS_EVENT_WRITE_CHRA_RSP);
#endif
#if CONFIG_NCP_HRS
    peripheral_hrs_event_put(HRS_EVENT_WRITE_CHRA_RSP);
#endif
#if CONFIG_NCP_BAS
    peripheral_bas_event_put(BAS_EVENT_WRITE_CHRA_RSP);
#endif
    return NCP_STATUS_SUCCESS;
}

static int ble_process_set_power_mode_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Set power mode successfully\r\n");
    }
    else
    {
        printf("Error: unable to set power mode\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_disc_prim_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Discover primary service successfully\r\n");
    }
    else
    {
        printf("Error: Failed to discover primary service\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_disc_chrc_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Discover characteristic attribute successfully\r\n");
    }
    else
    {
        printf("Error: Failed to discover characteristic attribute\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_disc_desc_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Discover characteristic descriptor attribute successfully\r\n");
    }
    else
    {
        printf("Error: Failed to discover characteristic descriptor attribute\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_cfg_notify_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Configure Service notify successfully\r\n");
    }
    else
    {
        printf("Error: Failed to configure service nofity\r\n");
    }
    return NCP_STATUS_SUCCESS;
}


static int ble_process_cfg_indicate_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("Configure Service indicate successfully\r\n");
    }
    else
    {
        printf("Error: Failed to configure service indicate\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

static int ble_process_register_service_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        NCP_REGISTER_SERVICE *register_sev_rp = (NCP_REGISTER_SERVICE *)&cmd_res->params.register_service;
        uint8_t svc_length = register_sev_rp->svc_length;
        for(int i = 0; i < svc_length; i++) {
            printf("Register Service : ");
            uint8_t id = reg_serv[i];
            switch (id)
            {
                case 1:
                    printf("Peripheral_HTS ");
                    break; 
                case 2:
                    printf("Peripheral_HRS ");
                    break;
                case 3:
                    printf("BAS");
                    break;
                case 4:
                    printf("Central_HTS ");
                    break;
                case 5:
                    printf("Central_HRS ");
                    break;

                default:
                    break;
            }
            if (register_sev_rp->service[i] == NCP_CMD_RESULT_OK)
            {
                printf("Success\r\n");
            }else {
                printf("Failed\r\n");
            }
        }
    }
    else
    {
        printf("Error: Failed to register service \r\n");
    }
    memset(reg_serv, 0, MAX_SUPPORT_SERVICE);
    return NCP_STATUS_SUCCESS;
}

/* device central service notify function */
static void htc_central_notify(uint8_t *data)
{
    struct temp_measurement temp_measurement;
    uint32_t temperature;

    /* temperature value display */
    temp_measurement = *(struct temp_measurement*)data;
    temperature = sys_get_le32(temp_measurement.temperature);

    if ((temp_measurement.flags & 0x01) == hts_unit_celsius_c)
    {
        printf("Temperature %d degrees Celsius \n", temperature);
    }
    else
    {
        printf("Temperature %d degrees Fahrenheit \n", temperature);
    }
}

static void hrc_central_notify(uint8_t *data)
{
    struct hr_measurement *hr_measurement = (struct hr_measurement *) data;
    
    uint8_t sensor = hr_measurement->sensor;
    uint8_t rate   = hr_measurement->rate;
    
    /* heart rate value display */
    printf("[NOTIFICATION] sensor id is %d, heart rate is %d\n", sensor, rate);
}

/**
 * @brief      This function processes l2cap connect resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_connect_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("le l2cap start to connect\r\n");
    }
    else
    {
        printf("Error: unable to start le l2cap connect\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes l2cap disconnect from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_disconnect_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("le l2cap start to disconnect\r\n");
    }
    else
    {
        printf("Error: unable to start le l2cap disconnect\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes l2cap send resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_send_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("l2cap send successfully\r\n");
    }
    else
    {
        printf("Error: unable to l2cap send\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes l2cap register resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_register_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("l2cap register successfully\r\n");
    }
    else
    {
        printf("Error: unable to l2cap register\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes l2cap metrics resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_metrics_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("set metrics successfully\r\n");
    }
    else
    {
        printf("Error: unable to set metrics\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      This function processes l2cap receive resopnse from ncp_ble
 *
 * @param res  A pointer to uint8_t
 * @return     Status returned
 */
static int ble_process_l2cap_receive_response(uint8_t *res)
{
    MCU_NCPCmd_DS_COMMAND *cmd_res = (MCU_NCPCmd_DS_COMMAND *)res;

    if (cmd_res->header.result == NCP_CMD_RESULT_OK)
    {
        printf("set l2cap receive delay times successfully\r\n");
    }
    else
    {
        printf("Error: unable to set l2cap receive delay times\r\n");
    }
    return NCP_STATUS_SUCCESS;
}

/**
 * @brief      command list
 *
 */

static struct ncp_host_cli_command ncp_host_app_cli_commands_ble[] = {
    {"ble-set-adv-data", "<adv_data>", ble_set_adv_data_command},
    {"ble-start-adv", NULL, ble_start_adv_command},
    {"ble-stop-adv", NULL, ble_stop_adv_command},
    {"ble-set-scan-param", "<filter_option> <interval> <window>", ble_set_scan_param_command},
    {"ble-start-scan", "<scan_type>", ble_start_scan_command},
    {"ble-stop-scan", NULL, ble_stop_scan_command},
    {"ble-connect", "<addr_type> <addr>", ble_connect_command},
    {"ble-disconnect", "<addr_type> <addr>", ble_disconnect_command},
    {"ble-set-data-len", "<addr_type> <addr> <tx_max_len> [optional<tx_max_time>]", ble_set_data_len_command},
    {"ble-set-phy", "<addr_type> <addr> <tx_phy> <rx_phy>", ble_set_phy_command},
    {"ble-conn-param-update", "<addr_type> <addr> <max_interval> <min_interval> <latency> <timeout>", ble_connect_paramter_update_command},
    {"ble-set-filter-list", "<filter_addr_num> <addr_type> <addr> ... <addr_type> <addr>", ble_set_filter_list_command},
    {"ble-start-encryption", "<addr_type> <addr>", ble_start_encryption_command},
    {"ble-set-value", "<uuid_len> <uuid> <value_len> <value ...>", ble_write_characteristic_command},
    {"ble-read-characteristic", "<addr_type> <addr> <handle>", ble_read_characteristic_command},
    {"ble-set-power-mode", "<0/1>", ble_set_power_mode_command},
#if 0
    {"ble-set-device-address", "<addr>", ble_set_device_address_command},
    {"ble-set-device-name", "<name>", ble_set_device_name_command},
#endif
    {"ble-host-svc-add", NULL, ble_host_service_add_command},
    // {"ble-host-svc-disc", "<prim|chrc|desc> <addr_type> <addr> <uuid> [<start_handle> <end_handle>]", ble_host_service_discover_command},
    {"ble-register-service", "<num_of_service> <service id_1> <service id_2> ...", ble_register_service_command},
    {"ble-start-service", "<profile_name[hts/htc/hrs/hrc/bas]>", ble_start_service_command},
    {"ble-cfg-subscribe", "<indicate/notify> <addr_type> <addr> <enable[0/1]> <ccc_handle>", ble_cfg_subscribe_command},
    // {"ble-discover-prim", "<addr_type> <addr> <uuid_len> <uuid>", ble_disc_prim_command},
    // {"ble-discover-chrc", "<addr_type> <addr> <uuid_len> <uuid> <start_handle> <end_handle>", ble_disc_chrc_command},
    // {"ble-discover-desc", "<addr_type> <addr> <uuid_len> <uuid> <start_handle> <end_handle>", ble_disc_desc_command},

    {"ble-l2cap-connect", "<addr_type> <addr> <psm>", ble_l2cap_connection_command},
    {"ble-l2cap-disconnect", "<addr_type> <addr>", ble_l2cap_disconnect_command},
    {"ble-l2cap-send", "<addr_type> <addr> <times>", ble_l2cap_send_command},
    {"ble-l2cap-register", "<psm>", ble_l2cap_register_command},

    // {"ble-set-uart-br", "<baud rate>", NULL},
    // {"ble-cfg-multi-adv", NULL, NULL},
};

/**
 * @brief      Register  ncp_host_cli commands for ble
 *
 * @return     Status returned
 */
int ncp_host_ble_command_init()
{
    if (ncp_host_cli_register_commands(ncp_host_app_cli_commands_ble,
                                       sizeof(ncp_host_app_cli_commands_ble) / sizeof(struct ncp_host_cli_command)) != 0)
        return -NCP_STATUS_ERROR;

    return NCP_STATUS_SUCCESS;
}

#endif
