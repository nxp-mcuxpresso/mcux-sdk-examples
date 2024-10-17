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
#include <bluetooth/rfcomm.h>
#include <bluetooth/map_mce.h>
#include <bluetooth/sdp.h>
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"
#include "app_map_mce.h"
#include "app_connect.h"
#include "app_discover.h"

#define MAP_MCE_CLASS_OF_DEVICE (0x10010CU) /* Object Transfer, Computer, Laptop */

#define MAP_MCE_MAS_TX_NET_BUF_COUNT   (1U)
#define MAP_MCE_MAS_TX_NET_BUF_SIZE    (1024U + 2U) /* L2CAP I-frame Enhanced Control Field(2-byte) */

static struct bt_sdp_attribute map_mce_attrs[] = {
    BT_SDP_NEW_SERVICE,
    /* ServiceClassIDList */
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_MAP_MCE_SVCLASS) //11 33
        },
        )
    ),
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
                BT_SDP_ARRAY_8(BT_RFCOMM_CHAN_MAP_MCE) //RFCOMM CHANNEL
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
                BT_SDP_ARRAY_16(BT_SDP_MAP_SVCLASS) //11 34
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0104U) //01 04
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("MAP MNS-name"),
    /* GoepL2CapPsm */
    BT_SDP_ATTR_GOEP_L2CAP_PSM,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT16), 
        BT_SDP_ARRAY_16(BT_BR_PSM_MAP_MCE)
    },
    /*  SupportedFeatures */
    BT_SDP_ATTR_MAP_SUPPORTED_FEATURES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT32), 
        BT_SDP_ARRAY_32(CONFIG_BT_MAP_MCE_MAS_SUPPORTED_FEATURES)
    },
};

static struct bt_sdp_record map_mce_rec = BT_SDP_RECORD(map_mce_attrs);

/* The following message is encoded by G-7bit including twenty "Bluetooth MAP Test!".
The message can be used for testing with Android phone and boards with MSE function.
The message may not delivered successfully by MSE(e.g. Android phone) because the 
phone number is 0000000000000. If wanting MSE to deliver this message successfully,
users need to modify the phone number(FN, N, TEL and 000000000000f0 in the message)
to the valid and modify the length(1080) accordingly.
For example, if users want to deliver the messsage to the phone number 123456,
FN, N and TEL shall be +123456, 000000000000f0 shall be 214365 and
the length(1080) shall be 1048(1080 - (14 - 6) * 4).
For example, If users want to deliver the messsage to the phone number 1234567,
FN, N and TEL shall be +1234567, 000000000000f0 shall be 214365f7 and
the length(1080) shall be 1056(1080 - (14 - 8) * 4). */
#define MAP_MCE_MSG_G_7BIT \
"BEGIN:BMSG\r\n\
VERSION:1.0\r\n\
STATUS:READ\r\n\
TYPE:SMS_GSM\r\n\
FOLDER:\r\n\
BEGIN:BENV\r\n\
BEGIN:VCARD\r\n\
VERSION:3.0\r\n\
FN:+0000000000000\r\n\
N:+0000000000000\r\n\
TEL:+0000000000000\r\n\
END:VCARD\r\n\
BEGIN:BBODY\r\n\
ENCODING:G-7BIT\r\n\
LENGTH:1080\r\n\
BEGIN:MSG\r\n\
0041000d91000000000000f00000a0050003080401622e90905d2fd3df6f3a1ad40c4241d4f29c1e52c85c2021bb5ea6bfdf7434a8198482a8e5393da498b9404276bd4c7fbfe9685033080551cb737a4841738184ec7a99fe7ed3d1a066100aa296e7f490a2e60209d9f532fdfda6a341cd2014442dcfe92185cd0512b2eb65fafb4d47839a4128885a9ed3438a9b0b2464d7cbf4f79b8e063583\r\n\
END:MSG\r\n\
BEGIN:MSG\r\n\
0041000d91000000000000f00000a0050003080402a0206a794e0f29702e90905d2fd3df6f3a1ad40c4241d4f29c1e52e45c2021bb5ea6bfdf7434a8198482a8e5393da488c15c2021bb5ea6bfdf7434a8198482a8e5393da488c55c2021bb5ea6bfdf7434a8198482a8e5393da488c95c2021bb5ea6bfdf7434a8198482a8e5393da488cd5c2021bb5ea6bfdf7434a8198482a8e5393da488d15c\r\n\
END:MSG\r\n\
BEGIN:MSG\r\n\
0041000d91000000000000f00000a0050003080403404276bd4c7fbfe9685033080551cb737a4811abb9404276bd4c7fbfe9685033080551cb737a4811b3b9404276bd4c7fbfe9685033080551cb737a4811bbb9404276bd4c7fbfe9685033080551cb737a4811c3b9404276bd4c7fbfe9685033080551cb737a4811cbb9404276bd4c7fbfe9685033080551cb737a482183b9404276bd4c7fbfe9\r\n\
END:MSG\r\n\
BEGIN:MSG\r\n\
0041000d91000000000000f0000012050003080404d0a066100aa296e7f410\r\n\
END:MSG\r\n\
END:BBODY\r\n\
END:BENV\r\n\
END:BMSG"

/* The following message is encoded by UTF-8 including twenty "Bluetooth MAP Test!".
The message can be used for testing with iPhone and boards with MSE function.
The message may not delivered successfully by MSE(e.g. iPhone) because the 
phone number is 0000000000000. If wanting MSE to deliver this message successfully,
users need to modify the phone number(FN, N, TEL).
For example, if users want to deliver the messsage to the phone number 123456,
FN, N and TEL shall be +123456. */
#define MAP_MCE_MSG_UTF_8 \
"BEGIN:BMSG\r\n\
VERSION:1.0\r\n\
STATUS:UNREAD\r\n\
TYPE:SMS_GSM\r\n\
FOLDER:\r\n\
BEGIN:VCARD\r\n\
VERSION:2.1\r\n\
N;CHARSET=UTF-8:\r\n\
TEL;CHARSET=UTF-8:\r\n\
END:VCARD\r\n\
BEGIN:BENV\r\n\
BEGIN:VCARD\r\n\
VERSION:2.1\r\n\
FN;CHARSET=UTF-8:+0000000000000\r\n\
N;CHARSET=UTF-8:+0000000000000\r\n\
TEL:+0000000000000\r\n\
END:VCARD\r\n\
BEGIN:BBODY\r\n\
CHARSET:UTF-8\r\n\
LANGUAGE:UNKNOWN\r\n\
LENGTH:492\r\n\
BEGIN:MSG\r\n\
1. Bluetooth MAP Test!\n\
2. Bluetooth MAP Test!\n\
3. Bluetooth MAP Test!\n\
4. Bluetooth MAP Test!\n\
5. Bluetooth MAP Test!\n\
6. Bluetooth MAP Test!\n\
7. Bluetooth MAP Test!\n\
8. Bluetooth MAP Test!\n\
9. Bluetooth MAP Test!\n\
10. Bluetooth MAP Test!\n\
11. Bluetooth MAP Test!\n\
12. Bluetooth MAP Test!\n\
13. Bluetooth MAP Test!\n\
14. Bluetooth MAP Test!\n\
15. Bluetooth MAP Test!\n\
16. Bluetooth MAP Test!\n\
17. Bluetooth MAP Test!\n\
18. Bluetooth MAP Test!\n\
19. Bluetooth MAP Test!\n\
20. Bluetooth MAP Test!\r\n\
END:MSG\r\n\
END:BBODY\r\n\
END:BENV\r\n\
END:BMSG"

struct map_hdr
{
    uint8_t *value;
    uint16_t length;
};

static void map_mce_mns_connected(struct bt_map_mce_mns *mce_mns);
static void map_mce_mns_disconnected(struct bt_map_mce_mns *mce_mns, uint8_t result);
static void app_send_event_cb(struct bt_map_mce_mns *mce_mns, struct net_buf *buf, enum bt_obex_req_flags flag);
static void map_mce_mas_connected(struct bt_map_mce_mas *mce_mas);
static void map_mce_mas_disconnected(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_abort_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_get_folder_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_set_folder_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_get_msg_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_get_msg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_set_msg_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_push_msg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, char *name);
static void app_set_ntf_reg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_update_inbox_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_get_mas_inst_info_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_set_owner_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_get_owner_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_get_convo_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf);
static void app_set_ntf_filter_cb(struct bt_map_mce_mas *mce_mas, uint8_t result);
static void app_state_machine(void);

struct app_map_mce_instance app_instance;
NET_BUF_POOL_FIXED_DEFINE(mce_mas_tx_pool, MAP_MCE_MAS_TX_NET_BUF_COUNT, BT_L2CAP_BUF_SIZE(MAP_MCE_MAS_TX_NET_BUF_SIZE), NULL);
static const char map_msg_example[] = MAP_MCE_MSG_G_7BIT;

struct bt_map_mce_mas_cb map_mas_cb = {
    .connected = map_mce_mas_connected,
    .disconnected = map_mce_mas_disconnected,
    .abort = app_abort_cb,
    .get_folder_listing = app_get_folder_listing_cb,
    .set_folder = app_set_folder_cb,
    .get_msg_listing = app_get_msg_listing_cb,
    .get_msg = app_get_msg_cb,
    .set_msg_status = app_set_msg_status_cb,
    .push_msg = app_push_msg_cb,
    .set_ntf_reg = app_set_ntf_reg_cb,
    .update_inbox = app_update_inbox_cb,
    .get_mas_inst_info = app_get_mas_inst_info_cb,
    .set_owner_status = app_set_owner_status_cb,
    .get_owner_status = app_get_owner_status_cb,
    .get_convo_listing = app_get_convo_listing_cb,
    .set_ntf_filter = app_set_ntf_filter_cb,
};

struct bt_map_mce_mns_cb map_mns_cb = {
    .connected = map_mce_mns_connected,
    .disconnected = map_mce_mns_disconnected,
    .send_event = app_send_event_cb,
};

static uint8_t *app_strnstr(const uint8_t *haystack, size_t haystack_len, const char *needle)
{
    size_t index = 0;
    size_t needle_len = strlen(needle);

    if (haystack_len < needle_len)
    {
        return NULL;
    }

    do
    {
        if (memcmp(&haystack[index], needle, needle_len) == 0)
        {
            return (uint8_t *)&haystack[index + needle_len];
        }
        index++;
    } while(index < (haystack_len - needle_len));

    return NULL;
}

static void app_print_body(struct net_buf *buf)
{
    struct map_hdr body;
    if (bt_map_mce_get_body(buf, &body.value, &body.length) == 0)
    {
        PRINTF ("============== BODY ==============\r\n");
        PRINTF("%.*s\r\n", body.length, body.value);
        PRINTF ("============ END BODY ============\r\n");
    }
    else
    {
        PRINTF ("BODY not Found \r\n");
    }
}

static bool app_app_param_cb(struct bt_data *data, void *user_data)
{
    switch (data->type)
    {
        case BT_MAP_TAG_ID_NEW_MESSAGE:
            PRINTF ("New Message - %d\r\n",  data->data[0]);
            break;

        case BT_MAP_TAG_ID_MAS_INSTANCE_ID:
            PRINTF ("MAS Instance ID - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_FOLDER_LISTING_SIZE:
            if (data->data_len < 2U)
            {
                return false;
            }
            PRINTF ("Folder Listing Size - %d\r\n", sys_get_be16(data->data));
            break;

        case BT_MAP_TAG_ID_LISTING_SIZE:
            if (data->data_len < 2U)
            {
                return false;
            }
            PRINTF ("Listing Size - %d\r\n", sys_get_be16(data->data));
            break;

        case BT_MAP_TAG_ID_FRACTION_DELIVER:
            PRINTF ("Fraction Deliver - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_MSE_TIME:
            PRINTF ("MSE Time - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_DATABASE_IDENTIFIER:
            PRINTF ("Database Identifier - ");
            for (uint8_t index = 0; index < data->data_len; index++)
            {
                PRINTF("%02X", data->data[index]);
            }
            PRINTF("\r\n");
            break;

        case BT_MAP_TAG_ID_CONV_LIST_VER_CNTR:
            PRINTF("Conversation Listing Version Counter - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_PRESENCE_AVAILABILITY:
            PRINTF("Presence Availability - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_PRESENCE_TEXT:
            PRINTF ("============== Presence Text ==============\r\n");
            PRINTF("%.*s\r\n", data->data_len, data->data);
            PRINTF ("============ END Presence Text ============\r\n");
            break;

        case BT_MAP_TAG_ID_LAST_ACTIVITY:
            PRINTF("Last Activity - %.*s\r\n", data->data_len, data->data);
            break;
 
        case BT_MAP_TAG_ID_CHAT_STATE:
            PRINTF("Chat State - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_FOLDER_VER_CNTR:
            PRINTF ("Folder Version Counter - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_OWNER_UCI:
            PRINTF ("============== Owner UCI ==============\r\n");
            PRINTF("%.*s\r\n", data->data_len, data->data);
            PRINTF ("============ END Owner UCI ============\r\n");
            break;

        case BT_MAP_TAG_ID_MAP_SUPPORTED_FEATURES:
            if (data->data_len < 4U)
            {
                return false;
            }
            PRINTF ("Supported Features - %08X\r\n", sys_get_be32(data->data));
            break;

        default:
            break;
    }

    return true;
}

static void map_mce_mns_connected(struct bt_map_mce_mns *mce_mns)
{
    app_instance.mce_mns = mce_mns;
    PRINTF("MCE MNS connection\r\n");
    if (bt_map_mce_mns_get_max_pkt_len(mce_mns, &app_instance.mns_max_pkt_len) == 0)
    {
        PRINTF("MAX Packet Length - %d\r\n", app_instance.mns_max_pkt_len);
    }
    else
    {
        PRINTF("MAX Packet Length is invalid\r\n");
    }
    app_state_machine();
}

static void map_mce_mns_disconnected(struct bt_map_mce_mns *mce_mns, uint8_t result)
{
    app_instance.mce_mns = NULL;
    PRINTF("MCE MNS disconnection - 0x%02X\r\n", result);
    app_state_machine();
}

static void map_mce_mas_connected(struct bt_map_mce_mas *mce_mas)
{
    app_instance.mce_mas = mce_mas;
    PRINTF("MCE MAS connection\r\n");
    if (bt_map_mce_get_max_pkt_len(mce_mas, &app_instance.max_pkt_len) == 0)
    {
        PRINTF("MAX Packet Length - %d\r\n", app_instance.max_pkt_len);
    }
    else
    {
        PRINTF("MAX Packet Length is invalid\r\n");
    }

    if (app_instance.supported_features & BT_MAP_BROWSING_FEATURE)
    {
        app_instance.state = GET_FOLDER_LISTING_ROOT;
        app_state_machine();
    }
    else
    {
        PRINTF("Browsing feature is not supported");
    }
}

static void map_mce_mas_disconnected(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    app_instance.mce_mas = NULL;
    PRINTF("MCE MAS disconnection - 0x%02X\r\n", result);
}

void app_mce_disconnect(void)
{
    PRINTF("MAP MCE MAS Disconnect\r\n");
    if (bt_map_mce_disconnect(app_instance.mce_mas) < 0)
    {
        PRINTF("Failed to disconnect\r\n");
    }
}

void app_mce_connect(void)
{
    int res;

    PRINTF("MAP MCE Connect\r\n");
    if (app_instance.goep_version >= BT_GOEP_VERSION_2_0)
    {
        res = bt_map_mce_psm_connect(app_instance.acl_conn, app_instance.psm, app_instance.supported_features, &app_instance.mce_mas);
    }
    else
    {
        res = bt_map_mce_scn_connect(app_instance.acl_conn, app_instance.scn, app_instance.supported_features, &app_instance.mce_mas);
    }

    if (res < 0)
    {
        PRINTF("fail to connect MSE (err: %d)\r\n", res);
    }
}

void app_mce_abort(void)
{
    PRINTF("MAP Abort\r\n");
    int err = bt_map_mce_abort(app_instance.mce_mas);

    if (err == -EINPROGRESS)
    {
        PRINTF("Abort is pending\r\n");
    }
    else if (err == 0)
    {
        /* nothing to do */
    }
    else
    {
        PRINTF("Failed to abort\r\n");
    }
}

static void app_abort_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF("MCE Abort - 0x%02X\r\n", result);
}

void app_mce_mns_disconnect(void)
{
    PRINTF("MAP MCE MNS Disconnect\r\n");
    if (bt_map_mce_mns_disconnect(app_instance.mce_mns) < 0)
    {
        PRINTF("Failed to close MNS transport\r\n");
    }
}

static void app_send_event_cb(struct bt_map_mce_mns *mce_mns, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;
    PRINTF ("MAP Recv Send Event\r\n");
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (flag & BT_OBEX_REQ_END)
    {
        result = BT_MAP_RSP_SUCCESS;
    }
    else
    {
        result = BT_MAP_RSP_CONTINUE;
    }
    if (bt_map_mce_send_event_response(mce_mns, result, false) < 0)
    {
        PRINTF ("Failed to send event response\r\n");
    }
}

void app_get_folder_listing(uint8_t num_srmp_wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get Folder Listing\r\n");
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_FOLDER_LISTING(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    app_instance.num_srmp_wait = num_srmp_wait;
    if (bt_map_mce_get_folder_listing(app_instance.mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get folder listing\r\n");
    }
}

static void app_get_folder_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    PRINTF ("MAP Get Folder Listing CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_FOLDER_LISTING(app_instance.mce_mas, BT_OBEX_REQ_END));
        if (bt_map_mce_get_folder_listing(mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get folder listing\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_set_folder(char *name)
{
    PRINTF("MAP Set Folder\r\n");
    if (name != NULL)
    {
        PRINTF("Name - %s\r\n", name);
    }
    if (bt_map_mce_set_folder(app_instance.mce_mas, name) != 0)
    {
        PRINTF ("Failed to set folder\r\n");
    }
}

static void app_set_folder_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Set Folder CNF - 0x%02X\r\n", result);
    if (result == BT_MAP_RSP_SUCCESS)
    {
        app_state_machine();
    }
}

void app_get_msg_listing(char *name, uint16_t max_list_cnt, uint8_t wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get MSG Listing\r\n");
    PRINTF("MAX List Count - %d\r\n", max_list_cnt);
    PRINTF("SRMP Wait Count - %d\r\n", wait);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MSG_LISTING(app_instance.mce_mas, name, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_MAX_LIST_COUNT(buf, max_list_cnt);
    app_instance.num_srmp_wait = wait;
    if (bt_map_mce_get_msg_listing(app_instance.mce_mas, buf, name, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get msg listing\r\n");
    }
}

static void app_get_msg_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    struct map_hdr body;
    uint8_t *msg_handle;
    uint8_t index;

    PRINTF ("MAP Get MSG Listing CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    if (bt_map_mce_get_body(buf, &body.value, &body.length) == 0)
    {
        PRINTF ("============== BODY ==============\r\n");
        PRINTF("%.*s\r\n", body.length, body.value);
        PRINTF ("============ END BODY ============\r\n");
        if (app_instance.msg_handle[0] == '\0')
        {
            msg_handle = app_strnstr(body.value, body.length, "msg handle = \"");
            if (msg_handle == NULL)
            {
                msg_handle = app_strnstr(body.value, body.length, "msg handle=\"");
            }
            if (msg_handle != NULL)
            {
                for (index = 0; index < BT_MAP_MSG_HANDLE_SIZE / 2U - 1U; index++)
                {
                    if (msg_handle[index] == '"')
                    {
                        break;
                    }
                    app_instance.msg_handle[index] = (char)msg_handle[index];
                }
                app_instance.msg_handle[index] = '\0';
            }
        }
    }
    else
    {
        PRINTF ("BODY not Found \r\n");
    }
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MSG_LISTING(app_instance.mce_mas, NULL, BT_OBEX_REQ_END));
        if (bt_map_mce_get_msg_listing(mce_mas, buf, NULL, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get msg listing\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_get_msg(char *name, bool attachment, bool charset, uint8_t wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get MSG\r\n");
    PRINTF("Name - %s\r\n", name);
    PRINTF("Attachment - %d\r\n", attachment);
    PRINTF("Charset - %d\r\n", charset);
    PRINTF("SRMP Wait Count - %d\r\n", wait);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MSG(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_ATTACHMENT(buf, (uint8_t)attachment);
    BT_MAP_ADD_CHARSET(buf, (uint8_t)charset);
    app_instance.num_srmp_wait = wait;
    if (bt_map_mce_get_msg(app_instance.mce_mas, buf, name, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get msg\r\n");
    }
}

static void app_get_msg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    PRINTF ("MAP Get MSG CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MSG(app_instance.mce_mas, BT_OBEX_REQ_END));
        if (bt_map_mce_get_msg(mce_mas, buf, NULL, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get msg\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_set_msg_status(char *name, uint8_t status_ind, uint8_t status_val)
{
    struct net_buf *buf;

    PRINTF("MAP Set MSG Status\r\n");
    PRINTF("Name - %s\r\n", name);
    PRINTF("Status Indicator - %d\r\n", status_ind);
    PRINTF("Status Value - %d\r\n", status_val);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_SET_MSG_STATUS(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_STATUS_INDICATOR(buf, status_ind);
    BT_MAP_ADD_STATUS_VALUE(buf, status_val);

    if (bt_map_mce_set_msg_status(app_instance.mce_mas, buf, name, BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to set msg status\r\n");
    }
}

static void app_set_msg_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Set MSG Status CNF - 0x%02X\r\n", result);
    app_state_machine();
    /* If the result is BT_MAP_RSP_CONTINUE and the message is not sent completely,
        call bt_map_mce_set_msg_status to continue to send */
}

void app_push_msg(char *name, bool charset)
{
    struct net_buf *buf;
    enum bt_obex_req_flags flags = BT_OBEX_REQ_UNSEG;
    uint16_t max_body_len;
    uint16_t actual;

    PRINTF("MAP Push MSG\r\n");
    if (name != NULL)
    {
        PRINTF("Name - %s\r\n", name);
    }
    PRINTF("Charset - %d\r\n", charset);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_PUSH_MSG(app_instance.mce_mas, name, flags));
    BT_MAP_ADD_CHARSET(buf, (uint8_t)charset);
    max_body_len = app_instance.max_pkt_len;
    max_body_len -= BT_MAP_MCE_RSV_LEN_PUSH_MSG(app_instance.mce_mas, name, flags) - BT_L2CAP_BUF_SIZE(2U);
    max_body_len -= buf->len; /* application parameters */
    max_body_len -= sizeof(struct bt_obex_hdr_bytes);

    actual = strlen(map_msg_example);
    if (actual > max_body_len)
    {
        actual = max_body_len;
        flags = BT_OBEX_REQ_START;
    }

    if (flags == BT_OBEX_REQ_START)
    {
        BT_MAP_ADD_BODY(buf, (uint8_t *)map_msg_example, actual);
    }
    else
    {
        BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)map_msg_example, actual);
    }
    if (bt_map_mce_push_msg(app_instance.mce_mas, buf, name, flags) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to push msg\r\n");
    }
    else
    {
        app_instance.tx_cnt = actual;
    }
}

static void app_push_msg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, char *name)
{
    struct net_buf *buf;
    uint16_t actual;
    enum bt_obex_req_flags flags = BT_OBEX_REQ_END;
    uint16_t max_body_len;

    PRINTF ("MAP Push MSG CNF - 0x%02X\r\n", result);
    if (name != NULL)
    {
        PRINTF ("Name - %.*s\r\n", BT_MAP_MSG_HANDLE_SIZE / 2U, name);
    }

    if (result == BT_MAP_RSP_CONTINUE)
    {
        if (app_instance.tx_cnt < strlen(map_msg_example))
        {
            buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
            net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_PUSH_MSG(app_instance.mce_mas, NULL, flags));
            max_body_len = app_instance.max_pkt_len;
            max_body_len -= BT_MAP_MCE_RSV_LEN_PUSH_MSG(app_instance.mce_mas, NULL, flags) - BT_L2CAP_BUF_SIZE(2U);
            max_body_len -= sizeof(struct bt_obex_hdr_bytes);
            actual = strlen(map_msg_example) - app_instance.tx_cnt;
            if (actual > max_body_len)
            {
                actual = max_body_len;
                flags = BT_OBEX_REQ_CONTINUE;
            }
            if (flags == BT_OBEX_REQ_CONTINUE)
            {
                BT_MAP_ADD_BODY(buf, (uint8_t *)(map_msg_example + app_instance.tx_cnt), actual);
            }
            else
            {
                BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)(map_msg_example + app_instance.tx_cnt), actual);
            }
            if (bt_map_mce_push_msg(mce_mas, buf, NULL, flags) != 0)
            {
                net_buf_unref(buf);
                PRINTF ("Failed to push msg\r\n");
            }
            else
            {
                app_instance.tx_cnt += actual;
            }
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_set_ntf_reg(bool ntf_status)
{
    struct net_buf *buf;

    PRINTF("MAP Set Notification Registration\r\n");
    PRINTF("Notification Status - %d\r\n", ntf_status);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_SET_NTF_REG(app_instance.mce_mas));
    BT_MAP_ADD_NOTIFICATION_STATUS(buf, ntf_status ? 1 : 0);

    if (bt_map_mce_set_ntf_reg(app_instance.mce_mas, buf) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to set notification registration\r\n");
    }
}

static void app_set_ntf_reg_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Set Notification Registration CNF - 0x%02X\r\n", result);
}

void app_update_inbox(void)
{
    PRINTF("MAP Update Inbox\r\n");

    if (bt_map_mce_update_inbox(app_instance.mce_mas) != 0)
    {
        PRINTF ("Failed to udpate inbox\r\n");
    }
}

static void app_update_inbox_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Update Inbox CNF - 0x%02X\r\n", result);
    app_state_machine();
}

void app_get_mas_inst_info(uint8_t mas_inst_id, uint8_t wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get MAS Instance Info\r\n");
    PRINTF("MAS Instance ID - %d\r\n", mas_inst_id);
    PRINTF("SRMP Wait Count - %d\r\n", wait);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MAS_INST_INFO(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_MAS_INSTANCE_ID(buf, mas_inst_id);
    app_instance.num_srmp_wait = wait;
    if (bt_map_mce_get_mas_inst_info(app_instance.mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get MAS instance infomation\r\n");
    }
}

static void app_get_mas_inst_info_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    PRINTF ("MAP Get MAS Instance Info CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_MAS_INST_INFO(app_instance.mce_mas, BT_OBEX_REQ_END));
        if (bt_map_mce_get_mas_inst_info(mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get MAS instance infomation\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_set_owner_status(uint8_t chat_state)
{
    struct net_buf *buf;

    PRINTF("MAP Set Owner Status\r\n");
    PRINTF("Chat State - %d\r\n", chat_state);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_SET_OWNER_STATUS(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_CHAT_STATE(buf, chat_state);
    if (bt_map_mce_set_owner_status(app_instance.mce_mas, buf, BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to set owner status\r\n");
    }
}

static void app_set_owner_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Set Owner Status CNF - 0x%02X\r\n", result);
    app_state_machine();
}

void app_get_owner_status(uint8_t wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get Owner Status\r\n");
    PRINTF("SRMP Wait Count - %d\r\n", wait);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_OWNER_STATUS(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    app_instance.num_srmp_wait = wait;
    if (bt_map_mce_get_owner_status(app_instance.mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get owner status\r\n");
    }
}

static void app_get_owner_status_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    PRINTF ("MAP Get Owner Status CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_OWNER_STATUS(app_instance.mce_mas, BT_OBEX_REQ_END));
        if (bt_map_mce_get_owner_status(mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get owner status\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_get_convo_listing(uint16_t max_list_cnt, uint8_t wait)
{
    struct net_buf *buf;

    PRINTF("MAP Get Conversation Listing\r\n");
    PRINTF("MAX List Count - %d\r\n", max_list_cnt);
    PRINTF("SRMP Wait Count - %d\r\n", wait);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_CONVO_LISTING(app_instance.mce_mas, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_MAX_LIST_COUNT(buf, max_list_cnt);
    app_instance.num_srmp_wait = wait;
    if (bt_map_mce_get_convo_listing(app_instance.mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_UNSEG) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to get conversation listing\r\n");
    }
}

static void app_get_convo_listing_cb(struct bt_map_mce_mas *mce_mas, uint8_t result, struct net_buf *buf)
{
    PRINTF ("MAP Get Conversation Listing CNF - 0x%02X\r\n", result);
    bt_map_mce_app_param_parse(buf, app_app_param_cb, NULL);
    app_print_body(buf);
    net_buf_unref(buf);

    if (result == BT_MAP_RSP_CONTINUE)
    {
        app_instance.num_srmp_wait = app_instance.num_srmp_wait ? app_instance.num_srmp_wait - 1 : 0;
        buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_GET_CONVO_LISTING(app_instance.mce_mas, BT_OBEX_REQ_END));
        if (bt_map_mce_get_convo_listing(mce_mas, buf, (app_instance.num_srmp_wait ? true : false), BT_OBEX_REQ_END) != 0)
        {
            net_buf_unref(buf);
            PRINTF ("Failed to get conversation listing\r\n");
        }
    }
    else
    {
        app_state_machine();
    }
}

void app_set_ntf_filter(uint32_t ntf_filter_mask)
{
    struct net_buf *buf;

    PRINTF("MAP Set Notification Filter\r\n");
    PRINTF("Notification Filter Mask - %d\r\n", ntf_filter_mask);
    buf = net_buf_alloc(&mce_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MCE_RSV_LEN_SET_NTF_FILTER(app_instance.mce_mas));
    BT_MAP_ADD_NOTIFICATION_FILTER_MASK(buf, ntf_filter_mask);

    if (bt_map_mce_set_ntf_filter(app_instance.mce_mas, buf) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to set notification filter\r\n");
    }
}

static void app_set_ntf_filter_cb(struct bt_map_mce_mas *mce_mas, uint8_t result)
{
    PRINTF ("MAP Set NTF Filter CNF - 0x%02X\r\n", result);
    app_state_machine();
}

static void app_check_supported_features(void)
{
    uint8_t *state = &app_instance.state;

    if ((*state == WAIT_GET_MSG_LISTING) || (*state == WAIT_SET_MSG_STATUS))
    {
        if (app_instance.supported_features & BT_MAP_CONVO_LISTING)
        {
            *state = GET_CONVO_LISTING;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_CONVO_LISTING is not supported\r\n", GET_CONVO_LISTING, WAIT_GET_CONVO_LISTING);
        }
    }

    if ((*state == WAIT_GET_MSG_LISTING) || (*state == WAIT_SET_MSG_STATUS) || (*state == WAIT_GET_CONVO_LISTING))
    {
        if (app_instance.supported_features & BT_MAP_INST_INFO_FEATURE)
        {
            *state = GET_MAS_INST_INFO;
            return;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_INST_INFO_FEATURE is not supported\r\n", GET_MAS_INST_INFO, WAIT_GET_MAS_INST_INFO);
        }
    }

    if ((*state == WAIT_GET_MSG_LISTING) || (*state == WAIT_SET_MSG_STATUS) || (*state == WAIT_GET_CONVO_LISTING) ||
        (*state == WAIT_GET_MAS_INST_INFO))
    {
        if (app_instance.supported_features & BT_MAP_NTF_FILTERING)
        {
            *state = SET_NTF_FILTER;
            return;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_NTF_FILTERING is not supported\r\n", SET_NTF_FILTER, WAIT_SET_NTF_FILTER);
        }
    }

    if ((*state == WAIT_GET_MSG_LISTING) || (*state == WAIT_SET_MSG_STATUS) || (*state == WAIT_GET_CONVO_LISTING) ||
        (*state == WAIT_GET_MAS_INST_INFO) || (*state == WAIT_SET_NTF_FILTER))
    {
        if (app_instance.supported_features & BT_MAP_NTF_REG_FEATURE)
        {
            *state = SET_NTF_REG_ON;
            return;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_NTF_REG_FEATURE is not supported\r\n", SET_NTF_REG_ON, WAIT_SET_NTF_REG_OFF);
        }
    }

    if ((*state == WAIT_GET_MSG_LISTING) || (*state == WAIT_SET_MSG_STATUS) || (*state == WAIT_GET_CONVO_LISTING) ||
        (*state == WAIT_GET_MAS_INST_INFO) || (*state == WAIT_SET_NTF_FILTER) || (*state == WAIT_SET_NTF_REG_OFF))
    {
        if ((app_instance.supported_features & BT_MAP_OWNER_STATUS) && (app_instance.supported_features & BT_MAP_UPLOADING_FEATURE))
        {
            *state = GET_OWNER_STATUS;
            return;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_OWNER_STATUS is not supported\r\n", GET_OWNER_STATUS, WAIT_SET_OWNER_STATUS);
        }
        if (app_instance.supported_features & BT_MAP_UPLOADING_FEATURE)
        {
            *state = SET_FOLDER_PARENT;
            return;
        }
        else
        {
            PRINTF("[%d] ~ [%d] Skip, BT_MAP_UPLOADING_FEATURE is not supported\r\n", SET_FOLDER_PARENT, WAIT_PUSH_MSG);
        }
        *state = MCE_MAS_DISCONNECT;
    }
}


static void app_state_machine(void)
{
    uint8_t *state = &app_instance.state;

    switch (*state)
    {
        /* Browsing Feature */
        case WAIT_GET_FOLDER_LISTING_ROOT:
            PRINTF("[%d]: GET_FOLDER_LISTING_ROOT Complete\r\n", *state);
            *state = SET_FOLDER_TELECOM;
            break;
        case WAIT_SET_FOLDER_TELECOM:
            PRINTF("[%d]: SET_FOLDER_TELECOM Complete\r\n", *state);
            *state = SET_FOLDER_MSG;
            break;
        case WAIT_SET_FOLDER_MSG:
            PRINTF("[%d]: SET_FOLDER_MSG Complete\r\n", *state);
            *state = SET_FOLDER_INBOX;
            break;
        case WAIT_SET_FOLDER_INBOX:
            PRINTF("[%d]: SET_FOLDER_INBOX Complete\r\n", *state);
            *state = UPDATE_INBOX;
            break;
        case WAIT_UPDATE_INBOX:
            PRINTF("[%d]: UPDATE_INBOX Complete\r\n", *state);
            *state = GET_MSG_LISTING;
            break;
        case WAIT_GET_MSG_LISTING:
            PRINTF("[%d]: GET_MSG_LISTING Complete\r\n", *state);
            if (app_instance.msg_handle[0] != '\0')
            {
                *state = GET_MSG;
            }
            else
            {
                PRINTF("MSG handle not found in GET_MSG_LISTING\r\n");
                PRINTF("[%d] ~ [%d] Skip\r\n", GET_MSG, WAIT_SET_MSG_STATUS);
                app_check_supported_features();
            }
            break;
        case WAIT_GET_MSG:
            PRINTF("[%d]: GET_MSG Complete\r\n", *state);
            *state = SET_MSG_STATUS;
            break;
        case WAIT_SET_MSG_STATUS:
            PRINTF("[%d]: SET_MSG_STATUS Complete\r\n", *state);
            app_check_supported_features();
            break;
        case WAIT_GET_CONVO_LISTING:
            PRINTF("[%d]: GET_CONVO_LISTING Complete\r\n", *state);
            app_check_supported_features();
            break;

        /* Instance Information Feature */
        case WAIT_GET_MAS_INST_INFO:
            PRINTF("[%d]: GET_MAS_INST_INFO Complete\r\n", *state);
            app_check_supported_features();
            break;

        /* Notification Feature */
        case WAIT_SET_NTF_FILTER:
            PRINTF("[%d]: SET_NTF_FILTER Complete\r\n", *state);
            app_check_supported_features();
            break;

        /* Notification Registration Feature */
        case WAIT_SET_NTF_REG_ON:
            PRINTF("[%d]: SET_NTF_REG_ON Complete\r\n", *state);
            *state = SET_NTF_REG_OFF;
            break;
        case WAIT_SET_NTF_REG_OFF:
            PRINTF("[%d]: SET_NTF_REG_OFF Complete\r\n", *state);
            app_check_supported_features();
            break;

        /* Uploading Feature */
        case WAIT_GET_OWNER_STATUS:
            PRINTF("[%d]: GET_OWNER_STATUS Complete\r\n", *state);
            *state = SET_OWNER_STATUS;
            break;
        case WAIT_SET_OWNER_STATUS:
            PRINTF("[%d]: SET_OWNER_STATUS Complete\r\n", *state);
            *state = SET_FOLDER_PARENT;
            break;
        case WAIT_SET_FOLDER_PARENT:
            PRINTF("[%d]: SET_FOLDER_PARENT Complete\r\n", *state);
            *state = SET_FOLDER_OUTBOX;
            break;
        case WAIT_SET_FOLDER_OUTBOX:
            PRINTF("[%d]: SET_FOLDER_OUTBOX Complete\r\n", *state);
            *state = PUSH_MSG;
            break;
        case WAIT_PUSH_MSG:
            PRINTF("[%d]: PUSH_MSG Complete\r\n", *state);
            *state = MCE_MAS_DISCONNECT;
            break;
        case WAIT_MCE_MAS_DISCONNECT:
            PRINTF("[%d]: MCE_MAS_DISCONNECT Complete\r\n", *state);
            *state = MAP_NOT_STARTED;
        default:
            break;
    }

    switch (*state)
    {
        /* Browsing Feature */
        case GET_FOLDER_LISTING_ROOT:
            PRINTF("[%d]: GET_FOLDER_LISTING_ROOT\r\n", *state);
            app_get_folder_listing(0);
            *state = WAIT_GET_FOLDER_LISTING_ROOT;
            break;
        case SET_FOLDER_TELECOM:
            PRINTF("[%d]: SET_FOLDER_TELECOM\r\n", *state);
            app_set_folder("telecom");
            *state = WAIT_SET_FOLDER_TELECOM;
            break;
        case SET_FOLDER_MSG:
            PRINTF("[%d]: SET_FOLDER_MSG\r\n", *state);
            app_set_folder("msg");
            *state = WAIT_SET_FOLDER_MSG;
            break;
        case SET_FOLDER_INBOX:
            PRINTF("[%d]: SET_FOLDER_INBOX\r\n", *state);
            app_set_folder("inbox");
            *state = WAIT_SET_FOLDER_INBOX;
            break;
        case UPDATE_INBOX:
            PRINTF("[%d]: UPDATE_INBOX\r\n", *state);
            app_update_inbox();
            *state = WAIT_UPDATE_INBOX;
            break;
        case GET_MSG_LISTING:
            PRINTF("[%d]: GET_MSG_LISTING\r\n", *state);
            memset(app_instance.msg_handle, 0U, BT_MAP_MSG_HANDLE_SIZE / 2);
            app_get_msg_listing(NULL, 10, 0);
            *state = WAIT_GET_MSG_LISTING;
            break;
        case GET_MSG:
            PRINTF("[%d]: GET_MSG\r\n", *state);
            app_get_msg(app_instance.msg_handle, false, false, 0);
            *state = WAIT_GET_MSG;
            break;
        case SET_MSG_STATUS:
            PRINTF("[%d]: SET_MSG_STATUS\r\n", *state);
            app_set_msg_status(app_instance.msg_handle, 0, 0);
            *state = WAIT_SET_MSG_STATUS;
            break;
        case GET_CONVO_LISTING:
            PRINTF("[%d]: GET_CONVO_LISTING\r\n", *state);
            app_get_convo_listing(10, 0);
            *state = WAIT_GET_CONVO_LISTING;
            break;

        /* Instance Information Feature */
        case GET_MAS_INST_INFO:
            PRINTF("[%d]: GET_MAS_INST_INFO\r\n", *state);
            app_get_mas_inst_info(app_instance.mas_instance_id, 0);
            *state = WAIT_GET_MAS_INST_INFO;
            break;
        /* Notification Feature */
        case SET_NTF_FILTER:
            PRINTF("[%d]: SET_NTF_FILTER\r\n", *state);
            app_set_ntf_filter(0);
            *state = WAIT_SET_NTF_FILTER;
            break;

        /* Notification Registration Feature */
        case SET_NTF_REG_ON:
            PRINTF("[%d]: SET_NTF_REG_ON\r\n", *state);
            app_set_ntf_reg(true);
            *state = WAIT_SET_NTF_REG_ON;
            break;
        case SET_NTF_REG_OFF:
            PRINTF("[%d]: SET_NTF_REG_OFF\r\n", *state);
            app_set_ntf_reg(false);
            *state = WAIT_SET_NTF_REG_OFF;
            break;

        /* Uploading Feature */
        case GET_OWNER_STATUS:
            PRINTF("[%d]: GET_OWNER_STATUS\r\n", *state);
            app_get_owner_status(0);
            *state = WAIT_GET_OWNER_STATUS;
            break;
        case SET_OWNER_STATUS:
            PRINTF("[%d]: SET_OWNER_STATUS\r\n", *state);
            app_set_owner_status(0);
            *state = WAIT_SET_OWNER_STATUS;
            break;
        case PUSH_MSG:
            PRINTF("[%d]: PUSH_MSG\r\n", *state);
            app_push_msg(NULL, false);
            *state = WAIT_PUSH_MSG;
            break;
        case SET_FOLDER_PARENT:
            PRINTF("[%d]: SET_FOLDER_PARENT\r\n", *state);
            app_set_folder("../");
            *state = WAIT_SET_FOLDER_PARENT;
            break;
        case SET_FOLDER_OUTBOX:
            PRINTF("[%d]: SET_FOLDER_OUTBOX\r\n", *state);
            app_set_folder("outbox");
            *state = WAIT_SET_FOLDER_OUTBOX;
            break;
        case MCE_MAS_DISCONNECT:
            PRINTF("[%d]: MCE_MAS_DISCONNECT\r\n", *state);
            app_mce_disconnect();
            *state = WAIT_MCE_MAS_DISCONNECT;
            break;
        default:
            break;
    }
}

static void bt_ready(int err)
{
    struct net_buf *buf = NULL;
    struct bt_hci_cp_write_class_of_device *cp;

    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\r\n", err);
        return;
    }

#if (defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0))
    settings_load();
#endif /* CONFIG_BT_SETTINGS */

    PRINTF("Bluetooth initialized\r\n");

    buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, sizeof(*cp));
    if (buf != NULL)
    {
        cp = net_buf_add(buf, sizeof(*cp));
        sys_put_le24(MAP_MCE_CLASS_OF_DEVICE, &cp->class_of_device[0]);
        err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, buf, NULL);
    }
    else
    {
        err = -ENOBUFS;
    }

    if (err)
    {
        PRINTF("setting class of device failed\r\n");
    }

    err = bt_br_set_connectable(true);
    if (err)
    {
        PRINTF("BR/EDR set/rest connectable failed (err %d)\r\n", err);
        return;
    }
    err = bt_br_set_discoverable(true);
    if (err)
    {
        PRINTF("BR/EDR set discoverable failed (err %d)\r\n", err);
        return;
    }
    PRINTF("BR/EDR set connectable and discoverable done\r\n");

    app_connect_init();
    bt_sdp_register_service(&map_mce_rec);
    app_discover();
}

void map_mce_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth MAP MCE demo start...\r\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\r\n", err);
        return;
    }
    vTaskDelete(NULL);
}
