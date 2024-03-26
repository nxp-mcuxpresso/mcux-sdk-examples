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
#include <bluetooth/conn.h>
#include <bluetooth/audio/audio.h>
#include <bluetooth/audio/bap.h>
#include <bluetooth/audio/bap_lc3_preset.h>
#include <sys/byteorder.h>

#include "le_audio_common.h"
#include "le_audio_shell.h"
#include "le_audio_service.h"

#ifndef PRINTF
#define PRINTF PRINTF
#endif

/* Audio Source parameters. */
#define MAX_AUDIO_SAMPLE_RATE		48000
#define MAX_AUDIO_CHANNEL_COUNT		2
#define MAX_AUDIO_BYTES_PER_SAMPLE 	4
#define MAX_AUDIO_BUFF_SIZE		(MAX_AUDIO_SAMPLE_RATE / 100 * MAX_AUDIO_BYTES_PER_SAMPLE)

/* wav file */
#include "host_msd_fatfs.h"
#include "wav_file.h"

wav_file_t wav_file;
static uint8_t wav_file_buff[MAX_AUDIO_CHANNEL_COUNT * MAX_AUDIO_BUFF_SIZE];

/* LC3 encoder variables. */
#include "lc3_codec.h"
lc3_encoder_t encoder[MAX_AUDIO_CHANNEL_COUNT];

static uint8_t audio_buff[MAX_AUDIO_CHANNEL_COUNT][MAX_AUDIO_BUFF_SIZE];
static uint8_t sdu_buff[MAX_AUDIO_CHANNEL_COUNT][LC3_FRAME_SIZE_MAX];

static lc3_codec_info_t lc3_codec_info;

/* When BROADCAST_ENQUEUE_COUNT > 1 we can enqueue enough buffers to ensure that
 * the controller is never idle
 */
#define UNICAST_ENQUEUE_COUNT 16U
#define TOTAL_BUF_NEEDED (UNICAST_ENQUEUE_COUNT * CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT)

BUILD_ASSERT_MSG(CONFIG_BT_ISO_TX_BUF_COUNT >= TOTAL_BUF_NEEDED,
	     "CONFIG_BT_ISO_TX_BUF_COUNT should be at least "
	     "BROADCAST_ENQUEUE_COUNT * CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT");


static void start_scan(void);

static struct bt_bap_unicast_client_cb unicast_client_cbs;
static struct bt_conn *default_conn[CONFIG_BT_MAX_CONN];
static int default_conn_index;
static struct bt_bap_unicast_group *unicast_group;
static struct audio_sink {
	struct bt_bap_ep *ep;
	uint16_t seq_num;
} sinks[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
NET_BUF_POOL_FIXED_DEFINE(tx_pool, TOTAL_BUF_NEEDED,
			  CONFIG_BT_ISO_TX_MTU + BT_ISO_CHAN_SEND_RESERVE, NULL);

static struct bt_bap_stream streams[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];

/* Select a codec configuration to apply that is mandatory to support by both client and server.
 * Allows this sample application to work without logic to parse the codec capabilities of the
 * server and selection of an appropriate codec configuration.
 */
static struct bt_bap_lc3_preset lc3_preset;

/* This parameter should be used for fix "connection timeout" issue. */
#define CONNECTION_PARAMETERS BT_LE_CONN_PARAM(80, 80, 0, 400)

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_connected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_disconnected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_mtu_exchanged);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_security_updated);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_sinks_discovered);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_vcs_discovered);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_sources_discovered);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_configured);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_qos);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_enabled);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_disabled);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_started);

static bool cis_stream_play = true;
static bool cis_stream_play_update = false;

static uint16_t get_and_incr_seq_num(const struct bt_bap_stream *stream)
{
	for (size_t i = 0U; i < CONFIG_BT_MAX_CONN; i++) {
		if (stream->ep == sinks[i].ep) {
			return sinks[i].seq_num++;
		}
	}

	PRINTF("Could not find endpoint from stream %p\n", stream);

	return 0;
}

static int audio_stream_encode(void)
{
	int res;

	/* read one frame samples. */
	do
	{
		res = wav_file_read_samples(&wav_file, wav_file_buff, lc3_codec_info.samples_per_frame);
		if(res == WAV_FILE_END)
		{
			if(wav_file_rewind(&wav_file))
			{
				PRINTF("\nwav_file_rewind fail!\n");
				return -1;
			}
			
			continue;
		}
		if(res == WAV_FILE_ERR)
		{
			PRINTF("\nwav_file_rewind fail!\n");
			return -1;
		}
	} while (res != 0);

	/* copy data from pcm form to channel format */
	if(2 == lc3_codec_info.channels)
	{
		(void)audio_data_stereo_split(lc3_codec_info.samples_per_frame, wav_file.bits, wav_file_buff, audio_buff[0], audio_buff[1]);
	}
	else
	{
		/* Todo. */
		while(1);
	}

	/* encode every channels */
	for(int i = 0; i < lc3_codec_info.channels; i++)
	{
		int lc3_res = lc3_encoder(&encoder[i], audio_buff[i], sdu_buff[i]);
		if(lc3_res)
		{
			PRINTF("\nlc3_encoder fail!\n");
		}
	}

	struct net_buf *buf[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
	for(int i = 0; i < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; i++)
	{
		do {
			buf[i] = net_buf_alloc(&tx_pool, 10);
			if(buf[i] == NULL)
			{
				PRINTF("iso net buff alloc timeout!\n");
			}
		} while (buf[i] == NULL);
		net_buf_reserve(buf[i], BT_ISO_CHAN_SEND_RESERVE);
	}

	for(int i = 0; i < lc3_codec_info.channels; i++)
	{
		net_buf_add_mem(buf[i], sdu_buff[i], lc3_codec_info.octets_per_frame);
	}

	for(int i = 0; i < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; i++)
	{
		int ret = bt_bap_stream_send(&streams[i], buf[i],
							get_and_incr_seq_num(&streams[i]),
							BT_ISO_TIMESTAMP_NONE);
		if (ret < 0) {
			/* This will end send stream. */
			PRINTF("Unable to send stream on %p: %d\n", &streams[i], ret);
			net_buf_unref(buf[i]);
			return -1;
		}
	}

	return 0;
}

static void open_wav_file(void)
{
	int res;
	/* Host msd init. */
#if (defined(BT_BLE_PLATFORM_INIT_ESCAPE) && (BT_BLE_PLATFORM_INIT_ESCAPE > 0))
	USB_HostMsdFatfsInit();
	PRINTF("FatFs initialized\n");
#endif

	/* Try to open wav file until success, because we don't know when the USB storage is ready. */
	while(true)
	{
		res = wav_file_open(&wav_file, "1:/music_16_2.wav");
		if(res)
		{
			PRINTF("\nwav_file_open fail!\n");
			OSA_TimeDelay(1000);
			continue;
		}

		PRINTF("wav file info:\n");
		PRINTF("\tsample_rate: %d\n", 	wav_file.sample_rate);
		PRINTF("\tchannels: %d\n", 		wav_file.channels);
		PRINTF("\tbits: %d\n", 			wav_file.bits);
		PRINTF("\tsize: %d\n", 			wav_file.size);
		PRINTF("\tsamples: %d\n", 		wav_file.samples);
		break;
	}

	switch (wav_file.sample_rate)
	{
		case 8000: break;
		case 16000: break;
		case 24000: break;
		case 32000: break;
		case 48000: break;
		default:
			PRINTF("\nwav file sample rate %d not support!\n", wav_file.sample_rate);
			while(1);
			break;
	}

	if(wav_file.channels != 2)
	{
		PRINTF("\nwav file not 2 channels!\n");
		while(1);
	}

	if((wav_file.bits != 16) && (wav_file.bits != 24) && (wav_file.bits != 32))
	{
		PRINTF("\nwav file %d bits not support!\n", wav_file.bits);
		while(1);
	}

	/* set the LC3 encoder parameters. */
	lc3_codec_info.sample_rate = wav_file.sample_rate;
#if LE_AUDIO_FRAME_DURATION_7_5MS
	lc3_codec_info.frame_duration_us = 7500;
#else
	lc3_codec_info.frame_duration_us = 10000;
#endif
	lc3_codec_info.octets_per_frame = lc3_preset_get_octets_per_frame_value(lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us);
	lc3_codec_info.blocks_per_sdu = 1;
	lc3_codec_info.chan_allocation = 0; /* not used. */

	lc3_codec_info.channels = wav_file.channels;
	lc3_codec_info.samples_per_frame = lc3_codec_info.sample_rate * (lc3_codec_info.frame_duration_us / 100) / 10000;
	lc3_codec_info.bytes_per_channel_frame = lc3_codec_info.samples_per_frame * wav_file.bits / 8;

	/* LC3 Encoder Init. */
	for (int i = 0; i < lc3_codec_info.channels; i++)
	{
		int lc3_res = lc3_encoder_init(&encoder[i], wav_file.sample_rate, lc3_codec_info.frame_duration_us, lc3_codec_info.octets_per_frame, wav_file.bits);
		if(lc3_res)
		{
			PRINTF("\nlc3_encoder_init fail!\n");
		}
	}
	PRINTF("LC3 encoder setup done!\n");

	/* set codec data. */
	lc3_preset.codec_cfg.id = BT_HCI_CODING_FORMAT_LC3;
#if 0 /* This part code is disabled, because codec_lookup_id() will check cid & vid. */
	lc3_preset.codec_cfg.cid = 0x0025; /* NXP Semiconductors */
	lc3_preset.codec_cfg.vid = 0x0000;
#endif

	bt_audio_codec_cfg_set_freq(&lc3_preset.codec_cfg, (enum bt_audio_codec_config_freq)bt_audio_codec_cfg_freq_hz_to_freq(lc3_codec_info.sample_rate));
	bt_audio_codec_cfg_set_frame_duration(&lc3_preset.codec_cfg, lc3_preset_get_duration_value(lc3_codec_info.frame_duration_us));
	bt_audio_codec_cfg_set_chan_allocation(&lc3_preset.codec_cfg, (enum bt_audio_location)lc3_codec_info.chan_allocation);
	bt_audio_codec_cfg_set_octets_per_frame(&lc3_preset.codec_cfg, (uint16_t)lc3_codec_info.octets_per_frame);
	bt_audio_codec_cfg_set_frame_blocks_per_sdu(&lc3_preset.codec_cfg, (uint8_t)lc3_codec_info.blocks_per_sdu);

	uint16_t meta_context = BT_AUDIO_CONTEXT_TYPE_MEDIA;
	bt_audio_codec_cfg_meta_set_val(&lc3_preset.codec_cfg, BT_AUDIO_METADATA_TYPE_STREAM_CONTEXT, (uint8_t *)&meta_context, sizeof(meta_context));

	/* set codec qos. */
#if LE_AUDIO_FRAME_DURATION_7_5MS
	struct bt_audio_codec_qos qos = BT_AUDIO_CODEC_LC3_QOS_7_5_UNFRAMED(lc3_codec_info.blocks_per_sdu * lc3_codec_info.octets_per_frame,
													 lc3_preset_get_rtn_value(lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us),
													 lc3_preset_get_latency_value(lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us),
													 40000);
#else
	struct bt_audio_codec_qos qos = BT_AUDIO_CODEC_LC3_QOS_10_UNFRAMED(lc3_codec_info.blocks_per_sdu * lc3_codec_info.octets_per_frame,
													 lc3_preset_get_rtn_value(lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us),
													 lc3_preset_get_latency_value(lc3_codec_info.sample_rate, lc3_codec_info.frame_duration_us),
													 40000);
#endif
	memcpy(&lc3_preset.qos, &qos, sizeof(struct bt_audio_codec_qos));
}

static void close_wav_file(void)
{
	int res = wav_file_close(&wav_file);
	if(res)
	{
		PRINTF("\nwav_file_close fail!\n");
	}
}

static void print_hex(const uint8_t *ptr, size_t len)
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

static void print_codec_cap(const struct bt_audio_codec_cap *codec_cap)
{
	PRINTF("codec id 0x%02x cid 0x%04x vid 0x%04x count %u\n", codec_cap->id, codec_cap->cid,
	       codec_cap->vid, codec_cap->data_len);

	if (codec_cap->id == BT_HCI_CODING_FORMAT_LC3) {
		bt_audio_data_parse(codec_cap->data, codec_cap->data_len, print_cb, "data");
	} else { /* If not LC3, we cannot assume it's LTV */
		PRINTF("data: ");
		print_hex(codec_cap->data, codec_cap->data_len);
		PRINTF("\n");
	}

	bt_audio_data_parse(codec_cap->meta, codec_cap->meta_len, print_cb, "meta");
}

static bool check_audio_support_and_connect(struct bt_data *data,
					    void *user_data)
{
	bt_addr_le_t *addr = user_data;
	char complete_name[32];

	PRINTF("[AD]: %u data_len %u\n", data->type, data->data_len);

	switch (data->type) {
	case BT_DATA_NAME_COMPLETE:
		memset(complete_name, 0, sizeof(complete_name));
		memcpy(complete_name, data->data, data->data_len);
		PRINTF("[device name]: %s\n", complete_name);

		/* Check if the device name is cis left/right. */
		if(default_conn_index == 0)
		{
			if(0 != strcmp(complete_name, "unicast_media_receiver_left"))
			{
				break;
			}
		}
		else
		{
			if(0 != strcmp(complete_name, "unicast_media_receiver_right"))
			{
				break;
			}
		}

		int err = bt_le_scan_stop();
		if (err != 0) {
			PRINTF("Failed to stop scan: %d\n", err);
			return false;
		}

		PRINTF("Audio server found; connecting\n");

		err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
			CONNECTION_PARAMETERS,
			&default_conn[default_conn_index]);
		if (err != 0) {
			PRINTF("Create conn to failed (%u)\n", err);
			start_scan();
		}

		return false; /* Stop parsing */
	}

	return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
			 struct net_buf_simple *ad)
{
	char addr_str[BT_ADDR_LE_STR_LEN];

	if (default_conn[default_conn_index] != NULL) {
		/* Already connected. */
		return;
	}

	/* We're only interested in connectable events */
	if (type != BT_GAP_ADV_TYPE_ADV_IND &&
	    type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND &&
	    type != BT_GAP_ADV_TYPE_EXT_ADV) {
		return;
	}

	(void)bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	PRINTF("Device found: %s (RSSI %d)\n", addr_str, rssi);

	/* connect only to devices in close proximity */
	if (rssi < -70) {
		return;
	}

	bt_data_parse(ad, check_audio_support_and_connect, (void *)addr);
}

static void start_scan(void)
{
	int err;

	/* This demo doesn't require active scan */
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
	if (err != 0) {
		PRINTF("Scanning failed to start (err %d)\n", err);
		return;
	}

	PRINTF("Scanning successfully started\n");
}

static void stream_configured(struct bt_bap_stream *stream,
			      const struct bt_audio_codec_qos_pref *pref)
{
	PRINTF("Audio Stream %p configured\n", stream);

	OSA_SemaphorePost(sem_stream_configured);
}

static void stream_qos_set(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p QoS set\n", stream);

	OSA_SemaphorePost(sem_stream_qos);
}

static void stream_enabled(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p enabled\n", stream);

	OSA_SemaphorePost(sem_stream_enabled);
}

static void stream_started(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p started\n", stream);

	/* Reset sequence number for sinks */
	for (size_t i = 0U; i < CONFIG_BT_MAX_CONN; i++) {
		if (stream->ep == sinks[i].ep) {
			sinks[i].seq_num = 0U;
			break;
		}
	}

	OSA_SemaphorePost(sem_stream_started);
}

static void stream_metadata_updated(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p metadata updated\n", stream);
}

static void stream_disabled(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p disabled\n", stream);

	OSA_SemaphorePost(sem_stream_disabled);
}

static void stream_stopped(struct bt_bap_stream *stream, uint8_t reason)
{
	PRINTF("Audio Stream %p stopped with reason 0x%02X\n", stream, reason);
}

static void stream_released(struct bt_bap_stream *stream)
{
	PRINTF("Audio Stream %p released\n", stream);
}

static struct bt_bap_stream_ops stream_ops = {
	.configured = stream_configured,
	.qos_set = stream_qos_set,
	.enabled = stream_enabled,
	.started = stream_started,
	.metadata_updated = stream_metadata_updated,
	.disabled = stream_disabled,
	.stopped = stream_stopped,
	.released = stream_released,
};

static void add_remote_sink(struct bt_bap_ep *ep, uint8_t index)
{
	PRINTF("Sink #%u: ep %p\n", index, ep);

	if (index >= ARRAY_SIZE(sinks)) {
		PRINTF("Could not add sink ep[%u]\n", index);
		return;
	}

	sinks[index].ep = ep;
}

static void print_remote_codec_cap(const struct bt_audio_codec_cap *codec_cap,
				   enum bt_audio_dir dir)
{
	PRINTF("codec_cap %p dir 0x%02x\n", codec_cap, dir);

	print_codec_cap(codec_cap);
}

static void discover_sinks_cb(struct bt_conn *conn, int err, enum bt_audio_dir dir)
{
	if (err != 0 && err != BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
		PRINTF("Discovery failed: %d\n", err);
		return;
	}

	if (err == BT_ATT_ERR_ATTRIBUTE_NOT_FOUND) {
		PRINTF("Discover sinks completed without finding any sink ASEs\n");
	} else {
		PRINTF("Discover sinks complete: err %d\n", err);
	}

	OSA_SemaphorePost(sem_sinks_discovered);

}

static void connected(struct bt_conn *conn, uint8_t err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	(void)bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (err != 0) {
		PRINTF("Failed to connect to %s (%u)\n", addr, err);

		bt_conn_unref(default_conn[default_conn_index]);
		default_conn[default_conn_index] = NULL;

		start_scan();
		return;
	}

	if (conn != default_conn[default_conn_index]) {
		return;
	}

	PRINTF("Connected: %s\n", addr);
	OSA_SemaphorePost(sem_connected);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int i;

	PRINTF("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	for (i = 0; i < CONFIG_BT_MAX_CONN; i++)
	{
		if(conn == default_conn[i])
		{
			break;
		}
	}

	if(i >= CONFIG_BT_MAX_CONN)
	{
		return;
	}

	if (conn != default_conn[i]) {
		return;
	}

	(void)bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	PRINTF("Disconnected: %s (reason 0x%02x)\n", addr, reason);

	bt_conn_unref(default_conn[i]);
	default_conn[i] = NULL;

	OSA_SemaphorePost(sem_disconnected);
}

static void security_changed_cb(struct bt_conn *conn, bt_security_t level,
				enum bt_security_err err)
{
	if (err == 0) {
		OSA_SemaphorePost(sem_security_updated);
	} else {
		PRINTF("Failed to set security level: %u", err);
	}
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
    .security_changed = security_changed_cb
};

static void att_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx)
{
	PRINTF("MTU exchanged: %u/%u\n", tx, rx);
	OSA_SemaphorePost(sem_mtu_exchanged);
}

static struct bt_gatt_cb gatt_callbacks = {
	.att_mtu_updated = att_mtu_updated,
};

static void unicast_client_location_cb(struct bt_conn *conn,
				      enum bt_audio_dir dir,
				      enum bt_audio_location loc)
{
	PRINTF("dir %u loc %X\n", dir, loc);
}

static void available_contexts_cb(struct bt_conn *conn,
				  enum bt_audio_context snk_ctx,
				  enum bt_audio_context src_ctx)
{
	PRINTF("snk ctx %u src ctx %u\n", snk_ctx, src_ctx);
}

static void pac_record_cb(struct bt_conn *conn, enum bt_audio_dir dir,
			  const struct bt_audio_codec_cap *codec_cap)
{
	print_remote_codec_cap(codec_cap, dir);
}

static void endpoint_cb(struct bt_conn *conn, enum bt_audio_dir dir, struct bt_bap_ep *ep)
{
	int index;

	for(index = 0; index < CONFIG_BT_MAX_CONN; index++)
	{
		if(conn == default_conn[index])
		{
			break;
		}
	}

	if (dir == BT_AUDIO_DIR_SINK) {
		add_remote_sink(ep, index);
	}
}

static struct bt_bap_unicast_client_cb unicast_client_cbs = {
	.location = unicast_client_location_cb,
	.available_contexts = available_contexts_cb,
	.pac_record = pac_record_cb,
	.endpoint = endpoint_cb,
};

static int init(void)
{
	int err;

    (void)OSA_SemaphoreCreate(sem_connected, 0);
    (void)OSA_SemaphoreCreate(sem_disconnected, 0);
    (void)OSA_SemaphoreCreate(sem_mtu_exchanged, 0);
    (void)OSA_SemaphoreCreate(sem_security_updated, 0);
    (void)OSA_SemaphoreCreate(sem_sinks_discovered, 0);
	(void)OSA_SemaphoreCreate(sem_vcs_discovered, 0);
    (void)OSA_SemaphoreCreate(sem_sources_discovered, 0);
    (void)OSA_SemaphoreCreate(sem_stream_configured, 0);
    (void)OSA_SemaphoreCreate(sem_stream_qos, 0);
    (void)OSA_SemaphoreCreate(sem_stream_enabled, 0);
    (void)OSA_SemaphoreCreate(sem_stream_disabled, 0);
    (void)OSA_SemaphoreCreate(sem_stream_started, 0);

	err = bt_enable(NULL);
	if (err != 0) {
		PRINTF("Bluetooth enable failed (err %d)\n", err);
		return err;
	}

	bt_conn_cb_register(&conn_callbacks);

	for (size_t i = 0; i < ARRAY_SIZE(streams); i++) {
		streams[i].ops = &stream_ops;
	}

	bt_gatt_cb_register(&gatt_callbacks);

	return 0;
}

static int scan_and_connect(int index)
{
	int err;

	if(index == 0)
	{
		PRINTF("Scan & connect left cis sink:\n");
		default_conn_index = 0;
	}
	if(index == 1)
	{
		PRINTF("Scan & connect right cis sink:\n");
		default_conn_index = 1;
	}
	
	start_scan();

	err = OSA_SemaphoreWait(sem_connected, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_connected (err %d)\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_mtu_exchanged, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_mtu_exchanged (err %d)\n", err);
		return err;
	}

	err = bt_conn_set_security(default_conn[index], BT_SECURITY_L2);
	if (err != 0) {
		PRINTF("failed to set security (err %d)\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_security_updated, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_security_updated (err %d)\n", err);
		return err;
	}

	return 0;
}

static void vcs_client_discover_callback(struct bt_conn *conn, int err)
{
	if(err)
	{
		PRINTF("VCS discover failed with %d\n", err);
		return;
	}

	(void)OSA_SemaphorePost(sem_vcs_discovered);
}

static int discover_vcs(int index)
{
	int ret;
	ret = le_audio_vcs_discover(default_conn[index], index);
	if(ret) 
	{
		PRINTF("vcs discover fail %d\n", ret);
		return -1;
	}

	(void)OSA_SemaphoreWait(sem_vcs_discovered, osaWaitForever_c);
        
        return 0;
}

static int discover_sinks(int index)
{
	int err;

	unicast_client_cbs.discover = discover_sinks_cb;

	err = bt_bap_unicast_client_discover(default_conn[index], BT_AUDIO_DIR_SINK);
	if (err != 0) {
		PRINTF("Failed to discover sinks: %d\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_sinks_discovered, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_sinks_discovered (err %d)\n", err);
		return err;
	}

	return 0;
}

static int configure_stream(struct bt_bap_stream *stream, struct bt_bap_ep *ep)
{
	int err;
	int i;
	enum bt_audio_location chan_allocation_value;

	for(i = 0; i < CONFIG_BT_MAX_CONN; i++)
	{
		if(ep == sinks[i].ep)
		{
			break;
		}
	}

	if(i >= CONFIG_BT_MAX_CONN)
	{
		return -1;
	}

	/* change channel allocation. */
	chan_allocation_value = (i == 0) ? BT_AUDIO_LOCATION_FRONT_LEFT : BT_AUDIO_LOCATION_FRONT_RIGHT;
	bt_audio_codec_cfg_set_chan_allocation(&lc3_preset.codec_cfg, chan_allocation_value);

	err = bt_bap_stream_config(default_conn[i], stream, ep,
					&lc3_preset.codec_cfg);
	if (err != 0) {
		return err;
	}

	err = OSA_SemaphoreWait(sem_stream_configured, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_stream_configured (err %d)\n", err);
		return err;
	}

	return 0;
}

static int configure_streams(int index)
{
	int err;

	struct bt_bap_ep *ep = sinks[index].ep;
	struct bt_bap_stream *stream = &streams[index];

	if (ep == NULL) {
		return -1;
	}

	err = configure_stream(stream, ep);
	if (err != 0) {
		PRINTF("Could not configure sink stream[%zu]: %d\n",
				index, err);
		return err;
	}

	PRINTF("Configured sink stream[%zu]\n", index);

	return 0;
}

static int create_group(void)
{
	const size_t params_count = CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT;
	struct bt_bap_unicast_group_stream_pair_param pair_params[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT + CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT];
	struct bt_bap_unicast_group_stream_param stream_params[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT + CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SRC_COUNT];
	struct bt_bap_unicast_group_param param;
	int err;

	for (size_t i = 0U; i < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; i++) {
		stream_params[i].stream = &streams[i];
		stream_params[i].qos = &lc3_preset.qos;
		
		pair_params[i].tx_param = &stream_params[i];
		pair_params[i].rx_param = NULL;
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

static int delete_group(void)
{
	int err;

	err = bt_bap_unicast_group_delete(unicast_group);
	if (err != 0) {
		PRINTF("Could not create unicast group (err %d)\n", err);
		return err;
	}

	return 0;
}

static int set_stream_qos(int index)
{
	int err;

	err = bt_bap_stream_qos(default_conn[index], unicast_group);
	if (err != 0) {
		PRINTF("Unable to setup QoS: %d\n", err);
		return err;
	}

	PRINTF("QoS: waiting for %zu streams\n", index);
	(void)OSA_SemaphoreWait(sem_stream_qos, osaWaitForever_c);

	return 0;
}

static int enable_streams(int index)
{
	int err;

	err = bt_bap_stream_enable(&streams[index],
						lc3_preset.codec_cfg.meta,
						lc3_preset.codec_cfg.meta_len);
	if (err != 0) {
		PRINTF("Unable to enable stream: %d\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_stream_enabled, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_stream_enabled (err %d)\n", err);
		return err;
	}

	return 0;
}

static int disable_streams(int index)
{
	int err;

	err = bt_bap_stream_disable(&streams[index]);
	if (err != 0) {
		PRINTF("Unable to enable stream: %d\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_stream_disabled, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_stream_disabled (err %d)\n", err);
		return err;
	}

	return 0;
}

static int start_streams(int index)
{
	int err;

	err = bt_bap_stream_start(&streams[index]);
	if (err != 0) {
		PRINTF("Unable to start stream: %d\n", err);
		return err;
	}

	err = OSA_SemaphoreWait(sem_stream_started, osaWaitForever_c);
	if (err != 0) {
		PRINTF("failed to take sem_stream_started (err %d)\n", err);
		return err;
	}

	return 0;
}

static void reset_data(void)
{
	(void)OSA_SemaphoreDestroy(sem_connected);
	(void)OSA_SemaphoreDestroy(sem_disconnected);
	(void)OSA_SemaphoreDestroy(sem_mtu_exchanged);
	(void)OSA_SemaphoreDestroy(sem_security_updated);
	(void)OSA_SemaphoreDestroy(sem_sinks_discovered);
	(void)OSA_SemaphoreDestroy(sem_vcs_discovered);
	(void)OSA_SemaphoreDestroy(sem_sources_discovered);
	(void)OSA_SemaphoreDestroy(sem_stream_configured);
	(void)OSA_SemaphoreDestroy(sem_stream_qos);
	(void)OSA_SemaphoreDestroy(sem_stream_enabled);
	(void)OSA_SemaphoreDestroy(sem_stream_disabled);
	(void)OSA_SemaphoreDestroy(sem_stream_started);

    (void)OSA_SemaphoreCreate(sem_connected, 0);
    (void)OSA_SemaphoreCreate(sem_disconnected, 0);
    (void)OSA_SemaphoreCreate(sem_mtu_exchanged, 0);
    (void)OSA_SemaphoreCreate(sem_security_updated, 0);
    (void)OSA_SemaphoreCreate(sem_sinks_discovered, 0);
	(void)OSA_SemaphoreCreate(sem_vcs_discovered, 0);
    (void)OSA_SemaphoreCreate(sem_sources_discovered, 0);
    (void)OSA_SemaphoreCreate(sem_stream_configured, 0);
    (void)OSA_SemaphoreCreate(sem_stream_qos, 0);
    (void)OSA_SemaphoreCreate(sem_stream_enabled, 0);
    (void)OSA_SemaphoreCreate(sem_stream_disabled, 0);
    (void)OSA_SemaphoreCreate(sem_stream_started, 0);

	memset(sinks, 0, sizeof(sinks));
}

void mcs_server_state_cb(int state)
{
	if(state == MCS_SERVER_STATE_PLAYING)
	{
		PRINTF("\nMCS server state: playing\n");
		if(cis_stream_play)
		{
			return;
		}

		cis_stream_play_update = true;
		cis_stream_play = true;
	}

	if(state == MCS_SERVER_STATE_PAUSED)
	{
		PRINTF("\nMCS server state: pause\n");
		if(!cis_stream_play)
		{
			return;
		}

		cis_stream_play_update = true;
		cis_stream_play = false;
	}
}

void unicast_media_sender_task(void *param)
{
	int err;

	/* shell init. */
	le_audio_shell_init();

	PRINTF("Initializing\n");
	err = init();
	if (err != 0) {
		while(1);
	}
	PRINTF("Initialized\n");

	/* VCS */
	le_audio_vcs_client_init(vcs_client_discover_callback);

	/* MCS */
	le_audio_mcs_server_init(mcs_server_state_cb);

	/* Open wav file */
	open_wav_file();

	err = bt_bap_unicast_client_register_cb(&unicast_client_cbs);
	if (err != 0) {
		PRINTF("Failed to register client callbacks: %d", err);
		while(1);
	}

	while (true) {
		reset_data();

		PRINTF("Creating unicast group\n");
		err = create_group();
		if (err != 0) {
			break;
		}
		PRINTF("Unicast group created\n");

		for (int i = 0; i < CONFIG_BT_MAX_CONN; i++)
        {
			PRINTF("Waiting for connection\n");
			err = scan_and_connect(i);
			if (err != 0) {
				break;
			}
			PRINTF("Connected\n");
		}

		for (int i = 0; i < CONFIG_BT_MAX_CONN; i++)
        {
			PRINTF("Discover VCS\n");
			err = discover_vcs(i);
			if(err)
			{
				PRINTF("discover vcs %d failed, error %d\n", i, err);
			}
			PRINTF("Discover VCS complete.\n");

			PRINTF("Discovering sinks\n");
			err = discover_sinks(i);
			if (err != 0) {
				break;
			}
			PRINTF("Sinks discovered\n");

			PRINTF("Configuring streams\n");
			err = configure_streams(i);
			if (err != 0) {
				break;
			}
			PRINTF("Stream configured\n");

			PRINTF("Setting stream QoS\n");
			err = set_stream_qos(i);
			if (err != 0) {
				break;
			}
			PRINTF("Stream QoS Set\n");

			PRINTF("Enabling streams\n");
			err = enable_streams(i);
			if (err != 0) {
				break;
			}
			PRINTF("Streams enabled\n");

			PRINTF("Starting streams\n");
			err = start_streams(i);
			if (err != 0) {
				break;
			}
			PRINTF("Streams started\n");
		}

		int res;
		do{
			if (cis_stream_play)
			{
				if(cis_stream_play_update)
				{
					cis_stream_play_update = false;
					for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
					{
						err = enable_streams(i);
						if(err)
						{
							PRINTF("\nEnable stream %d err %d\n", i, err);
						}

						err = start_streams(i);
						if(err)
						{
							PRINTF("\nStart stream %d err %d\n", i, err);
						}
					}
				}
				res = audio_stream_encode();
			}
			else
			{
				if(cis_stream_play_update)
				{
					cis_stream_play_update = false;
#if 1				/* MCUX-62728: walkaround to disconnect 2rd CIS first for controller fw limitation. */
					for(int i = 1; i >= 0; i--)
#else
					for(int i = 0; i < CONFIG_BT_MAX_CONN; i++)
#endif
					{
						err = disable_streams(i);
						if(err)
						{
							PRINTF("\nDisable stream %d err %d\n", i, err);
						}
					}
				}
				OSA_TimeDelay(2);
			}
		} while (0 == res);

		/* Wait for disconnect */
		err = OSA_SemaphoreWait(sem_disconnected, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_disconnected (err %d)\n", err);
			break;
		}

		PRINTF("Deleting group\n");
		err = delete_group();
		if (err != 0) {
			break;
		}
		PRINTF("Group deleted\n");
	}

	close_wav_file();

	while(1);
}
