/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)

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
#include <bluetooth/a2dp.h>
#include <bluetooth/a2dp_codec_sbc.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

extern bt_addr_t a2dp_bdaddr;

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void security_changed(struct bt_conn *conn, bt_security_t level,
				 enum bt_security_err err);

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed,
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    struct bt_conn_info info;

    bt_conn_get_info(conn, &info);

    if(info.type == BT_CONN_TYPE_BR)
    {
        if (err) {
            PRINTF("BR Connection failed (err 0x%02x)\n", err);
        } else {
            PRINTF("BR Connected\n");
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct bt_conn_info info;

    bt_conn_get_info(conn, &info);

    if(info.type == BT_CONN_TYPE_BR)
    {
        PRINTF("BR Disconnected (reason 0x%02x)\n", reason);
    }
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
				 enum bt_security_err err)
{
    struct bt_conn_info info;

    char addr[BT_ADDR_LE_STR_LEN];

    bt_conn_get_info(conn, &info);

    if(info.type == BT_CONN_TYPE_BR)
    {
        bt_addr_to_str(bt_conn_get_dst_br(conn), addr, sizeof(addr));

        /* Save bd addr for switch role. */
        memcpy(&a2dp_bdaddr, bt_conn_get_dst_br(conn), sizeof(a2dp_bdaddr));

        if (!err)
        {
            PRINTF("BR Security changed: %s level %u\n", addr, level);
        }
        else
        {
            PRINTF("BR Security failed: %s level %u err %d\n", addr, level, err);
            if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING)
            {
                struct bt_conn_info info;
                int ret;

                bt_conn_get_info(conn, &info);
                if (info.type == BT_CONN_TYPE_LE)
                {
                    return;
                }

                PRINTF("The peer device seems to have lost the bonding information.\n");
                PRINTF("Delete the bonding information of the peer, please try again.\n");
                ret = bt_unpair(BT_ID_DEFAULT, info.le.remote);
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
}

void app_connect_init(void)
{
    bt_conn_cb_register(&conn_callbacks);
}

#endif /* CONFIG_BT_A2DP_SINK */