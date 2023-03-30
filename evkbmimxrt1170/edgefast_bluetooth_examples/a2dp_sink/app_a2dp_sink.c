/*
 * Copyright 2020 - 2022 NXP
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
#include "fsl_adapter_audio.h"
#include "fsl_codec_common.h"
#include "fsl_codec_adapter.h"
#include "fsl_debug_console.h"
#include "app_connect.h"

#define A2DP_CLASS_OF_DEVICE (0x200404U)
#define APP_A2DP_STREAMER_SYNC_TASK_PRIORITY (5U)
#define A2DP_CODEC_DAC_VOLUME (100U) /* Range: 0 ~ 100 */
#define A2DP_CODEC_HP_VOLUME  (70U)  /* Range: 0 ~ 100 */

extern uint32_t BOARD_SwitchAudioFreq(uint32_t sampleRate);

struct bt_a2dp *default_a2dp;
struct bt_a2dp_endpoint *default_a2dp_endpoint;
static uint32_t audio_start;
static QueueHandle_t audio_sem;
BT_A2DP_SBC_SINK_ENDPOINT(sbcEndpoint);

extern hal_audio_config_t audioTxConfig;
extern codec_config_t boardCodecConfig;
AT_NONCACHEABLE_SECTION_ALIGN(static HAL_AUDIO_HANDLE_DEFINE(audio_tx_handle), 4);
static codec_handle_t codec_handle;

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

static void tx_callback(hal_audio_handle_t handle, hal_audio_status_t completionStatus, void *callbackParam)
{
    app_audio_streamer_task_signal();
}

void sbc_configured(struct bt_a2dp_endpoint_configure_result *configResult)
{
    if (configResult->err == 0)
    {
        default_a2dp_endpoint = &sbcEndpoint;

        audioTxConfig.sampleRate_Hz  = bt_a2dp_sbc_get_sampling_frequency((struct bt_a2dp_codec_sbc_params *)&configResult->config.media_config->codec_ie[0]);
        audioTxConfig.lineChannels = (hal_audio_channel_t)bt_a2dp_sbc_get_channel_num((struct bt_a2dp_codec_sbc_params *)&configResult->config.media_config->codec_ie[0]);
        audioTxConfig.srcClock_Hz = BOARD_SwitchAudioFreq(audioTxConfig.sampleRate_Hz);

        PRINTF("a2dp configure sample rate %dHz\r\n", audioTxConfig.sampleRate_Hz);

        HAL_AudioTxInit((hal_audio_handle_t)&audio_tx_handle[0], &audioTxConfig);
        HAL_AudioTxInstallCallback((hal_audio_handle_t)&audio_tx_handle[0], tx_callback, NULL);

        if (CODEC_Init(&codec_handle, &boardCodecConfig) != kStatus_Success)
        {
            PRINTF("codec init failed!\r\n");
        }
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        CODEC_SetFormat(&codec_handle, audioTxConfig.srcClock_Hz, audioTxConfig.sampleRate_Hz, audioTxConfig.bitWidth);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeDAC, A2DP_CODEC_DAC_VOLUME);
        CODEC_SetVolume(&codec_handle, kCODEC_VolumeHeadphoneLeft | kCODEC_VolumeHeadphoneRight, A2DP_CODEC_HP_VOLUME);
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, false);
    }
}

void sbc_deconfigured(int err)
{
    if (err == 0)
    {
        audio_start = 0;
        /* Stop Audio Player */
        PRINTF("a2dp deconfigure\r\n");
        CODEC_SetMute(&codec_handle, kCODEC_PlayChannelHeadphoneRight | kCODEC_PlayChannelHeadphoneLeft, true);
        HAL_AudioTxDeinit((hal_audio_handle_t)&audio_tx_handle[0]);
        (void)BOARD_SwitchAudioFreq(0U);
    }
}

void sbc_start_play(int err)
{
    if (err == 0)
    {
        audio_start = 1;
        /* Start Audio Player */
        PRINTF("a2dp start playing\r\n");
    }
}

void sbc_stop_play(int err)
{
    if (err == 0)
    {
        audio_start = 0;
        /* Stop Audio Player */
        PRINTF("a2dp stop playing\r\n");
        HAL_AudioTransferAbortSend((hal_audio_handle_t)&audio_tx_handle[0]);
    }
}

void sbc_streamer_data(uint8_t *data, uint32_t length)
{
    if ((data != NULL) && (length != 0U))
    {
        hal_audio_transfer_t xfer;

        if(0 == audio_start)
        {
            /*return;*/
        }

        xfer.dataSize       = length;
        xfer.data           = data;
        if (kStatus_HAL_AudioSuccess != HAL_AudioTransferSendNonBlocking((hal_audio_handle_t)&audio_tx_handle[0], &xfer))
        {
            PRINTF("prime fail\r\n");
        }
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
    audio_start = 0;
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

    /* Initializate BT Host stack */
    err = bt_enable(bt_ready);
    if (err) {
        PRINTF("Bluetooth init failed (err %d)\n", err);
        while (1)
        {
            vTaskDelay(2000);
        }
    }

    vTaskDelete(NULL);
}
