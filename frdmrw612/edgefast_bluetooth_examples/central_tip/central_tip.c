/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include <fsl_debug_console.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <central_tip.h>
#include <bluetooth/services/tip.h>

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static int scan_start(void);
static void scan_timeout_process(void);
static void scan_timeout_handler(TimerHandle_t timer_id);
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SCAN_PERIOD_MS  30000

static struct bt_conn *default_conn;
static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_subscribe_params subscribe_params;
static TimerHandle_t scan_timer;
static QueueHandle_t scan_timer_sync;

static struct bt_le_scan_param scan_param = {
    .type       = BT_LE_SCAN_TYPE_PASSIVE,
    .options    = BT_LE_SCAN_OPT_NONE,
    .interval   = BT_GAP_SCAN_FAST_INTERVAL,
    .window     = BT_GAP_SCAN_FAST_WINDOW,
};

static char *days_of_week[] =
{
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};

static char *months[] =
{
    "Month Unknown",
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

/*******************************************************************************
 * Code
 ******************************************************************************/
static uint8_t notify_func(struct bt_conn *conn,
               struct bt_gatt_subscribe_params *params,
               const void *data, uint16_t length)
{
    cts_measurement_t current_time;

    if (!data)
    {
        PRINTF("Unsubscribed \n");
        params->value_handle = 0U;
        return BT_GATT_ITER_STOP;
    }

    /* current time display */
    current_time = *(cts_measurement_t *)data;

    cts_date_time_t date_time = current_time.exact_time.cts_day_date_time.date_time;
    cts_day_of_week_tag day = current_time.exact_time.cts_day_date_time.day_of_week;

    PRINTF("%s, %d of %s %d, %d:%d:%d\n", days_of_week[day], date_time.day,
                                        months[date_time.month], date_time.year,
                                        date_time.hours, date_time.minutes,
                                        date_time.seconds);

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

    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_CTS) == 0)
    {
        /* Current Time service discovered.
         * Start Current Time characteristic discovery */
        memcpy(&uuid, BT_UUID_CTS_CURRENT_TIME, sizeof(uuid));
        discover_params.uuid = &uuid.uuid;
        discover_params.start_handle = attr->handle + 1;
        discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &discover_params);
        if (err)
        {
            PRINTF("Discover failed (err %d)\n", err);
        }
    }
    else if (bt_uuid_cmp(discover_params.uuid, BT_UUID_CTS_CURRENT_TIME) == 0)
    {
        /* Current Time characteristic discovered.
         * Start Current Time CCCD discovery*/
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
        /* Subscribe to Current Time service */
        subscribe_params.notify = notify_func;
        subscribe_params.value = BT_GATT_CCC_NOTIFY;
        subscribe_params.ccc_handle = attr->handle;

        err = bt_gatt_subscribe(conn, &subscribe_params);
        if (err && err != -EALREADY)
        {
            PRINTF("Subscribe failed (err %d)\n", err);
        }
        else
        {
            PRINTF("Subscribed to CTS\n");
        }

        return BT_GATT_ITER_STOP;
    }

    return BT_GATT_ITER_STOP;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int32_t err;

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (conn_err)
    {
        PRINTF("Failed to connect to %s (err %u)\n", addr, conn_err);
        bt_conn_unref(default_conn);
        default_conn = NULL;

        /* Restart scanning */
        scan_param.interval = BT_GAP_SCAN_FAST_INTERVAL;
        scan_param.window = BT_GAP_SCAN_FAST_WINDOW;

        /* Reset scan timer */
        xTimerReset(scan_timer, 0);

        scan_start();
        return;
    }

    PRINTF("Connected to peer: %s\n", addr);

    if (conn == default_conn)
    {
        memcpy(&uuid, BT_UUID_CTS, sizeof(uuid));
        discover_params.uuid = &uuid.uuid;
        discover_params.func = discover_func;
        discover_params.start_handle = 0x0001;
        discover_params.end_handle = 0xffff;
        discover_params.type = BT_GATT_DISCOVER_PRIMARY;

        /* Start service discovery for the CTS */
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

static bool device_scanned(struct bt_data *data, void *user_data)
{
    bt_addr_le_t *addr = user_data;
    struct bt_uuid *uuid;
    uint16_t u16;
    int err;
    int i;
    bool continueParse = true;
    char dev[BT_ADDR_LE_STR_LEN];

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

                /* search for the CTS UUID in the advertising data */
                if (bt_uuid_cmp(uuid, BT_UUID_CTS) == 0)
                {
                    /* found the CTS server - stop scanning */
                    xTimerStop(scan_timer, 0);

                    err = bt_le_scan_stop();
                    if (err)
                    {
                        PRINTF("Stop LE scan failed (err %d)\n", err);
                        break;
                    }
                    bt_addr_le_to_str(addr, dev, sizeof(dev));
                    PRINTF("Found device: %s", dev);

                    /* Send connection request */
                    err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                                            BT_LE_CONN_PARAM_DEFAULT,
                                            &default_conn);
                    if (err)
                    {
                        PRINTF("Create connection failed (err %d)\n", err);

                        scan_param.interval = BT_GAP_SCAN_FAST_INTERVAL;
                        scan_param.window = BT_GAP_SCAN_FAST_WINDOW;

                        scan_start();

                        xTimerReset(scan_timer, 0);
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

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    char dev[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(addr, dev, sizeof(dev));
	PRINTF("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n",
	       dev, type, ad->len, rssi);

	/* We're only interested in connectable events */
	if (type == BT_GAP_ADV_TYPE_ADV_IND ||
	    type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
    {
		bt_data_parse(ad, device_scanned, (void *)addr);
	}
}

static int scan_start(void)
{
    return bt_le_scan_start(&scan_param, device_found);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    int32_t err;

    PRINTF("Disconnected reason 0x%02x\n", reason);

    if (default_conn != conn)
    {
        return;
    }

    bt_conn_unref(default_conn);
    default_conn = NULL;

    /* Restart scanning */
    scan_param.interval = BT_GAP_SCAN_FAST_INTERVAL;
    scan_param.window = BT_GAP_SCAN_FAST_WINDOW;

    err = scan_start();
    if (err)
    {
        PRINTF("Scanning failed to start (err %d)\n", err);
    }
    else
    {
        PRINTF("Scanning started\n");
    }

    xTimerReset(scan_timer, 0);
}
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

static void scan_timeout_handler(TimerHandle_t timer_id)
{
    if (NULL != scan_timer_sync)
    {
        /* it is not in isr context */
        xSemaphoreGive(scan_timer_sync);
    }
    else
    {
        scan_timeout_process();
    }
}

static void scan_timeout_process()
{
    int err;

    /* Stop scan timer */
    xTimerStop(scan_timer, 0);

    /* Stop scanning */
    err = bt_le_scan_stop();
    if (err)
    {
        PRINTF("Stop LE scan failed (err %d)\n", err);
    }
    else
    {
        /* Restart scanning in reduced power mode */
        scan_param.interval = BT_GAP_SCAN_SLOW_INTERVAL_1;
        scan_param.window = BT_GAP_SCAN_SLOW_WINDOW_1;

        err = scan_start();
        if (err)
        {
            PRINTF("Scanning failed to start (err %d)\n", err);
        }
    }
}

static void bt_ready(int err)
{
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    if (IS_ENABLED(CONFIG_BT_SETTINGS))
    {
        settings_load();
    }
    PRINTF("Bluetooth initialized\n");

    /* Register connection callback */
    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

    /* Create and start scanning timer */
    scan_timer = xTimerCreate("central tip scan timer", (SCAN_PERIOD_MS / portTICK_PERIOD_MS),
                              pdTRUE, NULL, scan_timeout_handler);

    /* Start scanning */
    err = scan_start();
    if (err)
    {
        PRINTF("Scanning failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Scanning started\n");

    /* Start scan timer */
    xTimerStart(scan_timer, 0);
}

void central_tip_task(void *pvParameters)
{
    int err;


    scan_timer_sync = xSemaphoreCreateCounting(0xFFu, 0u);
    if (NULL == scan_timer_sync)
    {
        PRINTF("faile to create scan_timer_sync\n");
    }

    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
    }

    if (NULL != scan_timer_sync)
    {
        while(1)
        {
            if (pdTRUE == xSemaphoreTake(scan_timer_sync, portMAX_DELAY))
            {
                scan_timeout_process();
            }
        }
    }
    else
    {
        while (1)
        {
            vTaskDelay(1000);
        }
    }
}

