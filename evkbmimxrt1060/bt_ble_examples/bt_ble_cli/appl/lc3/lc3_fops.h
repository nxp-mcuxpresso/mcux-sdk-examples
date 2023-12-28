#ifdef LC3_TEST
/**
 *  \file LC3_fops.h
 *
 *  This file has all definitions of constants and declarations APIs
 *  for LC3 and Wave File Operations.
 */

/*
 *  Copyright (C) 2021. Mindtree Ltd.
 *  All rights reserved.
 */

#ifndef _H_LC3_FOPS_
#define _H_LC3_FOPS_

/* ------------------------- Header File Inclusion */
/* EtherMind Common Header Files */
#include "lc3_EM_os.h"
#include "lc3_EM_fops.h"
#include "lc3_EM_debug.h"

/* ------------------------- Debug Macros */
#ifndef LC3_FOPS_NO_DEBUG
#define LC3_FOPS_ERR(...)         EM_debug_error(0, __VA_ARGS__)
#else  /* LC3_FOPS_NO_DEBUG */
#define LC3_FOPS_ERR              EM_debug_null
#endif /* LC3_FOPS_NO_DEBUG */

#ifdef LC3_FOPS_DEBUG

#define LC3_FOPS_TRC(...)         EM_debug_trace(0, __VA_ARGS__)
#define LC3_FOPS_INF(...)         EM_debug_info(0, __VA_ARGS__)

#define LC3_FOPS_debug_dump_bytes(data, datalen) EM_debug_dump_bytes(0, (data), (datalen))

#else /* LC3_FOPS_DEBUG */

#define LC3_FOPS_TRC              EM_debug_null
#define LC3_FOPS_INF              EM_debug_null

#define LC3_FOPS_debug_dump_bytes(data, datalen)

#endif /* LC3_FOPS_DEBUG */

/* Function Return Value type */
/* TODO: Move to appropriate location */
typedef UINT16 LC3_RESULT;

/* ------------------------- Global Definitions */
/* Platform type for File handle */
typedef EM_fops_file_handle LC3_fops_file_handle;

typedef struct _LC3_FOPS_WAV_FILE_CTX
{
    /** File Handle */
    LC3_fops_file_handle file_handle;

    /* TODO: Change Names */
    /** Current Position */
    UINT32               position;

    /** Number of Sameples */
    UINT32       length;

    /** Bits per Sample */
    UINT32       bps;

}LC3_FOPS_WAV_FILE_CTX;

/* -------------------------------------------- Data Structures */


/* ------------------------- Function Declarations */

/* Wave File/Stream Interfaces */
LC3_RESULT LC3_fops_open_wav_writer
           (
               /* IN */  UINT8                  * file_name,
               /* IN */  UINT32                   sampling_rate,
               /* IN */  UINT16                   num_channels,
               /* IN */  UINT16                   bps,
               /* OUT */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           );

LC3_RESULT LC3_fops_open_wav_reader
           (
                /* IN */  UINT8                  * file_name,
                /* OUT */ UINT32                 * sampling_rate,
                /* OUT */ UINT16                 * num_channels,
                /* OUT */ UINT32                 * num_samples,
                /* OUT */ UINT16                 * bps,
                /* OUT */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           );

LC3_RESULT LC3_fops_wav_write_samples
           (
               /* IN */  LC3_FOPS_WAV_FILE_CTX  * wav_ctx,
               /* IN */  UINT32                 * samples,
               /* IN */  UINT32                   num_samples,
               /* OUT */ UINT32                 * samples_wrote
           );

LC3_RESULT LC3_fops_wav_read_samples
           (
               /* IN */   LC3_FOPS_WAV_FILE_CTX  * wav_ctx,
               /* OUT */  UINT32                 * samples,
               /* IN */   UINT32                   num_samples,
               /* OUT */  UINT32                 * samples_read
           );

LC3_RESULT LC3_fops_close_wav_writer
           (
               /* IN */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           );

/* LC3 File/Stream Interfaces */
LC3_RESULT LC3_fops_open_lc3_writer
           (
                /* IN */  UINT8                * file_name,
                /* IN */  UINT32                 sampling_rate,
                /* IN */  UINT32                 bitrate,
                /* IN */  UINT16                 num_channels,
                /* IN */  UINT32                 frame_ms,
                /* IN */  UINT32                 signal_length,
                /* OUT */ LC3_fops_file_handle * file_handle
           );

LC3_RESULT LC3_fops_open_lc3_reader
           (
                /* IN */  UINT8                * file_name,
                /* OUT */ UINT32               * sampling_rate,
                /* OUT */ UINT32               * bitrate,
                /* OUT */ UINT16               * num_channels,
                /* OUT */ UINT32               * frame_ms,
                /* OUT */ UINT32               * signal_length,
                /* OUT */ LC3_fops_file_handle * file_handle
           );

LC3_RESULT LC3_fops_lc3_write_frame
           (
               /* IN */  LC3_fops_file_handle   file_handle,
               /* IN */  UINT8                * frame,
               /* IN */  UINT32                 frame_length,
               /* OUT */ UINT32               * bytes_written
           );

LC3_RESULT LC3_fops_lc3_write_multichannel_frames
           (
               /* IN */  LC3_fops_file_handle   file_handle,
               /* IN */  UINT8               ** frames,
               /* IN */  UINT32               * frame_lengths,
               /* IN */  UINT32                 total_size,
               /* IN */  UINT16                 num_channels
           );

LC3_RESULT LC3_fops_lc3_read_frame
           (
               /* IN */   LC3_fops_file_handle   file_handle,
               /* IN */   UINT8                * frame,
               /* IN */   UINT32                 frame_length,
               /* OUT */  UINT32               * bytes_read
           );

LC3_RESULT LC3_fops_file_close
           (
               /* IN */  LC3_fops_file_handle  file_handle
           );

#endif /* _H_LC3_FOPS_ */

#endif