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
#include <bluetooth/audio/audio.h>
#include <bluetooth/audio/bap.h>
#include <bluetooth/audio/bap_lc3_preset.h>
#include <bluetooth/audio/pacs.h>

#include "le_audio_common.h"
#include "le_audio_shell.h"
#include "broadcast_media_receiver.h"

#ifndef printk
#define printk PRINTF
#endif

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#endif

/* Audio Sink parameters. */
#define MAX_AUDIO_SAMPLE_RATE		48000
#define MAX_AUDIO_CHANNEL_COUNT		2
#define MAX_AUDIO_BYTES_PER_SAMPLE 	4

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
static lc3_decoder_t decoder;

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

#define INVALID_BROADCAST_ID      (BT_AUDIO_BROADCAST_ID_MAX + 1)

#define SYNC_RETRY_COUNT          6 /* similar to retries for connections */
#define PA_SYNC_SKIP              5

/* Semaphores */
#define SEM_TIMEOUT 30 * 1000
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_broadcaster_found);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_pa_synced);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_base_received);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_syncable);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_broadcast_code_received);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_started);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stream_stopped);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_pa_sync_lost);

volatile static bool base_info_received = false;

static struct bt_bap_broadcast_sink *broadcast_sink;
static struct bt_le_scan_recv_info broadcaster_info;
static bt_addr_le_t broadcaster_addr;
static struct bt_le_per_adv_sync *pa_sync;
static uint32_t broadcaster_broadcast_id;
static struct bt_bap_stream streams[CONFIG_BT_BAP_BROADCAST_SNK_STREAM_COUNT];

/* Mandatory support preset by both source and sink */
static struct bt_bap_lc3_preset lc3_preset;

static const struct bt_audio_codec_cap codec_cap_10 = BT_AUDIO_CODEC_CAP_LC3(
	BT_AUDIO_CODEC_LC3_FREQ_8KHZ | BT_AUDIO_CODEC_LC3_FREQ_16KHZ | BT_AUDIO_CODEC_LC3_FREQ_24KHZ | BT_AUDIO_CODEC_LC3_FREQ_32KHZ | BT_AUDIO_CODEC_LC3_FREQ_48KHZ,
	BT_AUDIO_CODEC_LC3_DURATION_10, BT_AUDIO_CODEC_LC3_CHAN_COUNT_SUPPORT(1), 30u, 130u, 1u,
	BT_AUDIO_CONTEXT_TYPE_MEDIA);

static const struct bt_audio_codec_cap codec_cap_7_5 = BT_AUDIO_CODEC_CAP_LC3(
	BT_AUDIO_CODEC_LC3_FREQ_8KHZ | BT_AUDIO_CODEC_LC3_FREQ_16KHZ | BT_AUDIO_CODEC_LC3_FREQ_24KHZ | BT_AUDIO_CODEC_LC3_FREQ_32KHZ | BT_AUDIO_CODEC_LC3_FREQ_48KHZ,
	BT_AUDIO_CODEC_LC3_DURATION_7_5, BT_AUDIO_CODEC_LC3_CHAN_COUNT_SUPPORT(1), 26u, 97u, 1u,
	BT_AUDIO_CONTEXT_TYPE_MEDIA);

static uint32_t requested_bis_sync;
static uint32_t bis_index_bitfield;
static uint8_t sink_broadcast_code[BT_AUDIO_BROADCAST_CODE_SIZE];
static bool stream_stoped = false;

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
static uint32_t get_big_sync_delay(void)
{
	struct bt_iso_info iso_info;
	uint32_t BIG_Sync_Delay_us;
	uint32_t SDU_Interval_us;
	uint32_t ISO_Interval_us;
	uint32_t Transport_Latency_BIG_us;
	uint32_t PTO;
	uint32_t BN;
	uint32_t IRC;
	uint32_t NSE;

	bt_iso_chan_get_info(broadcast_sink->bis[0], &iso_info);

	SDU_Interval_us = lc3_codec_info.frame_duration_us;
	ISO_Interval_us = iso_info.iso_interval * 1250;
	Transport_Latency_BIG_us = iso_info.sync_receiver.latency;
	PTO = iso_info.sync_receiver.pto;
	BN  = iso_info.sync_receiver.bn;
	IRC = iso_info.sync_receiver.irc;
	NSE = iso_info.max_subevent;

	/* unframe mode: ISO_interval == N * SDU_interval */
	/* frame mode: ISO_interval != N * SDU_interval */
	if(ISO_Interval_us % SDU_Interval_us == 0)
	{
		BIG_Sync_Delay_us = Transport_Latency_BIG_us - ((PTO * (NSE / BN - IRC) + 1) * ISO_Interval_us - SDU_Interval_us);
	}
	else
	{
		BIG_Sync_Delay_us = Transport_Latency_BIG_us - (PTO * (NSE / BN - IRC) * ISO_Interval_us + ISO_Interval_us + SDU_Interval_us);
	}

	return BIG_Sync_Delay_us;
}

static uint32_t get_iso_interval(void)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(broadcast_sink->bis[0], &iso_info);

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

	osa_status_t status;
	do {
		status = OSA_MsgQGet(sdu_fifo, &sdu_buf, 10);
		if(KOSA_StatusSuccess != status)
		{
			if(stream_stoped)
			{
				return -1;
			}
		}
	} while(KOSA_StatusSuccess != status);

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
		(void)audio_data_make_stereo(lc3_codec_info.samples_per_frame, 16, audio_buff[0], audio_buff[0], audio_i2s_buff);
	}
	else
	{
		/* Todo. */
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

static void stream_started_cb(struct bt_bap_stream *stream)
{
	PRINTF("Stream %p started\n", stream);
	stream_stoped = false;
	OSA_SemaphorePost(sem_stream_started);

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
        le_audio_sync_start(get_iso_interval(), get_big_sync_delay(), lc3_codec_info.sample_rate, lc3_codec_info.samples_per_frame, lc3_preset.qos.pd, 0);
    }
    if(AUDIO_SINK_ROLE_RIGHT == le_audio_sink_role_get())
    {
        /* right stream start event is after first sync signal, so we set sync_index_init to 1. */
        le_audio_sync_start(get_iso_interval(), get_big_sync_delay(), lc3_codec_info.sample_rate, lc3_codec_info.samples_per_frame, lc3_preset.qos.pd, 1);
    }
#endif
}

static void stream_stopped_cb(struct bt_bap_stream *stream, uint8_t reason)
{
	PRINTF("Stream %p stopped\n", stream);
	stream_stoped = true;

#if defined(LE_AUDIO_SYNC_ENABLE) && (LE_AUDIO_SYNC_ENABLE > 0)
	hw_codec_mute();
	audio_i2s_deinit();
	le_audio_sync_stop();
#endif

	OSA_SemaphorePost(sem_stream_stopped);
}

static void stream_recv_cb(struct bt_bap_stream *stream,
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

static struct bt_bap_stream_ops stream_ops = {
	.started = stream_started_cb,
	.stopped = stream_stopped_cb,
	.recv = stream_recv_cb
};

static bool bis_data_parse(struct bt_data *data, void *user_data)
{
    struct bt_bap_base_bis_data const *bis_data = user_data;

	bis_index_bitfield |= BIT(bis_data->index);

    if(data->type == BT_AUDIO_CODEC_CONFIG_LC3_CHAN_ALLOC)
    {
            uint32_t channel_allocation = 0;
            memcpy(&channel_allocation, data->data, data->data_len);
            
            if(channel_allocation == (uint32_t)BT_AUDIO_LOCATION_FRONT_LEFT)
            {
                    if(AUDIO_SINK_ROLE_LEFT == le_audio_sink_role_get())
                    {
                            bt_audio_codec_cfg_set_chan_allocation(&lc3_preset.codec_cfg, (enum bt_audio_location)channel_allocation);

                            requested_bis_sync = BIT(bis_data->index);
                            
                            return false;
                    }
            }
            else if(channel_allocation == (uint32_t)BT_AUDIO_LOCATION_FRONT_RIGHT)
            {
                    if(AUDIO_SINK_ROLE_RIGHT == le_audio_sink_role_get())
                    {
                            bt_audio_codec_cfg_set_chan_allocation(&lc3_preset.codec_cfg, (enum bt_audio_location)channel_allocation);

                            requested_bis_sync = BIT(bis_data->index);
                            
                            return false;
                    }
            }
            else
            {
                    PRINTF("\nBASE channel allocation 0x%08x not support.\n", channel_allocation);
            }
    }
    
    return true;
}

static void base_recv_cb(struct bt_bap_broadcast_sink *sink, const struct bt_bap_base *base)
{
	if (base_info_received) {
		return;
	}

	PRINTF("Received BASE with %u subgroups from broadcast sink %p\n",
	       base->subgroup_count, sink);

	/* Here just select the first subgroup. */
	for(int i = 0; i < base->subgroup_count; i++)
	{
		/* Here we only support one subgroup. */
		if(i == 0)
		{
			/* Save codec */
			memcpy(&lc3_preset.codec_cfg, &base->subgroups[0].codec_cfg, sizeof(lc3_preset.codec_cfg));

			/* Audio channel selection. */
			for(int j = 0; j < base->subgroups[0].bis_count; j++)
			{
				struct bt_bap_base_bis_data const *bis_data = &base->subgroups[0].bis_data[j];

				(void)bt_audio_data_parse(bis_data->data, bis_data->data_len, bis_data_parse, (void *)bis_data);
			}
		}
		else
		{
			PRINTF("\nBIG subgroup %d/%d not support!\n", i, base->subgroup_count);
		}
	}

	base_info_received = true;
	OSA_SemaphorePost(sem_base_received);
}

static void syncable_cb(struct bt_bap_broadcast_sink *sink, bool encrypted)
{
	if (encrypted) {
		PRINTF("Cannot sync to encrypted broadcast source\n");
		return;
	}

	OSA_SemaphorePost(sem_syncable);

	if (!encrypted) {
		OSA_SemaphorePost(sem_broadcast_code_received);
	}
}

static struct bt_bap_broadcast_sink_cb broadcast_sink_cbs = {
	.base_recv = base_recv_cb,
	.syncable = syncable_cb,
};

static uint16_t interval_to_sync_timeout(uint16_t interval)
{
	uint32_t interval_ms;
	uint16_t timeout;

	/* Ensure that the following calculation does not overflow silently */
	__ASSERT(SYNC_RETRY_COUNT < 10, "SYNC_RETRY_COUNT shall be less than 10");

	/* Add retries and convert to unit in 10's of ms */
	interval_ms = BT_GAP_PER_ADV_INTERVAL_TO_MS(interval);
	timeout = (interval_ms * SYNC_RETRY_COUNT) / 10;

	/* Enforce restraints */
	timeout = CLAMP(timeout, BT_GAP_PER_ADV_MIN_TIMEOUT,
			BT_GAP_PER_ADV_MAX_TIMEOUT);

	return timeout;
}

static struct bt_pacs_cap cap_10 = {
	.codec_cap = &codec_cap_10,
};

static struct bt_pacs_cap cap_7_5 = {
	.codec_cap = &codec_cap_7_5,
};

static void audio_codec_config(void)
{
	/* Get codec info. */
	lc3_codec_info.sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_config_freq)bt_audio_codec_cfg_get_freq(&lc3_preset.codec_cfg));
	lc3_codec_info.frame_duration_us = bt_audio_codec_cfg_get_frame_duration_us(&lc3_preset.codec_cfg);
	lc3_codec_info.samples_per_frame = lc3_codec_info.sample_rate * (lc3_codec_info.frame_duration_us / 100) / 10000;
	lc3_codec_info.octets_per_frame = bt_audio_codec_cfg_get_octets_per_frame(&lc3_preset.codec_cfg);
	lc3_codec_info.blocks_per_sdu = bt_audio_codec_cfg_get_frame_blocks_per_sdu(&lc3_preset.codec_cfg, true);
	bt_audio_codec_cfg_get_chan_allocation(&lc3_preset.codec_cfg, (enum bt_audio_location *)&lc3_codec_info.chan_allocation);
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
}

static bool scan_check_broadcast_id(struct bt_data *data, void *user_data)
{
	struct bt_uuid_16 adv_uuid;

	if (data->type != BT_DATA_SVC_DATA16) {
		return true;
	}

	if (data->data_len < BT_UUID_SIZE_16 + BT_AUDIO_BROADCAST_ID_SIZE) {
		return true;
	}

	if (!bt_uuid_create(&adv_uuid.uuid, data->data, BT_UUID_SIZE_16)) {
		return true;
	}

	if (bt_uuid_cmp(&adv_uuid.uuid, BT_UUID_BROADCAST_AUDIO)) {
		return true;
	}

	broadcaster_broadcast_id = sys_get_le24(data->data + BT_UUID_SIZE_16);
        
	return false;
}

static bool scan_check_and_sync_broadcast(struct bt_data *data, void *user_data)
{
	bool *found = user_data;
	char device_name[CONFIG_BT_DEVICE_NAME_MAX];

	if(data->type == BT_DATA_NAME_COMPLETE)
	{
		memset(device_name, 0, sizeof(device_name));
		memcpy(device_name, data->data, data->data_len);
		if(0 == strcmp(device_name, "broadcast_media_sender"))
		{
			PRINTF("\n[device name]:%s\n", device_name);
			PRINTF("connect...\n");
			*found = true;
			return false;
		}
	}

	return true;
}

static void broadcast_scan_recv(const struct bt_le_scan_recv_info *info, struct net_buf_simple *buf)
{
	bool found = false;
	struct net_buf_simple_state buf_state;

	net_buf_simple_save(buf, &buf_state);

	bt_data_parse(buf, scan_check_and_sync_broadcast, &found);

	net_buf_simple_restore(buf, &buf_state);

	if(found)
	{
		/* Store info for PA sync parameters */
		memcpy(&broadcaster_info, info, sizeof(broadcaster_info));
		bt_addr_le_copy(&broadcaster_addr, info->addr);
		
		bt_data_parse(buf, scan_check_broadcast_id, NULL);

		OSA_SemaphorePost(sem_broadcaster_found);
	}
}

static struct bt_le_scan_cb bap_scan_cb = {
	.recv = broadcast_scan_recv,
};

static void bap_pa_sync_synced_cb(struct bt_le_per_adv_sync *sync,
		       struct bt_le_per_adv_sync_synced_info *info)
{
	PRINTF("PA synced for sync %p with sid 0x%02X\n",
	       sync, info->sid);

	OSA_SemaphorePost(sem_pa_synced);
}

static void bap_pa_sync_terminated_cb(struct bt_le_per_adv_sync *sync,
				      const struct bt_le_per_adv_sync_term_info *info)
{
	if (sync == pa_sync) {
		printk("PA sync %p lost with reason %u\n", sync, info->reason);
		pa_sync = NULL;

		//k_sem_give(&sem_pa_sync_lost);
	}
}

static struct bt_le_per_adv_sync_cb bap_pa_sync_cb = {
	.synced = bap_pa_sync_synced_cb,
	.term = bap_pa_sync_terminated_cb,
};

static int init(void)
{
	int err;

	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth enable failed (err %d)\n", err);
		return err;
	}

	printk("Bluetooth initialized\n");

	bt_bap_broadcast_sink_init();

	err = bt_pacs_cap_register(BT_AUDIO_DIR_SINK, &cap_10);
	if (err) {
		printk("Capability register failed (err %d)\n", err);
		return err;
	}

	err = bt_pacs_cap_register(BT_AUDIO_DIR_SINK, &cap_7_5);
	if (err) {
		PRINTF("Capability register failed (err %d)\n", err);
		return err;
	}

	bt_bap_broadcast_sink_register_cb(&broadcast_sink_cbs);
	bt_le_per_adv_sync_cb_register(&bap_pa_sync_cb);
	bt_le_scan_cb_register(&bap_scan_cb);

	for (size_t i = 0U; i < ARRAY_SIZE(streams); i++) {
		streams[i].ops = &stream_ops;
	}

	return 0;
}

static int reset(void)
{
	int err;

	bis_index_bitfield = 0U;
	base_info_received = false;
	requested_bis_sync = 0U;

	(void)memset(sink_broadcast_code, 0, sizeof(sink_broadcast_code));
	(void)memset(&broadcaster_info, 0, sizeof(broadcaster_info));
	(void)memset(&broadcaster_addr, 0, sizeof(broadcaster_addr));
	broadcaster_broadcast_id = INVALID_BROADCAST_ID;

	OSA_SemaphoreDestroy(sem_broadcaster_found);
	OSA_SemaphoreDestroy(sem_pa_synced);
	OSA_SemaphoreDestroy(sem_base_received);
	OSA_SemaphoreDestroy(sem_syncable);
	OSA_SemaphoreDestroy(sem_broadcast_code_received);
	OSA_SemaphoreDestroy(sem_stream_started);
	OSA_SemaphoreDestroy(sem_stream_stopped);
	OSA_SemaphoreDestroy(sem_pa_sync_lost);

	OSA_SemaphoreCreate(sem_broadcaster_found, 0);
	OSA_SemaphoreCreate(sem_pa_synced, 0);
	OSA_SemaphoreCreate(sem_base_received, 0);
	OSA_SemaphoreCreate(sem_syncable, 0);
	OSA_SemaphoreCreate(sem_broadcast_code_received, 0);
	OSA_SemaphoreCreate(sem_stream_started, 0);
	OSA_SemaphoreCreate(sem_stream_stopped, 0);
	OSA_SemaphoreCreate(sem_pa_sync_lost, 0);

	if (broadcast_sink != NULL) {
		err = bt_bap_broadcast_sink_stop(broadcast_sink);
		if (err && (err != -EALREADY)) {
			PRINTF("Deleting broadcast sink failed (err %d)\n", err);
		}

		err = bt_bap_broadcast_sink_delete(broadcast_sink);
		if (err) {
			printk("Deleting broadcast sink failed (err %d)\n", err);

			return err;
		}

		broadcast_sink = NULL;
	}

	if (pa_sync != NULL) {
		err = bt_le_per_adv_sync_delete(pa_sync);
		if (err) {
			printk("Deleting PA sync failed (err %d)\n", err);

			return err;
		}

		pa_sync = NULL;
	}

#if 0
	err = bt_bap_broadcast_sink_scan_stop();
	if (err && (err != -EALREADY)) {
		PRINTF("Stop broadcast sink scan failed (err %d)\n", err);
	}
#endif

	OSA_TimeDelay(1000);
        
        return 0;
}

static volatile bool bis_stream_play = true;
static volatile bool bis_stream_play_update = false;

void le_audio_bis_play(void)
{
	if(bis_stream_play)
	{
		return;
	}

	bis_stream_play_update = true;
	bis_stream_play = true;
}

void le_audio_bis_pause(void)
{
	if(!bis_stream_play)
	{
		return;
	}

	bis_stream_play_update = true;
	bis_stream_play = false;
}

static int pa_sync_create(void)
{
	struct bt_le_per_adv_sync_param create_params = {0};

	bt_addr_le_copy(&create_params.addr, &broadcaster_addr);
	create_params.options = BT_LE_PER_ADV_SYNC_OPT_NONE;
	create_params.sid = broadcaster_info.sid;
	create_params.skip = PA_SYNC_SKIP;
	create_params.timeout = interval_to_sync_timeout(broadcaster_info.interval);

	return bt_le_per_adv_sync_create(&create_params, &pa_sync);
}

void broadcast_media_receiver_task(void *param)
{
	struct bt_bap_stream *streams_p[ARRAY_SIZE(streams)];
	int err;

	OSA_SemaphoreCreate(sem_broadcaster_found, 0);
	OSA_SemaphoreCreate(sem_pa_synced, 0);
	OSA_SemaphoreCreate(sem_base_received, 0);
	OSA_SemaphoreCreate(sem_syncable, 0);
	OSA_SemaphoreCreate(sem_broadcast_code_received, 0);
	OSA_SemaphoreCreate(sem_stream_started, 0);
	OSA_SemaphoreCreate(sem_stream_stopped, 0);
	OSA_SemaphoreCreate(sem_pa_sync_lost, 0);

	OSA_MsgQCreate((osa_msgq_handle_t)sdu_fifo, PCM_BUFF_COUNT, sizeof(void *));

	/* shell init. */
	le_audio_shell_init();

	/* bluetooth init. */
	err = init();
	if (err) {
		printk("Init failed (err %d)\n", err);
		while(1);
	}

	for (size_t i = 0U; i < ARRAY_SIZE(streams_p); i++) {
		streams_p[i] = &streams[i];
	}

	while (true) {
		err = reset();
		if (err != 0) {
			printk("Resetting failed: %d - Aborting\n", err);

			continue;
		}

		printk("Scanning for broadcast sources\n");
		err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, NULL);
		if ((err != 0) && (err != -EALREADY)) {
			printk("Unable to start scan for broadcast sources: %d\n",
			       err);
			continue;
		}

		err = OSA_SemaphoreWait(sem_broadcaster_found, SEM_TIMEOUT);
		if (err != 0) {
			printk("sem_broadcaster_found timed out, resetting\n");
			continue;
		}
		printk("Broadcast source found, waiting for PA sync\n");

		err = bt_le_scan_stop();
		if (err != 0) {
			printk("bt_le_scan_stop failed with %d, resetting\n", err);
			continue;
		}                

		printk("Attempting to PA sync to the broadcaster with id 0x%06X\n",
		       broadcaster_broadcast_id);
		err = pa_sync_create();
		if (err != 0) {
			printk("Could not create Broadcast PA sync: %d, resetting\n", err);
			continue;
		}

		printk("Waiting for PA synced\n");
		err = OSA_SemaphoreWait(sem_pa_synced, SEM_TIMEOUT);
		if (err != 0) {
			printk("sem_pa_synced timed out, resetting\n");
			continue;
		}

		printk("Broadcast source PA synced, creating Broadcast Sink\n");
		err = bt_bap_broadcast_sink_create(pa_sync, broadcaster_broadcast_id,
						   &broadcast_sink);
		if (err != 0) {
			printk("Failed to create broadcast sink: %d\n", err);
			continue;
		}

		printk("Broadcast Sink created, waiting for BASE\n");
		err = OSA_SemaphoreWait(sem_base_received, SEM_TIMEOUT);
		if (err != 0) {
			printk("sem_base_received timed out, resetting\n");
			continue;
		}
		printk("BASE received, waiting for syncable\n");

		audio_codec_config();
		PRINTF("Audio codec configed, waiting for syncable\n");

		err = OSA_SemaphoreWait(sem_syncable, SEM_TIMEOUT);
		if (err != 0) {
			printk("sem_syncable timed out, resetting\n");
			continue;
		}

		/* sem_broadcast_code_received is also given if the
		 * broadcast is not encrypted
		 */
		printk("Waiting for broadcast code OK\n");
		err = OSA_SemaphoreWait(sem_broadcast_code_received, SEM_TIMEOUT);
		if (err != 0) {
			printk("sem_syncable timed out, resetting\n");
			continue;
		}

		printk("Syncing to broadcast\n");
		err = bt_bap_broadcast_sink_sync(broadcast_sink,
						 bis_index_bitfield & requested_bis_sync,
						 streams_p, sink_broadcast_code);
		if (err != 0) {
			printk("Unable to sync to broadcast source: %d\n", err);
			continue;
		}

		err = OSA_SemaphoreWait(sem_stream_started, SEM_TIMEOUT);
		if (err != 0) {
			PRINTF("sem_stream_started timed out, resetting\n");
			continue;
		}

		int res;
		do{
			if (bis_stream_play)
			{
				if(bis_stream_play_update)
				{
					bis_stream_play_update = false;
					
					/* Enable stream. */
					PRINTF("Syncing to broadcast\n");
					err = bt_bap_broadcast_sink_sync(broadcast_sink,
									bis_index_bitfield & requested_bis_sync,
									streams_p, sink_broadcast_code);
					if (err != 0) {
						PRINTF("Unable to sync to broadcast source: %d\n", err);
					}

					err = OSA_SemaphoreWait(sem_stream_started, SEM_TIMEOUT);
					if (err != 0) {
						PRINTF("sem_stream_started timed out, resetting\n");
					}
				}
				else
				{
					res = audio_stream_decode();
				}
			}
			else
			{
				if(bis_stream_play_update)
				{
					bis_stream_play_update = false;
					
					/* Disable stream. */
					err = bt_bap_broadcast_sink_stop(broadcast_sink);
					if (err != 0) {
						PRINTF("Unable to stop broadcast sink: %d\n", err);
					}

					err = OSA_SemaphoreWait(sem_stream_stopped, SEM_TIMEOUT);
					if (err != 0) {
						PRINTF("sem_stream_stopped timed out, resetting\n");
					}
					PRINTF("\nBroadcast sink stoped!\n");
				}
				OSA_TimeDelay(2);
			}
		} while (0 == res);
	}
}
