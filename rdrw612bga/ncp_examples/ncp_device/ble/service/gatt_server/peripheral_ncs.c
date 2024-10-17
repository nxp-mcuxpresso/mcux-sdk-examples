/*
 * Copyright 2021-2024 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#if CONFIG_NCP_BLE

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include <fsl_debug_console.h>
#include <sys/byteorder.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "service.h"

#include "ncp_glue_ble.h"
#include "peripheral_ncs.h"


/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct bt_conn *default_conn = NULL;
static bool is_registered = false;
NCP_NCS_INFO_EV ncs_info;

static const struct bt_data ncs_ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, ADV_NAME, 12),
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void ncs_adv_start(void);


static ssize_t write_ssid(struct bt_conn *conn, const struct bt_gatt_attr *attr,
             const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    PRINTF("write_ssid, len: %d, ssid: %s\n", len, buf);

    memset((void *)(ncs_info.ssid), 0, sizeof(ncs_info.ssid));

    ncs_info.ssid_len = len;
    memcpy(ncs_info.ssid, buf, len);

    return len;
}

static ssize_t write_pswd(struct bt_conn *conn, const struct bt_gatt_attr *attr,
             const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    PRINTF("write_pswd, len: %d, password: %s\n", len, buf);
    
    memset((void *)ncs_info.pswd, 0, sizeof(ncs_info.pswd));

    ncs_info.pswd_len = len;
    memcpy(ncs_info.pswd, buf, len);

    return len;
}

static ssize_t write_secu(struct bt_conn *conn, const struct bt_gatt_attr *attr,
             const void *buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    PRINTF("write_secu, len: %d, secure mode: %s\n", len, buf);
    
    memset((void *)ncs_info.secu, 0, sizeof(ncs_info.secu));

    ncs_info.secu_len = len;
    memcpy(ncs_info.secu, buf, len);

    // send commission info to Host
    ble_prepare_status(NCP_EVENT_GATT_NCS_INFO, NCP_CMD_RESULT_OK, (uint8_t *)&ncs_info, sizeof(NCP_NCS_INFO_EV));

    return len;
}

/*******************************************************************************
 * Definitions
 ******************************************************************************/
//char ncs_data_ssid[30];

static struct bt_gatt_attr ncs_attrs[] = {
    /* NXP Commission Service Declaration */
    BT_GATT_PRIMARY_SERVICE(COMMISSION_UUID),
    BT_GATT_CHARACTERISTIC(COMMISSION_UUID_SSID, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, write_ssid, NULL),
    BT_GATT_CHARACTERISTIC(COMMISSION_UUID_PSWD, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, write_pswd, NULL),
    BT_GATT_CHARACTERISTIC(COMMISSION_UUID_SECU, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, write_secu, NULL),
};

// define ncs service
static struct bt_gatt_service ncs_svc = BT_GATT_SERVICE(ncs_attrs);

/*******************************************************************************
 * Code
 ******************************************************************************/

void peripheral_ncs_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t*) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);
}

void peripheral_ncs_disconnect(void *args)
{
    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
    
    ncs_adv_start();
}

void init_ncs_service(void) {
    if (!is_registered)
    {
        bt_gatt_service_register(&ncs_svc);
        is_registered = true;
    }
}

static void ncs_adv_start(void)
{
    int status;
    
    // should use BT_LE_ADV_CONN instead of BT_LE_ADV_CONN_NAME when wanting adv data to contain name, otherwise start adv will fail
    // when use BT_LE_ADV_CONN_NAME setting, the adv name will fetch from bt_dev.name once enable CONFIG_BT_DEVICE_NAME_DYNAMIC macro
    if(bt_le_adv_start(BT_LE_ADV_CONN, ncs_ad, ARRAY_SIZE(ncs_ad), NULL, 0) < 0) {
        status = NCP_CMD_RESULT_ERROR;
    }else {
        status = NCP_CMD_RESULT_OK;
    }
   
    ble_prepare_status(NCP_RSP_BLE_GAP_START_ADV, status, NULL, 0);
}

void peripheral_ncs_task(void *pvParameters)
{
    ncs_adv_start();
  
    while(1)
    {
        vTaskDelay(1000);
    }
}

#endif /* CONFIG_NCP_BLE */