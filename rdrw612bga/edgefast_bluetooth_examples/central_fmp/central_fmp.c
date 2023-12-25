/*
 * Copyright (c) 2020 SixOctets Systems
 * Copyright 2021-2022 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include "fsl_debug_console.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <central_fmp.h>
#include <bluetooth/services/fmp.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define SCAN_PERIOD_MS            30000

/* Number of alerts after which to disconnect from the target */
#define TARGET_ALERT_THRESHOLD    5U

/* Remote User Terminated Connection */
#define DISCONNECT_REASON         0x13U
#define APP_DELAY             ( TickType_t ) 0x1388UL
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static int scan_start(void);
static void scan_timeout_handler(TimerHandle_t timer_id);
static void scan_timeout_process(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
static struct bt_conn *default_conn;
static uint16_t default_conn_handle;
static QueueHandle_t scan_timeout_sync;

static struct bt_uuid_16 uuid = BT_UUID_INIT_16(0);
static struct bt_gatt_discover_params discover_params;
static struct bt_gatt_write_params write_params;
static uint8_t write_buf = 0;

static uint16_t ias_value_handle = 0xFFFFU;
static bool ias_alert_triggered = false;
static uint8_t alert_count = 0U;
static TimerHandle_t scan_timer = NULL;

/*******************************************************************************
 * APIs
 ******************************************************************************/
static void write_func(struct bt_conn *conn, uint8_t err,
				 struct bt_gatt_write_params *params)
{
    if (err == 0)
    {
        PRINTF("GATT Write successful\n");
    }
    else
    {
        PRINTF("GATT Write failed (err = %d)", err);
    }
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

    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_IAS) == 0)
    {
        /* Immediate Alert Service discovered */
        /* Next, discover Immediate Alert Level characteristic */
        memcpy(&uuid, BT_UUID_ALERT_LEVEL, sizeof(uuid));
        discover_params.uuid = &uuid.uuid;
        discover_params.start_handle = attr->handle + 1;
        discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, &discover_params);
        if (err)
        {
            PRINTF("Discover failed (err %d)\n", err);
        }
    }
    if (bt_uuid_cmp(discover_params.uuid, BT_UUID_ALERT_LEVEL) == 0)
    {
        ias_value_handle = bt_gatt_attr_value_handle(attr);
    }
    else
    {
        /* Nothing else to discover */
    }

    return BT_GATT_ITER_STOP;
}

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    int32_t err;

    xTimerStop(scan_timer, 0);
    xTimerDelete(scan_timer, 0);
    scan_timer = NULL;
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (conn_err)
    {
        PRINTF("Failed to connect to %s (err %u)\n", addr, conn_err);
        bt_conn_unref(default_conn);
        default_conn = NULL;

        /* Restart scanning */
        scan_start();
        return;
    }

    PRINTF("Connected to peer: %s\n", addr);

    if (conn == default_conn)
    {
        memcpy(&uuid, BT_UUID_IAS, sizeof(uuid));
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

static bool device_scanned(struct bt_data *data, void *user_data)
{
    bt_addr_le_t *addr = user_data;
    struct bt_uuid *uuid;
    uint16_t u16;
    int err;
    int i;
    char dev[BT_ADDR_LE_STR_LEN];
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

                /* search for the IAS UUID in the advertising data */
                if (bt_uuid_cmp(uuid, BT_UUID_IAS) == 0)
                {
                    /* found the Intermediate Alert Service - stop scanning */
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

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
             struct net_buf_simple *ad)
{
    char dev[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(addr, dev, sizeof(dev));

    /* We're only interested in connectable events */
    if (type == BT_GAP_ADV_TYPE_ADV_IND ||
        type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
    {
        bt_data_parse(ad, device_scanned, (void *)addr);
    }
}

static int scan_start(void)
{
    struct bt_le_scan_param scan_param = {
            .type       = BT_LE_SCAN_TYPE_PASSIVE,
            .options    = BT_LE_SCAN_OPT_NONE,
            .interval   = BT_GAP_SCAN_FAST_INTERVAL,
            .window     = BT_GAP_SCAN_FAST_WINDOW,
    };

    /* Start timer to change parameters after 30s */
    if (scan_timer == NULL)
    {
        scan_timer = xTimerCreate("scan timer", (SCAN_PERIOD_MS / portTICK_PERIOD_MS),
                                      pdFALSE, NULL, scan_timeout_handler);
    }
    xTimerStart(scan_timer, 0);

    return bt_le_scan_start(&scan_param, device_found);
}

static void scan_timeout_handler(TimerHandle_t timer_id)
{
    if (NULL != scan_timeout_sync)
    {
        /* it is not in isr context */
        xSemaphoreGive(scan_timeout_sync);
    }
    else
    {
        scan_timeout_process();
    }
}

static void scan_timeout_process(void)
{
    /* Change scan interval and scan window for lower power consumption */
    struct bt_le_scan_param scan_param = {
            .type       = BT_LE_SCAN_TYPE_PASSIVE,
            .options    = BT_LE_SCAN_OPT_NONE,
            .interval   = BT_GAP_SCAN_SLOW_INTERVAL_1,
            .window     = BT_GAP_SCAN_SLOW_WINDOW_1,
    };

    bt_le_scan_start(&scan_param, device_found);
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
    alert_count = 0U;

    /* Restart scanning */
    ias_value_handle = 0xFFFFU;
    err = scan_start();
    if (err)
    {
        PRINTF("Scanning failed to start (err %d)\n", err);
    }
    else
    {
        PRINTF("Scanning started\n");
    }
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

int alert_target(void)
{
    int err = 0;
    static uint8_t alert = MILD_ALERT;

    if (default_conn != NULL)
    {
        err = bt_hci_get_conn_handle(default_conn, &default_conn_handle);
        if (err)
        {
            PRINTF("Failed to get connection handle\n");
            return err;
        }
        
        if (alert_count == TARGET_ALERT_THRESHOLD)
        {
            bt_conn_disconnect(default_conn, DISCONNECT_REASON);
            alert_count = 0U;
            return err;
        }

        if ((ias_value_handle != 0xFFFFU) && (ias_value_handle != 0x0000))
        {
            if (!ias_alert_triggered)
            {
                /* Write Alert Level */
                write_params.func = write_func;
                write_params.handle = ias_value_handle;
                write_params.offset = 0;
                write_buf = alert;
                write_params.data = &write_buf;
                write_params.length = 1;
                err = bt_gatt_write(default_conn, &write_params);

                if (err)
                {
                    PRINTF("GATT Write failed (err %d)\n", err);
                }
                else
                {
                    ias_alert_triggered = true;
                }

                if (alert == MILD_ALERT)
                {
                    alert = HIGH_ALERT;
                }
                else
                {
                    alert = MILD_ALERT;
                }

            }
            else
            {
                /* Write Alert Level */
                write_params.func = write_func;
                write_params.handle = ias_value_handle;
                write_params.offset = 0;
                write_buf = NO_ALERT;
                write_params.data = &write_buf;
                write_params.length = 1;
                err = bt_gatt_write(default_conn, &write_params);

                if (err)
                {
                    PRINTF("GATT Write failed (err %d)\n", err);
                }
                else
                {
                    ias_alert_triggered = false;
                }
            }
            alert_count++;
        }
    }

    return err;
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

    /* Start scanning */
    err = scan_start();
    if (err)
    {
        PRINTF("Scanning failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Scanning started\n");
}

void central_fmp_task(void *pvParameters)
{
    int err;

    scan_timeout_sync = xSemaphoreCreateCounting(0xFFu, 0u);
    if (NULL == scan_timeout_sync)
    {
        PRINTF("faile to create scan_timeout_sync\n");
    }

    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
    }

    if (NULL != scan_timeout_sync)
    {   
        /* Toggle Allert on Target */
        while(1)
        {
            if (pdTRUE == xSemaphoreTake(scan_timeout_sync, APP_DELAY))
            {
                scan_timeout_process();
            }
            err = alert_target();
            if (err)
            {
                PRINTF("Failed to trigger alert on target.\n");
            }
        }
    }
    else
    {
        /* Toggle Allert on Target */
        while (1) {
        vTaskDelay(5000);
        err = alert_target();
        if (err)
        {
            PRINTF("Failed to trigger alert on target.\n");
        }
        }
    }
}

