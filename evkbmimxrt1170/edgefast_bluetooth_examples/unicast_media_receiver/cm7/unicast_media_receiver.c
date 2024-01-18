/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/types.h>
#include <stdio.h>
#include <stddef.h>
#include <errno/errno.h>
#include <toolchain.h>
#include <porting.h>
#include "fsl_debug_console.h"

#include <bluetooth/bluetooth.h>
#include <bluetooth/byteorder.h>
#include <bluetooth/conn.h>
#include <bluetooth/audio/audio.h>
#include <bluetooth/audio/bap.h>
#include <bluetooth/audio/pacs.h>
#include <sys/byteorder.h>

#include <bluetooth/audio/bap_lc3_preset.h>

#include "le_audio_common.h"
#include "le_audio_service.h"
#include "le_audio_shell.h"
#include "unicast_media_receiver.h"

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#include "audio/bap_iso.h"
#endif

#ifndef PRINTF
#define PRINTF PRINTF
#endif

#define AVAILABLE_SINK_CONTEXT  (BT_AUDIO_CONTEXT_TYPE_UNSPECIFIED | \
				 BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL | \
				 BT_AUDIO_CONTEXT_TYPE_MEDIA | \
				 BT_AUDIO_CONTEXT_TYPE_GAME | \
				 BT_AUDIO_CONTEXT_TYPE_INSTRUCTIONAL)

#define AVAILABLE_SOURCE_CONTEXT (BT_AUDIO_CONTEXT_TYPE_PROHIBITED)



static const struct bt_audio_codec_cap lc3_codec_cap = BT_AUDIO_CODEC_CAP_LC3(
	BT_AUDIO_CODEC_LC3_FREQ_ANY, BT_AUDIO_CODEC_LC3_DURATION_10,
	BT_AUDIO_CODEC_LC3_CHAN_COUNT_SUPPORT(1), 40u, 120u, 1u,
	(BT_AUDIO_CONTEXT_TYPE_CONVERSATIONAL | BT_AUDIO_CONTEXT_TYPE_MEDIA));

static struct bt_conn *default_conn;
static struct bt_bap_stream sink_streams[CONFIG_BT_ASCS_ASE_SNK_COUNT];

static struct bt_bap_lc3_preset lc3_preset;

static const struct bt_audio_codec_qos_pref qos_pref =
	BT_AUDIO_CODEC_QOS_PREF(true, BT_GAP_LE_PHY_2M, 0x02, 10, 40000, 40000, 40000, 40000);


static uint8_t unicast_server_addata[] = {
	BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL), /* ASCS UUID */
	BT_AUDIO_UNICAST_ANNOUNCEMENT_TARGETED, /* Target Announcement */
	BT_BYTES_LIST_LE16(AVAILABLE_SINK_CONTEXT),
	BT_BYTES_LIST_LE16(AVAILABLE_SOURCE_CONTEXT),
	0x00, /* Metadata length */
};

/* TODO: Expand with BAP data */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID16_ALL, BT_UUID_16_ENCODE(BT_UUID_ASCS_VAL)),
	BT_DATA(BT_DATA_SVC_DATA16, unicast_server_addata, ARRAY_SIZE(unicast_server_addata)),
};

/* Audio Sink parameters. */
#define MAX_AUDIO_SAMPLE_RATE		48000
#define MAX_AUDIO_CHANNEL_COUNT		2
#define MAX_AUDIO_BYTES_PER_SAMPLE 	2

/* Codec related variables and functions. */
#include "hw_codec.h"
#include "audio_i2s.h"

#define PCM_BUFF_COUNT 			10
#define PCM_AUDIO_BUFF_SIZE		(MAX_AUDIO_SAMPLE_RATE / 100 * MAX_AUDIO_CHANNEL_COUNT * MAX_AUDIO_BYTES_PER_SAMPLE)

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#else
static uint8_t audio_i2s_buff[PCM_AUDIO_BUFF_SIZE];
#endif
static bool audio_codec_initialized = false;

/* LC3 decoder variables. */
#include "lc3_codec.h"
lc3_decoder_t decoder;

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#else
static uint8_t audio_buff[MAX_AUDIO_CHANNEL_COUNT][PCM_AUDIO_BUFF_SIZE];
#endif
static OSA_MSGQ_HANDLE_DEFINE(sdu_fifo, PCM_BUFF_COUNT, sizeof(void *));
NET_BUF_POOL_FIXED_DEFINE(sdu_pool,
			  PCM_BUFF_COUNT,
			  sizeof(sdu_packet_t),
			  NULL);

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#include "le_audio_sync.h"
static frame_packet_t frame;
#endif

static lc3_codec_info_t lc3_codec_info;

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_disconnected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_enabled);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_security_changed);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_mcs_server_discovered);

static bool stream_stop = false;

static int get_channel_count_from_allocation(uint32_t allocation)
{
	int count = 0;
	for (int i = 0; i < 32; i++)
	{
		if(allocation & (1U<<i))
		{
			count++;
			allocation &= ~(1U<<i);
		}
		if(!allocation)
		{
			break;
		}
	}
	return count;
}

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
static uint32_t get_cig_sync_delay(void)
{
	struct bt_iso_info iso_info;

	bt_iso_chan_get_info(&sink_streams[0].ep->iso->chan, &iso_info);

	return iso_info.unicast.cig_sync_delay;
}

static uint32_t get_iso_interval(void)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(&sink_streams[0].ep->iso->chan, &iso_info);

	ISO_Interval_us = iso_info.iso_interval * 1250;

	return ISO_Interval_us;
}
#endif

static int audio_stream_decode(void)
{
	struct net_buf *sdu_buf;
	sdu_packet_t *sdu;
	int frame_flags = LC3_FRAME_FLAG_BAD;
	uint8_t *temp_audio_buff;

	osa_status_t status = OSA_MsgQGet(sdu_fifo, &sdu_buf, osaWaitForever_c);
	if(KOSA_StatusSuccess != status)
	{
		return -1;
	}

	sdu = (sdu_packet_t *)sdu_buf->data;

	if(sdu->info.flags & BT_ISO_FLAGS_VALID)
	{
		frame_flags = LC3_FRAME_FLAG_GOOD;
	}

#if 0 /* used for packet lost sdu debug. */
	if((sdu->info.flags & BT_ISO_FLAGS_VALID) == 0)
	{
		PRINTF("seq: %d, t: %d, flag: 0x%02x, len: %d\n", sdu->info.seq_num, sdu->info.ts, sdu->info.flags, sdu->len);
	}

	if(sdu->info.flags & BT_ISO_FLAGS_ERROR)
	{
		PRINTF("seq: %d, t: %d, flag: 0x%02x, len: %d, BT_ISO_FLAGS_ERROR!\n", sdu->info.seq_num, sdu->info.ts, sdu->info.flags, sdu->len);
	}

	if(sdu->info.flags & BT_ISO_FLAGS_LOST)
	{
		PRINTF("seq: %d, t: %d, flag: 0x%02x, len: %d, BT_ISO_FLAGS_LOST!\n", sdu->info.seq_num, sdu->info.ts, sdu->info.flags, sdu->len);
	}
#endif

	/* LC3 decode. */
#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	temp_audio_buff = (uint8_t *)frame.buff;
#else
	temp_audio_buff = audio_buff[0];
#endif
	int lc3_res = lc3_decoder(&decoder, sdu->buff, frame_flags, temp_audio_buff);
	if(lc3_res)
	{
		PRINTF("\nlc3_decoder fail!\n");
	}

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	memcpy(&frame.info, &sdu->info, sizeof(struct bt_iso_recv_info));
	frame.len = lc3_codec_info.samples_per_frame * 2; /* here we assume it is 10ms 16bits frame. */
	frame.flags = BT_ISO_FLAGS_VALID;
	/* handle the invalid frame. */
	if(lc3_res)
	{
		frame.flags = BT_ISO_FLAGS_ERROR;
	}

	le_audio_sync_process(&frame);
#else
	/* fill pcm buff when it have empty buff */
	if(lc3_codec_info.channels == 1)
	{
		(void)audio_data_make_stereo(lc3_codec_info.samples_per_frame, 16, temp_audio_buff, temp_audio_buff, audio_i2s_buff);
	}

	int res;
	do
	{
		res = audio_i2s_write(audio_i2s_buff, lc3_codec_info.samples_per_frame * 4);
		if(res)
		{
			PRINTF("\naudio_i2s_write err %d\n", res);
			OSA_TimeDelay(2);
		}
	} while(res != 0);

	if(!audio_i2s_is_working())
	{
		audio_i2s_start();
	}
#endif

	net_buf_unref(sdu_buf);

	return 0;
}

void print_hex(const uint8_t *ptr, size_t len)
{
	while (len-- != 0) {
		PRINTF("%02x", *ptr++);
	}
}

static bool print_cb(struct bt_data *data, void *user_data)
{
	const char *str = (const char *)user_data;

	PRINTF("%s: type 0x%02x value_len %u\n", str, data->type, data->data_len);
	print_hex(data->data, data->data_len);
	PRINTF("\n");

	return true;
}

static void print_codec_cfg(const struct bt_audio_codec_cfg *codec_cfg)
{
	PRINTF("codec_cfg 0x%02x cid 0x%04x vid 0x%04x count %u\n", codec_cfg->id, codec_cfg->cid,
	       codec_cfg->vid, codec_cfg->data_len);

	if (codec_cfg->id == BT_HCI_CODING_FORMAT_LC3) {
		enum bt_audio_location chan_allocation;
		int ret;

		/* LC3 uses the generic LTV format - other codecs might do as well */

		bt_audio_data_parse(codec_cfg->data, codec_cfg->data_len, print_cb, "data");

		ret = bt_audio_codec_cfg_get_freq(codec_cfg);
		if (ret > 0) {
			PRINTF("  Frequency: %d Hz\n", bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_config_freq)ret));
		}

		PRINTF("  Frame Duration: %d us\n",
		       bt_audio_codec_cfg_get_frame_duration_us(codec_cfg));
		if (bt_audio_codec_cfg_get_chan_allocation(codec_cfg, &chan_allocation) == 0) {
			PRINTF("  Channel allocation: 0x%x\n", chan_allocation);
		}

		PRINTF("  Octets per frame: %d (negative means value not pressent)\n",
		       bt_audio_codec_cfg_get_octets_per_frame(codec_cfg));
		PRINTF("  Frames per SDU: %d\n",
		       bt_audio_codec_cfg_get_frame_blocks_per_sdu(codec_cfg, true));
	} else {
		print_hex(codec_cfg->data, codec_cfg->data_len);
	}

	bt_audio_data_parse(codec_cfg->meta, codec_cfg->meta_len, print_cb, "meta");
}

static void print_qos(const struct bt_audio_codec_qos *qos)
{
	PRINTF("QoS: interval %u framing 0x%02x phy 0x%02x sdu %u "
	       "rtn %u latency %u pd %u\n",
	       qos->interval, qos->framing, qos->phy, qos->sdu,
	       qos->rtn, qos->latency, qos->pd);
}

static enum bt_audio_dir stream_dir(const struct bt_bap_stream *stream)
{
	for (size_t i = 0U; i < ARRAY_SIZE(sink_streams); i++) {
		if (stream == &sink_streams[i]) {
			return BT_AUDIO_DIR_SINK;
		}
	}

	__ASSERT(false, "Invalid stream %p", stream);
	return (enum bt_audio_dir)0;
}

static struct bt_bap_stream *stream_alloc(void)
{
	for (size_t i = 0; i < ARRAY_SIZE(sink_streams); i++) {
		struct bt_bap_stream *stream = &sink_streams[i];

		if (!stream->conn) {
			return stream;
		}
	}

	return NULL;
}

static int lc3_config(struct bt_conn *conn, const struct bt_bap_ep *ep, enum bt_audio_dir dir,
		      const struct bt_audio_codec_cfg *codec_cfg, struct bt_bap_stream **stream,
		      struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("ASE Codec Config: conn %p ep %p dir %u\n", conn, ep, dir);

	print_codec_cfg(codec_cfg);

	*stream = stream_alloc();
	if (*stream == NULL) {
		PRINTF("No streams available\n");
		*rsp = BT_BAP_ASCS_RSP(BT_BAP_ASCS_RSP_CODE_NO_MEM, BT_BAP_ASCS_REASON_NONE);

		return -ENOMEM;
	}

	PRINTF("ASE Codec Config stream %p\n", *stream);

	*pref = qos_pref;

	return 0;
}

static int lc3_reconfig(struct bt_bap_stream *stream, enum bt_audio_dir dir,
			const struct bt_audio_codec_cfg *codec_cfg,
			struct bt_audio_codec_qos_pref *const pref, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("ASE Codec Reconfig: stream %p\n", stream);

	print_codec_cfg(codec_cfg);


	*rsp = BT_BAP_ASCS_RSP(BT_BAP_ASCS_RSP_CODE_CONF_UNSUPPORTED, BT_BAP_ASCS_REASON_NONE);

	/* We only support one QoS at the moment, reject changes */
	return -ENOEXEC;
}

static int lc3_qos(struct bt_bap_stream *stream, const struct bt_audio_codec_qos *qos,
		   struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("QoS: stream %p qos %p\n", stream, qos);

	print_qos(qos);

	return 0;
}

static int lc3_enable(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
		      struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Enable: stream %p meta_len %zu\n", stream, meta_len);

	/* Get codec info. */
	lc3_codec_info.sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_config_freq)bt_audio_codec_cfg_get_freq(stream->codec_cfg));
	lc3_codec_info.frame_duration_us = bt_audio_codec_cfg_get_frame_duration_us(stream->codec_cfg);
	lc3_codec_info.samples_per_frame = lc3_codec_info.sample_rate * (lc3_codec_info.frame_duration_us / 100) / 10000;
	lc3_codec_info.octets_per_frame = bt_audio_codec_cfg_get_octets_per_frame(stream->codec_cfg);
	lc3_codec_info.blocks_per_sdu = bt_audio_codec_cfg_get_frame_blocks_per_sdu(stream->codec_cfg, true);
	bt_audio_codec_cfg_get_chan_allocation(stream->codec_cfg, (enum bt_audio_location *)&lc3_codec_info.chan_allocation);
	lc3_codec_info.channels = get_channel_count_from_allocation(lc3_codec_info.chan_allocation);
	PRINTF("\tCodec: freq %d, channel count %d, duration %d, channel alloc 0x%08x, frame len %d, frame blocks per sdu %d\n",
		lc3_codec_info.sample_rate, lc3_codec_info.channels, lc3_codec_info.frame_duration_us, lc3_codec_info.chan_allocation, lc3_codec_info.octets_per_frame, lc3_codec_info.blocks_per_sdu);

	/* Limit channels to MAX_AUDIO_CHANNEL_COUNT */
	lc3_codec_info.channels = (lc3_codec_info.channels <= MAX_AUDIO_CHANNEL_COUNT) ? lc3_codec_info.channels : MAX_AUDIO_CHANNEL_COUNT;
	
	if(lc3_codec_info.channels != 1)
	{
		PRINTF("There should be only one channel, rather than %d channels.\n", lc3_codec_info.channels);
		while(1);
	}

	lc3_codec_info.bytes_per_channel_frame = lc3_codec_info.samples_per_frame * MAX_AUDIO_BYTES_PER_SAMPLE;

	/* Get mete info */
		/* Todo. */
	/* Config Audio Codec */
	if(audio_codec_initialized)
	{
		audio_codec_initialized = true;
#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#else
		hw_codec_mute();
		(void)audio_i2s_deinit();
#endif
		hw_codec_deinit();
	}

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
#else
	hw_codec_mute();
	(void)audio_i2s_init(lc3_codec_info.sample_rate, 2, 16, AUDIO_I2S_MODE_TX);
	hw_codec_unmute();
	hw_codec_vol_set(hw_codec_vol_get());
#endif

	/* Init HW codec. */
	if(hw_codec_init(lc3_codec_info.sample_rate, 2, 16))
	{
		PRINTF("\nHW Codec init fail!\n");
	}

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	/* Audio sync init */
	le_audio_sync_init();

#if defined(LE_AUDIO_SYNC_TEST) && (LE_AUDIO_SYNC_TEST > 0)
	le_audio_sync_test_init(lc3_codec_info.sample_rate);
#endif
#endif

	/* Config LC3 decoder */
	int lc3_res = lc3_decoder_init(&decoder, lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us, lc3_codec_info.octets_per_frame, 16);
	if(lc3_res)
	{
		PRINTF("\nlc3_decoder_init fail!\n");
	}

	OSA_SemaphorePost(sem_stream_enabled);

	return 0;
}

static int lc3_start(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Start: stream %p\n", stream);

	return 0;
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

static int lc3_metadata(struct bt_bap_stream *stream, const uint8_t meta[], size_t meta_len,
			struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Metadata: stream %p meta_len %zu\n", stream, meta_len);

	return bt_audio_data_parse(meta, meta_len, data_func_cb, rsp);
}

static int lc3_disable(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Disable: stream %p\n", stream);

	return 0;
}

static int lc3_stop(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Stop: stream %p\n", stream);

	return 0;
}

static int lc3_release(struct bt_bap_stream *stream, struct bt_bap_ascs_rsp *rsp)
{
	PRINTF("Release: stream %p\n", stream);
	stream_stop = true;
	return 0;
}

static const struct bt_bap_unicast_server_cb unicast_server_cb = {
	.config = lc3_config,
	.reconfig = lc3_reconfig,
	.qos = lc3_qos,
	.enable = lc3_enable,
	.start = lc3_start,
	.metadata = lc3_metadata,
	.disable = lc3_disable,
	.stop = lc3_stop,
	.release = lc3_release,
};

static void stream_recv_lc3_codec(struct bt_bap_stream *stream,
				  const struct bt_iso_recv_info *info,
				  struct net_buf *buf)
{
	struct net_buf *sdu_buf;
	sdu_packet_t *sdu;

	/* alloc sdu buf from sdu pool */
	sdu_buf = net_buf_alloc(&sdu_pool, osaWaitForever_c);
	if(!sdu_buf)
	{
		PRINTF("sdu buf alloc failed!\n");
		return;
	}

	/* copy sdu to buff. */
	sdu = net_buf_add(sdu_buf, sizeof(sdu_packet_t) - sizeof(sdu->buff) + buf->len);
	memcpy(&sdu->info, info, sizeof(struct bt_iso_recv_info));
	memcpy(sdu->buff, buf->data, buf->len);
	sdu->len = buf->len;

	/* put sdu buf to sdu fifo */
	osa_status_t status = OSA_MsgQPut(sdu_fifo, &sdu_buf);
	if(status != KOSA_StatusSuccess)
	{
		net_buf_unref(sdu_buf);
		PRINTF("Put sdu to sdu_fifo failed!\n");
	}
}

static void stream_started(struct bt_bap_stream *stream)
{
	PRINTF("Stream %p started\n", stream);

	memcpy(&lc3_preset.qos, stream->qos, sizeof(struct bt_audio_codec_qos));

	/* Start Sync. */
#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	hw_codec_mute();
	(void)audio_i2s_init(lc3_codec_info.sample_rate, 2, 16, AUDIO_I2S_MODE_TX);
	hw_codec_unmute();
	hw_codec_vol_set(hw_codec_vol_get());

	if(AUDIO_SINK_ROLE_LEFT == le_audio_sink_role_get())
	{
		/* left stream start event is before first sync signal, so we set sync_index_init to 0. */
		le_audio_sync_start(get_iso_interval(), get_cig_sync_delay(), lc3_codec_info.sample_rate, lc3_codec_info.samples_per_frame, lc3_preset.qos.pd, 0);
	}
	if(AUDIO_SINK_ROLE_RIGHT == le_audio_sink_role_get())
	{
		/* right stream start event is after first sync signal, so we set sync_index_init to 1. */
		le_audio_sync_start(get_iso_interval(), get_cig_sync_delay(), lc3_codec_info.sample_rate, lc3_codec_info.samples_per_frame, lc3_preset.qos.pd, 1);
	}
#endif
}

static void stream_stopped(struct bt_bap_stream *stream, uint8_t reason)
{
	PRINTF("Audio Stream %p stopped with reason 0x%02X\n", stream, reason);

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	hw_codec_mute();
	audio_i2s_deinit();
	le_audio_sync_stop();
#endif
}

static void stream_enabled_cb(struct bt_bap_stream *stream)
{
	/* The unicast server is responsible for starting sink ASEs after the
	 * client has enabled them.
	 */
	if (stream_dir(stream) == BT_AUDIO_DIR_SINK) {
		const int err = bt_bap_stream_start(stream);

		if (err != 0) {
			PRINTF("Failed to start stream %p: %d", stream, err);
		}
	}
}

static struct bt_bap_stream_ops stream_ops = {
	.recv = stream_recv_lc3_codec,
	.started = stream_started,
	.stopped = stream_stopped,
	.enabled = stream_enabled_cb,
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err != 0) {
		PRINTF("Failed to connect to %s (%u)\n", addr, err);

		default_conn = NULL;
		return;
	}

	PRINTF("Connected: %s\n", addr);
	default_conn = bt_conn_ref(conn);

#if CONFIG_BT_SMP
	if (bt_conn_set_security(conn, BT_SECURITY_L2))
	{
		PRINTF("Failed to set security\n");
	}
#endif
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];

	if (conn != default_conn) {
		return;
	}

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	PRINTF("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	bt_conn_unref(default_conn);
	default_conn = NULL;

	OSA_SemaphorePost(sem_disconnected);
}

#if CONFIG_BT_SMP
static void security_changed(struct bt_conn *conn, bt_security_t level, enum bt_security_err err)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Security changed: %s level %u (error %d)\n", addr, level, err);

	(void)OSA_SemaphorePost(sem_security_changed);
}

static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Passkey for %s: %06u\n", addr, passkey);
}

static void auth_cancel(struct bt_conn *conn)
{
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    PRINTF("Pairing cancelled: %s\n", addr);
}
#endif

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
#if CONFIG_BT_SMP
    .security_changed = security_changed,
#endif
};

#if CONFIG_BT_SMP
static struct bt_conn_auth_cb auth_cb_display = {
    .passkey_display = auth_passkey_display,
    .passkey_entry = NULL,
    .cancel = auth_cancel,
};
#endif

static struct bt_pacs_cap cap_sink = {
	.codec_cap = &lc3_codec_cap,
};

static int set_location(void)
{
	int err;
	enum bt_audio_location audio_location = BT_AUDIO_LOCATION_PROHIBITED;

	switch(le_audio_sink_role_get())
	{
		case AUDIO_SINK_ROLE_LEFT:  audio_location = BT_AUDIO_LOCATION_FRONT_LEFT;  break;
		case AUDIO_SINK_ROLE_RIGHT: audio_location = BT_AUDIO_LOCATION_FRONT_RIGHT; break;
		default:
			/* This is not possiable. */
			break;
	}

	if (IS_ENABLED(CONFIG_BT_PAC_SNK_LOC)) {
		err = bt_pacs_set_location(BT_AUDIO_DIR_SINK, audio_location);
		if (err != 0) {
			PRINTF("Failed to set sink location (err %d)\n", err);
			return err;
		}
	}

	PRINTF("Location successfully set\n");

	return 0;
}

static int set_supported_contexts(void)
{
	int err;

	if (IS_ENABLED(CONFIG_BT_PAC_SNK)) {
		err = bt_pacs_set_supported_contexts(BT_AUDIO_DIR_SINK,
						     (enum bt_audio_context)AVAILABLE_SINK_CONTEXT);
		if (err != 0) {
			PRINTF("Failed to set sink supported contexts (err %d)\n",
			       err);

			return err;
		}
	}

	PRINTF("Supported contexts successfully set\n");

	return 0;
}

static int set_available_contexts(void)
{
	int err;

	if (IS_ENABLED(CONFIG_BT_PAC_SNK)) {
		err = bt_pacs_set_available_contexts(BT_AUDIO_DIR_SINK,
						     (enum bt_audio_context)AVAILABLE_SINK_CONTEXT);
		if (err != 0) {
			PRINTF("Failed to set sink available contexts (err %d)\n", err);
			return err;
		}
	}

	PRINTF("Available contexts successfully set\n");
	return 0;
}

static void mcs_server_discover_cb(struct bt_conn *conn)
{
	OSA_SemaphorePost(sem_mcs_server_discovered);
}

static void vcs_server_vol_callback(uint8_t volume, uint8_t mute)
{
	if (mute)
	{
		if(hw_codec_mute())
		{
			PRINTF("\nHW Codec set mute fail!\r\n");
		}
	}
	else
	{
		if(hw_codec_vol_set(volume * 100 / 255))
		{
			PRINTF("\nHW Codec set volume fail!\r\n");
		}
	}

}

void unicast_media_receiver_task(void *param)
{
	struct bt_le_ext_adv *adv;
	int err;
	char device_name[CONFIG_BT_DEVICE_NAME_MAX];

	(void)OSA_SemaphoreCreate(sem_stream_enabled, 0);
	(void)OSA_SemaphoreCreate(sem_disconnected, 0);
	(void)OSA_SemaphoreCreate(sem_mcs_server_discovered, 0);
	(void)OSA_SemaphoreCreate(sem_security_changed, 0);

	OSA_MsgQCreate((osa_msgq_handle_t)sdu_fifo, PCM_BUFF_COUNT, sizeof(void *));

	/* shell init. */
	le_audio_shell_init();

	/* bluetooth init. */
	err = bt_enable(NULL);
	if (err != 0) {
		PRINTF("Bluetooth init failed (err %d)\n", err);
		while(1);
	}

	/* Set device name. */
	strcpy(device_name, "unicast_media_receiver");

	switch(le_audio_sink_role_get())
	{
		case AUDIO_SINK_ROLE_LEFT : strcat(device_name, "_left");  break;
		case AUDIO_SINK_ROLE_RIGHT: strcat(device_name, "_right"); break;
	}

	bt_set_name(device_name);

	bt_conn_cb_register(&conn_callbacks);
#if CONFIG_BT_SMP
    bt_conn_auth_cb_register(&auth_cb_display);
#endif

	PRINTF("Bluetooth initialized\n");

	/* VCS server init. */
	le_audio_vcs_server_init(vcs_server_vol_callback);

	/* MCS client init. */
	le_audio_mcs_client_init(mcs_server_discover_cb);

	/* Unicast server init. */
	bt_bap_unicast_server_register_cb(&unicast_server_cb);

	bt_pacs_cap_register(BT_AUDIO_DIR_SINK, &cap_sink);

	for (size_t i = 0; i < ARRAY_SIZE(sink_streams); i++) {
		bt_bap_stream_cb_register(&sink_streams[i], &stream_ops);
	}

	err = set_location();
	if (err != 0) {
		while(1);
	}

	err = set_supported_contexts();
	if (err != 0) {
		while(1);
	}

	err = set_available_contexts();
	if (err != 0) {
		while(1);
	}

	/* Create a non-connectable non-scannable advertising set */
	err = bt_le_ext_adv_create(BT_LE_EXT_ADV_CONN_NAME, NULL, &adv);
	if (err) {
		PRINTF("Failed to create advertising set (err %d)\n", err);
		while(1);
	}

	err = bt_le_ext_adv_set_data(adv, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		PRINTF("Failed to set advertising data (err %d)\n", err);
		while(1);
	}

	while (true) {
		err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
		if (err) {
			PRINTF("Failed to start advertising set (err %d)\n", err);
			break;
		}

		PRINTF("Advertising successfully started\n");

		err = OSA_SemaphoreWait(sem_security_changed, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_security_changed (err %d)\n", err);
			while(1);
		}

		PRINTF("MCS server discover:\n");

		(void)le_audio_mcs_discover(default_conn);

		err = OSA_SemaphoreWait(sem_mcs_server_discovered, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_mcs_server_discovered (err %d)\n", err);
			while(1);
		}
		PRINTF("MCS server discovered.\n");

		err = OSA_SemaphoreWait(sem_stream_enabled, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_stream_enabled (err %d)\n", err);
			while(1);
		}

		PRINTF("Unicast stream started\n");

		int res;
		do {
			res = audio_stream_decode();
			if(stream_stop)
				break;
		} while(res == 0);

		PRINTF("Unicast stream stoped\n");

		err = OSA_SemaphoreWait(sem_disconnected, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_disconnected (err %d)\n", err);
			break;
		}
	}
	
	while(1);
}
