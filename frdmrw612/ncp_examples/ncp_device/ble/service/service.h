
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

/*******************************************************************************
* Definitions
******************************************************************************/
#define PERIPHERAL_HTS_SERVICE_ID      1
#define PERIPHERAL_HRS_SERVICE_ID      2
#define BAS_SERVICE_ID      3
#define CENTRAL_HTC_SERVICE_ID         4
#define CENTRAL_HRC_SERVICE_ID         5
#define PERIPHERAL_NCS_SERVICE_ID      6

/* HTS flag values */
#define hts_unit_celsius_c        0x00U /* bit 0 unset */
#define hts_unit_fahrenheit_c     0x01U /* bit 0 set */

#define hts_include_temp_type     0x04U /* bit 2 set */


/* Temperature measurement format */
struct temp_measurement
{
    uint8_t flags;
    uint8_t temperature[4];
    uint8_t type;
};

/* Possible temperature sensor locations */
enum
{
    hts_no_temp_type = 0x00U,
    hts_armpit       = 0x01U,
    hts_body         = 0x02U,
    hts_ear          = 0x03U,
    hts_finger       = 0x04U,
    hts_gastroInt    = 0x05U,
    hts_mouth        = 0x06U,
    hts_rectum       = 0x07U,
    hts_toe          = 0x08U,
    hts_tympanum     = 0x09U,
};

/*******************************************************************************
* Prototypes
******************************************************************************/

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

struct adv_report_data
{
    uint8_t type;
    uint8_t data_len;
    uint8_t *data;
};

struct adv_report_cb_t
{
    uint8_t svc_id;
    bool (*p_adv_report_fn)(struct adv_report_data *data, void *user_addr);
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

void le_service_adv_report_process(uint8_t *data);

void svc_scan_start(void);

#endif
