/** @file
 *  @brief GATT Battery Service
 */

/*
 * Copyright 2021 NXP
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#if CONFIG_NCP_BLE

#include <errno/errno.h>
#include <stdbool.h>
#include <zephyr/types.h>
#include <sys/atomic.h>

#include <porting.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/uuid.h>
#include "bas.h"
#include "service.h"
#include "ncp_glue_ble.h"

#define LOG_ENABLE IS_ENABLED(CONFIG_BT_DEBUG_SERVICE)
#define LOG_MODULE_NAME bt_bas
#include "fsl_component_log.h"
LOG_MODULE_DEFINE(LOG_MODULE_NAME, kLOG_LevelTrace);

#define BT_DIS_BAS_NAME      "NCP BAS Demo"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint8_t battery_level = 100U;
static bool is_registered = false;
static struct bt_conn *default_conn = NULL;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void bas_adv_start(void);
static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr, uint16_t value);
static ssize_t read_blvl(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset);
uint8_t bt_bas_get_battery_level(void);
int bt_bas_set_battery_level(uint8_t level);

static const struct bt_data bas_ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
        BT_DATA(BT_DATA_NAME_COMPLETE, BT_DIS_BAS_NAME, 12),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL,
		      BT_UUID_16_ENCODE(BT_UUID_BAS_VAL),
		      BT_UUID_16_ENCODE(BT_UUID_DIS_VAL))
};

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static struct bt_gatt_attr bas_attrs[] = {
        /* Battery Service Declaration */
        BT_GATT_PRIMARY_SERVICE(BT_UUID_BAS),
	BT_GATT_CHARACTERISTIC(BT_UUID_BAS_BATTERY_LEVEL,
			       BT_GATT_CHRC_AUTH | BT_GATT_CHRC_READ | BT_GATT_CHRC_NOTIFY,
			       BT_GATT_PERM_READ_ENCRYPT, read_blvl, NULL,
			       &battery_level),
	BT_GATT_CCC(blvl_ccc_cfg_changed,
		    BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
};

// define bas service
static struct bt_gatt_service bas_svc = BT_GATT_SERVICE(bas_attrs);

/*******************************************************************************
 * Code
 ******************************************************************************/
void bas_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t *) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);
}

void bas_disconnect(void *args)
{
    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

    //restart adv
    bas_adv_start();
}

static void blvl_ccc_cfg_changed(const struct bt_gatt_attr *attr,
				       uint16_t value)
{
	ARG_UNUSED(attr);

	bool notif_enabled = (value == BT_GATT_CCC_NOTIFY);

	BT_INFO("BAS Notifications %s", notif_enabled ? "enabled" : "disabled");

    (void)notif_enabled;
}

static ssize_t read_blvl(struct bt_conn *conn,
			       const struct bt_gatt_attr *attr, void *buf,
			       uint16_t len, uint16_t offset)
{
	uint8_t lvl8 = battery_level;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, &lvl8,
				 sizeof(lvl8));
}

uint8_t bt_bas_get_battery_level(void)
{
	return battery_level;
}

int bt_bas_set_battery_level(uint8_t level)
{
	int rc;

	if (level > 100U) {
		return -EINVAL;
	}

	battery_level = level;

	rc = bt_gatt_notify(NULL, &bas_svc.attrs[1], &level, sizeof(level));

	return rc == -ENOTCONN ? 0 : rc;
}

static void bas_notify(void)
{
	uint8_t battery_level = bt_bas_get_battery_level();

	battery_level--;

	if (!battery_level) {
		battery_level = 100U;
	}

	bt_bas_set_battery_level(battery_level);
}

static void bas_adv_start(void)
{
    int status;

    if(bt_le_adv_start(BT_LE_ADV_CONN, bas_ad, ARRAY_SIZE(bas_ad), NULL, 0) < 0) {
        status = NCP_CMD_RESULT_ERROR;
    }else {
        status = NCP_CMD_RESULT_OK;
    }

    ble_prepare_status(NCP_RSP_BLE_GAP_START_ADV, status, NULL, 0);
}

void init_bas_service(void) 
{
	if (!is_registered)
	{
		// register bas service
   		bt_gatt_service_register(&bas_svc);
		is_registered = true;
	}
}

void peripheral_bas_task(void *pvParameters)
{
    bas_adv_start();
    while(1)
    {
        vTaskDelay(1000);
        /* Battery level simulation */
        bas_notify();
    }
}

#endif /* CONFIG_NCP_BLE */