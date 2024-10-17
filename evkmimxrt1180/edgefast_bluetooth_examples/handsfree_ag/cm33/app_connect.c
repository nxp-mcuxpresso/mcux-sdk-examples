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
#include <bluetooth/l2cap.h>
#include <bluetooth/hfp_ag.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app_handsfree_ag.h"

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
extern struct bt_conn *default_conn;
bt_addr_t default_peer_addr;
static uint8_t default_connect_initialized;

static struct bt_conn_cb conn_callbacks = {
    .connected        = connected,
    .disconnected     = disconnected,
    .security_changed = security_changed,
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    int res;
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
        default_conn = conn;
        if (1U == default_connect_initialized)
        {
            struct bt_conn_info info;

            default_connect_initialized = 0U;
            bt_conn_get_info(conn, &info);
            if (info.type == BT_CONN_TYPE_LE)
            {
                return;
            }

            /*
             * Do an SDP Query on Successful ACL connection complete with the
             * required device
             */
            if (0 == memcmp(info.br.dst, &default_peer_addr, 6U))
            {
                res = bt_hfp_ag_discover(default_conn, &app_hfp_ag_discover);
                if (res)
                {
                    PRINTF("SDP discovery failed: result\r\n");
                }
                else
                {
                    PRINTF("SDP discovery started\r\n");
                }
            }
            PRINTF("Connected\n");
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);

    if (default_conn != conn)
    {
        return;
    }

    if (default_conn)
    {
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

void app_connect(uint8_t *addr)
{
    default_connect_initialized = 1U;
    memcpy(&default_peer_addr, addr, 6U);
    default_conn = bt_conn_create_br(&default_peer_addr, BT_BR_CONN_PARAM_DEFAULT);
    if (!default_conn)
    {
        default_connect_initialized = 0U;
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

void app_delete(void)
{
}

void app_connect_init(void)
{
    bt_conn_cb_register(&conn_callbacks);
}
