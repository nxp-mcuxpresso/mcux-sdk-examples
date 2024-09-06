/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"

#include "FreeRTOS.h"
#include "task.h"

#include <porting.h>
#include <string.h>
#include <errno.h>
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
#include "BT_common.h"
#include "BT_pbap_api.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"
#include "db_gen.h"
#include "app_pbap_pce.h"
#include "app_connect.h"
#include "app_discover.h"

app_pbap_pce_t g_PbapPce;

#define CONFIG_BT_PRINTF_VALUE 1

#define PBAP_SET_PATH_CHILD_FLAG  "./"
#define PBAP_SET_PATH_ROOT_FLAG   "/"
#define PBAP_SET_PATH_PARENT_FLAG ".."

#define PBAP_PULL_PHONEBOOK_DEMO_NAME     "telecom/pb.vcf"
#define PBAP_PULL_VCARD_LISTING_DEMO_NAME "cch"
#define PBAP_PULL_VCARD_ENTRY_DEMO_NAME   "4.vcf"

#define PBAP_ROOT_PATH                        "root"
#define PBAP_PHONEBOOK_DEMO_RELATIVE_PATH     "telecom"
#define PBAP_VCARD_LISTING_DEMO_RELATIVE_PATH "cch"
#define PBAP_VCARD_LISTING_ABSOLUTE_PATH      "root/telecom"
#define PBAP_VCARD_ENTRY_ABSOLUTE_PATH        "root/telecom/cch"

NET_BUF_POOL_FIXED_DEFINE(pbap_appl_pool, CONFIG_BT_MAX_CONN, CONFIG_BT_PBAP_PCE_MAX_PKT_LEN, NULL);

#define PBAP_CLASS_OF_DEVICE (0x10020CU)/* Object Transfer, Phone, Smartphone */

static struct bt_sdp_attribute pbap_pce_attrs[] = {
    BT_SDP_NEW_SERVICE,
        BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        /* ServiceClassIDList */
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_PBAP_PCE_SVCLASS) //11 2E
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROFILE_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 8), //35 08
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6), //35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PBAP_SVCLASS) //11 30
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0102U) //01 02
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("Phonebook Access PCE"),
};

static struct bt_sdp_record pbap_pce_rec = BT_SDP_RECORD(pbap_pce_attrs);
struct pbap_hdr
{
    uint8_t *value;
    uint16_t length;
};

static void app_connected(struct bt_pbap_pce *pbap_pce)
{
    PRINTF("PABP connect successfully\r\n");
    memcpy(g_PbapPce.currentpath, PBAP_ROOT_PATH, BT_str_len(PBAP_ROOT_PATH));
    app_pull_phonebook(PBAP_PULL_PHONEBOOK_DEMO_NAME);
}

static void app_get_auth_info_cb(struct bt_pbap_pce *pbap_pce, struct bt_pbap_auth *pbap_atuh_info)
{
    return;
}

static void app_disconnected(struct bt_pbap_pce *pbap_pce, uint8_t result)
{
    PRINTF("PABP disconnect successfully: %x\r\n", result);
    if (result == BT_PBAP_FORBIDDEN_RSP)
    {
        PRINTF("Possible reasons is Authentication failure\r\n");
    }
}

static void app_error_result(uint8_t result)
{
    switch (result)
    {
        case BT_PBAP_BAD_REQ_RSP:
            PRINTF("Function not recognized or ill-formatted: %x\r\n", result);
            break;
        case BT_PBAP_NOT_IMPLEMENTED_RSP:
            PRINTF("Function recognized but not supported: %x\r\n", result);
            break;
        case BT_PBAP_UNAUTH_RSP:
            PRINTF("Access is not authorized: %x\r\n", result);
            break;
        case BT_PBAP_PRECOND_FAILED_RSP:
            PRINTF("One of request parameter is wrong: %x\r\n", result);
            break;
        case BT_PBAP_NOT_FOUND_RSP:
            PRINTF("object not found: %x\r\n", result);
            break;
        case BT_PBAP_NOT_ACCEPTABLE_RSP:
            PRINTF("PSE can not meet one of request parameter: %x\r\n", result);
            break;
        case BT_PBAP_NO_SERVICE_RSP:
            PRINTF("System condition prevents it: %x\r\n", result);
            break;
        case BT_PBAP_FORBIDDEN_RSP:
            PRINTF("Temporarily barred: %x\r\n", result);
            break;
    }
    return;
}

static void app_print_body(struct net_buf *buf)
{
    struct pbap_hdr body;
    if (bt_pbap_pce_get_body(buf, &body.value, &body.length) == 0)
    {
        PRINTF("============== BODY ==============\r\n");
        PRINTF("%.*s\r\n", body.length, body.value);
        PRINTF("============ END BODY ============\r\n");
    }
    else
    {
        PRINTF("BODY not Found \r\n");
    }
}

static bool app_app_param_cb(struct bt_data *data, void *user_data)
{
    switch (data->type)
    {
        case BT_PBAP_TAG_ID_PHONE_BOOK_SIZE:
            PRINTF("Phonebook Size - %d\r\n", sys_get_be16(data->data));
            break;

        case BT_PBAP_TAG_ID_NEW_MISSED_CALLS:
            PRINTF("New Missed Calls -%d\r\n", data->data[0]);
            break;

        case BT_PBAP_TAG_ID_PRIMARY_FOLDER_VERSION:
            PRINTF("Primary Floder Version - ");
            for (uint8_t index = 0; index < data->data_len; index++)
            {
                PRINTF("%02X", data->data[index]);
            }
            PRINTF("\r\n");
            break;

        case BT_PBAP_TAG_ID_SECONDARY_FOLDER_VERSION:
            PRINTF("Secondary Floder Version - ");
            for (uint8_t index = 0; index < data->data_len; index++)
            {
                PRINTF("%02X", data->data[index]);
            }
            PRINTF("\r\n");
            break;

        case BT_PBAP_TAG_ID_DATABASE_IDENTIFIER:
            PRINTF("Database Identifier - ");
            for (uint8_t index = 0; index < data->data_len; index++)
            {
                PRINTF("%02X", data->data[index]);
            }
            PRINTF("\r\n");
            break;
    }
    return true;
}

static void app_pull_phonebook_cb(struct bt_pbap_pce *pbap_pce, uint8_t result, struct net_buf *buf)
{
    int revert;
    if (result != PBAP_CONTINUE_RSP && result != BT_PBAP_SUCCESS_RSP)
    {
        app_error_result(result);
    }
    else
    {
        PRINTF("pull phonebook result - 0x%02X\r\n", result);
    }
    bt_pbap_pce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == PBAP_CONTINUE_RSP)
    {
        g_PbapPce.lcl_srmp_wait = (--g_PbapPce.num_srmp_wait) > 0 ? true : false;
        buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
        if (buf == NULL)
        {
            return;
        }
        net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_PHONEBOOK(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_END));
        revert = bt_pbap_pce_pull_phonebook(g_PbapPce.pbap_pceHandle, buf, NULL, g_PbapPce.lcl_srmp_wait,
                                            BT_OBEX_REQ_END);
        if (revert != 0)
        {
            net_buf_unref(buf);
            PRINTF("pb_download callback  failed %x", result);
        }
    }
    else if (result == BT_PBAP_SUCCESS_RSP)
    {
        revert = app_set_phonebook_path(PBAP_SET_PATH_CHILD_FLAG PBAP_PHONEBOOK_DEMO_RELATIVE_PATH);
        if (revert < 0)
        {
            PRINTF("set path command send failed\r\n");
            return;
        }
        memcpy(g_PbapPce.currentpath, PBAP_VCARD_LISTING_ABSOLUTE_PATH, BT_str_len(PBAP_VCARD_LISTING_ABSOLUTE_PATH) + 1);
    }
    return;
}

static void app_set_phonebook_path_cb(struct bt_pbap_pce *pbap_pce, uint8_t result)
{
    int revert = 0;
    switch (result)
    {
        case BT_PBAP_SUCCESS_RSP:
            PRINTF("pbap pse path set success\r\n");
            break;

        default:
            PRINTF("pbap pse path set fail %x\r\n", result);
            return;
    }
    if (strcmp(g_PbapPce.currentpath, PBAP_VCARD_LISTING_ABSOLUTE_PATH) == 0)
    {
        revert = app_pull_vcard_listing(PBAP_PULL_VCARD_LISTING_DEMO_NAME);
    }
    else if (strcmp(g_PbapPce.currentpath, PBAP_VCARD_ENTRY_ABSOLUTE_PATH) == 0)
    {
        revert = app_pull_vcard_entry(PBAP_PULL_VCARD_ENTRY_DEMO_NAME);
    }
    else if (strcmp(g_PbapPce.currentpath, PBAP_ROOT_PATH) == 0)
    {
        revert = bt_pbap_pce_disconnect(pbap_pce);
    }
    if (revert < 0)
    {
        PRINTF("set path command send failed\r\n");
        return;
    }
}

static void app_pull_vcard_listing_cb(struct bt_pbap_pce *pbap_pce, uint8_t result, struct net_buf *buf)
{
    int revert;
    if (result != PBAP_CONTINUE_RSP && result != BT_PBAP_SUCCESS_RSP)
    {
        app_error_result(result);
    }
    else
    {
        PRINTF("pull vcard listing result - 0x%02X\r\n", result);
    }
    bt_pbap_pce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_PBAP_CONTINUE_RSP)
    {
        if (result == BT_PBAP_CONTINUE_RSP)
        {
            g_PbapPce.lcl_srmp_wait = --g_PbapPce.num_srmp_wait > 0 ? true : false;
            buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
            if (buf == NULL)
            {
                return;
            }
            net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_VCARD_LISTING(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_END));

            revert = bt_pbap_pce_pull_vcard_listing(g_PbapPce.pbap_pceHandle, buf, NULL, g_PbapPce.lcl_srmp_wait, BT_OBEX_REQ_END);
            if (revert != 0)
            {
                net_buf_unref(buf);
                PRINTF("pull_vcard_listing callback failed %x", result);
            }
        }
    }
    else if (result == BT_PBAP_SUCCESS_RSP)
    {
        revert = app_set_phonebook_path(PBAP_SET_PATH_CHILD_FLAG PBAP_VCARD_LISTING_DEMO_RELATIVE_PATH);
        if (revert < 0)
        {
            PRINTF("set path command send failed\r\n");
            return;
        }
        memcpy(g_PbapPce.currentpath, PBAP_VCARD_ENTRY_ABSOLUTE_PATH, BT_str_len(PBAP_VCARD_ENTRY_ABSOLUTE_PATH) + 1);
    }
    return;
}

static void app_pull_vcard_entry_cb(struct bt_pbap_pce *pbap_pce, uint8_t result, struct net_buf *buf)
{
    int revert;
    if (result != PBAP_CONTINUE_RSP && result != BT_PBAP_SUCCESS_RSP)
    {
        app_error_result(result);
    }
    else
    {
        PRINTF("pull vcard listing result - 0x%02X\r\n", result);
    }
    bt_pbap_pce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_PBAP_CONTINUE_RSP)
    {
        g_PbapPce.lcl_srmp_wait = --g_PbapPce.num_srmp_wait > 0 ? true : false;
        buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
        if (buf == NULL)
        {
            return;
        }
        net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_VCARD_ENTRY(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_END));

        revert = bt_pbap_pce_pull_vcard_entry(g_PbapPce.pbap_pceHandle, buf, NULL, g_PbapPce.lcl_srmp_wait, BT_OBEX_REQ_END);
        if (revert != 0)
        {
            net_buf_unref(buf);
            PRINTF("pull_vcard_entry callback failed %x", result);
        }
    }
    else if (result == BT_PBAP_SUCCESS_RSP)
    {
        revert = app_set_phonebook_path(PBAP_SET_PATH_ROOT_FLAG);
        if (revert < 0)
        {
            PRINTF("set path command send failed\r\n");
            return;
        }
        memcpy(g_PbapPce.currentpath, PBAP_ROOT_PATH, BT_str_len(PBAP_ROOT_PATH) + 1);
    }
}

static struct bt_pbap_pce_cb pce_cb = {
    .connected     = app_connected,
    .disconnected  = app_disconnected,
    .get_auth_info = app_get_auth_info_cb,
    .pull_phonebook     = app_pull_phonebook_cb,
    .set_phonebook_path = app_set_phonebook_path_cb,
    .pull_vcard_listing = app_pull_vcard_listing_cb,
    .pull_vcard_entry   = app_pull_vcard_entry_cb,
};

static void bt_ready(int err)
{
    struct net_buf *buf = NULL;
    struct bt_hci_cp_write_class_of_device *cp;

    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    PRINTF("Bluetooth initialized\n");

#if (defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0))
    settings_load();
#endif /* CONFIG_BT_SETTINGS */

    buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, sizeof(*cp));
    if (buf != NULL)
    {
        cp = net_buf_add(buf, sizeof(*cp));
        sys_put_le24(PBAP_CLASS_OF_DEVICE, &cp->class_of_device[0]);
        err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, buf, NULL);
    }
    else
    {
        err = -ENOBUFS;
    }

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

    app_connect_init();
    pbap_pce_rec.handle = DB_RECORD_PBAP_PCE;
    bt_sdp_register_service(&pbap_pce_rec);
    err = bt_pbap_pce_register(&pce_cb);
    if (err != 0)
    {
        PRINTF("PBAP register failed\r\n");
        return;
    }
    app_discover();
}

int app_pull_phonebook(char *name)
{
    API_RESULT retval = 0;
    struct net_buf *buf;

    if (name == NULL)
    {
        return -EINVAL;
    }

    buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
    if (!buf)
    {
        return -EINVAL;
    }
    net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_PHONEBOOK(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_UNSEG));

    BT_PBAP_ADD_PARAMS_MAX_LIST_COUNT(buf, 65535);

    g_PbapPce.num_srmp_wait = 0;
    g_PbapPce.lcl_srmp_wait = g_PbapPce.num_srmp_wait > 0 ? true : false;

    retval = bt_pbap_pce_pull_phonebook(g_PbapPce.pbap_pceHandle, buf, name, g_PbapPce.lcl_srmp_wait, BT_OBEX_REQ_UNSEG);
    if (API_SUCCESS != retval)
    {
        net_buf_unref(buf);
        PRINTF("Send request failed\r\n");
    }

    return retval;
}

int app_set_phonebook_path(char *name)
{
    API_RESULT retval = 0;
    struct net_buf *buf;

    if (name == NULL)
    {
        return -EINVAL;
    }

    buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
    if (!buf)
    {
        return -EINVAL;
    }
    net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_SET_PATH(g_PbapPce.pbap_pceHandle));

    retval = bt_pbap_pce_set_phonebook_path(g_PbapPce.pbap_pceHandle, buf, name);
    if (API_SUCCESS != retval)
    {
        net_buf_unref(buf);
        PRINTF("Send request failed\r\n");
    }

    return retval;
}

int app_pull_vcard_listing(char *name)
{
    API_RESULT retval = 0;
    struct net_buf *buf;

    if (name == NULL)
    {
        return -EINVAL;
    }

    buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
    if (!buf)
    {
        return -EINVAL;
    }
    net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_VCARD_LISTING(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_UNSEG));

    BT_PBAP_ADD_PARAMS_MAX_LIST_COUNT(buf, 65535);

    g_PbapPce.num_srmp_wait = 0;
    g_PbapPce.lcl_srmp_wait = g_PbapPce.num_srmp_wait > 0 ? true : false;

    retval = bt_pbap_pce_pull_vcard_listing(g_PbapPce.pbap_pceHandle, buf, name, g_PbapPce.lcl_srmp_wait, BT_OBEX_REQ_UNSEG);
    if (API_SUCCESS != retval)
    {
        net_buf_unref(buf);
        PRINTF("Send request failed\r\n");
    }

    return retval;
}

int app_pull_vcard_entry(char *name)
{
    API_RESULT retval = 0;
    struct net_buf *buf;

    if (name == NULL)
    {
        return -EINVAL;
    }

    buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
    if (!buf)
    {
        return -EINVAL;
    }
    net_buf_reserve(buf, BT_PBAP_PCE_RSV_LEN_PULL_VCARD_ENTRY(g_PbapPce.pbap_pceHandle, BT_OBEX_REQ_UNSEG));

    g_PbapPce.num_srmp_wait = 0;
    g_PbapPce.lcl_srmp_wait = g_PbapPce.num_srmp_wait > 0 ? true : false;

    retval = bt_pbap_pce_pull_vcard_entry(g_PbapPce.pbap_pceHandle, buf, name, g_PbapPce.lcl_srmp_wait, BT_OBEX_REQ_UNSEG);
    if (API_SUCCESS != retval)
    {
        net_buf_unref(buf);
        PRINTF("Send request failed\r\n");
    }

    return retval;
}

void pbap_pce_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth PBAP PCE demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }
    vTaskDelete(NULL);
}
