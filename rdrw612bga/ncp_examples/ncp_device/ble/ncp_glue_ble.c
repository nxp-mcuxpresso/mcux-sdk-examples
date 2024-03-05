/* @file ncp_glue_ble.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  Licensed under the LA_OPT_NXP_Software_License.txt (the "Agreement")
 */

#include <stdio.h>
#include <stdlib.h>
#include "board.h"
#include "fsl_debug_console.h"

#include <bluetooth/hci.h>

#include "fsl_component_serial_manager.h"
#include "fsl_os_abstraction.h"
#include "ncp_glue_ble.h"
#include "ncp_ble.h"
#include "service.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CMD_SUBCLASS_BLE_LEN    (sizeof(cmd_subclass_ble) / sizeof(struct cmd_subclass_t))

#define UNUSED(x) (void)(x)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

// extern void supported_commands(const uint8_t *data, uint16_t len);
// extern void supported_services(const uint8_t *data, uint16_t len);
extern void reset_board(const uint8_t *data, uint16_t len);
extern void set_adv_data(const uint8_t *data, uint16_t len);
extern void start_advertising(const uint8_t *data, uint16_t len);
extern void stop_advertising(const uint8_t *data, uint16_t len);
extern void set_scan_parameter(const uint8_t *data, uint16_t len);
extern void start_discovery(const uint8_t *data, uint16_t len);
extern void stop_discovery(const uint8_t *data, uint16_t len);
extern void ble_connect(const uint8_t *data, uint16_t len);
extern void disconnect(const uint8_t *data, uint16_t len);
extern void conn_param_update(const uint8_t *data, uint16_t len);
extern void set_filter_list(const uint8_t *data, uint16_t len);
extern void pair(const uint8_t *data, uint16_t len);
extern void disc_prim_uuid(uint8_t *data, uint16_t len);
extern void disc_chrc_uuid(uint8_t *data, uint16_t len);
extern void disc_desc_uuid(uint8_t *data, uint16_t len);
extern void disc_desc_uuid(uint8_t *data, uint16_t len);
extern void config_subscription(uint8_t *data, uint16_t len, uint16_t op);

//GATT
extern void add_service(uint8_t *data, uint16_t len);
extern void add_characteristic(uint8_t *data, uint16_t len);
extern void add_descriptor(uint8_t *data, uint16_t len);
extern void start_server(uint8_t *data, uint16_t len);
extern void set_value(const uint8_t *data, uint16_t len);
extern void read_data(const uint8_t *data, uint16_t len);
extern void write_data(const uint8_t *data, uint16_t len);

extern uint8_t ncp_ble_init_gatt(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern uint8_t res_buf[NCP_BRIDGE_INBUF_SIZE];

static uint8_t gatt_service_init = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

// #ifdef CONFIG_NCP_BRIDGE_DEBUG
#if 0
#define NCP_DEBUG_TIME_COUNT  512
#define NCP_DEBUG_TIME_FUNC   128
int ncp_debug_time_num = 0;
unsigned long ncp_debug_time[NCP_DEBUG_TIME_COUNT] = {0};
char ncp_debug_time_pos[NCP_DEBUG_TIME_COUNT][NCP_DEBUG_TIME_FUNC] = {0};
extern unsigned int os_get_timestamp(void);
void print_ncp_debug_time(void)
{
   for (int i = 0; i < ncp_debug_time_num; i++)
       (void)PRINTF("%d-%s-%lu\r\n", i, ncp_debug_time_pos[i], ncp_debug_time[i]);
   for (int i = 1; i < ncp_debug_time_num; i++)
       (void)PRINTF("[%d-%lu]\r\n", i, ncp_debug_time[i] - ncp_debug_time[i-1]);
  ncp_debug_time_num = 0;
}
void add_ncp_debug_time_item(const char *func)
{
   int func_len = strlen(func)+1 <= NCP_DEBUG_TIME_FUNC? strlen(func)+1 : NCP_DEBUG_TIME_FUNC;
   if (ncp_debug_time_num >= NCP_DEBUG_TIME_COUNT)
   {
       (void)PRINTF("the array is full, please increase NCP_DEBUG_TIME_COUNT\r\n");
       return;
   }
   ncp_debug_time[ncp_debug_time_num] = os_get_timestamp();
   memcpy(ncp_debug_time_pos[ncp_debug_time_num++], func, func_len);
}
#endif

void ncp_notify_attribute(const void *data, uint16_t len) 
{
    uint8_t *event_buf        = NULL;
    if(len <= 0)
        return;
    event_buf = OSA_MemoryAllocate(len);
    if (event_buf == NULL)
    {
        //ncp_e("failed to allocate memory for event");
        return;
    }

    memcpy(event_buf, data, len);
    ble_bridge_prepare_status(NCP_BRIDGE_EVENT_ATTR_VALUE_CHANGED, 0, event_buf, len);
}

/*scan fuctions*/

/*CMD handle functions*/
/*Set BT Sleep Mode*/
static int hci_le_vendor_power_mode_cfg(uint8_t power_mode)
{

    ncp_ble_low_power_mode_cfg *cp;
    struct net_buf *buf;

    buf = bt_hci_cmd_create(BT_HCI_VD_LOW_POWER_MODE, sizeof(*cp));
    if (!buf) {
        return -ENOBUFS;
    }

    cp = (ncp_ble_low_power_mode_cfg *)net_buf_add(buf, sizeof(*cp));

    if(power_mode == 0)
        cp->power_mode = 0x03; // Controller auto sleep enable
    else
        cp->power_mode = 0x02; // Controller auto sleep disable

    cp->power_mode = power_mode;
    cp->timeout = 0x0000;

    return bt_hci_cmd_send_sync(BT_HCI_VD_LOW_POWER_MODE, buf, NULL);
}

/*Write BD Address*/
static int hci_le_vendor_set_bd_address(uint8_t bd_address[6])
{
    ncp_ble_set_bd_address_cfg *cp;
    struct net_buf *buf;

    buf = bt_hci_cmd_create(BT_HCI_VD_SET_BD_ADDRESS, sizeof(*cp));
    if (!buf) {
        return -ENOBUFS;
    }

    cp = (ncp_ble_set_bd_address_cfg *)net_buf_add(buf, sizeof(*cp));

    cp->paramater_id = 0xFE;
    cp->bd_addr_len = 0x06;
    memcpy(cp->bd_address, &bd_address[0],sizeof(uint8_t) * cp->bd_addr_len);
    
    return bt_hci_cmd_send_sync(BT_HCI_VD_SET_BD_ADDRESS, buf, NULL);
}

/* Core Handlers */
static int ble_bridge_read_support_cmd(void *tlv) 
{
    // supported_commands(tlv, 0);
    return NCP_BRIDGE_CMD_RESULT_OK;
}

static int ble_bridge_read_support_ser(void *tlv) 
{
    // supported_services(tlv, 0);
    return NCP_BRIDGE_CMD_RESULT_OK;
}

static int ble_bridge_reset_board(void *tlv) {
    // reset_board(tlv, 0);
    return NCP_BRIDGE_CMD_RESULT_OK;
}

/* Gap handlers */

static int ble_bridge_set_adv_data(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    set_adv_data(tlv, 0);

    return ret;
}

static int ble_bridge_start_adv(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    start_advertising(tlv, 0);

    return ret;
}

static int ble_bridge_stop_adv(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    stop_advertising(tlv, 0);

    return ret;  
}

static int ble_bridge_set_scan_param(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    set_scan_parameter(tlv, 0);

    return ret;
}

static int ble_bridge_start_scan(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    start_discovery(tlv, 0);

    return ret;  
}

static int ble_bridge_stop_scan(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    stop_discovery(tlv, 0);

    return ret;  
}

static int ble_bridge_connect(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    ble_connect(tlv, 0);

    return ret;  
}

static int ble_bridge_disconnect(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    disconnect(tlv, 0);

    return ret;  
}

static int ble_bridge_conn_param_update(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    conn_param_update(tlv, 0);

    return ret;  
}

static int ble_bridge_set_filter_list(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    set_filter_list(tlv, 0);

    return ret;  
}

static int ble_bridge_pair(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    pair(tlv, 0);

    return ret;  
}

/* Gatt handlers*/
static int ble_bridge_set_value(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    set_value(tlv, 0);

    return ret;  
}

static int ble_bridge_read_data(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    read_data(tlv, 0);

    return ret;  
}

static int ble_bridge_write_data(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    write_data(tlv, 0);

    return ret;
}

static int ble_bridge_discover_prim_service(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    disc_prim_uuid(tlv, 0);

    return ret;
}

static int ble_bridge_discover_chrc(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    disc_chrc_uuid(tlv, 0);

    return ret;
}

static int ble_bridge_discover_desc(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    disc_desc_uuid(tlv, 0);

    return ret;
}

static int ble_bridge_cfg_indicate(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    config_subscription(tlv, 0, GATT_CFG_INDICATE);

    return ret;
}

static int ble_bridge_register(void *tlv)
{
    gatt_ncp_ble_add_service_cmd_t *cmd = (gatt_ncp_ble_add_service_cmd_t*) tlv;
    gatt_ncp_ble_add_service_rp_t *rp;
    
    struct net_buf_simple *buf = NET_BUF_SIMPLE(NCP_BLE_DATA_MAX_SIZE);
    net_buf_simple_init(buf, 0);
    rp = net_buf_simple_add(buf, sizeof(*rp));

    int ret = NCP_BRIDGE_CMD_RESULT_OK;
    for (size_t i = 0; i < cmd->svc_length; i++)
    {
        rp->status[i] = ncp_ble_register_service(cmd->svc[i]);
    }
    rp->svc_length = cmd->svc_length;
    
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE_GATT_REGISTER_SERVICE, ret, (uint8_t *) rp, sizeof(uint8_t) * (1 + cmd->svc_length));

    return ret;
}

/* Vendor Command handlers */
static int ble_bridge_set_power_mode(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    uint8_t *data = (uint8_t *)tlv;

    if(NCP_BRIDGE_CMD_RESULT_OK != hci_le_vendor_power_mode_cfg(*data)) {
        ret = NCP_BRIDGE_CMD_RESULT_ERROR;
    }
 
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE_VENDOR_POWER_MODE, ret, NULL, 0);

    return ret;
}

static int ble_bridge_set_uart_baud_rate(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;
    /* 
       TODO: This may change UART clock at CPU3 side, not support yet
       TODO: Need de-init UART, reset clock, then re-init UART
    */ 
    return ret;
}

static int ble_bridge_set_device_address(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    uint8_t *data = (uint8_t *)tlv;

    if(NCP_BRIDGE_CMD_RESULT_OK != hci_le_vendor_set_bd_address(data)) {
        ret = NCP_BRIDGE_CMD_RESULT_ERROR;
    }
 
    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE_VENDOR_SET_DEVICE_ADDR, ret, NULL, 0);

    return ret;
}

static int ble_bridge_set_device_name(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;
    char value[CONFIG_BT_DEVICE_NAME_MAX + 1] = {0};

    memcpy(value, tlv, strlen((char *)tlv) + 1);

	if(NCP_BRIDGE_CMD_RESULT_OK != bt_set_name(value)) {
        ret = NCP_BRIDGE_CMD_RESULT_ERROR;
    }

    ble_bridge_prepare_status(NCP_BRIDGE_CMD_BLE_VENDOR_SET_DEVICE_NAME, ret, NULL, 0);

    return ret;
}

static int ble_bridge_config_multi_adv(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    //TODO: 
    return ret;
}

static int ble_bridge_add_service(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    if(!gatt_service_init)
    {
        ncp_ble_init_gatt();
        gatt_service_init = 1;
    }
    add_service(tlv, 0);

    return ret; 
}

static int ble_bridge_add_chra(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    add_characteristic(tlv, 0);

    return ret; 
}

static int ble_bridge_add_descriptor(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    add_descriptor(tlv, 0);

    return ret; 
}

static int ble_bridge_start_service(void *tlv)
{
    int ret = NCP_BRIDGE_CMD_RESULT_OK;

    start_server(NULL, 0);

    return ret; 
}

NCPCmd_DS_COMMAND *ncp_bridge_get_ble_response_buffer()
{
    return (NCPCmd_DS_COMMAND *)(res_buf);
}

/** Prepare TLV command response */
// extern os_mutex_t resp_buf_mutex;
int ble_bridge_prepare_status(uint32_t cmd,
    uint8_t status,
    uint8_t *data,
    size_t len)
{
    //os_mutex_get(&resp_buf_mutex, OS_WAIT_FOREVER);
    NCPCmd_DS_COMMAND *cmd_res = ncp_bridge_get_ble_response_buffer();

    cmd_res->header.cmd        = cmd;
    cmd_res->header.size       = NCP_BRIDGE_CMD_HEADER_LEN + len;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.msg_type   = ((cmd & 0xFFFF) >= 0x80) ? NCP_BRIDGE_MSG_TYPE_EVENT : NCP_BRIDGE_MSG_TYPE_RESP;
    cmd_res->header.result     = status;

    if (data != NULL)
    {
        memcpy((uint8_t *)cmd_res + NCP_BRIDGE_CMD_HEADER_LEN, data, len);
    }

    ble_ncp_send_response((uint8_t *) cmd_res);
    
    //os_mutex_put(&resp_buf_mutex);

    return NCP_BRIDGE_CMD_RESULT_OK;
}

struct cmd_t ble_cmd_core[] = {
   {NCP_BRIDGE_CMD_BLE_CORE_SUPPORT_CMD, "read supported commands", ble_bridge_read_support_cmd, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_CORE_SUPPORT_SER, "read supported services", ble_bridge_read_support_ser, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_CORE_RESET, "reset board", ble_bridge_reset_board, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};
struct cmd_t ble_cmd_gap[] = {
   {NCP_BRIDGE_CMD_BLE_GAP_SET_ADV_DATA, "set adv data", ble_bridge_set_adv_data, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_START_ADV, "start advertising", ble_bridge_start_adv, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_STOP_ADV, "stop advertising", ble_bridge_stop_adv, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_SET_SCAN_PARAM, "set scan parameter", ble_bridge_set_scan_param, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_START_SCAN, "start discovery", ble_bridge_start_scan, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_STOP_SCAN, "stop discovery", ble_bridge_stop_scan, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_CONNECT, "create a connection", ble_bridge_connect, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_DISCONNECT, "terminate a connection", ble_bridge_disconnect, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_CONN_PARAM_UPDATE, "connection parameters update", ble_bridge_conn_param_update, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_SET_FILTER_LIST, "set filter accept list", ble_bridge_set_filter_list, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GAP_PAIR, "enable encryption with peer", ble_bridge_pair, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};
struct cmd_t ble_cmd_gatt[] = {
   {NCP_BRIDGE_CMD_BLE_GATT_ADD_SERVICE, "add primary/secondary service", ble_bridge_add_service, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_GATT_ADD_CHARACTERISTIC, "add characteristic", ble_bridge_add_chra, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_GATT_ADD_DESCRIPTOR, "add descriptor", ble_bridge_add_descriptor, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_GATT_SET_VALUE, "set characteristic/descriptor Value", ble_bridge_set_value, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_GATT_START_SERVICE, "start service", ble_bridge_start_service, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_GAP_SET_FILTER_LIST, "read characteristic/descriptor", ble_bridge_read_data, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_REGISTER_SERVICE, "register profile services", ble_bridge_register, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_READ, "read characteristic/descriptor", ble_bridge_read_data, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_WRITE, "write characteristic/descriptor", ble_bridge_write_data, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_DISC_PRIM, "Discover Primary Service", ble_bridge_discover_prim_service, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_DISC_CHRC, "Discover Characteristics", ble_bridge_discover_chrc, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_DESC_CHRC, "Discover Descriptors", ble_bridge_discover_desc, CMD_SYNC},
   {NCP_BRIDGE_CMD_BLE_GATT_CFG_INDICATE, "Configure service indicate", ble_bridge_cfg_indicate, CMD_SYNC},
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_l2cap[] = {
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_powermgmt[] = {
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_vendor[] = {
   {NCP_BRIDGE_CMD_BLE_VENDOR_POWER_MODE, "set power mode", ble_bridge_set_power_mode, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_VENDOR_SET_UART_BR, "set uart baud rate", ble_bridge_set_uart_baud_rate, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_VENDOR_SET_DEVICE_ADDR, "set device address", ble_bridge_set_device_address, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_VENDOR_SET_DEVICE_NAME, "set device name", ble_bridge_set_device_name, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_BLE_VENDOR_CFG_MULTI_ADV, "config multi adv", ble_bridge_config_multi_adv, CMD_SYNC}, 
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_other[] = {
   {NCP_BRIDGE_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_subclass_t cmd_subclass_ble[8] = {
   {NCP_BRIDGE_CMD_BLE_CORE, ble_cmd_core},
   {NCP_BRIDGE_CMD_BLE_GAP, ble_cmd_gap},
   {NCP_BRIDGE_CMD_BLE_GATT, ble_cmd_gatt},
   {NCP_BRIDGE_CMD_BLE_L2CAP, ble_cmd_l2cap},
   {NCP_BRIDGE_CMD_BLE_POWERMGMT, ble_cmd_powermgmt},
   {NCP_BRIDGE_CMD_BLE_VENDOR, ble_cmd_vendor},
   {NCP_BRIDGE_CMD_BLE_OTHER, ble_cmd_other},
   {NCP_BRIDGE_CMD_INVALID, NULL},
};

