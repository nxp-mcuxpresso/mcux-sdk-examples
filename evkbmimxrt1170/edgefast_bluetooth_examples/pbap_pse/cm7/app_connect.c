/*
 * Copyright 2020, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <porting.h>
#include <string.h>
#include <errno/errno.h>
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app_pbap_pse.h"
#include "bt_pal_conn_internal.h"
#include "bt_pal_keys.h"
#include "bt_pal_pbap_internal.h"

static struct bt_conn *default_conn;
static bt_addr_t default_peer_addr;

extern app_pbap_pse_t g_PbapPse;

void app_connect(uint8_t *addr)
{
    memcpy(&default_peer_addr, addr, 6U);
    default_conn = bt_conn_create_br(&default_peer_addr, BT_BR_CONN_PARAM_DEFAULT);
    if (!default_conn)
    {
        PRINTF("Connection failed\r\n");
    }
    else
    {
        /* unref connection obj in advance as app user */
        bt_conn_unref(default_conn);
        PRINTF("Connection pending\r\n");
    }
}

void app_disconnect(void)
{
    if (bt_conn_disconnect(default_conn, 0x13U))
    {
        PRINTF("Disconnection failed\r\n");
    }
}

static void bt_connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        if (default_conn != NULL)
        {
            default_conn = NULL;
        }
        PRINTF("Connection failed (err 0x%02x)\n", err);
    }
    else
    {
        PRINTF("bt_connected\n");
        g_PbapPse.conn = conn;
    }
    return;
}

static void bt_disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);
    if (default_conn != conn)
    {
        return;
    }

    if (default_conn)
    {
        bt_conn_unref(default_conn);
        default_conn = NULL;
    }
    else
    {
        return;
    }
}

static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_to_str(bt_conn_get_dst_br(conn), addr, sizeof(addr));

    if (!err)
    {
        PRINTF("Security changed: %s level %u\n", addr, level);
    }
    else
    {
        PRINTF("Security failed: %s level %u err %d\n", addr, level, err);
        if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING)
        {
            bt_addr_le_t addr;
            struct bt_conn_info info;
            int ret;

            bt_conn_get_info(conn, &info);
            if (info.type == BT_CONN_TYPE_LE)
            {
                return;
            }

            PRINTF("The peer device seems to have lost the bonding information.\n");
            PRINTF("Delete the bonding information of the peer, please try again.\n");
            addr.type = BT_ADDR_LE_PUBLIC;
            addr.a = *info.br.dst;
            ret = bt_unpair(BT_ID_DEFAULT, &addr);
            if (ret)
            {
                PRINTF("fail to delete.\n");
            }
            else
            {
                PRINTF("success to delete.\n");
            }
        }
    }
}

static struct bt_conn_cb conn_callbacks = {
    .connected        = bt_connected,
    .disconnected     = bt_disconnected,
    .security_changed = security_changed,
};

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);
}

static void passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    PRINTF("Passkey %06u\n", passkey);
}

#if 0
static void passkey_confirm(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Confirm passkey for %s: %06u", addr, passkey);
    s_passkeyConfirm = 1;
}
#endif
static struct bt_conn_auth_cb auth_cb_display = {
    .cancel = auth_cancel, .passkey_display = passkey_display, /* Passkey display callback */

};

void app_connect_init(void)
{
    bt_conn_cb_register(&conn_callbacks);
    bt_conn_auth_cb_register(&auth_cb_display);
}
