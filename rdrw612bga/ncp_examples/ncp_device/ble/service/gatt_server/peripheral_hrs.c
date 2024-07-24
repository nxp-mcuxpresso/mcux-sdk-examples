/*
 * Copyright 2021 NXP
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#if CONFIG_NCP_BLE

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno/errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include "service.h"

#include "ncp_glue_ble.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define BT_DIS_HRS_NAME      "NCP HRS Demo"

static uint8_t hrs_blsc = 0x01;
static bool is_registered = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void hrs_adv_start(void);

static int bt_hrs_notify(uint16_t heartrate);

static void hrmc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);

static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, uint16_t len, uint16_t offset);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct bt_conn *default_conn = NULL;

static const struct bt_data hrs_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA(BT_DATA_NAME_COMPLETE, BT_DIS_HRS_NAME, 12),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_HRS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))
};

static struct bt_gatt_attr hrs_attrs[] = {
    /* Heart Rate Service Declaration */
    BT_GATT_PRIMARY_SERVICE(BT_UUID_HRS),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_MEASUREMENT, BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
	BT_GATT_CCC(hrmc_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_BODY_SENSOR, BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ, read_blsc, NULL, NULL),
	BT_GATT_CHARACTERISTIC(BT_UUID_HRS_CONTROL_POINT, BT_GATT_CHRC_WRITE,
			       BT_GATT_PERM_NONE, NULL, NULL, NULL),
};

// define hrs service
static struct bt_gatt_service hrs_svc = BT_GATT_SERVICE(hrs_attrs);

/*******************************************************************************
 * Code
 ******************************************************************************/
void peripheral_hrs_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t *) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);
}

void peripheral_hrs_disconnect(void *args)
{
    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
    
    //restart adv
    hrs_adv_start();
}

static void peripheral_hrs_notify(void)
{
	static uint8_t heartrate = 90U;

	/* Heartrate measurements simulation */
	heartrate++;
	if (heartrate == 160U) {
		heartrate = 90U;
	}

	bt_hrs_notify(heartrate);
}

static void hrmc_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	// BT_INFO("HRS notifications %s", notif_enabled ? "enabled" : "disabled");
    (void)notif_enabled;
}

static ssize_t read_blsc(struct bt_conn *conn, const struct bt_gatt_attr *attr,
			 void *buf, uint16_t len, uint16_t offset)
{
	return bt_gatt_attr_read(conn, attr, buf, len, offset, &hrs_blsc,
				 sizeof(hrs_blsc));
}

static int bt_hrs_notify(uint16_t heartrate)
{
	int rc;
	static uint8_t hrm[2];

	hrm[0] = 0x06; /* uint8, sensor contact */
	hrm[1] = heartrate;

	rc = bt_gatt_notify(NULL, &hrs_svc.attrs[1], &hrm, sizeof(hrm));

	return rc == -ENOTCONN ? 0 : rc;
}

static void hrs_adv_start(void)
{
    int status;
    
    if(bt_le_adv_start(BT_LE_ADV_CONN, hrs_ad, ARRAY_SIZE(hrs_ad), NULL, 0) < 0) {
        status = NCP_CMD_RESULT_ERROR;
    }else {
        status = NCP_CMD_RESULT_OK;
    }
   
    ble_prepare_status(NCP_RSP_BLE_GAP_START_ADV, status, NULL, 0);
}

void init_hrs_service(void)
{
	if (!is_registered) {
		// register hrs service
		PRINTF("REGISTER HRS SERVICE\n");
    	bt_gatt_service_register(&hrs_svc);
		is_registered = true;
	}
}

void peripheral_hrs_task(void *pvParameters)
{
    hrs_adv_start();
    
    while(1)
    {
        vTaskDelay(1000);

		/* Heartrate measurements simulation */
        peripheral_hrs_notify();
    }
}

#endif
