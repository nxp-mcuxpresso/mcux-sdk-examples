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
#include <bluetooth/l2cap.h>
#include "bt_pal_hci_core.h"

#include "fsl_debug_console.h"

#include "service.h"
#include "ncp_glue_ble.h"
/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;

static struct bt_conn *default_conn = NULL;

#define HRC_MAX_NOTIF_DATA                  (MIN(BT_L2CAP_RX_MTU, BT_L2CAP_TX_MTU) - 3)
static uint8_t hrc_ev_buf[sizeof(gatt_notification_ev_t) + HRC_MAX_NOTIF_DATA];
extern uint8_t host_svc;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

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
    struct gatt_notification_ev *ev = (void *) hrc_ev_buf;
    const bt_addr_le_t *addr;

    if (!conn || !data)
    {
        PRINTF("Unsubscribed");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    addr = bt_conn_get_dst(conn);
    ev->type = (uint8_t)params->value;
    ev->handle = sys_cpu_to_le16(params->value_handle);

    length = MIN(length, HRC_MAX_NOTIF_DATA);

    ev->data_length = sys_cpu_to_le16(length);
    memcpy(ev->data, data, length);
    if (addr != NULL)
    {
        memcpy(ev->address, addr->a.val, sizeof(ev->address));
        ev->address_type = addr->type;
    }
    
    ev->svc_id = CENTRAL_HRC_SERVICE_ID;

    ble_prepare_status(NCP_EVENT_GATT_NOTIFICATION, NCP_CMD_RESULT_OK, hrc_ev_buf, sizeof(*ev) + length);

    return BT_GATT_ITER_CONTINUE;
}

static uint8_t discover_func(struct bt_conn *conn,
			     const struct bt_gatt_attr *attr,
			     struct bt_gatt_discover_params *params)
{
	int err;

	if (!attr) {
		PRINTF("Discover complete, No attribute found \n");
		(void)memset(params, 0, sizeof(*params));
		return BT_GATT_ITER_STOP;
	}

	PRINTF("[ATTRIBUTE] handle %u\n", attr->handle);

	if (!bt_uuid_cmp(discover_params.uuid, BT_UUID_HRS)) {
        if (host_svc) {
            gatt_disc_prim_rp_t *ev = (void *) hrc_ev_buf;
            gatt_service_t service;
            struct bt_gatt_service_val *data;
            data = attr->user_data;
            service.uuid_length = 2;
            service.start_handle = attr->handle;
            service.end_handle = data->end_handle;
            service.uuid[0] = ((BT_UUID_HRS_VAL) >>  0) & 0xFF;
            service.uuid[1] = ((BT_UUID_HRS_VAL) >>  8) & 0xFF;
            ev->services_count = 1;
            memcpy(ev->services, &service, sizeof(gatt_service_t));
            ble_prepare_status(NCP_EVENT_GATT_DISC_PRIM, NCP_CMD_RESULT_OK, (uint8_t*)ev, sizeof(gatt_service_t)+1);
        }
		memcpy(&uuid, BT_UUID_HRS_MEASUREMENT, sizeof(uuid));
		discover_params.uuid = &uuid.uuid;
		discover_params.start_handle = attr->handle + 1;
		discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			PRINTF("Discover failed (err %d)\n", err);
		}
	} else if (!bt_uuid_cmp(discover_params.uuid,
				BT_UUID_HRS_MEASUREMENT)) {
        if (host_svc) {
            gatt_disc_chrc_rp_t *ev = (void *) hrc_ev_buf;
            gatt_characteristic_t characteristics;
            struct bt_gatt_chrc *data = attr->user_data;

            characteristics.uuid_length = 2;
            characteristics.characteristic_handle = attr->handle;
            characteristics.value_handle = attr->handle + 1;
            characteristics.properties = data->properties;
            characteristics.uuid[0] = ((BT_UUID_HRS_MEASUREMENT_VAL) >>  0) & 0xFF;
            characteristics.uuid[1] = ((BT_UUID_HRS_MEASUREMENT_VAL) >>  8) & 0xFF;
            ev->characteristics_count = 1;
            memcpy(ev->characteristics, &characteristics, sizeof(gatt_characteristic_t));
            ble_prepare_status(NCP_EVENT_GATT_DISC_CHRC, NCP_CMD_RESULT_OK, (uint8_t*)ev, sizeof(gatt_characteristic_t)+1);
        }
		memcpy(&uuid, BT_UUID_GATT_CCC, sizeof(uuid));
		discover_params.uuid = &uuid.uuid;
		discover_params.start_handle = attr->handle + 2;
		discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
		subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

		err = bt_gatt_discover(conn, &discover_params);
		if (err) {
			PRINTF("Discover failed (err %d)\n", err);
		}
	} else {
        if (host_svc) {
            gatt_disc_all_desc_rp_t *ev = (void *) hrc_ev_buf;
            gatt_descriptor_t descriptor;
            descriptor.uuid_length = 2;
            descriptor.descriptor_handle = attr->handle;
            descriptor.uuid[0] = ((BT_UUID_GATT_CCC_VAL) >>  0) & 0xFF;
            descriptor.uuid[1] = ((BT_UUID_GATT_CCC_VAL) >>  8) & 0xFF;
            memcpy(ev->descriptors, &descriptor, sizeof(gatt_descriptor_t));
            ble_prepare_status(NCP_EVENT_GATT_DISC_DESC, NCP_CMD_RESULT_OK, (uint8_t*)ev, sizeof(gatt_descriptor_t)+1);
        }
        else {
            subscribe_params.notify = notify_func;
            subscribe_params.value = BT_GATT_CCC_NOTIFY;
            subscribe_params.ccc_handle = attr->handle;

            err = bt_gatt_subscribe(conn, &subscribe_params);
            // send subscription event
            gatt_ncp_ble_svc_subscription_ev_t ev;
            
            ev.svc_id = CENTRAL_HRC_SERVICE_ID;
            ev.status = (err && err != -EALREADY) ? NCP_CMD_RESULT_ERROR : NCP_CMD_RESULT_OK;

            ble_prepare_status(NCP_EVENT_GATT_SUBSCRIPTIONED, NCP_CMD_RESULT_OK, (uint8_t *) &ev, sizeof(gatt_ncp_ble_svc_subscription_ev_t));
        }

		return BT_GATT_ITER_STOP;
	}

	return BT_GATT_ITER_STOP;
}

bool hrc_adv_report_processed(struct adv_report_data *data, void *user_addr)
{
    bt_addr_le_t *addr = user_addr;
    struct bt_uuid *uuid;
    uint16_t u16;
    int err;
    int i;
    bool continueParse = true;

    /* return true to continue parsing or false to stop parsing */
    switch (data->type)
    {
        case BT_DATA_UUID16_SOME:
        case BT_DATA_UUID16_ALL:
        {
            if (data->data_len % sizeof(uint16_t) != 0U)
            {
                PRINTF("AD malformed\n");
                return true;
            }
            
            for (i = 0; i < data->data_len; i += sizeof(uint16_t))
            {
                memcpy(&u16, &data->data[i], sizeof(u16));
                uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));

                /* search for the HTS UUID in the advertising data */
                if (bt_uuid_cmp(uuid, BT_UUID_HRS) == 0)
                {
                    /* found the temperature server - stop scanning */
                    err = bt_le_scan_stop();
                    if (err)
                    {
                        ble_prepare_status(NCP_RSP_BLE_GAP_STOP_SCAN, NCP_CMD_RESULT_ERROR, NULL, 0);
                        break;
                    }
                    ble_prepare_status(NCP_RSP_BLE_GAP_STOP_SCAN, NCP_CMD_RESULT_OK, NULL, 0);
                                    
                    /* Send connection request */
                    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                            BT_LE_CONN_PARAM_DEFAULT,
                                            &default_conn);
                    if (err)
                    {
                        PRINTF("Create connection failed (err %d)\n", err);
                        svc_scan_start();
                    }

                    continueParse = false;
                    break;
                }
            }
            break;
        }

        default:
        {
            break;
        }
    }

    return continueParse;
}

void central_hrc_connect(void *args)
{
    service_connect_param_t *param = (service_connect_param_t*) args;
    struct bt_conn *conn = param->conn;
    default_conn = bt_conn_ref(conn);
    memcpy(&uuid, BT_UUID_HRS, sizeof(uuid));
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

void central_hrc_disconnect(void *args)
{
	if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
    
    svc_scan_start();   
}

void central_hrc_task(void *pvParameters)
{
    // only start scan when scan is not enabled
    if (!atomic_test_bit(bt_dev.flags, BT_DEV_SCANNING)) {
        svc_scan_start();
    }
    
    while(1)
    {
        vTaskDelay(1000);
    }   
}

#endif /* CONFIG_NCP_BLE */
