/*
 * Copyright 2020 - 2021, 2024 NXP
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
#include "app_a2dp_sink.h"
#include "app_connect.h"
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
#include "lfs.h"
#include "littlefs_pl.h"
#endif

static int app_auto_connect_del_addr(bt_addr_t const *addr);
static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void security_changed(struct bt_conn *conn, bt_security_t level,
				 enum bt_security_err err);
struct bt_conn *default_conn;

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed,
};

static uint8_t default_connect_initialized;
static uint8_t connectable_set;
static bt_addr_t auto_connect_device;
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
static lfs_t * lfs;
static lfs_file_t lfs_file;
#define FILE_NAME "peer_addr"
#endif

void app_a2dp_set_connectable(void)
{
    int err;
    
    if (connectable_set)
    {
        return;
    }

    connectable_set = 1U;

    err = bt_br_set_connectable(true);
    if (err)
    {
        PRINTF("BR/EDR set/rest connectable failed (err %d)\n", err);
        return;
    }
    err = bt_br_set_discoverable(true);
    if (err)
    {
        PRINTF("BR/EDR set discoverable failed (err %d)\n", err);
        return;
    }
    PRINTF("BR/EDR set connectable and discoverable done\n");
    PRINTF("Wait for connection\r\n");
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
        PRINTF("Connection failed (err 0x%02x)\n", err);
        default_conn = NULL;
        if (default_connect_initialized)
        {
            default_connect_initialized = 0U;
            app_a2dp_set_connectable();
        }
    }
    else
    {
        PRINTF("Connected\n");
        default_conn = conn;
        if (default_connect_initialized)
        {
            struct bt_conn_info info;

            default_connect_initialized = 0U;
            bt_conn_get_info(conn, &info);
            if (info.type == BT_CONN_TYPE_LE)
            {
                return;
            }

            if (0U == memcmp(info.br.dst, &auto_connect_device, 6U))
            {
                app_a2dp_connect(conn);
            }
        }
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);
    default_conn = NULL;
    app_a2dp_set_connectable();
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
        PRINTF("Security failed: %s level %u err %d\n", addr, level, err);
        if (err == BT_SECURITY_ERR_PIN_OR_KEY_MISSING)
        {
            bt_addr_le_t addr;
            struct bt_conn_info info;
            int ret;
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
            int ret2;
#endif

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
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
            ret2 = app_auto_connect_del_addr(&addr.a);
            if (ret || ret2)
#else
            if (ret)
#endif
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

void app_connect_init(void)
{
    bt_conn_cb_register(&conn_callbacks);
}

#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
static int app_auto_connect_get_addr()
{
    int err;
    int len;

    err = lfs_file_open(lfs, &lfs_file, FILE_NAME, LFS_O_RDONLY);

    if (err)
    {
        return err;
    }

    len = lfs_file_read(lfs, &lfs_file, (char *)&auto_connect_device.val[0], 6U);
    if (len != 6U)
    {
        err = -EIO;
    }
    else
    {
        err = 0;
    }
    lfs_file_close(lfs, &lfs_file);

    return err;
}

static int app_auto_connect_del_addr(bt_addr_t const *addr)
{
    int err = lfs_remove(lfs, FILE_NAME);

    return err;
}

int app_auto_connect_save_addr(bt_addr_t const *addr)
{
    int err;
    int len;

    lfs_remove(lfs, FILE_NAME);
    err = lfs_file_open(lfs, &lfs_file, FILE_NAME, LFS_O_WRONLY | LFS_O_CREAT);

    if (err)
    {
        PRINTF("fail to save device addr\r\n");
        return err;
    }

    len = lfs_file_write(lfs, &lfs_file, addr, 6U);
    if (len != 6U)
    {
        PRINTF("fail to save device addr\r\n");
        err = -EIO;
    }
    else
    {
        err = 0;
    }
    lfs_file_close(lfs, &lfs_file);

    return err;
}
#endif

#if ((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
static void bond_info(const struct bt_bond_info *info, void *user_data)
{
	uint8_t *valid = (uint8_t *)user_data;

    if (!(*valid))
    {
        *valid = 1u;
        auto_connect_device = info->addr.a;
    }
}
#endif

void app_a2dp_snk_auto_connect(void)
{
#if ((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
    uint8_t valid = 0;

    memset(&auto_connect_device, 0, sizeof(auto_connect_device));
    bt_foreach_bond(BT_ID_DEFAULT, bond_info, &valid);
    
    if (valid)
    {
        struct bt_conn *conn;

        default_connect_initialized = 1U;
        conn = bt_conn_create_br(&auto_connect_device, BT_BR_CONN_PARAM_DEFAULT);
        if (!conn)
        {
            PRINTF("Connection failed\r\n");
        }
        else
        {
            bt_conn_unref(conn);
            PRINTF("Connection pending\r\n");
        }
    }
    else
    {
        app_a2dp_set_connectable();
    }
#else
    lfs = lfs_pl_init();
    if ((lfs != NULL) && !app_auto_connect_get_addr())
    {
        struct bt_conn *conn;

        default_connect_initialized = 1U;
        conn = bt_conn_create_br(&auto_connect_device, BT_BR_CONN_PARAM_DEFAULT);
        if (!conn)
        {
            PRINTF("Connection failed\r\n");
        }
        else
        {
            bt_conn_unref(conn);
            PRINTF("Connection pending\r\n");
        }
    }
    else
    {
        app_a2dp_set_connectable();
    }
#endif
}
