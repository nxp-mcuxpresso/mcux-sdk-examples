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
#include <bluetooth/sdp.h>
#include <bluetooth/pbap_pce.h>
#include "BT_sdp_api.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app_pbap_pce.h"
#include "bt_pal_conn_internal.h"
#include "bt_pal_keys.h"
#include "bt_pal_pbap_internal.h"

static struct bt_conn *default_conn;
static bt_addr_t default_peer_addr;

extern app_pbap_pce_t g_PbapPce;
#define SDP_CLIENT_USER_BUF_LEN 512U
NET_BUF_POOL_FIXED_DEFINE(sdp_client_pool, CONFIG_BT_MAX_CONN, SDP_CLIENT_USER_BUF_LEN, NULL);

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

static uint8_t bt_pbap_pce_sdp_user(struct bt_conn *conn, struct bt_sdp_client_result *result)
{
    int res;
    uint32_t peer_feature = 0;
    uint16_t rfommchannel = 0;
    uint16_t l2cappsm     = 0;
    uint16_t pbap_version = 0;
    uint8_t supported_repo = 0;
    if ((result) && (result->resp_buf))
    {
        PRINTF("sdp success callback\r\n");
        res = bt_sdp_get_profile_version(result->resp_buf, BT_SDP_PBAP_SVCLASS, &pbap_version);
        if (res < 0)
        {
            PRINTF("pbap version is not found\r\n");
        }
        else
        {
            PRINTF("pbap version is %x\r\n", pbap_version);
            g_PbapPce.pbap_version = pbap_version;
        }
        res = bt_sdp_get_supported_repositories(result->resp_buf, &supported_repo);
        if(res < 0)
        {
            PRINTF("pbap pse supported repositories is not found\r\n");
        }
        else
        {
            PRINTF("pbap pse supported repositories is %x\r\n", supported_repo);
            g_PbapPce.supported_repositories = supported_repo;
        }
        res = bt_sdp_get_pbap_map_ctn_features(result->resp_buf, &peer_feature);
        if (res < 0)
        {
            PRINTF("supported feature not found, use default feature_config : %x\r\n", BT_PBAP_SUPPORTED_FEATURES_V11);
            g_PbapPce.peer_feature = BT_PBAP_SUPPORTED_FEATURES_V11;
        }
        else
        {
            PRINTF("supported feature = %x\r\n", peer_feature);
            g_PbapPce.peer_feature = peer_feature;
        }
        g_PbapPce.loacal_feature = CONFIG_BT_PBAP_PCE_SUPPORTED_FEATURE;
        res = bt_sdp_get_goep_l2cap_psm(result->resp_buf, &l2cappsm);
        if (res >= 0)
        {
            PRINTF("l2cap_psm found. Connecting ...\n");
            res = bt_pbap_pce_psm_connect(conn, l2cappsm, NULL, peer_feature, &g_PbapPce.pbap_pceHandle);
            if (res < 0)
            {
                PRINTF("Send connect command failed\r\n");
            }
            g_PbapPce.goep_version = BT_GOEP_VERSION_2_0;
            return BT_SDP_DISCOVER_UUID_STOP;
        }

        res = bt_sdp_get_proto_param(result->resp_buf, BT_SDP_PROTO_RFCOMM, &rfommchannel);
        if (res < 0)
        {
            PRINTF("Fail to find rfcomm channel and l2cap_psm!\r\n");
        }
        else
        {
            PRINTF("rfcomm channel found. Connecting ...\n");
            res = bt_pbap_pce_scn_connect(conn, rfommchannel, NULL, peer_feature, &g_PbapPce.pbap_pceHandle);
            if (res < 0)
            {
                PRINTF("Send connect command failed\r\n");
            }
            g_PbapPce.goep_version = BT_GOEP_VERSION_1_1;
        }
    }

    return BT_SDP_DISCOVER_UUID_STOP;
}

static struct bt_sdp_discover_params discov_pbap_pce = {
    .uuid = BT_UUID_DECLARE_16(BT_SDP_PBAP_PSE_SVCLASS),
    .func = bt_pbap_pce_sdp_user,
    .pool = &sdp_client_pool,
};

static void bt_connected(struct bt_conn *conn, uint8_t err)
{
    int retval = 0;
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
        default_conn = bt_conn_ref(conn);
        retval       = bt_sdp_discover(conn, &discov_pbap_pce);
        if (retval)
        {
            PRINTF("SDP discovery failed: result\r\n");
        }
        else
        {
            PRINTF("SDP discovery started\r\n");
        }
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
