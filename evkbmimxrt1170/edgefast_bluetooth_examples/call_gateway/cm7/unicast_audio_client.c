/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <porting.h>

#include "bluetooth/conn.h"
#include "bluetooth/audio/bap.h"
#include "bluetooth/audio/vcp.h"
#include "bluetooth/audio/media_proxy.h"
#include "bluetooth/audio/bap_lc3_preset.h"

#include "LC3_api.h"

#include "fsl_os_abstraction.h"

#include "call_gateway.h"

#include "unicast_audio_client.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef UNICAST_AUDIO_SYNC_MODE
#define UNICAST_AUDIO_SYNC_MODE 1U
#endif /* UNICAST_AUDIO_SYNC_MODE */

#if defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0)
#include "srCvtFrm.h"
/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#include "audio/bap_iso.h"
#endif /* UNICAST_AUDIO_SYNC_MODE */

#ifndef UNICAST_AUDIO_SERVER_COUNT
#define UNICAST_AUDIO_SERVER_COUNT 2U
#endif /* UNICAST_AUDIO_SERVER_COUNT */

#define MAX_AUDIO_CHANNEL_COUNT     2U
#define BITS_RATES_OF_SAMPLE        16U

#define STREAM_RX_BUF_COUNT (CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT * 8U)

#define STREAM_TX_BUF_COUNT (CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT * 4U)

typedef void (*codec_rx_callback_t)(uint8_t *rx_buffer);
typedef void (*codec_tx_callback_t)(void);

enum {
    BT_STREAM_STATE_CONFIGURED,
    BT_STREAM_STATE_QOS,
    BT_STREAM_STATE_ENABLED,
    BT_STREAM_STATE_STARTED,
    BT_STREAM_STATE_RELEASED,

    /* Total number of flags - must be at the end of the enum */
    BT_STREAM_STATE_NUM_FLAGS,
};

struct stream_state
{
    struct bt_bap_stream stream;
    OSA_SEMAPHORE_HANDLE_DEFINE(sem_handle);
    osa_semaphore_handle_t sem;
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

struct codec_capability
{
    uint32_t frequency;
    uint32_t duration;
    uint32_t frame_bytes;
    uint32_t frame_blocks_per_sdu;
    uint32_t channel_count;
};

struct pacs_capability
{
    uint32_t frequency_bitmap;
    uint32_t duration_bitmap;
    uint32_t frame_bytes_min;
    uint32_t frame_bytes_max;
    uint32_t frame_blocks_per_sdu;
    uint32_t channel_count;
    uint32_t pref_context;
    uint32_t context;
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

struct source_info
{
    struct stream_state *stream;
    struct lc3_decoder *decoder;
    struct bt_bap_ep *ep;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    struct sync_status status;
    struct resampler_info resampler[MAX_AUDIO_CHANNEL_COUNT];
#endif /* UNICAST_AUDIO_SYNC_MODE */
    int16_t in_buffer[MAX_AUDIO_CHANNEL_COUNT * (480+128)];
};

struct sink_info
{
    struct stream_state *stream;
    struct lc3_encoder *encoder;
    struct bt_bap_ep *ep;
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    struct sync_status status;
    struct resampler_info resampler[MAX_AUDIO_CHANNEL_COUNT];
    int16_t in_buffer[MAX_AUDIO_CHANNEL_COUNT][(480+128)];
#endif /* UNICAST_AUDIO_SYNC_MODE */
    uint16_t seq_num;
};

struct conn_state
{
    struct bt_conn *conn;
    struct source_info src[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT];
    struct sink_info snk[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
    uint8_t src_cap_support;
    uint8_t snk_cap_support;
    struct pacs_capability src_pac;
    struct pacs_capability snk_pac;
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

enum {
    BT_STREAM_RINGTONE_MODE_NONE = 0,
    BT_STREAM_RINGTONE_MODE_LOCAL = 1,
    BT_STREAM_RINGTONE_MODE_REMOTE = 2
};

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);

static void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx);

static void vcs_client_discover(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t vocs_count, uint8_t aics_count);
static void vcs_client_state(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t volume, uint8_t mute);
static void vcs_client_flags(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t flags);

static void mcs_local_player_instance(struct media_player *player, int err);
static void mcs_media_state_recv(struct media_player *player, int err, uint8_t state);
static void mcs_command_recv(struct media_player *player, int err, const struct mpl_cmd_ntf *result);

static void unicast_client_location(struct bt_conn *conn, enum bt_audio_dir dir, enum bt_audio_location loc);
static void unicast_client_available_contexts(struct bt_conn *conn, enum bt_audio_context snk_ctx, enum bt_audio_context src_ctx);
static void unicast_client_pac_record(struct bt_conn *conn, enum bt_audio_dir dir, const struct bt_audio_codec_cap *codec_cap);
static void unicast_client_endpoint(struct bt_conn *conn, enum bt_audio_dir dir, struct bt_bap_ep *ep);

static void unicast_client_discover_sink_callback(struct bt_conn *conn, int err, enum bt_audio_dir dir);
static void unicast_client_discover_source_callback(struct bt_conn *conn, int err, enum bt_audio_dir dir);
static int unicast_client_discover_sink(struct bt_conn * conn);
static int unicast_client_discover_source(struct bt_conn * conn);

static void unicast_client_stream_configured(struct bt_bap_stream *stream, const struct bt_audio_codec_qos_pref *pref);
static void unicast_client_stream_qos_set(struct bt_bap_stream *stream);
static void unicast_client_stream_enabled(struct bt_bap_stream *stream);
static void unicast_client_stream_started(struct bt_bap_stream *stream);
static void unicast_client_stream_metadata_updated(struct bt_bap_stream *stream);
static void unicast_client_stream_disabled(struct bt_bap_stream *stream);
static void unicast_client_stream_stopped(struct bt_bap_stream *stream, uint8_t reason);
static void unicast_client_stream_released(struct bt_bap_stream *stream);
static void unicast_client_stream_recv(struct bt_bap_stream *stream, const struct bt_iso_recv_info *info, struct net_buf *buf);

static void parse_pacs_capability(const struct bt_audio_codec_cap *codec_cap, struct pacs_capability *cap);
static void get_capability_from_codec(const struct bt_audio_codec_cfg *codec_cfg, struct codec_capability *cap);
static int capability_compare(struct codec_capability *required_cap, struct codec_capability *discovered_cap);
static int pac_capability_compare(struct codec_capability *required_cap, struct pacs_capability *discovered_cap);

static int unicast_client_lc3_encoder_init(struct lc3_encoder *encoder);
static int unicast_client_lc3_decoder_init(struct lc3_decoder *decoder);

static int lc3_encode_stream(struct bt_conn * conn, uint8_t *pcm, struct net_buf *buf);
static int stream_send_out(struct bt_conn * conn, struct net_buf *buf);

static void ringtone_prime_timeout(struct k_work *work);
static void source_send_stream_task(void *param);

int BOARD_StartCodec(codec_tx_callback_t tx_callback,codec_rx_callback_t rx_callback, uint32_t simpleBitRate, uint32_t simpleBits);
uint8_t *BOARD_GetRxReadBuffer(void);
void BOARD_PrimeTxWriteBuffer(const uint8_t * buffer, uint32_t length);
int BOARD_StopCodec(void);
void BOARD_StartStream(void);

static void codec_rx_callback(uint8_t *rx_buffer);
static void codec_tx_callback(void);

static int lc3_decode_stream(struct bt_conn * conn, struct net_buf *buf, uint8_t *pcm);
static void sink_recv_stream_task(void *param);

static int bt_audio_codec_cfg_meta_set_val(struct bt_audio_codec_cfg *codec_cfg, uint8_t type,
			       const uint8_t *data, size_t data_len);

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

#if (defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0))
static struct bt_vcp_vol_ctlr_cb vcs_client_callbacks = {
    .discover = vcs_client_discover,
    .state = vcs_client_state,
    .flags = vcs_client_flags,
};
#endif /* CONFIG_BT_VCP_VOL_CTLR */

static struct media_proxy_ctrl_cbs mcs_callbacks = {
    .local_player_instance = mcs_local_player_instance,
    .media_state_recv = mcs_media_state_recv,
    .command_recv = mcs_command_recv,
};

static struct media_player *mcs_player;

static struct bt_bap_unicast_client_cb unicast_client_callbacks = {
    .location = unicast_client_location,
    .available_contexts = unicast_client_available_contexts,
    .pac_record = unicast_client_pac_record,
    .endpoint = unicast_client_endpoint,
};

static struct bt_bap_unicast_group *unicast_group = NULL;

static struct stream_state streams[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT + CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
BT_FIFO_DEFINE(free_streams);

static struct lc3_decoder decoders[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
BT_FIFO_DEFINE(free_decoders);

static struct lc3_encoder encoders[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT];
BT_FIFO_DEFINE(free_encoders);

struct conn_state server_state[UNICAST_AUDIO_SERVER_COUNT];

static struct bt_bap_stream_ops unicast_audio_stream_ops = {
    .configured = unicast_client_stream_configured,
    .qos_set = unicast_client_stream_qos_set,
    .enabled = unicast_client_stream_enabled,
    .started = unicast_client_stream_started,
    .metadata_updated = unicast_client_stream_metadata_updated,
    .disabled = unicast_client_stream_disabled,
    .stopped = unicast_client_stream_stopped,
    .released = unicast_client_stream_released,
    .recv = unicast_client_stream_recv
};

/* Select a codec configuration to apply that is mandatory to support by both client and server.
 * Allows this sample application to work without logic to parse the codec capabilities of the
 * server and selection of an appropriate codec configuration.
 */
static struct bt_bap_lc3_preset codec_configuration = BT_BAP_LC3_UNICAST_PRESET_16_2_1(
    BT_AUDIO_LOCATION_FRONT_LEFT | BT_AUDIO_LOCATION_FRONT_RIGHT, BT_AUDIO_CONTEXT_TYPE_UNSPECIFIED);

static struct codec_capability src_cap_required;
static struct codec_capability snk_cap_required;

static unicast_client_discover_done_callback_t discover_done;

struct ringtone_prime_work ringtone_work;

NET_BUF_POOL_FIXED_DEFINE(tx_pool, STREAM_TX_BUF_COUNT,
			  LC3_INPUT_FRAME_SIZE_MAX * MAX_AUDIO_CHANNEL_COUNT * BITS_RATES_OF_SAMPLE / 8, NULL);

osa_msgq_handle_t tx_stream_queue;
OSA_MSGQ_HANDLE_DEFINE(tx_stream_queue_handle, STREAM_TX_BUF_COUNT, sizeof(void *));


NET_BUF_POOL_FIXED_DEFINE(rx_pool, STREAM_RX_BUF_COUNT,
			  LC3_INPUT_FRAME_SIZE_MAX * MAX_AUDIO_CHANNEL_COUNT * BITS_RATES_OF_SAMPLE / 8, NULL);

osa_msgq_handle_t rx_stream_queue;
OSA_MSGQ_HANDLE_DEFINE(rx_stream_queue_handle, STREAM_RX_BUF_COUNT, sizeof(void *));

volatile uint8_t sync_timer_started;

static volatile uint8_t ringtone_mode = BT_STREAM_RINGTONE_MODE_NONE;

static volatile uint8_t voice_is_started = 0U;

static volatile uint8_t voice_is_hold = 0U;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
volatile uint32_t current_sync_index;
#endif /* UNICAST_AUDIO_SYNC_MODE */

/*******************************************************************************
 * Code
 ******************************************************************************/

int unicast_audio_client_init(unicast_client_discover_done_callback_t callback)
{
    int ret = -1;
    osa_status_t err;

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

    if (xTaskCreate(sink_recv_stream_task, "sink_recv_stream_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 1, NULL) != pdPASS)
    {
        PRINTF("Sink stream recv task creation failed!\r\n");
        return ret;
    }

    err = OSA_MsgQCreate((osa_msgq_handle_t)rx_stream_queue_handle, STREAM_RX_BUF_COUNT, sizeof(void*));
    if (KOSA_StatusSuccess != err)
    {
        return -ENOMEM;
    }
    rx_stream_queue = (osa_msgq_handle_t)rx_stream_queue_handle;

    discover_done = callback;

    k_work_init_delayable(&ringtone_work.work, ringtone_prime_timeout);

    PRINTF("Get required Source Capability from codec. ");
    get_capability_from_codec(&codec_configuration.codec_cfg, &src_cap_required);
    PRINTF("Get required Sink Capability from codec. ");
    get_capability_from_codec(&codec_configuration.codec_cfg, &snk_cap_required);

    codec_configuration.qos.sdu = codec_configuration.qos.sdu * src_cap_required.channel_count * src_cap_required.frame_blocks_per_sdu;

    bt_conn_cb_register(&conn_callbacks);

    bt_fifo_init(&free_streams);
    for (size_t i = 0; i < ARRAY_SIZE(streams); i++)
    {
        bt_bap_stream_cb_register(&streams[i].stream, &unicast_audio_stream_ops);
        bt_fifo_put(&free_streams, &streams[i]);
    }

    bt_fifo_init(&free_decoders);
    for (size_t i = 0; i < ARRAY_SIZE(decoders); i++)
    {
        bt_fifo_put(&free_decoders, &decoders[i]);
    }

    bt_fifo_init(&free_encoders);
    for (size_t i = 0; i < ARRAY_SIZE(encoders); i++)
    {
        bt_fifo_put(&free_encoders, &encoders[i]);
    }

    bt_gatt_cb_register(&gatt_callbacks);

#if (defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0))
    bt_vcp_vol_ctlr_cb_register(&vcs_client_callbacks);
#endif /* CONFIG_BT_VCP_VOL_CTLR */

#if (defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0))
    ret = media_proxy_pl_init();
    if(ret)
    {
        PRINTF("\nMCS server init fail %d\n", ret);
        return ret;
    }
    ret = media_proxy_ctrl_register(&mcs_callbacks);
    if(ret)
    {
        PRINTF("\nMCS ctrl register fail %d\n", ret);
        return ret;
    }
#endif /* CONFIG_BT_MCS */

#if (defined(CONFIG_BT_BAP_UNICAST_CLIENT) && (CONFIG_BT_BAP_UNICAST_CLIENT > 0))
    ret = bt_bap_unicast_client_register_cb(&unicast_client_callbacks);
    if (ret != 0) {
        PRINTF("Failed to register unicast client callbacks: %d\n", ret);
        return ret;
    }
#endif /* CONFIG_BT_BAP_UNICAST_CLIENT */

    return ret;
}

int unicast_client_create_group(void)
{
    size_t params_count = 0U;
    size_t streams_count = 0U;
    struct bt_bap_unicast_group_stream_pair_param pair_params[UNICAST_AUDIO_SERVER_COUNT];
    struct bt_bap_unicast_group_stream_param stream_params[UNICAST_AUDIO_SERVER_COUNT * (CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT + CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT)];
    struct bt_bap_unicast_group_param param;
    int err;

    if (unicast_group != NULL)
    {
        return 0;
    }

    for (uint32_t index = 0U; index < UNICAST_AUDIO_SERVER_COUNT; index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }
            for (uint32_t dir_index = 0U; dir_index < MAX(CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT, CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT); dir_index++)
            {
                pair_params[params_count].tx_param = NULL;
                pair_params[params_count].rx_param = NULL;

                if (dir_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT)
                {
                    if (NULL != server_state[index].snk[dir_index].stream)
                    {
                        atomic_clear_bit(server_state[index].snk[dir_index].stream->flags, BT_STREAM_STATE_RELEASED);
                        stream_params[streams_count].stream = &server_state[index].snk[dir_index].stream->stream;
                        stream_params[streams_count].qos = &codec_configuration.qos;
                        pair_params[params_count].tx_param = &stream_params[streams_count];
                        streams_count++;
                    }
                }

                if (dir_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT)
                {
                    if (NULL != server_state[index].src[dir_index].stream)
                    {
                        atomic_clear_bit(server_state[index].src[dir_index].stream->flags, BT_STREAM_STATE_RELEASED);
                        stream_params[streams_count].stream = &server_state[index].src[dir_index].stream->stream;
                        stream_params[streams_count].qos = &codec_configuration.qos;
                        pair_params[params_count].rx_param = &stream_params[streams_count];
                        streams_count++;
                    }
                }

                if ((NULL != pair_params[params_count].tx_param) || (NULL != pair_params[params_count].rx_param))
                {
                    params_count++;
                }
            }
        }
    }

    if (params_count < 1U)
    {
        return -EINVAL;
    }

    param.params = pair_params;
    param.params_count = params_count;
    param.packing = BT_ISO_PACKING_SEQUENTIAL;

    err = bt_bap_unicast_group_create(&param, &unicast_group);
    if (err != 0) {
        PRINTF("Could not create unicast group (err %d)\n", err);
        return err;
    }

    return 0;
}

int unicast_client_delete_group(void)
{
    int err;

    if (unicast_group == NULL)
    {
        return -ESRCH;
    }

    {
        err = bt_bap_unicast_group_delete(unicast_group);
        if (err != 0) {
            PRINTF("Could not delete unicast group (err %d)\n", err);
            return err;
        }
        unicast_group = NULL;
    }

    return 0;
}

int unicast_client_release_streams(void)
{
    struct conn_state *state = NULL;
    int err = -ENOTCONN;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
            {
                if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                {
                    continue;
                }
                if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_RELEASED))
                {
                    err = bt_bap_stream_release(&state->src[ep_index].stream->stream);
                    if (0 != err)
                    {
                        PRINTF("Fail to enable stream (err %d)\n", err);
                        return err;
                    }

                    do
                    {
                        osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                        if (KOSA_StatusSuccess != osa_ret)
                        {
                            PRINTF("Fail to wait stream configuration sem!\n");
                            return -EIO;
                        }
                    } while (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_RELEASED));
                }
            }
            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
            {
                if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                {
                    continue;
                }
                if (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_RELEASED))
                {
                    err = bt_bap_stream_release(&state->snk[ep_index].stream->stream);
                    if (0 != err)
                    {
                        PRINTF("Fail to enable stream (err %d)\n", err);
                        return err;
                    }

                    do
                    {
                        osa_ret = OSA_SemaphoreWait(state->snk[ep_index].stream->sem, osaWaitForever_c);
                        if (KOSA_StatusSuccess != osa_ret)
                        {
                            PRINTF("Fail to wait stream configuration sem!\n");
                            return -EIO;
                        }
                    } while (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_RELEASED));
                }
            }
        }
    }

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
            {
                if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                {
                    continue;
                }
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED);
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_QOS);
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED);
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_STARTED);
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_RELEASED);
            }
            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
            {
                if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                {
                    continue;
                }
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED);
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_QOS);
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_ENABLED);
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_STARTED);
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_RELEASED);
            }
        }
    }

    return err;
}

int unicast_client_configure_streams(void)
{
    struct conn_state *state = NULL;
    int err = 0;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
            {
                if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                {
                    continue;
                }
                if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED))
                {
                    err = bt_bap_stream_config(state->conn, &state->src[ep_index].stream->stream, state->src[ep_index].ep, &codec_configuration.codec_cfg);
                    if (0 != err)
                    {
                        PRINTF("Fail to config stream (err %d)\n", err);
                        return err;
                    }

                    do
                    {
                        osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                        if (KOSA_StatusSuccess != osa_ret)
                        {
                            PRINTF("Fail to wait stream configuration sem!\n");
                            return -EIO;
                        }
                    } while (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED));
                }
            }

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
            {
                if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                {
                    continue;
                }
                if(!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED))
                {
                    err = bt_bap_stream_config(state->conn, &state->snk[ep_index].stream->stream, state->snk[ep_index].ep, &codec_configuration.codec_cfg);
                    if (0 != err)
                    {
                        PRINTF("Fail to config stream (err %d)\n", err);
                        return err;
                    }

                    do
                    {
                        osa_ret = OSA_SemaphoreWait(state->snk[ep_index].stream->sem, osaWaitForever_c);
                        if (KOSA_StatusSuccess != osa_ret)
                        {
                            PRINTF("Fail to wait stream configuration sem!\n");
                            return -EIO;
                        }
                    } while (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_CONFIGURED));
                }
            }
        }
    }

    return err;
}

int unicast_client_set_stream_qos(void)
{
    struct conn_state *state = NULL;
    int err = 0;
    uint8_t need_to_wait = 0U;

    if (NULL == unicast_group)
    {
        return -ESRCH;
    }

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
            {
                if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                {
                    continue;
                }
                atomic_clear_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_QOS);
            }

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
            {
                if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                {
                    continue;
                }
                atomic_clear_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_QOS);
            }

            err = bt_bap_stream_qos(state->conn, unicast_group);
            if (0 != err)
            {
                PRINTF("Fail to set stream QoS (err %d)\n", err);
                return err;
            }

            do
            {
                if (need_to_wait > 0U)
                {
                    OSA_TimeDelay(1U);
                    need_to_wait = 0U;
                }

                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                    {
                        continue;
                    }
                    if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_QOS))
                    {
                        need_to_wait = 1U;
                        continue;
                    }
                }

                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                    {
                        continue;
                    }
                    if (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_QOS))
                    {
                        need_to_wait = 1U;
                        continue;
                    }
                }
            } while (need_to_wait == 1U);
        }
    }
    return err;
}

int unicast_client_enable_stream_unidirectional(uint8_t is_tx, uint16_t context)
{
    struct conn_state *state = NULL;
    int err = -ENOTCONN;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            if (is_tx == 0)
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                    {
                        continue;
                    }
                    if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED))
                    {
                        bt_audio_codec_cfg_meta_set_val(&codec_configuration.codec_cfg, BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT, (uint8_t *)&context, sizeof(context));
                        err = bt_bap_stream_enable(&state->src[ep_index].stream->stream, codec_configuration.codec_cfg.meta, codec_configuration.codec_cfg.meta_len);
                        if (0 != err)
                        {
                            PRINTF("Fail to enable stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED));
                    }
                }
            }
            else
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                    {
                        continue;
                    }
                    if (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_ENABLED))
                    {
                        bt_audio_codec_cfg_meta_set_val(&codec_configuration.codec_cfg, BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT, (uint8_t *)&context, sizeof(context));
                        err = bt_bap_stream_enable(&state->snk[ep_index].stream->stream, codec_configuration.codec_cfg.meta, codec_configuration.codec_cfg.meta_len);
                        if (0 != err)
                        {
                            PRINTF("Fail to enable stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->snk[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_ENABLED));
                    }
                }
            }
        }
    }

    return err;
}

int unicast_client_enable_streams(uint16_t tx_context, uint16_t rx_context)
{
    int err;

    err = unicast_client_enable_stream_unidirectional(0, rx_context);
    if (err >= 0)
    {
        err = unicast_client_enable_stream_unidirectional(1, tx_context);
    }

    return err;
}

int unicast_client_metadata_unidirectional(uint8_t is_tx, uint16_t context)
{
    struct conn_state *state = NULL;
    int err = -ENOTCONN;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            if (is_tx == 0)
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                    {
                        continue;
                    }

                    bt_audio_codec_cfg_meta_set_val(&codec_configuration.codec_cfg, BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT, (uint8_t *)&context, sizeof(context));
                    err = bt_bap_stream_metadata(&state->src[ep_index].stream->stream, codec_configuration.codec_cfg.meta, codec_configuration.codec_cfg.meta_len);
                    if (0 != err)
                    {
                        PRINTF("Fail to set stream metadata (err %d)\n", err);
                        return err;
                    }
                }
            }
            else
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                    {
                        continue;
                    }

                    bt_audio_codec_cfg_meta_set_val(&codec_configuration.codec_cfg, BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT, (uint8_t *)&context, sizeof(context));
                    err = bt_bap_stream_metadata(&state->snk[ep_index].stream->stream, codec_configuration.codec_cfg.meta, codec_configuration.codec_cfg.meta_len);
                    if (0 != err)
                    {
                        PRINTF("Fail to set stream metadata (err %d)\n", err);
                        return err;
                    }
                }
            }
        }
    }

    return err;
}

int unicast_client_metadata(uint16_t tx_context, uint16_t rx_context)
{
    int err;

    err = unicast_client_metadata_unidirectional(0, rx_context);
    if (err >= 0)
    {
        err = unicast_client_metadata_unidirectional(1, tx_context);
    }

    return err;
}

int unicast_client_disable_stream_unidirectional(uint8_t is_tx)
{
    struct conn_state *state = NULL;
    int err;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            state = &server_state[index];

            if (is_tx == 0)
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream))
                    {
                        continue;
                    }
                    if (atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED))
                    {
                        err = bt_bap_stream_disable(&state->src[ep_index].stream->stream);
                        if (0 != err)
                        {
                            PRINTF("Fail to disable stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED));
                    }
                }
            }
            else
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream))
                    {
                        continue;
                    }
                    if (atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_ENABLED))
                    {
                        err = bt_bap_stream_disable(&state->snk[ep_index].stream->stream);
                        if (0 != err)
                        {
                            PRINTF("Fail to disable stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->snk[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_ENABLED));
                    }
                }
            }
        }
    }
    return 0;
}

int unicast_client_disable_streams(void)
{
    int err;

    err = unicast_client_disable_stream_unidirectional(0);
    if (err >= 0)
    {
        err = unicast_client_disable_stream_unidirectional(1);
    }
    return err;
}

static void init_net_buf_simple_from_codec_cfg_meta(struct net_buf_simple *buf,
					       struct bt_audio_codec_cfg *codec_cfg)
{
	buf->__buf = codec_cfg->meta;
	buf->data = codec_cfg->meta;
	buf->size = sizeof(codec_cfg->meta);
	buf->len = codec_cfg->meta_len;
}

static int bt_audio_codec_cfg_meta_set_val(struct bt_audio_codec_cfg *codec_cfg, uint8_t type,
			       const uint8_t *data, size_t data_len)
{
	CHECKIF(codec_cfg == NULL) {
		PRINTF("codec_cfg is NULL");
		return -EINVAL;
	}

	CHECKIF(data == NULL) {
		PRINTF("data is NULL");
		return -EINVAL;
	}

	CHECKIF(data_len == 0U || data_len > UINT8_MAX) {
		PRINTF("Invalid data_len %zu", data_len);
		return -EINVAL;
	}

	for (uint16_t i = 0U; i < codec_cfg->meta_len;) {
		uint8_t *len = &codec_cfg->meta[i++];
		const uint8_t data_type = codec_cfg->meta[i++];
		const uint8_t value_len = *len - sizeof(data_type);

		if (data_type == type) {
			uint8_t *value = &codec_cfg->meta[i];

			if (data_len == value_len) {
				memcpy(value, data, data_len);
			} else {
				const int16_t diff = data_len - value_len;
				uint8_t *old_next_data_start;
				uint8_t *new_next_data_start;
				uint8_t data_len_to_move;

				/* Check if this is the last value in the buffer */
				if (value + value_len == codec_cfg->meta + codec_cfg->meta_len) {
					data_len_to_move = 0U;
				} else {
					old_next_data_start = value + value_len + 1;
					new_next_data_start = value + data_len + 1;
					data_len_to_move = codec_cfg->meta_len -
							   (old_next_data_start - codec_cfg->meta);
				}

				if (diff < 0) {
					/* In this case we need to move memory around after the copy
					 * to fit the new shorter data
					 */

					memcpy(value, data, data_len);
					if (data_len_to_move > 0U) {
						memmove(new_next_data_start, old_next_data_start,
							data_len_to_move);
					}
				} else {
					/* In this case we need to move memory around before
					 * the copy to fit the new longer data
					 */
					if ((codec_cfg->meta_len + diff) >
					    ARRAY_SIZE(codec_cfg->meta)) {
						PRINTF("Cannot fit meta_len %zu in buf with len "
							"%u and size %u",
							data_len, codec_cfg->meta_len,
							ARRAY_SIZE(codec_cfg->meta));
						return -ENOMEM;
					}

					if (data_len_to_move > 0) {
						memmove(new_next_data_start, old_next_data_start,
							data_len_to_move);
					}

					memcpy(value, data, data_len);
				}

				codec_cfg->meta_len += diff;
				*len += diff;
			}

			return codec_cfg->meta_len;
		}

		i += value_len;
	}

	/* If we reach here, we did not find the data in the buffer, so we simply add it */
	if ((codec_cfg->meta_len + data_len) <= ARRAY_SIZE(codec_cfg->meta)) {
		struct net_buf_simple buf;

		init_net_buf_simple_from_codec_cfg_meta(&buf, codec_cfg);

		net_buf_simple_add_u8(&buf, data_len + sizeof(type));
		net_buf_simple_add_u8(&buf, type);
		if (data_len > 0) {
			net_buf_simple_add_mem(&buf, data, data_len);
		}
		codec_cfg->meta_len = buf.len;
	} else {
		PRINTF("Cannot fit meta %zu in codec_cfg with len %u and size %u", data_len,
			codec_cfg->meta_len, ARRAY_SIZE(codec_cfg->meta));
		return -ENOMEM;
	}

	return codec_cfg->meta_len;
}

static int unicast_client_lc3_decoder_init(struct lc3_decoder *decoder)
{
    int ret;
    if (NULL == decoder)
    {
        return -EINVAL;
    }

    for (uint32_t i = 0U; i < src_cap_required.channel_count; i++)
    {
        decoder->enc_buf_list_in[i] = decoder->enc_buf_in[i];
        decoder->dec_buf_list_out[i] = decoder->dec_buf_out[i];
        ret = LC3_decoder_create
            (
                &decoder->decoder[i],
                src_cap_required.frequency,
                BITS_RATES_OF_SAMPLE,
                1,
                src_cap_required.duration / 1000 * 10,
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

static int unicast_client_lc3_encoder_init(struct lc3_encoder *encoder)
{
    int ret;
    if (NULL == encoder)
    {
        return -EINVAL;
    }

    for (uint32_t i = 0U; i < snk_cap_required.channel_count; i++)
    {
        encoder->pcm_buf_list_in[i] = encoder->pcm_buf_in[i];
        encoder->enc_buf_list_out[i] = encoder->enc_buf_out[i];
        encoder->target_enc_bytes[i] = snk_cap_required.frame_bytes;

        ret = LC3_encoder_create
            (
                &encoder->encoder[i],
                snk_cap_required.frequency,
                BITS_RATES_OF_SAMPLE,
                1,
                snk_cap_required.duration / 1000 * 10,
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

int unicast_client_start_stream_unidirectional(uint8_t is_tx)
{
    struct conn_state *state = NULL;
    int err = -ENOTCONN;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            if ((0U == server_state[index].src_cap_support) || (0U == server_state[index].snk_cap_support))
            {
                continue;
            }

            state = &server_state[index];

            if (is_tx == 0)
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream) || (NULL == state->src[ep_index].ep))
                    {
                        continue;
                    }

                    if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_STARTED))
                    {
                        err = bt_bap_stream_start(&state->src[ep_index].stream->stream);
                        if (0 != err)
                        {
                            PRINTF("Fail to start stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                            if (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_ENABLED))
                            {
                                return -ENOTCONN;
                            }
                        } while (!atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_STARTED));
                    }
                }
            }
            else
            {
                err = 0;
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream) || (NULL == state->snk[ep_index].ep))
                    {
                        continue;
                    }
                    if (!atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_STARTED))
                    {
                        err = bt_bap_stream_start(&state->snk[ep_index].stream->stream);
                        if ((0 != err) && (-EBUSY != err))
                        {
                            PRINTF("Fail to start stream (err %d)\n", err);
                        }
                        err = 0;
                    }
                }
            }
        }
    }

    return err;
}

int unicast_client_start_streams(void)
{
    int err;

    err = unicast_client_start_stream_unidirectional(0);
    if (err >= 0)
    {
        err = unicast_client_start_stream_unidirectional(1);
    }

    return err;
}

int unicast_client_stop_stream_unidirectional(uint8_t is_tx)
{
    struct conn_state *state = NULL;
    int err;
    osa_status_t osa_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            state = &server_state[index];

            if (is_tx == 0)
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                {
                    if ((NULL == state->src[ep_index].stream))
                    {
                        continue;
                    }
                    if (atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_STARTED))
                    {
                        err = bt_bap_stream_stop(&state->src[ep_index].stream->stream);
                        if (0 != err)
                        {
                            PRINTF("Fail to stop stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->src[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (atomic_test_bit(state->src[ep_index].stream->flags, BT_STREAM_STATE_STARTED));
                    }
                }
            }
            else
            {
                for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                {
                    if ((NULL == state->snk[ep_index].stream))
                    {
                        continue;
                    }
                    if (atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_STARTED))
                    {
                        err = bt_bap_stream_stop(&state->snk[ep_index].stream->stream);
                        if (0 != err)
                        {
                            PRINTF("Fail to stop stream (err %d)\n", err);
                            return err;
                        }

                        do
                        {
                            osa_ret = OSA_SemaphoreWait(state->snk[ep_index].stream->sem, osaWaitForever_c);
                            if (KOSA_StatusSuccess != osa_ret)
                            {
                                PRINTF("Fail to wait stream configuration sem!\n");
                                return -EIO;
                            }
                        } while (atomic_test_bit(state->snk[ep_index].stream->flags, BT_STREAM_STATE_STARTED));
                    }
                }
            }
        }
    }
    return 0;
}

int unicast_client_stop_streams(void)
{
    int err;

    err = unicast_client_stop_stream_unidirectional(0);

    if (err >= 0)
    {
        err = unicast_client_stop_stream_unidirectional(1);
    }

    return err;
}

#if 0
static int lc3_decode_stream(struct bt_conn * conn, struct net_buf *buf, uint8_t *pcm)
{
    /* LC3 decode. */
    INT32 flg_bfi[MAX_AUDIO_CHANNEL_COUNT];
    INT32 dec_byte_count[MAX_AUDIO_CHANNEL_COUNT];
    int lc3_ret;
    struct conn_state *state = NULL;
    int16_t *p = (int16_t *)pcm;
    uint8_t * in;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if ((conn == server_state[index].conn) || ((NULL == conn) && (NULL != server_state[index].conn)))
        {
            state = &server_state[index];
            break;
        }
    }

    if (NULL == pcm)
    {
        return -EINVAL;
    }

    if (NULL == state)
    {
        return -EINVAL;
    }

    if (NULL == state->src[0].stream)
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(state->src[0].stream->flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }

    in = buf->data;

    for (size_t index = 0; index < src_cap_required.frame_blocks_per_sdu; index++)
    {
        p = p + index * (src_cap_required.frequency * src_cap_required.duration / 1000000) * src_cap_required.channel_count;
        in = in + index * src_cap_required.frame_bytes * src_cap_required.channel_count;

        for (int i = 0; i < src_cap_required.channel_count; i++)
        {
            flg_bfi[i] = 0;
            dec_byte_count[i] = src_cap_required.frame_bytes;

            memcpy(state->src[0].decoder->enc_buf_in[i], &in[i * src_cap_required.frame_bytes], src_cap_required.frame_bytes);
            lc3_ret = LC3_decoder_process(&state->src[0].decoder->decoder[i], &flg_bfi[i], &dec_byte_count[i]);
            if(lc3_ret != LC3_DECODER_SUCCESS)
            {
                PRINTF("Fail to decoder stream! %d\n", lc3_ret);
                return lc3_ret;
            }
        }

        for (int j = 0; j < (src_cap_required.frequency * src_cap_required.duration / 1000000); j++)
        {
            for(int i = 0; i < src_cap_required.channel_count; i++)
            {
                p[j * src_cap_required.channel_count + i] = (int16_t)state->src[0].decoder->dec_buf_out[i][j];
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
    struct conn_state *state = NULL;
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

            state = NULL;
            for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
            {
                if (NULL != server_state[index].conn)
                {
                    state = &server_state[index];
                    break;
                }
            }
            if (NULL == state)
            {
                net_buf_unref(buf);
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
                for (int i = 0; i < state->src_pac.channel_count; i++)
                {
                    /* BAD frame 0=>G192_GOOD_FRAME,1=>G192_BAD_FRAME,2=>G192_REDUNDANCY_FRAME */
                    flg_bfi[i] = 1;
                }
                PRINTF("Invalid Frame\n");
            }
            else
            {
                for (int i = 0; i < state->src_pac.channel_count; i++)
                {
                    /* GOOD frame 0=>G192_GOOD_FRAME,1=>G192_BAD_FRAME,2=>G192_REDUNDANCY_FRAME */
                    flg_bfi[i] = 0;
                }
            }

            {
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

                update_delta = state->src[0].status.output;
                float ideal_samples_per_frame = (src_cap_required.frequency * src_cap_required.duration / 1000000);
                float actual_samples_per_frame = (src_cap_required.frequency * src_cap_required.duration / 1000000) - update_delta;
                update_delta = (ideal_samples_per_frame - actual_samples_per_frame) / ideal_samples_per_frame;
#endif
                for (int i = 0; i < src_cap_required.channel_count; i++)
                {
                    dec_byte_count[i] = src_cap_required.frame_bytes;

                    if (0 != buf->len)
                    {
                        memcpy(state->src[0].decoder->enc_buf_in[i], &in[i * src_cap_required.frame_bytes], src_cap_required.frame_bytes);
                        lc3_ret = LC3_decoder_process(&state->src[0].decoder->decoder[i], &flg_bfi[i], &dec_byte_count[i]);
                        if(lc3_ret != LC3_DECODER_SUCCESS)
                        {
                            memset(state->src[0].decoder->dec_buf_out[i], 0, sizeof(INT32) * LC3_INPUT_FRAME_SIZE_MAX);
                            PRINTF("Fail to decoder stream! %d\n", lc3_ret);
                        }
                    }
                    else
                    {
                        memset(state->src[0].decoder->dec_buf_out[i], 0, sizeof(INT32) * LC3_INPUT_FRAME_SIZE_MAX);
                    }
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
                    OSA_ENTER_CRITICAL();
                    if (0 == state->src[0].status.start_slot)
                    {
                        /* Resampler have 16 samples delay. */
                        state->src[0].status.system_delay_us = state->info.bits_pre_sample * state->info.sample_duration_us;
                        /* LC3 decode delay. 0 for default */
                        state->src[0].status.system_delay_us += 0.0;
                        /* Addition delay */
                        state->src[0].status.system_delay_us += System_Sync_offset(state->info.sample_rate);

                        state->src[0].status.start_slot = (uint32_t)((info->ts + state->src[0].status.presentation_delay_us - state->src[0].status.system_delay_us - state->info.cig_sync_delay_us) / state->info.iso_interval_us);
                        state->src[0].status.mute_frame_duration_us = (uint32_t)((info->ts + state->src[0].status.presentation_delay_us) - (state->src[0].status.start_slot * state->info.iso_interval_us + state->info.cig_sync_delay_us + state->src[0].status.system_delay_us));
                        state->src[0].status.mute_frame_samples = (int)(state->src[0].status.mute_frame_duration_us / state->info.sample_duration_us);
                        state->src[0].status.sync_offset_us = state->src[0].status.mute_frame_duration_us - (state->src[0].status.mute_frame_samples * state->info.sample_duration_us);

                        BOARD_PrimeTxWriteBuffer(NULL, state->src[0].status.mute_frame_samples * state->info.bits_pre_sample * src_cap_required.channel_count/ 8);

                        update_delta_init = - (double)(state->src[0].status.sync_offset_us / state->info.sample_duration_us);
                        for (int j = 0; j < src_cap_required.channel_count; j++)
                        {
                            srCvtSetFrcSmpl(&state->src[0].resampler[j].upSrc, update_delta_init);
                        }
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
                        state->snk[0].status.start_slot = state->src[0].status.start_slot + 4;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */
                    }
                    OSA_EXIT_CRITICAL();

                    srCvtUpdateFreqOffset(&state->src[0].resampler[i].upSrc, update_delta);

                    for (int j = 0; j < (src_cap_required.frequency * src_cap_required.duration / 1000000); j++)
                    {
                        state->src[0].in_buffer[j] = (int16_t)state->src[0].decoder->dec_buf_out[i][j];
                    }
                    /* resampler */
                    state->src[0].resampler[i].out_length = upCvtFrm(&state->src[0].resampler[i].upSrc, state->src[0].in_buffer, state->src[0].resampler[i].out_buffer);

                    output_length += state->src[0].resampler[i].out_length;
                    received_length += (src_cap_required.frequency * src_cap_required.duration / 1000000);
                    resampler_added_samples += (double)state->src[0].resampler[i].out_length - (double)((double)src_cap_required.frequency * (double)src_cap_required.duration / 1000000.0);
                    resampler_internal_samples += srCvtGetFrcSmpl(&state->src[0].resampler[i].upSrc);
#endif /* UNICAST_AUDIO_SYNC_MODE */
                }

                net_buf_unref(buf);

                pcm = (int16_t *)state->src[0].in_buffer;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
                OSA_ENTER_CRITICAL();
                state->src[0].status.output_length += output_length;
                state->src[0].status.received_length += received_length;
                state->src[0].status.resampler_added_samples += resampler_added_samples;
                state->src[0].status.resampler_internal_samples = resampler_internal_samples;
                OSA_EXIT_CRITICAL();

                channel_delta = state->src[0].resampler[0].out_length - state->src[0].resampler[1].out_length;

                tx_len = MIN(state->src[0].resampler[0].out_length, state->src[0].resampler[1].out_length);
                for (int j = 0; j < tx_len; j++)
                {
                    for(int i = 0; i < src_cap_required.channel_count; i++)
                    {
                        pcm[j * src_cap_required.channel_count + i] = (int16_t)state->src[0].resampler[i].out_buffer[j];
                    }
                }
                tx_len = tx_len * src_cap_required.channel_count * state->info.bits_pre_sample / 8;

#if 0
                PRINTF("slot %d ts %d seq %d flag %d p %d\n", state->src[0].status.current_slot, ts, seq, flag, tx_len);
#endif

#else /* UNICAST_AUDIO_SYNC_MODE */
                tx_len = (src_cap_required.frequency * src_cap_required.duration / 1000000);
                for (int j = 0; j < tx_len; j++)
                {
                    for(int i = 0; i < src_cap_required.channel_count; i++)
                    {
                        pcm[j * src_cap_required.channel_count + i] = (int16_t)state->src[0].decoder->dec_buf_out[i][j];
                    }
                }
                tx_len = tx_len * src_cap_required.channel_count * BITS_RATES_OF_SAMPLE / 8;
#endif /* UNICAST_AUDIO_SYNC_MODE */
            }
#if 1
            if ((0 == voice_is_hold) && (BT_STREAM_RINGTONE_MODE_NONE == ringtone_mode))
#else
            if ((0 == voice_is_hold))
#endif
            {
                BOARD_PrimeTxWriteBuffer((uint8_t *)state->src[0].in_buffer, tx_len);
            }
            else
            {
                net_buf_unref(buf);
                tx_len = (src_cap_required.frequency * src_cap_required.duration / 1000000);
                tx_len = tx_len * src_cap_required.channel_count * BITS_RATES_OF_SAMPLE / 8;
                BOARD_PrimeTxWriteBuffer(NULL, tx_len);
            }
        }
    }
}

static void source_send_stream_task(void *param)
{
    struct net_buf *buf;
    uint8_t * buffer = NULL;
    osa_status_t ret;
    int err;

    while (true)
    {
        ret = OSA_MsgQGet(tx_stream_queue, &buf, osaWaitForever_c);
        if ( KOSA_StatusSuccess == ret )
        {
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
            if (0 != voice_is_hold)
            {
                buf->len = 0;
            }
            err = stream_send_out(NULL, buf);
            if (err < 0)
            {
                net_buf_unref(buf);
                continue;
            }
        }
    }
}

static void ringtone_prime_timeout(struct k_work *work)
{
    struct ringtone_prime_work *ring = CONTAINER_OF(work, struct ringtone_prime_work, work);
    struct net_buf *buf;
    size_t tx_len = 0;
    osa_status_t ret;

    if (BT_STREAM_RINGTONE_MODE_REMOTE == ringtone_mode)
    {
        k_work_schedule(&ring->work, BT_MSEC(1));
    }
    else
    {
        k_work_cancel_delayable(&ring->work);
        return;
    }

    do
    {
        buf = net_buf_alloc(&tx_pool, 0U);
        if (NULL == buf)
        {
            break;
        }
        tx_len = snk_cap_required.channel_count * (snk_cap_required.frequency * snk_cap_required.duration * BITS_RATES_OF_SAMPLE / 8000000) * snk_cap_required.frame_blocks_per_sdu;
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

int unicast_client_start_voice(void)
{
    struct conn_state *state = NULL;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if ((NULL != server_state[index].conn))
        {
            state = &server_state[index];
            break;
        }
    }

    if (0 == (state->snk_pac.context & BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL))
    {
        return -1;
    }

    voice_is_started = 1U;

    return 0;
}

int unicast_client_stop_voice(void)
{
    voice_is_started = 0U;

    return 0;
}

int unicast_client_hold(void)
{
    voice_is_hold = 1U;

    return 0;
}

int unicast_client_retrieve(void)
{
    voice_is_hold = 0U;

    return 0;
}

int unicast_client_start_ringtone(const uint8_t *pcm, size_t pcm_length)
{
    struct conn_state *state = NULL;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if ((NULL != server_state[index].conn))
        {
            state = &server_state[index];
            break;
        }
    }

    k_work_cancel_delayable(&ringtone_work.work);

    ringtone_work.pcm = pcm;
    ringtone_work.pcm_length = pcm_length;
    ringtone_work.prime_index = 0;

    if (state->snk_pac.context & BT_AUDIO_CONTEXT_TYPE_RINGTONE)
    {
        ringtone_mode = BT_STREAM_RINGTONE_MODE_REMOTE;
        k_work_schedule(&ringtone_work.work, K_MSEC(0));
    }
    else
    {
        ringtone_mode = BT_STREAM_RINGTONE_MODE_LOCAL;
    }

    return 0;
}

int unicast_client_stop_ringtone(void)
{
    k_work_cancel_delayable(&ringtone_work.work);

    ringtone_work.pcm = NULL;
    ringtone_work.pcm_length = 0;
    ringtone_work.prime_index = 0;

    ringtone_mode = BT_STREAM_RINGTONE_MODE_NONE;

    return 0;
}

static int lc3_encode_stream(struct bt_conn * conn, uint8_t *pcm, struct net_buf *buf)
{
    int16_t *p = (int16_t *)pcm;
    struct conn_state *state = NULL;
    uint8_t * out;
    int lc3_ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if ((conn == server_state[index].conn) || ((NULL == conn) && (NULL != server_state[index].conn)))
        {
            state = &server_state[index];
            break;
        }
    }

    if (NULL == state)
    {
        return -EINVAL;
    }

    if (NULL == state->snk[0].stream)
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(state->snk[0].stream->flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }

    out = net_buf_tail(buf);

    for (size_t index = 0; index < snk_cap_required.frame_blocks_per_sdu; index++)
    {
        p = p + index * (snk_cap_required.frequency * snk_cap_required.duration / 1000000) * snk_cap_required.channel_count;
        out = out + index * snk_cap_required.frame_bytes * snk_cap_required.channel_count;
        for (int j = 0; j < (snk_cap_required.frequency * snk_cap_required.duration / 1000000); j++)
        {
            for(int i = 0; i < snk_cap_required.channel_count; i++)
            {
                state->snk[0].encoder->pcm_buf_in[i][j] = (int32_t)p[j*snk_cap_required.channel_count + i];
            }
        }

        for(int i = 0; i < snk_cap_required.channel_count; i++)
        {
            lc3_ret = LC3_encoder_process(&state->snk[0].encoder->encoder[i]);
            if(lc3_ret != snk_cap_required.frame_bytes)
            {
                PRINTF("Channel %d lc3 encode fail! %d\n", i, lc3_ret);
                return lc3_ret;
            }
            memcpy(&out[i * snk_cap_required.frame_bytes], state->snk[0].encoder->enc_buf_out[i], snk_cap_required.frame_bytes);
        }
    }

    (void)net_buf_add(buf, snk_cap_required.frame_bytes * snk_cap_required.channel_count * snk_cap_required.frame_blocks_per_sdu);

    return snk_cap_required.frame_bytes * snk_cap_required.channel_count * snk_cap_required.frame_blocks_per_sdu;
}

static int stream_send_out(struct bt_conn * conn, struct net_buf *buf)
{
    struct conn_state *state = NULL;
    int ret;

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if ((conn == server_state[index].conn) || ((NULL == conn) && (NULL != server_state[index].conn)))
        {
            state = &server_state[index];
            break;
        }
    }

    if (NULL == state)
    {
        return -EINVAL;
    }

    if (NULL == state->snk[0].stream)
    {
        return -EINVAL;
    }

    if (!atomic_test_bit(state->snk[0].stream->flags, BT_STREAM_STATE_STARTED))
    {
        return -EINVAL;
    }
    ret = bt_bap_stream_send(&state->snk[0].stream->stream, buf, state->snk[0].seq_num++, BT_ISO_TIMESTAMP_NONE);
    if (ret < 0)
    {
        PRINTF("Fail to send stream (error %d)\n", ret);
        return ret;
    }
    return ret;
}

static void get_capability_from_codec(const struct bt_audio_codec_cfg *codec_cfg, struct codec_capability *cap)
{
    int ret;
    enum bt_audio_location location;
    uint32_t tempU32;

    PRINTF("Codec configurations:\n");
    ret = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_config_freq)bt_audio_codec_cfg_get_freq(codec_cfg));
    if (ret >= 0)
    {
        cap->frequency = (uint32_t)ret;
    }
    PRINTF("    Frequency %d\n", cap->frequency);
    ret = bt_audio_codec_cfg_get_frame_duration_us(codec_cfg);
    if (ret >= 0)
    {
        cap->duration = (uint32_t)ret;
    }
    PRINTF("    Duration %d\n", cap->duration);
    ret = bt_audio_codec_cfg_get_octets_per_frame(codec_cfg);
    if (ret >= 0)
    {
        cap->frame_bytes = (uint32_t)ret;
    }
    PRINTF("    Frame bytes %d\n", cap->frame_bytes);
    ret = bt_audio_codec_cfg_get_frame_blocks_per_sdu(codec_cfg, 1);
    if (ret >= 0)
    {
        cap->frame_blocks_per_sdu = (uint32_t)ret;
    }
    PRINTF("    Frame blocks per SDU %d\n", cap->frame_blocks_per_sdu);
    ret = bt_audio_codec_cfg_get_chan_allocation(codec_cfg, &location);

    if (ret >= 0)
    {
        cap->channel_count = 0U;
        tempU32 = (uint32_t)location;
        while (tempU32 > 0U)
        {
            tempU32 = tempU32 & (tempU32 - 1);
            cap->channel_count ++;
        }
        PRINTF("    Location %d, channel count %d.\n", (uint32_t)location, cap->channel_count);
    }
    else
    {
        PRINTF("    Location is invalid\n");
    }
}

const uint32_t frequency_bitmap[] = {8000UL, 11000UL, 16000UL, 22000UL, 24000UL, 32000UL, 44100UL, 48000UL};
const uint32_t duration_bitmap[] = {7500UL, 10000UL};

static void parse_pacs_capability(const struct bt_audio_codec_cap *codec_cap, struct pacs_capability *cap)
{
    struct bt_audio_codec_octets_per_codec_frame codec_frame;

    PRINTF("Codec configurations:\n");
    cap->frequency_bitmap = bt_audio_codec_cap_get_freq(codec_cap);
    PRINTF("    Frequency");
    for (size_t index = 0; index < ARRAY_SIZE(frequency_bitmap); index++)
    {
        if (cap->frequency_bitmap & BIT(index))
        {
            PRINTF(" %d,", frequency_bitmap[index]);
        }
    }
    PRINTF("\n");

    cap->duration_bitmap = bt_audio_codec_cap_get_frame_duration(codec_cap);
    PRINTF("    Duration");
    for (size_t index = 0; index < ARRAY_SIZE(duration_bitmap); index++)
    {
        if (cap->duration_bitmap & BIT(index))
        {
            PRINTF(" %d,", duration_bitmap[index]);
        }
    }
    PRINTF("\n");

    cap->channel_count = bt_audio_codec_cap_get_supported_audio_chan_counts(codec_cap);
    PRINTF("    Channel count %d.\n", cap->channel_count);

    if(0 == bt_audio_codec_cap_get_octets_per_frame(codec_cap, &codec_frame))
    {
        cap->frame_bytes_min = codec_frame.min;
        cap->frame_bytes_max = codec_frame.max;
        PRINTF("    Frame length min %d, max %d\n", cap->frame_bytes_min, cap->frame_bytes_max);
    }

    cap->frame_blocks_per_sdu = bt_audio_codec_cap_get_max_codec_frames_per_sdu(codec_cap);
    PRINTF("    Frame blocks per SDU %d\n", cap->frame_blocks_per_sdu);

    cap->pref_context = bt_audio_codec_cap_meta_get_pref_context(codec_cap);
    PRINTF("    Pref context 0x%x\n", cap->pref_context);
}

static int capability_compare(struct codec_capability *required_cap, struct codec_capability *discovered_cap)
{
    if ((required_cap->channel_count == discovered_cap->channel_count) &&
        (required_cap->duration == discovered_cap->duration) &&
        (required_cap->frame_blocks_per_sdu == discovered_cap->frame_blocks_per_sdu) &&
        (required_cap->frame_bytes == discovered_cap->frame_bytes) &&
        (required_cap->frequency == discovered_cap->frequency)
    )
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

static int pac_capability_compare(struct codec_capability *required_cap, struct pacs_capability *discovered_cap)
{
    size_t index;

    if (!(required_cap->channel_count == discovered_cap->channel_count))
    {
        PRINTF("The discovered channel count %d is mismatched with the required channel count %d\n", discovered_cap->channel_count, required_cap->channel_count);
        return -1;
    }

    if (!(required_cap->frame_blocks_per_sdu == discovered_cap->frame_blocks_per_sdu))
    {
        PRINTF("The discovered frame blocks pre SDU %d is mismatched with the required %d\n", discovered_cap->frame_blocks_per_sdu, required_cap->frame_blocks_per_sdu);
        return -1;
    }

    for (index = 0; index < ARRAY_SIZE(frequency_bitmap); index++)
    {
        if (required_cap->frequency == frequency_bitmap[index])
        {
            break;
        }
    }
    if (index >= ARRAY_SIZE(frequency_bitmap))
    {
        PRINTF("The unicast server unsupports the frequency %d\n", required_cap->frequency);
        return -1;
    }

    for (index = 0; index < ARRAY_SIZE(duration_bitmap); index++)
    {
        if (required_cap->duration == duration_bitmap[index])
        {
            break;
        }
    }
    if (index >= ARRAY_SIZE(duration_bitmap))
    {
        PRINTF("The unicast server unsupports the duration %d\n", required_cap->duration);
        return -1;
    }

    if ((required_cap->frame_bytes > discovered_cap->frame_bytes_max) || (required_cap->frame_bytes < discovered_cap->frame_bytes_min))
    {
        PRINTF("The frame bytes (%d ~ %d) of unicast server is unsupported (required %d)\n", discovered_cap->frame_bytes_min, discovered_cap->frame_bytes_max, required_cap->frame_bytes);
        return -1;
    }

    return 0;
}

static void unicast_client_stream_configured(struct bt_bap_stream *stream,
                  const struct bt_audio_codec_qos_pref *pref)
{
    struct stream_state * stream_base;

    PRINTF("Audio Stream %p configured\n", stream);

    stream_base = (struct stream_state *)stream;

    atomic_set_bit(stream_base->flags, BT_STREAM_STATE_CONFIGURED);
    (void)OSA_SemaphorePost(stream_base->sem);
}

static void unicast_client_stream_qos_set(struct bt_bap_stream *stream)
{
    struct stream_state * stream_base;

    PRINTF("Audio Stream %p QoS set\n", stream);

    stream_base = (struct stream_state *)stream;

    atomic_set_bit(stream_base->flags, BT_STREAM_STATE_QOS);
    (void)OSA_SemaphorePost(stream_base->sem);
}

static void unicast_client_stream_enabled(struct bt_bap_stream *stream)
{
    struct stream_state * stream_base;

    PRINTF("Audio Stream %p enabled\n", stream);

    stream_base = (struct stream_state *)stream;

    for (uint32_t index = 0U; index < ARRAY_SIZE(server_state); index++)
    {
        for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
        {
            if (stream->ep == server_state[index].src[ep_index].ep)
            {
                BOARD_StartCodec(codec_tx_callback, codec_rx_callback, src_cap_required.frequency, BITS_RATES_OF_SAMPLE);
#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
                BOARD_SyncTimer_Init(sync_timer_callback);
#endif /* UNICAST_AUDIO_SYNC_MODE */
                break;
            }
        }
    }

    atomic_set_bit(stream_base->flags, BT_STREAM_STATE_ENABLED);
    (void)OSA_SemaphorePost(stream_base->sem);
}

#if defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0)
static uint32_t get_cig_sync_delay(struct conn_state * connect)
{
	struct bt_iso_info iso_info;

	bt_iso_chan_get_info(&connect->snk[0].ep->iso->chan, &iso_info);

	return iso_info.unicast.cig_sync_delay;
}

static uint32_t get_iso_interval(struct conn_state * connect)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(&connect->snk[0].ep->iso->chan, &iso_info);

	ISO_Interval_us = iso_info.iso_interval * 1250;

	return ISO_Interval_us;
}
#endif /* UNICAST_AUDIO_SYNC_MODE */

static void unicast_client_stream_started(struct bt_bap_stream *stream)
{
    struct stream_state * stream_base;
    struct conn_state *state = NULL;
    struct source_info * src = NULL;
    struct sink_info * snk = NULL;
    int err;

    PRINTF("Audio Stream %p started\n", stream);

    /* Reset sequence number for sinks */
    for (uint32_t index = 0U; index < ARRAY_SIZE(server_state); index++)
    {
        for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
        {
            if (stream->ep == server_state[index].snk[ep_index].ep)
            {
                server_state[index].snk[ep_index].seq_num = 0U;
                state = &server_state[index];
                snk = &server_state[index].snk[ep_index];
                err = unicast_client_lc3_encoder_init(server_state[index].snk[ep_index].encoder);
                if (0 != err)
                {
                    PRINTF("Fail to init encoder (err %d)\n", err);
                }
                break;
            }
            if (stream->ep == server_state[index].src[ep_index].ep)
            {
                state = &server_state[index];
                src = &server_state[index].src[ep_index];
                err = unicast_client_lc3_decoder_init(server_state[index].src[ep_index].decoder);
                if (0 != err)
                {
                    PRINTF("Fail to init decoder (err %d)\n", err);
                }
                break;
            }
        }
    }
    if (NULL == state)
    {
        return;
    }

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started == 0U)
    {
        sync_timer_started = 1U;
        state->info.bits_pre_sample = BITS_RATES_OF_SAMPLE;
        state->info.sample_rate = src_cap_required.frequency;
        state->info.sample_duration_us = 1000000.0 / (float)state->info.sample_rate;
        state->info.cig_sync_delay_us = get_cig_sync_delay(state);
        state->info.iso_interval_us = get_iso_interval(state);

        BORAD_SyncTimer_Start(state->info.sample_rate, state->info.bits_pre_sample * src_cap_required.channel_count);
    }
#else
    BOARD_StartStream();
#endif /* UNICAST_AUDIO_SYNC_MODE */

    stream_base = (struct stream_state *)stream;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (NULL != src)
    {
        for (int i = 0; i < src_cap_required.channel_count; i++)
        {
            src->resampler[i].upSrcCfg.fsIn = state->info.sample_rate;
            src->resampler[i].upSrcCfg.sfOut = state->info.sample_rate;
            src->resampler[i].upSrcCfg.phs = 32;
            src->resampler[i].upSrcCfg.fltTaps = 32;
            src->resampler[i].upSrcCfg.frmSizeIn = state->info.sample_rate / 100;
            src->resampler[i].upSrcCfg.frmSizeOut = state->info.sample_rate / 100;
            initUpCvtFrm(&src->resampler[i].upSrc, &src->resampler[i].upSrcCfg, 0.0);

            src->resampler[i].out_length = 0;
        }
        src->status.cumulative_error = 0.0;
        src->status.factor_proportional = 5;
        src->status.factor_integral = 3;
        src->status.factor_differential = 3;
        src->status.output = 0.0;

        src->status.presentation_delay_us = stream->qos->pd;
        src->status.resampler_added_samples = 0;
        src->status.start_slot = 0;
        src->status.resampler_internal_samples = 0.0;

        src->status.output_length = 0;
        src->status.received_length = 0;
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    if (NULL != snk)
    {
        for (int i = 0; i < snk_cap_required.channel_count; i++)
        {
            snk->resampler[i].upSrcCfg.fsIn = state->info.sample_rate;
            snk->resampler[i].upSrcCfg.sfOut = state->info.sample_rate;
            snk->resampler[i].upSrcCfg.phs = 32;
            snk->resampler[i].upSrcCfg.fltTaps = 32;
            snk->resampler[i].upSrcCfg.frmSizeIn = state->info.sample_rate / 100;
            snk->resampler[i].upSrcCfg.frmSizeOut = state->info.sample_rate / 100;
            initUpCvtFrm(   snk->resampler[i].upSrc,  snk->resampler[i].upSrcCfg, 0.0);

            snk->resampler[i].out_length = 0;
        }
        snk->status.cumulative_error = 0.0;
        snk->status.factor_proportional = 5;
        snk->status.factor_integral = 3;
        snk->status.factor_differential = 3;
        snk->status.output = 0.0;

        snk->status.presentation_delay_us = stream->qos->pd;
        snk->status.resampler_added_samples = 0;
        snk->status.start_slot = 0;
        snk->status.resampler_internal_samples = 0.0;

        snk->status.output_length = 0;
        snk->status.received_length = 0;
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */

    atomic_set_bit(stream_base->flags, BT_STREAM_STATE_STARTED);
    (void)OSA_SemaphorePost(stream_base->sem);

    (void)snk;
}

static void unicast_client_stream_metadata_updated(struct bt_bap_stream *stream)
{
    PRINTF("Audio Stream %p metadata updated\n", stream);
}

static void unicast_client_stream_disabled(struct bt_bap_stream *stream)
{
    struct stream_state * stream_base;
    int err = 0;

    PRINTF("Audio Stream %p disabled\n", stream);

    stream_base = (struct stream_state *)stream;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */
    BOARD_StopCodec();

    err = bt_bap_stream_stop(&stream_base->stream);
    if (0 != err)
    {
        PRINTF("Fail to stop stream (err %d)\n", err);
    }

    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_ENABLED);
    (void)OSA_SemaphorePost(stream_base->sem);
}

static void unicast_client_stream_stopped(struct bt_bap_stream *stream, uint8_t reason)
{
    struct stream_state * stream_base;

    PRINTF("Audio Stream %p stopped with reason 0x%02X\n", stream, reason);

    stream_base = (struct stream_state *)stream;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */
    BOARD_StopCodec();

    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_STARTED);
    (void)OSA_SemaphorePost(stream_base->sem);
}

static void unicast_client_stream_released(struct bt_bap_stream *stream)
{
    struct stream_state * stream_base;

    PRINTF("Audio Stream %p released\n", stream);

    stream_base = (struct stream_state *)stream;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */
    BOARD_StopCodec();

    atomic_set_bit(stream_base->flags, BT_STREAM_STATE_RELEASED);

    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_CONFIGURED);
    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_QOS);
    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_ENABLED);
    atomic_clear_bit(stream_base->flags, BT_STREAM_STATE_STARTED);

    (void)OSA_SemaphorePost(stream_base->sem);

    (void)stream_base;
}

static void codec_rx_callback(uint8_t *rx_buffer)
{
    struct net_buf *buf;
    size_t tx_len = 0;
    osa_status_t ret;
    uint16_t *src = (uint16_t *)rx_buffer;
    uint16_t *dst = NULL;

    if (voice_is_started == 0U)
    {
        return;
    }

    buf = net_buf_alloc(&tx_pool, 0U);
    if (NULL == buf)
    {
        return;
    }

    tx_len = snk_cap_required.frequency * snk_cap_required.duration / 1000000;
    dst = (uint16_t *)buf->data;

    for (size_t j = 0; j < tx_len; j ++)
    {
        for (size_t i = 0; i < snk_cap_required.channel_count; i++)
        {
            dst[j * snk_cap_required.channel_count + i] = src[j];
        }
    }
    buf->len = tx_len * snk_cap_required.channel_count * 2;

    ret = OSA_MsgQPut(tx_stream_queue, &buf);
    if (KOSA_StatusSuccess != ret)
    {
        net_buf_unref(buf);
    }
}

static void codec_tx_callback(void)
{
    if (BT_STREAM_RINGTONE_MODE_LOCAL == ringtone_mode)
    {
        if (NULL != ringtone_work.pcm)
        {
            size_t tx_len = snk_cap_required.channel_count * (snk_cap_required.frequency * snk_cap_required.duration * BITS_RATES_OF_SAMPLE / 8000000) * snk_cap_required.frame_blocks_per_sdu;
            if ((ringtone_work.prime_index + tx_len) > ringtone_work.pcm_length)
            {
                ringtone_work.prime_index = 0U;
            }
            BOARD_PrimeTxWriteBuffer(&ringtone_work.pcm[ringtone_work.prime_index], tx_len);

            ringtone_work.prime_index = ringtone_work.prime_index + tx_len;
        }
        else
        {
            ringtone_mode = BT_STREAM_RINGTONE_MODE_NONE;
        }
    }
}

static void unicast_client_stream_recv(struct bt_bap_stream *stream,
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
    net_buf_add_mem(rx_buf, (void*)&sync_index, sizeof(sync_index));
#endif /* UNICAST_AUDIO_SYNC_MODE */

    net_buf_add_mem(rx_buf, buf->data, buf->len);
    ret = OSA_MsgQPut(rx_stream_queue, &rx_buf);
    if (KOSA_StatusSuccess != ret)
    {
        net_buf_unref(rx_buf);
        PRINTF(" Fail to put the rx buf %p to queue\n", rx_buf);
        return;
    }
}

static void unicast_client_discover_sink_callback(struct bt_conn *conn, int err, enum bt_audio_dir dir)
{
    if (err != 0 && err != BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
        PRINTF("Discovery failed: %d\n", err);
        return;
    }

    if (err == BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
        PRINTF("Discover sinks completed without finding any source ASEs\n");
    } else {
        PRINTF("Discover sinks complete: err %d\n", err);
    }

    /* Discover source pac */
    unicast_client_discover_source(conn);
    /* TODO: */
}

static void unicast_client_discover_source_callback(struct bt_conn *conn, int err, enum bt_audio_dir dir)
{
    if (err != 0 && err != BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
        PRINTF("Discovery failed: %d\n", err);
        return;
    }

    if (err == BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
        PRINTF("Discover sinks completed without finding any source ASEs\n");
    } else {
        PRINTF("Discover sources complete: err %d\n", err);
    }

    /* TODO: */
    if (discover_done != NULL)
    {
        discover_done(conn, err);
    }
}

static int unicast_client_discover_sink(struct bt_conn * conn)
{
    int err;

    unicast_client_callbacks.discover = unicast_client_discover_sink_callback;

    err = bt_bap_unicast_client_discover(conn, BT_AUDIO_DIR_SINK);
    if (err != 0) {
        PRINTF("Failed to discover sinks: %d\n", err);
        return err;
    }
    return err;
}

static int unicast_client_discover_source(struct bt_conn * conn)
{
    int err;

    unicast_client_callbacks.discover = unicast_client_discover_source_callback;

    err = bt_bap_unicast_client_discover(conn, BT_AUDIO_DIR_SOURCE);
    if (err != 0) {
        PRINTF("Failed to discover source: %d\n", err);
        return err;
    }
    return err;
}

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (0 == err)
    {
        for (uint8_t i = 0; i < ARRAY_SIZE(server_state); i++)
        {
            if (NULL == server_state[i].conn)
            {
                server_state[i].conn = conn;
                unicast_client_discover_sink(conn);
                break;
            }
        }
    }
    /* TODO: Connected event */
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
    struct stream_state * stream_base;
    struct lc3_decoder * decoder;
    struct lc3_encoder * encoder;

#if (defined(UNICAST_AUDIO_SYNC_MODE) && (UNICAST_AUDIO_SYNC_MODE > 0))
    if (sync_timer_started > 0)
    {
        sync_timer_started = 0;
        BORAD_SyncTimer_Stop();
    }
#endif /* UNICAST_AUDIO_SYNC_MODE */

    unicast_client_disable_streams();
    unicast_client_release_streams();
    unicast_client_delete_group();

    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (conn == server_state[index].conn)
        {
            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
            {
                if (server_state[index].src[ep_index].stream != NULL)
                {
                    stream_base = (struct stream_state *)server_state[index].src[ep_index].stream;
                    if (NULL != stream_base->sem)
                    {
                        (void)OSA_SemaphoreDestroy(stream_base->sem);
                        stream_base->sem = NULL;
                    }
                    bt_fifo_put(&free_streams, stream_base);
                    server_state[index].src[ep_index].stream = NULL;
                }

                if (server_state[index].src[ep_index].decoder != NULL)
                {
                    decoder = server_state[index].src[ep_index].decoder;
                    bt_fifo_put(&free_decoders, decoder);
                    server_state[index].src[ep_index].decoder = NULL;
                }
            }

            for (uint32_t ep_index = 0U; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
            {
                if (server_state[index].snk[ep_index].stream != NULL)
                {
                    stream_base = (struct stream_state *)server_state[index].snk[ep_index].stream;
                    if (NULL != stream_base->sem)
                    {
                        (void)OSA_SemaphoreDestroy(stream_base->sem);
                        stream_base->sem = NULL;
                    }
                    bt_fifo_put(&free_streams, stream_base);
                    server_state[index].snk[ep_index].stream = NULL;
                }

                if (server_state[index].snk[ep_index].encoder != NULL)
                {
                    encoder = server_state[index].snk[ep_index].encoder;
                    bt_fifo_put(&free_encoders, encoder);
                    server_state[index].snk[ep_index].encoder = NULL;
                }
            }
            server_state[index].conn = NULL;
            memset(&server_state[index], 0, sizeof(server_state[index]));
            break;
        }
    }
    /* TODO: Disconnected event */
}

static void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
    PRINTF("MTU exchanged: %u/%u\n", tx, rx);
    /* TODO: ATT MTU Updated event */
}

#if defined(CONFIG_BT_VCP_VOL_CTLR) && (CONFIG_BT_VCP_VOL_CTLR > 0)
static void vcs_client_discover(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t vocs_count, uint8_t aics_count)
{
    if (err) {
        PRINTF("\nVCS discover finished callback error: %d\n", err);
    } else {
        PRINTF("\nVCS discover finished\n");
    }
}

static void vcs_client_state(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t volume, uint8_t mute)
{
    if (err)
    {
        PRINTF("\nVCS state callback error: %d\n", err);
        return;
    }

    /* Two cases here
     * Case 1: The volume state callback triggered due to function call bt_vcp_vol_ctlr_set_vol, bt_vcp_vol_ctlr_mute, or bt_vcp_vol_ctlr_unmute.
     *         In this case, we just update the local flag (just treat it as an ACK of function call).
     * Case 2: The volume state callback triggered due to peer volume updated. In this case, we need to update all peers by calling function
     *         bt_vcp_vol_ctlr_set_vol, bt_vcp_vol_ctlr_mute, or bt_vcp_vol_ctlr_unmute.
     */
    /* TODO: Implement volume update here*/
}

static void vcs_client_flags(struct bt_vcp_vol_ctlr *vol_ctlr, int err, uint8_t flags)
{
    if (err) {
        PRINTF("\nVCS flag callback error: %d\n", err);
    }
}
#endif /* CONFIG_BT_VCP_VOL_CTLR */

#if defined(CONFIG_BT_MCS) && (CONFIG_BT_MCS > 0)
static void mcs_local_player_instance(struct media_player *player, int err)
{
    if(err)
    {
        PRINTF("\nMCS instance is registered, err %d\n", err);
        return;
    }

    mcs_player = player;

    (void)mcs_player;
}

static void mcs_media_state_recv(struct media_player *player, int err, uint8_t state)
{
    if(err)
    {
        PRINTF("\nMCS state recv, player %p, err %d, state %d\n", player, err, state);
        return;
    }
}

static void mcs_command_recv(struct media_player *player, int err, const struct mpl_cmd_ntf *result)
{
    if(err)
    {
        PRINTF("\nMCS cmd recv, player %p, err %d, req_op %d, res %d\n", player, err, (int)result->requested_opcode, (int)result->result_code);
    }

    if(result->requested_opcode == BT_MCS_OPC_PLAY)
    {
        /* TODO: Media play */
    }
    else if(result->requested_opcode == BT_MCS_OPC_PAUSE)
    {
        /* TODO: Media pause */
    }
    else
    {
        PRINTF("\nMCS opcode %d not support!\n");
    }
}
#endif /* CONFIG_BT_MCS */

#if (defined(CONFIG_BT_BAP_UNICAST_CLIENT) && (CONFIG_BT_BAP_UNICAST_CLIENT > 0))
static void unicast_client_location(struct bt_conn *conn, enum bt_audio_dir dir, enum bt_audio_location loc)
{
    PRINTF("conn %p dir %u loc %X\n", conn, dir, loc);
}

static void unicast_client_available_contexts(struct bt_conn *conn, enum bt_audio_context snk_ctx, enum bt_audio_context src_ctx)
{
    struct conn_state * state = NULL;

    PRINTF("conn %p snk ctx %u src ctx %u\n", conn, snk_ctx, src_ctx);

    for (uint32_t index = 0U; index < ARRAY_SIZE(server_state); index++)
    {
        if (server_state[index].conn == conn)
        {
            state = &server_state[index];
            break;
        }
    }
    if (NULL == state)
    {
        PRINTF("The connection is known!\n");
        return;
    }

    state->snk_pac.context = snk_ctx;
    state->src_pac.context = src_ctx;
}

static void unicast_client_pac_record(struct bt_conn *conn, enum bt_audio_dir dir, const struct bt_audio_codec_cap *codec_cap)
{
    struct conn_state * state = NULL;

    PRINTF("codec capabilities on conn %p dir %u codec %p. ", conn, dir, codec_cap);
    for (uint32_t index = 0U; index < ARRAY_SIZE(server_state); index++)
    {
        if (server_state[index].conn == conn)
        {
            state = &server_state[index];
            break;
        }
    }
    if (NULL == state)
    {
        PRINTF("The connection is known!\n");
        return;
    }

    if (BT_AUDIO_DIR_SINK == dir)
    {
        parse_pacs_capability(codec_cap, &state->snk_pac);
        if (pac_capability_compare(&snk_cap_required, &state->snk_pac) >= 0)
        {
            state->snk_cap_support = 1U;
        }
    }
    else
    {
        parse_pacs_capability(codec_cap, &state->src_pac);
        if (pac_capability_compare(&src_cap_required, &state->src_pac) >= 0)
        {
            state->src_cap_support = 1U;
        }
    }
}

static void unicast_client_endpoint(struct bt_conn *conn, enum bt_audio_dir dir, struct bt_bap_ep *ep)
{
    struct stream_state * stream = NULL;
    struct lc3_encoder *encoder = NULL;
    struct lc3_decoder *decoder = NULL;
    osa_status_t osa_ret;
    uint8_t failed = 0U;

    PRINTF("conn %p dir %u ep %p\n", conn, dir, ep);

    stream = bt_fifo_get(&free_streams, K_NO_WAIT);
    if (NULL == stream)
    {
        assert(NULL == stream);
        failed = 1U;
    }

    if (failed == 0U)
    {
        if (NULL == stream->sem)
        {
            osa_ret = OSA_SemaphoreCreate(stream->sem_handle, 0);
            if (KOSA_StatusSuccess != osa_ret)
            {
                assert(KOSA_StatusSuccess != osa_ret);
                failed = 1U;
            }
            else
            {
                stream->sem = stream->sem_handle;
            }
        }
    }

    if (failed == 0U)
    {
        if (BT_AUDIO_DIR_SOURCE == dir)
        {
            decoder = bt_fifo_get(&free_decoders, K_NO_WAIT);
            if (NULL == decoder)
            {
                assert(NULL == decoder);
                failed = 1U;
            }
        }
        else
        {
            encoder = bt_fifo_get(&free_encoders, K_NO_WAIT);
            if (NULL == encoder)
            {
                assert(NULL == encoder);
                failed = 1U;
            }
        }
    }

    if (failed == 0U)
    {
        atomic_clear_bit(stream->flags, BT_STREAM_STATE_CONFIGURED);
        atomic_clear_bit(stream->flags, BT_STREAM_STATE_QOS);
        atomic_clear_bit(stream->flags, BT_STREAM_STATE_ENABLED);
        atomic_clear_bit(stream->flags, BT_STREAM_STATE_STARTED);
        atomic_clear_bit(stream->flags, BT_STREAM_STATE_RELEASED);

        for (uint32_t index = 0U; index < ARRAY_SIZE(server_state); index++)
        {
            if (server_state[index].conn == conn)
            {
                if (BT_AUDIO_DIR_SOURCE == dir)
                {
                    uint32_t ep_index = 0U;
                    for (; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT; ep_index++)
                    {
                        if (server_state[index].src[ep_index].ep == NULL)
                        {
                            server_state[index].src[ep_index].ep = ep;
                            server_state[index].src[ep_index].stream = stream;
                            server_state[index].src[ep_index].decoder = decoder;
                            break;
                        }
                    }
                    if (ep_index >= CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT)
                    {
                        assert(false);
                        failed = 1;
                        break;
                    }
                }
                else
                {
                    uint32_t ep_index = 0U;
                    for (; ep_index < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; ep_index++)
                    {
                        if (server_state[index].snk[ep_index].ep == NULL)
                        {
                            server_state[index].snk[ep_index].ep = ep;
                            server_state[index].snk[ep_index].stream = stream;
                            server_state[index].snk[ep_index].encoder = encoder;
                            break;
                        }
                    }
                    if (ep_index >= CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT)
                    {
                        assert(false);
                        failed = 1;
                        break;
                    }
                }
                break;
            }
        }
    }

    if (failed > 0U)
    {
        if (NULL != stream)
        {
            if (NULL != stream->sem)
            {
                (void)OSA_SemaphoreDestroy(stream->sem);
                stream->sem = NULL;
            }
            bt_fifo_put(&free_streams, stream);
        }

        if (NULL != decoder)
        {
            bt_fifo_put(&free_decoders, decoder);
        }

        if (NULL != encoder)
        {
            bt_fifo_put(&free_encoders, encoder);
        }
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

    struct conn_state *state = NULL;

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

    state = NULL;
    for (uint32_t index = 0; index < ARRAY_SIZE(server_state); index++)
    {
        if (NULL != server_state[index].conn)
        {
            state = &server_state[index];
            break;
        }
    }
    if (NULL == state)
    {
        return;
    }

    state->src[0].status.current_slot = sync_index;
#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
    state->snk[0].status.current_slot = sync_index;
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    sync_index_cycle ++;
    sync_index_cycle = sync_index_cycle % 1000;
    if ((sync_timer_started > 0) && (0 != state->src[0].status.start_slot))
    {
        if (sync_index == state->src[0].status.start_slot)
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
        else if (sync_index > state->src[0].status.start_slot)
        {
            actual_samples = (double)bclk_count / (BITS_RATES_OF_SAMPLE);
            actual_samples -= state->src[0].status.mute_frame_samples * (double)src_cap_required.channel_count;

            ideal_samples = (sync_index - state->src[0].status.start_slot) * (double)state->info.iso_interval_us - (double)state->src[0].status.mute_frame_duration_us;
            ideal_samples = ideal_samples / state->info.sample_duration_us;
            ideal_samples = ideal_samples * (double)src_cap_required.channel_count;

            ideal_samples = ideal_samples + state->src[0].status.resampler_added_samples + state->src[0].status.resampler_internal_samples;

            delta = ideal_samples - actual_samples;

            actual_samples = (double)(bclk_count- bclk_count_last) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = (sync_index - sync_index_last) * (double)state->info.iso_interval_us;
            ideal_samples = ideal_samples / state->info.sample_duration_us;
            ideal_samples = ideal_samples * (double)src_cap_required.channel_count;

            ideal_samples = ideal_samples + state->src[0].status.resampler_added_samples - resampler_added_samples_last  + state->src[0].status.resampler_internal_samples;

            delta_current = ideal_samples - actual_samples;

            sync_index_last = sync_index;
            bclk_count_last = bclk_count;
            resampler_added_samples_last = state->src[0].status.resampler_added_samples;

            state->src[0].status.cumulative_error = delta;
            state->src[0].status.output = state->src[0].status.factor_proportional * (delta_current) + state->src[0].status.factor_integral * state->src[0].status.cumulative_error + state->src[0].status.factor_differential * (delta_current - delta_last);

            delta_last = delta_current;

            output_max = 25.0 / ((double)state->info.sample_duration_us * 4.0);

            if (state->src[0].status.output > output_max)
            {
                state->src[0].status.output = output_max;
            }
            else if (state->src[0].status.output < -output_max)
            {
                state->src[0].status.output = -output_max;
            }
            else
            {
            }

#if 0
            PRINTF("%d i:%f a:%f o:%f\n", sync_index, ideal_samples, actual_samples, state->src[0].status.output * state->info.sample_duration_us);
#endif
        }
        else
        {

        }

#if (defined(UNICAST_AUDIO_SYNC_MODE_TX) && (UNICAST_AUDIO_SYNC_MODE_TX > 0))
        if (sync_index >= state->snk[0].status.start_slot)
        {
            if (0 == src_sync_index_start)
            {
                src_bclk_count_start = bclk_count;
                src_sync_index_start = sync_index;
                src_bclk_count_last = bclk_count;
                src_sync_index_last = sync_index;
            }
            actual_samples = (double)(bclk_count - src_bclk_count_start) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = state->snk[0].status.output_length;

            actual_samples = actual_samples + state->snk[0].status.resampler_added_samples + state->snk[0].status.resampler_internal_samples;

            delta = ideal_samples - actual_samples;

            actual_samples = (double)(bclk_count- src_bclk_count_last) / (BITS_RATES_OF_SAMPLE);

            ideal_samples = (sync_index - src_sync_index_last) * (double)state->info.iso_interval_us;
            ideal_samples = ideal_samples / state->info.sample_duration_us;
            ideal_samples = ideal_samples * (double)src_cap_required.channel_count;

            actual_samples = actual_samples + state->snk[0].status.resampler_added_samples - src_resampler_added_samples_last  + state->snk[0].status.resampler_internal_samples;

            delta_current = ideal_samples - actual_samples;

            src_sync_index_last = sync_index;
            src_bclk_count_last = bclk_count;
            src_resampler_added_samples_last = state->snk[0].status.resampler_added_samples;

            state->snk[0].status.cumulative_error = delta;
            state->snk[0].status.output = state->snk[0].status.factor_proportional * (delta_current) + state->snk[0].status.factor_integral * state->snk[0].status.cumulative_error + state->snk[0].status.factor_differential * (delta_current - src_delta_last);

            src_delta_last = delta_current;

            output_max = 25.0 / ((double)state->info.sample_duration_us * 4.0);

            if (state->snk[0].status.output > output_max)
            {
                state->snk[0].status.output = output_max;
            }
            else if (state->snk[0].status.output < -output_max)
            {
                state->snk[0].status.output = -output_max;
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
            PRINTF("%d i:%f a:%f o:%f\n", sync_index, ideal_samples, actual_samples, state->snk[0].status.output * state->info.sample_duration_us);
#endif
        }
#endif /* UNICAST_AUDIO_SYNC_MODE_TX */

    }

}
#endif /* UNICAST_AUDIO_SYNC_MODE */

#endif /* CONFIG_BT_BAP_UNICAST_CLIENT */
