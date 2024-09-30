/* @file ncp_glue_ble.c
 *
 *  @brief This file contains declaration of the API functions.
 *
 *  Copyright 2008-2023 NXP
 *
 *  SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_BLE

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
extern void start_advertising(void);
extern void stop_advertising(const uint8_t *data, uint16_t len);
extern void set_scan_parameter(const uint8_t *data, uint16_t len);
extern void start_discovery(const uint8_t *data, uint16_t len);
extern void stop_discovery(const uint8_t *data, uint16_t len);
extern void ble_connect(const uint8_t *data, uint16_t len);
extern void disconnect(const uint8_t *data, uint16_t len);
#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
extern void set_data_len(const uint8_t *data, uint16_t len);
#endif
#if (defined(CONFIG_BT_USER_PHY_UPDATE) && ((CONFIG_BT_USER_PHY_UPDATE) > 0U))
extern void set_phy(const uint8_t *data, uint16_t len);
#endif
extern void conn_param_update(const uint8_t *data, uint16_t len);
extern void set_filter_list(const uint8_t *data, uint16_t len);
extern void pair(const uint8_t *data, uint16_t len);
extern void disc_prim_uuid(uint8_t *data, uint16_t len);
extern void disc_chrc_uuid(uint8_t *data, uint16_t len);
extern void disc_desc_uuid(uint8_t *data, uint16_t len);
extern void disc_desc_uuid(uint8_t *data, uint16_t len);
extern void config_subscription(uint8_t *data, uint16_t len, uint16_t op);

//L2CAP
extern void ble_l2cap_set_recv(uint8_t *data, uint16_t len);
extern void ble_l2cap_metrics(uint8_t *data, uint16_t len);
extern void bt_l2cap_register(uint8_t *data, uint16_t len);
extern void ble_l2cap_connect(uint8_t *data, uint16_t len);
extern void ble_l2cap_disconnect(uint8_t *data, uint16_t len);
extern void ble_l2cap_send_data(uint8_t *data, uint16_t len);
//GATT
extern int add_service(const void *cmd, uint16_t cmd_len, void *rsp);
extern int add_characteristic(const void *cmd, uint16_t cmd_len, void *rsp);
extern int add_descriptor(const void *cmd, uint16_t cmd_len, void *rsp);
extern int start_server(uint8_t *data, uint16_t len);
extern void set_value(const uint8_t *data, uint16_t len);
extern void read_data(const uint8_t *data, uint16_t len);
extern void write_data(const uint8_t *data, uint16_t len);

extern uint8_t ncp_ble_init_gatt(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

uint8_t ble_res_buf[NCP_INBUF_SIZE];

static uint8_t gatt_service_init = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

// #ifdef CONFIG_NCP__DEBUG
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
    ble_prepare_status(NCP_EVENT_ATTR_VALUE_CHANGED, 0, event_buf, len);
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
static int ble_read_support_cmd(void *tlv) 
{
    // supported_commands(tlv, 0);
    return NCP_CMD_RESULT_OK;
}

static int ble_read_support_ser(void *tlv) 
{
    // supported_services(tlv, 0);
    return NCP_CMD_RESULT_OK;
}

static int ble_reset_board(void *tlv) {
    // reset_board(tlv, 0);
    return NCP_CMD_RESULT_OK;
}

/* Gap handlers */

static int ble_set_adv_data(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_adv_data(tlv, 0);

    return ret;
}

static int ble_start_adv(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    start_advertising();

    return ret;
}

static int ble_stop_adv(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    stop_advertising(tlv, 0);

    return ret;  
}

static int ble_set_scan_param(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_scan_parameter(tlv, 0);

    return ret;
}

static int ble_start_scan(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    start_discovery(tlv, 0);

    return ret;  
}

static int ble_stop_scan(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    stop_discovery(tlv, 0);

    return ret;  
}

static int ble_start_connect(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_connect(tlv, 0);

    return ret;  
}

static int ble_start_disconnect(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    disconnect(tlv, 0);

    return ret;  
}

#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
static int ble_set_data_len(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_data_len(tlv, 0);

    return ret;
}
#endif

#if (defined(CONFIG_BT_USER_PHY_UPDATE) && ((CONFIG_BT_USER_PHY_UPDATE) > 0U))
static int ble_set_phy(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_phy(tlv, 0);

    return ret;
}
#endif

static int ble_conn_param_update(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    conn_param_update(tlv, 0);

    return ret;  
}

static int ble_set_filter_list(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_filter_list(tlv, 0);

    return ret;  
}

static int ble_pair(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    pair(tlv, 0);

    return ret;  
}

/* Gatt handlers*/
static int ble_set_value(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    set_value(tlv, 0);

    return ret;  
}

static int ble_read_data(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    read_data(tlv, 0);

    return ret;  
}

static int ble_write_data(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    write_data(tlv, 0);

    return ret;
}

static int ble_discover_prim_service(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    disc_prim_uuid(tlv, 0);

    return ret;
}

static int ble_discover_chrc(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    disc_chrc_uuid(tlv, 0);

    return ret;
}

static int ble_discover_desc(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    disc_desc_uuid(tlv, 0);

    return ret;
}

static int ble_cfg_indicate(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    config_subscription(tlv, 0, GATT_CFG_INDICATE);

    return ret;
}

static int ble_cfg_notify(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    config_subscription(tlv, 0, GATT_CFG_NOTIFY);

    return ret;
}

static int ble_register(void *tlv)
{
    gatt_ncp_ble_add_service_cmd_t *cmd = (gatt_ncp_ble_add_service_cmd_t*) tlv;
    gatt_ncp_ble_add_service_rp_t *rp;
    
    struct net_buf_simple *buf = NET_BUF_SIMPLE(NCP_BLE_DATA_MAX_SIZE);
    net_buf_simple_init(buf, 0);
    rp = net_buf_simple_add(buf, sizeof(*rp));

    int ret = NCP_CMD_RESULT_OK;
    for (size_t i = 0; i < cmd->svc_length; i++)
    {
        rp->status[i] = ncp_ble_register_service(cmd->svc[i]);
    }
    rp->svc_length = cmd->svc_length;
    
    ble_prepare_status(NCP_RSP_BLE_GATT_REGISTER_SERVICE, ret, (uint8_t *) rp, sizeof(uint8_t) * (1 + cmd->svc_length));

    return ret;
}

/* Vendor Command handlers */
static int ble_set_power_mode(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    uint8_t *data = (uint8_t *)tlv;

    if(NCP_CMD_RESULT_OK != hci_le_vendor_power_mode_cfg(*data)) {
        ret = NCP_CMD_RESULT_ERROR;
    }
 
    ble_prepare_status(NCP_RSP_BLE_VENDOR_POWER_MODE, ret, NULL, 0);

    return ret;
}

static int ble_set_uart_baud_rate(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;
    /* 
       TODO: This may change UART clock at CPU3 side, not support yet
       TODO: Need de-init UART, reset clock, then re-init UART
    */ 
    return ret;
}

static int ble_set_device_address(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    uint8_t *data = (uint8_t *)tlv;

    if(NCP_CMD_RESULT_OK != hci_le_vendor_set_bd_address(data)) {
        ret = NCP_CMD_RESULT_ERROR;
    }
 
    ble_prepare_status(NCP_RSP_BLE_VENDOR_SET_DEVICE_ADDR, ret, NULL, 0);

    return ret;
}

static int ble_set_device_name(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;
    char value[CONFIG_BT_DEVICE_NAME_MAX + 1] = {0};

    memcpy(value, tlv, strlen((char *)tlv) + 1);

	if(NCP_CMD_RESULT_OK != bt_set_name(value)) {
        ret = NCP_CMD_RESULT_ERROR;
    }

    ble_prepare_status(NCP_RSP_BLE_VENDOR_SET_DEVICE_NAME, ret, NULL, 0);

    return ret;
}

static int ble_config_multi_adv(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    //TODO: 
    return ret;
}

static int ble_host_service_add(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK, tlv_buf_len = 0;
    uint8_t *ptlv_pos                  = NULL;
    NCP_TLV_HEADER  *ptlv_header = NULL;
    uint8_t info[1], auto_start = 0;

    struct bt_data data;
    struct gap_set_adv_data_cmd adv; 
    uint8_t adv_uuids[] = {0}, adv_uuid_all_len = 0;
    uint16_t rsp = 0;

    NCP_CMD_SERVICE_ADD *service_add = (NCP_CMD_SERVICE_ADD *)tlv;

    (void)memset(info, 0, sizeof(info));

    if(!gatt_service_init)
    {
        ncp_ble_init_gatt();
        gatt_service_init = 1;
    }

    ptlv_pos    = service_add->tlv_buf;
    tlv_buf_len = service_add->tlv_buf_len;
    do
    {
        ptlv_header = (NCP_TLV_HEADER *)ptlv_pos;

        switch(ptlv_header->type)
        {
            case NCP_CMD_GATT_ADD_SERVICE_TLV:
                if(!ncp_ble_test_bit(info, NCP_CMD_GATT_ADD_SERVICE_TLV))
                {
                    gatt_add_service_cmd_t *add_service_tlv = (gatt_add_service_cmd_t *)ptlv_pos;
                    ret = add_service(add_service_tlv, 0, &rsp);
                    memcpy(&adv_uuids[adv_uuid_all_len], add_service_tlv->uuid, add_service_tlv->uuid_length);
                    adv_uuid_all_len += add_service_tlv->uuid_length;
                    ncp_ble_set_bit(info, NCP_CMD_GATT_ADD_SERVICE_TLV);
                }
                break;
            case NCP_CMD_GATT_ADD_CHRC_TLV:
                {
                    gatt_add_characteristic_cmd_t * add_chrc_tlv = (gatt_add_characteristic_cmd_t *)ptlv_pos;
                    add_chrc_tlv->svc_id = rsp;

                    ret = add_characteristic(add_chrc_tlv, 0, &rsp);
                }
                break;
            case NCP_CMD_GATT_ADD_DESC_TLV:
                {
                    gatt_add_descriptor_cmd_t * add_desc_tlv = (gatt_add_descriptor_cmd_t *)ptlv_pos;
                    add_desc_tlv->char_id = rsp;

                    ret = add_descriptor(add_desc_tlv, 0, NULL);
                }
                break;
            case NCP_CMD_GATT_START_SVC_TLV:
                {
                    gatt_start_service_cmd_t *start_service_tlv = (gatt_start_service_cmd_t *)ptlv_pos;
                    auto_start = start_service_tlv->started;
                    ret = start_server(NULL, 0);
                }
                break;
        }

        ptlv_pos += NCP_TLV_HEADER_LEN + ptlv_header->size;
        tlv_buf_len -= NCP_TLV_HEADER_LEN + ptlv_header->size;
    } while (tlv_buf_len > 0);

    ble_prepare_status(NCP_RSP_BLE_HOST_SERVICE_ADD, ret, NULL, 0);

    if (auto_start)
    {
        // set adv data
        memset(&adv, 0, sizeof(struct gap_set_adv_data_cmd));
        data = (struct bt_data)BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR));
        bt_data_serialize(&data, &adv.data[adv.adv_data_len]);
        adv.adv_data_len += bt_data_get_len(&data,1);

        data = (struct bt_data)BT_DATA(BT_DATA_UUID16_ALL, adv_uuids, adv_uuid_all_len);
        bt_data_serialize(&data, &adv.data[adv.adv_data_len]);
        adv.adv_data_len += bt_data_get_len(&data, 1);

        const char *name = bt_get_name();
        data = (struct bt_data)BT_DATA(BT_DATA_NAME_COMPLETE, name, strlen(name));
        bt_data_serialize(&data, &adv.data[adv.adv_data_len]);
        adv.adv_data_len += bt_data_get_len(&data, 1);
        set_adv_data((uint8_t *)&adv, 0);
        // start adv
        start_advertising();
    }

    return ret; 
}

static int ble_start_service(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;
    NCP_CMD_START_SERVICE *cmd = (NCP_CMD_START_SERVICE*) tlv;

    extern uint8_t host_svc;
    host_svc = cmd->form_host;
    ret = ncp_ble_register_service(cmd->svc_id);
    
#if 0
    ret = start_server(NULL, 0);
#endif
    ble_prepare_status(NCP_RSP_BLE_GATT_START_SERVICE, ret, NULL, 0);
    return ret;
}

static int ble_start_l2cap_connect(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_l2cap_connect(tlv, 0);

    return ret;
}

static int ble_start_l2cap_disconnect(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_l2cap_disconnect(tlv, 0);

    return ret;
}

static int ble_l2cap_register(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    bt_l2cap_register(tlv, 0);

    return ret;
}

static int ble_l2cap_receive(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_l2cap_set_recv(tlv, 0);

    return ret;
}

static int ble_l2cap_send(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_l2cap_send_data(tlv, 0);

    return ret;
}

static int ble_start_l2cap_metrics(void *tlv)
{
    int ret = NCP_CMD_RESULT_OK;

    ble_l2cap_metrics(tlv, 0);

    return ret;
}

NCPCmd_DS_COMMAND *ncp__get_ble_response_buffer()
{
    return (NCPCmd_DS_COMMAND *)(ble_res_buf);
}

/** Prepare TLV command response */
// extern os_mutex_t resp_buf_mutex;
int ble_prepare_status(uint32_t cmd,
    uint8_t status,
    uint8_t *data,
    size_t len)
{
    //os_mutex_get(&resp_buf_mutex, OS_WAIT_FOREVER);
    ncp_get_ble_resp_buf_lock();
    NCPCmd_DS_COMMAND *cmd_res = ncp__get_ble_response_buffer();

    cmd_res->header.cmd        = cmd;
    cmd_res->header.size       = NCP_CMD_HEADER_LEN + len;
    cmd_res->header.seqnum     = 0x00;
    cmd_res->header.result     = status;

    if (data != NULL)
    {
        memcpy((uint8_t *)cmd_res + NCP_CMD_HEADER_LEN, data, len);
    }

    ble_ncp_send_response((uint8_t *) cmd_res);
    
    //os_mutex_put(&resp_buf_mutex);

    return NCP_CMD_RESULT_OK;
}

struct cmd_t ble_cmd_core[] = {
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};
struct cmd_t ble_cmd_gap[] = {
   {NCP_CMD_BLE_GAP_SET_ADV_DATA, "set adv data", ble_set_adv_data, CMD_SYNC},
   {NCP_CMD_BLE_GAP_START_ADV, "start advertising", ble_start_adv, CMD_SYNC},
   {NCP_CMD_BLE_GAP_STOP_ADV, "stop advertising", ble_stop_adv, CMD_SYNC},
   {NCP_CMD_BLE_GAP_SET_SCAN_PARAM, "set scan parameter", ble_set_scan_param, CMD_SYNC},
   {NCP_CMD_BLE_GAP_START_SCAN, "start discovery", ble_start_scan, CMD_SYNC},
   {NCP_CMD_BLE_GAP_STOP_SCAN, "stop discovery", ble_stop_scan, CMD_SYNC},
   {NCP_CMD_BLE_GAP_CONNECT, "create a connection", ble_start_connect, CMD_SYNC},
   {NCP_CMD_BLE_GAP_DISCONNECT, "terminate a connection", ble_start_disconnect, CMD_SYNC},
#if (defined(CONFIG_BT_USER_DATA_LEN_UPDATE) && (CONFIG_BT_USER_DATA_LEN_UPDATE > 0))
   {NCP_CMD_BLE_GAP_SET_DATA_LEN, "set data len", ble_set_data_len, CMD_SYNC},
#endif
#if (defined(CONFIG_BT_USER_PHY_UPDATE) && ((CONFIG_BT_USER_PHY_UPDATE) > 0U))
   {NCP_CMD_BLE_GAP_SET_PHY, "set phy", ble_set_phy, CMD_SYNC},
#endif
   {NCP_CMD_BLE_GAP_CONN_PARAM_UPDATE, "connection parameters update", ble_conn_param_update, CMD_SYNC},
   {NCP_CMD_BLE_GAP_SET_FILTER_LIST, "set filter accept list", ble_set_filter_list, CMD_SYNC},
   {NCP_CMD_BLE_GAP_PAIR, "enable encryption with peer", ble_pair, CMD_SYNC}, 
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};
struct cmd_t ble_cmd_gatt[] = {
   {NCP_CMD_BLE_HOST_SERVICE_ADD, "host service add", ble_host_service_add, CMD_SYNC},
   {NCP_CMD_BLE_GATT_START_SERVICE, "start service", ble_start_service, CMD_SYNC},
   {NCP_CMD_BLE_GATT_SET_VALUE, "set characteristic/descriptor Value", ble_set_value, CMD_SYNC}, 
   {NCP_CMD_BLE_GAP_SET_FILTER_LIST, "read characteristic/descriptor", ble_read_data, CMD_SYNC},
   {NCP_CMD_BLE_GATT_REGISTER_SERVICE, "register profile services", ble_register, CMD_SYNC},
   {NCP_CMD_BLE_GATT_READ, "read characteristic/descriptor", ble_read_data, CMD_SYNC},
   {NCP_CMD_BLE_GATT_WRITE, "write characteristic/descriptor", ble_write_data, CMD_SYNC},
   {NCP_CMD_BLE_GATT_DISC_PRIM, "Discover Primary Service", ble_discover_prim_service, CMD_SYNC},
   {NCP_CMD_BLE_GATT_DISC_CHRC, "Discover Characteristics", ble_discover_chrc, CMD_SYNC},
   {NCP_CMD_BLE_GATT_DESC_CHRC, "Discover Descriptors", ble_discover_desc, CMD_SYNC},
   {NCP_CMD_BLE_GATT_CFG_NOTIFY, "Configure service notify", ble_cfg_notify, CMD_SYNC},
   {NCP_CMD_BLE_GATT_CFG_INDICATE, "Configure service indicate", ble_cfg_indicate, CMD_SYNC},
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_l2cap[] = {
   {NCP_CMD_BLE_L2CAP_CONNECT, "l2cap connect", ble_start_l2cap_connect, CMD_SYNC},
   {NCP_CMD_BLE_L2CAP_DISCONNECT, "l2cap disconnect", ble_start_l2cap_disconnect, CMD_SYNC},
   {NCP_CMD_BLE_L2CAP_REGISTER, "l2cap register", ble_l2cap_register, CMD_SYNC},
   {NCP_CMD_BLE_L2CAP_RECEIVE, "l2cap recieve", ble_l2cap_receive, CMD_SYNC},
   {NCP_CMD_BLE_L2CAP_SEND, "l2cap send", ble_l2cap_send, CMD_SYNC},
   {NCP_CMD_BLE_L2CAP_METRICS, "l2cap metrics", ble_start_l2cap_metrics, CMD_SYNC},
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_powermgmt[] = {
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_vendor[] = {
   {NCP_CMD_BLE_VENDOR_POWER_MODE, "set power mode", ble_set_power_mode, CMD_SYNC}, 
   {NCP_CMD_BLE_VENDOR_SET_UART_BR, "set uart baud rate", ble_set_uart_baud_rate, CMD_SYNC}, 
   {NCP_CMD_BLE_VENDOR_SET_DEVICE_ADDR, "set device address", ble_set_device_address, CMD_SYNC}, 
   {NCP_CMD_BLE_VENDOR_SET_DEVICE_NAME, "set device name", ble_set_device_name, CMD_SYNC}, 
   {NCP_CMD_BLE_VENDOR_CFG_MULTI_ADV, "config multi adv", ble_config_multi_adv, CMD_SYNC}, 
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_t ble_cmd_other[] = {
   {NCP_CMD_INVALID, NULL, NULL, NULL},
};

struct cmd_subclass_t cmd_subclass_ble[8] = {
   {NCP_CMD_BLE_CORE, ble_cmd_core},
   {NCP_CMD_BLE_GAP, ble_cmd_gap},
   {NCP_CMD_BLE_GATT, ble_cmd_gatt},
   {NCP_CMD_BLE_L2CAP, ble_cmd_l2cap},
   {NCP_CMD_BLE_POWERMGMT, ble_cmd_powermgmt},
   {NCP_CMD_BLE_VENDOR, ble_cmd_vendor},
   {NCP_CMD_BLE_OTHER, ble_cmd_other},
   {NCP_CMD_INVALID, NULL},
};

#endif /* CONFIG_NCP_BLE */
