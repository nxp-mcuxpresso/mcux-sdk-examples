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
#include <errno/errno.h>
#include <stdbool.h>
#include <sys/atomic.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys/slist.h>
#include <sys/work_queue.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hfp_ag.h>
#include <bluetooth/sdp.h>
#include <bluetooth/rfcomm.h>
#include "BT_common.h"
#include "BT_hci_api.h"
#include "BT_sm_api.h"
#include "BT_sdp_api.h"

#include "BT_config.h"
#include "app_handsfree_ag.h"
#include "app_connect.h"
#include "app_shell.h"

/* User may need to change it for real production */
#define APP_CLASS_OF_DEVICE (0x200000U)
struct bt_conn *default_conn;
static uint8_t s_hfp_in_calling_status = 0xff;
typedef struct app_hfp_ag_
{
    struct bt_hfp_ag *hfp_agHandle;
    struct bt_conn *conn;
    uint8_t peerKeyMissed;
    uint8_t appl_acl_initiated;
    uint8_t peer_bd_addr[6];
    uint8_t selectCodec;
} app_hfp_ag_t;
static app_hfp_ag_t g_HfpAg;
static TimerHandle_t s_xTimers    = 0;
static TimerHandle_t s_xTwcTimers = 0;
static struct bt_work s_ataRespWork;
static struct bt_sdp_attribute hfp_ag_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6), //35 06
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_HANDSFREE_AGW_SVCLASS) //11 1F
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
                BT_SDP_ARRAY_16(BT_RFCOMM_CHAN_HFP_AG) //channel number
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
                BT_SDP_ARRAY_16(BT_SDP_HANDSFREE_AGW_SVCLASS) //11 1F
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0108U) //01 08
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("Handsfree_ag"),
    BT_SDP_SUPPORTED_FEATURES(0x2100),
};
static struct bt_sdp_record hfp_ag_rec = BT_SDP_RECORD(hfp_ag_attrs);
static void ag_connected(struct bt_hfp_ag *hfp_ag)
{
    printf("HFP AG Connected!\n");
    g_HfpAg.hfp_agHandle = hfp_ag;
    s_hfp_in_calling_status = 1;
    g_HfpAg.selectCodec = 1;
}
static void ag_disconnected(struct bt_hfp_ag *hfp_ag)
{
    printf("HFP AG Disconnected!\n");
    bt_hfp_ag_disconnect(hfp_ag);
}

hfp_ag_get_config hfp_ag_config = {
    .bt_hfp_ag_vgs             = 15,
    .bt_hfp_ag_vgm             = 15,
    .bt_hfp_ag_codec           = 1,
    .bt_hfp_ag_nrec            = 1,
    .bt_hfp_ag_inband          = 0,
    .bt_hfp_ag_codec_negotiate = 0,
    .bt_hfp_ag_dial            = 0,
};
static void bt_work_ata_response(struct bt_work *work)
{
    printf("HFP HP have accepted the call\n");
    s_hfp_in_calling_status = 3;
    bt_hfp_ag_send_callsetup_indicator(g_HfpAg.hfp_agHandle, 0);
    bt_hfp_ag_send_call_indicator(g_HfpAg.hfp_agHandle, 1);
    if (s_xTimers != 0)
    {
        xTimerStop(s_xTimers, 0);
        xTimerDelete(s_xTimers, 0);
        s_xTimers = 0;
    }
    bt_hfp_ag_call_status_pl(g_HfpAg.hfp_agHandle, hfp_ag_call_call_incoming);
}
void dial(struct bt_hfp_ag *hfp_ag, char *number)
{
    printf("HFP HP have a in coming call :%s\n", number);
    if (s_hfp_in_calling_status == 1)
    {
        PRINTF("Simulate a outcoming calling!!\r\n");
        bt_hfp_ag_send_callsetup_indicator(g_HfpAg.hfp_agHandle, 1);
        //     s_xTimers = xTimerCreate("RingTimer", (2000) + 10, pdTRUE, 0, vTimerRingCallback);
        //   xTimerStart(s_xTimers, 0);
        //   bt_hfp_ag_send_callring(g_HfpAg.hfp_agHandle);
        s_hfp_in_calling_status = 2;
    }
}
void ata_response(struct bt_hfp_ag *hfp_ag)
{
    bt_work_submit(&s_ataRespWork);
}

void chup_response(struct bt_hfp_ag *hfp_ag)
{
    printf("HFP HP have ended the call\n");
    s_hfp_in_calling_status = 1;
    bt_hfp_ag_call_status_pl(g_HfpAg.hfp_agHandle, hfp_ag_call_call_end);
    bt_hfp_ag_send_call_indicator(g_HfpAg.hfp_agHandle, 0);
    if (s_xTimers != 0)
    {
        xTimerStop(s_xTimers, 0);
        xTimerDelete(s_xTimers, 0);
        s_xTimers = 0;
    }
}

static void brva(struct bt_hfp_ag *hfp_ag, uint32_t value)
{
    printf("HFP voice recognition :%d\n", value);
}
static void codec_negotiate(struct bt_hfp_ag *hfp_ag, uint32_t value)
{
    printf("HFP codec negotiate :%d\n", value);
}

static void chld(struct bt_hfp_ag *hfp_ag, uint8_t option, uint8_t index)
{
    printf("AT_CHLD mutlipcall option  index :%d %d\n", option, index);
    if (option == 0)
    {
        printf(
            " Release all Held Calls and set UUDB tone "
            "(Reject new incoming waiting call)\n");
    }
    else if (option == 1)
    {
        printf("  Release Active Calls and accept held/waiting call\n");
    }
    else if (option == 2)
    {
        printf(
            "  Hold Active Call and accept already "
            "held/new waiting call\n");
    }
    else if (option == 3)
    {
        printf(" bt multipcall 3. Conference all calls\n");
    }
    else if (option == 4)
    {
        printf(" bt multipcall 4. Connect other calls and disconnect self from TWC\n");
    }
    if (s_xTwcTimers != 0)
    {
        xTimerStop(s_xTwcTimers, 0);
        xTimerDelete(s_xTwcTimers, 0);
        s_xTwcTimers = 0;
    }
}

void get_config(struct bt_hfp_ag *hfp_ag, hfp_ag_get_config **config)
{
    *config = &hfp_ag_config;
}

static struct bt_hfp_ag_cb ag_cb = {
    .connected       = ag_connected,
    .disconnected    = ag_disconnected,
    .ata_response    = ata_response,
    .chup_response   = chup_response,
    .dial            = dial,
    .brva            = brva,
    .chld            = chld,
    .codec_negotiate = codec_negotiate,
    .get_config      = get_config,
};

int app_hfp_ag_discover(struct bt_conn *conn, uint8_t channel)
{
    int status                   = 0;
    hfp_ag_config.server_channel = channel;
    if (default_conn == conn)
    {
        status = bt_hfp_ag_connect(default_conn, &hfp_ag_config, &ag_cb, &g_HfpAg.hfp_agHandle);
        if (0 != status)
        {
            PRINTF("fail to connect hfp_hf (err: %d)\r\n", status);
        }
    }
    return status;
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
        sys_put_le24(APP_CLASS_OF_DEVICE, &cp->class_of_device[0]);
        err = bt_hci_cmd_send_sync(BT_HCI_OP_WRITE_CLASS_OF_DEVICE, buf, NULL);
    }
    else
    {
        err = -ENOBUFS;
    }

    app_connect_init();

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
    bt_sdp_register_service(&hfp_ag_rec);

    bt_hfp_ag_init();
    bt_hfp_ag_register_cb(&ag_cb);
    app_shell_init();
    bt_work_init(&s_ataRespWork, bt_work_ata_response);
}

static void vTimerRingCallback(TimerHandle_t xTimer)
{
    bt_hfp_ag_send_callring(g_HfpAg.hfp_agHandle);
}

static void vTimerTwcRingCallback(TimerHandle_t xTimer)
{
    bt_hfp_ag_send_callring(g_HfpAg.hfp_agHandle);
}
int app_hfp_ag_start_incoming_call()
{
    if (s_hfp_in_calling_status == 1)
    {
        PRINTF("Simulate a incoming call an incoming calling!!\r\n");
        bt_hfp_ag_send_callsetup_indicator(g_HfpAg.hfp_agHandle, 1);
        s_xTimers = xTimerCreate("RingTimer", (2000) + 10, pdTRUE, 0, vTimerRingCallback);
        xTimerStart(s_xTimers, 0);
        bt_hfp_ag_send_callring(g_HfpAg.hfp_agHandle);
        s_hfp_in_calling_status = 2;
        return 0;
    }
    return -1;
}
int app_hfp_ag_start_twc_incoming_call(void)
{
    if (s_hfp_in_calling_status == 3)
    {
        PRINTF("Simulate a mutiple call incoming call!!\r\n");
        bt_hfp_ag_send_callsetup_indicator(g_HfpAg.hfp_agHandle, 1);
        bt_hfp_ag_send_ccwa_indicator(g_HfpAg.hfp_agHandle, "1234567");
        s_xTwcTimers = xTimerCreate("TwcRingTimer", (2000) + 10, pdTRUE, 0, vTimerTwcRingCallback);
        xTimerStart(s_xTwcTimers, 0);
        bt_hfp_ag_send_callring(g_HfpAg.hfp_agHandle);
        s_hfp_in_calling_status = 4;
        return 0;
    }
    return -1;
}

void app_hfp_ag_open_audio()
{
    bt_hfp_ag_open_audio(g_HfpAg.hfp_agHandle, g_HfpAg.selectCodec - 1);
}
void app_hfp_ag_close_audio()
{
    bt_hfp_ag_close_audio(g_HfpAg.hfp_agHandle);
}
int app_hfp_ag_accept_incoming_call()
{
    if (s_hfp_in_calling_status == 2)
    {
        printf("HFP AG have accepted the incoming call\n");
        s_hfp_in_calling_status = 3;
        bt_hfp_ag_send_callsetup_indicator(g_HfpAg.hfp_agHandle, 0);
        bt_hfp_ag_send_call_indicator(g_HfpAg.hfp_agHandle, 1);
        if (s_xTimers != 0)
        {
            xTimerStop(s_xTimers, 0);
            xTimerDelete(s_xTimers, 0);
            s_xTimers = 0;
        }
        bt_hfp_ag_call_status_pl(g_HfpAg.hfp_agHandle, hfp_ag_call_call_incoming);
        return 0;
    }
    return -1;
}
int app_hfp_ag_stop_incoming_call()
{
    if (s_hfp_in_calling_status >= 2)
    {
        bt_hfp_ag_call_status_pl(g_HfpAg.hfp_agHandle, hfp_ag_call_call_end);
        if (s_xTimers != 0)
        {
            xTimerStop(s_xTimers, 0);
            xTimerDelete(s_xTimers, 0);
            s_xTimers = 0;
        }
        bt_hfp_ag_send_call_indicator(g_HfpAg.hfp_agHandle, 0);
        printf("HFP AG have ended the call\n");
        s_hfp_in_calling_status = 1;
        return 0;
    }
    return -1;
}
int app_hfp_ag_codec_select(uint8_t codec)
{
    g_HfpAg.selectCodec = codec;
    return bt_hfp_ag_codec_selector(g_HfpAg.hfp_agHandle, codec);
}
void app_hfp_ag_set_phnum_tag(char *name)
{
    bt_hfp_ag_set_phnum_tag(g_HfpAg.hfp_agHandle, name);
}
void app_hfp_ag_volume_update(hf_volume_type_t type, int volume)
{
    bt_hfp_ag_set_volume_control(g_HfpAg.hfp_agHandle, type, volume);
}

void peripheral_hfp_ag_task(void *pvParameters)
{
    int err = 0;

    PRINTF("Bluetooth Handsfree AG demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err)
    {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }
    vTaskDelete(NULL);
}
