/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <porting.h>

#include "bluetooth/conn.h"
#include "bluetooth/audio/bap.h"
#include "bluetooth/audio/audio.h"
#include "bluetooth/audio/aics.h"
#include "bluetooth/audio/pacs.h"

#include "LC3_api.h"

#include "fsl_os_abstraction.h"

#include "unicast_audio_server.h"

#include "ringtone.h"

#ifndef UNICAST_AUDIO_SYNC_MODE
#define UNICAST_AUDIO_SYNC_MODE 1U
#endif /* UNICAST_AUDIO_SYNC_MODE */

#ifndef UNICAST_AUDIO_SYNC_MODE_TX
#define UNICAST_AUDIO_SYNC_MODE_TX 0U
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

#if UNICAST_AUDIO_SYNC_MODE_TX
#undef UNICAST_AUDIO_SYNC_MODE
#define UNICAST_AUDIO_SYNC_MODE 1U
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

#if defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0)
#include "srCvtFrm.h"
/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#include "audio/bap_iso.h"
#endif /* UNICAST_AUDIO_SYNC_MODE */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_AUDIO_CHANNEL_COUNT     2U
#define BITS_RATES_OF_SAMPLE        16U

#define STREAM_RX_BUF_COUNT (CONFIG_BT_ASCS_ASE_SNK_COUNT * 8U)

#define STREAM_TX_BUF_COUNT (CONFIG_BT_ASCS_ASE_SRC_COUNT * 4U)

typedef void (*codec_rx_callback_t)(uint8_t *rx_buffer);
typedef void (*codec_tx_callback_t)(void);

struct codec_capability
{
    uint32_t frequency;
    uint32_t duration;
    uint32_t frame_bytes;
    uint32_t frame_blocks_per_sdu;
    uint32_t channel_count;
};

enum {
    BT_STREAM_STATE_CONFIGURED,
    BT_STREAM_STATE_QOS,
    BT_STREAM_STATE_ENABLED,
    BT_STREAM_STATE_STARTED,

    /* Total number of flags - must be at the end of the enum */
    BT_STREAM_STATE_NUM_FLAGS,
};

struct stream_state
{
    struct bt_bap_stream stream;
#if 0
    OSA_SEMAPHORE_HANDLE_DEFINE(sem_handle);
    osa_semaphore_handle_t sem;
#endif
    uint16_t max_sdu;
    uint16_t pd;
    ATOMIC_DEFINE(flags, BT_STREAM_STATE_NUM_FLAGS);
    sys_snode_t             node;
};

struct lc3_encoder
{
    LC3_ENCODER_CNTX encoder[MAX_AUDIO_CHANNEL_COUNT];
    INT32* pcm_buf_list_in[MAX_AUDIO_CHANNEL_COUNT];
    UINT8* enc_buf_list_out[MAX_AUDIO_CHANNEL_COUNT];
    INT32 target_enc_bytes[MAX_AUDIO_CHANNEL_COUNT];
    INT32 pcm_buf_in[MAX_AUDIO_CHANNEL_COUNT][LC3_INPUT_FRAME_SIZE_MAX];
    UINT8 enc_buf_out[MAX_AUDIO_CHANNEL_COUNT][LC3_FRAME_SIZE_MAX];
    UINT8 enc_core_buffer[MAX_AUDIO_CHANNEL_COUNT][LC3_ENCODER_CORE_BUFFER_SIZE_MAX];
    UINT8 enc_work_buffer[MAX_AUDIO_CHANNEL_COUNT][LC3_ENCODER_WORK_BUFFER_SIZE_MAX];
    sys_snode_t node;
};

struct lc3_decoder
{
    LC3_DECODER_CNTX decoder[MAX_AUDIO_CHANNEL_COUNT];
    UINT8* enc_buf_list_in[MAX_AUDIO_CHANNEL_COUNT];
    INT32* dec_buf_list_out[MAX_AUDIO_CHANNEL_COUNT];
    UINT8 enc_buf_in[MAX_AUDIO_CHANNEL_COUNT][LC3_FRAME_SIZE_MAX];
    INT32 dec_buf_out[MAX_AUDIO_CHANNEL_COUNT][LC3_INPUT_FRAME_SIZE_MAX];
    UINT8 dec_core_buffer[MAX_AUDIO_CHANNEL_COUNT][LC3_DECODER_CORE_BUFFER_SIZE_MAX];
    UINT8 dec_work_buffer[MAX_AUDIO_CHANNEL_COUNT][LC3_DECODER_WORK_BUFFER_SIZE_MAX];
    sys_snode_t node;
};


struct sync_info
{
    uint32_t iso_interval_us;
    uint32_t sample_rate;
    uint32_t bits_pre_sample;
    float cig_sync_delay_us;
    float sample_duration_us;
};

struct sync_status
{
    uint64_t output_length;
    uint64_t received_length;
    uint32_t presentation_delay_us;
    volatile double resampler_added_samples;

    volatile uint32_t start_slot;
    volatile uint32_t current_slot;

    int32_t mute_frame_samples;
    uint32_t mute_frame_duration_us;

    float sync_offset_us;

    float system_delay_us;

    volatile double resampler_internal_samples;
    double factor_proportional;
    double factor_integral;
    double factor_differential;
    double cumulative_error;
    volatile double output;
};

struct resampler_info
{
    SrCvtFrmCfg_t upSrcCfg;
    SrCvtFrm_t    upSrc;
    int16_t out_buffer[2*(480 + 128)];
    uint32_t out_length;
};

struct conn_state
{
    struct bt_conn * conn;
    struct source_info
    {
        struct stream_state stream;
        struct lc3_encoder encoder;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
        struct sync_status status;
        struct resampler_info resampler[MAX_AUDIO_CHANNEL_COUNT];
        int16_t in_buffer[MAX_AUDIO_CHANNEL_COUNT][(480+128)];
#endif /* UNICAST_AUDIO_SYNC_MODE */
        uint16_t seq_num;
    } src;
    struct sink_info
    {
        struct stream_state stream;
        struct lc3_decoder decoder;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
        struct sync_status status;
        struct resampler_info resampler[MAX_AUDIO_CHANNEL_COUNT];
#endif /* UNICAST_AUDIO_SYNC_MODE */
        int16_t in_buffer[MAX_AUDIO_CHANNEL_COUNT * (480+128)];
    } snk;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    struct sync_info info;
#endif /* UNICAST_AUDIO_SYNC_MODE */
};

struct ringtone_prime_work
{
    struct k_work_delayable work;
    const uint8_t * pcm;
    size_t pcm_length;
    size_t prime_index;
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx);

static int unicast_server_config(struct bt_conn *conn, const struct bt_bap_ep *ep, enum bt_audio_dir dir,
		      const struct bt_audio_codec_cfg *codec_cfg, struct bt_bap_stream **stream,
		      struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp);
static int unicast_server_reconfig(struct bt_bap_stream *stream, enum bt_audio_dir dir,
			const struct bt_audio_codec_cfg *codec_cfg,
			struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp);
static int unicast_server_qos(struct bt_bap_stream *stream, const struct bt_audio_codec_qos *qos,
		   struct bt_bap_ascs_rsp *rsp);
static int unicast_server_enable(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
		      struct bt_bap_ascs_rsp *rsp);
static int unicast_server_start(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp);
static bool valid_metadata_type(uint8_t type, uint8_t len);
static int unicast_server_metadata(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
			struct bt_bap_ascs_rsp *rsp);
static int unicast_server_disable(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp);
static int unicast_server_stop(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp);
static int unicast_server_release(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp);

static void get_capability_from_codec(const struct bt_audio_codec_cfg *codec, struct codec_capability *cap);

static void unicast_server_stream_recv(struct bt_bap_stream *stream,
            const struct bt_iso_recv_info *info,
            struct net_buf *buf);
static void unicast_server_stream_started(struct bt_bap_stream *stream);
static void unicast_server_stream_stoped(struct bt_bap_stream *stream, uint8_t reason);
static void unicast_server_stream_enabled(struct bt_bap_stream *stream);

static int unicast_server_lc3_decoder_init(struct lc3_decoder *decoder);
static int unicast_server_lc3_encoder_init(struct lc3_encoder *encoder);

static void sink_recv_stream_task(void *param);
static int lc3_decode_stream(struct net_buf *buf, uint8_t *pcm);

int BOARD_StartCodec(codec_tx_callback_t tx_callback,codec_rx_callback_t rx_callback, uint32_t simpleBitRate, uint32_t simpleBits);
uint8_t *BOARD_GetRxReadBuffer(void);
void BOARD_PrimeTxWriteBuffer(const uint8_t * buffer, uint32_t length);
int BOARD_StopCodec(void);
void BOARD_StartStream(void);

static void codec_rx_callback(uint8_t *rx_buffer);
static void codec_tx_callback(void);

static int lc3_encode_stream(struct bt_conn * conn, uint8_t *pcm, struct net_buf *buf);
static int stream_send_out(struct bt_conn * conn, struct net_buf *buf);
static void source_send_stream_task(void *param);

static void ringtone_prime_timeout(struct k_work *work);

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
extern void BOARD_SyncTimer_Init(void (*sync_timer_callback)(uint32_t sync_index, uint64_t bclk_count));
void BORAD_SyncTimer_Start(uint32_t sample_rate, uint32_t bits_per_sample);
void BORAD_SyncTimer_Stop(void);
static void sync_timer_callback(uint32_t sync_index, uint64_t bclk_count);
#endif /* UNICAST_AUDIO_SYNC_MODE */

/*******************************************************************************
 * Variables
 ******************************************************************************/

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

static struct bt_gatt_cb gatt_callbacks = {
    .att_mtu_updated = att_mtu_updated,
};

static const struct bt_bap_unicast_server_cb unicast_server_callbacks = {
    .config = unicast_server_config,
    .reconfig = unicast_server_reconfig,
    .qos = unicast_server_qos,
    .enable = unicast_server_enable,
    .start = unicast_server_start,
    .metadata = unicast_server_metadata,
    .disable = unicast_server_disable,
    .stop = unicast_server_stop,
    .release = unicast_server_release,
};

static struct bt_audio_codec_cap unicast_server_codec =
    BT_AUDIO_CODEC_CAP_LC3(BT_AUDIO_CODEC_LC3_FREQ_ANY, BT_AUDIO_CODEC_LC3_DURATION_10,
             BT_AUDIO_CODEC_LC3_CHAN_COUNT_SUPPORT(2), 40u, 120u, 1u,
             (BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL | BT_AUDIO_CONTEXT_TYPE_MEDIA | BT_AUDIO_CONTEXT_TYPE_RINGTONE));

static const struct bt_audio_codec_qos_pref qos_pref = BT_AUDIO_CODEC_QOS_PREF(true, BT_GAP_LE_PHY_2M, 0x02,
                                   10, 40000, 40000, 40000, 40000);

static struct bt_pacs_cap unicast_server_src_cap = {
    .codec_cap = &unicast_server_codec,
};

static struct bt_pacs_cap unicast_server_snk_cap = {
    .codec_cap = &unicast_server_codec,
};

static struct bt_bap_stream_ops unicast_audio_stream_ops = {
    .recv = unicast_server_stream_recv,
    .started = unicast_server_stream_started,
    .stopped = unicast_server_stream_stoped,
    .enabled = unicast_server_stream_enabled,
};

struct conn_state connection;

NET_BUF_POOL_FIXED_DEFINE(rx_pool, STREAM_RX_BUF_COUNT,
			  LC3_INPUT_FRAME_SIZE_MAX * MAX_AUDIO_CHANNEL_COUNT * BITS_RATES_OF_SAMPLE / 8, NULL);

osa_msgq_handle_t rx_stream_queue;
OSA_MSGQ_HANDLE_DEFINE(rx_stream_queue_handle, STREAM_RX_BUF_COUNT, sizeof(void *));

static struct codec_capability snk_config_cap;
static struct codec_capability src_config_cap;


NET_BUF_POOL_FIXED_DEFINE(tx_pool, STREAM_TX_BUF_COUNT,
			  LC3_INPUT_FRAME_SIZE_MAX * MAX_AUDIO_CHANNEL_COUNT * BITS_RATES_OF_SAMPLE / 8, NULL);

osa_msgq_handle_t tx_stream_queue;
OSA_MSGQ_HANDLE_DEFINE(tx_stream_queue_handle, STREAM_TX_BUF_COUNT, sizeof(void *));

struct ringtone_prime_work ringtone_work;

volatile uint8_t sync_timer_started;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
osa_semaphore_handle_t sync_sem;
OSA_SEMAPHORE_HANDLE_DEFINE(sync_sem_handle);
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
volatile uint32_t current_sync_index;
#endif /* UNICAST_AUDIO_SYNC_MODE */

/*******************************************************************************
 * Code
 ******************************************************************************/

int unicast_audio_server_init(void)
{
    int ret = -1;
    osa_status_t err;

    if (xTaskCreate(sink_recv_stream_task, "sink_recv_stream_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("Source stream recv task creation failed!\r\n");
        return ret;
    }

    err = OSA_MsgQCreate((osa_msgq_handle_t)rx_stream_queue_handle, STREAM_RX_BUF_COUNT, sizeof(void*));
    if (KOSA_StatusSuccess != err)
    {
        return -ENOMEM;
    }
    rx_stream_queue = (osa_msgq_handle_t)rx_stream_queue_handle;

    if (xTaskCreate(source_send_stream_task, "source_send_stream_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("Source stream send task creation failed!\r\n");
        return ret;
    }

    err = OSA_MsgQCreate((osa_msgq_handle_t)tx_stream_queue_handle, STREAM_TX_BUF_COUNT, sizeof(void*));
    if (KOSA_StatusSuccess != err)
    {
        return -ENOMEM;
    }
    tx_stream_queue = (osa_msgq_handle_t)tx_stream_queue_handle;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    err = OSA_SemaphoreCreate((osa_semaphore_handle_t)sync_sem_handle, 0);
    if (KOSA_StatusSuccess != err)
    {
        return -ENOMEM;
    }
    sync_sem = (osa_semaphore_handle_t)sync_sem_handle;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    k_work_init_delayable(&ringtone_work.work, ringtone_prime_timeout);

    bt_conn_cb_register(&conn_callbacks);

    bt_gatt_cb_register(&gatt_callbacks);

#if (defined(CONFIG_BT_BAP_UNICAST_SERVER) && (CONFIG_BT_BAP_UNICAST_SERVER > 0))
    ret = bt_bap_unicast_server_register_cb(&unicast_server_callbacks);
    if (ret != 0)
    {
        PRINTF("Failed to register unicast server callbacks: %d\n", ret);
        return ret;
    }
#endif /* CONFIG_BT_BAP_UNICAST_SERVER */
#if (defined(CONFIG_BT_PAC_SNK) && (CONFIG_BT_PAC_SNK > 0))
    bt_pacs_cap_register(BT_AUDIO_DIR_SINK, &unicast_server_snk_cap);
#endif /* CONFIG_BT_PAC_SNK */
#if (defined(CONFIG_BT_PAC_SRC) && (CONFIG_BT_PAC_SRC > 0))
    bt_pacs_cap_register(BT_AUDIO_DIR_SOURCE, &unicast_server_src_cap);
#endif /* CONFIG_BT_PAC_SRC */

#if (defined(CONFIG_BT_PAC_SNK) && (CONFIG_BT_PAC_SNK > 0))
    ret = bt_pacs_set_location(BT_AUDIO_DIR_SINK,
                    (enum bt_audio_location)(BT_AUDIO_LOCATION_FRONT_LEFT |
                    BT_AUDIO_LOCATION_FRONT_RIGHT));
    if (ret != 0)
    {
        PRINTF("Failed to set sink location (err %d)\n", ret);
        return ret;
    }
    ret = bt_pacs_set_supported_contexts(BT_AUDIO_DIR_SINK,
                             (enum bt_audio_context)AVAILABLE_SINK_CONTEXT);
    if (ret != 0) {
        PRINTF("Failed to set sink supported contexts (err %d)\n", ret);
        return ret;
    }
    ret = bt_pacs_set_available_contexts(BT_AUDIO_DIR_SINK,
                            (enum bt_audio_context)AVAILABLE_SINK_CONTEXT);
    if (ret != 0) {
        PRINTF("Failed to set sink available contexts (err %d)\n", ret);
        return ret;
    }
#endif /* CONFIG_BT_PAC_SNK */
#if (defined(CONFIG_BT_PAC_SRC) && (CONFIG_BT_PAC_SRC > 0))
    ret = bt_pacs_set_location(BT_AUDIO_DIR_SOURCE,
                    (enum bt_audio_location)(BT_AUDIO_LOCATION_FRONT_LEFT |
                    BT_AUDIO_LOCATION_FRONT_RIGHT));
    if (ret != 0)
    {
        PRINTF("Failed to set source location (err %d)\n", ret);
        return ret;
    }
    ret = bt_pacs_set_supported_contexts(BT_AUDIO_DIR_SOURCE,
                            (enum bt_audio_context)AVAILABLE_SOURCE_CONTEXT);
    if (ret != 0) {
        PRINTF("Failed to set source supported contexts (err %d)\n", ret);
        return ret;
    }
    ret = bt_pacs_set_available_contexts(BT_AUDIO_DIR_SOURCE,
                            (enum bt_audio_context)AVAILABLE_SOURCE_CONTEXT);
    if (ret != 0) {
        PRINTF("Failed to set source available contexts (err %d)\n", ret);
        return ret;
    }
#endif /* CONFIG_BT_PAC_SRC */

    bt_bap_stream_cb_register(&connection.snk.stream.stream, &unicast_audio_stream_ops);
    bt_bap_stream_cb_register(&connection.src.stream.stream, &unicast_audio_stream_ops);

    return ret;
}

static void ringtone_prime_timeout(struct k_work *work)
{
    struct ringtone_prime_work *ring = CONTAINER_OF(work, struct ringtone_prime_work, work);
    struct net_buf *buf;
    size_t tx_len = 0;
    osa_status_t ret;

    k_work_schedule(&ring->work, BT_MSEC(1));
    do
    {
        buf = net_buf_alloc(&tx_pool, 0U);
        if (NULL == buf)
        {
            break;
        }
        tx_len = src_config_cap.channel_count * (src_config_cap.frequency * src_config_cap.duration * BITS_RATES_OF_SAMPLE / 8000000) * src_config_cap.frame_blocks_per_sdu;
        if ((ring->prime_index + tx_len) > ring->pcm_length)
        {
            ring->prime_index = 0U;
        }
        net_buf_add_mem(buf, &ring->pcm[ring->prime_index], tx_len);

        ret = OSA_MsgQPut(tx_stream_queue, &buf);
        if (KOSA_StatusSuccess != ret)
        {
            net_buf_unref(buf);
        }
        else
        {
            ring->prime_index = ring->prime_index + tx_len;
        }
        buf = NULL;
    } while (true);
}

int unicast_server_start_ringtone(const uint8_t *pcm, size_t pcm_length)
{
    k_work_cancel_delayable(&ringtone_work.work);

    ringtone_work.pcm = pcm;
    ringtone_work.pcm_length = pcm_length;
    ringtone_work.prime_index = 0;

    k_work_schedule(&ringtone_work.work, K_MSEC(0));

    return 0;
}

int unicast_client_stop_ringtone(void)
{
    k_work_cancel_delayable(&ringtone_work.work);

    return 0;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    /* TODO: Connected event */
    if (0 != err)
    {
        return;
    }

    if (NULL == connection.conn)
    {
        connection.conn = conn;
    }
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    /* TODO: Disconnected event */
    if (conn == connection.conn)
    {
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
        if (sync_timer_started > 0)
        {
            sync_timer_started = 0;
            BORAD_SyncTimer_Stop();
        }
#endif /* UNICAST_AUDIO_SYNC_MODE */
        BOARD_StopCodec();
        memset(&connection, 0, sizeof(connection));
        bt_bap_stream_cb_register(&connection.snk.stream.stream, &unicast_audio_stream_ops);
        bt_bap_stream_cb_register(&connection.src.stream.stream, &unicast_audio_stream_ops);
    }
}

static void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
    PRINTF("MTU exchanged: %u/%u\n", tx, rx);
    /* TODO: ATT MTU Updated event */
}

static int unicast_server_lc3_decoder_init(struct lc3_decoder *decoder)
{
    int ret;
    if (NULL == decoder)
    {
        return -EINVAL;
    }

    for (uint32_t i = 0U; i < snk_config_cap.channel_count; i++)
    {
        decoder->enc_buf_list_in[i] = decoder->enc_buf_in[i];
        decoder->dec_buf_list_out[i] = decoder->dec_buf_out[i];
        ret = LC3_decoder_create
            (
                &decoder->decoder[i],
                snk_config_cap.frequency,
                BITS_RATES_OF_SAMPLE,
                1,
                snk_config_cap.duration / 1000 * 10,
                0,
                decoder->dec_core_buffer[i],
                decoder->dec_work_buffer[i],
                &decoder->enc_buf_list_in[i],
                &decoder->dec_buf_list_out[i]
            );
        if(ret != LC3_ENCODER_SUCCESS)
        {
            PRINTF("Failed to create lc3 decoder %d\n", ret);
            return -EIO;
        }
    }

    return 0;
}

static int unicast_server_lc3_encoder_init(struct lc3_encoder *encoder)
{
    int ret;
    if (NULL == encoder)
    {
        return -EINVAL;
    }

    for (uint32_t i = 0U; i < src_config_cap.channel_count; i++)
    {
        encoder->pcm_buf_list_in[i] = encoder->pcm_buf_in[i];
        encoder->enc_buf_list_out[i] = encoder->enc_buf_out[i];
        encoder->target_enc_bytes[i] = src_config_cap.frame_bytes;

        ret = LC3_encoder_create
            (
                &encoder->encoder[i],
                src_config_cap.frequency,
                BITS_RATES_OF_SAMPLE,
                1,
                src_config_cap.duration / 1000 * 10,
                &encoder->target_enc_bytes[i],
                encoder->enc_core_buffer[i],
                encoder->enc_work_buffer[i],
                &encoder->pcm_buf_list_in[i],
                &encoder->enc_buf_list_out[i]
            );
        if(ret != LC3_ENCODER_SUCCESS)
        {
            PRINTF("Failed to create lc3 encoder %d\n", ret);
            return -EIO;
        }
    }
    return 0;
}

static int unicast_server_config(struct bt_conn *conn, const struct bt_bap_ep *ep, enum bt_audio_dir dir,
		      const struct bt_audio_codec_cfg *codec_cfg, struct bt_bap_stream **stream,
		      struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp)
{
    int err = -EINVAL;

    PRINTF("ASE Codec Config: conn %p ep %p dir %u\n", conn, ep, dir);

    if (conn != connection.conn)
    {
        PRINTF("Unknown connection: conn %p\n", conn);
        return -EINVAL;
    }

    if (BT_AUDIO_DIR_SINK == dir)
    {
        get_capability_from_codec(codec_cfg, &snk_config_cap);
        *stream = &connection.snk.stream.stream;
        err = unicast_server_lc3_decoder_init(&connection.snk.decoder);
        atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_CONFIGURED);

    }
    if (BT_AUDIO_DIR_SOURCE == dir)
    {
        get_capability_from_codec(codec_cfg, &src_config_cap);
        *stream = &connection.src.stream.stream;
        err = unicast_server_lc3_encoder_init(&connection.src.encoder);
        atomic_set_bit(connection.src.stream.flags, BT_STREAM_STATE_CONFIGURED);
    }

    if (err < 0)
    {
        return err;
    }

    *pref = qos_pref;

    /* TODO: Handle the configuration from client both for source and sink.
             Check whether the configuration is valid or not.
     */
#if 0
    *stream = stream_alloc(dir);
    if (*stream == NULL) {
        PRINTF("No streams available\n");
        *rsp = BT_BAP_ASCS_RSP(BT_BAP_ASCS_RSP_CODE_NO_MEM, BT_BAP_ASCS_REASON_NONE);

        return -ENOMEM;
    }

    PRINTF("ASE Codec Config stream %p\n", *stream);

    if (dir == BT_AUDIO_DIR_SOURCE) {
        configured_source_stream_count++;
    }
#endif

    return 0;
}

static int unicast_server_reconfig(struct bt_bap_stream *stream, enum bt_audio_dir dir,
			const struct bt_audio_codec_cfg *codec_cfg,
			struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp)
{
    struct codec_capability cap;

    PRINTF("ASE Codec Reconfig: stream %p\n", stream);

    get_capability_from_codec(codec_cfg, &cap);

    /* TODO: Handle the reconfiguration from client both for source and sink.
             Check whether the configuration is valid or not.
     */

    *rsp = BT_BAP_ASCS_RSP(BT_BAP_ASCS_RSP_CODE_CONF_UNSUPPORTED, BT_BAP_ASCS_REASON_NONE);

    /* We only support one QoS at the moment, reject changes */
    return -ENOEXEC;
}

static int unicast_server_qos(struct bt_bap_stream *stream, const struct bt_audio_codec_qos *qos,
		   struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("QoS: stream %p qos %p\n", stream, qos);
    PRINTF("    interval %u framing 0x%02x phy 0x%02x sdu %u "
           "rtn %u latency %u pd %u\n",
           qos->interval, qos->framing, qos->phy, qos->sdu,
           qos->rtn, qos->latency, qos->pd);
    /* TODO: Update the max sdu of stream. */
#if 0
    for (size_t i = 0U; i < configured_source_stream_count; i++) {
        if (stream == &source_streams[i].stream) {
            source_streams[i].max_sdu = qos->sdu;
            break;
        }
    }
#endif
    if (stream == &connection.src.stream.stream)
    {
        connection.src.stream.max_sdu = qos->sdu;
        connection.src.stream.pd = qos->pd;
        atomic_set_bit(connection.src.stream.flags, BT_STREAM_STATE_QOS);
    }
    else
    {
        connection.snk.stream.max_sdu = qos->sdu;
        connection.snk.stream.pd = qos->pd;
        atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_QOS);
    }

    return 0;
}

static int unicast_server_enable(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
		      struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("Enable: stream %p meta_len %u\n", stream, meta_len);
    if (stream == &connection.src.stream.stream)
    {
        atomic_set_bit(connection.src.stream.flags, BT_STREAM_STATE_ENABLED);
    }
    else
    {
        atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_ENABLED);
    }
    /* TODO: Enable the stream */
    return 0;
}

static int unicast_server_start(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("Start: stream %p\n", stream);

    /* TODO: Set the seq_num and len_to_send for the stream.
             Start the stream for source role.
     */
    return 0;
}

static bool valid_metadata_type(uint8_t type, uint8_t len)
{
    switch (type) {
    case BT_AUDIO_METADATA_TYPE_PREF_CONTEXT:
    case BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT:
        if (len != 2) {
            return false;
        }

        return true;
    case BT_AUDIO_METADATA_TYPE_STREAM_LANG:
        if (len != 3) {
            return false;
        }

        return true;
    case BT_AUDIO_METADATA_TYPE_PARENTAL_RATING:
        if (len != 1) {
            return false;
        }

        return true;
    case BT_AUDIO_METADATA_TYPE_EXTENDED: /* 1 - 255 octets */
    case BT_AUDIO_METADATA_TYPE_VENDOR: /* 1 - 255 octets */
        if (len < 1) {
            return false;
        }

        return true;
    case BT_AUDIO_METADATA_TYPE_CCID_LIST: /* 2 - 254 octets */
        if (len < 2) {
            return false;
        }

        return true;
    case BT_AUDIO_METADATA_TYPE_PROGRAM_INFO: /* 0 - 255 octets */
    case BT_AUDIO_METADATA_TYPE_PROGRAM_INFO_URI: /* 0 - 255 octets */
        return true;
    default:
        return false;
    }
}

static bool data_func_cb(struct bt_data *data, void *user_data)
{
	struct bt_bap_ascs_rsp *rsp = (struct bt_bap_ascs_rsp *)user_data;

	if (!BT_AUDIO_METADATA_TYPE_IS_KNOWN((int)data->type)) {
		PRINTF("Invalid metadata type %u or length %u\n", data->type, data->data_len);
		*rsp = BT_BAP_ASCS_RSP(BT_BAP_ASCS_RSP_CODE_METADATA_REJECTED, (enum bt_bap_ascs_reason)data->type);

		return -EINVAL;
	}

	return true;
}

static int unicast_server_metadata(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
			struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Metadata: stream %p meta_len %zu\n", stream, meta_len);

	return bt_audio_data_parse(meta, meta_len, data_func_cb, rsp);
}

static int unicast_server_disable(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("Disable: stream %p\n", stream);
    if (stream == &connection.src.stream.stream)
    {
        atomic_clear_bit(connection.src.stream.flags, BT_STREAM_STATE_ENABLED);
    }
    else
    {
        atomic_clear_bit(connection.snk.stream.flags, BT_STREAM_STATE_ENABLED);
    }
    return 0;
}

static int unicast_server_stop(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("Stop: stream %p\n", stream);

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */
    BOARD_StopCodec();

    if (stream == &connection.src.stream.stream)
    {
        atomic_clear_bit(connection.src.stream.flags, BT_STREAM_STATE_STARTED);
    }
    else
    {
        atomic_clear_bit(connection.snk.stream.flags, BT_STREAM_STATE_STARTED);
    }
    return 0;
}

static int unicast_server_release(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
    PRINTF("Release: stream %p\n", stream);
    if (stream == &connection.src.stream.stream)
    {
        atomic_clear_bit(connection.src.stream.flags, BT_STREAM_STATE_CONFIGURED);
    }
    else
    {
        atomic_clear_bit(connection.snk.stream.flags, BT_STREAM_STATE_CONFIGURED);
    }
    return 0;
}

static void get_capability_from_codec(const struct bt_audio_codec_cfg *codec, struct codec_capability *cap)
{
    int ret;
    enum bt_audio_location location;
    uint32_t tempU32;

    PRINTF("Codec configurations:\n");
    ret = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_config_freq)bt_audio_codec_cfg_get_freq(codec));
    if (ret >= 0)
    {
        cap->frequency = (uint32_t)ret;
    }
    PRINTF("    Frequency %d\n", cap->frequency);
    ret = bt_audio_codec_cfg_get_frame_duration_us(codec);
    if (ret >= 0)
    {
        cap->duration = (uint32_t)ret;
    }
    PRINTF("    Duration %d\n", cap->duration);
    ret = bt_audio_codec_cfg_get_octets_per_frame(codec);
    if (ret >= 0)
    {
        cap->frame_bytes = (uint32_t)ret;
    }
    PRINTF("    Frame bytes %d\n", cap->frame_bytes);
    ret = bt_audio_codec_cfg_get_frame_blocks_per_sdu(codec, 1);
    if (ret >= 0)
    {
        cap->frame_blocks_per_sdu = (uint32_t)ret;
    }
    PRINTF("    Frame blocks per SDU %d\n", cap->frame_blocks_per_sdu);
    ret = bt_audio_codec_cfg_get_chan_allocation(codec, &location);

    if (ret >= 0)
    {
        cap->channel_count = 0U;
        tempU32 = (uint32_t)location;
        while (tempU32 > 0U)
        {
            tempU32 = tempU32 & (tempU32 - 1);
            cap->channel_count ++;
        }
        PRINTF("    Location %d, channel count %d.", (uint32_t)location, cap->channel_count);
        PRINTF("\n");
    }
    else
    {
        PRINTF("    Location is invalid\n");
    }
}

#if 0
static int lc3_decode_stream(struct net_buf *buf, uint8_t *pcm)
{
    /* LC3 decode. */
    INT32 flg_bfi[MAX_AUDIO_CHANNEL_COUNT];
    INT32 dec_byte_count[MAX_AUDIO_CHANNEL_COUNT];
    int lc3_ret;
    int16_t *p = (int16_t *)pcm;
    uint8_t * in;

    if (NULL == pcm)
    {
        return -EINVAL;
    }

    if ((NULL == connection.conn))
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(connection.snk.stream.flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }

    in = buf->data;

    for (size_t index = 0; index < snk_config_cap.frame_blocks_per_sdu; index++)
    {
        p = p + index * (snk_config_cap.frequency * snk_config_cap.duration / 1000000) * snk_config_cap.channel_count;
        in = in + index * snk_config_cap.frame_bytes * snk_config_cap.channel_count;

        for (int i = 0; i < snk_config_cap.channel_count; i++)
        {
            flg_bfi[i] = 0;
            dec_byte_count[i] = snk_config_cap.frame_bytes;

            memcpy(connection.snk.decoder.enc_buf_in[i], &in[i * snk_config_cap.frame_bytes], snk_config_cap.frame_bytes);
            lc3_ret = LC3_decoder_process(&connection.snk.decoder.decoder[i], &flg_bfi[i], &dec_byte_count[i]);
            if(lc3_ret != LC3_DECODER_SUCCESS)
            {
                PRINTF("Fail to decoder stream! %d\n", lc3_ret);
                return lc3_ret;
            }
        }

        for (int j = 0; j < (snk_config_cap.frequency * snk_config_cap.duration / 1000000); j++)
        {
            for(int i = 0; i < snk_config_cap.channel_count; i++)
            {
                p[j * snk_config_cap.channel_count + i] = (int16_t)connection.snk.decoder.dec_buf_out[i][j];
            }
        }
    }
    return 0;
}
#endif

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
/* This value is measured externaly. */
static float System_Sync_offset(int sample_rate)
{
    float us;

    switch (sample_rate)
    {
        case 8000:  us = 666.0;   break;
        case 16000: us = 544.0;   break;
        case 24000: us = 502.0;   break;
        case 32000: us = 444.0;   break;
        case 48000: us = 427.0;   break;
        default:    us = 0.0;     break;
    }

    return us;
}
#endif /* UNICAST_AUDIO_SYNC_MODE */

volatile double channel_delta = 0.0;

static void sink_recv_stream_task(void *param)
{
    struct net_buf *buf;
    struct bt_iso_recv_info * info;
    osa_status_t ret;
    INT32 flg_bfi[MAX_AUDIO_CHANNEL_COUNT];
    INT32 dec_byte_count[MAX_AUDIO_CHANNEL_COUNT];
    int lc3_ret;
    uint8_t * in;
    int16_t * pcm;
    uint32_t tx_len;
    double update_delta = 0.0;
    double update_delta_init = 0.0;
    uint64_t output_length;
    uint64_t received_length;
    double resampler_added_samples;
    double resampler_internal_samples;
    uint32_t ts;
    uint32_t seq;
    uint32_t flag;
    uint32_t handled_sync_index_last = (uint32_t)(-1);
    uint32_t handled_sync_index_current = 0;
    uint32_t handled_flag_last = 0;

    OSA_SR_ALLOC();

    while (true)
    {
        ret = OSA_MsgQGet(rx_stream_queue, &buf, osaWaitForever_c);
        if ( KOSA_StatusSuccess == ret )
        {
            if (NULL == buf)
            {
                continue;
            }

            info = (struct bt_iso_recv_info * ) buf->data;
            ts = info->ts;
            seq = info->seq_num;
            flag = info->flags;

            (void)ts;
            (void)seq;
            (void)flag;

            if (!(info->flags & BT_ISO_FLAGS_VALID))
            {
                for (int i = 0; i < snk_config_cap.channel_count; i++)
                {
                    /* BAD frame 0=>G192_GOOD_FRAME,1=>G192_BAD_FRAME,2=>G192_REDUNDANCY_FRAME */
                    flg_bfi[i] = 1;
                }
                PRINTF("Invalid Frame\n");
            }
            else
            {
                for (int i = 0; i < snk_config_cap.channel_count; i++)
                {
                    /* GOOD frame 0=>G192_GOOD_FRAME,1=>G192_BAD_FRAME,2=>G192_REDUNDANCY_FRAME */
                    flg_bfi[i] = 0;
                }
            }

            in = net_buf_pull(buf, sizeof(*info));

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
            memcpy(&handled_sync_index_current, in, sizeof(handled_sync_index_current));
#if 0
            if (handled_sync_index_current == handled_sync_index_last)
            {
                if ((!(handled_flag_last & BT_ISO_FLAGS_VALID)) && (!(flag & BT_ISO_FLAGS_VALID)))
                {
                    handled_flag_last = flag;
                    continue;
                }
            }
#endif
            in = net_buf_pull(buf, sizeof(handled_sync_index_current));

            handled_sync_index_last = handled_sync_index_current;
            handled_flag_last = flag;

            (void)handled_sync_index_last;
            (void)handled_flag_last;

            output_length = 0;
            received_length = 0;
            resampler_added_samples = 0;
            resampler_internal_samples = 0;

            update_delta = connection.snk.status.output;
            float ideal_samples_per_frame = (snk_config_cap.frequency * snk_config_cap.duration / 1000000);
            float actual_samples_per_frame = (snk_config_cap.frequency * snk_config_cap.duration / 1000000) - update_delta;
            update_delta = (ideal_samples_per_frame - actual_samples_per_frame) / ideal_samples_per_frame;
#endif
            for (int i = 0; i < snk_config_cap.channel_count; i++)
            {
                dec_byte_count[i] = snk_config_cap.frame_bytes;

                if (0 != buf->len)
                {
                    memcpy(connection.snk.decoder.enc_buf_in[i], &in[i * snk_config_cap.frame_bytes], snk_config_cap.frame_bytes);
                    lc3_ret = LC3_decoder_process(&connection.snk.decoder.decoder[i], &flg_bfi[i], &dec_byte_count[i]);
                    if(lc3_ret != LC3_DECODER_SUCCESS)
                    {
                        memset(connection.snk.decoder.dec_buf_out[i], 0, sizeof(INT32) * LC3_INPUT_FRAME_SIZE_MAX);
                        PRINTF("Fail to decoder stream! %d\n", lc3_ret);
                    }
                }
                else
                {
                    memset(connection.snk.decoder.dec_buf_out[i], 0, sizeof(INT32) * LC3_INPUT_FRAME_SIZE_MAX);
                }
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
                OSA_ENTER_CRITICAL();
                if (0 == connection.snk.status.start_slot)
                {
                    /* Resampler have 16 samples delay. */
                    connection.snk.status.system_delay_us = connection.info.bits_pre_sample * connection.info.sample_duration_us;
                    /* LC3 decode delay. 0 for default */
                    connection.snk.status.system_delay_us += 0.0;
                    /* Addition delay */
                    connection.snk.status.system_delay_us += System_Sync_offset(connection.info.sample_rate);

                    connection.snk.status.start_slot = (uint32_t)((info->ts + connection.snk.status.presentation_delay_us - connection.snk.status.system_delay_us - connection.info.cig_sync_delay_us) / connection.info.iso_interval_us);
                    connection.snk.status.mute_frame_duration_us = (uint32_t)((info->ts + connection.snk.status.presentation_delay_us) - (connection.snk.status.start_slot * connection.info.iso_interval_us + connection.info.cig_sync_delay_us + connection.snk.status.system_delay_us));
                    connection.snk.status.mute_frame_samples = (int)(connection.snk.status.mute_frame_duration_us / connection.info.sample_duration_us);
                    connection.snk.status.sync_offset_us = connection.snk.status.mute_frame_duration_us - (connection.snk.status.mute_frame_samples * connection.info.sample_duration_us);

                    BOARD_PrimeTxWriteBuffer(NULL, connection.snk.status.mute_frame_samples * connection.info.bits_pre_sample * snk_config_cap.channel_count/ 8);

                    update_delta_init = - (double)(connection.snk.status.sync_offset_us / connection.info.sample_duration_us);
                    for (int j = 0; j < snk_config_cap.channel_count; j++)
                    {
                        srCvtSetFrcSmpl(&connection.snk.resampler[j].upSrc, update_delta_init);
                    }
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
                    connection.src.status.start_slot = connection.snk.status.start_slot + 4;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */
                }
                OSA_EXIT_CRITICAL();

                srCvtUpdateFreqOffset(&connection.snk.resampler[i].upSrc, update_delta);

                for (int j = 0; j < (snk_config_cap.frequency * snk_config_cap.duration / 1000000); j++)
                {
                    connection.snk.in_buffer[j] = (int16_t)connection.snk.decoder.dec_buf_out[i][j];
                }
                /* resampler */
                connection.snk.resampler[i].out_length = upCvtFrm(&connection.snk.resampler[i].upSrc, connection.snk.in_buffer, connection.snk.resampler[i].out_buffer);

                output_length += connection.snk.resampler[i].out_length;
                received_length += (snk_config_cap.frequency * snk_config_cap.duration / 1000000);
                resampler_added_samples += (double)connection.snk.resampler[i].out_length - (double)((double)snk_config_cap.frequency * (double)snk_config_cap.duration / 1000000.0);
                resampler_internal_samples += srCvtGetFrcSmpl(&connection.snk.resampler[i].upSrc);
#endif /* UNICAST_AUDIO_SYNC_MODE */
            }

            net_buf_unref(buf);

            pcm = (int16_t *)connection.snk.in_buffer;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
            OSA_ENTER_CRITICAL();
            connection.snk.status.output_length += output_length;
            connection.snk.status.received_length += received_length;
            connection.snk.status.resampler_added_samples += resampler_added_samples;
            connection.snk.status.resampler_internal_samples = resampler_internal_samples;
            OSA_EXIT_CRITICAL();

            channel_delta = connection.snk.resampler[0].out_length - connection.snk.resampler[1].out_length;

            tx_len = MIN(connection.snk.resampler[0].out_length, connection.snk.resampler[1].out_length);
            for (int j = 0; j < tx_len; j++)
            {
                for(int i = 0; i < snk_config_cap.channel_count; i++)
                {
                    pcm[j * snk_config_cap.channel_count + i] = (int16_t)connection.snk.resampler[i].out_buffer[j];
                }
            }
            tx_len = tx_len * snk_config_cap.channel_count * connection.info.bits_pre_sample / 8;

#if 0
            PRINTF("slot %d ts %d seq %d flag %d p %d\n", state->src[0].status.current_slot, ts, seq, flag, tx_len);
#endif

#else /* UNICAST_AUDIO_SYNC_MODE */
            tx_len = (snk_config_cap.frequency * snk_config_cap.duration / 1000000);
            for (int j = 0; j < tx_len; j++)
            {
                for(int i = 0; i < snk_config_cap.channel_count; i++)
                {
                    pcm[j * snk_config_cap.channel_count + i] = (int16_t)connection.snk.decoder.dec_buf_out[i][j];
                }
            }
            tx_len = tx_len * snk_config_cap.channel_count * BITS_RATES_OF_SAMPLE / 8;
#endif /* UNICAST_AUDIO_SYNC_MODE */
            BOARD_PrimeTxWriteBuffer((uint8_t *)connection.snk.in_buffer, tx_len);
        }
    }
}

static int lc3_encode_stream(struct bt_conn * conn, uint8_t *pcm, struct net_buf *buf)
{
    int16_t *p = (int16_t *)pcm;
    uint8_t * out;
    int lc3_ret;

    if (NULL == pcm)
    {
        return -EINVAL;
    }

    if ((NULL == connection.conn))
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(connection.src.stream.flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }

    out = net_buf_tail(buf);

    for (size_t index = 0; index < src_config_cap.frame_blocks_per_sdu; index++)
    {
        p = p + index * (src_config_cap.frequency * src_config_cap.duration / 1000000) * src_config_cap.channel_count;
        out = out + index * src_config_cap.frame_bytes * src_config_cap.channel_count;
        for (int j = 0; j < (src_config_cap.frequency * src_config_cap.duration / 1000000); j++)
        {
            for(int i = 0; i < src_config_cap.channel_count; i++)
            {
                connection.src.encoder.pcm_buf_in[i][j] = (int32_t)p[j*src_config_cap.channel_count + i];
            }
        }

        for(int i = 0; i < src_config_cap.channel_count; i++)
        {
            lc3_ret = LC3_encoder_process(&connection.src.encoder.encoder[i]);
            if(lc3_ret != src_config_cap.frame_bytes)
            {
                PRINTF("Channel %d lc3 encode fail! %d\n", i, lc3_ret);
                return lc3_ret;
            }
            memcpy(&out[i * src_config_cap.frame_bytes], connection.src.encoder.enc_buf_out[i], src_config_cap.frame_bytes);
        }
    }

    (void)net_buf_add(buf, src_config_cap.frame_bytes * src_config_cap.channel_count * src_config_cap.frame_blocks_per_sdu);

    return src_config_cap.frame_bytes * src_config_cap.channel_count * src_config_cap.frame_blocks_per_sdu;
}

static int stream_send_out(struct bt_conn * conn, struct net_buf *buf)
{
    int ret;

    if ((NULL == connection.conn))
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(connection.src.stream.flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }

    ret = bt_bap_stream_send(&connection.src.stream.stream, buf, connection.src.seq_num++, BT_ISO_TIMESTAMP_NONE);
    if (ret < 0)
    {
        PRINTF("Fail to send stream (error %d)\n", ret);
        return ret;
    }
    return ret;
}

static void source_send_stream_task(void *param)
{
    struct net_buf *buf;
    uint8_t * buffer = NULL;
    osa_status_t ret;
    int err;
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    double update_delta = 0.0;
    float ideal_samples_per_frame;
    float actual_samples_per_frame;
    uint32_t tx_len;
    int16_t * src;
    uint32_t out_length;
    uint64_t output_length;
    uint64_t received_length;
    double resampler_added_samples;
    double resampler_internal_samples;

    OSA_SR_ALLOC();
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    while (true)
    {
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
        ret = OSA_SemaphoreWait(sync_sem, osaWaitForever_c);
        if ( KOSA_StatusSuccess != ret )
        {
            continue;
        }
        tx_len = (snk_config_cap.frequency * snk_config_cap.duration / 1000000);
#if 0
        PRINTF("index %d ", connection.src.status.current_slot);
#endif
        out_length = 0;
        for (int i = 0; i < snk_config_cap.channel_count;i++)
        {
            out_length += connection.src.resampler[i].out_length;
        }

        while (out_length < (tx_len * snk_config_cap.channel_count))
        {
            /* Reset the buffering size */
            out_length = 0;

            output_length = 0;
            received_length = 0;
            resampler_added_samples = 0;
            resampler_internal_samples = 0;

            update_delta = connection.src.status.output;
            ideal_samples_per_frame = (snk_config_cap.frequency * snk_config_cap.duration / 1000000);
            actual_samples_per_frame = (snk_config_cap.frequency * snk_config_cap.duration / 1000000) + update_delta;
            update_delta = (ideal_samples_per_frame - actual_samples_per_frame) / ideal_samples_per_frame;
            src = (int16_t *)BOARD_GetRxReadBuffer();

            for (int i = 0; i < snk_config_cap.channel_count;i++)
            {
                memcpy(connection.src.in_buffer[i], src, tx_len * connection.info.bits_pre_sample / 8);
            }

            for (int i = 0; i < snk_config_cap.channel_count;i++)
            {
                uint32_t length;
                srCvtUpdateFreqOffset(&connection.src.resampler[i].upSrc, update_delta);
                length = upCvtFrm(&connection.src.resampler[i].upSrc, connection.src.in_buffer[i], &connection.src.resampler[i].out_buffer[connection.src.resampler[i].out_length]);
                output_length += length;
                received_length += tx_len;
                resampler_added_samples += (double)length - (double)tx_len;
                resampler_internal_samples += srCvtGetFrcSmpl(&connection.src.resampler[i].upSrc);
                connection.src.resampler[i].out_length += length;
                out_length += connection.src.resampler[i].out_length;
            }
            OSA_ENTER_CRITICAL();
            connection.src.status.output_length += output_length;
            connection.src.status.received_length += received_length;
            connection.src.status.resampler_added_samples += resampler_added_samples;
            connection.src.status.resampler_internal_samples = resampler_internal_samples;
            OSA_EXIT_CRITICAL();
        }

        if (out_length >= (tx_len * snk_config_cap.channel_count))
        {
            buf = net_buf_alloc(&tx_pool, osaWaitForever_c);
            src = (int16_t *)buf->data;
            for (int j = 0; j < tx_len; j++)
            {
                for(int i = 0; i < snk_config_cap.channel_count; i++)
                {
                    src[j * snk_config_cap.channel_count + i] = connection.src.resampler[i].out_buffer[j];
                }
            }

            for(int i = 0; i < snk_config_cap.channel_count; i++)
            {
                if (connection.src.resampler[i].out_length >= tx_len)
                {
                    connection.src.resampler[i].out_length -= tx_len;
                }
                else
                {
                    connection.src.resampler[i].out_length = 0;
                }
                if (connection.src.resampler[i].out_length > 0)
                {
#if 0
                    for (int j = 0; j < connection.src.resampler[i].out_length; j++)
                    {
                        connection.src.resampler[i].out_buffer[j] = connection.src.resampler[i].out_buffer[j + tx_len];
                    }
#else
                    memcpy(&connection.src.resampler[i].out_buffer[0], &connection.src.resampler[i].out_buffer[tx_len], connection.src.resampler[i].out_length * connection.info.bits_pre_sample / 8);
#endif
                }
            }
            out_length -= (tx_len * snk_config_cap.channel_count);

            buf->len = 0;
            buffer = buf->data;

            net_buf_reserve(buf, BT_ISO_CHAN_SEND_RESERVE);
            err = lc3_encode_stream(NULL, buffer, buf);
            if (err < 0)
            {
                net_buf_unref(buf);
#if 0
                PRINTF("failed 1\n");
#endif
                continue;
            }

            err = stream_send_out(NULL, buf);
            if (err < 0)
            {
                net_buf_unref(buf);
#if 0
                PRINTF("failed 2\n");
#endif
                continue;
            }
#if 0
            PRINTF("sent\n");
#endif
        }
        else
        {
#if 0
            PRINTF("**empty**\n");
#endif
        }
#else
        ret = OSA_MsgQGet(tx_stream_queue, &buf, osaWaitForever_c);
        if ( KOSA_StatusSuccess != ret )
        {
            continue;
        }
        if (NULL == buf)
        {
            continue;
        }

        buf->len = 0;
        buffer = buf->data;
        net_buf_reserve(buf, BT_ISO_CHAN_SEND_RESERVE);
        err = lc3_encode_stream(NULL, buffer, buf);
        if (err < 0)
        {
            net_buf_unref(buf);
            continue;
        }

        err = stream_send_out(NULL, buf);
        if (err < 0)
        {
            net_buf_unref(buf);
            continue;
        }
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */
    }
}

static void codec_tx_callback(void)
{
}

static void codec_rx_callback(uint8_t *rx_buffer)
{
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
#if 0
    static uint32_t sem_post_count = 0;

    osa_status_t ret;

    if (connection.src.status.current_slot >= connection.src.status.start_slot)
    {
        sem_post_count ++;

        while (sem_post_count > 0)
        {
            ret = OSA_SemaphorePost(sync_sem);
            if (KOSA_StatusSuccess != ret)
            {
#if 0
                PRINTF("Post sem failed!\n");
#endif
                break;
            }
            sem_post_count --;
        }
    }
#endif
#else
    struct net_buf *buf;
    size_t tx_len = 0;
    osa_status_t ret;
    uint16_t *src = (uint16_t *)rx_buffer;
    uint16_t *dst = NULL;

    buf = net_buf_alloc(&tx_pool, 0U);
    if (NULL == buf)
    {
        return;
    }

    tx_len = src_config_cap.frequency * src_config_cap.duration / 1000000;
    dst = (uint16_t *)buf->data;

    for (size_t j = 0; j < tx_len; j ++)
    {
        for (size_t i = 0; i < src_config_cap.channel_count; i++)
        {
            dst[j * src_config_cap.channel_count + i] = src[j];
        }
    }
    buf->len = tx_len * src_config_cap.channel_count * 2;

    ret = OSA_MsgQPut(tx_stream_queue, &buf);
    if (KOSA_StatusSuccess != ret)
    {
        net_buf_unref(buf);
    }
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */
}

static void unicast_server_stream_recv(struct bt_bap_stream *stream,
            const struct bt_iso_recv_info *info,
            struct net_buf *buf)
{
    struct net_buf *rx_buf;
    osa_status_t ret;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    uint32_t sync_index = current_sync_index;
#endif /* UNICAST_AUDIO_SYNC_MODE */

    rx_buf = net_buf_alloc(&rx_pool, osaWaitForever_c);
    if (NULL == rx_buf)
    {
        PRINTF("Fail to alloc rx buffer\n");
        return;
    }

    net_buf_add_mem(rx_buf, info, sizeof(*info));
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    net_buf_add_mem(rx_buf, &sync_index, sizeof(sync_index));
#endif /* UNICAST_AUDIO_SYNC_MODE */
    net_buf_add_mem(rx_buf, buf->data, buf->len);
    ret = OSA_MsgQPut(rx_stream_queue, &rx_buf);
    if (KOSA_StatusSuccess != ret)
    {
        net_buf_unref(rx_buf);
        PRINTF(" Fail to put the rx buf %p to queue\n", rx_buf);
        return;
    }

    /* TODO: Handle the received stream data. */
}

#if defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0)
static uint32_t get_cig_sync_delay(void)
{
	struct bt_iso_info iso_info;

	bt_iso_chan_get_info(&connection.src.stream.stream.ep->iso->chan, &iso_info);

	return iso_info.unicast.cig_sync_delay;
}

static uint32_t get_iso_interval(void)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(&connection.src.stream.stream.ep->iso->chan, &iso_info);

	ISO_Interval_us = iso_info.iso_interval * 1250;

	return ISO_Interval_us;
}
#endif /* UNICAST_AUDIO_SYNC_MODE */

static void unicast_server_stream_started(struct bt_bap_stream *stream)
{
	PRINTF("Stream %p started\n", stream);

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started == 0U)
    {
        sync_timer_started = 1U;
        connection.info.bits_pre_sample = BITS_RATES_OF_SAMPLE;
        connection.info.sample_rate = snk_config_cap.frequency;
        connection.info.sample_duration_us = 1000000.0 / (float)connection.info.sample_rate;
        connection.info.cig_sync_delay_us = get_cig_sync_delay();
        connection.info.iso_interval_us = get_iso_interval();

        BORAD_SyncTimer_Start(connection.info.sample_rate, connection.info.bits_pre_sample * snk_config_cap.channel_count);
    }
#else
    BOARD_StartStream();
#endif /* UNICAST_AUDIO_SYNC_MODE */

    /* TODO: Handle the start event of stream */
    if (stream == &connection.src.stream.stream)
    {
        connection.src.seq_num = 0U;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
        for (int i = 0; i < src_config_cap.channel_count; i++)
        {
            connection.src.resampler[i].upSrcCfg.fsIn = connection.info.sample_rate;
            connection.src.resampler[i].upSrcCfg.sfOut = connection.info.sample_rate;
            connection.src.resampler[i].upSrcCfg.phs = 32;
            connection.src.resampler[i].upSrcCfg.fltTaps = 32;
            connection.src.resampler[i].upSrcCfg.frmSizeIn = connection.info.sample_rate / 100;
            connection.src.resampler[i].upSrcCfg.frmSizeOut = connection.info.sample_rate / 100;
            initUpCvtFrm(&connection.src.resampler[i].upSrc, &connection.src.resampler[i].upSrcCfg, 0.0);

            connection.src.resampler[i].out_length = 0;
        }
        connection.src.status.cumulative_error = 0.0;
        connection.src.status.factor_proportional = 5;
        connection.src.status.factor_integral = 3;
        connection.src.status.factor_differential = 3;
        connection.src.status.output = 0.0;

        connection.src.status.presentation_delay_us = stream->qos->pd;
        connection.src.status.resampler_added_samples = 0;
        connection.src.status.start_slot = 0;
        connection.src.status.resampler_internal_samples = 0.0;

        connection.src.status.output_length = 0;
        connection.src.status.received_length = 0;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

        atomic_set_bit(connection.src.stream.flags, BT_STREAM_STATE_STARTED);
    }
    else
    {
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
        for (int i = 0; i < snk_config_cap.channel_count; i++)
        {
            connection.snk.resampler[i].upSrcCfg.fsIn = connection.info.sample_rate;
            connection.snk.resampler[i].upSrcCfg.sfOut = connection.info.sample_rate;
            connection.snk.resampler[i].upSrcCfg.phs = 32;
            connection.snk.resampler[i].upSrcCfg.fltTaps = 32;
            connection.snk.resampler[i].upSrcCfg.frmSizeIn = connection.info.sample_rate / 100;
            connection.snk.resampler[i].upSrcCfg.frmSizeOut = connection.info.sample_rate / 100;
            initUpCvtFrm(&connection.snk.resampler[i].upSrc, &connection.snk.resampler[i].upSrcCfg, 0.0);

            connection.snk.resampler[i].out_length = 0;
        }
        connection.snk.status.cumulative_error = 0.0;
        connection.snk.status.factor_proportional = 5;
        connection.snk.status.factor_integral = 3;
        connection.snk.status.factor_differential = 3;
        connection.snk.status.output = 0.0;

        connection.snk.status.presentation_delay_us = stream->qos->pd;
        connection.snk.status.resampler_added_samples = 0;
        connection.snk.status.start_slot = 0;
        connection.snk.status.resampler_internal_samples = 0.0;

        connection.snk.status.output_length = 0;
        connection.snk.status.received_length = 0;
#endif /* UNICAST_AUDIO_SYNC_MODE */

        atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_STARTED);
    }
}

static void unicast_server_stream_stoped(struct bt_bap_stream *stream, uint8_t reason)
{
    PRINTF("Audio Stream %p stopped with reason 0x%02X\n", stream, reason);

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */
    BOARD_StopCodec();

    if (stream == &connection.src.stream.stream)
    {
        atomic_clear_bit(connection.src.stream.flags, BT_STREAM_STATE_STARTED);
    }
    else
    {
        atomic_clear_bit(connection.snk.stream.flags, BT_STREAM_STATE_STARTED);
    }
    /* TODO: Handle the stop event of stream */
}


static void unicast_server_stream_enabled(struct bt_bap_stream *stream)
{
    /* The unicast server is responsible for starting sink ASEs after the
     * client has enabled them.
     */
    /* TODO: */
    if (stream == &connection.snk.stream.stream)
    {
        const int err = bt_bap_stream_start(stream);

        atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_ENABLED);

        if (err != 0)
        {
            PRINTF("Failed to start stream %p: %d\n", stream, err);
        }
        else
        {
            BOARD_StartCodec(codec_tx_callback, codec_rx_callback, snk_config_cap.frequency, BITS_RATES_OF_SAMPLE);
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
            BOARD_SyncTimer_Init(sync_timer_callback);
#endif /* UNICAST_AUDIO_SYNC_MODE */
            atomic_set_bit(connection.snk.stream.flags, BT_STREAM_STATE_STARTED);
            PRINTF("Start: stream %p\n", stream);
        }
    }
    else
    {
        atomic_set_bit(connection.src.stream.flags, BT_STREAM_STATE_ENABLED);
    }
}

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))

static uint32_t sync_index_cycle = 0;

static void sync_timer_callback(uint32_t sync_index, uint64_t bclk_count)
{
    static double resampler_added_samples_last = 0.0;
    static uint32_t sync_index_last = 0;
    static uint64_t bclk_count_last = 0;
    static double delta_last = 0.0;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    static double src_resampler_added_samples_last = 0.0;
    static uint32_t src_sync_index_last = 0;
    static uint64_t src_bclk_count_last = 0;
    static double src_delta_last = 0.0;
    static uint64_t src_bclk_count_start = 0;
    static uint32_t src_sync_index_start = 0;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    double actual_samples = 0.0;
    double ideal_samples = 0.0;
    double delta = 0.0;
    double delta_current = 0.0;
    double output_max = 0.0;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    static uint32_t sem_post_count = 0;

    osa_status_t ret;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    current_sync_index = sync_index;

    connection.snk.status.current_slot = sync_index;
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    connection.src.status.current_slot = sync_index;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    sync_index_cycle ++;
    sync_index_cycle = sync_index_cycle % 1000;
    if ((sync_timer_started > 0) && (0 != connection.snk.status.start_slot))
    {
        if (sync_index == connection.snk.status.start_slot)
        {
            delta_last = 0.0;
            bclk_count_last = 0.0;
            sync_index_last = 0.0;
            resampler_added_samples_last = 0.0;

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
            src_delta_last = 0.0;
            src_bclk_count_last = 0.0;
            src_sync_index_last = 0.0;
            src_resampler_added_samples_last = 0.0;
            src_bclk_count_start = 0;
            src_sync_index_start = 0;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

            BOARD_StartStream();
        }
        else if (sync_index > connection.snk.status.start_slot)
        {
            actual_samples = (double)bclk_count / (BITS_RATES_OF_SAMPLE);
            actual_samples -= connection.snk.status.mute_frame_samples * (double)snk_config_cap.channel_count;

            ideal_samples = (sync_index - connection.snk.status.start_slot) * (double)connection.info.iso_interval_us - (double)connection.snk.status.mute_frame_duration_us;
            ideal_samples = ideal_samples / connection.info.sample_duration_us;
            ideal_samples = ideal_samples * (double)snk_config_cap.channel_count;

            ideal_samples = ideal_samples + connection.snk.status.resampler_added_samples + connection.snk.status.resampler_internal_samples;

            delta = ideal_samples - actual_samples;

            actual_samples = (double)(bclk_count- bclk_count_last) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = (sync_index - sync_index_last) * (double)connection.info.iso_interval_us;
            ideal_samples = ideal_samples / connection.info.sample_duration_us;
            ideal_samples = ideal_samples * (double)snk_config_cap.channel_count;

            ideal_samples = ideal_samples + connection.snk.status.resampler_added_samples - resampler_added_samples_last  + connection.snk.status.resampler_internal_samples;

            delta_current = ideal_samples - actual_samples;

            sync_index_last = sync_index;
            bclk_count_last = bclk_count;
            resampler_added_samples_last = connection.snk.status.resampler_added_samples;

            connection.snk.status.cumulative_error = delta;
            connection.snk.status.output = connection.snk.status.factor_proportional * (delta_current) + connection.snk.status.factor_integral * connection.snk.status.cumulative_error + connection.snk.status.factor_differential * (delta_current - delta_last);

            delta_last = delta_current;

            output_max = 25.0 / ((double)connection.info.sample_duration_us * 4.0);

            if (connection.snk.status.output > output_max)
            {
                connection.snk.status.output = output_max;
            }
            else if (connection.snk.status.output < -output_max)
            {
                connection.snk.status.output = -output_max;
            }
            else
            {
            }

#if 0
            PRINTF("%d i:%f a:%f o:%f\n", sync_index, ideal_samples, actual_samples, connection.snk.status.output * connection.info.sample_duration_us);
#endif
        }
        else
        {

        }

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
        if (sync_index >= connection.src.status.start_slot)
        {
            if (0 == src_sync_index_start)
            {
                src_bclk_count_start = bclk_count;
                src_sync_index_start = sync_index;
                src_bclk_count_last = bclk_count;
                src_sync_index_last = sync_index;
            }
            actual_samples = (double)(bclk_count - src_bclk_count_start) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = connection.src.status.output_length;

            actual_samples = actual_samples + connection.src.status.resampler_added_samples + connection.src.status.resampler_internal_samples;

            delta = ideal_samples - actual_samples;

            actual_samples = (double)(bclk_count- src_bclk_count_last) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = (sync_index - src_sync_index_last) * (double)connection.info.iso_interval_us;
            ideal_samples = ideal_samples / connection.info.sample_duration_us;
            ideal_samples = ideal_samples * (double)snk_config_cap.channel_count;

            actual_samples = actual_samples + connection.src.status.resampler_added_samples - src_resampler_added_samples_last  + connection.src.status.resampler_internal_samples;

            delta_current = ideal_samples - actual_samples;

            src_sync_index_last = sync_index;
            src_bclk_count_last = bclk_count;
            src_resampler_added_samples_last = connection.src.status.resampler_added_samples;

            connection.src.status.cumulative_error = delta;
            connection.src.status.output = connection.src.status.factor_proportional * (delta_current) + connection.src.status.factor_integral * connection.src.status.cumulative_error + connection.src.status.factor_differential * (delta_current - src_delta_last);

            src_delta_last = delta_current;

            output_max = 25.0 / ((double)connection.info.sample_duration_us * 4.0);

            if (connection.src.status.output > output_max)
            {
                connection.src.status.output = output_max;
            }
            else if (connection.src.status.output < -output_max)
            {
                connection.src.status.output = -output_max;
            }
            else
            {
            }

            sem_post_count ++;

            while (sem_post_count > 0)
            {
                ret = OSA_SemaphorePost(sync_sem);
                if (KOSA_StatusSuccess != ret)
                {
#if 0
                    PRINTF("Post sem failed!\n");
#endif
                    break;
                }
                sem_post_count --;
            }
#if 0
            PRINTF("%d i:%f a:%f o:%f\n", sync_index, ideal_samples, actual_samples, connection.src.status.output * connection.info.sample_duration_us);
#endif
        }
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    }

}

#endif /* UNICAST_AUDIO_SYNC_MODE */
