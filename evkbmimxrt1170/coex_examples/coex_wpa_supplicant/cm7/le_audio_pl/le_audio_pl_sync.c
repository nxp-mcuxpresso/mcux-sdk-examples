/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)

#include "BT_hci_api.h"
#include "le_audio_pl_sync.h"
#include "le_audio_pl_i2s.h"
#include "srCvtFrm.h"
#include "fsl_debug_console.h"
#include "audio_pl.h"
#include "leaudio_pl.h"

extern void le_audio_pl_ext_sync_timer_init(void (*sync_timer_callback)(UINT32 sync_index, uint64_t bclk_count));
extern void le_audio_pl_ext_sync_timer_start(UINT8 ep, UINT32 sink_cnter);
extern void le_audio_pl_ext_sync_timer_stop(void);

#define AUDIO_SYNC_STATE_STOP  		0
#define AUDIO_SYNC_STATE_START 		1U
#define AUDIO_SYNC_STATE_KEEP  		2U
#define AUDIO_SYNC_PCM_PACKET_CNT 	20U

typedef struct _audio_sync_info {
    INT32 sample_rate;
    INT32 samples_per_frame;
    INT32 bits_per_sample;
    UINT32 iso_interval_us;
    UINT32 sync_delay_us;
    UINT32 presentation_delay_us;
    UINT32 transport_latency;
    UINT32 cig_big_sync_delay;
    INT32 state;
    INT32 pre_state;
    UINT32 sync_signal_index_to_start;
    INT32 mute_frame_samples;
    UINT32 mute_frame_duration_us;
    float system_delay_us;
    float sample_duration_us;
    float sync_offset_us;
    double i2s_output_samples;
    INT32 resampler_added_samples;
    double resampler_internal_samples;
    double extra_samples_needed;
    double fix_kp;
    double fix_ki;
    double fix_i;
    double fix_output;
    UINT32 sink_sync_counter;
} audio_sync_info_t;

typedef struct _audio_sync_prop {
    INT32 sample_rate;
    INT32 samples_per_frame;
    INT32 bits_per_sample;
    INT32 no_of_channels;
    UINT32 fd_us;
    UINT8 is_big_established;
    UINT32 iso_interval_us;
    UINT32 sdu_interval_us;
    UINT32 sync_delay_us;
    UINT32 presentation_delay_us;
    UINT32 transport_m_s_latency;
    UINT32 transport_latency;
    UINT32 cig_big_sync_delay;
    UINT32 framed_unframed;
    UINT32 audio_location;
    UINT32 PTO;
    UINT32 BN;
    UINT32 IRC;
    UINT32 NSE;
    float system_delay_us;
    float sample_duration_us;
    float sync_offset_us;
} _audio_sync_prop_t;

static _audio_sync_prop_t audio_sync_prop_obj;
static audio_sync_info_t audio_sync_info;

typedef struct _pcm_packet {
	uint16_t len;
	int16_t buff[(480 + 128) * 2]; /* 480 (samples) * 2 (16bits) = 960 bytes. */
} pcm_packet_t;

typedef struct _audio_sync_pcm {
    pcm_packet_t pcm_packet[AUDIO_SYNC_PCM_PACKET_CNT];
    pcm_packet_t pcm_packet_mute;
    INT32 in;
    INT32 out;
    INT32 count;
} audio_sync_pcm_t;
static audio_sync_pcm_t audio_sync_pcm; 

/* Resampler */
SrCvtFrmCfg_t upSrcCfg;
SrCvtFrm_t    upSrc_ch1;
SrCvtFrm_t    upSrc_ch2;

extern volatile UINT32 SyncTimer_Trigger_Counter;
extern uint64_t SyncTimer_Bclk_Value;

static void Resampler_Init(INT32 sample_rate, INT32 samples_per_frame, UINT8 cc)
{
    upSrcCfg.fsIn = sample_rate;            //input sampling rate
    upSrcCfg.sfOut = sample_rate;           //output sampling rate
    upSrcCfg.phs = 32;                              //the filter phases
    upSrcCfg.fltTaps = 32;                          //the filter taps
    upSrcCfg.frmSizeIn = samples_per_frame;   //input frame size, 10ms interval
    upSrcCfg.frmSizeOut = samples_per_frame;  //output frame size
    upSrcCfg.cc = cc;

    if (cc == 1)
    {
        initUpCvtFrm(&upSrc_ch1, &upSrcCfg, 0.0);
    }
    else if (cc == 2)
    {
        initUpCvtFrm(&upSrc_ch1, &upSrcCfg, 0.0);
        initUpCvtFrm(&upSrc_ch2, &upSrcCfg, 0.0);
    }
    else
    {
    	//
    }
}

/* offset: 1.0 mean one sample. */
static void Resampler_SetOffset(double offset)
{
	if (upSrcCfg.cc == 1)
	{
	    srCvtSetFrcSmpl(&upSrc_ch1, offset);
	}
    else if (upSrcCfg.cc == 2)
    {
	    srCvtSetFrcSmpl(&upSrc_ch1, offset);
	    srCvtSetFrcSmpl(&upSrc_ch2, offset);
    }
    else
    {
    	//
    }
}

static INT32 Resampler(int16_t *in, int16_t *out, int8_t curr_ch)
{
    INT32 samples_out = 0;
    if (curr_ch == 1U)
    {
        /* Here we only use up convter.  */
        samples_out = upCvtFrm(&upSrc_ch1, in, out);
    }
    else if (curr_ch == 2U)
    {
        /* Here we only use up convter.  */
        samples_out = upCvtFrm(&upSrc_ch2, in, out);
    }
    else
    {
    	//
    }

    return samples_out;
}

/* Here we define fs_offset = ideal_samples_per_frame - real_samples_per_frame. */
static void Resampler_Update_FsOffset(double real_samples_per_frame, double ideal_samples_per_frame, UINT8 curr_ch)
{
    double samples_offset = (ideal_samples_per_frame - real_samples_per_frame) / ideal_samples_per_frame;

    //PRINTF("Resampler_Update_FsOffset() real samples %3.3f ",real_samples_per_frame);
    //PRINTF("Resampler_Update_FsOffset() ideal samples %3.3f ",ideal_samples_per_frame);
    //PRINTF("Resampler_Update_FsOffset() samples offset (%d) %3.3f\n",curr_ch,samples_offset);
	if (curr_ch == 1)
	{
	    srCvtUpdateFreqOffset(&upSrc_ch1, samples_offset);
	}
    else if (curr_ch == 2)
    {
        srCvtUpdateFreqOffset(&upSrc_ch2, samples_offset);
    }
    else
    {
    	//
    }
}

static double Resampler_GetFrc(UINT8 cc)
{
	double ret;
	if (cc == 1)
	{
		ret = srCvtGetFrcSmpl(&upSrc_ch1);
	}
    else if (cc == 2)
    {
		ret = srCvtGetFrcSmpl(&upSrc_ch2);
    }
    else
    {
    	ret = 0;
    }

    return ret;
}

/* This value is measured externaly. */
static float System_Sync_offset(INT32 sample_rate)
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

static uint64_t Sync_Signal_Index_Timestamp(UINT32 sync_signal_index)
{
    return (uint64_t)sync_signal_index * (uint64_t)audio_sync_info.iso_interval_us + (uint64_t)audio_sync_info.sync_delay_us;
}

static void sync_cb(UINT32 sync_index, uint64_t bclk_count)
{
   // PRINTF("i:%d, bclk:%llu, sts:%llu\n", sync_index, bclk_count, Sync_Signal_Index_Timestamp(sync_index));

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
        else if (audio_sync_info.sync_signal_index_to_start < sync_index)
        {
        	PRINTF ("i2s not started @ %d, starting now (counter %d)\n", audio_sync_info.sync_signal_index_to_start, sync_index);
            (void)audio_i2s_start();
            /* set to keep state. */
            audio_sync_info.pre_state = audio_sync_info.state;
            audio_sync_info.state = AUDIO_SYNC_STATE_KEEP;
        }
    }
#if defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)
    /* keep process. */
    if((audio_sync_info.state == AUDIO_SYNC_STATE_KEEP) && (audio_sync_info.sync_signal_index_to_start != sync_index))
    {
        /* i2s output samples. */
        audio_sync_info.i2s_output_samples = ((double)bclk_count / audio_sync_info.bits_per_sample );

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
        
       // PRINTF("m:%d r:%.3f, t:%.3f, d:%8.3fus, f:%8.6f\n", audio_sync_info.mute_frame_samples, audio_sync_info.i2s_output_samples, ideal_played_samples, audio_sync_info.extra_samples_needed * audio_sync_info.sample_duration_us, audio_sync_info.fix_i);
    }
#endif /*defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)*/
}

void le_audio_sync_init(void)
{
    /* Init Sync Timer. */
	le_audio_pl_ext_sync_timer_init(sync_cb);
}

void le_audio_sync_set_config(UINT32 iso_interval_us, UINT32 sync_delay_us, INT32 sample_rate, INT32 samples_per_frame, UINT32 presentation_delay_us, UINT32 sync_index_init, UINT32 bps, UINT8 cc)
{
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
    PRINTF ("le_audio_sync_set_config, iso %d, sync_delay %d, sr %d, spf %d pd %d location %d bps %d cc %d\n", iso_interval_us,sync_delay_us,sample_rate,samples_per_frame,presentation_delay_us,sync_index_init,bps, cc);
#endif
    audio_sync_info.iso_interval_us       = iso_interval_us;
    audio_sync_info.sync_delay_us         = sync_delay_us;
    audio_sync_info.sample_rate           = sample_rate;
    audio_sync_info.samples_per_frame     = samples_per_frame;
    audio_sync_info.presentation_delay_us = presentation_delay_us;
    audio_sync_info.bits_per_sample       = bps;
    audio_sync_info.state                 = AUDIO_SYNC_STATE_STOP;
    audio_sync_info.pre_state             = AUDIO_SYNC_STATE_STOP;
    audio_sync_info.sample_duration_us    = 1000000.0 / (float)sample_rate;
    audio_sync_info.resampler_added_samples = 0;
    audio_sync_info.resampler_internal_samples = 0;
    audio_sync_info.extra_samples_needed  = 0.0;
    
    audio_sync_info.fix_kp = 0.2;
    audio_sync_info.fix_ki = 0.002;
    audio_sync_info.fix_i  = 0.0;
    audio_sync_info.fix_output = 0.0;

    /* Init resampler. */
    Resampler_Init(sample_rate, samples_per_frame, cc);
}

void le_audio_sync_start(UINT8 ep, UINT32 sink_cnter)
{
	le_audio_pl_ext_sync_timer_start(ep, sink_cnter);
}


/* 
1, start sync.
2, keep sync.
3, stop sync.

start sync: when we at stop state, and receive one vaild frame. start handled in sync_time_callback.
keep sync: receive continue vaild or invaild frame, stop when got a frame interrupt.
stop sync: clean all state.
 */
UINT16 le_audio_asrc_process(UINT8 bfi, HCI_ISO_HEADER *iso_hdr, INT16* in, UINT16 length, INT16* out, UINT8 curr_ch)
{
	INT32 resampler_samples_out=0;
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
    //PRINTF("[%d, %u] %.3f, %8.3fus, fi:%8.6f, fo:%8.6f\n",iso_hdr->seqnum, iso_hdr->ts, audio_sync_info.i2s_output_samples, audio_sync_info.extra_samples_needed, audio_sync_info.fix_i,audio_sync_info.fix_output);
#endif

    if(audio_sync_info.state == AUDIO_SYNC_STATE_STOP)
    {
        /* if a frame is valid, just start. */
        if (bfi == LE_AUDIO_SDU_FLAG_VALID)
        {
            //PRINTF("hw-codec to start @ %d iso-event\n",(iso_hdr->ts - audio_sync_info.sync_delay_us) / audio_sync_info.iso_interval_us);
        	le_audio_sync_stop();
        	le_audio_sync_start(AUDIO_EP_SINK, ((iso_hdr->ts - audio_sync_info.sync_delay_us) / audio_sync_info.iso_interval_us));

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
            audio_sync_info.sync_signal_index_to_start = (iso_hdr->ts + audio_sync_info.presentation_delay_us - audio_sync_info.sync_delay_us - (INT32)audio_sync_info.system_delay_us) / audio_sync_info.iso_interval_us;
            audio_sync_info.mute_frame_duration_us = (iso_hdr->ts + audio_sync_info.presentation_delay_us) - (audio_sync_info.sync_signal_index_to_start * audio_sync_info.iso_interval_us + audio_sync_info.sync_delay_us + (INT32)audio_sync_info.system_delay_us);
            audio_sync_info.mute_frame_samples = (INT32)(audio_sync_info.mute_frame_duration_us / audio_sync_info.sample_duration_us);
            audio_sync_info.sync_offset_us = audio_sync_info.mute_frame_duration_us - (audio_sync_info.mute_frame_samples * audio_sync_info.sample_duration_us);
            PRINTF("%d %d\n",(iso_hdr->ts - audio_sync_info.sync_delay_us) / audio_sync_info.iso_interval_us, audio_sync_info.sync_signal_index_to_start);
            //PRINTF("%3.3f %d %d\n", audio_sync_info.system_delay_us, audio_sync_info.mute_frame_duration_us, audio_sync_info.sync_delay_us);

#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
            PRINTF("total system delay in usec %3.3f\n",audio_sync_info.system_delay_us);
            PRINTF("sync_signal_index_to_start %d\n",audio_sync_info.sync_signal_index_to_start);
            PRINTF("mute_frame_duration_usec %d\n",audio_sync_info.mute_frame_duration_us);
            PRINTF("mute_frame_samples %d\n",audio_sync_info.mute_frame_samples);
            PRINTF("sync_offset_us %2.2f\n",audio_sync_info.sync_offset_us);
            PRINTF("%d %d\n",(iso_hdr->ts - audio_sync_info.sync_delay_us) / audio_sync_info.iso_interval_us, audio_sync_info.sync_signal_index_to_start);
#endif /*defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)*/

            /* here we will put the mute frame before the audio frame.
                This could make the audio frame play at right point, but the adjust unit is sample.
                So there might be a error less than one sample to the 'right point'. */
            audio_sync_pcm.pcm_packet_mute.len = audio_sync_info.mute_frame_samples * 2; /*todo: 16bits 1ch */
            memset(audio_sync_pcm.pcm_packet_mute.buff, 0, audio_sync_pcm.pcm_packet_mute.len);
            if(audio_sync_info.mute_frame_samples > 0)
            {
                /* the mute frame buffer may less than mute frame needed,
                    so here we will handle the case by send multiple times mute frame buff. */
                INT32 mute_frame_buff_len = sizeof(audio_sync_pcm.pcm_packet_mute.buff);
                INT32 mute_frame_len = audio_sync_pcm.pcm_packet_mute.len * le_audio_pl_get_config_values(LE_AUDIO_NO_OF_CHANNELS);
                do
                {
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

            //PRINTF("audio_sync_info.sample_duration_us %2.3f\n",audio_sync_info.sample_duration_us);
#if defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)
            /* here we feed the sync offset error to resampler, this will move the wavform to left, then the sync offset will be removed. */
            Resampler_SetOffset(- (double)(audio_sync_info.sync_offset_us / audio_sync_info.sample_duration_us));
#endif /*defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)*/
            /* init the pcm packet buff */
            audio_sync_pcm.in    = 0;
            audio_sync_pcm.out   = 0;
            audio_sync_pcm.count = 0;

            /* set state to start. */
            audio_sync_info.pre_state = audio_sync_info.state;
            audio_sync_info.state = AUDIO_SYNC_STATE_START;
        }
    }

    if((audio_sync_info.state == AUDIO_SYNC_STATE_KEEP) || (audio_sync_info.state == AUDIO_SYNC_STATE_START))
    {
#if defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)
    	static double fix_op = 0;
    	if (curr_ch == 1U)
    	{
    		fix_op = audio_sync_info.fix_output;
    	}

        Resampler_Update_FsOffset(audio_sync_info.samples_per_frame + fix_op, audio_sync_info.sample_rate / 100, curr_ch);
        resampler_samples_out = Resampler(in, out, curr_ch);
        audio_sync_info.resampler_added_samples += resampler_samples_out - audio_sync_info.samples_per_frame;
        audio_sync_info.resampler_internal_samples = Resampler_GetFrc(curr_ch);
#else
        for (UINT16 sample = 0U; sample < length; sample++)
		{
			out[sample] = (INT16)(in[sample]);
		}
        resampler_samples_out = length;
#endif /* defined(LE_AUDIO_SINK_WITH_ASRC) && (LE_AUDIO_SINK_WITH_ASRC > 0)*/
    }

    return resampler_samples_out;
}

void le_audio_snk_i2s_enqueue(
								/* IN */  UCHAR* data,
								/* IN */  UINT16  datalen
							 )
{
	if ((data != NULL) && (datalen > 0))
	{
	    if(audio_sync_pcm.count < AUDIO_SYNC_PCM_PACKET_CNT)
	    {
	        audio_sync_pcm.pcm_packet[audio_sync_pcm.in].len = datalen;
	        memcpy((uint8_t *)audio_sync_pcm.pcm_packet[audio_sync_pcm.in].buff, (uint8_t *)data, audio_sync_pcm.pcm_packet[audio_sync_pcm.in].len);
	        audio_sync_pcm.in = (audio_sync_pcm.in + 1) % AUDIO_SYNC_PCM_PACKET_CNT;
	        audio_sync_pcm.count += 1;
	    }
	    else
	    {
	        PRINTF("%");
	    }

	    if(audio_sync_pcm.count > 0)
	    {
	    	if (audio_i2s_write((uint8_t *)audio_sync_pcm.pcm_packet[audio_sync_pcm.out].buff, audio_sync_pcm.pcm_packet[audio_sync_pcm.out].len) == 0)
	    	{
		        audio_sync_pcm.out = (audio_sync_pcm.out + 1) % AUDIO_SYNC_PCM_PACKET_CNT;
		        audio_sync_pcm.count -= 1;
			}
	    }
	}
}

void le_audio_sync_stop(void)
{
	le_audio_pl_ext_sync_timer_stop();
}

void le_audio_pl_set_config_values (le_audio_pl_config_value_t prop , UINT32 value)
{
    switch (prop)
    {
        case LE_AUDIO_IS_BIG_ESTABLISHED:
            audio_sync_prop_obj.is_big_established = value;
            break;
        case LE_AUDIO_ISO_INTERVAL:
            audio_sync_prop_obj.iso_interval_us = value;
            break;
        case LE_AUDIO_SDU_INTERVAL:
            audio_sync_prop_obj.sdu_interval_us = value;
            break;
        case LE_AUDIO_TRANSPORT_LATENCY:
            audio_sync_prop_obj.transport_latency = value;
            break;
        case LE_AUDIO_BIG_CIG_DELAY:
            audio_sync_prop_obj.cig_big_sync_delay = value;
            break;
        case LE_AUDIO_PRESENTATION_DELAY:
            audio_sync_prop_obj.presentation_delay_us = value;
            break;
        case LE_AUDIO_TOTAL_SYNC_DELAY:
            audio_sync_prop_obj.sync_delay_us = value;
            break;
        case LE_AUDIO_SYNC_OFFSET:
            audio_sync_prop_obj.sync_offset_us = value;
            break;
        case LE_AUDIO_SYSTEM_DELAY:
            audio_sync_prop_obj.system_delay_us = value;
            break;
        case LE_AUDIO_SAMPLE_RATE:
            audio_sync_prop_obj.sample_rate = value;
            break;
        case LE_AUDIO_SAMPLE_PER_FRAME:
            audio_sync_prop_obj.samples_per_frame = value;
            break;
        case LE_AUDIO_FRAMED_UNFRAMED:
            audio_sync_prop_obj.framed_unframed = value;
            break;
        case LE_AUDIO_BITS_PER_SAMPLE:
            audio_sync_prop_obj.bits_per_sample = value;
            break;
        case LE_AUDIO_NO_OF_CHANNELS:
			audio_sync_prop_obj.no_of_channels = value;
			break;
        case LE_AUDIO_LOCATION:
            audio_sync_prop_obj.audio_location = value;
            break;
        case LE_AUDIO_BC_BN:
            audio_sync_prop_obj.BN = value;
            break;
        case LE_AUDIO_BC_PTO:
            audio_sync_prop_obj.PTO = value;
            break;
        case LE_AUDIO_BC_IRC:
            audio_sync_prop_obj.IRC = value;
            break;
        case LE_AUDIO_BC_NSE:
            audio_sync_prop_obj.NSE = value;
            break;
        case LE_AUDIO_INVALID_PROP:
            break;
        default:
            break;
    }
}

UINT32 le_audio_pl_get_config_values (le_audio_pl_config_value_t prop)
{
    UINT32 value = 0xFFFFU;

    switch (prop)
    {
        case LE_AUDIO_IS_BIG_ESTABLISHED:
            value = audio_sync_prop_obj.is_big_established;
            break;
        case LE_AUDIO_ISO_INTERVAL:
            value = audio_sync_prop_obj.iso_interval_us;
            break;
        case LE_AUDIO_SDU_INTERVAL:
            value = audio_sync_prop_obj.sdu_interval_us;
            break;
        case LE_AUDIO_TRANSPORT_LATENCY:
            value = audio_sync_prop_obj.transport_latency;
            break;
        case LE_AUDIO_BIG_CIG_DELAY:
            /*1: BIG, 2: CIG*/
            if (audio_sync_prop_obj.is_big_established == AUDIO_SYNC_BIG_ESTABLISHED)
            {
                UINT32 iso_interval_us = audio_sync_prop_obj.iso_interval_us;
                UINT32 sdu_interval_us = audio_sync_prop_obj.sdu_interval_us;
                UINT32 transport_latency = audio_sync_prop_obj.transport_latency;
                UINT32 PTO = audio_sync_prop_obj.PTO;
                UINT32 NSE = audio_sync_prop_obj.NSE;
                UINT32 BN = audio_sync_prop_obj.BN;
                UINT32 IRC = audio_sync_prop_obj.IRC;

                //value = audio_sync_prop_obj.cig_big_sync_delay;

                /*framing = 00 is unframmed , and 01 is frammed*/
                if(audio_sync_prop_obj.framed_unframed == 0) /*UnFramed*/
                {
                    value = transport_latency - ((PTO * (NSE / BN - IRC) + 1) * iso_interval_us - sdu_interval_us);
                }
                else
                {
                    value = transport_latency - (PTO * (NSE / BN - IRC) * iso_interval_us + iso_interval_us + sdu_interval_us);
                }
#if defined(LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING) && (LE_AUDIO_ENABLE_PRINTS_FOR_STREAMING == 1)
                PRINTF ("delay %d, t %d pto %d nse %d bn %d irc %d iso %d sdu %d\n",value,transport_latency,PTO, NSE,BN,IRC, iso_interval_us,sdu_interval_us);
#endif
            }
            else if (audio_sync_prop_obj.is_big_established == AUDIO_SYNC_CIG_ESTABLISHED)
            {
                value = audio_sync_prop_obj.cig_big_sync_delay;
            }
            else
            {
                //
            }
            break;
        case LE_AUDIO_PRESENTATION_DELAY:
            value = audio_sync_prop_obj.presentation_delay_us;
            break;
        case LE_AUDIO_TOTAL_SYNC_DELAY:
            value = audio_sync_prop_obj.sync_delay_us;
            break;
        case LE_AUDIO_SYNC_OFFSET:
            value = audio_sync_prop_obj.sync_offset_us;
            break;
        case LE_AUDIO_SYSTEM_DELAY:
            value = audio_sync_prop_obj.system_delay_us;
            break;
        case LE_AUDIO_SAMPLE_RATE:
            value = audio_sync_prop_obj.sample_rate;
            break;
        case LE_AUDIO_SAMPLE_PER_FRAME:
            value = audio_sync_prop_obj.samples_per_frame;
            break;
        case LE_AUDIO_FRAMED_UNFRAMED:
            value = audio_sync_prop_obj.framed_unframed;
            break;
        case LE_AUDIO_BITS_PER_SAMPLE:
            value = audio_sync_prop_obj.bits_per_sample;
            break;
        case LE_AUDIO_NO_OF_CHANNELS:
			value = audio_sync_prop_obj.no_of_channels;
			break;
        case LE_AUDIO_LOCATION:
            value = audio_sync_prop_obj.audio_location;
            break;
        case LE_AUDIO_BC_BN:
            value = audio_sync_prop_obj.BN;
            break;
        case LE_AUDIO_BC_PTO:
            value = audio_sync_prop_obj.PTO;
            break;
        case LE_AUDIO_BC_IRC:
            value = audio_sync_prop_obj.IRC;
            break;
        case LE_AUDIO_BC_NSE:
            value = audio_sync_prop_obj.NSE;
            break;
        case LE_AUDIO_INVALID_PROP:
            value = 0xFFFFU;
            break;
    }

    //PRINTF ("le_audio_pl_get_config_values, prop %d , value %d\n", prop, value);

    return value;
}

#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
