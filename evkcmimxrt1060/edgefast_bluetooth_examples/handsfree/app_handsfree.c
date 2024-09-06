/*
 * Copyright 2021, 2024 NXP
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
#include <bluetooth/hfp_hf.h>
#include <bluetooth/sdp.h>
#include <bluetooth/rfcomm.h>
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"
#include "app_handsfree.h"
#include "db_gen.h"
#include "app_shell.h"
#include "app_connect.h"

#define APP_HFP_HF_INITIAL_VGS_GAIN 12
#define APP_HFP_HF_INITIAL_VGM_GAIN 12
#define HFP_CLASS_OF_DEVICE (0x200404U)

static volatile uint8_t s_call_status = 0;
hfp_hf_get_config hfp_hf_config = {
    .bt_hfp_hf_vgs             = APP_HFP_HF_INITIAL_VGS_GAIN,
    .bt_hfp_hf_vgm             = APP_HFP_HF_INITIAL_VGM_GAIN,
};

static struct bt_sdp_attribute hfp_hf_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6), //35 06
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_HANDSFREE_SVCLASS) //11 1E
        },
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_GENERIC_AUDIO_SVCLASS) //12 03
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 12),//35 10
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3),// 35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP) // 01 00
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 5),// 35 05
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_RFCOMM) // 00 19
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT8), //08
                BT_SDP_ARRAY_16(BT_RFCOMM_CHAN_HFP_HF) //channel number
            },
            )
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
                BT_SDP_ARRAY_16(BT_SDP_HANDSFREE_SVCLASS) //11 1E
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0108U) //01 08
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("Handsfree"),
    BT_SDP_SUPPORTED_FEATURES(0x3400),
};
static struct bt_sdp_record hfp_hf_rec = BT_SDP_RECORD(hfp_hf_attrs);

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
                                                               //  .passkey_confirm = passkey_confirm,
};
static void connected(struct bt_conn *conn)
{
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
    struct bt_conn_info info;
#endif

    printf("HFP HF Connected!\n");
    default_conn = conn;
#if !((defined AUTO_CONNECT_USE_BOND_INFO) && (AUTO_CONNECT_USE_BOND_INFO))
    bt_conn_get_info(conn, &info);
    app_auto_connect_save_addr(info.br.dst);
#endif
}

static void disconnected(struct bt_conn *conn)
{
    PRINTF("HFP BT Disconnected !\n");

    if (default_conn)
    {
        default_conn = NULL;
    }
}

static void service(struct bt_conn *conn, uint32_t value)
{
    printf("Service indicator value: %u\n", value);
}

static void call(struct bt_conn *conn, uint32_t value)
{
    if (value == 1)
    {
        s_call_status = 2;
    }
    else
    {
        s_call_status = 0;
    }
    printf("Call indicator value: %u\n", value);
}

static void call_setup(struct bt_conn *conn, uint32_t value)
{
    printf("Call Setup indicator value: %u\n", value);
}

static void call_held(struct bt_conn *conn, uint32_t value)
{
    printf("Call Held indicator value: %u\n", value);
    if (value == 1)
    {
        s_call_status = 2;
    }
}

static void signal(struct bt_conn *conn, uint32_t value)
{
    printf("Signal indicator value: %u\n", value);
}

static void roam(struct bt_conn *conn, uint32_t value)
{
    printf("Roaming indicator value: %u\n", value);
}

static void battery(struct bt_conn *conn, uint32_t value)
{
    printf("Battery indicator value: %u\n", value);
}

static void ring_cb(struct bt_conn *conn)
{
    printf("Incoming Call...\n");
    s_call_status = 1;
}
static void call_phnum(struct bt_conn *conn, char *number)
{
    printf("Phone call number: %s\n", number);
}
static void voicetag_phnum(struct bt_conn *conn, char *number)
{
    printf("voice tag_phnum number: %s\n", number);
}

static void waiting_call(struct bt_conn *conn, hf_waiting_call_state_t *wcs)
{
    printf("> CALL WAITING Received Number : %s\n", wcs->number);
    printf("> Please use <multipcall> to handle multipe call operation\n");
    printf(
        " bt multipcall 0. Release all Held Calls and set UUDB tone "
        "(Reject new incoming waiting call)\n");
    printf(" bt multipcall 1. Release Active Calls and accept held/waiting call\n");
    printf(
        " bt multipcall 2. Hold Active Call and accept already "
        "held/new waiting call\n");
    printf(" bt multipcall 3. Conference all calls\n");
    printf(" bt multipcall 4. Connect other calls and disconnect self from TWC\n");
}

static void app_hfp_hf_get_config( hfp_hf_get_config **config)
{
    *config = &hfp_hf_config;
}
static struct bt_hfp_hf_cb hf_cb = {
    .connected       = connected,
    .disconnected    = disconnected,
    .service         = service,
    .call            = call,
    .call_setup      = call_setup,
    .call_held       = call_held,
    .signal          = signal,
    .roam            = roam,
    .battery         = battery,
    .ring_indication = ring_cb,
    .call_phnum      = call_phnum,
    .voicetag_phnum  = voicetag_phnum,
    .waiting_call    = waiting_call,
    .get_config      = app_hfp_hf_get_config,
};

static void handsfree_enable(void)
{
    int err;

    err = bt_hfp_hf_register(&hf_cb);
    if (err < 0)
    {
        printf("HFP HF Registration failed (err %d)\n", err);
    }
}

int app_hfp_hf_discover(struct bt_conn *conn, uint8_t channel)
{
    int err = 0;

    if (default_conn == conn)
    {
        err = bt_hfp_hf_connect(default_conn, channel);
        if (err)
        {
            PRINTF("fail to connect hfp_hf (err: %d)\r\n", err);
        }
    }
    return err;
}

static void bt_ready(int err)
{
    struct net_buf *buf = NULL;
    struct bt_hci_cp_write_class_of_device *cp;

    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

#if (defined(CONFIG_BT_SETTINGS) && (CONFIG_BT_SETTINGS > 0))
    settings_load();
#endif /* CONFIG_BT_SETTINGS */

    PRINTF("Bluetooth initialized\n");

    buf = bt_hci_cmd_create(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, sizeof(*cp));
    if (buf != NULL)
    {
        cp = net_buf_add(buf, sizeof(*cp));
        sys_put_le24(HFP_CLASS_OF_DEVICE, &cp->class_of_device[0]);
        err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, buf, NULL);
    }
    else
    {
        err = -ENOBUFS;
    }

    if (err)
    {
        PRINTF("setting class of device failed\n");
    }

    app_connect_init();
    bt_conn_auth_cb_register(&auth_cb_display);
    bt_sdp_register_service(&hfp_hf_rec);
    handsfree_enable();
    app_shell_init();
    app_a2dp_hf_auto_connect();
    
}

void hfp_AnswerCall(void)
{
    bt_hfp_hf_send_cmd(default_conn, BT_HFP_HF_ATA);
    s_call_status = 2;
}

void hfp_RejectCall(void)
{
    bt_hfp_hf_send_cmd(default_conn, BT_HFP_HF_AT_CHUP);
    s_call_status = 0;
}
void hfp_dial(const char *number)
{
    bt_hfp_hf_dial(default_conn, number);
}
void dial_memory(int location)
{
    bt_hfp_hf_dial_memory(default_conn, location);
}
void hfp_last_dial(void)
{
    bt_hfp_hf_last_dial(default_conn);
}
void hfp_start_voice_recognition(void)
{
    bt_hfp_hf_start_voice_recognition(default_conn);
}

void hfp_hf_get_last_voice_tag_number(void)
{
    bt_hfp_hf_get_last_voice_tag_number(default_conn);
}

void hfp_stop_voice_recognition(void)
{
    bt_hfp_hf_stop_voice_recognition(default_conn);
}

void hfp_volume_update(hf_volume_type_t type, int volume)
{
    bt_hfp_hf_volume_update(default_conn, type, volume);
}
void hfp_enable_ccwa(uint8_t enable)
{
    if (enable)
    {
        bt_hfp_hf_enable_call_waiting_notification(default_conn);
    }
    else
    {
        bt_hfp_hf_disable_call_waiting_notification(default_conn);
    }
}
void hfp_enable_clip(uint8_t enable)
{
    if (enable)
    {
        bt_hfp_hf_enable_clip_notification(default_conn);
    }
    else
    {
        bt_hfp_hf_disable_clip_notification(default_conn);
    }
}

void hfp_multiparty_call_option(uint8_t option)
{
    bt_hfp_hf_multiparty_call_option(default_conn, (hf_multiparty_call_option_t)option);
}
void peripheral_hfp_hf_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth Handsfree demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }
    vTaskDelete(NULL);
}
