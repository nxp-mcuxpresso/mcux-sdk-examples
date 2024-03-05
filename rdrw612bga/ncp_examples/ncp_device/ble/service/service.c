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
#include "service/hts.h"
#include "service/hrs.h"
#include "service/bas.h"
#include "service/gatt_server/peripheral_hrs.h"
#include "service/gatt_client/central_hrc.h"
#include "service/gatt_client/central_htc.h"
#include "ncp_glue_ble.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define CONFIG_SVC_PRIO         1
#define CONFIG_SVC_STACK_SIZE   configMINIMAL_STACK_SIZE * 8

#define REG_SERVICE_LEN           (sizeof(svc_list) / sizeof(struct service_t))
#define CONNECT_HANDLER_LEN       (sizeof(svc_cb_connect) / sizeof(struct service_cb_t))
#define DISCONNECT_HANDLER_LEN    (sizeof(svc_cb_disconnect) / sizeof(struct service_cb_t))
#define SECURITY_HANDLER_LEN      (sizeof(svc_cb_security) / sizeof(struct service_cb_t))
#define AUTH_PASS_HANDLER_LEN     (sizeof(svc_cb_auth_pass) / sizeof(struct service_cb_t))
#define AUTH_CANCEL_HANDLER_LEN   (sizeof(svc_cb_auth_cancel) / sizeof(struct service_cb_t))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/


/*******************************************************************************
 * Variables
 ******************************************************************************/
// service register flag


struct service_t svc_list[] = {
    {PERIPHERAL_HTS_SERVICE_ID, false, "PERIPHERAL HTS Service", peripheral_hts_task, init_hts_service},
    {PERIPHERAL_HRS_SERVICE_ID, false, "HRS Service", peripheral_hrs_task, init_hrs_service},
    {BAS_SERVICE_ID, false, "BAS Service", peripheral_bas_task, init_bas_service},
    {CENTRAL_HTC_SERVICE_ID, false,"CENTRAL HTC SERVICE", NULL, NULL},
    {CENTRAL_HRC_SERVICE_ID, false, "CENTRAL HRC SERVICE", NULL, NULL},
};

struct service_cb_t svc_cb_connect[] = {
    {PERIPHERAL_HTS_SERVICE_ID, peripheral_hts_connect},
    {PERIPHERAL_HRS_SERVICE_ID, peripheral_hrs_connect},
    {BAS_SERVICE_ID, NULL},
    {CENTRAL_HTC_SERVICE_ID, central_htc_connect},
    {CENTRAL_HRC_SERVICE_ID, central_hrc_connect},
};

struct service_cb_t svc_cb_disconnect[] = {
    {PERIPHERAL_HTS_SERVICE_ID, peripheral_hts_disconnect},
    {PERIPHERAL_HRS_SERVICE_ID, peripheral_hrs_disconnect},
    {BAS_SERVICE_ID, NULL},
    {CENTRAL_HTC_SERVICE_ID, central_htc_disconnect},
    {CENTRAL_HRC_SERVICE_ID, central_hrc_disconnect},
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

/*******************************************************************************
 * Code
 ******************************************************************************/
void le_service_connect(struct bt_conn *conn, uint8_t err) {

    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
    PRINTF("Connected to peer: %s\n", addr);

#if CONFIG_BT_SMP
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

int ncp_ble_register_service(uint8_t id) {

    int ret;
    int svc_id = id - PERIPHERAL_HTS_SERVICE_ID;
    if (svc_id < 0 || svc_id >= REG_SERVICE_LEN)
    {
       PRINTF("invalid service id : %d\r\n", id);
       return NCP_BRIDGE_CMD_RESULT_INVALID_INDEX;
    }
    
    // fixme : peripheral and central use the same service, fix this
    if (svc_list[svc_id].is_registered)
    {
       PRINTF("%s already registered\r\n", svc_list[id].def);
       return NCP_BRIDGE_CMD_RESULT_ERROR;
    }
    svc_list[svc_id].is_registered = true;

    if (svc_list[svc_id].init != NULL)
    {
       // init service attribute
       svc_list[svc_id].init();
    }

    if (svc_list[svc_id].svc_task == NULL)
    {
        return NCP_BRIDGE_CMD_RESULT_OK;
    }

    // create service task
    ret = xTaskCreate(svc_list[svc_id].svc_task, "service_task_#id", CONFIG_SVC_STACK_SIZE, NULL, CONFIG_SVC_PRIO, NULL);
    if (ret != pdPASS)
    {
      PRINTF("register service %d failed\r\n", id);
      return NCP_BRIDGE_CMD_RESULT_ERROR;
    }
    return NCP_BRIDGE_CMD_RESULT_OK;
}
