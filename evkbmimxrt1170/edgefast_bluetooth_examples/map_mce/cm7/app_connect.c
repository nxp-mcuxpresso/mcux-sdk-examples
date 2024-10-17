/*
 * Copyright 2024 NXP
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
#include <bluetooth/map_mce.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app_map_mce.h"
#include "bt_pal_conn_internal.h"
#include "bt_pal_keys.h"

#define SDP_CLIENT_USER_BUF_LEN 512U

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);
static uint8_t sdp_discover_cb(struct bt_conn *conn, struct bt_sdp_client_result *result);

extern struct app_map_mce_instance app_instance;
extern struct bt_map_mce_mas_cb map_mas_cb;
extern struct bt_map_mce_mns_cb map_mns_cb;
NET_BUF_POOL_FIXED_DEFINE(sdp_client_pool, CONFIG_BT_MAX_CONN, SDP_CLIENT_USER_BUF_LEN, NULL);


static struct bt_conn_cb conn_callbacks = {
    .connected        = connected,
    .disconnected     = disconnected,
    .security_changed = security_changed,
};

static struct bt_sdp_discover_params discov_map_mse = {
    .uuid = BT_UUID_DECLARE_16(BT_SDP_MAP_MSE_SVCLASS),
    .func = sdp_discover_cb,
    .pool = &sdp_client_pool,
};

static uint8_t sdp_discover_cb(struct bt_conn *conn, struct bt_sdp_client_result *result)
{
    int res;
    uint16_t scn;
    uint16_t psm = 0;
    uint32_t supported_features;
    uint16_t map_version;
    const char *service_name;
    uint8_t mas_instance_id;
    uint8_t supported_msg_type;

    if ((app_instance.acl_conn == conn) && (result) && (result->resp_buf))
    {
        PRINTF("sdp success callback\r\n");
        res = bt_sdp_get_proto_param(result->resp_buf, BT_SDP_PROTO_RFCOMM, &scn);
        if (res < 0)
        {
            PRINTF("REFCOMM channel number is not found\r\n");
            return BT_SDP_DISCOVER_UUID_CONTINUE;
        }
        PRINTF("REFCOMM channel number %d\r\n", scn);
        res = bt_sdp_get_goep_l2cap_psm(result->resp_buf, &psm);
        if (res < 0)
        {
            app_instance.goep_version = BT_GOEP_VERSION_1_1;
            PRINTF("L2CAP PSM is not found\r\n");
        }
        else
        {
            app_instance.goep_version = BT_GOEP_VERSION_2_0;
            PRINTF("L2CAP PSM  0x%04X\r\n", psm);
        }
        res = bt_sdp_get_profile_version(result->resp_buf, BT_SDP_MAP_SVCLASS, &map_version);
        if (res < 0)
        {
            PRINTF("MAP version is not found\r\n");
        }
        else
        {
            PRINTF("MAP version 0x%04X\r\n", map_version);
            app_instance.map_version = map_version;
        }
        res = bt_sdp_get_pbap_map_ctn_features(result->resp_buf, &supported_features);
        if (res < 0)
        {
            switch (app_instance.map_version)
            {
                case BT_MAP_VERSION_1_1:
                    app_instance.supported_features = BT_MAP_MSE_MAS_SUPPORTED_FEATURES_V11;
                    break;
                case BT_MAP_VERSION_1_2:
                    app_instance.supported_features = BT_MAP_MSE_MAS_SUPPORTED_FEATURES_V12;
                    break;
                case BT_MAP_VERSION_1_3:
                    app_instance.supported_features = BT_MAP_MSE_MAS_SUPPORTED_FEATURES_V13;
                    break;
                case BT_MAP_VERSION_1_4:
                    app_instance.supported_features = BT_MAP_MSE_MAS_SUPPORTED_FEATURES_V14;
                    break;
                default:
                    app_instance.supported_features = 0;
                    break;
            }
            PRINTF("Supported features is not found"
            "Use the default supported features 0x%08X\r\n", app_instance.supported_features);
        }
        else
        {
            PRINTF("MAP supported features 0x%08X\r\n", supported_features);
            app_instance.supported_features = supported_features;
        }
        res = bt_sdp_get_instance_id(result->resp_buf, &mas_instance_id);
        if (res < 0)
        {
            PRINTF("MAS instance ID is not found\r\n");
        }
        else
        {
            PRINTF("MAS instance ID %d\r\n", mas_instance_id);
            app_instance.mas_instance_id = mas_instance_id;
        }
        res = bt_sdp_get_supported_msg_type(result->resp_buf, &supported_msg_type);
        if (res < 0)
        {
            PRINTF("Supported message type is not found\r\n");
        }
        else
        {
            PRINTF("Supported message type 0x%02X\r\n", supported_msg_type);
        }
        res = bt_sdp_get_service_name(result->resp_buf, &service_name);
        if (res < 0)
        {
            PRINTF("Service name is not found\r\n");
        }
        else
        {
            PRINTF("Service name %s\r\n", service_name);
        }
        if ((scn != 0U) || (psm != 0U))
        {
            PRINTF("Message Access Server found. Connecting ...\n");

            res = bt_map_mce_mas_register(&map_mas_cb);
            if (0 != res)
            {
                PRINTF("fail to register MCE MAS callback (err: %d)\r\n", res);
            }
            app_instance.psm = psm;
            app_instance.scn = scn;
            if (app_instance.goep_version >= BT_GOEP_VERSION_2_0)
            {
                res = bt_map_mce_psm_connect(app_instance.acl_conn, psm, supported_features, &app_instance.mce_mas);
            }
            else
            {
                res = bt_map_mce_scn_connect(app_instance.acl_conn, (uint8_t)scn, supported_features, &app_instance.mce_mas);
            }
            if (0 != res)
            {
                PRINTF("fail to connect MSE (err: %d)\r\n", res);
            }
        }
        return BT_SDP_DISCOVER_UUID_STOP;
    }
    else
    {
        PRINTF("sdp fail callback\r\n");
        return BT_SDP_DISCOVER_UUID_CONTINUE;
    }
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    int res;
    if (err)
    {
        if (app_instance.acl_conn != NULL)
        {
            app_instance.acl_conn = NULL;
        }
        PRINTF("Connection failed (err 0x%02x)\n", err);
    }
    else
    {
        struct bt_conn_info info;

        bt_conn_get_info(conn, &info);
        if (info.type == BT_CONN_TYPE_LE)
        {
            return;
        }

        app_instance.acl_conn = bt_conn_ref(conn);
        bt_map_mce_mns_register(&map_mns_cb);
        /*
         * Do an SDP Query on Successful ACL connection complete with the
         * required device
         */
        res = bt_sdp_discover(conn, &discov_map_mse);
        if (res)
        {
            PRINTF("SDP discovery failed: result\r\n");
        }
        else
        {
            PRINTF("SDP discovery started\r\n");
        }
        PRINTF("Connected\n");
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    PRINTF("Disconnected (reason 0x%02x)\n", reason);

    if (app_instance.acl_conn != conn)
    {
        return;
    }

    if (app_instance.acl_conn)
    {
        bt_conn_unref(app_instance.acl_conn);
        memset(&app_instance, 0U, sizeof(app_instance));
    }
    else
    {
        return;
    }
#if 0
    app_hfp_ag_disconnect();
#endif
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

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\r\n", addr);
}

static void passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    PRINTF("Passkey %06u\r\n", passkey);
}

static struct bt_conn_auth_cb auth_cb_display = {
    .cancel = auth_cancel, .passkey_display = passkey_display, /* Passkey display callback */
};

void app_connect(uint8_t *addr)
{
    bt_addr_t default_peer_addr;

    memcpy(&default_peer_addr, addr, 6U);
    app_instance.acl_conn = bt_conn_create_br(&default_peer_addr, BT_BR_CONN_PARAM_DEFAULT);
    if (!app_instance.acl_conn)
    {
        PRINTF("Connection failed\r\n");
    }
    else
    {
        /* unref connection obj in advance as app user */
        bt_conn_unref(app_instance.acl_conn);
        PRINTF("Connection pending\r\n");
    }
}

void app_disconnect(void)
{
    if (bt_conn_disconnect(app_instance.acl_conn, 0x13U))
    {
        PRINTF("Disconnection failed\r\n");
    }
}

void app_delete(void)
{
}

void app_connect_init(void)
{
    bt_conn_auth_cb_register(&auth_cb_display);
    bt_conn_cb_register(&conn_callbacks);
}
