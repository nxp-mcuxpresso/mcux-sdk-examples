#ifdef LC3_TEST
/**
 *  \file  lc3_test.c
 *
 *  \brief This file implements a sample LC3 encoder and decoder test application.
 */

/*
 *  Copyright (C) 2021. Mindtree Ltd.
 *  All rights reserved.
 */

/* ------------------------------------------- Header File Inclusion */
#include "stdio.h"
#include "stdlib.h"
#include <string.h>

#if defined (LC3_DSP) && (LC3_DSP == 1)
#include "LC3_api.h"
#elif defined (LC3_DSP) && (LC3_DSP == 0)
#include "LC3_ndsp_api.h"
#elif defined (LC3_DSP) && (LC3_DSP == 2)
#include "LC3_odsp_api.h"
#else
#include "LC3_api.h"
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/
#include "lc3_fops.h"


/* -------------------------------------------------- Global Definitions */
/**
 * Notes:
 * Testing on ARM :
 *  a. Disable "LC3_TEST_CMD_LINE" for ARM/DSP standalone testing/profiling.
 *  b. Change the relative path of input_filename, output_filename for encoder and decoder.
 */
/* #define LC3_TEST_CMD_LINE */
#define DEBUG_LOG
/* #define DEBUG_LOG_MEMORY */

/**
 * Flag to enable LC3 profiling
 *
 * For the LC3 profiling, the PCM input data for LC3 Encoder is taken
 * from a Global Array (instead of from a file).
 */
//#define HAVE_LC3_PROFILING


#ifdef HAVE_LC3_PROFILING
/**
 * Sampling Rate:
 */
#define SAMPLING_RATE               48000

/**
 * Number of Channels
 */
#define CHANNEL_COUNT               1

/**
 * Octets per codec frame
 */
#define OCTETS_PER_FRAME            100

/**
 * Frame Duration(in deci-milliseonds)
 * Set to 100 for 10 msec
 * set to 75  for 7.5 msec
 */
#define FRAME_DUR                   100

#if (SAMPLING_RATE == 48000)
#include "lc3_encoder_input_48000.h"
#else /* (SAMPLING_RATE == 48000) */
//#error "Create and include file "lc3_encoder_input_samplerate.h""
#include "lc3_encoder_input_44100.h"
#endif /* SAMPLING_RATE == 48000 */

/**
 * Max. no. of PCM frames to encode when HAVE_LC3_PROFILING is enabled.
 *
 * Note:
 * Set this value according to no. of frames present in 'lc3_enc_input_data[]'
 * in lc3_encoder_input_<SAMPLING_RATE>.h
 */
#define MAX_PCM_FRAME_CNT           50
#endif /* HAVE_LC3_PROFILING */

/**
 * Octets per codec frame
 */
#define OCTETS_PER_CODEC_FRAME      100

/**
 * Frame Duration(in deci-milliseonds)
 * Set to 100 for 10 msec
 * set to 75  for 7.5 msec
 */
#define FRAME_DURATION              100

/**
 * Below Macro supports bits-per-sample value from i/p WAV file to generate equal bit-rate output audio.
 * Disable this macro to use default bits-per-sample value 16.
 */
#define APPL_ENABLE_EQUAL_IO_BITRATE 1U

#define min(a,b) (((a) < (b)) ? (a) : (b))

/** LC3 Application Input - Data Structure */
typedef struct _LC3_APP_INPUTS
{
    INT8    input_filename[256];     /**< Name of input file */
    INT8    output_filename[256];    /**< Name of output file */
    UINT8   mode;                    /**< 1:Encoder, 2:Decoder, 0 or Any:Encoder+Decoder */
    UINT8   frame_ms;                /**< 100:10ms, 75:7.5ms [in deci-milliseonds] */
    UINT8   bps_out;                 /**< Bits Per Sample: 16 or 24 or 32  */
    UINT32  nbytes;                  /**< Number of Bytes */
}LC3_APP_INPUTS;


/* ------------------------------------------- External Global Variables */


/* ------------------------------------------- Exported Global Variables */


/* ------------------------------------------- Static Global Variables */
#if defined (LC3_DSP) && (LC3_DSP == 1)
static LC3_ENCODER_CNTX  g_lc3enc;
static LC3_DECODER_CNTX  g_lc3dec;
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
static LC3_ENCODER_CONFIG lc3_enc_config;
static LC3_DECODER_CONFIG lc3_dec_config;
static LC3_ENCODER_CHANNEL_CNTX g_lc3enc[LC3_CHANNELS_MAX];
static LC3_DECODER_CHANNEL_CNTX g_lc3dec[LC3_CHANNELS_MAX];
#else
static LC3_ENCODER_CNTX  g_lc3enc;
static LC3_DECODER_CNTX  g_lc3dec;
#endif

#ifndef LC3_STANDALONE
/* extern inputs */
static INT8 input_filename[256] =
    EM_FOPS_PATH_JOIN(EM_FOPS_BASE,"data" EM_FOPS_PATH_SEP "lc3" EM_FOPS_PATH_SEP "SineWaveMinus16.wav");
static INT8 output_filename_enc[256] =
    EM_FOPS_PATH_JOIN(EM_FOPS_BASE,"data" EM_FOPS_PATH_SEP "lc3" EM_FOPS_PATH_SEP "output_44k.bin");
static INT8 output_filename_dec[256] =
    EM_FOPS_PATH_JOIN(EM_FOPS_BASE,"data" EM_FOPS_PATH_SEP "lc3" EM_FOPS_PATH_SEP "output_44k.wav");

#else /* LC3_STANDALONE */

/* static INT8 input_filename[256] = "../data/all_48000hz.wav"; */
/*static INT8 input_filename[256] = "../data/all_44100hz.wav"; */
/* INT8 input_filename[256] = "../data/all_32000hz.wav"; */
/* INT8 input_filename[256] = "../data/all_24000hz.wav"; */
/* INT8 input_filename[256] = "../data/all_16000hz.wav"; */
/* INT8 input_filename[256] = "../data/all_8000hz.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_1ch_16b.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_1ch_24b.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_1ch_32b.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_2ch_16b.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_2ch_24b.wav";  */
/* INT8 input_filename[256] = "../data/all_48000hz_2ch_32b.wav";  */
static INT8 input_filename[256] = "../data/SineWaveMinus16.wav"; 
static INT8 output_filename_enc[256] = "../data/output_1.bin";
static INT8 output_filename_dec[256] = "../data/output_1.wav";
#endif /* LC3_STANDALONE */

/* static INT32 n_dms = 100; */      /* 10ms  */
/* static INT32 n_dms = 75; */   /* 7.5ms */
/* INT32 payload_bytes[LC3_CHANNELS_MAX] = { 160, 160 }; */   /* 128kbps 10ms */
/* static INT32 payload_bytes[LC3_CHANNELS_MAX] = { 100, 100 }; */      /* 80kbps 10ms */
/* INT32 payload_bytes[LC3_CHANNELS_MAX] = { 80, 80 }; */
/* INT32 payload_bytes[LC3_CHANNELS_MAX] = { 40, 40 }; */ /* stereo - 64kbps total */
/* INT32 payload_bytes[LC3_CHANNELS_MAX] = { 60, 60 }; */
/* Note: to match "bitrate" [i,e total bitrate] of reference for stereo or multichannel,
 * channel bytes = (channel-bytes-of-mono)/channels  */
/*  ex. 64kbps 'total' of stereo of reference == 40,40 bytes  for 10ms */
/*      64kbps 'total' of mono of reference == 80 bytes  for 10ms */

static INT32 bps_dec = 16U; /* 16,24,32 */
static INT32 n_dms = FRAME_DURATION;       /* 10ms  */
static INT32 payload_bytes[LC3_CHANNELS_MAX] = { OCTETS_PER_CODEC_FRAME, OCTETS_PER_CODEC_FRAME };      /* 80kbps 10ms */

static INT32 plc_method = 0; /* 0 = default */

/* test wrapper IO buffers */
static INT32  pcm_buffer[LC3_CHANNELS_MAX][LC3_INPUT_FRAME_SIZE_MAX];
static UINT8  enc_buffer[LC3_CHANNELS_MAX][LC3_FRAME_SIZE_MAX];
static INT32  dec_buffer[LC3_CHANNELS_MAX][LC3_INPUT_FRAME_SIZE_MAX];

static INT32  *pcm_buf[LC3_CHANNELS_MAX];
static UINT8  *enc_buf[LC3_CHANNELS_MAX];
static INT32  *dec_buf[LC3_CHANNELS_MAX];

static UINT8  enc_core_buffer[LC3_CHANNELS_MAX*LC3_ENCODER_CORE_BUFFER_SIZE_MAX];
static UINT8  enc_work_buffer[LC3_CHANNELS_MAX*LC3_ENCODER_WORK_BUFFER_SIZE_MAX];

static UINT8  dec_core_buffer[LC3_CHANNELS_MAX*LC3_DECODER_CORE_BUFFER_SIZE_MAX];
static UINT8  dec_work_buffer[LC3_CHANNELS_MAX*LC3_DECODER_WORK_BUFFER_SIZE_MAX];

/* Buffers for Wav File read/write */
static INT32  sample_buf[LC3_CHANNELS_MAX * LC3_INPUT_FRAME_SIZE_MAX];
static UINT8  sample_buf_dec[LC3_CHANNELS_MAX * LC3_FRAME_SIZE_MAX];

/* Application Input */
static LC3_APP_INPUTS AppDataIn;

#if defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
static INT32 lc3_encoder_get_encoded_bitrate(void);
static INT32 lc3_decoder_get_delay_length(void);
static INT32 lc3_encoder_clear_io_data_buffer(INT32 n_samples_idx);
#endif
/* ------------------------------------------- Functions */

/* separate pcm channels from wave file read to encoder */
static void channel_deinterleave_pcm(INT32 *in, INT32 **out, UINT32 len, UINT32 num_channels)
{
    UINT32 i, j;
    for (j = 0; j < num_channels; j++)
    {
        for (i = 0; i < len; i++)
        {
            out[j][i] = in[(i*num_channels) + j];
        }
    }
}

/* separate encoded bitstream from encoded file read to decoder */
static void channel_deinterleave_bs(UINT8 *in, UINT8 **out, UINT32 len, UINT32 num_channels)
{
    UINT32 i, j;
    for (j = 0; j < num_channels; j++)
    {
        for (i = 0; i < len; i++)
        {
            out[j][i] = in[(j*len) + i];
        }
    }
}

/* interleave/mix decoded pcm channels for wave file write */
static void channel_interleave_pcm(INT32 **in, INT32 *out, UINT32 len, UINT32 num_channels)
{
    UINT32 i, j;
    for (j = 0; j < num_channels; j++)
    {
        for (i = 0; i < len; i++)
        {
            out[(i*num_channels) + j] = in[j][i];
        }
    }
}

/* Encoder test function */
static INT32 test_encoder(LC3_APP_INPUTS *pAppIn)
{
    INT32   ret = 0;
    INT32   i;
    UINT32  samples_read = 0;
    UINT32  total_samples = 0xffffffff;
    UINT32  sample_rate = 0;
    INT16   channel_count = 0, bps_in = 0;
    INT32   samples_per_frame;
    INT32   encoded_bitrate;
    LC3_FOPS_WAV_FILE_CTX input_wav_ctx;
    LC3_fops_file_handle output_bitstream;
    LC3_RESULT  retval;
    INT32   nbytes;
    UINT32  processed_frames;

#ifdef LC3_TEST_CMD_LINE
    strcpy(input_filename, pAppIn->input_filename);
    strcpy(output_filename_enc, pAppIn->output_filename);
    n_dms = (INT32)(pAppIn->frame_ms);

    for (i = 0; i < LC3_CHANNELS_MAX; i++)
    {
        payload_bytes[i] = pAppIn->nbytes;
    }
#endif /* LC3_TEST_CMD_LINE */

    /* set IO buffer pointers */
    memset(pcm_buffer, 0, sizeof(pcm_buffer));
    memset(enc_buffer, 0, sizeof(enc_buffer));

    for ( i = 0; i < LC3_CHANNELS_MAX; i++)
    {
        pcm_buf[i] = &pcm_buffer[i][0];
        enc_buf[i] = &enc_buffer[i][0];
    }

    /* Open input wave file */
    retval = LC3_fops_open_wav_reader
             (
                 (UINT8*)input_filename,
                 &sample_rate,
                 (UINT16*)&channel_count,
                 &total_samples,
                 (UINT16*)&bps_in,
                 &input_wav_ctx
             );

    if (EM_SUCCESS != retval)
    {
        return retval;
    }

    printf("input_filename = %s\n",input_filename);
    printf("output_filename = %s\n",output_filename_enc);
    printf("samples_rate = %d\n",sample_rate);
    printf("channel_count = %d\n",channel_count);
    printf("total_samples = %d\n",total_samples);
    printf("bps_in = %d\n",bps_in);
    printf("n_dms= %d\n",n_dms);

#if defined (LC3_DSP) && (LC3_DSP == 1)
    ret = LC3_encoder_create
          (
              &g_lc3enc,
              sample_rate,
              bps_in,
              channel_count,
              n_dms,
              payload_bytes,
              enc_core_buffer,
              enc_work_buffer,
              pcm_buf,
              enc_buf
          );
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    ret = LC3_ENC_SET_CONFIG_PARAMS
				(
					&lc3_enc_config,
					sample_rate,			//the sampling rate
					channel_count,			//number of channels.  If CIS/BIS, = 1
					n_dms,					//frame period
					bps_in					//the compressed data rate
				);

	if (ret == LC3_ENCODER_SUCCESS)
	{
		for (uint8_t chan_id = 0; (!ret) && (chan_id < channel_count); chan_id++)
		{
			//inititialize the ch cnxt
			ret = lc3_enc_init_ch_cntx
					(
						&g_lc3enc[chan_id],
						&lc3_enc_config,
						payload_bytes[chan_id]
					);
		}
	}
#else
    ret = LC3_encoder_create
          (
              &g_lc3enc,
              sample_rate,
              bps_in,
              channel_count,
              n_dms,
              payload_bytes,
              enc_core_buffer,
              enc_work_buffer,
              pcm_buf,
              enc_buf
          );
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

    if (LC3_ENCODER_SUCCESS != ret)
    {
        LC3_fops_file_close(input_wav_ctx.file_handle);

        /* return 2; */

        return ((ret<<8)|(3));
    }

    /* allocate external memory if needed */
#ifdef DEBUG_LOG_MEMORY
    {
#if defined(LC3_DSP) && (LC3_DSP == 1)
        INT32 core_buffer_size = LC3_encoder_get_core_context_buffer_size(&g_lc3enc);
        INT32 work_buffer_size  = LC3_encoder_get_work_buffer_size(&g_lc3enc);
        {
            printf("\n encoder core  size = %d", core_buffer_size);
            printf("\n encoder work/scratch size = %d", work_buffer_size);
        }
#endif /*defined(LC3_DSP) && (LC3_DSP == 1)*/
    }
#endif /* DEBUG_LOG_MEMORY */

#if defined(LC3_DSP) && (LC3_DSP == 1)
    samples_per_frame = LC3_encoder_get_frame_length(&g_lc3enc);
    encoded_bitrate = LC3_encoder_get_encoded_bitrate(&g_lc3enc);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    samples_per_frame = g_lc3enc[0].config->n_f_len;
    encoded_bitrate = lc3_encoder_get_encoded_bitrate();
#else
    samples_per_frame = LC3_encoder_get_frame_length(&g_lc3enc);
    encoded_bitrate = LC3_encoder_get_encoded_bitrate(&g_lc3enc);
#endif /*defined(LC3_DSP) && (LC3_DSP == 0)*/

    /* Open output bitsream file */
    retval = LC3_fops_open_lc3_writer
             (
                 (UINT8*)output_filename_enc,
                 sample_rate,
                 encoded_bitrate,
                 channel_count,
                 n_dms,
                 total_samples,
                 &output_bitstream
             );
    if (EM_SUCCESS != retval)
    {
        /* TODO: Check what value to return */
        return retval;
    }

    /* Iterate - till all frames are encoded */
    processed_frames = 0;
    for(;;)
    {
        /* read input samples -frame size Nf  */

        retval = LC3_fops_wav_read_samples
                 (
                     &input_wav_ctx,
                     (UINT32*)sample_buf,
                     samples_per_frame * channel_count,
                     &samples_read
                 );

        if (EM_SUCCESS != retval)
        {
            /* TODO: Add error log */
            break;
        }

        /* zero out rest of last frame */
        memset(sample_buf + samples_read, 0, (samples_per_frame * channel_count - samples_read) * sizeof(sample_buf[0]));

        if (0 >= samples_read)
        {
            break;
        }

        channel_deinterleave_pcm(sample_buf, pcm_buf, samples_per_frame, channel_count);

        /* clear/zero out rest of last frame */
        if (samples_read < (UINT32)samples_per_frame)
        {
#if defined (LC3_DSP) && (LC3_DSP == 1)
            LC3_encoder_clear_io_data_buffer(&g_lc3enc, samples_read);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
            lc3_encoder_clear_io_data_buffer(samples_read);
#else
            LC3_encoder_clear_io_data_buffer(&g_lc3enc, samples_read);
#endif  /*defined (LC3_DSP) && (LC3_DSP == 1)*/
        }

        /* encode frame */
#if defined (LC3_DSP) && (LC3_DSP == 1)
        nbytes = LC3_encoder_process(&g_lc3enc);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
		nbytes 	= 	LC3_encode_a_frame
					(
						&g_lc3enc[0],
						( INT32 *) pcm_buffer[0],
						(UINT8 *) enc_buffer[0],
						enc_work_buffer
					);
#else
        nbytes = LC3_encoder_process(&g_lc3enc);
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

        if (0 == nbytes)
        {
            break;
        }

        retval = LC3_fops_lc3_write_multichannel_frames
                 (
                     output_bitstream,
                     enc_buf,
                     (UINT32*)payload_bytes,
                     nbytes,
                     channel_count
                 );

        /* TODO: Check retval */

        processed_frames += 1;
#ifdef DEBUG_LOG
        printf("\nEncoder: frames=%d", processed_frames);
#endif /* DEBUG_LOG */
    }
    /* loop-ends */

    printf("\nEncoder:All frames processed=%d\n", processed_frames);

    retval = LC3_fops_file_close(output_bitstream);

#if defined (LC3_DSP) && (LC3_DSP == 1)
    LC3_encoder_delete(&g_lc3enc);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    memset (&g_lc3enc, 0, sizeof (g_lc3enc));
#else
    LC3_encoder_delete(&g_lc3enc);
#endif

    LC3_fops_file_close(input_wav_ctx.file_handle);

#ifdef APPL_ENABLE_EQUAL_IO_BITRATE
    bps_dec = bps_in;
#endif //APPL_ENABLE_EQUAL_IO_BITRATE

    return ret;
}

static INT32 test_decoder(LC3_APP_INPUTS *pAppIn)
{
    INT32   ret = 0;
    INT32   i;
    UINT32  sample_rate;
    INT32   bitrate;
    INT16   channel_count;
    UINT32  total_samples;
    INT32   frame_ms;
    LC3_fops_file_handle input_bitstream;
    LC3_RESULT retval;
    INT32   samples_per_frame;
    INT32   delay;
    INT32   flg_bfi[LC3_CHANNELS_MAX];
    INT32   channel_bytes[LC3_CHANNELS_MAX];
    LC3_FOPS_WAV_FILE_CTX output_wav_ctx;
    UINT32 samples_wrote;
    UINT32  processed_frames;
    INT32   flg_BFI = 0;
    INT32   nbytes;

#ifdef LC3_TEST_CMD_LINE
    strcpy(output_filename_enc, pAppIn->input_filename);
    strcpy(output_filename_dec, pAppIn->output_filename);
    n_dms = (INT32)(pAppIn->frame_ms);
    bps_dec = pAppIn->bps_out;
#endif /* LC3_TEST_CMD_LINE */

    /* set IO buffer pointers */
    memset(enc_buffer, 0, sizeof(enc_buffer));
    memset(dec_buffer, 0, sizeof(dec_buffer));

    for ( i = 0; i < LC3_CHANNELS_MAX; i++)
    {
        enc_buf[i] = &enc_buffer[i][0];
        dec_buf[i] = &dec_buffer[i][0];
    }

    /* open encoder input  file */
    retval = LC3_fops_open_lc3_reader
             (
                 (UINT8*)output_filename_enc,
                 &sample_rate,
                 (UINT32*)&bitrate,
                 (UINT16*)&channel_count,
                 (UINT32*)&frame_ms,
                 &total_samples,
                 &input_bitstream
             );
    if (EM_SUCCESS != retval)
    {
        /* TODO: Check what value to return */
        return retval;
    }

    printf("input_filename = %s\n",output_filename_enc);
    printf("output_filename = %s\n",output_filename_dec);
    printf("samples_rate = %d\n",sample_rate);
    printf("channel_count = %d\n",channel_count);
    printf("total_samples = %d\n",total_samples);
    printf("bps_out= %d\n",bps_dec);
    printf("n_dms= %d\n",n_dms);

#if defined (LC3_DSP) && (LC3_DSP == 1)
    /* create context/session */
    ret = LC3_decoder_create
          (
              &g_lc3dec,
              sample_rate,
              bps_dec,
              channel_count,
              n_dms,
              plc_method,
              dec_core_buffer,
              dec_work_buffer,
              enc_buf,
              dec_buf
          );
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    ret = LC3_DEC_SET_CONFIG_PARAMS
    			 (
    				 &lc3_dec_config,
					 sample_rate,
					 channel_count,
					 plc_method,
					 n_dms,
					 bps_dec
				 );

    if (LC3_DECODER_SUCCESS == ret)
    {
        for (uint8_t chan_index = 0; (!ret) && (chan_index < channel_count); chan_index++)
        {
        	ret = lc3_dec_init_ch_cntx
        				(
        					&g_lc3dec[chan_index],
							&lc3_dec_config,
							100				  //TODO: Verify this param
						);
        }
    }
#else
    /* create context/session */
    ret = LC3_decoder_create
          (
              &g_lc3dec,
              sample_rate,
              bps_dec,
              channel_count,
              n_dms,
              plc_method,
              dec_core_buffer,
              dec_work_buffer,
              enc_buf,
              dec_buf
          );
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

    if (LC3_DECODER_SUCCESS != ret)
    {
        retval = LC3_fops_file_close(input_bitstream);

        return ((ret << 8) | (3));
    }

    /* allocate external memory if needed using below size*/
#ifdef DEBUG_LOG_MEMORY
    {
#if defined (LC3_DSP) && (LC3_DSP == 1)
        INT32 core_buffer_size = LC3_decoder_get_core_context_buffer_size(&g_lc3dec);
        INT32 work_buffer_size = LC3_decoder_get_work_buffer_size(&g_lc3dec);
        printf("\n decoder core  size = %d", core_buffer_size);
        printf("\n decoder work/scratch size = %d", work_buffer_size);
    }
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/
#endif /* DEBUG_LOG_MEMORY */

#if defined(LC3_DSP) && (LC3_DSP == 1)
    samples_per_frame = LC3_decoder_get_frame_length(&g_lc3dec);
    /* delay compensated in decoded side (default) */
    delay = LC3_decoder_get_delay_length(&g_lc3dec);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    samples_per_frame = lc3_dec_config.n_f_len;
  //  samples_per_frame = g_lc3enc[0].config->n_f_len;
    delay = lc3_decoder_get_delay_length();
#else
    samples_per_frame = LC3_decoder_get_frame_length(&g_lc3dec);
    /* delay compensated in decoded side (default) */
    delay = LC3_decoder_get_delay_length(&g_lc3dec);
#endif /*defined(LC3_DSP) && (LC3_DSP == 0)*/

    /* open output wav file */
    retval = LC3_fops_open_wav_writer
             (
                 (UINT8*)output_filename_dec,
                 sample_rate,
                 channel_count,
                 bps_dec,
                 &output_wav_ctx
             );

    if (EM_SUCCESS != retval)
    {
#if defined (LC3_DSP) && (LC3_DSP == 1)
        LC3_decoder_delete(&g_lc3dec);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
        memset (&g_lc3dec, 0, sizeof (g_lc3dec));
#else
        LC3_decoder_delete(&g_lc3dec);
#endif
        retval = LC3_fops_file_close(input_bitstream);
        return 4;
    }

    memset(flg_bfi, 0, sizeof(flg_bfi));
    memset(channel_bytes, 0, sizeof(channel_bytes));

    /* loop-starts */
    processed_frames = 0;
    for(;;)
    {
        /* read encoded input samples */
        flg_BFI=0; /* External error simulation : set flg_BFI=1 */

        retval = LC3_fops_lc3_read_frame
                 (
                     input_bitstream,
                     sample_buf_dec,
                     sizeof(sample_buf_dec),
                     (UINT32*)&nbytes
                 );

        /* Check retval */
        if (EM_SUCCESS != retval)
        {
            break;
        }

        nbytes = nbytes / channel_count;   /* per channel bytes ,assuming same bitrate for all channels */
        for ( i = 0; i < channel_count; i++)
        {
            flg_bfi[i] = flg_BFI;
            channel_bytes[i] = nbytes;
        }
        /* unpack data to particular channel buffer */
        channel_deinterleave_bs(sample_buf_dec, enc_buf, nbytes, channel_count);

        /* decode the 1 frame */
#if defined (LC3_DSP) && (LC3_DSP == 1)
        ret = LC3_decoder_process(&g_lc3dec, flg_bfi, channel_bytes);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
        ret = LC3_decode_a_frame
					(
						&g_lc3dec[0],
						(UINT8 *) sample_buf_dec,
						(INT32 *) sample_buf,
						(INT16) flg_bfi[0],
						dec_work_buffer
					);
#else
        ret = LC3_decoder_process(&g_lc3dec, flg_bfi, channel_bytes);
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

        if (0 != (ret))
        {
            /* TODO: Error logs */
            /* frame decode error occurred and concealed */
        }

        channel_interleave_pcm(dec_buf,sample_buf, samples_per_frame, channel_count);

        /* write/save decoded data to wav file  */
        retval = LC3_fops_wav_write_samples
                 (
                     &output_wav_ctx,
                     (UINT32*)(sample_buf + delay * channel_count),
                     (min((INT32)(samples_per_frame - delay), (INT32)total_samples) * channel_count),
                     &samples_wrote
                 );

        total_samples -= samples_per_frame - delay;
        delay = 0;

        processed_frames += 1;

#ifdef DEBUG_LOG
        printf("\nDecoder: frames =%d", processed_frames);
#endif /* DEBUG_LOG */
    }
    /* loop-ends */

    /* write last residual chunks .. if any */
    if (total_samples > (UINT32)0 && total_samples < (UINT32)samples_per_frame)
    {
        memset(sample_buf, 0, (total_samples * channel_count) * sizeof(sample_buf[0]));

        retval = LC3_fops_wav_write_samples
                 (
                     &output_wav_ctx,
                     (UINT32*)sample_buf,
                     total_samples * channel_count,
                     &samples_wrote
                 );
    }

    printf("\nDecoder:All frames processed=%d\n", processed_frames);

    retval = LC3_fops_file_close(input_bitstream);

#if defined (LC3_DSP) && (LC3_DSP == 1)
    LC3_decoder_delete(&g_lc3dec);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    memset (&g_lc3dec, 0, sizeof (g_lc3dec));
#else
    LC3_decoder_delete(&g_lc3dec);
#endif

    retval = LC3_fops_close_wav_writer(&output_wav_ctx);

    return ret;
}

#ifdef LC3_TEST_CMD_LINE
static const char lc3_appl_usage_options[] =
"Usage: LC3test [OPTIONS] INPUT OUTPUT BYTES\n"
"\n"
"  INPUT and OUTPUT are wav or binary  files, depends on mode selected in OPTIONS.\n"
"  BYTES is  in bytes per channel for encoder. [20 to 400]\n"
"\n options:\n"
" -mode NUM                   NUM = 0,1,2.                                     \n"
"       mode=0 =>Encode and Decode, mode=1 => Encode, mode=2 => Decode        \n"
"  -frame_ms               NUM Frame length in ms. NUM = 10 (default), 7.5 .\n"
"  -bps NUM                Decode output bits per sample. NUM = 16 (default),24,32.\n"
"\n"
"Examples:\n"
"LC3test -mode 1 -frame_ms 10   thetest48.wav   thetest48.bin  60\n"
"LC3test -mode 2 -frame_ms 10   thetest48.bin   thetest48-out.wav  60\n"
"LC3test -mode 0 -frame_ms 7.5   thetest48.wav   thetest48-out2.wav  60\n"
"LC3test -mode 2 -bps 16 -frame_ms 10    thetest48.bin   thetest48-out.wav   60\n"
"LC3test -mode 0 -bps 24 -frame_ms 7.5   thetest48.wav   thetest48-out2.wav  60\n"
"\n"
;
#endif /* LC3_TEST_CMD_LINE */

static const CHAR *ENCODER_LOG_STRING[4] = {
    "",                                                 /* no error */
    "Input file open failed!",
    "Encoder context create failed!",
    "Encoder init failed!",
};

static const CHAR *DECODER_LOG_STRING[5] = {
    "",                                                 /* no error */
    "Input file open failed!",
    "Decoder context create failed!",
    "Decoder init failed!",
    "Output file open failed!",
};

static const CHAR *ENCODER_SUB_LOG_STRING[11] = {
    "",                                                 /* no error */
    "core buffer size is 0!",
    "core buffer handle is null!",
    "parameter init is failed!",
    "bitrate update is failed!",
    "work/scratch buffer handle is null!",
    "invalid sampling rate!",
    "invalid frame_ms!",
    "invalid number of channels!",
    "invalid target bytes",
    "invalid context!",
};

static const CHAR *DECODER_SUB_LOG_STRING[10] = {
    "",                                                 /* no error */
    "core buffer size is 0!",
    "core buffer handle is null!",
    "parameter init is failed!",
    "work/scratch buffer handle is null!",
    "invalid sampling rate!",
    "invalid frame_ms!",
    "invalid number of channels!",
    "invalid PLC method!",
    "invalid context!",
};

#ifdef LC3_TEST_CMD_LINE
static INT32 parseInputs(INT32 ac, INT8 **av, LC3_APP_INPUTS *arg)
{
    INT32 ret = 0;

    if (ac < 2)
    {
        printf("Error : no arguments\n");
        printf("\n%s", lc3_appl_usage_options);
        return 1;
    }

    INT32 pos = 1;

    /* parse options in any order */
    for (; (pos < ac) && ('-' == av[pos][0]); pos++)
    {
        if (0 == (strcmp(av[pos], "-h")))
        {
            printf("\n%s", lc3_appl_usage_options);
            return 2;
        }
        /* mode  */
        if (!strcmp(av[pos], "-mode") && pos + 1 < ac)
        {
            arg->mode = (INT32)atoi(av[++pos]);
        }
        /* bps  */
        if (!strcmp(av[pos], "-bps") && pos + 1 < ac)
        {
            arg->bps_out = (INT32)atoi(av[++pos]);
        }
        /* frame length in ms */
        if (!strcmp(av[pos], "-frame_ms") && pos + 1 < ac)
        {
            arg->frame_ms = (INT32)(((float)atof(av[++pos]))*10);
        }
    }

    if((pos + 1) >= ac)
    {
        printf("Error : missing arguments\n");
        return 3;
    }

    strcpy(arg->input_filename,av[pos++]);
    strcpy(arg->output_filename,av[pos++]);

    if (2 != arg->mode)
    {
        if (pos >= ac) /* last param */
        {
            printf("Error : missing arguments\n");
            return 3;
        }
        /*arg->bitrate = atoi(av[pos]); */
        arg->nbytes = atoi(av[pos]);
    }

    /*Check inputs */

    if (!((arg->mode >= 0) && (arg->mode < 3)))
    {
        printf("Error : wrong arguments : mode \n");
        return 4;
    }
    if (!((100 == arg->frame_ms) || (75 == arg->frame_ms)))
    {
        printf("Error : wrong arguments : frame_ms \n");
        return 4;
    }
#if 0
    if ((arg->bitrate <= 0))
    {
        printf("Error : wrong arguments : bitrate \n");
        return 4;
    }
#endif /* 0 */
    if ((arg->nbytes < 20) || (arg->nbytes > 400))
    {
        printf("Error : wrong arguments : byte \n");
        return 4;
    }
    if (!((16 == arg->bps_out) || (24 == arg->bps_out) || (32 == arg->bps_out)))
    {
        printf("Error : wrong arguments : byte \n");
        return 4;

    }
    return ret;
}
#endif /* LC3_TEST_CMD_LINE */


#ifdef LC3_TEST_CMD_LINE
#ifndef EM_PLATFORM_MAIN
INT32 main(int ac, char **av)
#else /* EM_PLATFORM_MAIN */
INT32 appl_lc3(int ac, char **av)
#endif /* EM_PLATFORM_MAIN */
{
    INT32 ret = 0;
    INT32 err = 0;
    INT32 err_sub = 0;

    memset(&AppDataIn, 0, sizeof(LC3_APP_INPUTS));
    /* defaults */

    /* AppDataIn.bitrate = 64000; */
    AppDataIn.bps_out = 16;
    AppDataIn.frame_ms = 100;
    AppDataIn.mode = 0;
    AppDataIn.nbytes = 80; /* 64kbps for 10.0 ms */
    /* strcpy(AppDataIn.input_filename, input_filename); */
    /* strcpy(AppDataIn.OuputFileName, output_filename_dec); */

    ret = parseInputs(ac, av, &AppDataIn);
    if (0 != (ret))
    {
        /* error print */
        /* printf("\nError\n"); */
        /* puts(USAGE_MESSAGE); */
        return 2;
    }
    /*  Input Checks */

    if (0 == AppDataIn.mode)
    {
        LC3_APP_INPUTS AppDataInE, AppDataInD;
        memcpy(&AppDataInE, &AppDataIn, sizeof(LC3_APP_INPUTS));
        memcpy(&AppDataInD, &AppDataIn, sizeof(LC3_APP_INPUTS));

        strcpy(&AppDataInE.output_filename[strlen(AppDataInE.output_filename) - 4],"_tmp.bin");
        strcpy(AppDataInD.input_filename, AppDataInE.output_filename);
        AppDataInE.mode = 1;
        AppDataInD.mode = 2;

        ret = test_encoder(&AppDataInE);
        if (ret)
        {
            err = (ret & 0xFF);
            err_sub = ((ret>>8) & 0xFF);
            printf("Encoder Error : %s : %s\n", ENCODER_LOG_STRING[err], ENCODER_SUB_LOG_STRING[err_sub]);
        }
        else
        {
            ret = test_decoder(&AppDataInD);
            if (0 != (ret))
            {
                err = (ret & 0xFF);
                err_sub = ((ret>>8) & 0xFF);
                printf("Decoder Error : %s : %s\n", DECODER_LOG_STRING[err], DECODER_SUB_LOG_STRING[err_sub]);
            }
        }
    }
    else if (1 == AppDataIn.mode)
    {
        ret = test_encoder(&AppDataIn);
        if (0 != (ret))
        {
            err = (ret & 0xFF);
            err_sub = ((ret>>8) & 0xFF);
            printf("Encoder Error : %s : %s\n", ENCODER_LOG_STRING[err], ENCODER_SUB_LOG_STRING[err_sub]);
        }
    }
    else if (2 == AppDataIn.mode)
    {
        ret = test_decoder(&AppDataIn);
        if (0 != (ret))
        {
            err = (ret & 0xFF);
            err_sub = ((ret>>8) & 0xFF);
            printf("Decoder Error : %s : %s\n", DECODER_LOG_STRING[err], DECODER_SUB_LOG_STRING[err_sub]);
        }
    }

    return 0;
}

#else

#ifdef HAVE_LC3_PROFILING
/* Flag to diplay LC3 Encoder input/output data on the Console */
#define DISPLAY_LC3_IN_OUT_DATA

/* Number of frames to display when DISPLAY_LC3_IN_OUT_DATA is enabled */
#define MAX_LC3_FRAMES_TO_DISPLAY            20


/* Defines used for LC3 Codec profiling on ARM Corterx-M7 */
uint32_t lc3_enc_frame_counter;
uint32_t total_lc3_enc_frame_counter;
uint32_t total_lc3_enc_frame_counter_1;

uint32_t lc3_dec_frame_counter;
uint32_t total_lc3_dec_frame_counter;
uint32_t total_lc3_dec_frame_counter_1;

volatile unsigned int *DWT_CYCCNT = (uint32_t *)0xE0001004; /* address of the register */
volatile unsigned int *DWT_CONTROL = (uint32_t *)0xE0001000; /* address of the register */
volatile unsigned int *SCB_DEMCR = (uint32_t *)0xE000EDFC; /* address of the register */

/**
 * This function gets the input PCM data from global array present in 'lc3_encoder_input_data.h'
 * and encodes the PCM frame and then decode the encoded frame.
 *
 * If 'HAVE_LC3_PROFILING' is defined, then calculates the CPU cycles for each Encoded & Decoded
 * frame and diplays for every 5 frames.
 */
static INT32 test_lc3_encoder_decoder(void)
{
    INT32   ret = 0;
    INT32   i;
    UINT32  samples_read = 0;
    UINT32  sample_rate = 0;
    INT16   channel_count = 0, bps_in = 0;
    INT32   samples_per_frame;
    INT32   encoded_bitrate;

    INT32   delay;
    INT32   flg_bfi[LC3_CHANNELS_MAX];
    INT32   channel_bytes[LC3_CHANNELS_MAX];

    LC3_RESULT  retval;
    INT32   nbytes;
    UINT32  processed_frames;

    INT32   flg_BFI = 0;

#ifdef DISPLAY_LC3_IN_OUT_DATA
    UINT32  in_index;
    UINT32  sample_cnt;
#endif /* DISPLAY_LC3_IN_OUT_DATA */

    UINT32  pcm_offset;
    int choice;

    /* Set IO buffer pointers */
    memset(pcm_buffer, 0, sizeof(pcm_buffer));
    memset(enc_buffer, 0, sizeof(enc_buffer));
    memset(dec_buffer, 0, sizeof(dec_buffer));

    for (i = 0; i < LC3_CHANNELS_MAX; i++)
    {
        pcm_buf[i] = &pcm_buffer[i][0];
        enc_buf[i] = &enc_buffer[i][0];
        dec_buf[i] = &dec_buffer[i][0];
    }

    /* Init */
    sample_rate = SAMPLING_RATE;
    channel_count = CHANNEL_COUNT;
    n_dms= FRAME_DUR;
    payload_bytes[0] = OCTETS_PER_FRAME;
    payload_bytes[1] = OCTETS_PER_FRAME;

    pcm_offset = 0;
    bps_in = 16;
    bps_dec = 16;

    printf("sample_rate = %d\n",sample_rate);
    printf("channel_count = %d\n",channel_count);
    printf("bps_in = %d\n",bps_in);
    printf("bps_out= %d\n",bps_dec);
    printf("n_dms= %d\n",n_dms);

    printf ("Default Configuration for Sampling Rate %d? (1/0): ", sample_rate);
    scanf("%d", &choice);

    if (0 == choice)
    {
        printf("Enter Channel Count: ");
        scanf("%d", &choice);
        channel_count = choice;

        printf("Enter Frame Duration: ");
        scanf("%d", &choice);
        n_dms = choice;

        printf("Enter Octets per Frame: ");
        scanf("%d", &choice);
        payload_bytes[0] = choice;
        payload_bytes[1] = choice;
    }

#if defined (LC3_DSP) && (LC3_DSP == 1)
    ret = LC3_encoder_create
          (
              &g_lc3enc,
              sample_rate,
              bps_in,
              channel_count,
              n_dms,
              payload_bytes,
              enc_core_buffer,
              enc_work_buffer,
              pcm_buf,
              enc_buf
          );
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    ret = LC3_ENC_SET_CONFIG_PARAMS
				(
					&lc3_enc_config,
					sample_rate,			//the sampling rate
					channel_count,			//number of channels.  If CIS/BIS, = 1
					n_dms,					//frame period
					bps_in					//the compressed data rate
				);

	if (ret == LC3_ENCODER_SUCCESS)
	{
		for (uint8_t chan_id = 0; (!ret) && (chan_id < channel_count); chan_id++)
		{
			//inititialize the ch cnxt
			ret = lc3_enc_init_ch_cntx
					(
						&g_lc3enc[chan_id],
						&lc3_enc_config,
						payload_bytes[chan_id]
					);
		}
	}
#else
    ret = LC3_encoder_create
          (
              &g_lc3enc,
              sample_rate,
              bps_in,
              channel_count,
              n_dms,
              payload_bytes,
              enc_core_buffer,
              enc_work_buffer,
              pcm_buf,
              enc_buf
          );
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

    if (LC3_ENCODER_SUCCESS != ret)
    {
        printf("Encoder Error : %s : %s\n", ENCODER_LOG_STRING[3], ENCODER_SUB_LOG_STRING[ret]);
        return ret;
    }

#if defined (LC3_DSP) && (LC3_DSP == 1)
    /* create context/session */
    ret = LC3_decoder_create
          (
              &g_lc3dec,
              sample_rate,
              bps_dec,
              channel_count,
              n_dms,
              plc_method,
              dec_core_buffer,
              dec_work_buffer,
              enc_buf,
              dec_buf
          );
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    ret = LC3_DEC_SET_CONFIG_PARAMS
    			 (
    				 &lc3_dec_config,
					 sample_rate,
					 channel_count,
					 plc_method,
					 n_dms,
					 bps_dec
				 );

    if (LC3_DECODER_SUCCESS == ret)
    {
        for (uint8_t chan_index = 0; (!ret) && (chan_index < channel_count); chan_index++)
        {
        	ret = lc3_dec_init_ch_cntx
        				(
        					&g_lc3dec[chan_index],
							&lc3_dec_config,
							payload_bytes[chan_index]  //TODO: Verify this param, updated
						);
        }
    }
#else
    /* create context/session */
    ret = LC3_decoder_create
          (
              &g_lc3dec,
              sample_rate,
              bps_dec,
              channel_count,
              n_dms,
              plc_method,
              dec_core_buffer,
              dec_work_buffer,
              enc_buf,
              dec_buf
          );
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

    if (LC3_DECODER_SUCCESS != ret)
    {
        printf("Decoder Error : %s : %s\n", DECODER_LOG_STRING[3], DECODER_SUB_LOG_STRING[ret]);
        return ret;
    }

#if defined(LC3_DSP) && (LC3_DSP == 1)
    samples_per_frame = LC3_encoder_get_frame_length(&g_lc3enc);
    encoded_bitrate = LC3_encoder_get_encoded_bitrate(&g_lc3enc);
    /* delay compensated in decoded side (default) */
    delay = LC3_decoder_get_delay_length(&g_lc3dec);
#elif defined (LC3_DSP) && ( (LC3_DSP == 0) || (LC3_DSP == 2))
    samples_per_frame = g_lc3enc[0].config->n_f_len;
    encoded_bitrate = lc3_encoder_get_encoded_bitrate();
    /* delay compensated in decoded side (default) */
    delay = lc3_decoder_get_delay_length();
#else
    samples_per_frame = LC3_encoder_get_frame_length(&g_lc3enc);
    encoded_bitrate = LC3_encoder_get_encoded_bitrate(&g_lc3enc);
    /* delay compensated in decoded side (default) */
    delay = LC3_decoder_get_delay_length(&g_lc3dec);
#endif /*defined(LC3_DSP) && (LC3_DSP == 0)*/

    memset(flg_bfi, 0, sizeof(flg_bfi));
    memset(channel_bytes, 0, sizeof(channel_bytes));

    /* Iterate - till all frames are encoded */
    processed_frames = 0;

    printf("samples_per_frame = %d\n",samples_per_frame);

    lc3_enc_frame_counter = 0;
    total_lc3_enc_frame_counter = 0;
    total_lc3_enc_frame_counter_1 = 0;

    lc3_dec_frame_counter = 0;
    total_lc3_dec_frame_counter = 0;
    total_lc3_dec_frame_counter_1 = 0;

    for(;;)
    {
        /* read input samples -frame size Nf  */

        if (processed_frames >= MAX_PCM_FRAME_CNT)
        {
            /* printf ("\n=== Processed Frame Count: %d ===\n", processed_frames); */
            break;
        }

        /* Copy the PCM data from global array */
        memcpy(&sample_buf[0], &lc3_enc_input_pcm_data[pcm_offset],  4*samples_per_frame * channel_count);
        pcm_offset +=  samples_per_frame * channel_count;
        samples_read = samples_per_frame * channel_count;

#ifdef DISPLAY_LC3_IN_OUT_DATA
        if (processed_frames <= MAX_LC3_FRAMES_TO_DISPLAY)
        {
            sample_cnt = 0;
            printf ("\n*%d*----------LC3 Encoder input:%d----------\n", processed_frames, samples_per_frame);
            for(in_index = 0; in_index < samples_per_frame; in_index++)
            {
                printf ("0x%08X, ", sample_buf[in_index]);
                sample_cnt += 1;

                if (10 == sample_cnt)
                {
                    printf ("\n");
                sample_cnt = 0;
                }
            }
            printf ("\n--------------------\n");
        }
#endif /* DISPLAY_LC3_IN_OUT_DATA */

        /* zero out rest of last frame */
        memset(sample_buf + samples_read, 0, (samples_per_frame * channel_count - samples_read) * sizeof(sample_buf[0]));

        if (0 >= samples_read)
        {
            break;
        }

        channel_deinterleave_pcm(sample_buf, pcm_buf, samples_per_frame, channel_count);

        /* clear/zero out rest of last frame */
        if (samples_read < (UINT32)samples_per_frame)
        {
#if defined (LC3_DSP) && (LC3_DSP == 1)
            LC3_encoder_clear_io_data_buffer(&g_lc3enc, samples_read);
#elif defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2))
            lc3_encoder_clear_io_data_buffer(samples_read);
#else
            LC3_encoder_clear_io_data_buffer(&g_lc3enc, samples_read);
#endif  /*defined (LC3_DSP) && (LC3_DSP == 1)*/
        }

        /* configure and start the clock cycles counter */
        lc3_enc_frame_counter = 0;
        *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
        *DWT_CYCCNT = 0;
        *DWT_CONTROL |=  1;

        /* encode frame */
#if defined (LC3_DSP) && (LC3_DSP == 1)
        nbytes = LC3_encoder_process(&g_lc3enc);
#elif defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2))
		nbytes 	= 	LC3_encode_a_frame
					(
						&g_lc3enc[0],
						pcm_buffer[0],
						enc_buffer[0],
						enc_work_buffer
					);
#else
        nbytes = LC3_encoder_process(&g_lc3enc);
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/
        if (0 == nbytes)
        {
            break;
        }

        /* stop and get the counter value */
        *DWT_CONTROL &= ~1;
        lc3_enc_frame_counter = *DWT_CYCCNT;

        total_lc3_enc_frame_counter += lc3_enc_frame_counter;
        total_lc3_enc_frame_counter_1 += lc3_enc_frame_counter;

#ifdef DISPLAY_LC3_IN_OUT_DATA
        if (processed_frames <= MAX_LC3_FRAMES_TO_DISPLAY)
        {
            sample_cnt = 0;
            printf ("\n-%d-*****LC3 Encoder Output:%d *****\n", processed_frames, nbytes);
            for(in_index = 0; in_index < nbytes; in_index++)
            {
                printf ("0x%02X, ", enc_buffer[0][in_index]);
                /* printf ("0x%02X, ", enc_buf[in_index]); */

                sample_cnt += 1;

                if (10 == sample_cnt)
                {
                    printf ("\n");
                    sample_cnt = 0;
                }
            }
            printf ("\n***********************************\n");
        }
#endif /* DISPLAY_LC3_IN_OUT_DATA */

        /* LC3 Decoder related */

        /* read encoded input samples */
        flg_BFI=0; /* External error simulation : set flg_BFI=1 */

        nbytes = nbytes / channel_count;   /* per channel bytes ,assuming same bitrate for all channels */
        for ( i = 0; i < channel_count; i++)
        {
            flg_bfi[i] = flg_BFI;
            channel_bytes[i] = nbytes;
        }

        /* unpack data to particular channel buffer */
        /* channel_deinterleave_bs(sample_buf_dec, enc_buf, nbytes, channel_count); */

        /* configure and start the clock cycles counter */
        lc3_dec_frame_counter = 0;
        *SCB_DEMCR = *SCB_DEMCR | 0x01000000;
        *DWT_CYCCNT = 0;
        *DWT_CONTROL |=  1;

        /* decode the 1 frame */
#if defined (LC3_DSP) && (LC3_DSP == 1)
        ret = LC3_decoder_process(&g_lc3dec, flg_bfi, channel_bytes);
#elif defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2))
        ret = LC3_decode_a_frame
					(
						&g_lc3dec[0],
						enc_buffer[0],
						dec_buffer[0],
						flg_bfi[0],
						dec_work_buffer
					);
#else
        ret = LC3_decoder_process(&g_lc3dec, flg_bfi, channel_bytes);
#endif /*defined (LC3_DSP) && (LC3_DSP == 1)*/

        if (0 != (ret))
        {
            /* TODO: Error logs */
            /* frame decode error occurred and concealed */
        }

        /* stop and get the counter value */
        *DWT_CONTROL &= ~1;
        lc3_dec_frame_counter = *DWT_CYCCNT;

        total_lc3_dec_frame_counter += lc3_dec_frame_counter;
        total_lc3_dec_frame_counter_1 += lc3_dec_frame_counter;

        channel_interleave_pcm(dec_buf,sample_buf, samples_per_frame, channel_count);

#ifdef DISPLAY_LC3_IN_OUT_DATA
        if (processed_frames <= MAX_LC3_FRAMES_TO_DISPLAY)
        {
            sample_cnt = 0;
            printf ("\n*%d*----- LC3 Decoder Output:%d -----\n", processed_frames, (samples_per_frame*channel_count));
            for(in_index = 0; in_index < (samples_per_frame*channel_count); in_index++)
            {
                printf ("0x%08X, ", sample_buf[in_index]);
                sample_cnt += 1;

                if (10 == sample_cnt)
                {
                    printf ("\n");
                    sample_cnt = 0;
                }
            }
            printf ("\n-----------------------------------\n");
        }
#endif /* DISPLAY_LC3_IN_OUT_DATA */

        processed_frames += 1;

#ifdef DEBUG_LOG
        if (0 == (processed_frames%5))
        {
            printf("\nEnc:%u, Dec:%u", total_lc3_enc_frame_counter_1, total_lc3_dec_frame_counter_1);
            total_lc3_enc_frame_counter_1 = 0;
            total_lc3_dec_frame_counter_1 = 0;
        }
#endif /* DEBUG_LOG */
    }
    /* loop-ends */

    /* print the counter value */
    printf("\n\nTotal Enc. Cycle Count for %u frames = : %u\n\r", processed_frames, total_lc3_enc_frame_counter);
    printf("Total Dec. Cycle Count for %u frames = : %u\n\r", processed_frames, total_lc3_dec_frame_counter);

#if defined (LC3_DSP) && (LC3_DSP == 1)
        LC3_decoder_delete(&g_lc3dec);
        LC3_encoder_delete(&g_lc3enc);
#elif defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2))
        memset (&g_lc3enc, 0, sizeof (g_lc3enc));
        memset (&g_lc3dec, 0, sizeof (g_lc3dec));
#else
        LC3_encoder_delete(&g_lc3enc);
        LC3_decoder_delete(&g_lc3dec);
#endif

    return ret;
}
#endif /* HAVE_LC3_PROFILING */


#ifndef EM_PLATFORM_MAIN
INT32 main(int ac, char **av)
#else /* EM_PLATFORM_MAIN */
INT32 appl_lc3(void)
#endif /* EM_PLATFORM_MAIN */
{
    INT32 ret = 0;
#ifdef HAVE_LC3_PROFILING
    int choice;
    do {
        ret = test_lc3_encoder_decoder();
        printf ("Process Return - 0x%08X\n", ret);

        printf ("Continue with another configuration for %d Sampling Rate? (1/0): ", SAMPLING_RATE);
        scanf("%d", &choice);

    } while (choice == 1);

    printf ("Exiting the LC3 Test. To Test Again, Please Restart The Application\n");
#else /* HAVE_LC3_PROFILING */

    INT32 err = 0;
    INT32 err_sub = 0;

    ret = test_encoder(&AppDataIn);
    if (0 != (ret))
    {
        /* printf("Encoder:Error : %d \n", ret); */
        err = (ret & 0xFF);
        err_sub = ((ret>>8) & 0xFF);
        printf("Error : %s : %s\n", ENCODER_LOG_STRING[err], ENCODER_SUB_LOG_STRING[err_sub]);
    }
    ret = test_decoder(&AppDataIn);
    if (0 != (ret))
    {
        /* printf("Decoder:Error : %d \n", ret); */
        err = (ret & 0xFF);
        err_sub = ((ret>>8) & 0xFF);
        printf("Error : %s : %s\n", DECODER_LOG_STRING[err], DECODER_SUB_LOG_STRING[err_sub]);
    }
#endif /* HAVE_LC3_PROFILING */

    return ret;
}
#endif /* LC3_TEST_CMD_LINE */

#if defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2))
static INT32 lc3_encoder_get_encoded_bitrate( void )
{
    INT32 i;
    INT32 bitrate = 0;
    INT32 totalBytes = 0;

    if (0 < lc3_enc_config.bitrate)
    {
        for (i = 0; i < lc3_enc_config.n_c; i++)
        {
            totalBytes +=g_lc3enc[i].nbytes;
        }
        bitrate = (INT32)((totalBytes * 80000 + lc3_enc_config.n_dms - 1) / lc3_enc_config.n_dms);

        if (44100 == lc3_enc_config.sample_rate)
        {
            INT32 rem = bitrate % 480;
            bitrate = ((bitrate - rem) / 480) * 441 + (rem * 441) / 480;
        }
    }

    return bitrate;
}

static INT32 lc3_decoder_get_delay_length(void)
{
    INT32 samples = 0;
    samples = lc3_dec_config.n_f_len - 2 * lc3_dec_config.z_len;
    return samples;
}

static INT32 lc3_encoder_clear_io_data_buffer(INT32 n_samples_idx)
{
    INT32 i;
    INT32 *pInput;
    INT32 samples, samples2;
    samples = lc3_enc_config.n_f_len;;

    for (i = 0; i < lc3_enc_config.n_c; i++)
    {
        pInput = (INT32 *) &pcm_buffer[i];
        samples2 = samples - n_samples_idx;
        if (0 < samples2)
        {
            memset(&pInput[n_samples_idx], 0, sizeof(INT32)*(samples2));
        }
    }
    return (INT32)0;
}
#endif /* defined (LC3_DSP) && ((LC3_DSP == 0) || (LC3_DSP == 2)) */


#endif
