/*
 * Copyright 2023-2024 NXP
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
#include <bluetooth/audio/csip.h>
#include <sys/byteorder.h>

#include "le_audio_common.h"
#include "le_audio_shell.h"
#include "le_audio_service.h"

/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#include "audio/bap_iso.h"

extern void BOARD_SyncSignal_Start(uint32_t init_offset);
extern void BOARD_SyncSignal_Stop(void);
extern uint32_t BOARD_SyncSignal_Count(void);

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

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
#include "sys/ring_buffer.h"
RING_BUF_DECLARE(a2dp_to_ums_audio_buf, 10 * 480 * 4);
int a2dp_sink_audio_frame_size = 0;
extern void app_audio_streamer_task_signal(void);
static int ring_buff_read_out_bytes = 0;

int a2dp_audio_sample_rate = 0;
int a2dp_audio_channels = 0;
int a2dp_audio_bits = 0;
#endif

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

static bt_addr_le_t devices_list[16];
static bt_addr_le_t target_devices[2];
static int devices_list_count = 0;
static uint8_t set_sirk[BT_CSIP_SET_SIRK_SIZE];
static bool set_sirk_set = false;

static struct bt_bap_unicast_client_cb unicast_client_cbs;
static struct bt_conn *default_conn[CONFIG_BT_MAX_CONN];
static int default_conn_index;
static struct bt_bap_unicast_group *unicast_group;
static struct audio_sink {
	struct bt_bap_ep *ep;
	uint16_t seq_num;
} sinks[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
NET_BUF_POOL_FIXED_DEFINE(tx_pool1, TOTAL_BUF_NEEDED/2,
			  CONFIG_BT_ISO_TX_MTU + BT_ISO_CHAN_SEND_RESERVE, NULL);
NET_BUF_POOL_FIXED_DEFINE(tx_pool2, TOTAL_BUF_NEEDED/2,
			  CONFIG_BT_ISO_TX_MTU + BT_ISO_CHAN_SEND_RESERVE, NULL);
static struct net_buf_pool *tx_pool[] = { &tx_pool1, &tx_pool2 };

static struct bt_bap_stream streams[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
static enum bt_audio_location audio_receiver_loc[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];
static bool stream_disconnected[CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT];

/* Select a codec configuration to apply that is mandatory to support by both client and server.
 * Allows this sample application to work without logic to parse the codec capabilities of the
 * server and selection of an appropriate codec configuration.
 */
static struct bt_bap_lc3_preset lc3_preset;
static int new_rtn = -1;
static int new_pd = -1;
static int new_phy = -1;
static uint8_t iso_packing = BT_ISO_PACKING_SEQUENTIAL;

struct named_lc3_preset {
	const char *name;
	struct bt_bap_lc3_preset preset;
};

#define LOCATION BT_AUDIO_LOCATION_FRONT_LEFT | BT_AUDIO_LOCATION_FRONT_RIGHT
#define CONTEXT BT_AUDIO_CONTEXT_TYPE_MEDIA

static const struct named_lc3_preset lc3_unicast_presets[] = {
	{"8_1_1", BT_BAP_LC3_UNICAST_PRESET_8_1_1(LOCATION, CONTEXT)},
	{"8_2_1", BT_BAP_LC3_UNICAST_PRESET_8_2_1(LOCATION, CONTEXT)},
	{"16_1_1", BT_BAP_LC3_UNICAST_PRESET_16_1_1(LOCATION, CONTEXT)},
	{"16_2_1", BT_BAP_LC3_UNICAST_PRESET_16_2_1(LOCATION, CONTEXT)},
	{"24_1_1", BT_BAP_LC3_UNICAST_PRESET_24_1_1(LOCATION, CONTEXT)},
	{"24_2_1", BT_BAP_LC3_UNICAST_PRESET_24_2_1(LOCATION, CONTEXT)},
	{"32_1_1", BT_BAP_LC3_UNICAST_PRESET_32_1_1(LOCATION, CONTEXT)},
	{"32_2_1", BT_BAP_LC3_UNICAST_PRESET_32_2_1(LOCATION, CONTEXT)},
	{"441_1_1", BT_BAP_LC3_UNICAST_PRESET_441_1_1(LOCATION, CONTEXT)},
	{"441_2_1", BT_BAP_LC3_UNICAST_PRESET_441_2_1(LOCATION, CONTEXT)},
	{"48_1_1", BT_BAP_LC3_UNICAST_PRESET_48_1_1(LOCATION, CONTEXT)},
	{"48_2_1", BT_BAP_LC3_UNICAST_PRESET_48_2_1(LOCATION, CONTEXT)},
	{"48_3_1", BT_BAP_LC3_UNICAST_PRESET_48_3_1(LOCATION, CONTEXT)},
	{"48_4_1", BT_BAP_LC3_UNICAST_PRESET_48_4_1(LOCATION, CONTEXT)},
	{"48_5_1", BT_BAP_LC3_UNICAST_PRESET_48_5_1(LOCATION, CONTEXT)},
	{"48_6_1", BT_BAP_LC3_UNICAST_PRESET_48_6_1(LOCATION, CONTEXT)},
	/* High-reliability presets */
	{"8_1_2", BT_BAP_LC3_UNICAST_PRESET_8_1_2(LOCATION, CONTEXT)},
	{"8_2_2", BT_BAP_LC3_UNICAST_PRESET_8_2_2(LOCATION, CONTEXT)},
	{"16_1_2", BT_BAP_LC3_UNICAST_PRESET_16_1_2(LOCATION, CONTEXT)},
	{"16_2_2", BT_BAP_LC3_UNICAST_PRESET_16_2_2(LOCATION, CONTEXT)},
	{"24_1_2", BT_BAP_LC3_UNICAST_PRESET_24_1_2(LOCATION, CONTEXT)},
	{"24_2_2", BT_BAP_LC3_UNICAST_PRESET_24_2_2(LOCATION, CONTEXT)},
	{"32_1_2", BT_BAP_LC3_UNICAST_PRESET_32_1_2(LOCATION, CONTEXT)},
	{"32_2_2", BT_BAP_LC3_UNICAST_PRESET_32_2_2(LOCATION, CONTEXT)},
	{"441_1_2", BT_BAP_LC3_UNICAST_PRESET_441_1_2(LOCATION, CONTEXT)},
	{"441_2_2", BT_BAP_LC3_UNICAST_PRESET_441_2_2(LOCATION, CONTEXT)},
	{"48_1_2", BT_BAP_LC3_UNICAST_PRESET_48_1_2(LOCATION, CONTEXT)},
	{"48_2_2", BT_BAP_LC3_UNICAST_PRESET_48_2_2(LOCATION, CONTEXT)},
	{"48_3_2", BT_BAP_LC3_UNICAST_PRESET_48_3_2(LOCATION, CONTEXT)},
	{"48_4_2", BT_BAP_LC3_UNICAST_PRESET_48_4_2(LOCATION, CONTEXT)},
	{"48_5_2", BT_BAP_LC3_UNICAST_PRESET_48_5_2(LOCATION, CONTEXT)},
	{"48_6_2", BT_BAP_LC3_UNICAST_PRESET_48_6_2(LOCATION, CONTEXT)},
};

/* set conn interval to 10ms and timeout to 100ms. */
#define CONNECTION_PARAMETERS BT_LE_CONN_PARAM(8, 8, 0, 10)

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_wav_opened);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_lc3_preset);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_device_selected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_connected);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_csip_discovered);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_member_discovered);
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

static uint32_t seq_num = 0;
static uint64_t tx_samples = 0;
static uint32_t tx_time_stamp_start;

static uint32_t get_cig_sync_delay(void)
{
	struct bt_iso_info iso_info;

	bt_iso_chan_get_info(&streams[0].ep->iso->chan, &iso_info);

	return iso_info.unicast.cig_sync_delay;
}

static uint32_t get_iso_interval(void)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(&streams[0].ep->iso->chan, &iso_info);

	ISO_Interval_us = iso_info.iso_interval * 1250;

	return ISO_Interval_us;
}

static uint32_t get_sync_signal_timestamp(void)
{
	uint32_t time_stamp;

	time_stamp = BOARD_SyncSignal_Count() * get_iso_interval() + get_cig_sync_delay();

	return time_stamp;
}

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

static int audio_stream_encode(bool mute)
{
	int res;
	uint32_t sdu_time_stamp;
	int bits;

	/* read one frame samples. */
#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
	if(!mute)
	{
		int ret = ring_buf_get(&a2dp_to_ums_audio_buf, wav_file_buff, lc3_codec_info.samples_per_frame * 4);

		if (ret != lc3_codec_info.samples_per_frame * 4)
		{
			int clear_bytes = lc3_codec_info.samples_per_frame * 4 - ret;
			memset(wav_file_buff + ret, 0, clear_bytes);
		}

		ring_buff_read_out_bytes += ret;

		if(a2dp_sink_audio_frame_size > 0)
		{
			for (; ring_buff_read_out_bytes >= a2dp_sink_audio_frame_size; ring_buff_read_out_bytes -= a2dp_sink_audio_frame_size)
			{
				app_audio_streamer_task_signal();
			}
		}
	}

	bits = 16;
#else
	if(!mute)
	{
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
	}

	bits = wav_file.bits;
#endif

	if(mute)
	{
		memset(wav_file_buff, 0, lc3_codec_info.samples_per_frame * 2 * (bits / 8));
	}

	/* copy data from pcm form to channel format */
	if(2 == lc3_codec_info.channels)
	{
		(void)audio_data_stereo_split(lc3_codec_info.samples_per_frame, bits, wav_file_buff, audio_buff[0], audio_buff[1]);
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
			buf[i] = net_buf_alloc(tx_pool[i], 10);
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

	if(seq_num == 0)
	{
		tx_samples = 0;
		tx_time_stamp_start = get_sync_signal_timestamp() + get_iso_interval();
		sdu_time_stamp = tx_time_stamp_start;
	}
	else
	{
		tx_samples += lc3_codec_info.samples_per_frame;
		sdu_time_stamp = (uint32_t)((double)tx_time_stamp_start + (double)tx_samples * 1000000.0 / (double)lc3_codec_info.sample_rate);
	}

	for(int i = 0; i < CONFIG_BT_BAP_UNICAST_CLIENT_ASE_SNK_COUNT; i++)
	{
		struct bt_bap_stream *stream = NULL;
		enum bt_audio_location loc = (i == 0) ? BT_AUDIO_LOCATION_FRONT_LEFT : BT_AUDIO_LOCATION_FRONT_RIGHT;

		/* find the left/right audio channel stream before send. */
		for (int index = 0; index < LE_CONN_COUNT; index++)
		{
			if(audio_receiver_loc[index] & loc)
			{
				stream = &streams[index];
				break;
			}
		}

		if(stream_disconnected[i])
		{
			net_buf_unref(buf[i]);
		}
		else
		{
			int ret = bt_bap_stream_send_ts(stream, buf[i],
								get_and_incr_seq_num(stream),
								sdu_time_stamp);
			if (ret < 0) {
				/* This will end send stream. */
				PRINTF("Unable to send stream on %p: %d\n", stream, ret);
				net_buf_unref(buf[i]);
				if(ret == -ENOTCONN)
				{
					stream_disconnected[i] = true;
				}
			}
		}

		/* return err if both stream disconnected. */
		if(stream_disconnected[0] && stream_disconnected[1])
		{
			return -1;
		}
	}

	seq_num += 1;

	return 0;
}

static void wav_file_list(const char *path)
{
	DIR dir;
	FRESULT res;
	FILINFO info;
	char fname[FF_LFN_BUF + 1];
	int i = 0;

	PRINTF("\nwav file list:\n");

	res = f_findfirst(&dir, &info, path, "*.wav");
	while((res == FR_OK) && (info.fname[0]))
	{
		i++;
		memset(fname, 0, sizeof(fname));
		strcat(fname, path);
		strcat(fname, info.fname);
		PRINTF("%d, %s\n", i, fname);
		res = f_findnext(&dir, &info);
	}

	PRINTF("wav file list complete!\n");
}

int open_wav_file(char *path)
{
	int res;

	res = wav_file_open(&wav_file, path);
	if(res)
	{
		PRINTF("\nwav_file_open fail!\n");
		return -1;
	}

	PRINTF("wav file info:\n");
	PRINTF("\tsample_rate: %d\n", 	wav_file.sample_rate);
	PRINTF("\tchannels: %d\n", 		wav_file.channels);
	PRINTF("\tbits: %d\n", 			wav_file.bits);
	PRINTF("\tsize: %d\n", 			wav_file.size);
	PRINTF("\tsamples: %d\n", 		wav_file.samples);

	switch (wav_file.sample_rate)
	{
		case 8000: break;
		case 16000: break;
		case 24000: break;
		case 32000: break;
		case 44100: break;
		case 48000: break;
		default:
			PRINTF("\nwav file sample rate %d not support!\n", wav_file.sample_rate);
			return -1;
	}

	if(wav_file.channels != 2)
	{
		PRINTF("\nwav file not 2 channels!\n");
		return -1;
	}

	if((wav_file.bits != 16) && (wav_file.bits != 24) && (wav_file.bits != 32))
	{
		PRINTF("\nwav file %d bits not support!\n", wav_file.bits);
		return -1;
	}

	(void)OSA_SemaphorePost(sem_wav_opened);

	return 0;
}

static void close_wav_file(void)
{
	int res = wav_file_close(&wav_file);
	if(res)
	{
		PRINTF("\nwav_file_close fail!\n");
	}
}

static void print_lc3_preset(const char *name, const struct bt_bap_lc3_preset *preset)
{
	const struct bt_audio_codec_cfg *codec_cfg = &preset->codec_cfg;
	const struct bt_audio_codec_qos *qos = &preset->qos;

	PRINTF("%s:\n", name);

	PRINTF("\tcodec_cfg - sample_rate: %d, duration: %d, len: %d\n",
			bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_cfg_freq)bt_audio_codec_cfg_get_freq(codec_cfg)),
			bt_audio_codec_cfg_frame_dur_to_frame_dur_us((enum bt_audio_codec_cfg_frame_dur)bt_audio_codec_cfg_get_frame_dur(codec_cfg)),
			bt_audio_codec_cfg_get_octets_per_frame(codec_cfg)
		);

	PRINTF("\tqos - interval: %d, framing: %d, phy: %d, sdu: %d, rtn: %d, pd: %d\n",
			qos->interval,
			qos->framing,
			qos->phy,
			qos->sdu,
			qos->rtn,
			qos->pd
		);
}

void print_all_preset(int sample_rate)
{
	PRINTF("\nlc3 preset list:\n");
	for(int i = 0; i < ARRAY_SIZE(lc3_unicast_presets); i++)
	{
		if(sample_rate != 0)
		{
			const struct bt_audio_codec_cfg *codec_cfg = &lc3_unicast_presets[i].preset.codec_cfg;
			int codec_sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_cfg_freq)bt_audio_codec_cfg_get_freq(codec_cfg));
			if(codec_sample_rate != sample_rate)
			{
				continue;
			}
		}
		print_lc3_preset(lc3_unicast_presets[i].name, &lc3_unicast_presets[i].preset);
	}
}

int select_lc3_preset(char *preset_name)
{
	bool find = false;

	for(int i = 0; i < ARRAY_SIZE(lc3_unicast_presets); i++)
	{
		const struct bt_audio_codec_cfg *codec_cfg = &lc3_unicast_presets[i].preset.codec_cfg;
		
		if(0 == strcmp(lc3_unicast_presets[i].name, preset_name))
		{
			int sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_cfg_freq)bt_audio_codec_cfg_get_freq(codec_cfg));
#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
			if(sample_rate != a2dp_audio_sample_rate)
#else
			if(sample_rate != wav_file.sample_rate)
#endif
			{
				PRINTF("preset sample rate %d not align with wav %d\n", sample_rate, wav_file.sample_rate);
				return -1;
			}
			find = true;
			memcpy(&lc3_preset, codec_cfg, sizeof(lc3_preset));
		}
	}

	if(!find)
	{
		return -1;
	}

	print_lc3_preset(preset_name, &lc3_preset);

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
	if((a2dp_audio_sample_rate == 44100) || (a2dp_audio_sample_rate == 48000))
	{
		lc3_preset.qos.rtn = 1;
	}

	print_lc3_preset("a2dp_bridge_limit_rtn=1", &lc3_preset);
#endif

	(void)OSA_SemaphorePost(sem_lc3_preset);

	return 0;
}

int modify_rtn(int rtn)
{
	if(!IN_RANGE(rtn, 0, 255))
	{
		return -1;
	}

	new_rtn = rtn;

	return 0;
}

int modify_pd(int pd)
{
	if(!IN_RANGE(pd, 10000, 70000))
	{
		return -1;
	}

	new_pd = pd;

	return 0;
}

int modify_phy(int phy)
{
	/* 1: 1M, 2: 2M, 4: Coded. */
	if((phy == BT_AUDIO_CODEC_QOS_1M) || (phy == BT_AUDIO_CODEC_QOS_2M) || (phy == BT_AUDIO_CODEC_QOS_CODED))
	{
		new_phy = phy;
		return 0;
	}

	return -1;
}

int modify_packing(int packing)
{
	/* 0: sequentially, 1: interleaved. */
	if((packing == BT_ISO_PACKING_SEQUENTIAL) || (packing == BT_ISO_PACKING_INTERLEAVED))
	{
		iso_packing = packing;
		return 0;
	}

	return -1;
}

void print_sync_info(void)
{
	uint32_t iso_interval = get_iso_interval();
	uint32_t sync_delay = get_cig_sync_delay();

	PRINTF("sync info - iso_interval: %u, sync_delay: %u\n", iso_interval, sync_delay);
}

void config_audio_parameters(int sample_rate, int channels, int bits)
{
	/* set the LC3 encoder parameters. */
	lc3_codec_info.sample_rate = sample_rate;
	lc3_codec_info.frame_duration_us = bt_audio_codec_cfg_frame_dur_to_frame_dur_us((enum bt_audio_codec_cfg_frame_dur)bt_audio_codec_cfg_get_frame_dur(&lc3_preset.codec_cfg));
	lc3_codec_info.octets_per_frame = bt_audio_codec_cfg_get_octets_per_frame(&lc3_preset.codec_cfg);
	lc3_codec_info.blocks_per_sdu = 1;
	lc3_codec_info.chan_allocation = 0; /* not used. */

	lc3_codec_info.channels = channels;
	if(lc3_codec_info.sample_rate == 44100)
	{
		if(lc3_codec_info.frame_duration_us == 7500)
		{
			lc3_codec_info.samples_per_frame = 360;
		}
		else
		{
			lc3_codec_info.samples_per_frame = 480;
		}
	}
	else
	{
		lc3_codec_info.samples_per_frame = lc3_codec_info.sample_rate * (lc3_codec_info.frame_duration_us / 100) / 10000;
	}
	lc3_codec_info.bytes_per_channel_frame = lc3_codec_info.samples_per_frame * bits / 8;

	/* LC3 Encoder Init. */
	for (int i = 0; i < lc3_codec_info.channels; i++)
	{
		int lc3_res = lc3_encoder_init(&encoder[i], sample_rate, lc3_codec_info.frame_duration_us, lc3_codec_info.octets_per_frame, bits);
		if(lc3_res)
		{
			PRINTF("\nlc3_encoder_init fail!\n");
		}
	}
	PRINTF("LC3 encoder setup done!\n");

	/* set codec data. */

	/* set codec qos. */
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

	if(stream == &streams[0])
	{
		seq_num = 0;
		BOARD_SyncSignal_Start(0);
	}

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
	BOARD_SyncSignal_Stop();
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
	struct bt_conn_info info;

	bt_conn_get_info(conn, &info);

	if(info.type == BT_CONN_TYPE_LE)
	{

		(void)bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

		if (err != 0) {
			PRINTF("LE Failed to connect to %s (%u)\n", addr, err);

			bt_conn_unref(default_conn[default_conn_index]);
			default_conn[default_conn_index] = NULL;

			return;
		}

		if (conn != default_conn[default_conn_index]) {
			return;
		}

		PRINTF("LE Connected: %s\n", addr);

		OSA_SemaphorePost(sem_connected);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	char addr[BT_ADDR_LE_STR_LEN];
	int i;

	struct bt_conn_info info;

	bt_conn_get_info(conn, &info);

    	if(info.type == BT_CONN_TYPE_LE)
	{
		PRINTF("LE Disconnected: %s (reason 0x%02x)\n", addr, reason);

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

		PRINTF("LE Disconnected: %s (reason 0x%02x)\n", addr, reason);

		bt_conn_unref(default_conn[i]);
		default_conn[i] = NULL;

		OSA_SemaphorePost(sem_disconnected);
	}
}

static void security_changed_cb(struct bt_conn *conn, bt_security_t level,
				enum bt_security_err err)
{
	struct bt_conn_info info;

	bt_conn_get_info(conn, &info);

    	if(info.type == BT_CONN_TYPE_LE)
	{
		if (err == 0) {
			OSA_SemaphorePost(sem_security_updated);
		} else {
			PRINTF("LE Failed to set security level: %u", err);
		}
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

	for(int index = 0; index < CONFIG_BT_MAX_CONN; index++)
	{
		if(conn == default_conn[index])
		{
			audio_receiver_loc[index] = loc;
			break;
		}
	}
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

	(void)OSA_SemaphoreCreate(sem_wav_opened, 0);
	(void)OSA_SemaphoreCreate(sem_lc3_preset, 0);
	(void)OSA_SemaphoreCreate(sem_device_selected, 0);
	(void)OSA_SemaphoreCreate(sem_connected, 0);
	(void)OSA_SemaphoreCreate(sem_csip_discovered, 0);
	(void)OSA_SemaphoreCreate(sem_member_discovered, 0);
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

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
#else
	err = bt_enable(NULL);
	if (err != 0) {
		PRINTF("Bluetooth enable failed (err %d)\n", err);
		return err;
	}
#endif

	bt_conn_cb_register(&conn_callbacks);

	for (size_t i = 0; i < ARRAY_SIZE(streams); i++) {
		streams[i].ops = &stream_ops;
	}

	bt_gatt_cb_register(&gatt_callbacks);

	return 0;
}

static bool get_advertise_data(struct net_buf_simple *ad, uint8_t type, struct bt_data *ad_data)
{
	bool found = false;
	struct net_buf_simple_state state;

	net_buf_simple_save(ad, &state);

	while(ad->len > 1)
	{
		uint8_t len = net_buf_simple_pull_u8(ad);
		if (len == 0U) {
			/* Early termination */
			break;
		}

		if (len > ad->len) {
			/* malformed advertising data */
			break;
		}

		uint8_t ad_type = net_buf_simple_pull_u8(ad);
		if(ad_type == type)
		{
			if(ad_data)
			{
				ad_data->type = type;
				ad_data->data_len = len - 1;
				ad_data->data = ad->data;
			}
			found = true;
			break;
		}

		net_buf_simple_pull(ad, len - 1);
	}

	net_buf_simple_restore(ad, &state);

	return found;
}

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *ad)
{
	struct bt_data ad_data;
	char addr_str[BT_ADDR_LE_STR_LEN];
	char device_name[32];

	/* Resolvable Set Identifier */
	if(!get_advertise_data(ad, BT_DATA_CSIS_RSI, NULL))
	{
		return;
	}

	if(rssi < -70)
	{
		return;
	}

	/* skip the addr in the list. */	
	for(int i = 0; i < devices_list_count; i++)
	{
		if(0 == memcmp(addr, &devices_list[i], sizeof(bt_addr_le_t)))
		{
			return;
		}
	}

	/* save the addr */
	if (devices_list_count >= ARRAY_SIZE(devices_list))
	{
		return;
	}
	memcpy(&devices_list[devices_list_count], addr, sizeof(bt_addr_le_t));
	devices_list_count += 1;

	/* print the device info */
	(void)bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	memset(device_name, 0, sizeof(device_name));
	if(get_advertise_data(ad, BT_DATA_NAME_COMPLETE, &ad_data))
	{
		memcpy(device_name, ad_data.data, (ad_data.data_len < sizeof(device_name)) ? ad_data.data_len : (sizeof(device_name) - 1));
	}
	PRINTF("[%d]: %s, rssi %d, %s\n", devices_list_count - 1, addr_str, rssi, device_name);
}

int device_scan(void)
{
	int err;

	/* This demo doesn't require active scan */
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, scan_cb);
	if(err)
	{
		if(err == -EALREADY) {
			PRINTF("Scan already started!\n");
		}
		else
		{
			PRINTF("Scan failed to start (err %d)\n", err);
			return -1;
		}
	}

	return 0;
}

int device_select(int index)
{
	if(!IN_RANGE(index, 0, devices_list_count))
	{
		PRINTF("index should in range [0, %d]\n", devices_list_count - 1);
		return -1;
	}

	memcpy(&target_devices[0], &devices_list[index], sizeof(bt_addr_le_t));

	int err = bt_le_scan_stop();
	if(err)
	{
		PRINTF("scan stop fail with err %d\n", err);
	}

	(void)OSA_SemaphorePost(sem_device_selected);

	return 0;
}

static void member_scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type, struct net_buf_simple *ad)
{
	struct bt_data ad_data;
	char addr_str[BT_ADDR_LE_STR_LEN];
	char device_name[32];

	if(rssi < -70)
	{
		return;
	}

	/* Resolvable Set Identifier */
	if(!get_advertise_data(ad, BT_DATA_CSIS_RSI, &ad_data))
	{
		return;
	}

	if(!bt_csip_set_coordinator_is_set_member(set_sirk, &ad_data))
	{
		return;
	}

	/* save the addr */
	memcpy(&target_devices[1], addr, sizeof(bt_addr_le_t));

	/* print the device info */
	(void)bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
	memset(device_name, 0, sizeof(device_name));
	if(get_advertise_data(ad, BT_DATA_NAME_COMPLETE, &ad_data))
	{
		memcpy(device_name, ad_data.data, (ad_data.data_len < sizeof(device_name)) ? ad_data.data_len : (sizeof(device_name) - 1));
	}
	PRINTF("member: %s, rssi %d, %s\n", addr_str, rssi, device_name);

	int err = bt_le_scan_stop();
	if(err)
	{
		PRINTF("scan stop fail with err %d\n", err);
	}

	(void)OSA_SemaphorePost(sem_member_discovered);
}

static int member_scan(void)
{
	int err;

	/* This demo doesn't require active scan */
	err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, member_scan_cb);
	if(err)
	{
		if(err == -EALREADY) {
			PRINTF("Scan already started!\n");
		}
		else
		{
			PRINTF("Scan failed to start (err %d)\n", err);
			return -1;
		}
	}

	return 0;
}

static int device_connect(int index)
{
	int err;

	if(index == 0)
	{
		PRINTF("Connect first device\n");
		default_conn_index = 0;
	}
	if(index == 1)
	{
		PRINTF("Connect second device\n");
		default_conn_index = 1;
	}

	err = bt_conn_le_create(&target_devices[default_conn_index], BT_CONN_LE_CREATE_CONN,
		CONNECTION_PARAMETERS,
		&default_conn[default_conn_index]);
	if (err != 0) {
		PRINTF("Create conn to failed (%u)\n", err);
		return err;
	}

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
	chan_allocation_value = (enum bt_audio_location)(audio_receiver_loc[i] & (BT_AUDIO_LOCATION_FRONT_LEFT | BT_AUDIO_LOCATION_FRONT_RIGHT));
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
	param.packing = iso_packing;

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
	(void)OSA_SemaphoreDestroy(sem_wav_opened);
	(void)OSA_SemaphoreDestroy(sem_lc3_preset);
	(void)OSA_SemaphoreDestroy(sem_device_selected);
	(void)OSA_SemaphoreDestroy(sem_connected);
	(void)OSA_SemaphoreDestroy(sem_csip_discovered);
	(void)OSA_SemaphoreDestroy(sem_member_discovered);
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

	(void)OSA_SemaphoreCreate(sem_wav_opened, 0);
	(void)OSA_SemaphoreCreate(sem_lc3_preset, 0);
	(void)OSA_SemaphoreCreate(sem_device_selected, 0);
	(void)OSA_SemaphoreCreate(sem_connected, 0);
	(void)OSA_SemaphoreCreate(sem_csip_discovered, 0);
	(void)OSA_SemaphoreCreate(sem_member_discovered, 0);
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

static void csip_set_coordinator_discover_cb(
	struct bt_conn *conn,
	const struct bt_csip_set_coordinator_set_member *member,
	int err, size_t set_count)
{
	if(err) {
		PRINTF("CSIP conn %p csip discover err %d\n", conn, err);
	}
	else {
		PRINTF("CSIP conn %p discovered set count %d\n", conn, set_count);
		for(int i = 0; i < set_count; i++)
		{
			const struct bt_csip_set_coordinator_set_info *info = &member->insts[i].info;
			PRINTF("set %d/%d info:\n", i+1, set_count);
			PRINTF("\tsirk: ");
			for(int j = 0; j < BT_CSIP_SET_SIRK_SIZE; j++)
			{
				PRINTF("%02x ", info->set_sirk[j]);
			}
			PRINTF("\n");

			PRINTF("\tset_size: %d\n", info->set_size);

			PRINTF("\trank: %d\n", info->rank);

			PRINTF("\tlockable: %d\n", info->lockable);

			if(!set_sirk_set)
			{
				set_sirk_set = true;
				memcpy(&set_sirk, &info->set_sirk[i], BT_CSIP_SET_SIRK_SIZE);
			}
		}
	}
	(void)OSA_SemaphorePost(sem_csip_discovered);
}

static struct bt_csip_set_coordinator_cb csip_cb = {
	.discover = csip_set_coordinator_discover_cb
};

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

	/* CSIP */
	err = bt_csip_set_coordinator_register_cb(&csip_cb);
	if (err != 0) {
		PRINTF("Failed to register csip callbacks: %d", err);
		while(1);
	}

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
#else
	/* Host msd init. */
#if (defined(BT_BLE_PLATFORM_INIT_ESCAPE) && (BT_BLE_PLATFORM_INIT_ESCAPE > 0))
	USB_HostMsdFatfsInit();
	PRINTF("FatFs initialized\n");
#endif
	/* List wav file in the root dir. */
	wav_file_list("1:/");
	/* Open wav file */
	PRINTF("\nPlease open the wav file you want use \"wav_open <path>\" command.\n");
	OSA_SemaphoreWait(sem_wav_opened, osaWaitForever_c);
#endif
	/* Select LC3 preset */
#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
	print_all_preset(a2dp_audio_sample_rate);
#else
	print_all_preset(wav_file.sample_rate);
#endif
	PRINTF("\nPlease select lc3 preset use \"lc3_preset <name>\" command.\n");
	OSA_SemaphoreWait(sem_lc3_preset, osaWaitForever_c);
	/* Config audio parameters. */
#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
	config_audio_parameters(a2dp_audio_sample_rate, a2dp_audio_channels, a2dp_audio_bits);
#else
	config_audio_parameters(wav_file.sample_rate, wav_file.channels, wav_file.bits);
#endif
	/* overlay rtn & pd if set */
	if(new_rtn >= 0)
	{
		lc3_preset.qos.rtn = new_rtn;
	}
	if(new_pd >= 0)
	{
		lc3_preset.qos.pd = new_pd;
	}
	if(new_phy > 0)
	{
		lc3_preset.qos.phy = new_phy;
	}

	if((new_rtn >= 0) || (new_pd >= 0) || (new_phy > 0))
	{
		print_lc3_preset("new_preset", &lc3_preset);
	}

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

		PRINTF("Please scan and connect the devices you want!\n");
		err = OSA_SemaphoreWait(sem_device_selected, osaWaitForever_c);
		if (err != 0) {
			PRINTF("failed to take sem_device_selected (err %d)\n", err);
			break;
		}
		PRINTF("device selected!\n");

		for (int i = 0; i < LE_CONN_COUNT; i++)
		{
			PRINTF("Connecting\n");
			err = device_connect(i);
			if (err != 0) {
				break;
			}
			PRINTF("Connected\n");

			PRINTF("CSIP discover\n");
			err = bt_csip_set_coordinator_discover(default_conn[i]);
			if (err != 0) {
				PRINTF("CSIP set coordinator discover err %d\n", err);
				break;
			}
			OSA_SemaphoreWait(sem_csip_discovered, osaWaitForever_c);
			PRINTF("CSIP discovered\n");

			if(i == 0)
			{
				PRINTF("Scan another member\n");

				member_scan();

				OSA_SemaphoreWait(sem_member_discovered, osaWaitForever_c);
				PRINTF("Member discovered\n");
			}
		}

		for (int i = 0; i < LE_CONN_COUNT; i++)
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

		int res = 0;
		do{
			if (cis_stream_play)
			{
				if(cis_stream_play_update)
				{
					cis_stream_play_update = false;
					for(int i = 0; i < LE_CONN_COUNT; i++)
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
				res = audio_stream_encode(false);
			}
			else
			{
				if(cis_stream_play_update)
				{
					cis_stream_play_update = false;

					/* send 2 mute frames to sink to avoid LC3 PLC noise. */
					(void)audio_stream_encode(true);
					(void)audio_stream_encode(true);

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

#if defined(CONFIG_BT_A2DP_SINK) && (CONFIG_BT_A2DP_SINK > 0)
#else
	close_wav_file();
#endif

	while(1);
}
