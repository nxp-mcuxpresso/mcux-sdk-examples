
/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __NCP_BLE_SERVICE_H__
#define __NCP_BLE_SERVICE_H__

#include <stddef.h>
#include <sys/slist.h>
#if !(defined(__ICCARM__) || defined(__CC_ARM) || defined(__ARMCC_VERSION))
#include <sys/types.h>
#endif
#include <sys/util.h>
#include <bluetooth/conn.h>

#include <toolchain.h>
#include "ncp_tlv_adapter.h"

#define PERIPHERAL_HTS_SERVICE_ID      1
#define PERIPHERAL_HRS_SERVICE_ID      2
#define BAS_SERVICE_ID      3
#define CENTRAL_HTC_SERVICE_ID         4
#define CENTRAL_HRC_SERVICE_ID         5

struct service_t
{
    uint8_t svc_id;
    bool is_registered;
    const char *def;
    void (*svc_task)(void *args);
    void (*init)(void);
};

struct service_cb_t
{
    uint8_t svc_id;
    void    (*svc_cb)(void *args);
    // void    (*svc_cb)(struct bt_conn *conn, ...);
};

typedef NCP_TLV_PACK_START struct service_connect_param {
    struct bt_conn *conn;
} NCP_TLV_PACK_END service_connect_param_t;

typedef NCP_TLV_PACK_START struct service_scan_param {
    struct bt_data *data;
    void *user_data;
} NCP_TLV_PACK_END service_scan_param_t;

int ncp_ble_register_service(uint8_t id);

void le_service_connect(struct bt_conn *conn, uint8_t err);

void le_service_disconnect(struct bt_conn *conn, uint8_t reason);

void le_service_security(struct bt_conn *conn, bt_security_t level, enum bt_security_err err);

void le_service_auth_passkey(struct bt_conn *conn, unsigned int passkey);

void le_service_auth_cancel(struct bt_conn *conn);

void le_service_eir_found(struct net_buf_simple* adv_buf);

#endif
