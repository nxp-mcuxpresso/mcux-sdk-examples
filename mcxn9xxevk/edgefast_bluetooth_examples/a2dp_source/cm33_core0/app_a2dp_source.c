/*
 * Copyright 2020 - 2021, 2024 NXP
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
#include <bluetooth/a2dp.h>
#include <bluetooth/a2dp_codec_sbc.h>
#include <bluetooth/sdp.h>
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "app_connect.h"
#include "a2dp_pl_media_48KHz.h"
#include "app_shell.h"

/* User may need to change it for real production */
#define APP_CLASS_OF_DEVICE (0x200000U)
#define SDP_CLIENT_USER_BUF_LEN		512U
NET_BUF_POOL_FIXED_DEFINE(app_sdp_client_pool, CONFIG_BT_MAX_CONN,
			  SDP_CLIENT_USER_BUF_LEN, NULL);

static uint8_t app_sdp_a2sink_user(struct bt_conn *conn, struct bt_sdp_client_result *result);

static struct bt_sdp_discover_params discov_a2dp_sink =
{
    .uuid = BT_UUID_DECLARE_16(BT_SDP_AUDIO_SINK_SVCLASS),
    .func = app_sdp_a2sink_user,
    .pool = &app_sdp_client_pool,
};

#define APPL_A2DP_MTU   (672U)

static uint32_t a2dp_src_sf;
static uint16_t a2dp_src_num_samples;

static int32_t a2dp_src_sent_ms;
static uint32_t a2dp_src_missed_count;
static volatile uint8_t a2dp_src_playback;
static TimerHandle_t a2dp_src_timer;
static int tone_index;
uint8_t a2dp_src_nc;

#define A2DP_SRC_PERIOD_MS    10

struct bt_a2dp *default_a2dp;
struct bt_a2dp_endpoint *default_a2dp_endpoint;
BT_A2DP_SBC_SOURCE_ENDPOINT(sbcEndpoint, A2DP_SBC_SAMP_FREQ_48000);

uint8_t a2dp_connect_initialized;
struct k_work_delayable configure_sbc_work;
#define CONFIGURE_SBC_DELAY K_MSEC(2000)
static struct bt_sdp_attribute a2dp_source_attrs[] = {
    BT_SDP_NEW_SERVICE,
    BT_SDP_LIST(
        BT_SDP_ATTR_SVCLASS_ID_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 3),
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE(BT_SDP_UUID16),
            BT_SDP_ARRAY_16(BT_SDP_AUDIO_SOURCE_SVCLASS)
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROTO_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 16),
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6),
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16),
                BT_SDP_ARRAY_16(BT_SDP_PROTO_L2CAP)
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16),
                BT_SDP_ARRAY_16(BT_UUID_AVDTP_VAL)
            },
            )
        },
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6),
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16),
                BT_SDP_ARRAY_16(BT_UUID_AVDTP_VAL)
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16),
                BT_SDP_ARRAY_16(0X0100u)
            },
            )
        },
        )
    ),
    BT_SDP_LIST(
        BT_SDP_ATTR_PROFILE_DESC_LIST,
        BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 8),
        BT_SDP_DATA_ELEM_LIST(
        {
            BT_SDP_TYPE_SIZE_VAR(BT_SDP_SEQ8, 6),
            BT_SDP_DATA_ELEM_LIST(
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UUID16),
                BT_SDP_ARRAY_16(BT_SDP_ADVANCED_AUDIO_SVCLASS)
            },
            {
                BT_SDP_TYPE_SIZE(BT_SDP_UINT16),
                BT_SDP_ARRAY_16(0x0103U)
            },
            )
        },
        )
    ),
    BT_SDP_SERVICE_NAME("A2DPSource"),
    BT_SDP_SUPPORTED_FEATURES(0x0001U),
};

static struct bt_sdp_record a2dp_source_rec = BT_SDP_RECORD(a2dp_source_attrs);
static void a2dp_pl_produce_media(void)
{
    uint8_t * media;
    uint16_t  medialen;
    uint8_t malloc = 0;

    /* Music Audio is Stereo */
    medialen = (a2dp_src_num_samples << a2dp_src_nc);

    /* For mono or dual configuration, skip alternative samples */
    if (1 == a2dp_src_nc)
    {
        uint16_t index;

        /* Allocate Memory */
        malloc = 1;
        media = (uint8_t *)pvPortMalloc(medialen);

        if (NULL == media)
        {
            return;
        }

        for (index = 0; index < a2dp_src_num_samples; index++)
        {
            media[(2 * index)] = *((uint8_t *)beethoven + tone_index);
            media[(2 * index) + 1] = *((uint8_t *)beethoven + tone_index + 1);
            /* Update the tone index */
            tone_index += 4u;
            if (tone_index >= sizeof(beethoven))
            {
                tone_index = 0U;
            }
        }
    }
    else
    {
        if ((tone_index + (a2dp_src_num_samples << 2)) > sizeof(beethoven))
        {
            malloc = 1;
            media = (uint8_t *)pvPortMalloc(medialen);
            if (NULL == media)
            {
                PRINTF("Memory Allocation failed in Produce Media\n");
                return;
            }
            memcpy(media, ((uint8_t *)beethoven + tone_index), sizeof(beethoven) - tone_index);
            memcpy(&media[sizeof(beethoven) - tone_index],
                   ((uint8_t *)beethoven),
                   ((a2dp_src_num_samples << 2) - (sizeof(beethoven) - tone_index)));
            /* Update the tone index */
            tone_index = ((a2dp_src_num_samples << 2) - (sizeof(beethoven) - tone_index));
        }
        else
        {
            media = ((uint8_t *)beethoven + tone_index);
            /* Update the tone index */
            tone_index += (a2dp_src_num_samples << 2);
            if (tone_index >= sizeof(beethoven))
            {
                tone_index = 0U;
            }
        }
    }

    /* Give data to callback */
    bt_a2dp_src_media_write(default_a2dp_endpoint, media, medialen);

    if (malloc == 1)
    {
        vPortFree(media);
    }
}

static void a2dp_pl_playback_timeout_handler(TimerHandle_t timer_id)
{
    int32_t now_ms, period_ms;
    TickType_t ticks;

    /* If stopped then return */
    if (0U == a2dp_src_playback)
    {
        return;
    }

    /* Get the current time */
    if (0U != __get_IPSR())
    {
        ticks = xTaskGetTickCountFromISR();
    }
    else
    {
        ticks = xTaskGetTickCount();
    }

    now_ms = ((uint32_t)((uint64_t)(ticks)*1000uL / (uint64_t)configTICK_RATE_HZ));
    period_ms = A2DP_SRC_PERIOD_MS;

    /* Adjust the period */
    if (a2dp_src_sent_ms > 0)
    {
        period_ms = (now_ms > a2dp_src_sent_ms)?
            (now_ms - a2dp_src_sent_ms): A2DP_SRC_PERIOD_MS;
    }

    /* Get the number of samples */
    a2dp_src_num_samples = (period_ms * a2dp_src_sf) / 1000;
    a2dp_src_missed_count += (period_ms * a2dp_src_sf) % 1000;

    /* Raw adjust for the drift */
    while (a2dp_src_missed_count >= 1000)
    {
        a2dp_src_num_samples++;
        a2dp_src_missed_count -= 1000;
    }

    /* Update the sent timestamp */
    a2dp_src_sent_ms = now_ms;

    a2dp_pl_produce_media();
}

static void a2dp_pl_start_playback_timer(void)
{
    a2dp_src_timer = NULL;
    a2dp_src_timer = xTimerCreate("a2dp play", (A2DP_SRC_PERIOD_MS / portTICK_PERIOD_MS),
                                  pdTRUE, NULL, a2dp_pl_playback_timeout_handler);
	a2dp_src_sent_ms = 0;
    xTimerStart(a2dp_src_timer, 0);
}

static void music_control_a2dp_start_callback(int err)
{
    /* Start Audio Source */
    a2dp_src_playback = 1U;
    a2dp_pl_start_playback_timer();
}

void app_endpoint_configured(struct bt_a2dp_endpoint_configure_result *result)
{
    if (result->err == 0)
    {
        k_work_cancel_delayable(&configure_sbc_work);
        default_a2dp_endpoint = &sbcEndpoint;

        a2dp_src_sf = bt_a2dp_sbc_get_sampling_frequency((struct bt_a2dp_codec_sbc_params *)&result->config.media_config->codec_ie[0]);
        a2dp_src_nc = bt_a2dp_sbc_get_channel_num((struct bt_a2dp_codec_sbc_params *)&result->config.media_config->codec_ie[0]);
        bt_a2dp_start(default_a2dp_endpoint);
        PRINTF("a2dp start playing\r\n");
    }
}

void app_configured(int err)
{
    if (err)
    {
        PRINTF("configure fail\r\n");
    }
}

static void configure_sbc_work_handle(struct k_work *work)
{
    if (default_a2dp)
    {
        bt_a2dp_configure(default_a2dp, app_configured);
    }
}

void app_connected(struct bt_a2dp *a2dp, int err)
{
    if (!err)
    {
        default_a2dp = a2dp;
        if (a2dp_connect_initialized)
        {
            a2dp_connect_initialized = 0U;
            bt_a2dp_configure(a2dp, app_configured);
        }
        else
        {
            /* usually the peer headset will configure a2dp_source demo automatically if connecting from peer headset.
             * there are headsets that doesn't do it.
             */
            k_work_init_delayable(&configure_sbc_work, configure_sbc_work_handle);
            k_work_schedule(&configure_sbc_work, CONFIGURE_SBC_DELAY);
        }
        PRINTF("a2dp connected success\r\n");
    }
    else
    {
        if (default_a2dp != NULL)
        {
            default_a2dp = NULL;
        }
        PRINTF("a2dp connected fail\r\n");
    }
}

void app_disconnected(struct bt_a2dp *a2dp)
{
    if (default_a2dp != NULL)
    {
        default_a2dp = NULL;
    }
    a2dp_src_playback = 0U;
    if (a2dp_src_timer != NULL)
    {
        /* Stop Audio Source */
        xTimerStop(a2dp_src_timer, 0);
        xTimerDelete(a2dp_src_timer, 0);
        a2dp_src_timer = NULL;
    }
    PRINTF("a2dp disconnected\r\n");
}

static uint8_t app_sdp_a2sink_user(struct bt_conn *conn,
			   struct bt_sdp_client_result *result)
{
    uint16_t param;
    int res;

    if ((result) && (result->resp_buf))
    {
        PRINTF("sdp success callback\r\n");
        res = bt_sdp_get_proto_param(result->resp_buf, BT_SDP_PROTO_L2CAP, &param);
        if (res < 0)
        {
            PRINTF("PSM is not found\r\n");
            return BT_SDP_DISCOVER_UUID_CONTINUE;
        }
        if (param == BT_UUID_AVDTP_VAL)
        {
            PRINTF ("A2DP Service found. Connecting ...\n");
            a2dp_connect_initialized = 1U;
            default_a2dp = bt_a2dp_connect(default_conn);
            if (NULL == default_a2dp)
            {
                PRINTF ("fail to connect a2dp\r\n");
            }
            return BT_SDP_DISCOVER_UUID_STOP;
        }
        return BT_SDP_DISCOVER_UUID_CONTINUE;
    }
    else
    {
        PRINTF("sdp fail callback\r\n");
        return BT_SDP_DISCOVER_UUID_CONTINUE;
    }
}

void app_sdp_discover_a2dp_sink(void)
{
    int res;
    res = bt_sdp_discover(default_conn, &discov_a2dp_sink);
    if (res)
    {
        PRINTF("SDP discovery failed: result\r\n");
    }
    else
    {
        PRINTF("SDP discovery started\r\n");
    }
}

static void app_edgefast_a2dp_init(void)
{
    struct bt_a2dp_connect_cb connectCb;
    connectCb.connected = app_connected;
    connectCb.disconnected = app_disconnected;

    sbcEndpoint.control_cbs.start_play = music_control_a2dp_start_callback;
    sbcEndpoint.control_cbs.configured = app_endpoint_configured;
    bt_a2dp_register_endpoint(&sbcEndpoint, BT_A2DP_AUDIO, BT_A2DP_SOURCE);

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
        sys_put_le24(APP_CLASS_OF_DEVICE, &cp->class_of_device[0]);
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
    bt_sdp_register_service(&a2dp_source_rec);
    app_edgefast_a2dp_init();
    app_shell_init();
}

void app_a2dp_source_task(void *pvParameters)
{
    int err = 0;
    (void)err;

    PRINTF("Bluetooth A2dp Source demo start...\n");

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err) {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        return;
    }

    vTaskDelete(NULL);
}
