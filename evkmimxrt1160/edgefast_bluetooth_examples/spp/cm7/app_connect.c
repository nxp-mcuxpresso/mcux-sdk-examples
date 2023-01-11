/*
 * Copyright 2020 NXP
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
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void security_changed(struct bt_conn *conn, bt_security_t level,
				 enum bt_security_err err);

struct bt_conn *default_conn;

struct bt_conn *br_conns[CONFIG_BT_MAX_CONN];

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed,
};

static void connected(struct bt_conn *conn, uint8_t err)
{
    struct bt_conn_info info;
    uint8_t             index;
    char                conn_addr[BT_ADDR_LE_STR_LEN];

    if (err)
    {
        if (default_conn != NULL)
        {
            default_conn = NULL;
        }
        PRINTF("BR connection failed (err 0x%02x)\n", err);
    }
    else
    {
        bt_conn_get_info(conn, &info);
        if (info.type == BT_CONN_TYPE_LE)
        {
            PRINTF("Invalid br connection, disconnecting...\n");
            bt_conn_disconnect(conn, 0x13U);
        }
        else
        {
            default_conn = bt_conn_ref(conn);

            for(index = 0U; index < CONFIG_BT_MAX_CONN; index++)
            {
                if(NULL == br_conns[index])
                {
                    break;
                }
            }

            if(CONFIG_BT_MAX_CONN == index)
            {
                PRINTF("CONFIG_BT_MAX_CONN %d is reached!\n", index);
                bt_conn_disconnect(default_conn, BT_HCI_ERR_CONN_LIMIT_EXCEEDED);
            }
            else
            {
                bt_addr_to_str(bt_conn_get_dst_br(default_conn), conn_addr, sizeof(conn_addr));
                PRINTF("BR connection with %s is created successfully!\n", conn_addr);
                br_conns[index] = default_conn;
            }
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    uint8_t index;
    char    conn_addr[BT_ADDR_LE_STR_LEN];

    bt_conn_unref(conn);

    for(index = 0U; index < CONFIG_BT_MAX_CONN; index++)
    {
        if(br_conns[index] == conn)
        {
            break;
        }
    }

    if(CONFIG_BT_MAX_CONN == index)
    {
        return;
    }
    else
    {
        bt_addr_to_str(bt_conn_get_dst_br(conn), conn_addr, sizeof(conn_addr));
        PRINTF("BR connection with %s is disconnected (reason 0x%02x)\n", conn_addr, reason);

        if (br_conns[index] == default_conn)
        {
            default_conn = NULL;
        }
        br_conns[index] = NULL;
    }
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
            PRINTF("\n");

            PRINTF("___________________________________________________________\n");
            PRINTF("The peer device seems to have lost the bonding information.\n");
            PRINTF("Kindly delete the bonding information of the peer from the\n");
            PRINTF("and try again.\n\n");

            PRINTF("\n");
        }
        PRINTF("Security failed: %s level %u err %d\n", addr, level, err);
    }
}

void app_connect(uint8_t *addr)
{
    bt_addr_t peer_addr;

    memcpy(&peer_addr, addr, 6U);
    default_conn = bt_conn_create_br(&peer_addr, BT_BR_CONN_PARAM_DEFAULT);
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

void app_delete(void)
{
}

void app_connect_init(void)
{
    uint8_t index;

    bt_conn_cb_register(&conn_callbacks);

    for(index = 0U; index < CONFIG_BT_MAX_CONN; index++)
    {
        br_conns[index] = NULL;
    }
}
