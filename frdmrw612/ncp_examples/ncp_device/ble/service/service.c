/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#if CONFIG_NCP_BLE

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "fsl_os_abstraction.h"
#include "service.h"
#include "FreeRTOS.h"
#include "fsl_debug_console.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "service/bas.h"
#include "service/gatt_server/peripheral_hts.h"
#include "service/gatt_server/peripheral_hrs.h"
#include "service/gatt_server/peripheral_ncs.h"
#include "service/gatt_client/central_hrc.h"
#include "service/gatt_client/central_htc.h"
#include "ncp_glue_ble.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CONFIG_SVC_PRIO         1
#define CONFIG_SVC_STACK_SIZE   configMINIMAL_STACK_SIZE * 8

#define REG_SERVICE_LEN                 (sizeof(svc_list) / sizeof(struct service_t))
#define CONNECT_HANDLER_LEN             (sizeof(svc_cb_connect) / sizeof(struct service_cb_t))
#define DISCONNECT_HANDLER_LEN          (sizeof(svc_cb_disconnect) / sizeof(struct service_cb_t))
#define SECURITY_HANDLER_LEN            (sizeof(svc_cb_security) / sizeof(struct service_cb_t))
#define AUTH_PASS_HANDLER_LEN           (sizeof(svc_cb_auth_pass) / sizeof(struct service_cb_t))
#define AUTH_CANCEL_HANDLER_LEN         (sizeof(svc_cb_auth_cancel) / sizeof(struct service_cb_t))
#define ADV_REPORT_PROCESS_FUN_LEN      (sizeof(p_adv_report) / sizeof(struct adv_report_cb_t))

/*consider ext_adv case*/
#define SVC_ADV_BUF_LEN (sizeof(struct gap_device_found_ev) + 2 * 229)
static struct net_buf_simple *svc_adv_buf = NET_BUF_SIMPLE(SVC_ADV_BUF_LEN);

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
   
struct service_t svc_list[] = {
    {PERIPHERAL_HTS_SERVICE_ID, false, "PERIPHERAL HTS Service", peripheral_hts_task, init_hts_service},
    {PERIPHERAL_HRS_SERVICE_ID, false, "HRS Service", peripheral_hrs_task, init_hrs_service},
    {BAS_SERVICE_ID, false, "BAS Service", peripheral_bas_task, init_bas_service},
    {CENTRAL_HTC_SERVICE_ID, false,"CENTRAL HTC SERVICE", central_htc_task, NULL},
    {CENTRAL_HRC_SERVICE_ID, false, "CENTRAL HRC SERVICE", central_hrc_task, NULL},
    {PERIPHERAL_NCS_SERVICE_ID, false, "PERIPHERAL NCS SERVICE", peripheral_ncs_task, init_ncs_service},
};

struct service_cb_t svc_cb_connect[] = {
    {PERIPHERAL_HTS_SERVICE_ID, peripheral_hts_connect},
    {PERIPHERAL_HRS_SERVICE_ID, peripheral_hrs_connect},
    {BAS_SERVICE_ID, bas_connect},
    {CENTRAL_HTC_SERVICE_ID, central_htc_connect},
    {CENTRAL_HRC_SERVICE_ID, central_hrc_connect},
    {PERIPHERAL_NCS_SERVICE_ID, peripheral_ncs_connect},
};

struct service_cb_t svc_cb_disconnect[] = {
    {PERIPHERAL_HTS_SERVICE_ID, peripheral_hts_disconnect},
    {PERIPHERAL_HRS_SERVICE_ID, peripheral_hrs_disconnect},
    {BAS_SERVICE_ID, bas_disconnect},
    {CENTRAL_HTC_SERVICE_ID, central_htc_disconnect},
    {CENTRAL_HRC_SERVICE_ID, central_hrc_disconnect},
    {PERIPHERAL_NCS_SERVICE_ID, peripheral_ncs_disconnect},
};

struct service_cb_t svc_cb_security[] = {
    {PERIPHERAL_HTS_SERVICE_ID, NULL},
    {PERIPHERAL_HRS_SERVICE_ID, NULL},
    {BAS_SERVICE_ID, NULL},
    {CENTRAL_HTC_SERVICE_ID, NULL},
    {CENTRAL_HRC_SERVICE_ID, NULL},
};

struct service_cb_t svc_cb_auth_pass[] = {
    {PERIPHERAL_HTS_SERVICE_ID, NULL},
    {PERIPHERAL_HRS_SERVICE_ID, NULL},
    {BAS_SERVICE_ID, NULL},
};

struct service_cb_t svc_cb_auth_cancel[] = {
    {PERIPHERAL_HTS_SERVICE_ID, NULL},
    {PERIPHERAL_HRS_SERVICE_ID, NULL},
    {BAS_SERVICE_ID, NULL},
};

struct adv_report_cb_t p_adv_report[] = {
    {PERIPHERAL_HTS_SERVICE_ID, NULL},
    {PERIPHERAL_HRS_SERVICE_ID, NULL},
    {BAS_SERVICE_ID, NULL},
    {CENTRAL_HTC_SERVICE_ID, htc_adv_report_processed},
    {CENTRAL_HRC_SERVICE_ID, hrc_adv_report_processed},
};

struct bt_le_scan_param svc_common_scan_param = {
    .type       = BT_LE_SCAN_TYPE_PASSIVE,
    .options    = BT_LE_SCAN_OPT_NONE,
    .interval   = BT_GAP_SCAN_FAST_INTERVAL,
    .window     = BT_GAP_SCAN_FAST_WINDOW,
};

uint8_t host_svc = 0; /* Indicate profile run at host side, used in GATT clinet */

/*******************************************************************************
 * Code
 ******************************************************************************/
void le_service_connect(struct bt_conn *conn, uint8_t err) {

    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    PRINTF("Connected to peer: %s\n", addr);

#if 0
    if (bt_conn_set_security(conn, BT_SECURITY_L2))
    {
        PRINTF("Failed to set security\n");
    }
#endif

    for (size_t i = 0; i < CONNECT_HANDLER_LEN ; i++)
    {
        if(svc_list[i].is_registered)
        {
            if(svc_cb_connect[i].svc_cb == NULL) 
                continue;
            service_connect_param_t param;
            param.conn = conn;
            svc_cb_connect[i].svc_cb((void *) &param);
        }

    }

}

void le_service_disconnect(struct bt_conn *conn, uint8_t reason) {
    PRINTF("Disconnected (reason 0x%02x)\n", reason);

    for (size_t i = 0; i < DISCONNECT_HANDLER_LEN ; i++)
    {

        if(svc_list[i].is_registered)
        {
            if(svc_cb_disconnect[i].svc_cb == NULL) 
                continue;
            service_connect_param_t param;
            param.conn = conn;
            svc_cb_disconnect[i].svc_cb((void *) &param);
        }
    }
}

void le_service_security(struct bt_conn *conn, bt_security_t level, enum bt_security_err err) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Security changed: %s level %u (error %d)\n", addr, level, err);

    for (size_t i = 0; i < SECURITY_HANDLER_LEN ; i++)
    {

        if(svc_list[i].is_registered)
        {
            if(svc_cb_security[i].svc_cb == NULL) 
                continue;
            service_connect_param_t param;
            param.conn = conn;
            svc_cb_security[i].svc_cb((void *) &param);
        }
    }
}

void le_service_auth_passkey(struct bt_conn *conn, unsigned int passkey) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Passkey for %s: %06u\n", addr, passkey);

    for (size_t i = 0; i < AUTH_PASS_HANDLER_LEN ; i++)
    {

        if(svc_list[i].is_registered)
        {
            if(svc_cb_auth_pass[i].svc_cb == NULL) 
                continue;
            service_connect_param_t param;
            param.conn = conn;
            svc_cb_auth_pass[i].svc_cb((void *) &param);
        }
    }
}

void le_service_auth_cancel(struct bt_conn *conn) {
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);

    for (size_t i = 0; i < AUTH_CANCEL_HANDLER_LEN ; i++)
    {

        if(svc_list[i].is_registered)
        {
            if(svc_cb_auth_cancel[i].svc_cb == NULL) 
                continue;
            service_connect_param_t param;
            param.conn = conn;
            svc_cb_auth_cancel[i].svc_cb((void *) &param);
        }
    }
}

void le_service_device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t evtype,
             struct net_buf_simple *ad)
{
    struct gap_device_found_ev *ev;

    /* cleanup */
    net_buf_simple_init(svc_adv_buf, 0);

    ev = net_buf_simple_add(svc_adv_buf, sizeof(*ev));

    memcpy(ev->address, addr->a.val, sizeof(ev->address));
    ev->address_type = addr->type;
    ev->rssi = rssi;
    ev->flags = GAP_DEVICE_FOUND_FLAG_AD | GAP_DEVICE_FOUND_FLAG_RSSI;
    ev->eir_data_len = ad->len;
    memcpy(net_buf_simple_add(svc_adv_buf, ad->len), ad->data, ad->len);
  
    // send adv report event to Host
    ble_prepare_status(NCP_EVENT_ADV_REPORT, NCP_CMD_RESULT_OK, svc_adv_buf->data, svc_adv_buf->len);
    
    // only care about connectable adv for service profile
    if (evtype == BT_GAP_ADV_TYPE_ADV_IND || evtype == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
        for (size_t j = 0; j < ADV_REPORT_PROCESS_FUN_LEN ; j++)
        {
            if(svc_list[j].is_registered)
            {
                for(int i = 0; i < ev->eir_data_len; ) {
                    
                    if(p_adv_report[j].p_adv_report_fn == NULL) 
                        continue;
                  
                    struct adv_report_data data;
                    uint8_t ad_len;
                    
                    ad_len = ev->eir_data[i++];
                    data.type = ev->eir_data[i++];
                    data.data_len = ad_len - 1;
                    data.data = &ev->eir_data[i];
                    i += data.data_len;
                                       
                    if(!p_adv_report[j].p_adv_report_fn(&data, (void *)addr)) {
                        // if adv match, terminate the process loop
                        return;
                    }                           
                }   
            }
        }
    }
    
    // clean buf
    net_buf_simple_reset(svc_adv_buf);
    
}

void svc_scan_start(void)
{
    
    int status;
    
    if(bt_le_scan_start(&svc_common_scan_param, le_service_device_found) < 0) {
        status = NCP_CMD_RESULT_ERROR;
    }else {
        status = NCP_CMD_RESULT_OK;
    }
   
    ble_prepare_status(NCP_RSP_BLE_GAP_START_SCAN, status, NULL, 0);
}

int ncp_ble_register_service(uint8_t id) {

    int ret;
    int svc_id = id - PERIPHERAL_HTS_SERVICE_ID;
    if (svc_id < 0 || svc_id >= REG_SERVICE_LEN)
    {
       PRINTF("invalid service id : %d\r\n", id);
       return NCP_CMD_RESULT_INVALID_INDEX;
    }
    
    // fixme : peripheral and central use the same service, fix this
    if (svc_list[svc_id].is_registered)
    {
       PRINTF("%s already registered\r\n", svc_list[id].def);
       return NCP_CMD_RESULT_ERROR;
    }
    svc_list[svc_id].is_registered = true;

    if (svc_list[svc_id].init != NULL)
    {
       // init service attribute
       svc_list[svc_id].init();
    }

    if (svc_list[svc_id].svc_task == NULL)
    {
        return NCP_CMD_RESULT_OK;
    }

    // create service task
    ret = xTaskCreate(svc_list[svc_id].svc_task, "service_task_#id", CONFIG_SVC_STACK_SIZE, NULL, CONFIG_SVC_PRIO, NULL);
    if (ret != pdPASS)
    {
      PRINTF("register service %d failed\r\n", id);
      return NCP_CMD_RESULT_ERROR;
    }
    return NCP_CMD_RESULT_OK;
}

#endif