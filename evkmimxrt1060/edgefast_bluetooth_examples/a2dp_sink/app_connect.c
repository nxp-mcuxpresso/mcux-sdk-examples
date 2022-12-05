/*
 * Copyright 2020 - 2021 NXP
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
#include <bluetooth/a2dp.h>
#include <bluetooth/a2dp_codec_sbc.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

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
    if (err) {
        PRINTF("Connection failed (err 0x%02x)\n", err);
    } else {
        PRINTF("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);
}

static void security_changed(struct bt_conn *conn, bt_security_t level,
				 enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_to_str(bt_conn_get_dst_br(conn), addr, sizeof(addr));

    if (!err)
    {
        PRINTF("Security changed: %s level %u\n", addr, level);
    }
    else
    {
        if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING)
        {
            PRINTF("\r\n");

            PRINTF("___________________________________________________________\n");
            PRINTF("The peer device seems to have lost the bonding information.\n");
            PRINTF("Kindly delete the bonding information of the peer from the\n");
            PRINTF("and try again.\r\n\r\n");

            PRINTF("\r\n");
        }
        PRINTF("Security failed: %s level %u err %d\n", addr, level, err);
    }
}

void app_connect_init(void)
{
    bt_conn_cb_register(&conn_callbacks);
}
