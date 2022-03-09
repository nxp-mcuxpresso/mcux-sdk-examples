/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <bluetooth/services/ipsp.h>

#include "fsl_debug_console.h"

#define MSG_LEN             5

NET_BUF_POOL_DEFINE(buf_pool, 1, MSG_LEN, USER_DATA_MIN, NULL);
uint8_t message[MSG_LEN] = {'h','e','l','l','o'};

static void bt_ready(int err);
static int scan_start(void);
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad);
static bool device_scanned(struct bt_data *data, void *user_data);
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static uint8_t discover_func(struct bt_conn *conn,
                 const struct bt_gatt_attr *attr,
                 struct bt_gatt_discover_params *params);
static int ipsp_rx_cb(struct net_buf *buf);

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Security changed: %s level %u (error %d)\n", addr, level, err);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);
}
#endif
struct bt_conn *default_conn = NULL;
static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

#if CONFIG_BT_SMP
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};
#endif
static struct bt_gatt_discover_params discover_params;
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);


/* 6LowPan entry point */
static int ipsp_rx_cb(struct net_buf *buf)
{
    int i;

    if (buf != NULL)
    {
        PRINTF("Received message: ");
        for(i = 0; i < buf->len; i++)
        {
            PRINTF("%c", buf->data[i]);
        }
        PRINTF("\r\n");
    }

    return 0;
}

static int scan_start(void)
{
    struct bt_le_scan_param scan_param = {
            .type       = BT_LE_SCAN_TYPE_PASSIVE,
            .options    = BT_LE_SCAN_OPT_NONE,
            .interval   = BT_GAP_SCAN_FAST_INTERVAL,
            .window     = BT_GAP_SCAN_FAST_WINDOW,
    };

    return bt_le_scan_start(&scan_param, device_found);
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    char dev[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, dev, sizeof(dev));
    PRINTF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
           dev, type, ad->len, rssi);

    /* Check for connectable advertising */
    if ((type ==  BT_GAP_ADV_TYPE_ADV_IND) || (type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND))
    {
        bt_data_parse(ad, device_scanned, (void *)addr);
    }
}

static bool device_scanned(struct bt_data *data, void *user_data)
{
    bt_addr_le_t *addr = user_data;
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
                if (bt_uuid_cmp(uuid, BT_UUID_IPSS) == 0)
                {
                    /* found one IPSP node - stop scanning */
                    err = bt_le_scan_stop();
                    if (err)
                    {
                        PRINTF("Stop LE scan failed (err %d)\n", err);
                        break;
                    }
                    PRINTF("Found device: %s", addr);

                    /* Send connection request */
                    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                            BT_LE_CONN_PARAM_DEFAULT,
                                            &default_conn);
                    if (err)
                    {
                        PRINTF("Create connection failed (err %d)\n", err);
                        scan_start();
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

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		PRINTF("Connection failed (err 0x%02x)\n", err);
        if (NULL != default_conn)
        {
            (void)bt_conn_unref(default_conn);
            default_conn = NULL;
        }
	} else {
		PRINTF("Connected\n");
        if (conn == default_conn)
        {
        	memcpy(&uuid, BT_UUID_IPSS, sizeof(uuid));
            discover_params.uuid = &uuid.uuid;
            discover_params.func = discover_func;
            discover_params.start_handle = 0x0001;
            discover_params.end_handle = 0xffff;
            discover_params.type = BT_GATT_DISCOVER_PRIMARY;

            /* Start service discovery */
            err = bt_gatt_discover(default_conn, &discover_params);
            if (err)
            {
                PRINTF("Discover failed(err %d)\n", err);
            }
            else
            {
                PRINTF("Starting service discovery \n");
            }
        }
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    int err;
	PRINTF("Disconnected (reason 0x%02x)\n", reason);

	if (default_conn) {
		bt_conn_unref(default_conn);
		default_conn = NULL;
	}

	/* Restart scanning */
	err = scan_start();
	if (err)
	{
	    PRINTF("Scanning failed to start (err %d)\n", err);
	    return;
	}

	PRINTF("Scanning started\n");
}

static uint8_t discover_func(struct bt_conn *conn,
                 const struct bt_gatt_attr *attr,
                 struct bt_gatt_discover_params *params)
{
    int err;

    if (!attr)
    {
        PRINTF("Discover complete, No attribute found \n");
        (void)memset(params, 0, sizeof(*params));
        return BT_GATT_ITER_STOP;
    }

    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_IPSS) == 0)
    {
        err = ipsp_connect(conn);
        if(err)
        {
            PRINTF("IPSP connect failed (err %d)\r\n", err);
        }
    }


    return BT_GATT_ITER_STOP;
}

static void bt_ready(int err)
{
	if (err) {
		PRINTF("Bluetooth init failed (err %d)\n", err);
		return;
	}

	if (IS_ENABLED(CONFIG_BT_SETTINGS)) 
    {
        settings_load();
    }
    PRINTF("Bluetooth initialized\n");

    /* Register connection callbacks */
	bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

	/* Start scanning */
    err = scan_start();
    if (err)
    {
        PRINTF("Scanning failed to start (err %d)\n", err);
        return;
    }
    PRINTF("Scanning started\n");

    /* Initialize IPSP Router */
	ipsp_init(ipsp_rx_cb);
}


void central_ipsp_task(void *pvParameters)
{
    int err;
    struct net_buf *buf;

    err = bt_enable(bt_ready);
	if (err) {
		PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
	}

    while (1)
    {
        vTaskDelay(5000);
        
        buf = net_buf_alloc(&buf_pool, osaWaitNone_c);
        if (NULL != buf)
        {
            net_buf_add_mem(buf, message, MSG_LEN);
            err = ipsp_send(buf);
            if (err)
            {
                net_buf_unref(buf);
                if (-ENOTCONN == err)
                {
                    /*Socket is not connected*/
                }
                else
                {
                    PRINTF("IPSP send failed (err %d)\r\n", err);
                }
                
            }
            else
            {
                PRINTF("Sending message...\r\n");
            }
        }

    }
}

