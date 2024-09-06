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
#include <bluetooth/map_mse.h>
#include <bluetooth/sdp.h>
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"
#include "app_map_mse.h"
#include "app_connect.h"
#include "ff.h"
#include "diskio.h"

#define MAP_MSE_CLASS_OF_DEVICE (0x10020CU) /* Object Transfer, Phone, Smartphone */

#define SDP_CLIENT_USER_BUF_LEN 512U
#define MAP_MSE_MAS_TX_NET_BUF_COUNT   (1U)
#define MAP_MSE_MNS_TX_NET_BUF_COUNT   (1U)
#define MAP_MSE_MAS_TX_NET_BUF_SIZE    (1790U + 2U) /* L2CAP I-frame Enhanced Control Field(2-byte) */
#define MAP_MSE_MNS_TX_NET_BUF_SIZE    (CONFIG_BT_MAP_MSE_MNS_MAX_PKT_LEN + 2U) /* L2CAP I-frame Enhanced Control Field(2-byte) */

#define MAP_MSE_DRIVE_NUMBER "0:" /* RAMDISK + '0' */
#define MAP_MSE_REPO_ROOT "0:/root" /* RAMDISK + '0' */
#define MAP_MSE_XML_FOLDER_LISTING_PATH "0:/folder_listing.xml"
#define MAP_MSE_XML_MSG_LISTING_PATH    "0:/msg_listing.xml"
#define MAP_MSE_MAS_INSTANCE_INFO "SMS/MMS"
#define MAP_MSE_TIME "20180101T000000+0000"

#define MAP_FLAG_STRING(index) ((char *[]){"", "START", "CONTINUE", "", "END", "UNSEG"})[index]

#define DATE_YEAR_SHIFT   (9U)
#define DATE_YEAR_MASK    (0xFE00U)
#define DATE_MONTH_SHIFT  (5U)
#define DATE_MONTH_MASK   (0x01E0U)
#define DATE_DAY_SHIFT    (0U)
#define DATE_DAY_MASK     (0x001FU)
#define TIME_HOUR_SHIFT   (11U)
#define TIME_HOUR_MASK    (0xF800U)
#define TIME_MINUTE_SHIFT (5U)
#define TIME_MINUTE_MASK  (0x07E0U)
#define TIME_SECOND_SHIFT (0U)
#define TIME_SECOND_MASK  (0x001FU)

static struct bt_sdp_attribute map_mse_instatance_1_attrs[] = {
    BT_SDP_NEW_SERVICE,
    /* ServiceClassIDList */
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_MAP_MSE_SVCLASS) //11 32
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
                BT_SDP_ARRAY_8(BT_RFCOMM_CHAN_MAP_MSE) //RFCOMM CHANNEL
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
    BT_SDP_SERVICE_NAME("MAP MAS-name"),
    /* GoepL2CapPsm */
    BT_SDP_ATTR_GOEP_L2CAP_PSM,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT16), 
        BT_SDP_ARRAY_16(BT_BR_PSM_MAP_MSE_1)
    },
    /* MASInstanceID */
    BT_SDP_ATTR_MAS_INSTANCE_ID,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
        BT_SDP_ARRAY_8(0x0)
    },
    /* SupportedMessageTypes */
    BT_SDP_ATTR_SUPPORTED_MESSAGE_TYPES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
        BT_SDP_ARRAY_8(0x1F)
    },
    /*  SupportedFeatures */
    BT_SDP_ATTR_MAP_SUPPORTED_FEATURES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT32), 
        BT_SDP_ARRAY_32(CONFIG_BT_MAP_MSE_MNS_SUPPORTED_FEATURES)
    },
};

static struct bt_sdp_attribute map_mse_instatance_2_attrs[] = {
    BT_SDP_NEW_SERVICE,
    /* ServiceClassIDList */
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_MAP_MSE_SVCLASS) //11 32
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
                BT_SDP_ARRAY_8(BT_RFCOMM_CHAN_MAP_MSE) //RFCOMM CHANNEL
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
    BT_SDP_SERVICE_NAME("MAP MAS-name"),
    /* GoepL2CapPsm */
    BT_SDP_ATTR_GOEP_L2CAP_PSM,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT16), 
        BT_SDP_ARRAY_16(BT_BR_PSM_MAP_MSE_2)
    },
    /* MASInstanceID */
    BT_SDP_ATTR_MAS_INSTANCE_ID,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
        BT_SDP_ARRAY_8(0x0)
    },
    /* SupportedMessageTypes */
    BT_SDP_ATTR_SUPPORTED_MESSAGE_TYPES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
        BT_SDP_ARRAY_8(0x1F)
    },
    /*  SupportedFeatures */
    BT_SDP_ATTR_MAP_SUPPORTED_FEATURES,
    {
        BT_SDP_TYPE_SIZE(BT_SDP_UINT32), 
        BT_SDP_ARRAY_32(CONFIG_BT_MAP_MSE_MNS_SUPPORTED_FEATURES)
    },
};

static struct bt_sdp_record map_mse_inst_1_rec = BT_SDP_RECORD(map_mse_instatance_1_attrs);
static struct bt_sdp_record map_mse_inst_2_rec = BT_SDP_RECORD(map_mse_instatance_2_attrs);

#define MAP_MSE_MSG_UTF_8 \
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

#define MAP_MSE_XML_CONVO_LISTING \
"<MAP-convo-listing version = \"1.0\">\r\n\
    <conversation id=\"E1E2E3E4F1F2F3F4A1A2A3A4B1B2B3B4\" name=\"Beergarden Connection\" last_activity=\"20140612T105430+0100\" read_status=\"no\" version_counter=\"A1A1B2B2C3C3D4D5E5E6F6F7A7A8B8B\">\r\n\
        <participant uci=\"4986925814@s.whateverapp.net\" display_name=\"Tien\" chat_state=\"3\" last_activity=\"20140612T105430+0100\"/>\r\n\
        <participant uci=\"4912345678@s.whateverapp.net\" display_name=\"Jonas\" chat_state=\"5\" last_activity=\"20140610T115130+0100\"/>\r\n\
        <participant uci=\"4913579864@s.whateverapp.net\" display_name=\"Max\" chat_state=\"2\" last_activity=\"20140608T225435+0100\"/>\r\n\
        <participant uci=\"4924689753@s.whateverapp.net\" display_name=\"Nils\" chat_state=\"0\" last_activity=\"20140612T092320+0100\"/>\r\n\
        <participant uci=\"4923568910@s.whateverapp.net\" display_name=\"Alex\" chat_state=\"4\" last_activity=\"20140612T104110+0100\"/>\r\n\
    </conversation>\r\n\
    <conversation id=\"C1C2C3C4D1D2D3D4E1E2E3E4F1F2F3F4\" name=\"\" last_activity=\"20140801T012900+0100\" read_status=\"yes\" version_counter=\"0A0A1B1B2C2C3D3D4E4E5F5F6A6A7B7B\">\r\n\
        <participant uci=\"malo@email.de\" display_name=\"Mari\" chat_state=\"2\" last_activity=\"20140801T012900+0100\" x_bt_uid=\" A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\"/>\r\n\
    </conversation>\r\n\
    <conversation id=\"F1F2F3F4E1E2E3E4D1D2D3D4C1C2C3C4\" name=\"family\" last_activity=\"20140925T133700+0100\" read_status=\"yes\" version_counter=\"1A1A2B2B3C3C4D4D5E5E6F6F7A7A8B8B\">\r\n\
        <participant uci=\"malo@email.de\" display_name=\"Mari\" chat_state=\"2\" last_activity=\"20140801T012900+0100\" x_bt_uid=\" A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\" name=\"Mareike Loh\" presence_availability=\"2\" presence_text=\"Wow it's hot today\" priority=\"100\"/>\r\n\
        <participant uci=\"alois.s@august.de\" display_name=\"Lil Al\" chat_state=\"1\" last_activity=\"20140801T012800+0100\" x_bt_uid=\" A1A2A3A4B1B2C1C2D1D2E1E2E3E4F1F2\" name=\"Alois S.\" presence_availability=\"0\" presence_text=\"On my way\" priority=\"100\"/>\r\n\
    </conversation>\r\n\
</MAP-convo-listing>"

#define MAP_MSE_XML_EVENT_REPORT \
"<MAP-event-report version = \"1.0\">\r\n\
    <event type = \"NewMessage\" handle = \"12345678\" folder = \"TELECOM/MSG/INBOX\" msg_type = \"SMS_CDMA\" />\r\n\
</MAP-event-report>"

struct map_hdr
{
    uint8_t *value;
    uint16_t length;
};

struct map_app_param_user_data
{
    enum map_cmd_id id;
    void *data;
};

struct map_xml_msg_listing
{
    uint32_t listing_size;
    uint32_t unread_cnt;
};

struct map_msg_status
{
    uint8_t status_ind;
    uint8_t status_val;
    uint8_t ext_data[30];
};

struct map_owner_status
{
    char convo_id[BT_MAP_CONVO_ID_SIZE];
    uint8_t last_activity[sizeof("YYYYMMDDTHHMMSS±HHMM")];
    uint8_t pres_text[30];
    uint8_t pres_avail;
    uint8_t chat_state;
};

static uint8_t sdp_discover_cb(struct bt_conn *conn, struct bt_sdp_client_result *result);
static void map_mse_mns_connected(struct bt_map_mse_mns *mse_mns);
static void map_mse_mns_disconnected(struct bt_map_mse_mns *mse_mns, uint8_t result);
static void app_send_event_cb(struct bt_map_mse_mns *mse_mns, uint8_t result);
static void map_mse_mas_connected(struct bt_map_mse_mas *mse_mas, uint16_t psm, uint8_t scn);
static void map_mse_mas_disconnected(struct bt_map_mse_mas *mse_mas, uint8_t result);
static void app_abort_cb(struct bt_map_mse_mas *mse_mas);
static void app_get_folder_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_set_folder_cb(struct bt_map_mse_mas *mse_mas, char *name);
static void app_get_msg_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag);
static void app_get_msg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag);
static void app_set_msg_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag);
static void app_push_msg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag);
static void app_set_ntf_reg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_update_inbox_cb(struct bt_map_mse_mas *mse_mas);
static void app_get_mas_inst_info_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_set_owner_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_get_owner_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_get_convo_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);
static void app_set_ntf_filter_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag);

struct app_map_mse_instance app_instance;
NET_BUF_POOL_FIXED_DEFINE(sdp_client_pool, CONFIG_BT_MAX_CONN, SDP_CLIENT_USER_BUF_LEN, NULL);
NET_BUF_POOL_FIXED_DEFINE(mse_mas_tx_pool, MAP_MSE_MAS_TX_NET_BUF_COUNT, BT_L2CAP_BUF_SIZE(MAP_MSE_MAS_TX_NET_BUF_SIZE), NULL);
NET_BUF_POOL_FIXED_DEFINE(mse_mns_tx_pool, MAP_MSE_MNS_TX_NET_BUF_COUNT, BT_L2CAP_BUF_SIZE(MAP_MSE_MNS_TX_NET_BUF_SIZE), NULL);
static FATFS map_fatfs;
static BYTE map_fatfs_work[FF_MAX_SS];
static FIL map_fsrc;
static FIL map_fdes;
static struct map_owner_status map_owner_status;
static struct map_msg_status map_msg_status;

static struct bt_sdp_discover_params discov_map_mce = {
    .uuid = BT_UUID_DECLARE_16(BT_SDP_MAP_MCE_SVCLASS),
    .func = sdp_discover_cb,
    .pool = &sdp_client_pool,
};

struct bt_map_mse_mas_cb map_mas_cb = {
    .connected = map_mse_mas_connected,
    .disconnected = map_mse_mas_disconnected,
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

struct bt_map_mse_mns_cb map_mns_cb = {
    .connected = map_mse_mns_connected,
    .disconnected = map_mse_mns_disconnected,
    .send_event = app_send_event_cb,
};

static uint8_t sdp_discover_cb(struct bt_conn *conn, struct bt_sdp_client_result *result)
{
    int res;
    uint16_t scn;
    uint16_t psm = 0;
    uint32_t supported_features;
    uint16_t map_version;
    const char *service_name;

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
            PRINTF("L2CAP PSM is not found\r\n");
        }
        else
        {
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
        }
        res = bt_sdp_get_pbap_map_ctn_features(result->resp_buf, &supported_features);
        if (res < 0)
        {
            PRINTF("Supported features is not found\r\n");
        }
        else
        {
            PRINTF("MAP supported features 0x%08X\r\n", supported_features);
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
            PRINTF("Message Notification Server found. Connecting ...\n");

            if (psm != 0)
            {
                res = bt_map_mse_mns_psm_connect(app_instance.acl_conn, psm, supported_features, &app_instance.mse_mns);
            }
            else
            {
                res = bt_map_mse_mns_scn_connect(app_instance.acl_conn, (uint8_t)scn, supported_features, &app_instance.mse_mns);
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

static int map_dirname(char *name)
{
    char *str;

    if (name == NULL)
    {
        return -EINVAL;
    }

    str = strrchr(name, '/');
    if (str != NULL)
    {
        str[0] = '\0';
    }
    else
    {
        return -EINVAL;
    }

    return 0;
}

static int map_joinpath(char *name1, char *name2)
{
    FRESULT fr;

    if ((name1 == NULL) || (name2 == NULL))
    {
        return -EINVAL;
    }
    if (strlen(name1) + strlen("/") + strlen(name2) >= MAP_MSE_MAX_PATH_LEN)
    {
        return -ENAMETOOLONG;
    }
    strcat(name1, "/");
    strcat(name1, name2);

    fr = f_stat(name1, NULL);
    if (fr == FR_OK)
    {
        return 0;
    }
    else if (fr == FR_NO_FILE)
    {
        map_dirname(name1);
        return -ENOENT;
    }
    else
    {
        map_dirname(name1);
        return -EIO;
    }
}

static int map_fs_init(void)
{
    MKFS_PARM formatOptions;
    UINT actual;
    FATFS *fs;
    DWORD fre_clust;
    DWORD fre_sect;
    DWORD tot_sect;
    WORD sector_size;

    if (f_mount(&map_fatfs, MAP_MSE_DRIVE_NUMBER, 0) != FR_OK)
    {
        return -EBUSY;
    }
    memset(&formatOptions, 0, sizeof(formatOptions));
    formatOptions.fmt = FM_SFD | FM_ANY;
    if (f_mkfs(MAP_MSE_DRIVE_NUMBER, &formatOptions, &map_fatfs_work, FF_MAX_SS) != FR_OK)
    {
        return -EBUSY;
    }
    (void)sprintf(&app_instance.path[0], "%s", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/inbox", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/outbox", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/sent", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/deleted", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/draft", MAP_MSE_REPO_ROOT);
    if (f_mkdir((const TCHAR *)&app_instance.path[0]) != FR_OK)
    {
        return -EIO;
    }

    /* write one message into inbox */
    (void)sprintf(&app_instance.path[0], "%s/telecom/msg/inbox/%016llX", MAP_MSE_REPO_ROOT, app_instance.msg_handle);
    app_instance.msg_handle++;
    if (f_open(&map_fdes, &app_instance.path[0], FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
    {
        return -EIO;
    }
    if ((f_write(&map_fdes, MAP_MSE_MSG_UTF_8, strlen(MAP_MSE_MSG_UTF_8), &actual) != FR_OK) || (actual != strlen(MAP_MSE_MSG_UTF_8)))
    {
        return -EIO;
    }
    f_close(&map_fdes);

    if ((f_getfree(MAP_MSE_DRIVE_NUMBER, &fre_clust, &fs) != FR_OK) || (disk_ioctl(RAMDISK, GET_SECTOR_SIZE, &sector_size) != RES_OK))
    {
        return -EIO;
    }
    tot_sect = (fs->n_fatent - 2) * fs->csize;
    fre_sect = fre_clust * fs->csize;
    PRINTF("File system mounted\r\n");
    PRINTF("Total drive space - %lluB\r\n", (uint64_t)tot_sect * (uint64_t)sector_size);
    PRINTF("Free drive space - %lluB\r\n", (uint64_t)fre_sect * (uint64_t)sector_size);

    (void)sprintf(&app_instance.path[0], "%s", MAP_MSE_REPO_ROOT);

    return 0;
}

static void map_mse_mns_connected(struct bt_map_mse_mns *mse_mns)
{
#if 0
    struct net_buf *buf;
    uint16_t max_pkt_len;
    uint16_t actual;
    char event_report[] = MAP_MSE_XML_EVENT_REPORT;
    enum bt_obex_req_flags flag;
#endif

    app_instance.mse_mns = mse_mns;
    app_instance.mns_tx_cnt = 0;
    PRINTF("MSE MNS connection\r\n");
    if (bt_map_mse_mns_get_max_pkt_len(mse_mns, &app_instance.mns_max_pkt_len) == 0)
    {
        PRINTF("MAX Packet Length - %d\r\n", app_instance.mns_max_pkt_len);
    }
    else
    {
        PRINTF("MAX Packet Length is invalid\r\n");
    }

#if 0
    buf = net_buf_alloc(&mse_mns_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_EVENT(mse_mns, BT_OBEX_REQ_UNSEG));
    BT_MAP_ADD_MAS_INSTANCE_ID(buf, 0);
    max_pkt_len = app_instance.mns_max_pkt_len;
    max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_EVENT(mse_mns, BT_OBEX_REQ_UNSEG) - BT_L2CAP_BUF_SIZE(2U);
    max_pkt_len -= buf->len; /* application parameter */
    max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
    actual = strlen(event_report);
    if (actual > max_pkt_len)
    {
        flag = BT_OBEX_REQ_START;
        BT_MAP_ADD_BODY(buf, (uint8_t *)event_report, max_pkt_len);
        app_instance.mns_tx_cnt += max_pkt_len;
    }
    else
    {
        flag = BT_OBEX_REQ_UNSEG;
        BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)event_report, actual);
        app_instance.mns_tx_cnt = 0;
    }

    if (bt_map_mse_send_event(app_instance.mse_mns, buf, flag) != 0)
    {
        net_buf_unref(buf);
        PRINTF("Failed to send event report\r\n");
    }
#endif
}

static void map_mse_mns_disconnected(struct bt_map_mse_mns *mse_mns, uint8_t result)
{
    app_instance.mse_mns = NULL;
    PRINTF("MSE MNS disconnection - 0x%02X\r\n", result);
}

static void map_mse_mas_connected(struct bt_map_mse_mas *mse_mas, uint16_t psm, uint8_t scn)
{
    app_instance.msg_handle = 0;
    app_instance.tx_cnt = 0;
    app_instance.mse_mas = mse_mas;
    if (map_fs_init() != 0)
    {
        PRINTF("File system init failed\r\n");
    }
    PRINTF("MSE MAS connection\r\n");
    if (bt_map_mse_get_max_pkt_len(mse_mas, &app_instance.max_pkt_len) == 0)
    {
        PRINTF("MAX Packet Length - %d\r\n", app_instance.max_pkt_len);
    }
    else
    {
        PRINTF("MAX Packet Length is invalid\r\n");
    }
}

static void map_mse_mas_disconnected(struct bt_map_mse_mas *mse_mas, uint8_t result)
{
    app_instance.mse_mas = NULL;
    PRINTF("MSE MAS disconnection - 0x%02X\r\n", result);
}

static void app_abort_cb(struct bt_map_mse_mas *mse_mas)
{
    PRINTF("MCE Abort IND\r\n");
    switch (app_instance.cmd_id)
    {
        case CMD_ID_GET_FOLDER_LISTING:
        case CMD_ID_GET_MSG_LISTING:
        case CMD_ID_GET_MAS_INST_INFO:
        case CMD_ID_GET_CONVO_LISTING:
            app_instance.tx_cnt = 0;
            break;
        case CMD_ID_GET_MSG:
            app_instance.tx_cnt = 0;
            map_dirname(&app_instance.path[0]);
            break;
        case CMD_ID_PUSH_MSG:
            f_close(&map_fdes);
            break;
        case CMD_ID_SET_NTF_FILTER:
        case CMD_ID_SET_OWNER_STATUS:
        case CMD_ID_GET_OWNER_STATUS:
        case CMD_ID_SET_NTF_REG:
        case CMD_ID_UPDATE_INBOX:
        case CMD_ID_SET_MSG_STATUS:
        case CMD_ID_SET_FOLDER:
        default:
            break;
    }

    app_instance.cmd_id = CMD_ID_NONE;
}

static void app_send_event_cb(struct bt_map_mse_mns *mse_mns, uint8_t result)
{
    struct net_buf *buf;
    uint16_t max_pkt_len;
    uint16_t actual;
    char event_report[] = MAP_MSE_XML_EVENT_REPORT;
    enum bt_obex_req_flags flag;

    PRINTF ("MAP Recv Send Event CNF - 0x%02X\r\n", result);
    if (result == BT_MAP_RSP_CONTINUE)
    {
        buf = net_buf_alloc(&mse_mns_tx_pool, osaWaitForever_c);
        net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_EVENT(mse_mns, BT_OBEX_REQ_END));
        if (app_instance.mns_tx_cnt < strlen(event_report))
        {
            max_pkt_len = app_instance.mns_max_pkt_len;
            max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_EVENT(mse_mns, BT_OBEX_REQ_END) - BT_L2CAP_BUF_SIZE(2U);
            max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
            actual = strlen(event_report) - app_instance.mns_tx_cnt;
            if (actual > max_pkt_len)
            {
                BT_MAP_ADD_BODY(buf, (uint8_t *)&event_report[app_instance.mns_tx_cnt], max_pkt_len);
                app_instance.mns_tx_cnt += max_pkt_len;
                flag = BT_OBEX_REQ_CONTINUE;
            }
            else
            {
                BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)&event_report[app_instance.mns_tx_cnt], actual);
                app_instance.mns_tx_cnt = 0;
                flag = BT_OBEX_REQ_END;
            }
        }
        else
        {
            app_instance.mns_tx_cnt = 0;
            flag = BT_OBEX_REQ_END;
        }
        if (bt_map_mse_send_event(app_instance.mse_mns, buf, flag) != 0)
        {
            net_buf_unref(buf);
            PRINTF("Failed to send event report\r\n");
        }
    }
    else
    {
        app_instance.mns_tx_cnt = 0;
    }
}

static bool app_app_param_cb(struct bt_data *data, void *user_data)
{
    struct map_app_param_user_data *app_param = (struct map_app_param_user_data *)user_data;

    switch (data->type)
    {
        case BT_MAP_TAG_ID_MAX_LIST_COUNT:
            if (data->data_len < 2U)
            {
                return false;
            }
            PRINTF ("Max List Count - %d\r\n", sys_get_be16(data->data));
            break;

        case BT_MAP_TAG_ID_LIST_START_OFFSET:
            if (data->data_len < 2U)
            {
                return false;
            }
            PRINTF ("List Start Offset - %d\r\n", sys_get_be16(data->data));
            break;

        case BT_MAP_TAG_ID_FILTER_MESSAGE_TYPE:
            PRINTF ("Filter Message Type - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_FILTER_PERIOD_BEGIN:
        	PRINTF ("Filter Period Begin - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_FILTER_PERIOD_END:
            PRINTF ("Filter Period End - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_FILTER_READ_STATUS:
            PRINTF ("Filter Read Status - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_FILTER_RECIPIENT:
            PRINTF ("Filter Recipient - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_FILTER_ORIGINATOR:
            PRINTF ("Filter Originator - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_FILTER_PRIORITY:
            PRINTF ("Filter Priority - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_ATTACHMENT:
            PRINTF ("Attachment - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_TRANSPARENT:
            PRINTF ("Transparent - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_RETRY:
            PRINTF ("Retry - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_NOTIFICATION_STATUS:
            PRINTF ("Notification Status - %d\r\n", data->data[0]);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_NTF_REG))
            {
                *(uint8_t *)app_param->data = data->data[0];
            }
            break;

        case BT_MAP_TAG_ID_MAS_INSTANCE_ID:
            PRINTF ("MAS Instance ID - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_PARAMETER_MASK:
            if (data->data_len < 4U)
            {
                return false;
            }
            PRINTF ("Parameter Mask - %08X\r\n", sys_get_be32(data->data));
            break;

        case BT_MAP_TAG_ID_SUBJECT_LENGTH:
            PRINTF ("Subject Length - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_CHARSET:
            PRINTF ("Charset - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_FRACTION_REQUEST:
            PRINTF ("Fraction Request - %d\r\n", data->data[0]);
            break;

        case BT_MAP_TAG_ID_STATUS_INDICATOR:
            PRINTF ("Status Indicator - %d\r\n", data->data[0]);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_MSG_STATUS))
            {
                ((struct map_msg_status *)app_param->data)->status_ind = data->data[0];
            }
            break;

        case BT_MAP_TAG_ID_STATUS_VALUE:
            PRINTF ("Status Value - %d\r\n", data->data[0]);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_MSG_STATUS))
            {
                ((struct map_msg_status *)app_param->data)->status_val = data->data[0];
            }
            break;

        case BT_MAP_TAG_ID_PRESENCE_AVAILABILITY:
            PRINTF ("Presence  - %d\r\n", data->data[0]);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_OWNER_STATUS))
            {
                ((struct map_owner_status *)app_param->data)->pres_avail = data->data[0];
            }
            break;

        case BT_MAP_TAG_ID_PRESENCE_TEXT:
            PRINTF ("============== Presence Text ==============\r\n");
            PRINTF("%.*s\r\n", data->data_len, data->data);
            PRINTF ("============ END Presence Text ============\r\n");
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_OWNER_STATUS))
            {
                uint8_t length = data->data_len > (sizeof(((struct map_owner_status *)app_param->data)->pres_text) - 1U) ?
                                 (sizeof(((struct map_owner_status *)app_param->data)->pres_text) - 1U) : data->data_len;
                memcpy(((struct map_owner_status *)app_param->data)->pres_text, data->data, length);
            }
            break;

        case BT_MAP_TAG_ID_LAST_ACTIVITY:
            PRINTF ("Last Activity - %.*s\r\n", data->data_len, data->data);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_OWNER_STATUS))
            {
                uint8_t length = data->data_len > (sizeof(((struct map_owner_status *)app_param->data)->last_activity) - 1U) ?
                                 (sizeof(((struct map_owner_status *)app_param->data)->last_activity) - 1U) : data->data_len;
                memcpy(((struct map_owner_status *)app_param->data)->last_activity, data->data, length);
            }
            break;

        case BT_MAP_TAG_ID_FILTER_LAST_ACTIVITY_BEGIN:
            PRINTF ("Last Activity Begin - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_FILTER_LAST_ACTIVITY_END:
            PRINTF ("Last Activity End - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_CHAT_STATE:
            PRINTF ("Chat State - %d\r\n", data->data[0]);
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_OWNER_STATUS))
            {
                ((struct map_owner_status *)app_param->data)->chat_state = data->data[0];
            }
            break;

        case BT_MAP_TAG_ID_CONVERSATION_ID:
            PRINTF ("Conversation ID - %.*s\r\n", data->data_len, data->data);
            if (app_param != NULL)
            {
                if (app_param->id == CMD_ID_SET_OWNER_STATUS)
                {
                    uint8_t length = data->data_len > (BT_MAP_CONVO_ID_SIZE - 1U) ?
                                     (BT_MAP_CONVO_ID_SIZE - 1U) : data->data_len;
                    memcpy(((struct map_owner_status *)app_param->data)->convo_id, data->data, length);
                }
                else if (app_param->id == CMD_ID_GET_OWNER_STATUS)
                {
                    uint8_t length = data->data_len > (BT_MAP_CONVO_ID_SIZE - 1U) ?
                                     (BT_MAP_CONVO_ID_SIZE - 1U) : data->data_len;
                    memcpy(app_param->data, data->data, length);
                }
                else
                {
                    /* no action */
                }
            }
            break;

        case BT_MAP_TAG_ID_FILTER_MSG_HANDLE:
            PRINTF ("Filter Message Handle - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_NOTIFICATION_FILTER_MASK:
            if (data->data_len < 4U)
            {
                return false;
            }
            PRINTF ("Notification Filter Mask - %08X\r\n", sys_get_be32(data->data));
            break;

        case BT_MAP_TAG_ID_CONV_PARAMETER_MASK:
            if (data->data_len < 4U)
            {
                return false;
            }
            PRINTF ("Conversation Parameter Mask - %08X\r\n", sys_get_be32(data->data));
            break;

        case BT_MAP_TAG_ID_EXTENDED_DATA:
        	PRINTF ("============== Extended Data ==============\r\n");
            PRINTF ("Extended Data - %.*s\r\n", data->data_len, data->data);
            PRINTF ("============ END Extended Data ============\r\n");
            if ((app_param != NULL) && (app_param->id == CMD_ID_SET_MSG_STATUS))
            {
                uint8_t length = data->data_len > (sizeof(((struct map_msg_status *)app_param->data)->ext_data) - 1U) ?
                                 (sizeof(((struct map_msg_status *)app_param->data)->ext_data) - 1U) : data->data_len;
                memcpy(((struct map_msg_status *)app_param->data)->ext_data, data->data, length);
            }
            break;

        case BT_MAP_TAG_ID_MAP_SUPPORTED_FEATURES:
            if (data->data_len < 4U)
            {
                return false;
            }
            PRINTF ("Supported Features - %08X\r\n", sys_get_be32(data->data));
            break;

        case BT_MAP_TAG_ID_MESSAGE_HANDLE:
            PRINTF ("Message Handle - %.*s\r\n", data->data_len, data->data);
            break;

        case BT_MAP_TAG_ID_MODIFY_TEXT:
            PRINTF ("Modify Text - %d\r\n", data->data[0]);
            break;

        default:
            break;
    }

    return true;
}

static int map_create_xml_folder_listing(char *xml_name, char *dir_name)
{
    int err = 0;
    DIR dir;
    FILINFO fno;
    UINT actual;
    char head[] = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\r\n<folder-listing version=\"1.0\">\r\n";
    char tail[] = "</folder-listing>";

    if (f_open(&map_fdes, xml_name, FA_CREATE_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
    {
        return -EIO;
    }
    (void)f_chmod(xml_name, AM_SYS | AM_HID, AM_SYS | AM_HID);
    if ((f_write(&map_fdes, head, strlen(head), &actual) != FR_OK) || (actual < strlen(head)))
    {
        f_close(&map_fdes);
        return -EIO;
    }
    if (f_opendir(&dir, dir_name) != FR_OK)
    {
        f_close(&map_fdes);
        return -EIO;
    }
    for (;;)
    {
        if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0)
        {
            break;
        }
        if (fno.fattrib & AM_DIR)
        {
            (void)f_printf(&map_fdes, "    <folder name = \"%s\"/>\r\n", fno.fname);
        }
    }
    if ((f_write(&map_fdes, tail, strlen(tail), &actual) != FR_OK) || (actual < strlen(tail)))
    {
        err = -EIO;
    }
    f_closedir(&dir);
    f_close(&map_fdes);

    return err;
}

static int map_create_xml_msg_listing(char *xml_name, char *dir_name, struct map_xml_msg_listing *out)
{
    int err = 0;
    DIR dir;
    FILINFO fno;
    UINT actual;
    char path[MAP_MSE_MAX_PATH_LEN];
    char head[] = "<?xml version='1.0' encoding='utf-8' standalone='yes' ?>\r\n<MAP-msg-listing version=\"1.0\">\r\n";
    char tail[] = "</MAP-msg-listing>";
    uint8_t rd_buf[64];
    uint8_t vcard_bgn;
    uint8_t benv_bgn;
    uint8_t msg_bgn;
    struct attr
    {
        uint8_t subject[30];
        uint8_t datetime[sizeof("YYYYMMDDTHHMMSS±HHMM")];
        uint8_t sd_name[20];
        uint8_t sd_addr[20];
        uint8_t rcp_name[20];
        uint8_t rcp_addr[20];
        uint8_t type[9]; /* EMAIL, SMS_GSM, SMS_CDMA, MMS, IM */
        uint8_t size[11]; /* 32 bit decimal string */
        uint8_t read[4]; /* "yes"/"no" */
    } attr;

    memset(out, 0, sizeof(struct map_xml_msg_listing));
    if (f_open(&map_fdes, xml_name, FA_CREATE_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
    {
        return -EIO;
    }
    (void)f_chmod(xml_name, AM_SYS | AM_HID, AM_SYS | AM_HID);
    if ((f_write(&map_fdes, head, strlen(head), &actual) != FR_OK) || (actual < strlen(head)))
    {
        f_close(&map_fdes);
        return -EIO;
    }
    if (f_opendir(&dir, dir_name) != FR_OK)
    {
        f_close(&map_fdes);
        return -EIO;
    }
    for (;;)
    {
        if ((f_readdir(&dir, &fno) != FR_OK) || (fno.fname[0] == 0))
        {
            break;
        }
        if ((fno.fattrib & AM_DIR) == 0U)
        {
            out->listing_size++;
            (void)snprintf(path, MAP_MSE_MAX_PATH_LEN, "%s/%s", dir_name, fno.fname);
            vcard_bgn = 0;
            benv_bgn = 0;
            msg_bgn = 0;
            memset(&attr, 0U, sizeof(attr));
            f_open(&map_fsrc, path, FA_READ);
            for (;;)
            {
                if (f_gets((char *)&rd_buf[0], sizeof(rd_buf), &map_fsrc) == NULL)
                {
                    break;
                }
                for (uint8_t index = 0; index < sizeof(rd_buf); index++)
                {
                    if ((rd_buf[index] == '\r') || (rd_buf[index] == '\n'))
                    {
                        rd_buf[index] = '\0';
                        break;
                    }
                }
                if (strstr((char *)&rd_buf[0], "STATUS:"))
                {
                    if (strstr((char *)&rd_buf[sizeof("STATUS:") - 1U], "UNREAD"))
                    {
                        out->unread_cnt++;
                        snprintf((char *)&attr.read[0], sizeof(attr.read), "%s", "no");
                    }
                    else
                    {
                        snprintf((char *)&attr.read[0], sizeof(attr.read), "%s", "yes");
                    }
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "TYPE:"))
                {
                    snprintf((char *)&attr.type[0], sizeof(attr.type), "%s", &rd_buf[sizeof("TYPE:") - 1U]);
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "BEGIN:VCARD"))
                {
                    vcard_bgn++;
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "BEGIN:BENV"))
                {
                    benv_bgn++;
                    continue;
                }
                if (vcard_bgn > 0)
                {
                    if ((strstr((char *)&rd_buf[0], "VERSION:")) || (strstr((char *)&rd_buf[0], "FN:")))
                    {
                        continue;
                    }
                    else if (strstr((char *)&rd_buf[0], "N:"))
                    {
                        if (benv_bgn == 0)
                        {
                            snprintf((char *)&attr.sd_name[0], sizeof(attr.sd_name), "%s", &rd_buf[sizeof("N:") - 1U]);
                        }
                        else
                        {
                            snprintf((char *)&attr.rcp_name[0], sizeof(attr.rcp_name), "%s", &rd_buf[sizeof("N:") - 1U]);
                        }
                        continue;
                    }
                    else if (strstr((char *)&rd_buf[0], "TEL:"))
                    {
                        if (benv_bgn == 0)
                        {
                            snprintf((char *)&attr.sd_addr[0], sizeof(attr.sd_addr), "%s", &rd_buf[sizeof("TEL:") - 1U]);
                        }
                        else
                        {
                            snprintf((char *)&attr.rcp_addr[0], sizeof(attr.rcp_addr), "%s", &rd_buf[sizeof("TEL:") - 1U]);
                        }
                        continue;
                    }
                    else if (strstr((char *)&rd_buf[0], "EMAIL:"))
                    {
                        if (benv_bgn == 0)
                        {
                            snprintf((char *)&attr.sd_addr[0], sizeof(attr.sd_addr), "%s", &rd_buf[sizeof("EMAIL:") - 1U]);
                        }
                        else
                        {
                            snprintf((char *)&attr.rcp_addr[0], sizeof(attr.rcp_addr), "%s", &rd_buf[sizeof("EMAIL:") - 1U]);
                        }
                        continue;
                    }
                    else
                    {
                        /* no action */
                    }
                }
                if (strstr((char *)&rd_buf[0], "BEGIN:BENV"))
                {
                    benv_bgn--;
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "END:VCARD"))
                {
                    vcard_bgn--;
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "LENGTH:"))
                {
                    snprintf((char *)&attr.size[0], sizeof(attr.size), "%s", &rd_buf[sizeof("LENGTH:") - 1U]);
                    continue;
                }
                if (strstr((char *)&rd_buf[0], "BEGIN:MSG"))
                {
                    msg_bgn++;
                    continue;
                }
                if (msg_bgn > 0)
                {
                    snprintf((char *)&attr.subject[0], sizeof(attr.subject), "%s", &rd_buf[0]);
                    break;
                }
            }
            f_close(&map_fsrc);

            (void)sprintf((char *)&attr.datetime[0], "%.4d%.2d%.2dT%.2d%.2d%.2d+0000",
                ((fno.fdate & DATE_YEAR_MASK) >> DATE_YEAR_SHIFT) + 1980U, /* Year origin from 1980 */
                (fno.fdate & DATE_MONTH_MASK) >> DATE_MONTH_SHIFT,
                (fno.fdate & DATE_DAY_MASK) >> DATE_DAY_SHIFT,
                (fno.ftime & TIME_HOUR_MASK) >> TIME_HOUR_SHIFT,
                (fno.ftime & TIME_MINUTE_MASK) >> TIME_MINUTE_SHIFT,
                ((fno.ftime & TIME_SECOND_MASK) >> TIME_SECOND_SHIFT) << 1U); /* Second / 2 (0...29) */

            (void)f_printf(&map_fdes, "    <msg handle = \"%s\" ", fno.fname);
            (void)f_printf(&map_fdes, "subject = \"%s\" ", &attr.subject[0]);
            (void)f_printf(&map_fdes, "datetime = \"%s\" ", &attr.datetime[0]);
            (void)f_printf(&map_fdes, "sender_name = \"%s\" ", &attr.sd_name[0]);
            (void)f_printf(&map_fdes, "sender_addressing = \"%s\" ", &attr.sd_addr[0]);
            (void)f_printf(&map_fdes, "recipient_name = \"%s\" ", &attr.rcp_name[0]);
            (void)f_printf(&map_fdes, "recipient_addressing = \"%s\" ", &attr.rcp_addr[0]);
            (void)f_printf(&map_fdes, "type = \"%s\" ", &attr.type[0]);
            (void)f_printf(&map_fdes, "size = \"%s\" ", &attr.size[0]);
            (void)f_printf(&map_fdes, "text = \"%s\" ", "yes");
            (void)f_printf(&map_fdes, "recipient_status = \"%s\" ", "complete");
            (void)f_printf(&map_fdes, "attachment_size = \"%s\" ", "0");
            (void)f_printf(&map_fdes, "priority = \"%s\" ", "no");
            (void)f_printf(&map_fdes, "read = \"%s\" ", &attr.read[0]);
            (void)f_printf(&map_fdes, "sent = \"%s\" ", "no");
            (void)f_printf(&map_fdes, "protected = \"%s\"/>\r\n", "no");
        }
    }
    if ((f_write(&map_fdes, tail, strlen(tail), &actual) != FR_OK) || (actual < strlen(tail)))
    {
        err = -EIO;
    }
    f_close(&map_fdes);
    f_closedir(&dir);

    return err;
}

static int map_handle_set_msg_status(char *file_name, struct map_msg_status *msg_status)
{
    char path[MAP_MSE_MAX_PATH_LEN];
    char *msg_handle;
    char *found;
    uint8_t rd_buf[256];

    strcpy(&path[0], file_name);
    if (msg_status->status_ind == 1U) /* deleted */
    {
        map_dirname(&path[0]);
        msg_handle = &file_name[strlen(&path[0]) + 1U];
        map_dirname(&path[0]);
        if (msg_status->status_val == 1)
        {
            map_joinpath(&path[0], "deleted");
            sprintf(&path[strlen(&path[0])], "/%s", msg_handle);
            if (f_rename(file_name, &path[0]) != FR_OK)
            {
                return -EIO;
            }
        }
        else if (msg_status->status_val == 0)
        {
            map_joinpath(&path[0], "inbox");
            sprintf(&path[strlen(&path[0])], "/%s", msg_handle);
            if (f_rename(file_name, &path[0]) != FR_OK)
            {
                return -EIO;
            }
        }
        else
        {
            /* no action */
        }
    }
    else if (msg_status->status_ind == 0U)
    {
        map_dirname(&path[0]);
        sprintf(&path[strlen(&path[0])], "/%s", "internal.msg");
        if (f_open(&map_fsrc, file_name, FA_READ) != FR_OK)
        {
            return -EIO;
        }
        if (f_open(&map_fdes, &path[0], FA_CREATE_ALWAYS | FA_READ | FA_WRITE) != FR_OK)
        {
            f_close(&map_fsrc);
            return -EIO;
        }
        for(;;)
        {
            if (f_gets((char *)&rd_buf[0], sizeof(rd_buf), &map_fsrc) == NULL)
            {
                break;
            }
            if ((strstr((char *)&rd_buf[0], "STATUS:") != NULL) && 
                (strstr((char *)&rd_buf[sizeof("STATUS:") - 1U], "READ") != NULL))
            {
                found = strstr((char *)&rd_buf[0], "STATUS:");
                found[0] = '\0';
                (void)f_printf(&map_fdes, "%s", &rd_buf[0]);
                found = &found[sizeof("STATUS:") - 1U];
                found = strstr(found, "READ");
                (void)f_printf(&map_fdes, "%s", "STATUS:");
                if (msg_status->status_val == 0)
                {
                    (void)f_printf(&map_fdes, "%s", "UNREAD");
                }
                else
                {
                    (void)f_printf(&map_fdes, "%s", "READ");
                }
                (void)f_printf(&map_fdes, "%s", &found[sizeof("READ") - 1U]);
                continue;
            }
            if (f_puts((char *)&rd_buf[0], &map_fdes) != strlen((char *)&rd_buf[0]))
            {
                f_close(&map_fsrc);
                f_close(&map_fdes);
                f_unlink(&path[0]);
                return -EIO;
            }
        }
        f_close(&map_fsrc);
        f_close(&map_fdes);
        f_unlink(file_name);
        f_rename(&path[0], file_name);
    }
    else
    {
        /* handle extended data */
    }

    return 0;
}

static void app_get_folder_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    UINT actual;
    uint8_t *rd_buf;
    uint16_t max_pkt_len;
    uint8_t result;

    PRINTF ("MAP Get Folder Listing IND - %s\r\n", MAP_FLAG_STRING(flag));
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        result = BT_MAP_RSP_SUCCESS;
        if (app_instance.tx_cnt == 0)
        {
            if (map_create_xml_folder_listing(MAP_MSE_XML_FOLDER_LISTING_PATH, &app_instance.path[0]) != 0)
            {
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
        }

        if (result == BT_MAP_RSP_SUCCESS)
        {
            max_pkt_len = app_instance.max_pkt_len;
            max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas) - BT_L2CAP_BUF_SIZE(2U);
            max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
            rd_buf = buf->data + sizeof(struct bt_obex_hdr_bytes); /* body header */

            if ((f_open(&map_fdes, MAP_MSE_XML_FOLDER_LISTING_PATH, FA_READ) != FR_OK) ||
                (f_lseek(&map_fdes, app_instance.tx_cnt) != FR_OK) ||
                (f_read(&map_fdes, rd_buf, max_pkt_len, &actual) != FR_OK))
            {
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
            else
            {
                app_instance.tx_cnt += actual;
                if (app_instance.tx_cnt < f_size(&map_fdes))
                {
                    app_instance.cmd_id = CMD_ID_GET_FOLDER_LISTING;
                    BT_MAP_ADD_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_CONTINUE;
                }
                else
                {
                    app_instance.cmd_id = CMD_ID_NONE;
                    app_instance.tx_cnt = 0;
                    BT_MAP_ADD_END_OF_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_SUCCESS;
                }
            }
            f_close(&map_fdes);
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }
    if (bt_map_mse_get_folder_listing_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_set_folder_cb(struct bt_map_mse_mas *mse_mas, char *name)
{
    uint8_t result;

    PRINTF ("MAP Set Folder IND\r\n");
    PRINTF ("Name - %s\r\n", (name != NULL) ? name : "NULL");

    if (name != NULL)
    {
        result = BT_MAP_RSP_SUCCESS;
        if (strcmp(name, "/") == 0)
        {
            strcpy(&app_instance.path[0], MAP_MSE_REPO_ROOT);
        }
        else if (strstr(name, "../") != NULL)
        {
            if (map_dirname(&app_instance.path[0]) == 0)
            {
                if (name[3] != '\0')
                {
                    if ((strchr(&name[3], '/') != NULL) || (map_joinpath(&app_instance.path[0], &name[3]) != 0))
                    {
                        result = BT_MAP_RSP_PRECOND_FAILED;
                    }
                }
            }
            else
            {
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
        }
        else
        {
            if (name[0] != '\0')
            {
                if (map_joinpath(&app_instance.path[0], name) != 0)
                {
                    result = BT_MAP_RSP_PRECOND_FAILED;
                }
            }
        }
    }
    else
    {
        result = BT_MAP_RSP_BAD_REQ;
    }
    if (bt_map_mse_set_folder_response(mse_mas, result) != 0)
    {
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_get_msg_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag)
{
    UINT actual;
    uint8_t *rd_buf;
    uint16_t max_pkt_len;
    struct map_xml_msg_listing listing;
    uint8_t result;

    PRINTF ("MAP Get MSG Listing IND - %s\r\n", MAP_FLAG_STRING(flag));
    PRINTF ("Name - %s\r\n", (name != NULL) ? name : "NULL");
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        result = BT_MAP_RSP_SUCCESS;
        if (app_instance.tx_cnt == 0)
        {
            if ((name == NULL) || (name[0] == '\0'))
            {
                if (map_create_xml_msg_listing(MAP_MSE_XML_MSG_LISTING_PATH, &app_instance.path[0], &listing) != 0)
                {
                    result = BT_MAP_RSP_INT_SERVER_ERR;
                }
            }
            else if (strchr(name, '/') != NULL)
            {
                result = BT_MAP_RSP_PRECOND_FAILED;
            }
            else if (map_joinpath(&app_instance.path[0], name) == 0)
            {
                if (map_create_xml_msg_listing(MAP_MSE_XML_MSG_LISTING_PATH, &app_instance.path[0], &listing) != 0)
                {
                    result = BT_MAP_RSP_INT_SERVER_ERR;
                }
                map_dirname(&app_instance.path[0]);
            }
            else
            {
                result = BT_MAP_RSP_PRECOND_FAILED;
            }
        }

        if (result == BT_MAP_RSP_SUCCESS)
        {
            max_pkt_len = app_instance.max_pkt_len;
            max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas) - BT_L2CAP_BUF_SIZE(2U);
            if (app_instance.tx_cnt == 0)
            {
                BT_MAP_ADD_NEW_MESSAGE(buf, (listing.unread_cnt > 0) ? 1 : 0);
                BT_MAP_ADD_MSE_TIME(buf, (uint8_t *)MAP_MSE_TIME, sizeof(MAP_MSE_TIME));
                BT_MAP_ADD_LISTING_SIZE(buf, listing.listing_size);
                max_pkt_len -= buf->len; /* application parameters */
            }
            max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
            rd_buf = buf->data + buf->len + sizeof(struct bt_obex_hdr_bytes); /* body header */

            if ((f_open(&map_fdes, MAP_MSE_XML_MSG_LISTING_PATH, FA_READ) != FR_OK) ||
                (f_lseek(&map_fdes, app_instance.tx_cnt) != FR_OK) ||
                (f_read(&map_fdes, rd_buf, max_pkt_len, &actual) != FR_OK))
            {
                app_instance.tx_cnt = 0;
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
            else
            {
                app_instance.tx_cnt += actual;
                if (app_instance.tx_cnt < f_size(&map_fdes))
                {
                    app_instance.cmd_id = CMD_ID_GET_MSG_LISTING;
                    BT_MAP_ADD_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_CONTINUE;
                }
                else
                {
                    app_instance.cmd_id = CMD_ID_NONE;
                    app_instance.tx_cnt = 0;
                    BT_MAP_ADD_END_OF_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_SUCCESS;
                }
            }
            f_close(&map_fdes);
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }
    if (bt_map_mse_get_msg_listing_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_get_msg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag)
{
    UINT actual;
    uint8_t *rd_buf;
    uint16_t max_pkt_len;
    uint8_t result;

    PRINTF ("MAP Get MSG IND -%s\r\n", MAP_FLAG_STRING(flag));
    PRINTF ("Name - %s\r\n", (name != NULL) ? name : "NULL");
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        result = BT_MAP_RSP_SUCCESS;
        if (app_instance.tx_cnt == 0)
        {
            if (name == NULL)
            {
                result = BT_MAP_RSP_BAD_REQ;
            }
            else if ((name[0] == '\0') || (strspn(name, "0123456789abcdefABCDEF") != strlen(name)))
            {
                result = BT_MAP_RSP_PRECOND_FAILED;
            }
            else if (map_joinpath(&app_instance.path[0], name) == 0)
            {
                /* success */
            }
            else
            {
                result = BT_MAP_RSP_PRECOND_FAILED;
            }
        }

        if (result == BT_MAP_RSP_SUCCESS)
        {
            max_pkt_len = app_instance.max_pkt_len;
            max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas) - BT_L2CAP_BUF_SIZE(2U);
            max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
            rd_buf = buf->data + sizeof(struct bt_obex_hdr_bytes); /* body header */

            if ((f_open(&map_fdes, &app_instance.path[0], FA_READ) != FR_OK) ||
                (f_lseek(&map_fdes, app_instance.tx_cnt) != FR_OK) ||
                (f_read(&map_fdes, rd_buf, max_pkt_len, &actual) != FR_OK))
            {
                app_instance.tx_cnt = 0;
                map_dirname(&app_instance.path[0]);
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
            else
            {
                app_instance.tx_cnt += actual;
                if (app_instance.tx_cnt < f_size(&map_fdes))
                {
                    app_instance.cmd_id = CMD_ID_GET_MSG;
                    BT_MAP_ADD_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_CONTINUE;
                }
                else
                {
                    app_instance.cmd_id = CMD_ID_NONE;
                    app_instance.tx_cnt = 0;
                    map_dirname(&app_instance.path[0]);
                    BT_MAP_ADD_END_OF_BODY(buf, rd_buf, actual);
                    result = BT_MAP_RSP_SUCCESS;
                }
            }
            f_close(&map_fdes);
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }
    if (bt_map_mse_get_msg_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_set_msg_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag)
{
    uint8_t result;
    struct map_app_param_user_data user_data;

    PRINTF("MAP Set MSG Status IND - %s\r\n", MAP_FLAG_STRING(flag));
    PRINTF ("Name - %s\r\n", (name != NULL) ? name : "NULL");

    result = BT_MAP_RSP_SUCCESS;
    if ((flag & BT_OBEX_REQ_START) != 0U)
    {
        if (name == NULL)
        {
            result = BT_MAP_RSP_BAD_REQ;
        }
        else if ((name[0] == '\0') || (strspn(name, "0123456789abcdefABCDEF") != strlen(name)))
        {
            result = BT_MAP_RSP_PRECOND_FAILED;
        }
        else if (map_joinpath(&app_instance.path[0], name) == 0)
        {
            /* success */
            map_msg_status.status_ind = 0xFF;
            map_msg_status.status_val = 0xFF;
            memset(&map_msg_status.ext_data[0], 0, sizeof(map_msg_status.ext_data));
        }
        else
        {
            result = BT_MAP_RSP_PRECOND_FAILED;
        }
    }

    user_data.id = CMD_ID_SET_MSG_STATUS;
    user_data.data = (void *)&map_msg_status;
    bt_map_mse_app_param_parse(buf, app_app_param_cb, &user_data);

    if (result == BT_MAP_RSP_SUCCESS)
    {
        if ((flag & BT_OBEX_REQ_END) != 0U)
        {
            if ((map_msg_status.status_ind == 0xFF) ||
                ((map_msg_status.status_ind == 0x00) && (map_msg_status.status_val == 0xFF)) ||
                ((map_msg_status.status_ind == 0x01) && (map_msg_status.status_val == 0xFF)) ||
                ((map_msg_status.status_ind == 0x02) && (strlen((char *)map_msg_status.ext_data) == 0)))
            {
                result = BT_MAP_RSP_BAD_REQ;
            }
            else
            {
                if (map_handle_set_msg_status(&app_instance.path[0], &map_msg_status) != 0)
                {
                    result = BT_MAP_RSP_INT_SERVER_ERR;
                }
            }
            map_dirname(&app_instance.path[0]);
        }
        else
        {
            result = BT_MAP_RSP_CONTINUE;
        }
    }
    net_buf_unref(buf);

    if (bt_map_mse_set_msg_status_response(mse_mas, result) != 0)
    {
        PRINTF("Failed to send response\r\n");
    }
}

static void app_push_msg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, char *name, enum bt_obex_req_flags flag)
{
    UINT actual;
    uint8_t result;
    char msg_handle[BT_MAP_MSG_HANDLE_SIZE / 2U];
    char path[MAP_MSE_MAX_PATH_LEN];
    char *name_req = NULL;
    struct map_hdr body;

    PRINTF ("MAP PUSH MSG IND - %s\r\n", MAP_FLAG_STRING(flag));
    PRINTF ("Name - %s\r\n", (name != NULL) ? name : "NULL");
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);

    result = BT_MAP_RSP_SUCCESS;
    if ((flag & BT_OBEX_REQ_START) != 0U)
    {
        strcpy(&path[0], &app_instance.path[0]);
        if ((name == NULL) || (name[0] == '\0'))
        {
            /* success */
        }
        else if (strchr(name, '/') != NULL)
        {
            result = BT_MAP_RSP_PRECOND_FAILED;
        }
        else if (map_joinpath(&path[0], name) == 0)
        {
            /* success */
        }
        else
        {
            result = BT_MAP_RSP_INT_SERVER_ERR;
        }

        if (result == BT_MAP_RSP_SUCCESS)
        {
            (void)sprintf(&path[strlen(&path[0])], "/%016llX", app_instance.msg_handle);
            if (f_open(&map_fdes, &path[0], FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
            {
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
        }
    }

    if (result == BT_MAP_RSP_SUCCESS)
    {
        if (bt_map_mse_get_body(buf, &body.value, &body.length) == 0)
        {
            PRINTF ("============== BODY ==============\r\n");
            PRINTF("%.*s\r\n", body.length, body.value);
            PRINTF ("============ END BODY ============\r\n");
            if ((f_write(&map_fdes, body.value, body.length, &actual) != FR_OK) ||
                (actual < body.length))
            {
                f_close(&map_fdes);
                result = BT_MAP_RSP_INT_SERVER_ERR;
            }
        }
        if (result == BT_MAP_RSP_SUCCESS)
        {
            if ((flag & BT_OBEX_REQ_END) != 0U)
            {
                (void)sprintf(&msg_handle[0], "%016llX", app_instance.msg_handle);
                app_instance.msg_handle++;
                app_instance.cmd_id = CMD_ID_NONE;
                name_req = msg_handle;
                f_close(&map_fdes);
            }
            else
            {
                app_instance.cmd_id = CMD_ID_PUSH_MSG;
                result = BT_MAP_RSP_CONTINUE;
            }
        }
    }
    net_buf_unref(buf);

    if (bt_map_mse_push_msg_response(mse_mas, result, name_req, false) != 0)
    {
        PRINTF("Failed to send response\r\n");
    }
}

static void app_set_ntf_reg_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;
    struct map_app_param_user_data user_data;
    uint8_t ntf_status = 0xFF;

    PRINTF ("MAP Set Notification Registration IND - %s\r\n", MAP_FLAG_STRING(flag));
    user_data.id = CMD_ID_SET_NTF_REG;
    user_data.data = (void *)&ntf_status;
    bt_map_mse_app_param_parse(buf, app_app_param_cb, &user_data);
    net_buf_unref(buf);

    if ((flag & BT_OBEX_REQ_END) != 0U)
    {
        result = BT_MAP_RSP_SUCCESS;
        if (ntf_status == 1U)
        {
            if (app_instance.mse_mns == NULL)
            {
                if (bt_sdp_discover(app_instance.acl_conn, &discov_map_mce) != 0)
                {
                    PRINTF("SDP discovery failed: result\r\n");
                }
                else
                {
                    PRINTF("SDP discovery started\r\n");
                }
            }
            else
            {
                PRINTF("MSE MNS connection alreay established\r\n");
            }
        }
        else if (ntf_status == 0U)
        {
            (void)bt_map_mse_mns_disconnect(app_instance.mse_mns);
        }
        else
        {
            result = BT_MAP_RSP_BAD_REQ;
        }
    }
    else
    {
        result = BT_MAP_RSP_CONTINUE;
    }

    if (bt_map_mse_set_ntf_reg_response(mse_mas, result) != 0)
    {
        PRINTF("Failed to send response\r\n");
    }
}

static void app_update_inbox_cb(struct bt_map_mse_mas *mse_mas)
{
    PRINTF ("MAP Update Inbox IND\r\n");

    if (bt_map_mse_update_inbox_response(mse_mas, BT_MAP_RSP_SUCCESS) != 0)
    {
        PRINTF("Failed to send response\r\n");
    }
}

static void app_get_mas_inst_info_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint16_t max_pkt_len;
    uint8_t result;
    uint16_t actual;
    char info[] = MAP_MSE_MAS_INSTANCE_INFO;

    PRINTF ("MAP Get MAS Instance Info IND - %s\r\n", MAP_FLAG_STRING(flag));
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        max_pkt_len = app_instance.max_pkt_len;
        max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas) - BT_L2CAP_BUF_SIZE(2U);
        max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
        actual = strlen(info) - app_instance.tx_cnt;
        actual = (actual > max_pkt_len) ? actual : max_pkt_len;
        if ((app_instance.tx_cnt + actual) < strlen(info))
        {
            BT_MAP_ADD_BODY(buf, (uint8_t *)&info[app_instance.tx_cnt], actual);
            app_instance.tx_cnt += actual;
            app_instance.cmd_id = CMD_ID_GET_MAS_INST_INFO;
            result = BT_MAP_RSP_CONTINUE;
        }
        else
        {
            BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)&info[app_instance.tx_cnt], actual);
            app_instance.tx_cnt = 0;
            app_instance.cmd_id = CMD_ID_NONE;
            result = BT_MAP_RSP_SUCCESS;
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }

    if (bt_map_mse_get_mas_inst_info_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_set_owner_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;
    struct map_app_param_user_data user_data;

    PRINTF ("MAP Set Owner Status IND - %s\r\n", MAP_FLAG_STRING(flag));
    memset(&map_owner_status, 0, sizeof(map_owner_status));
    user_data.id = CMD_ID_SET_OWNER_STATUS;
    user_data.data = (void *)&map_owner_status;
    bt_map_mse_app_param_parse(buf, app_app_param_cb, &user_data);
    net_buf_unref(buf);

    if ((flag & BT_OBEX_REQ_END) != 0U)
    {
        result = BT_MAP_RSP_SUCCESS;
    }
    else
    {
        result = BT_MAP_RSP_CONTINUE;
    }

    if (bt_map_mse_set_owner_status_response(mse_mas, result) != 0)
    {
        PRINTF("Failed to send response\r\n");
    }
}

static void app_get_owner_status_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;
    struct map_app_param_user_data user_data;
    char convo_id[BT_MAP_CONVO_ID_SIZE];

    PRINTF ("MAP Get Owner Status IND - %s\r\n", MAP_FLAG_STRING(flag));
    memset(&convo_id[0], 0, BT_MAP_CONVO_ID_SIZE);
    user_data.id = CMD_ID_GET_OWNER_STATUS;
    user_data.data = (void *)&convo_id[0];
    bt_map_mse_app_param_parse(buf, app_app_param_cb, &user_data);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        if (strcmp(convo_id, map_owner_status.convo_id) == 0)
        {
            BT_MAP_ADD_PRESENCE_AVAILABILITY(buf, map_owner_status.pres_avail);
            BT_MAP_ADD_PRESENCE_TEXT(buf, &map_owner_status.pres_text[0], strlen((char *)&map_owner_status.pres_text[0]) + 1U);
            BT_MAP_ADD_LAST_ACTIVITY(buf, &map_owner_status.last_activity[0], strlen((char *)&map_owner_status.last_activity[0]) + 1U);
            BT_MAP_ADD_CHAT_STATE(buf, map_owner_status.chat_state);
            BT_MAP_ADD_END_OF_BODY(buf, NULL, 0);
            result = BT_MAP_RSP_SUCCESS;
        }
        else
        {
            result = BT_MAP_RSP_NOT_FOUND;
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }

    if (bt_map_mse_get_owner_status_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_get_convo_listing_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;
    uint16_t max_pkt_len;
    uint16_t actual;
    char convo_listing[] = MAP_MSE_XML_CONVO_LISTING;

    PRINTF ("MAP Get Conversation Listing IND - %s\r\n", MAP_FLAG_STRING(flag));
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    buf = net_buf_alloc(&mse_mas_tx_pool, osaWaitForever_c);
    net_buf_reserve(buf, BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas));
    if (flag == BT_OBEX_REQ_UNSEG)
    {
        max_pkt_len = app_instance.max_pkt_len;
        max_pkt_len -= BT_MAP_MSE_RSV_LEN_SEND_RESP(mse_mas) - BT_L2CAP_BUF_SIZE(2U);
        max_pkt_len -= sizeof(struct bt_obex_hdr_bytes); /* body header */
        actual = strlen(convo_listing) - app_instance.tx_cnt;
        actual = (actual > max_pkt_len) ? max_pkt_len : actual;
        if ((app_instance.tx_cnt + actual) < strlen(convo_listing))
        {
            BT_MAP_ADD_BODY(buf, (uint8_t *)&convo_listing[app_instance.tx_cnt], actual);
            app_instance.tx_cnt += actual;
            app_instance.cmd_id = CMD_ID_GET_CONVO_LISTING;
            result = BT_MAP_RSP_CONTINUE;
        }
        else
        {
            BT_MAP_ADD_END_OF_BODY(buf, (uint8_t *)&convo_listing[app_instance.tx_cnt], actual);
            app_instance.tx_cnt = 0;
            app_instance.cmd_id = CMD_ID_NONE;
            result = BT_MAP_RSP_SUCCESS;
        }
    }
    else
    {
        result = BT_MAP_RSP_NOT_IMPLEMENTED;
    }

    if (bt_map_mse_get_convo_listing_response(mse_mas, result, buf, false) != 0)
    {
        net_buf_unref(buf);
        PRINTF ("Failed to send response\r\n");
    }
}

static void app_set_ntf_filter_cb(struct bt_map_mse_mas *mse_mas, struct net_buf *buf, enum bt_obex_req_flags flag)
{
    uint8_t result;

    PRINTF ("MAP Set Notification Filter IND - %s\r\n", MAP_FLAG_STRING(flag));
    bt_map_mse_app_param_parse(buf, app_app_param_cb, NULL);
    net_buf_unref(buf);

    if ((flag & BT_OBEX_REQ_END) != 0U)
    {
        result = BT_MAP_RSP_SUCCESS;
    }
    else
    {
        result = BT_MAP_RSP_CONTINUE;
    }

    if (bt_map_mse_set_ntf_filter_response(mse_mas, result) != 0)
    {
        PRINTF("Failed to send response\r\n");
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
        sys_put_le24(MAP_MSE_CLASS_OF_DEVICE, &cp->class_of_device[0]);
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

    bt_sdp_register_service(&map_mse_inst_1_rec);
    bt_sdp_register_service(&map_mse_inst_2_rec);

    app_connect_init();
    err = bt_map_mse_mas_register(&map_mas_cb);
    if (err != 0)
    {
        PRINTF("fail to register MSE MAS callback (err: %d)\r\n", err);
    }
    err = bt_map_mse_mns_register(&map_mns_cb);
    if (err != 0)
    {
        PRINTF("fail to register MSE MNS callback (err: %d)\r\n", err);
    }
}

void map_mse_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth MAP MSE demo start...\r\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\r\n", err);
        return;
    }
    vTaskDelete(NULL);
}
