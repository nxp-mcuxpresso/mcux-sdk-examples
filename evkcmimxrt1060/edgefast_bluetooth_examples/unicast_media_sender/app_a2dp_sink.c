/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)

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
#include <bluetooth/a2dp.h>
#include <bluetooth/a2dp_codec_sbc.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"

#include "fsl_debug_console.h"
#include "app_connect.h"
#include "sys/ring_buffer.h"

#if 1 /* Code change for bridge. */
#include "unicast_media_sender.h"

static OSA_SEMAPHORE_HANDLE_DEFINE(a2dp_sink_init_done);
extern struct ring_buf a2dp_to_ums_audio_buf;
extern int a2dp_sink_audio_frame_size;
bt_addr_t a2dp_bdaddr;

extern int a2dp_audio_sample_rate;
extern int a2dp_audio_channels;
extern int a2dp_audio_bits;
#endif

#define A2DP_CLASS_OF_DEVICE (0x200404U)
#define APP_A2DP_STREAMER_SYNC_TASK_PRIORITY (5U)

struct bt_a2dp *default_a2dp;
struct bt_a2dp_endpoint *default_a2dp_endpoint;
static QueueHandle_t audio_sem;
BT_A2DP_SBC_SINK_ENDPOINT(sbcEndpoint);

static struct bt_sdp_attribute a2dp_sink_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3), //35 03
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
            BT_SDP_ARRAY_16(BT_SDP_AUDIO_SINK_SVCLASS) //11 0B
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 16),//35 10
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6),// 35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP) // 01 00
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(BT_UUID_AVDTP_VAL) // 00 19
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6),// 35 06
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16), //19
                BT_SDP_ARRAY_16(BT_UUID_AVDTP_VAL) // 00 19
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0X0100u) //AVDTP version: 01 00
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
                BT_SDP_ARRAY_16(BT_SDP_ADVANCED_AUDIO_SVCLASS) //11 0d
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16), //09
                BT_SDP_ARRAY_16(0x0103U) //01 03
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("A2DPSink"),
    BT_SDP_SUPPORTED_FEATURES(0x0001U),
};

static struct bt_sdp_record a2dp_sink_rec = BT_SDP_RECORD(a2dp_sink_attrs);

void app_audio_streamer_task_signal(void)
{
    if (0U != __get_IPSR())
    {
        portBASE_TYPE task_to_wake = pdFALSE;

        if (pdTRUE == xSemaphoreGiveFromISR(audio_sem, &task_to_wake))
        {
            portYIELD_FROM_ISR((task_to_wake));
        }
    }
    else
    {
        xSemaphoreGive(audio_sem);
    }
}

void AudioTask(void *handle)
{
    audio_sem = xSemaphoreCreateCounting(0xFF, 0U);
    if (NULL == audio_sem)
    {
        vTaskDelete(NULL);
    }

    while (1U)
    {
        if (pdFALSE == xSemaphoreTake(audio_sem, portMAX_DELAY))
        {
            continue;
        }

        bt_a2dp_snk_media_sync(default_a2dp_endpoint, NULL, 0U);
    }
}

void sbc_configured(struct bt_a2dp_endpoint_configure_result *configResult)
{
    if (configResult->err == 0)
    {
        default_a2dp_endpoint = &sbcEndpoint;

#if 1 /* Code change for bridge. */
        a2dp_audio_sample_rate  = bt_a2dp_sbc_get_sampling_frequency((struct bt_a2dp_codec_sbc_params *)&configResult->config.media_config->codec_ie[0]);
        a2dp_audio_channels = bt_a2dp_sbc_get_channel_num((struct bt_a2dp_codec_sbc_params *)&configResult->config.media_config->codec_ie[0]);
        a2dp_audio_bits = 16;

        PRINTF("a2dp configure sample rate %dHz, ch %d\r\n", a2dp_audio_sample_rate, a2dp_audio_channels);
#endif
    }
}

void sbc_deconfigured(int err)
{
    if (err == 0)
    {
        /* Stop Audio Player */
        PRINTF("a2dp deconfigure\r\n");
    }
}

void sbc_start_play(int err)
{
    if (err == 0)
    {
        /* Start Audio Player */
        PRINTF("a2dp start playing\r\n");
#if 1 /* Code change for bridge. */
        OSA_SemaphorePost(a2dp_sink_init_done);
#endif
    }
}

void sbc_stop_play(int err)
{
    if (err == 0)
    {
        /* Stop Audio Player */
        PRINTF("a2dp stop playing\r\n");
    }
}

void sbc_streamer_data(uint8_t *data, uint32_t length)
{
    if ((data != NULL) && (length != 0U))
    {
#if 1 /* Code change for bridge. */
        if(a2dp_sink_audio_frame_size == 0)
        {
            a2dp_sink_audio_frame_size = length;
        }
        (void)ring_buf_put(&a2dp_to_ums_audio_buf, data, length);
#endif
    }
}

void app_connected(struct bt_a2dp *a2dp, int err)
{
    if (!err)
    {
        default_a2dp = a2dp;
        PRINTF("a2dp connected success\r\n");
    }
    else
    {
        PRINTF("a2dp connected fail\r\n");
    }
}

void app_disconnected(struct bt_a2dp *a2dp)
{
}

static void app_edgefast_a2dp_init(void)
{
    BaseType_t result = 0;
    result =
        xTaskCreate(AudioTask, "audio", 2048U, NULL, APP_A2DP_STREAMER_SYNC_TASK_PRIORITY, NULL);
    assert(pdPASS == result);
    (void)result;

    struct bt_a2dp_connect_cb connectCb;
    connectCb.connected = app_connected;
    connectCb.disconnected = app_disconnected;

    sbcEndpoint.control_cbs.configured = sbc_configured;
    sbcEndpoint.control_cbs.deconfigured = sbc_deconfigured;
    sbcEndpoint.control_cbs.start_play = sbc_start_play;
    sbcEndpoint.control_cbs.stop_play = sbc_stop_play;
    sbcEndpoint.control_cbs.sink_streamer_data = sbc_streamer_data;
    bt_a2dp_register_endpoint(&sbcEndpoint, BT_A2DP_AUDIO, BT_A2DP_SINK);

    bt_a2dp_register_connect_callback(&connectCb);
}

static void bt_ready(int err)
{
    struct net_buf *buf = NULL;
    struct bt_hci_cp_write_class_of_device *cp;

    if (err) {
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
        sys_put_le24(A2DP_CLASS_OF_DEVICE, &cp->class_of_device[0]);
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

    err = bt_br_set_connectable(true);
    if (err) {
        PRINTF("BR/EDR set/rest connectable failed (err %d)\n", err);
        return;
    }
    err = bt_br_set_discoverable(true);
    if (err) {
        PRINTF("BR/EDR set discoverable failed (err %d)\n", err);
        return;
    }
    PRINTF("BR/EDR set connectable and discoverable done\n");

    bt_sdp_register_service(&a2dp_sink_rec);
    app_edgefast_a2dp_init();
}

void app_a2dp_sink_task(void *pvParameters)
{
    int err = 0;
    (void)err;

    (void)OSA_SemaphoreCreate(a2dp_sink_init_done, 0);

    PRINTF("Bluetooth A2dp Bridge demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err) {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
    }

#if 1 /* Code change for bridge. */
    struct net_buf *buf = NULL;
    struct bt_hci_cp_switch_role *cp;
    char addr_str[BT_ADDR_LE_STR_LEN];

    OSA_SemaphoreWait(a2dp_sink_init_done, osaWaitForever_c);

    buf = bt_hci_cmd_create(BT_HCI_OP_SWITCH_ROLE, sizeof(*cp));
    if (buf != NULL)
    {
        cp = net_buf_add(buf, sizeof(*cp));
        //BD_ADDR for the connected device with which a role switch is to be performed.
        memcpy(&cp->bdaddr, &a2dp_bdaddr, sizeof(a2dp_bdaddr));
        (void)bt_addr_to_str(&a2dp_bdaddr, addr_str, sizeof(addr_str));
        // 0x00
        // Change own Role to Central for this BD_ADDR.
        // 0x01
        // Change own Role to Peripheral for this BD_ADDR.
        cp->role = 0x00;
        err = bt_hci_cmd_send_sync(BT_HCI_OP_SWITCH_ROLE, buf, NULL);
        PRINTF("Switch role for %s, err %d\r\n", addr_str, err);
    }
    else
    {
        PRINTF("Switch role cmd create fail\r\n");
    }

    if (xTaskCreate(unicast_media_sender_task, "unicast_media_sender_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("unicast_media_sender_task creation failed!\r\n");
        while (1)
            ;
    }
#endif

    vTaskDelete(NULL);
}

#endif /* CONFIG_BT_A2DP_SINK */