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
#include <bluetooth/audio/audio.h>
#include <bluetooth/audio/bap.h>
#include <bluetooth/audio/bap_lc3_preset.h>

#include "le_audio_common.h"
#include "le_audio_shell.h"
#include "broadcast_media_sender.h"

/* Note: this include should be remove once audio api could get bt_iso_chan. */
#include "audio/bap_endpoint.h"
#include "audio/bap_iso.h"

extern void BOARD_SyncSignal_Start(uint32_t init_offset);
extern void BOARD_SyncSignal_Stop(void);
extern uint32_t BOARD_SyncSignal_Count(void);

#ifndef printk
#define printk PRINTF
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

uint8_t wav_file_buff[MAX_AUDIO_CHANNEL_COUNT * MAX_AUDIO_BUFF_SIZE];

/* LC3 encoder variables. */
#include "lc3_codec.h"
lc3_encoder_t encoder[MAX_AUDIO_CHANNEL_COUNT];

static uint8_t audio_buff[MAX_AUDIO_CHANNEL_COUNT][MAX_AUDIO_BUFF_SIZE];
static uint8_t sdu_buff[MAX_AUDIO_CHANNEL_COUNT][LC3_FRAME_SIZE_MAX];

static lc3_codec_info_t lc3_codec_info;

/* When BROADCAST_ENQUEUE_COUNT > 1 we can enqueue enough buffers to ensure that
 * the controller is never idle
 */
#define BROADCAST_ENQUEUE_COUNT 16U
#define TOTAL_BUF_NEEDED (BROADCAST_ENQUEUE_COUNT * CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT)

BUILD_ASSERT_MSG(CONFIG_BT_ISO_TX_BUF_COUNT >= TOTAL_BUF_NEEDED,
	     "CONFIG_BT_ISO_TX_BUF_COUNT should be at least "
	     "BROADCAST_ENQUEUE_COUNT * CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT");

static struct bt_bap_lc3_preset lc3_preset;
static int new_rtn = -1;
static int new_pd = -1;
static int new_phy = -1;
static uint8_t iso_packing = BT_ISO_PACKING_SEQUENTIAL;
static uint8_t broadcast_code[BT_AUDIO_BROADCAST_CODE_SIZE] = { 0 };
static bool broadcast_code_set = false;

struct named_lc3_preset {
	const char *name;
	struct bt_bap_lc3_preset preset;
};

#define LOCATION BT_AUDIO_LOCATION_FRONT_LEFT | BT_AUDIO_LOCATION_FRONT_RIGHT
#define CONTEXT BT_AUDIO_CONTEXT_TYPE_MEDIA

static const struct named_lc3_preset lc3_broadcast_presets[] = {
	{"8_1_1", BT_BAP_LC3_BROADCAST_PRESET_8_1_1(LOCATION, CONTEXT)},
	{"8_2_1", BT_BAP_LC3_BROADCAST_PRESET_8_2_1(LOCATION, CONTEXT)},
	{"16_1_1", BT_BAP_LC3_BROADCAST_PRESET_16_1_1(LOCATION, CONTEXT)},
	{"16_2_1", BT_BAP_LC3_BROADCAST_PRESET_16_2_1(LOCATION, CONTEXT)},
	{"24_1_1", BT_BAP_LC3_BROADCAST_PRESET_24_1_1(LOCATION, CONTEXT)},
	{"24_2_1", BT_BAP_LC3_BROADCAST_PRESET_24_2_1(LOCATION, CONTEXT)},
	{"32_1_1", BT_BAP_LC3_BROADCAST_PRESET_32_1_1(LOCATION, CONTEXT)},
	{"32_2_1", BT_BAP_LC3_BROADCAST_PRESET_32_2_1(LOCATION, CONTEXT)},
	{"441_1_1", BT_BAP_LC3_BROADCAST_PRESET_441_1_1(LOCATION, CONTEXT)},
	{"441_2_1", BT_BAP_LC3_BROADCAST_PRESET_441_2_1(LOCATION, CONTEXT)},
	{"48_1_1", BT_BAP_LC3_BROADCAST_PRESET_48_1_1(LOCATION, CONTEXT)},
	{"48_2_1", BT_BAP_LC3_BROADCAST_PRESET_48_2_1(LOCATION, CONTEXT)},
	{"48_3_1", BT_BAP_LC3_BROADCAST_PRESET_48_3_1(LOCATION, CONTEXT)},
	{"48_4_1", BT_BAP_LC3_BROADCAST_PRESET_48_4_1(LOCATION, CONTEXT)},
	{"48_5_1", BT_BAP_LC3_BROADCAST_PRESET_48_5_1(LOCATION, CONTEXT)},
	{"48_6_1", BT_BAP_LC3_BROADCAST_PRESET_48_6_1(LOCATION, CONTEXT)},
	/* High-reliability presets */
	{"8_1_2", BT_BAP_LC3_BROADCAST_PRESET_8_1_2(LOCATION, CONTEXT)},
	{"8_2_2", BT_BAP_LC3_BROADCAST_PRESET_8_2_2(LOCATION, CONTEXT)},
	{"16_1_2", BT_BAP_LC3_BROADCAST_PRESET_16_1_2(LOCATION, CONTEXT)},
	{"16_2_2", BT_BAP_LC3_BROADCAST_PRESET_16_2_2(LOCATION, CONTEXT)},
	{"24_1_2", BT_BAP_LC3_BROADCAST_PRESET_24_1_2(LOCATION, CONTEXT)},
	{"24_2_2", BT_BAP_LC3_BROADCAST_PRESET_24_2_2(LOCATION, CONTEXT)},
	{"32_1_2", BT_BAP_LC3_BROADCAST_PRESET_32_1_2(LOCATION, CONTEXT)},
	{"32_2_2", BT_BAP_LC3_BROADCAST_PRESET_32_2_2(LOCATION, CONTEXT)},
	{"441_1_2", BT_BAP_LC3_BROADCAST_PRESET_441_1_2(LOCATION, CONTEXT)},
	{"441_2_2", BT_BAP_LC3_BROADCAST_PRESET_441_2_2(LOCATION, CONTEXT)},
	{"48_1_2", BT_BAP_LC3_BROADCAST_PRESET_48_1_2(LOCATION, CONTEXT)},
	{"48_2_2", BT_BAP_LC3_BROADCAST_PRESET_48_2_2(LOCATION, CONTEXT)},
	{"48_3_2", BT_BAP_LC3_BROADCAST_PRESET_48_3_2(LOCATION, CONTEXT)},
	{"48_4_2", BT_BAP_LC3_BROADCAST_PRESET_48_4_2(LOCATION, CONTEXT)},
	{"48_5_2", BT_BAP_LC3_BROADCAST_PRESET_48_5_2(LOCATION, CONTEXT)},
	{"48_6_2", BT_BAP_LC3_BROADCAST_PRESET_48_6_2(LOCATION, CONTEXT)},
};

static struct bt_audio_codec_cfg bis_codec_specific_config[CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT][1];

static struct broadcast_source_stream {
	struct bt_bap_stream stream;
	uint16_t seq_num;
	size_t sent_cnt;
} streams[CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT];
static struct bt_bap_broadcast_source *broadcast_source;

NET_BUF_POOL_FIXED_DEFINE(tx_pool,
			  TOTAL_BUF_NEEDED,
			  BT_ISO_SDU_BUF_SIZE(CONFIG_BT_ISO_TX_MTU), NULL);
static uint16_t seq_num;
static uint64_t tx_samples = 0;
static uint32_t tx_time_stamp_start;

static OSA_SEMAPHORE_HANDLE_DEFINE(sem_wav_opened);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_lc3_preset);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_started);
static OSA_SEMAPHORE_HANDLE_DEFINE(sem_stopped);

#define BROADCAST_SOURCE_LIFETIME  120U /* seconds */

static uint32_t get_big_sync_delay(void)
{
	struct bt_iso_info iso_info;
	uint32_t BIG_Sync_Delay_us;

	bt_iso_chan_get_info(&streams[0].stream.ep->iso->chan, &iso_info);

	BIG_Sync_Delay_us = iso_info.broadcaster.sync_delay;

	return BIG_Sync_Delay_us;
}

static uint32_t get_iso_interval(void)
{
	struct bt_iso_info iso_info;
	uint32_t ISO_Interval_us;

	bt_iso_chan_get_info(&streams[0].stream.ep->iso->chan, &iso_info);

	ISO_Interval_us = iso_info.iso_interval * 1250;

	return ISO_Interval_us;
}

static uint32_t get_sync_signal_timestamp(void)
{
	uint32_t time_stamp;

	time_stamp = BOARD_SyncSignal_Count() * get_iso_interval() + get_big_sync_delay();

	return time_stamp;
}

static void stream_started_cb(struct bt_bap_stream *stream)
{
	struct broadcast_source_stream *source_stream =
		CONTAINER_OF(stream, struct broadcast_source_stream, stream);

	if(stream == &streams[0].stream)
	{
		seq_num = 0;
		if(lc3_preset.qos.framing == BT_AUDIO_CODEC_QOS_FRAMING_FRAMED)
		{
			BOARD_SyncSignal_Start(0);
		}
	}

	source_stream->seq_num = 0U;
	source_stream->sent_cnt = 0U;
	OSA_SemaphorePost(sem_started);
}

static void stream_stopped_cb(struct bt_bap_stream *stream, uint8_t reason)
{
	if(lc3_preset.qos.framing == BT_AUDIO_CODEC_QOS_FRAMING_FRAMED)
	{
		BOARD_SyncSignal_Stop();
	}
	OSA_SemaphorePost(sem_stopped);
}

struct bt_bap_stream_ops stream_ops = {
	.started = stream_started_cb,
	.stopped = stream_stopped_cb,
};

static int setup_broadcast_source(struct bt_bap_broadcast_source **source)
{
	struct bt_bap_broadcast_source_stream_param
		stream_params[CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT];
	struct bt_bap_broadcast_source_subgroup_param
		subgroup_param[CONFIG_BT_BAP_BROADCAST_SRC_SUBGROUP_COUNT];
	struct bt_bap_broadcast_source_param create_param;
	const size_t streams_per_subgroup = ARRAY_SIZE(stream_params) / ARRAY_SIZE(subgroup_param);
	int err;

	(void)memset(streams, 0, sizeof(streams));

	for (size_t i = 0U; i < ARRAY_SIZE(subgroup_param); i++) {
		subgroup_param[i].params_count = streams_per_subgroup;
		subgroup_param[i].params = stream_params + i * streams_per_subgroup;
		subgroup_param[i].codec_cfg = &lc3_preset.codec_cfg;
	}

	for (size_t j = 0U; j < ARRAY_SIZE(stream_params); j++) {
		stream_params[j].stream = &streams[j].stream;

		stream_params[j].data = bis_codec_specific_config[j][0].data;
		stream_params[j].data_len = bis_codec_specific_config[j][0].data_len;

		bt_bap_stream_cb_register(stream_params[j].stream, &stream_ops);
	}

	create_param.params_count = ARRAY_SIZE(subgroup_param);
	create_param.params = subgroup_param;
	create_param.qos = &lc3_preset.qos;
	create_param.encryption = broadcast_code_set;
	create_param.packing = iso_packing;

	if(broadcast_code_set)
	{
		memcpy(create_param.broadcast_code, broadcast_code, BT_AUDIO_BROADCAST_CODE_SIZE);
	}

	printk("Creating broadcast source with %zu subgroups with %zu streams\n",
	       ARRAY_SIZE(subgroup_param),
	       ARRAY_SIZE(subgroup_param) * streams_per_subgroup);

	err = bt_bap_broadcast_source_create(&create_param, source);
	if (err != 0) {
		printk("Unable to create broadcast source: %d\n", err);
		return err;
	}

	return 0;
}

static int audio_stream_encode(void)
{
	int res;
	uint32_t sdu_time_stamp;

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

    struct net_buf *buf[CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT];
	
	for(int i = 0; i < CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT; i++)
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

	for(int i = 0; i < CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT; i++)
	{
		net_buf_add_mem(buf[i], sdu_buff[i], lc3_codec_info.octets_per_frame);
	}

	if(lc3_preset.qos.framing == BT_AUDIO_CODEC_QOS_FRAMING_FRAMED)
	{
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
	}
	else
	{
		sdu_time_stamp = BT_ISO_TIMESTAMP_NONE;
	}

	for(int i = 0; i < CONFIG_BT_BAP_BROADCAST_SRC_STREAM_COUNT; i++)
	{
		int ret = bt_bap_stream_send_ts(&streams[i].stream, buf[i], seq_num, sdu_time_stamp);
		if (ret < 0) {
			/* This will end broadcasting on this stream. */
			PRINTF("Unable to broadcast data on %p: %d\n", &streams[i], ret);
			net_buf_unref(buf[i]);
			return -1;
		}
	}

	seq_num++;

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
	for(int i = 0; i < ARRAY_SIZE(lc3_broadcast_presets); i++)
	{
		if(sample_rate != 0)
		{
			const struct bt_audio_codec_cfg *codec_cfg = &lc3_broadcast_presets[i].preset.codec_cfg;
			int codec_sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_cfg_freq)bt_audio_codec_cfg_get_freq(codec_cfg));
			if(codec_sample_rate != sample_rate)
			{
				continue;
			}
		}
		print_lc3_preset(lc3_broadcast_presets[i].name, &lc3_broadcast_presets[i].preset);
	}
}

int select_lc3_preset(char *preset_name)
{
	bool find = false;

	for(int i = 0; i < ARRAY_SIZE(lc3_broadcast_presets); i++)
	{
		const struct bt_audio_codec_cfg *codec_cfg = &lc3_broadcast_presets[i].preset.codec_cfg;
		
		if(0 == strcmp(lc3_broadcast_presets[i].name, preset_name))
		{
			int sample_rate = bt_audio_codec_cfg_freq_to_freq_hz((enum bt_audio_codec_cfg_freq)bt_audio_codec_cfg_get_freq(codec_cfg));
			if(sample_rate != wav_file.sample_rate)
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
	uint32_t sync_delay = get_big_sync_delay();

	PRINTF("sync info - iso_interval: %u, sync_delay: %u\n", iso_interval, sync_delay);
}

/* Here we don't require the user input all the bytes, and the left bytes will fill with 0. */
int config_broadcast_code(uint8_t *data, int len)
{
	memset(broadcast_code, 0, BT_AUDIO_BROADCAST_CODE_SIZE);
	if(len <= BT_AUDIO_BROADCAST_CODE_SIZE)
	{
		memcpy(broadcast_code, data, len);
		broadcast_code_set = true;

		PRINTF("broadcast_code: %s\n", bt_hex(broadcast_code, BT_AUDIO_BROADCAST_CODE_SIZE));
	}
	else
	{
		return -1;
	}

	return 0;
}

void config_audio_parameters(int sample_rate, int channels, int bits)
{
	/* set the LC3 encoder parameters. */
	lc3_codec_info.sample_rate = sample_rate;
	lc3_codec_info.frame_duration_us = bt_audio_codec_cfg_frame_dur_to_frame_dur_us((enum bt_audio_codec_cfg_frame_dur)bt_audio_codec_cfg_get_frame_dur(&lc3_preset.codec_cfg));
	lc3_codec_info.octets_per_frame = bt_audio_codec_cfg_get_octets_per_frame(&lc3_preset.codec_cfg);
	lc3_codec_info.blocks_per_sdu = 1;
	lc3_codec_info.chan_allocation = BT_AUDIO_LOCATION_FRONT_LEFT | BT_AUDIO_LOCATION_FRONT_RIGHT;

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

	/* config bis codec for each. */
	bt_audio_codec_cfg_unset_val(&lc3_preset.codec_cfg, BT_AUDIO_CODEC_CFG_CHAN_ALLOC);
	bt_audio_codec_cfg_set_chan_allocation(&bis_codec_specific_config[0][0], BT_AUDIO_LOCATION_FRONT_LEFT);
	bt_audio_codec_cfg_set_chan_allocation(&bis_codec_specific_config[1][0], BT_AUDIO_LOCATION_FRONT_RIGHT);
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

void broadcast_media_sender_task(void *param)
{
	struct bt_le_ext_adv *adv;
	int err;

	OSA_SemaphoreCreate(sem_wav_opened, 0);
	OSA_SemaphoreCreate(sem_lc3_preset, 0);
	OSA_SemaphoreCreate(sem_started, 0);
	OSA_SemaphoreCreate(sem_stopped, 0);

	/* shell init. */
	le_audio_shell_init();

	/* init bluetooth. */
	err = bt_enable(NULL);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}
	printk("Bluetooth initialized\n");

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
	/* Select LC3 preset */
	print_all_preset(wav_file.sample_rate);
	PRINTF("\nPlease select lc3 preset use \"lc3_preset <name>\" command.\n");
	OSA_SemaphoreWait(sem_lc3_preset, osaWaitForever_c);
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

	/* Config audio parameters. */
	config_audio_parameters(wav_file.sample_rate, wav_file.channels, wav_file.bits);

	while (true) {
		/* Broadcast Audio Streaming Endpoint advertising data */
		NET_BUF_SIMPLE_DEFINE(ad_buf,
				      BT_UUID_SIZE_16 + BT_AUDIO_BROADCAST_ID_SIZE);
		NET_BUF_SIMPLE_DEFINE(base_buf, 128);
		struct bt_data ext_ad;
		struct bt_data per_ad;
		uint32_t broadcast_id;

		/* Create a non-connectable non-scannable advertising set */
		err = bt_le_ext_adv_create(BT_LE_EXT_ADV_NCONN_NAME, NULL, &adv);
		if (err != 0) {
			printk("Unable to create extended advertising set: %d\n",
			       err);
			break;
		}

		/* Set periodic advertising parameters */
		err = bt_le_per_adv_set_param(adv, BT_LE_PER_ADV_DEFAULT);
		if (err) {
			printk("Failed to set periodic advertising parameters"
			" (err %d)\n", err);
			break;
		}

		printk("Creating broadcast source\n");
		err = setup_broadcast_source(&broadcast_source);
		if (err != 0) {
			printk("Unable to setup broadcast source: %d\n", err);
			break;
		}

		err = bt_bap_broadcast_source_get_id(broadcast_source, &broadcast_id);
		if (err != 0) {
			printk("Unable to get broadcast ID: %d\n", err);
			break;
		}

		/* Setup extended advertising data */
		net_buf_simple_add_le16(&ad_buf, BT_UUID_BROADCAST_AUDIO_VAL);
		net_buf_simple_add_le24(&ad_buf, broadcast_id);
		ext_ad.type = BT_DATA_SVC_DATA16;
		ext_ad.data_len = ad_buf.len;
		ext_ad.data = ad_buf.data;
		err = bt_le_ext_adv_set_data(adv, &ext_ad, 1, NULL, 0);
		if (err != 0) {
			printk("Failed to set extended advertising data: %d\n",
			       err);
			break;
		}

		/* Setup periodic advertising data */
		err = bt_bap_broadcast_source_get_base(broadcast_source, &base_buf);
		if (err != 0) {
			printk("Failed to get encoded BASE: %d\n", err);
			break;
		}

		per_ad.type = BT_DATA_SVC_DATA16;
		per_ad.data_len = base_buf.len;
		per_ad.data = base_buf.data;
		err = bt_le_per_adv_set_data(adv, &per_ad, 1);
		if (err != 0) {
			printk("Failed to set periodic advertising data: %d\n",
			       err);
			break;
		}

		/* Start extended advertising */
		err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
		if (err) {
			printk("Failed to start extended advertising: %d\n",
			       err);
			break;
		}

		/* Enable Periodic Advertising */
		err = bt_le_per_adv_start(adv);
		if (err) {
			printk("Failed to enable periodic advertising: %d\n",
			       err);
			break;
		}

		printk("Starting broadcast source\n");
		err = bt_bap_broadcast_source_start(broadcast_source, adv);
		if (err != 0) {
			printk("Unable to start broadcast source: %d\n", err);
			break;
		}

		/* Wait for all to be started */
		for (size_t i = 0U; i < ARRAY_SIZE(streams); i++) {
			OSA_SemaphoreWait(sem_started, osaWaitForever_c);
		}
		printk("Broadcast source started\n");

		int res = 0;
		do{
			if (bis_stream_play)
			{
				if(bis_stream_play_update)
				{
					bis_stream_play_update = false;
					
					/* Enable stream. */
					err = bt_bap_broadcast_source_start(broadcast_source, adv);
					if(err)
					{
						PRINTF("\nbroadcast source start fail %d\n", err);
					}

					/* Wait for all to be started */
					for (size_t i = 0U; i < ARRAY_SIZE(streams); i++) {
						OSA_SemaphoreWait(sem_started, osaWaitForever_c);
					}
					PRINTF("Broadcast source started\n");
				}
				res = audio_stream_encode();
			}
			else
			{
				if(bis_stream_play_update)
				{
					bis_stream_play_update = false;
					
					/* Disable stream. */
					err = bt_bap_broadcast_source_stop(broadcast_source);
					if(err)
					{
						PRINTF("\nbroadcast source stop fail %d\n", err);
					}

					/* Wait for all to be stopped */
					for (size_t i = 0U; i < ARRAY_SIZE(streams); i++) {
						OSA_SemaphoreWait(sem_stopped, osaWaitForever_c);
					}
					PRINTF("Broadcast source stopped\n");
				}
				OSA_TimeDelay(2);
			}
		} while (0 == res);

		PRINTF("Stopping broadcast source\n");
		err = bt_bap_broadcast_source_stop(broadcast_source);
		if (err != 0) {
			printk("Unable to stop broadcast source: %d\n", err);
			break;
		}

		/* Wait for all to be stopped */
		for (size_t i = 0U; i < ARRAY_SIZE(streams); i++) {
			OSA_SemaphoreWait(sem_stopped, osaWaitForever_c);
		}
		printk("Broadcast source stopped\n");

		printk("Deleting broadcast source\n");
		err = bt_bap_broadcast_source_delete(broadcast_source);
		if (err != 0) {
			printk("Unable to delete broadcast source: %d\n", err);
			break;
		}
		printk("Broadcast source deleted\n");
		broadcast_source = NULL;
		seq_num = 0;

		err = bt_le_per_adv_stop(adv);
		if (err) {
			printk("Failed to stop periodic advertising (err %d)\n",
			       err);
			break;
		}

		err = bt_le_ext_adv_stop(adv);
		if (err) {
			printk("Failed to stop extended advertising (err %d)\n",
			       err);
			break;
		}

		err = bt_le_ext_adv_delete(adv);
		if (err) {
			printk("Failed to delete extended advertising (err %d)\n",
			       err);
			break;
		}
	}

	close_wav_file();

	while(1);
}


