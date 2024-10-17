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
#include <bluetooth/pbap_pse.h>
#include <bluetooth/rfcomm.h>
#include "BT_common.h"
#include "BT_pbap_api.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"
#include "db_gen.h"
#include "app_pbap_pse.h"
#include "app_connect.h"

app_pbap_pse_t g_PbapPse;

#define PBAP_PSE_MAX_PKT_SIZE OBEX_MAX_PACKET_LENGTH

NET_BUF_POOL_FIXED_DEFINE(pbap_appl_pool, CONFIG_BT_MAX_CONN, PBAP_PSE_MAX_PKT_SIZE, NULL);

#define PBAP_CLASS_OF_DEVICE (0x10020CU)/* Object Transfer, Phone, Smartphone */

static uint8_t sample_primay_folder_version[16] =
       {
          0x05U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
          0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x01U
       };

static uint8_t sample_secondary_folder_version[16] =
       {
          0x06U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
          0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x02U
       };

static uint8_t sample_database_identifier[16] =
       {
          0x07U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,
          0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x03U
       };

static struct bt_sdp_attribute pbap_pse_attrs[] = {
    BT_SDP_NEW_SERVICE,
    /* ServiceClassIDList */
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_PBAP_PSE_SVCLASS) //11 2F
        },
        )
    ),
    /* ProtocolDescriptorList */
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 17), //35, 11
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), // 35 , 3
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP) //01 00
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 5),// 35 05
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_RFCOMM), // 00 03
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
                BT_SDP_ARRAY_8(BT_RFCOMM_CHAN_PBAP_PSE) //RFCOMM CHANNEL
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3),// 35 03
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTP_OBEX) // 00 08
            },
            )
        },
        )
    ),
    /* BluetoothProfileDescriptorList */
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
    BT_SDP_SERVICE_NAME("Phonebook Access PSE"),
     /* GoepL2CapPsm */
   BT_SDP_ATTR_GOEP_L2CAP_PSM,
   {
       BT_SDP_TYPE_SIZE(BT_SDP_UINT16), 
       BT_SDP_ARRAY_16(BT_BR_PSM_PBAP_PSE)
   },

    /* SupportedRepositories */
    BT_SDP_ATTR_SUPPORTED_REPOSITORIES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT8), 
        BT_SDP_ARRAY_8(CONFIG_BT_PBAP_PSE_SUPPORTED_REPOSITORIES)
    },

    /* PBAP_PSE SupportedFeatures */
    BT_SDP_ATTR_PBAP_SUPPORTED_FEATURES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT32), 
        BT_SDP_ARRAY_32(CONFIG_BT_PBAP_PSE_SUPPORTED_FEATURES)
    },
};

static struct bt_sdp_record pbap_pse_rec = BT_SDP_RECORD(pbap_pse_attrs);

static const char pbap_pse_phonebook_example[] =
    "BEGIN:VCARD\n\
VERSION:2.1\n\
FN;CHARSET=UTF-8:descvs\n\
N;CHARSET=UTF-8:descvs\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;cc;;;\n\
FN:cc\n\
TEL;CELL:154555845\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;qwe;;;\n\
FN:qwe\n\
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;\n\
TEL;CELL:151865216\n\
TEL;CELL:153464856\n\
EMAIL;HOME:wudhxjsjd@qq.com\n\
ADR;HOME:;;123456789;;;;\n\
NOTE:old\n\
BDAY:1904-05-24\n\
X-AIM:@qq.com\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
FN;CHARSET=UTF-8:descvs\n\
N;CHARSET=UTF-8:descvs\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;cc;;;\n\
FN:cc\n\
TEL;CELL:154555845\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;qwe;;;\n\
FN:qwe\n\
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;\n\
TEL;CELL:151865216\n\
TEL;CELL:153464856\n\
EMAIL;HOME:wudhxjsjd@qq.com\n\
ADR;HOME:;;123456789;;;;\n\
NOTE:old\n\
BDAY:1904-05-24\n\
X-AIM:@qq.com\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
FN;CHARSET=UTF-8:descvs\n\
N;CHARSET=UTF-8:descvs\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;cc;;;\n\
FN:cc\n\
TEL;CELL:154555845\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;qwe;;;\n\
FN:qwe\n\
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;\n\
TEL;CELL:151865216\n\
TEL;CELL:153464856\n\
EMAIL;HOME:wudhxjsjd@qq.com\n\
ADR;HOME:;;123456789;;;;\n\
NOTE:old\n\
BDAY:1904-05-24\n\
X-AIM:@qq.com\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
FN;CHARSET=UTF-8:descvs\n\
N;CHARSET=UTF-8:descvs\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;cc;;;\n\
FN:cc\n\
TEL;CELL:154555845\n\
END:VCARD\n\
BEGIN:VCARD\n\
VERSION:2.1\n\
N:;qwe;;;\n\
FN:qwe\n\
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;\n\
TEL;CELL:151865216\n\
TEL;CELL:153464856\n\
EMAIL;HOME:wudhxjsjd@qq.com\n\
ADR;HOME:;;123456789;;;;\n\
NOTE:old\n\
BDAY:1904-05-24\n\
X-AIM:@qq.com\n\
END:VCARD";

static const char pbap_pse_vcard_listing[] =
    "<?xml version=\"1.0\"?><!DOCTYPE vcard-listing SYSTEM \"vcard-listing.dtd\"><vCard-listing version=\"1.0\">\
<card handle=\"1.vcf\" name=\"qwe\"/><card handle=\"2.vcf\" name=\"qwe\"/><card handle=\"3.vcf\" name=\"qwe\"/>\
<card handle=\"4.vcf\" name=\"1155\"/><card handle=\"5.vcf\" name=\"051295205593\"/><card handle=\"6.vcf\" name=\"130\"/>/vCard-listing>";

static const char pbap_pse_vcard_entry[] =
    "BEGIN:VCARD\n\
VERSION:2.1\n\
FN:\n\
N:\n\
TEL;X-0:1155\n\
X-IRMC-CALL-DATETIME;DIALED:20220913T110607\n\
END:VCARD";

char *child_floader_name[8] = {"telecom", "pb", "ich", "och", "mch", "cch", "spd", "fav"};

static int startwith(char *str, char *prefix)
{
    uint8_t str_len    = strlen(str);
    uint8_t prefix_len = strlen(prefix);
    if (prefix_len > str_len)
    {
        return 0;
    }
    return strncmp(str, prefix, prefix_len) == 0;
}
static int endwith(char *str, char *suffix)
{
    uint8_t str_len    = BT_str_len(str);
    uint8_t suffix_len = BT_str_len(suffix);
    if (str_len < suffix_len)
    {
        return 0;
    }
    char *endpart = str + str_len - suffix_len;
    return strcmp(endpart, suffix) == 0;
}

static bool app_app_param_cb(struct bt_data *data, void *user_data)
{
    switch (data->type)
    {
        case BT_PBAP_TAG_ID_ORDER:
            g_PbapPse.req_appl_params.order = data->data[0];
            PRINTF("appl params order : %d\r\n", g_PbapPse.req_appl_params.order);
            break;
        case BT_PBAP_TAG_ID_SEARCH_VALUE:
            g_PbapPse.req_appl_params.search_value.value  = (uint8_t *)data->data;
            g_PbapPse.req_appl_params.search_value.length = data->data_len;
            PRINTF("appl params search value : %.*s\r\n", data->data_len, data->data);
            break;
        case BT_PBAP_TAG_ID_SEARCH_PROPERTY:
            g_PbapPse.req_appl_params.search_attr = data->data[0];
            PRINTF("appl params search property : %d\r\n", g_PbapPse.req_appl_params.search_attr);
            break;
        case BT_PBAP_TAG_ID_MAX_LIST_COUNT:
            g_PbapPse.req_appl_params.max_list_count = sys_get_be16(data->data);
            PRINTF("appl params max list count : %d\r\n", g_PbapPse.req_appl_params.max_list_count);
            break;
        case BT_PBAP_TAG_ID_LIST_START_OFFSET:
            g_PbapPse.req_appl_params.list_start_offset = sys_get_be16(data->data);
            PRINTF("appl params list start offset : %d\r\n", g_PbapPse.req_appl_params.list_start_offset);
            break;
        case BT_PBAP_TAG_ID_PROPERTY_SELECTOR:
            g_PbapPse.req_appl_params.property_selector = sys_get_be64(data->data);
            PRINTF("appl params property selector : %llu\r\n", g_PbapPse.req_appl_params.property_selector);
            break;
        case BT_PBAP_TAG_ID_FORMAT:
            g_PbapPse.req_appl_params.format = data->data[0];
            PRINTF("appl params format : %d\r\n", g_PbapPse.req_appl_params.format);
            break;
        case BT_PBAP_TAG_ID_PHONE_BOOK_SIZE:
            g_PbapPse.req_appl_params.phonebook_size = sys_get_be16(data->data);
            PRINTF("appl params phonebook size : %d\r\n", g_PbapPse.req_appl_params.phonebook_size);
            break;
        case BT_PBAP_TAG_ID_NEW_MISSED_CALLS:
            g_PbapPse.req_appl_params.new_missed_calls = data->data[0];
            PRINTF("appl params new mossed calls : %d\r\n", g_PbapPse.req_appl_params.new_missed_calls);
            break;
        case BT_PBAP_TAG_ID_VCARD_SELECTOR:
            g_PbapPse.req_appl_params.vcard_selector = sys_get_be64(data->data);
            PRINTF("appl params vcard selector : %llu\r\n", g_PbapPse.req_appl_params.vcard_selector);
            break;
        case BT_PBAP_TAG_ID_VCARD_SELECTOR_OPERATOR:
            g_PbapPse.req_appl_params.vcard_selector_operator = data->data[0];
            PRINTF("appl params vcard selector operator : %d\r\n", g_PbapPse.req_appl_params.vcard_selector_operator);
            break;
        case BT_PBAP_TAG_ID_RESET_NEW_MISSED_CALLS:
            g_PbapPse.req_appl_params.reset_new_missed_calls = data->data[0];
            PRINTF("appl params reset new missed calls : %d\r\n", g_PbapPse.req_appl_params.reset_new_missed_calls);
            break;
        case BT_PBAP_TAG_ID_PBAP_SUPPORTED_FEATURES:
            g_PbapPse.req_appl_params.supported_features = sys_get_be32(data->data);
            PRINTF("appl params supported features  : %d\r\n", g_PbapPse.req_appl_params.supported_features);
            break;
        default:
            break;
    }
    return 1;
}

static void app_connected(struct bt_pbap_pse *pbap_pse)
{
    PRINTF("PABP connect successfully\r\n");
    memcpy(g_PbapPse.currentpath, "root", 4);
    g_PbapPse.pbap_pseHandle         = pbap_pse;
    g_PbapPse.lcl_supported_features = CONFIG_BT_PBAP_PSE_SUPPORTED_FEATURES;
    bt_pbap_pse_get_peer_supported_features(pbap_pse, &g_PbapPse.rem_supported_features);
    g_PbapPse.send_rsp      = 0;
    g_PbapPse.remaining_rsp = 0;
}

static void app_get_auth_info_cb(struct bt_pbap_pse *pbap_pse, struct bt_pbap_auth *pbap_atuh_info, bool *active_auth)
{
    return;
}

static void app_disconnected(struct bt_pbap_pse *pbap_pse, uint8_t result)
{
    PRINTF("PABP disconnect successfully : %x\r\n", result);
    if (result == BT_PBAP_FORBIDDEN_RSP)
    {
        PRINTF("Possible reasons is Authentication failure\r\n");
    }
}

static int app_check_pull_phonebook_path(char *name)
{
    uint8_t length = BT_str_len(name);
    uint8_t index  = 0;
    if (name == NULL || length < BT_str_len("pb.vcf"))
    {
        return -EINVAL;
    }
    if (endwith((char *)name, "pb.vcf") == 0)
    {
        PRINTF("name invaild\r\n");
        return -EINVAL;
    }
    /* Check if requested phonebook file has 'SIMI' */
    char *pb_ptr = (char *)BT_str_str(name, "SIM1");
    if (NULL == pb_ptr)
    {
        /* pb_file_name does not have 'SIM1', check for 'telecom' */
        pb_ptr = (char *)BT_str_str(name, "telecom");
        if (NULL != pb_ptr)
        {
            memcpy(g_PbapPse.currentpath, "root/telecom", BT_str_len("root/SIM1/telecom"));
        }
        else
        {
            /* Requested pb_file_name is not valid, Send Error */
            PRINTF("name is invailed\r\n");
            return -EINVAL;
        }
    }
    else
    {
        index += 5U;
        pb_ptr = (char *)BT_str_str(&name[index], (char *)"telecom");
        if (NULL != pb_ptr)
        {
            memcpy(g_PbapPse.currentpath, "root/SIM1/telecom", BT_str_len("root/telecom"));
        }
        else
        {
            /* Requested pb_file_name is not valid, Send Error */
            PRINTF("name is invailed\r\n");
            return -EINVAL;
        }
    }
    return 0;
}

static void app_pull_phonebook_cb(struct bt_pbap_pse *pbap_pse,
                                  struct net_buf *buf,
                                  char *name,
                                  enum bt_obex_req_flags flag)
{
    struct pbap_hdr body;
    int revert           = 0;
    uint8_t result       = BT_PBAP_SUCCESS_RSP;
    uint16_t max_pkt_len = 0;
    if (pbap_pse == NULL || buf == NULL)
    {
        return;
    }
    bt_pbap_pse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);
    if (name != NULL)
    {
        memcpy(g_PbapPse.name, name, BT_str_len(name) + 1);
    }
    if (!(flag & BT_OBEX_REQ_UNSEG)) /* stack do not support now */
    {
        g_PbapPse.remaining_rsp = 0;
        result                  = BT_PBAP_NOT_IMPLEMENTED_RSP;
    }
    else
    {
        if (g_PbapPse.remaining_rsp == 0)
        {
            if (app_check_pull_phonebook_path(g_PbapPse.name) == 0)
            {
                memcpy(g_PbapPse.currentpath, "root\0", BT_str_len("root\0"));
                g_PbapPse.remaining_rsp = BT_str_len(pbap_pse_phonebook_example);
                if (g_PbapPse.remaining_rsp == 0)
                {
                    result = BT_PBAP_NOT_FOUND_RSP;
                }
                else
                {
                    g_PbapPse.send_rsp = 0;
                }
            }
            else
            {
                buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
                if (!buf)
                {
                    return;
                }
                net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
                result = BT_PBAP_NOT_FOUND_RSP;
                g_PbapPse.remaining_rsp = 0;
            }
        }
    }
    if (g_PbapPse.remaining_rsp != 0)
    {
        buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
        if (!buf)
        {
            return;
        }
        net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
        if ((g_PbapPse.lcl_supported_features & BT_PBAP_FEATURE_FOLDER_VERSION_COUNTERS) &&
            (g_PbapPse.rem_supported_features & BT_PBAP_FEATURE_FOLDER_VERSION_COUNTERS))
        {
            BT_PBAP_ADD_PARAMS_PRIMARY_FOLDER_VERSION(buf, sample_primay_folder_version);
            BT_PBAP_ADD_PARAMS_SECONDARY_FOLDER_VERSION(buf, sample_secondary_folder_version);
        }
        if ((g_PbapPse.lcl_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER) &&
            (g_PbapPse.rem_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER))
        {
            BT_PBAP_ADD_PARAMS_DATABASE_IDENTIFIER(buf, sample_database_identifier);
        }
        bt_pbap_pse_get_max_pkt_len(pbap_pse, &max_pkt_len);
        max_pkt_len -= sizeof(struct bt_pbap_push_response_hdr);
        max_pkt_len -= buf->len;
        max_pkt_len -= sizeof(struct bt_obex_hdr_bytes);
        if (g_PbapPse.remaining_rsp <= max_pkt_len)
        {
            body.value  = (uint8_t *)(uint8_t *)&pbap_pse_phonebook_example[g_PbapPse.send_rsp];
            body.length = g_PbapPse.remaining_rsp;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_END_OF_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp  = 0;
            result                   = BT_PBAP_SUCCESS_RSP;
            g_PbapPse.currentpath[4] = 0;
        }
        else
        {
            body.value  = (uint8_t *)&pbap_pse_phonebook_example[g_PbapPse.send_rsp];
            body.length = max_pkt_len;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp -= body.length;
            g_PbapPse.send_rsp += body.length;
            result = BT_PBAP_CONTINUE_RSP;
        }
    }
    PRINTF("send response : %x \r\n", result);
    revert = bt_pbap_pse_pull_phonebook_response(pbap_pse, result, buf, 0);
    if (revert != 0)
    {
        net_buf_unref(buf);
        PRINTF("Send Response failed %x", revert);
    }
}

static void app_set_phonebook_path_cb(struct bt_pbap_pse *pbap_pse, char *name)
{
    int revert      = 0;
    uint8_t result  = 0;
    uint8_t index   = 0;
    char *path_name = NULL;
    if (name == NULL)
    {
        return;
    }
    PRINTF("pse current path is %s\r\n", g_PbapPse.currentpath);
    if (strcmp(name, "/") == 0)
    {
        PRINTF("set path to root\r\n");
        memset(g_PbapPse.currentpath, 0, sizeof(g_PbapPse.currentpath));
        memcpy(g_PbapPse.currentpath, "root\0", 5);
        result = BT_PBAP_SUCCESS_RSP;
    }
    else if (strcmp(name, "..") == 0)
    {
        if (strcmp(g_PbapPse.currentpath, "root") == 0)
        {
            PRINTF("current path is root\r\n");
            result = BT_PBAP_NOT_FOUND_RSP;
        }
        else
        {
            for (int8_t index = BT_str_len(g_PbapPse.currentpath); index >= 0; index--)
            {
                if (g_PbapPse.currentpath[index] == '/')
                {
                    g_PbapPse.currentpath[index] = 0;
                    break;
                }
            }
            PRINTF("set path to parent\r\n");
            result = BT_PBAP_SUCCESS_RSP;
        }
    }
    else if (name[0] == '.' && name[1] == '/' && BT_str_len(name) > 2)
    {
        path_name = &name[2];
        PRINTF("set path to child %s\r\n", path_name);

        if (strcmp(path_name, "SIM1") == 0)
        {
            if (strcmp(g_PbapPse.currentpath, "root") == 0)
            {
                strcat(g_PbapPse.currentpath, "/");
                strcat(g_PbapPse.currentpath, path_name);
                result = BT_PBAP_SUCCESS_RSP;
            }
            else
            {
                result = BT_PBAP_NOT_FOUND_RSP;
            }
        }
        for (index = 0; index < 8; index++)
        {
            if (strcmp(path_name, child_floader_name[index]) == 0)
            {
                if (index == 0)
                {
                    if (endwith(g_PbapPse.currentpath, "root") || endwith(g_PbapPse.currentpath, "SIM1"))
                    {
                        strcat(g_PbapPse.currentpath, "/");
                        strcat(g_PbapPse.currentpath, path_name);
                        result = BT_PBAP_SUCCESS_RSP;
                        break;
                    }
                    else
                    {
                        PRINTF("path incorrent\r\n");
                        result = BT_PBAP_NOT_FOUND_RSP;
                        break;
                    }
                }
                else
                {
                    if (endwith(g_PbapPse.currentpath, "telecom"))
                    {
                        strcat((char *)g_PbapPse.currentpath, "/");
                        strcat((char *)g_PbapPse.currentpath, path_name);
                        result = BT_PBAP_SUCCESS_RSP;
                        break;
                    }
                    else
                    {
                        PRINTF("path incorrent\r\n");
                        result = BT_PBAP_NOT_FOUND_RSP;
                        break;
                    }
                }
            }
        }
        if (index >= 8)
        {
            PRINTF("path incorrent\r\n");
            result = BT_PBAP_NOT_FOUND_RSP;
        }
    }
    if (result == BT_PBAP_SUCCESS_RSP)
    {
        PRINTF("pse set current path is %s\r\n", g_PbapPse.currentpath);
    }
    revert = bt_pbap_pse_set_phonebook_path_response(pbap_pse, result);
    if (API_SUCCESS != revert)
    {
        PRINTF("Send Response failed %x", revert);
    }
}

static int app_check_pull_vcard_listing_path(char *name)
{
    uint8_t index = 0;
    if (name == NULL)
    {
        return -EINVAL;
    }
    for (index = 0; index < 8; index++)
    {
        if (strcmp(name, child_floader_name[index]) == 0)
        {
            break;
        }
    }
    if (index == 8)
    {
        return -EINVAL;
    }
    if (endwith(g_PbapPse.currentpath, "telecom"))
    {
        return 0;
    }
    else
    {
        return -EINVAL;
    }
}

static void app_pull_vcard_listing_cb(struct bt_pbap_pse *pbap_pse,
                                      struct net_buf *buf,
                                      char *name,
                                      enum bt_obex_req_flags flag)
{
    struct pbap_hdr body;
    int revert           = 0;
    uint8_t result       = BT_PBAP_SUCCESS_RSP;
    uint16_t max_pkt_len = 0;
    if (pbap_pse == NULL || buf == NULL)
    {
        return;
    }
    bt_pbap_pse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);
    if (name != NULL)
    {
        memcpy(g_PbapPse.name, name, BT_str_len(name) + 1);
    }
    if (!(flag & BT_OBEX_REQ_UNSEG)) /* stack do not support now */
    {
        g_PbapPse.remaining_rsp = 0;
        result                  = BT_PBAP_NOT_IMPLEMENTED_RSP;
    }
    else
    {
        if (g_PbapPse.remaining_rsp == 0)
        {
            if (app_check_pull_vcard_listing_path(g_PbapPse.name) == 0)
            {
                g_PbapPse.remaining_rsp = BT_str_len(pbap_pse_vcard_listing);
                if (g_PbapPse.remaining_rsp == 0)
                {
                    result = BT_PBAP_NOT_FOUND_RSP;
                }
                else
                {
                    g_PbapPse.send_rsp = 0;
                }
            }
            else
            {
                buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
                if (!buf)
                {
                    return;
                }
                net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
                result                  = BT_PBAP_NOT_FOUND_RSP;
                g_PbapPse.remaining_rsp = 0;
            }
        }
    }
    if (g_PbapPse.remaining_rsp != 0)
    {
        buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
        if (!buf)
        {
            return;
        }
        net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
        if ((g_PbapPse.lcl_supported_features & BT_PBAP_FEATURE_FOLDER_VERSION_COUNTERS) &&
            (g_PbapPse.rem_supported_features & BT_PBAP_FEATURE_FOLDER_VERSION_COUNTERS))
        {
            BT_PBAP_ADD_PARAMS_PRIMARY_FOLDER_VERSION(buf, sample_primay_folder_version);
            BT_PBAP_ADD_PARAMS_SECONDARY_FOLDER_VERSION(buf, sample_secondary_folder_version);
        }
        if ((g_PbapPse.lcl_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER) &&
            (g_PbapPse.rem_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER))
        {
            BT_PBAP_ADD_PARAMS_DATABASE_IDENTIFIER(buf, sample_database_identifier);
        }
        bt_pbap_pse_get_max_pkt_len(pbap_pse, &max_pkt_len);
        max_pkt_len -= sizeof(struct bt_pbap_push_response_hdr);
        max_pkt_len -= buf->len;
        max_pkt_len -= sizeof(struct bt_obex_hdr_bytes);
        if (g_PbapPse.remaining_rsp <= max_pkt_len)
        {
            body.value  = (uint8_t *)(uint8_t *)&pbap_pse_vcard_listing[g_PbapPse.send_rsp];
            body.length = g_PbapPse.remaining_rsp;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_END_OF_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp = 0;
            result                  = BT_PBAP_SUCCESS_RSP;
        }
        else
        {
            body.value  = (uint8_t *)&pbap_pse_vcard_listing[g_PbapPse.send_rsp];
            body.length = max_pkt_len;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp -= body.length;
            g_PbapPse.send_rsp += body.length;
            result = BT_PBAP_CONTINUE_RSP;
        }
    }
    PRINTF("send response : %x \r\n", result);
    revert = bt_pbap_pse_pull_vcard_listing_response(pbap_pse, result, buf, 0);
    if (revert != 0)
    {
        net_buf_unref(buf);
        PRINTF("Send Response failed %x", revert);
    }
}

static int app_check_pull_vcard_entry_path(char *name)
{
    uint8_t index = 1;
    if (name == NULL)
    {
        return -EINVAL;
    }
    if (endwith(name, ".vcf") == 0)
    {
        if (startwith(name, "X-BT-UID") == 0)
        {
            return -EINVAL;
        }
    }
    for (index = 1; index < 8; index++)
    {
        if (endwith(g_PbapPse.currentpath, child_floader_name[index]))
        {
            break;
        }
    }
    if (index >= 8)
    {
        return -EINVAL;
    }
    return 0;
}

static void app_pull_vcard_entry_cb(struct bt_pbap_pse *pbap_pse,
                                    struct net_buf *buf,
                                    char *name,
                                    enum bt_obex_req_flags flag)
{
    struct pbap_hdr body;
    int revert           = 0;
    uint8_t result       = BT_PBAP_SUCCESS_RSP;
    uint16_t max_pkt_len = 0;
    if (pbap_pse == NULL || buf == NULL)
    {
        return;
    }
    bt_pbap_pse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);
    if (name != NULL)
    {
        memcpy(g_PbapPse.name, name, BT_str_len(name) + 1);
    }
    if (!(flag & BT_OBEX_REQ_UNSEG)) /* stack do not support now */
    {
        g_PbapPse.remaining_rsp = 0;
        result                  = BT_PBAP_NOT_IMPLEMENTED_RSP;
    }
    else
    {
        if (g_PbapPse.remaining_rsp == 0)
        {
            if (app_check_pull_vcard_entry_path(g_PbapPse.name) == 0)
            {
                g_PbapPse.remaining_rsp = BT_str_len(pbap_pse_vcard_entry);
                if (g_PbapPse.remaining_rsp == 0)
                {
                    result = BT_PBAP_NOT_FOUND_RSP;
                }
                else
                {
                    g_PbapPse.send_rsp = 0;
                }
            }
            else
            {
                buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
                if (!buf)
                {
                    return;
                }
                net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
                result                  = BT_PBAP_NOT_FOUND_RSP;
                g_PbapPse.remaining_rsp = 0;
            }
        }
    }
    if (g_PbapPse.remaining_rsp != 0)
    {
        buf = net_buf_alloc(&pbap_appl_pool, osaWaitNone_c);
        if (!buf)
        {
            return;
        }
        net_buf_reserve(buf, BT_PBAP_PSE_RSV_LEN_SEND_RESPONSE(pbap_pse));
        if ((g_PbapPse.lcl_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER) &&
            (g_PbapPse.rem_supported_features & BT_PBAP_FEATURE_DATABASE_IDENTIFIER))
        {
            BT_PBAP_ADD_PARAMS_DATABASE_IDENTIFIER(buf, sample_database_identifier);
        }
        bt_pbap_pse_get_max_pkt_len(pbap_pse, &max_pkt_len);
        max_pkt_len -= sizeof(struct bt_pbap_push_response_hdr);
        max_pkt_len -= buf->len;
        max_pkt_len -= sizeof(struct bt_obex_hdr_bytes);
        if (g_PbapPse.remaining_rsp <= max_pkt_len)
        {
            body.value  = (uint8_t *)(uint8_t *)&pbap_pse_vcard_entry[g_PbapPse.send_rsp];
            body.length = g_PbapPse.remaining_rsp;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_END_OF_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp = 0;
            result                  = BT_PBAP_SUCCESS_RSP;
        }
        else
        {
            body.value  = (uint8_t *)&pbap_pse_vcard_entry[g_PbapPse.send_rsp];
            body.length = max_pkt_len;
            bt_obex_add_hdr(buf, BT_OBEX_HDR_BODY, body.value, body.length);
            g_PbapPse.remaining_rsp -= body.length;
            g_PbapPse.send_rsp += body.length;
            result = BT_PBAP_CONTINUE_RSP;
        }
    }

    PRINTF("send response : %x \r\n", result);
    revert = bt_pbap_pse_pull_vcard_entry_response(pbap_pse, result, buf, 0);
    if (revert != 0)
    {
        net_buf_unref(buf);
        PRINTF("Send Response failed %x", revert);
    }
}

static struct bt_pbap_pse_cb pse_cb = {
    .connected          = app_connected,
    .disconnected       = app_disconnected,
    .get_auth_info      = app_get_auth_info_cb,
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
    bt_sdp_register_service(&pbap_pse_rec);
    err = bt_pbap_pse_register(&pse_cb);
    if (err != 0)
    {
        PRINTF("PBAP register failed\r\n");
        return;
    }
}

void pbap_pse_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth PBAP PSE demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }
    vTaskDelete(NULL);
}
