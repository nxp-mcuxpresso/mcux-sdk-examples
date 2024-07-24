/*
 * Copyright 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "le_audio_sync.h"

#include "le_audio_common.h"
#include "fsl_debug_console.h"

#include "audio_i2s.h"

#include "srCvtFrm.h"

extern void BOARD_SyncTimer_Init(void (*sync_timer_callback)(uint32_t sync_index, uint64_t bclk_count));
extern void BORAD_SyncTimer_Start(uint32_t sample_rate, uint32_t bits_per_sample, uint32_t sync_index_init);
extern void BORAD_SyncTimer_Stop(void);

#define AUDIO_SYNC_TEST_MODE_SINE                  1
#define AUDIO_SYNC_TEST_MODE_10MS_SINE_20MS_MUTE   2

static int audio_sync_test_mode = 0;

/* Here we just use one second wav for test. */
static int sin_index = 0;
static int16_t sin_wav[480 * 3] = { 0 };

#include <math.h>

void le_audio_sync_test_init(int sample_rate)
{
    float ts = 1.0 / sample_rate; /* Sample Rate. */
    float Fr = 500.0; /* Sin wav frequency. */
    float v;
    int n;

    if (audio_sync_test_mode == AUDIO_SYNC_TEST_MODE_10MS_SINE_20MS_MUTE)
    {
        /* Generate 10ms sine wav. */
        n = sample_rate / 100;
    }
    else
    {
        /* Generate 30ms sine wav. */
        n = sample_rate / 100 * 3;
    }

    for(int i = 0; i < n; i++)
    {
        v = 32767.0 * sin(2 * 3.141592 * Fr * ts * i);
        sin_wav[i] = (int16_t)v;
    }
    /* Do nothing. */
}

int le_audio_sync_test_mode(int mode)
{
    if((mode == 0) || \
       (mode == AUDIO_SYNC_TEST_MODE_SINE) || \
       (mode == AUDIO_SYNC_TEST_MODE_10MS_SINE_20MS_MUTE))
    {
        audio_sync_test_mode = mode;
        return 0;
    }

    return -1;
}

#define AUDIO_SYNC_STATE_STOP  0
#define AUDIO_SYNC_STATE_START 1
#define AUDIO_SYNC_STATE_KEEP  2

typedef struct _audio_sync_info {
    int sample_rate;
    int samples_per_frame;
    int bits_per_sample;
    uint32_t iso_interval_us;
    uint32_t sync_delay_us;
    uint32_t presentation_delay_us;
    int state;
    int pre_state;
    uint32_t time_stamp;
    uint32_t sync_signal_index_to_start;
    int mute_frame_samples;
    uint32_t mute_frame_duration_us;
    float system_delay_us;
    float sample_duration_us;
    float sync_offset_us;
    double i2s_output_samples;
    int resampler_added_samples;
    double resampler_internal_samples;
    double extra_samples_needed;
    double fix_kp;
    double fix_ki;
    double fix_i;
    double fix_output;
} audio_sync_info_t;
static audio_sync_info_t audio_sync_info;

typedef struct _pcm_packet {
	struct bt_iso_recv_info info;
	uint16_t len;
	int16_t buff[(480 + 128) * 2]; /* 480 (samples) * 2 (16bits) = 960 bytes. */
} pcm_packet_t;

#define PCM_PACKET_COUNT 10
typedef struct _audio_sync_pcm {
    pcm_packet_t pcm_packet[PCM_PACKET_COUNT];
    pcm_packet_t pcm_packet_mute;
    int in;
    int out;
    int count;
} audio_sync_pcm_t;
static audio_sync_pcm_t audio_sync_pcm; 

/* Resampler */
SrCvtFrmCfg_t upSrcCfg;
SrCvtFrm_t    upSrc;
static int16_t resampler_buff[(480 + 128)] = { 0 };
volatile int resampler_buff_len = 0;

static void Resampler_Init(int sample_rate, int samples_per_frame)
{
    upSrcCfg.fsIn = sample_rate;            //input sampling rate
    upSrcCfg.sfOut = sample_rate;           //output sampling rate
    upSrcCfg.phs = 32;                              //the filter phases
    upSrcCfg.fltTaps = 32;                          //the filter taps
    upSrcCfg.frmSizeIn = samples_per_frame;   //input frame size, 10ms interval
    upSrcCfg.frmSizeOut = samples_per_frame;  //output frame size

    initUpCvtFrm(&upSrc, &upSrcCfg, 0.0);
}

/* offset: 1.0 mean one sample. */
static void Resampler_SetOffset(double offset)
{
    srCvtSetFrcSmpl(&upSrc, offset);
}

static int Resampler(int16_t *in, int16_t *out)
{
    int samples_out = 0;
    /* Here we only use up convter.  */
    samples_out = upCvtFrm(&upSrc, in, out);

    return samples_out;
}

/* Herer we define fs_offset = ideal_samples_per_frame - real_samples_per_frame. */
static void Resampler_Update_FsOffset(double real_samples_per_frame, double ideal_samples_per_frame)
{
    double samples_offset = (ideal_samples_per_frame - real_samples_per_frame) / ideal_samples_per_frame;
    srCvtUpdateFreqOffset(&upSrc, samples_offset);
}

static double Resampler_GetFrc(void)
{
    return srCvtGetFrcSmpl(&upSrc);
}

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

static uint64_t Sync_Signal_Index_Timestamp(uint32_t sync_signal_index)
{
    return (uint64_t)sync_signal_index * (uint64_t)audio_sync_info.iso_interval_us + (uint64_t)audio_sync_info.sync_delay_us;
}

static void sync_timer_callback(uint32_t sync_index, uint64_t bclk_count)
{
    //PRINTF("i:%d, bclk:%llu, sts:%llu\n", sync_index, bclk_count, Sync_Signal_Index_Timestamp(sync_index));

    /* start process. */
    if(audio_sync_info.state == AUDIO_SYNC_STATE_START)
    {
        /* check start point. */
        if(audio_sync_info.sync_signal_index_to_start == sync_index)
        {
            (void)audio_i2s_start();
            /* set to keep state. */
            audio_sync_info.pre_state = audio_sync_info.state;
            audio_sync_info.state = AUDIO_SYNC_STATE_KEEP;
        }
    }
    /* keep process. */
    if((audio_sync_info.state == AUDIO_SYNC_STATE_KEEP) && (audio_sync_info.sync_signal_index_to_start != sync_index))
    {
        /* i2s output samples. */
        audio_sync_info.i2s_output_samples = (double)bclk_count / audio_sync_info.bits_per_sample / 2;
        /* actual audio output samples, reduce the mute frame. */
        audio_sync_info.i2s_output_samples -= audio_sync_info.mute_frame_samples;
        /* ideal samples output. */
        uint64_t ideal_played_us = Sync_Signal_Index_Timestamp(sync_index) - Sync_Signal_Index_Timestamp(audio_sync_info.sync_signal_index_to_start) - audio_sync_info.mute_frame_duration_us;
        double ideal_played_samples = (double)ideal_played_us / audio_sync_info.sample_duration_us;
        /* resampler output samples. */
        audio_sync_info.extra_samples_needed = audio_sync_info.i2s_output_samples - (ideal_played_samples + audio_sync_info.resampler_added_samples + audio_sync_info.resampler_internal_samples);
        /* PI loop. */
        audio_sync_info.fix_i += audio_sync_info.extra_samples_needed;
        audio_sync_info.fix_output = audio_sync_info.fix_kp * audio_sync_info.extra_samples_needed + audio_sync_info.fix_ki * audio_sync_info.fix_i;
        
        //PRINTF("%d, r:%.3f, t:%.3f, d:%8.3fus\n", sync_index, audio_sync_info.i2s_output_samples, ideal_played_samples, audio_sync_info.extra_samples_needed * audio_sync_info.sample_duration_us);
    }
}

void le_audio_sync_init(void)
{
    /* Init Sync Timer. */
    BOARD_SyncTimer_Init(sync_timer_callback);
}

void le_audio_sync_start(int sample_rate, int samples_per_frame)
{
    audio_sync_info.sample_rate           = sample_rate;
    audio_sync_info.samples_per_frame     = samples_per_frame;
    audio_sync_info.sample_duration_us    = 1000000.0 / (float)sample_rate;

    /* Start the sync timer. */
    BORAD_SyncTimer_Start(sample_rate, 16 * 2, 0);

    /* Init resampler. */
    Resampler_Init(sample_rate, samples_per_frame);
}

void le_audio_sync_set(uint32_t iso_interval_us, uint32_t sync_delay_us, uint32_t presentation_delay_us)
{
    audio_sync_info.iso_interval_us       = iso_interval_us;
    audio_sync_info.sync_delay_us         = sync_delay_us;
    // audio_sync_info.sample_rate           = sample_rate;
    // audio_sync_info.samples_per_frame     = samples_per_frame;
    audio_sync_info.presentation_delay_us = presentation_delay_us;
    audio_sync_info.bits_per_sample       = 16;
    audio_sync_info.state                 = AUDIO_SYNC_STATE_STOP;
    audio_sync_info.pre_state             = AUDIO_SYNC_STATE_STOP;
    // audio_sync_info.sample_duration_us    = 1000000.0 / (float)sample_rate;
    audio_sync_info.resampler_added_samples = 0;
    audio_sync_info.extra_samples_needed  = 0.0;

    audio_sync_info.fix_kp = 0.2;
    audio_sync_info.fix_ki = 0.002;
    audio_sync_info.fix_i  = 0.0;
    audio_sync_info.fix_output = 0.0;

    audio_sync_info.time_stamp = 0xffffffff; /* This is the invalid value. */
}

/* 
1, start sync.
2, keep sync.
3, stop sync.

start sync: when we at stop state, and receive one vaild frame. start handled in sync_time_callback.
keep sync: receive continue vaild or invaild frame, stop when got a frame interrupt.
stop sync: clean all state.
 */
void le_audio_sync_process(frame_packet_t *frame)
{
    //PRINTF("seq:%d,ts:%d,f:%d,len:%d\n", frame->info.seq_num, frame->info.ts, frame->info.flags, frame->len);

    /* if frame is invalid, just drop and stop. */
    if((frame->flags & BT_ISO_FLAGS_VALID) == 0)
    {
        if(audio_sync_info.pre_state != AUDIO_SYNC_STATE_STOP)
        {
            /* put to stop mode. */
            le_audio_sync_stop();
        }
        audio_sync_info.pre_state = audio_sync_info.state;
        audio_sync_info.state = AUDIO_SYNC_STATE_STOP;
        return;
    }

    if(audio_sync_info.state == AUDIO_SYNC_STATE_STOP)
    {
        /* if a frame is valid, just start. */
        if((frame->flags & BT_ISO_FLAGS_VALID) != 0)
        {
            /* calculate the start parameters. */
            /* The other parameters will affect the total system delay. */
            audio_sync_info.system_delay_us = 16 * audio_sync_info.sample_duration_us; /* Resampler have 16 samples delay. */
            audio_sync_info.system_delay_us += 0.0; /* LC3 delay, here we assume it is zero. */
            audio_sync_info.system_delay_us += System_Sync_offset(audio_sync_info.sample_rate);

            /* Timestamp + Presentation_Delay = Sync_Signal_Index * ISO_interval + BIG_CIG_Sync_Delay + Mute_Audio_Frame_Duration + Sync_Offset + system_delay_us.
                Mute_Audio_Frame_Duration: less than ISO_interval.
                Sync_Offset: less than one audio sample.
                system_delay: other modules delay in the system.
             */
            audio_sync_info.sync_signal_index_to_start = (frame->info.ts + audio_sync_info.presentation_delay_us - audio_sync_info.sync_delay_us - (int)audio_sync_info.system_delay_us) / audio_sync_info.iso_interval_us;
            audio_sync_info.mute_frame_duration_us = (frame->info.ts + audio_sync_info.presentation_delay_us) - (audio_sync_info.sync_signal_index_to_start * audio_sync_info.iso_interval_us + audio_sync_info.sync_delay_us + (int)audio_sync_info.system_delay_us);
            audio_sync_info.mute_frame_samples = (int)(audio_sync_info.mute_frame_duration_us / audio_sync_info.sample_duration_us);
            audio_sync_info.sync_offset_us = audio_sync_info.mute_frame_duration_us - (audio_sync_info.mute_frame_samples * audio_sync_info.sample_duration_us);

            /* here we will put the mute frame before the audio frame.
                This could make the audio frame play at right point, but the adjust unit is sample.
                So there might be a error less than one sample to the 'right point'. */
            audio_sync_pcm.pcm_packet_mute.len = audio_sync_info.mute_frame_samples * 2 * 2; /* 16bits 2ch */
            memset(audio_sync_pcm.pcm_packet_mute.buff, 0, audio_sync_pcm.pcm_packet_mute.len);
            if(audio_sync_info.mute_frame_samples > 0)
            {
                /* the mute frame buffer may less than mute frame needed,
                    so here we will handle the case by send multiple times mute frame buff. */
                int mute_frame_buff_len = sizeof(audio_sync_pcm.pcm_packet_mute.buff);
                int mute_frame_len = audio_sync_pcm.pcm_packet_mute.len;
                do{
                    if(mute_frame_len <= mute_frame_buff_len)
                    {
                        audio_i2s_write((uint8_t *)audio_sync_pcm.pcm_packet_mute.buff, mute_frame_len);
                        mute_frame_len = 0;
                    }
                    else
                    {
                        audio_i2s_write((uint8_t *)audio_sync_pcm.pcm_packet_mute.buff, mute_frame_buff_len);
                        mute_frame_len -= mute_frame_buff_len;
                    }
                } while(mute_frame_len > 0);
            }

            /* here we feed the sync offset error to resampler, this will move the wavform to left, then the sync offset will be removed. */
            Resampler_SetOffset(- (double)(audio_sync_info.sync_offset_us / audio_sync_info.sample_duration_us));

            /* init the pcm packet buff */
            audio_sync_pcm.in    = 0;
            audio_sync_pcm.out   = 0;
            audio_sync_pcm.count = 0;

            /* set state to start. */
            audio_sync_info.pre_state = audio_sync_info.state;
            audio_sync_info.state = AUDIO_SYNC_STATE_START;

            /* Save the first SDU's timestamp. */
            audio_sync_info.time_stamp = frame->info.ts;
        }
    }

    if((audio_sync_info.state == AUDIO_SYNC_STATE_KEEP) || (audio_sync_info.state == AUDIO_SYNC_STATE_START))
    {
        /* feedback sync error adjust parameter to resampler. */
        Resampler_Update_FsOffset(audio_sync_info.samples_per_frame + audio_sync_info.fix_output, audio_sync_info.samples_per_frame);
        /* replace the audio when enable sync test. */
        if(audio_sync_test_mode)
        {
            memcpy(frame->buff, &sin_wav[sin_index * audio_sync_info.samples_per_frame], audio_sync_info.samples_per_frame * 2);
            if(audio_sync_info.iso_interval_us == 7500) {
                sin_index = (sin_index + 1) % 4;
            }
            else if(audio_sync_info.iso_interval_us == 10000) {
                sin_index = (sin_index + 1) % 3;
            }
            else {
                while(1);
            }
        }
        /* resampler process. */
        int resampler_samples_out = Resampler((int16_t *)frame->buff, resampler_buff);
        audio_sync_info.resampler_added_samples += resampler_samples_out - audio_sync_info.samples_per_frame;
        audio_sync_info.resampler_internal_samples = Resampler_GetFrc();
        
        if(audio_sync_pcm.count < PCM_PACKET_COUNT)
        {
            audio_data_make_stereo(resampler_samples_out, 16, (uint8_t *)resampler_buff, (uint8_t *)resampler_buff, (uint8_t *)audio_sync_pcm.pcm_packet[audio_sync_pcm.in].buff);
            memcpy(&audio_sync_pcm.pcm_packet[audio_sync_pcm.in].info, &frame->info, sizeof(struct bt_iso_recv_info));
            audio_sync_pcm.pcm_packet[audio_sync_pcm.in].len = resampler_samples_out * 2 * 2;
            audio_sync_pcm.in = (audio_sync_pcm.in + 1) % PCM_PACKET_COUNT;
            audio_sync_pcm.count += 1;
        }
        else
        {
            PRINTF("audio_sync_pcm full!\n");
        }

        if(audio_sync_pcm.count > 0)
        {
            audio_i2s_write((uint8_t *)audio_sync_pcm.pcm_packet[audio_sync_pcm.out].buff, audio_sync_pcm.pcm_packet[audio_sync_pcm.out].len);
            audio_sync_pcm.out = (audio_sync_pcm.out + 1) % PCM_PACKET_COUNT;
            audio_sync_pcm.count -= 1;
        }
    }
}

void le_audio_sync_stop(void)
{
    BORAD_SyncTimer_Stop();

    audio_sync_info.time_stamp = 0xffffffff; /* This is the invalid value. */
}

void le_audio_sync_info_get(struct sync_info *info)
{
    info->iso_interval       = audio_sync_info.iso_interval_us;
    info->sync_delay         = audio_sync_info.sync_delay_us;
    info->presentation_delay = audio_sync_info.presentation_delay_us;
    
    info->time_stamp         = audio_sync_info.time_stamp;
}
