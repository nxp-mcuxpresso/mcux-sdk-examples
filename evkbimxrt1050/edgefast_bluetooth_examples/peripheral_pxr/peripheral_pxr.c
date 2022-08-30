/*
 * Copyright (c) 2015-2016 Intel Corporation
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <toolchain.h>
#include <porting.h>
#include "fsl_debug_console.h"

#include <bluetooth/services/pxr.h>
#include <bluetooth/gatt.h>
#include <bluetooth/conn.h>

struct bt_conn *default_conn;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
              BT_UUID_16_ENCODE(BT_UUID_LLS_VAL),
              BT_UUID_16_ENCODE(BT_UUID_IAS_VAL),
              BT_UUID_16_ENCODE(BT_UUID_TPS_VAL))
};

void alert_ui(uint8_t alert_level);

static void connected(struct bt_conn *conn, uint8_t err)
{
    char addr[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err)
    {
        PRINTF("Failed to connect to %s (err %u)\n", addr, err);
    }
    else
    {
        //bt_conn_le_tx_power tx_pwr_lvl;
        uint8_t alert_level = NO_ALERT;

        default_conn = bt_conn_ref(conn);
        PRINTF("Connected to peer: %s\n", addr);

        //bt_conn_le_get_tx_power_level(conn, &tx_pwr_lvl);
        //pxr_tps_set_power_level(tx_pwr_lvl.current_level);
        pxr_tps_set_power_level(0);

        write_lls_alert_level(NULL, NULL, &alert_level, sizeof(alert_level), 0, 0);
        write_ias_alert_level(NULL, NULL, &alert_level, sizeof(alert_level), 0, 0);

        alert_ui(alert_level);
#if CONFIG_BT_SMP
        if (bt_conn_set_security(conn, BT_SECURITY_L2))
        {
            PRINTF("Failed to set security\n");
        }
#endif
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);

    if (default_conn) {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }

    if ((reason == 0x08) || (reason == 0x22))  /* Connection timeout */
    {
        PRINTF("Link Loss Alert Triggered...\r\n");
        alert_ui(pxr_lls_get_alert_level());
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

void alert_ui(uint8_t alert_level)
{
    switch(alert_level)
    {
        case NO_ALERT:
            PRINTF("\r\nALERT: OFF\r\n");
            break;

        case MILD_ALERT:
            PRINTF("\r\nALERT: MILD\r\n");
            break;

        case HIGH_ALERT:
            PRINTF("\r\nALERT: HIGH\r\n");
            break;

        default:
            break;
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

    pxr_init(alert_ui);

    bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

    err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err)
    {
        PRINTF("Advertising failed to start (err %d)\n", err);
        return;
    }

    PRINTF("Advertising successfully started\n");
}

void peripheral_pxr_task(void *pvParameters)
{
    int err;

    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
    }

    while(1)
    {
        vTaskDelay(1000);
    }
}
