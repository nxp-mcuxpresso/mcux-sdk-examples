/*
 * Copyright 2021 NXP
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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

#include "fsl_loader_utils.h"
#include "fsl_debug_console.h"
#include "host_msd_fatfs.h"

#include "service/hts.h"
#include "service.h"
#include "ncp_glue_ble.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;

static struct bt_conn *default_conn = NULL;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
// extern void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t evtype,
//                          struct net_buf_simple *ad);

static uint8_t notify_func(struct bt_conn *conn,
			   struct bt_gatt_subscribe_params *params,
			   const void *data, uint16_t length);

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params);

extern void ncp_notify_attribute(const void *data, uint16_t len);
/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t notify_func(struct bt_conn *conn,
               struct bt_gatt_subscribe_params *params,
               const void *data, uint16_t length)
{
    struct temp_measurement temp_measurement;
    uint32_t temperature;

    if (!data)
    {
        PRINTF("Unsubscribed \n");
        params->value_handle = 0U;
        return BT_GATT_ITER_STOP;
    }

    /* temperature value display */
    temp_measurement = *(struct temp_measurement*)data;
    temperature = sys_get_le32(temp_measurement.temperature);

    if ((temp_measurement.flags & BIT0) == hts_unit_celsius_c)
    {
        PRINTF("Temperature %d degrees Celsius \n", temperature);
    }
    else
    {
        PRINTF("Temperature %d degrees Fahrenheit \n", temperature);
    }

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
                 const struct bt_gatt_attr *attr,
                 struct bt_gatt_discover_params *params)
{
    int32_t err;

    if (!attr)
    {
        PRINTF("Discover complete, No attribute found \n");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    PRINTF("handle 0x%04x uuid %d properties 0x%02x", attr->handle,
		        attr->uuid->type, attr->user_data);
    
    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_HTS) == 0)
    {
        /* Health Thermometer service discovered */
        memcpy(&uuid, BT_UUID_HTS_MEASUREMENT, sizeof(uuid));
        discover_params.uuid = &uuid.uuid;
        discover_params.start_handle = attr->handle + 1;
        discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &discover_params);
        if (err)
        {
            PRINTF("Discover failed (err %d)\n", err);
        }
    }
    else if (bt_uuid_cmp(discover_params.uuid, BT_UUID_HTS_MEASUREMENT) == 0)
    {
        /* Health Thermometer Measurement characteristic discovered */
        memcpy(&uuid, BT_UUID_GATT_CCC, sizeof(uuid));
        discover_params.uuid = &uuid.uuid;
        discover_params.start_handle = attr->handle + 2;
        discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
        subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

        err = bt_gatt_discover(conn, &discover_params);
        if (err)
        {
            PRINTF("Discover failed (err %d)\n", err);
        }
    }
    else
    {
        /* Subscribe to Health Thermometer service */
        subscribe_params.notify = notify_func;
        subscribe_params.value = BT_GATT_CCC_INDICATE;
        subscribe_params.ccc_handle = attr->handle;

        err = bt_gatt_subscribe(conn, &subscribe_params);
        if (err && err != -EALREADY)
        {
            PRINTF("Subscribe failed (err %d)\n", err);
        }
        else
        {
            PRINTF("Subscribed to HTS\n");
        }

        return BT_GATT_ITER_STOP;
    }

    return BT_GATT_ITER_STOP;
}

void central_htc_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t*) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);

    memcpy(&uuid, BT_UUID_HTS, sizeof(uuid));
    discover_params.uuid = &uuid.uuid;
    discover_params.func = discover_func;
    discover_params.start_handle = 0x0001;
    discover_params.end_handle = 0xffff;
    discover_params.type = BT_GATT_DISCOVER_PRIMARY;
    
    int err = bt_gatt_discover(default_conn, &discover_params);
    if (err) {
        PRINTF("Discover failed(err %d)\n", err);
        return;
    }
}

void central_htc_disconnect(void *args)
{
	if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

	// bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
}

