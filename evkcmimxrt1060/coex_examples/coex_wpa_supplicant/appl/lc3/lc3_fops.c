#ifdef LC3_TEST
/**
 *  \file LC3_fops.c
 *
 *  This file contains all OS Abstraction functions for 
 *  for wave and LC3 file operations.
 */

/*
 *  Copyright (C) 2021. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "lc3_fops.h"


/* ----------------------------------------- External Global Variables */
/* TODO: Move to LC3 features header file */
#define LC3_FOPS_BULK_WAV_WRITE

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */


/* ----------------------------------------- Functions */
/**
 *  \brief To open Wave file and write header
 *
 *  \param [in] file_name
 *              Name of LC3 file to be open in write mode
 *
 *  \param [out] sampling_rate
 *
 *  \param [out] bitrate
 *
 *  \param [out] num_channels
 *
 *  \param [out] frame_ms
 *
 *  \param [out] signal_length
 *
 *  \param [out] wav_ctx
 *
 *  \return EM_RESULT: EM_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
LC3_RESULT LC3_fops_open_wav_writer
           (
               /* IN */  UINT8                  * file_name,
               /* IN */  UINT32                   sampling_rate,
               /* IN */  UINT16                   num_channels,
               /* IN */  UINT16                   bps,
               /* OUT */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           )
{
    LC3_RESULT retval;

    /* Parameter Check */
    /* TODO: Check why 48 */
    if ((NULL == file_name) || (NULL == wav_ctx) ||
        ((0 == sampling_rate) || (192000 < sampling_rate)) ||
        ((0 == num_channels) || (48 < num_channels)) ||
        ((16 != bps) && (24 != bps) && (32 != bps)))
    {
        LC3_FOPS_ERR(
            "[FOPS] <- Invalid Parameter in Wav File Open. Returning\n");

        return EM_FAILURE;
    }

    /* Open Wave file */
    retval = EM_fops_file_open(file_name, (UINT8 *)"wb", &wav_ctx->file_handle);

    /* Check - if success */
    if (EM_SUCCESS != retval)
    {
        LC3_FOPS_ERR(
            "[FOPS] <- REQ to Open Wav File Failed. File Name = %s. Retval: 0x%04X\n",
            file_name, retval);
    }
    else
    {
        /* To write, "RIFF", "WAVE", "fmt " and "data" */
        INT8 marker[4];
        UINT16 nwrite;
        UINT32 four_octets;
        UINT16 two_octets;

        /**
         *  ## Wave File Format ##
         *
         *  Ref: http://soundfile.sapp.org/doc/WaveFormat/
         *       -----------------------------------------
         *
         *  Offset  Size  Name             Description
         *
         *  The canonical WAVE format starts with the RIFF header:
         *
         *  0         4   ChunkID          Contains the letters "RIFF" in ASCII form
         *                                 (0x52494646 big-endian form).
         *  4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
         *                                 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
         *                                 This is the size of the rest of the chunk
         *                                 following this number.  This is the size of the
         *                                 entire file in bytes minus 8 bytes for the
         *                                 two fields not included in this count:
         *                                 ChunkID and ChunkSize.
         *  8         4   Format           Contains the letters "WAVE"
         *                                 (0x57415645 big-endian form).
         *
         *  The "WAVE" format consists of two subchunks: "fmt " and "data":
         *  The "fmt " subchunk describes the sound data's format:
         *
         *  12        4   Subchunk1ID      Contains the letters "fmt "
         *                                 (0x666d7420 big-endian form).
         *  16        4   Subchunk1Size    16 for PCM.  This is the size of the
         *                                 rest of the Subchunk which follows this number.
         *  20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
         *                                 Values other than 1 indicate some
         *                                 form of compression.
         *  22        2   NumChannels      Mono = 1, Stereo = 2, etc.
         *  24        4   SampleRate       8000, 44100, etc.
         *  28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
         *  32        2   BlockAlign       == NumChannels * BitsPerSample/8
         *                                 The number of bytes for one sample including
         *                                 all channels. I wonder what happens when
         *                                 this number isn't an integer?
         *  34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
         *            2   ExtraParamSize   if PCM, then doesn't exist
         *            X   ExtraParams      space for extra parameters
         *
         *  The "data" subchunk contains the size of the data and the actual sound:
         *
         *  36        4   Subchunk2ID      Contains the letters "data"
         *                                 (0x64617461 big-endian form).
         *  40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
         *                                 This is the number of bytes in the data.
         *                                 You can also think of this as the size
         *                                 of the read of the subchunk following this
         *                                 number.
         *  44        *   Data             The actual sound data.
         */

        /* Write Header */

        /* Write ChunkID: "RIFF" */
        marker[0] = (INT8)'R';
        marker[1] = (INT8)'I';
        marker[2] = (INT8)'F';
        marker[3] = (INT8)'F';

        retval = EM_fops_file_write
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        /* Check length */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- ChunkID Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write Chunk Size */
        /* Set Maximum Value */
        four_octets = 0xFFFFFFFF;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        /* Check length */
        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Failed to write ChunkSize. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write format: "WAVE" */
        marker[0] = (INT8)'W';
        marker[1] = (INT8)'A';
        marker[2] = (INT8)'V';
        marker[3] = (INT8)'E';

        retval = EM_fops_file_write
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        /* Check length */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Format Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write Sub Chunks */

        /* Write SubChunk ID */
        marker[0] = (INT8)'f';
        marker[1] = (INT8)'m';
        marker[2] = (INT8)'t';
        marker[3] = (INT8)' ';

        retval = EM_fops_file_write
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        /* Check length */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- SubChunkID Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write SubChunkSize */
        four_octets = 16;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- SubChunkSize Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write PCM header */

        /* AudioFormat - 2 Octets */
        /* For PCM value is 1 (Linear Quantization) */
        two_octets = 1;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- AudioForamt Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* NumChannels - 2 Octets */
        two_octets = num_channels;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- NumChannels Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* SampleRate - 4 Octets */
        four_octets = sampling_rate;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- SampleRate Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* ByteRate - 4 Octets */
        four_octets = sampling_rate * num_channels * (bps / 8);
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- ByteRate Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* BlockAlign - 2 Octets */
        two_octets = num_channels * (bps / 8);
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- BlockAlign Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* BitsPerSample - 2 Octets */
        two_octets = bps;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- BitsPerSample Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write SubChunkID "data" */
        marker[0] = (INT8)'d';
        marker[1] = (INT8)'a';
        marker[2] = (INT8)'t';
        marker[3] = (INT8)'a';

        retval = EM_fops_file_write
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        /* Check length and value */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- SubChunkID Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Write SubChunkSize */
        four_octets = 0xFFFFFFFF - 36;
        retval = EM_fops_file_write
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nwrite
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nwrite))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- SubChunkSize Write Failed. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Set other fields in the Wave File Context */
        wav_ctx->position = 0; /* To be incremented during write */
        wav_ctx->bps = bps;
        wav_ctx->length = 0xFFFFFFFF - 36;   /* Set as Limit */
    }

    return retval;
}

/**
 *  \brief To open Wave file and read header
 *
 *  \param [in] file_name
 *              Name of LC3 file to be open in read mode
 *
 *  \param [out] sampling_rate
 *
 *  \param [out] bitrate
 *
 *  \param [out] num_channels
 *
 *  \param [out] frame_ms
 *
 *  \param [out] signal_length
 *
 *  \param [out] wav_ctx
 *
 *  \return EM_RESULT: EM_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
LC3_RESULT LC3_fops_open_wav_reader
           (
                /* IN */  UINT8                  * file_name,
                /* OUT */ UINT32                 * sampling_rate,
                /* OUT */ UINT16                 * num_channels,
                /* OUT */ UINT32                 * num_samples,
                /* OUT */ UINT16                 * bps,
                /* OUT */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           )
{
    LC3_RESULT retval;

    /* Parameter Check */
    if ((NULL == file_name) || (NULL == sampling_rate) || (NULL == num_channels) ||
        (NULL == num_samples) || (NULL == bps) || (NULL == wav_ctx))
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Invalid Parameter in Wav File Open. Returning\n");

        return EM_FAILURE;
    }

    /* Open Wave file */
    retval = EM_fops_file_open(file_name, (UINT8 *)"rb", &wav_ctx->file_handle);

    /* Check - if success */
    if (EM_SUCCESS != retval)
    {
        LC3_FOPS_ERR(
        "[FOPS] <- REQ to Open Wav File Failed. File Name = %s. Retval: 0x%04X\n",
        file_name, retval);
    }
    else
    {
        /* To read, "RIFF", "WAVE", "fmt " and "data" */
        INT8 marker[4];
        UINT16 nread;
        UINT32 four_octets;
        UINT16 two_octets;
        UINT32 fmt_chunksize, extra_octets, index;
        UINT8  dummy_octet;
        UINT16 audio_format;

        /**
         *  ## Wave File Format ##
         *
         *  Ref: http://soundfile.sapp.org/doc/WaveFormat/
         *       -----------------------------------------
         *
         *  Offset  Size  Name             Description
         *  
         *  The canonical WAVE format starts with the RIFF header:
         *  
         *  0         4   ChunkID          Contains the letters "RIFF" in ASCII form
         *                                 (0x52494646 big-endian form).
         *  4         4   ChunkSize        36 + SubChunk2Size, or more precisely:
         *                                 4 + (8 + SubChunk1Size) + (8 + SubChunk2Size)
         *                                 This is the size of the rest of the chunk 
         *                                 following this number.  This is the size of the 
         *                                 entire file in bytes minus 8 bytes for the
         *                                 two fields not included in this count:
         *                                 ChunkID and ChunkSize.
         *  8         4   Format           Contains the letters "WAVE"
         *                                 (0x57415645 big-endian form).
         *  
         *  The "WAVE" format consists of two subchunks: "fmt " and "data":
         *  The "fmt " subchunk describes the sound data's format:
         *  
         *  12        4   Subchunk1ID      Contains the letters "fmt "
         *                                 (0x666d7420 big-endian form).
         *  16        4   Subchunk1Size    16 for PCM.  This is the size of the
         *                                 rest of the Subchunk which follows this number.
         *  20        2   AudioFormat      PCM = 1 (i.e. Linear quantization)
         *                                 Values other than 1 indicate some 
         *                                 form of compression.
         *  22        2   NumChannels      Mono = 1, Stereo = 2, etc.
         *  24        4   SampleRate       8000, 44100, etc.
         *  28        4   ByteRate         == SampleRate * NumChannels * BitsPerSample/8
         *  32        2   BlockAlign       == NumChannels * BitsPerSample/8
         *                                 The number of bytes for one sample including
         *                                 all channels. I wonder what happens when
         *                                 this number isn't an integer?
         *  34        2   BitsPerSample    8 bits = 8, 16 bits = 16, etc.
         *            2   ExtraParamSize   if PCM, then doesn't exist
         *            X   ExtraParams      space for extra parameters
         *  
         *  The "data" subchunk contains the size of the data and the actual sound:
         *  
         *  36        4   Subchunk2ID      Contains the letters "data"
         *                                 (0x64617461 big-endian form).
         *  40        4   Subchunk2Size    == NumSamples * NumChannels * BitsPerSample/8
         *                                 This is the number of bytes in the data.
         *                                 You can also think of this as the size
         *                                 of the read of the subchunk following this 
         *                                 number.
         *  44        *   Data             The actual sound data.
         */

        /* Read Header */

        /* Read ChunkID: "RIFF" */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nread
                 );

        /* Check length and value */
        if ((sizeof(marker) != nread) || (0 != EM_str_n_cmp(marker, "RIFF", nread)))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing ChunkID. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Read Chunk Size */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        /* Check length */
        if (sizeof(four_octets) != nread)
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing ChunkSize. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Read format: "WAVE" */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nread
                 );

        /* Check length and value */
        if ((sizeof(marker) != nread) || (0 != EM_str_n_cmp(marker, "WAVE", nread)))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing Format. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Read Sub Chunks */

        /* Read SubChunk ID */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nread
                 );

        /* Check length and value */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing SubChunkID. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Skip till SubChunkID is "fmt " */
        while (0 != EM_str_n_cmp(marker, "fmt ", sizeof(marker)))
        {
            UINT8 dummy_read;

            /* Read SubChunkSize and Skip */
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&four_octets,
                         (UINT16)sizeof(four_octets),
                         wav_ctx->file_handle,
                         &nread
                     );

            if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nread))
            {
                LC3_FOPS_ERR(
                "[FOPS] <- Invalid or missing SubChunkSize. Returning\n");

                /* TODO: Not closing the file */
                return EM_FAILURE;
            }

            /* Skip */
            while (0 != four_octets)
            {
                retval = EM_fops_file_read
                         (
                             (UINT8 *)&dummy_read,
                             (UINT16)sizeof(dummy_read),
                             wav_ctx->file_handle,
                             &nread
                         );

                if ((EM_SUCCESS != retval) || (sizeof(dummy_read) != nread))
                {
                    LC3_FOPS_ERR(
                    "[FOPS] <- Chunk Read Failed. Returning\n");
    
                    /* TODO: Not closing the file */
                    return EM_FAILURE;
                }

                four_octets--;
            }

            /* Read SubChunk ID - again and check */
            retval = EM_fops_file_read
                     (
                         (UINT8 *)marker,
                         (UINT16)sizeof(marker),
                         wav_ctx->file_handle,
                         &nread
                     );
    
            /* Check length and value */
            if ((EM_SUCCESS != retval) || (sizeof(marker) != nread))
            {
                LC3_FOPS_ERR(
                "[FOPS] <- Invalid or missing SubChunkID. Returning\n");
    
                /* TODO: Not closing the file */
                return EM_FAILURE;
            }
        }
        
        /* SubChukID is now "fmt " */

        /* Read SubChunkSize */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&fmt_chunksize,
                     (UINT16)sizeof(fmt_chunksize),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(fmt_chunksize) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing SubChunkSize. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* For PCM SubChukSize shall be 16 */
        /* TODO: Define PCM SubChuk Size as 16 */
        if (16 > fmt_chunksize)
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid SubChunkSize for PCM. Expecting: 16 or more. Received: %d. Returning\n",
            fmt_chunksize);

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Read PCM header */

        /* AudioFormat - 2 Octets */
        /* For PCM value is 1 (Linear Quantization) */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing AudioForamt. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

#if 0
        /* For PCM Audio Format shall be 1 */
        /* TODO: Define PCM Audio Format as 1 */
        if (1 != two_octets)
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid Audio Format for PCM. Expecting: 1. Received: %d. Returning\n",
            two_octets);

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }
#endif /* 0 */
        /* Save Audio Format */
        audio_format = two_octets;

        /* NumChannels - 2 Octets */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing NumChannels. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        *num_channels = two_octets;

        /* SampleRate - 4 Octets */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing SampleRate. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        *sampling_rate = four_octets;

        /* ByteRate - 4 Octets */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing ByteRate. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* BlockAlign - 2 Octets */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing BlockAlign. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* BitsPerSample - 2 Octets */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&two_octets,
                     (UINT16)sizeof(two_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(two_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing BitsPerSample. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        *bps = two_octets;

        /* Special handling if Audio Format is -2 (0xFFFE) */
        if (0xFFFE == audio_format)
        {
            /* Skip channel mask */
            EM_fops_file_seek(wav_ctx->file_handle, 8, EM_FOPS_SEEK_CUR);

            /* Read Audio Format, part of GUID */
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&two_octets,
                         (UINT16)sizeof(two_octets),
                         wav_ctx->file_handle,
                         &nread
                     );

            audio_format = two_octets;

            /* Skip remaining GUID */
            EM_fops_file_seek(wav_ctx->file_handle, 14, EM_FOPS_SEEK_CUR);
            extra_octets = fmt_chunksize - 40;
        }
        else
        {
            extra_octets = fmt_chunksize - 16;
        }

        /* Expected BitsPerSample is 16 or 24 or 32 */
        if ((16 != (*bps)) && (24 != (*bps)) && (32 != (*bps)))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid BitsPerSample. Expecting: 16/24/32. Received: %d. Returning\n",
            two_octets);

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* For PCM Audio Format shall be 1 */
        /* TODO: Define PCM Audio Format as 1 */
        if (1 != audio_format)
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid Audio Format for PCM. Expecting: 1. Received: %d. Returning\n",
            audio_format);

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /**
         * Note: Skip extra octets in subchunk before reading "data"
         * that follows "fmt ". And for format PCM, 16 octet
         * chunk size is checked and all those fields are already read.
         */
        /* Read the extra octets */
        for (index = 0; index < extra_octets; index++)
        {
            retval = EM_fops_file_read
                        (
                            &dummy_octet,
                            (UINT16)sizeof(dummy_octet),
                            wav_ctx->file_handle,
                            &nread
                        );

            /* TODO: Not checking return value */
        }

        /* Read SubChunkID "data" */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)marker,
                     (UINT16)sizeof(marker),
                     wav_ctx->file_handle,
                     &nread
                 );

        /* Check length */
        if ((EM_SUCCESS != retval) || (sizeof(marker) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing SubChunkID. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Skip till SubChunkID is "data" */
        while (0 != EM_str_n_cmp(marker, "data", sizeof(marker)))
        {
            UINT8 dummy_read;

            /* Read SubChunkSize and Skip */
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&four_octets,
                         (UINT16)sizeof(four_octets),
                         wav_ctx->file_handle,
                         &nread
                     );

            if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nread))
            {
                LC3_FOPS_ERR(
                "[FOPS] <- Invalid or missing SubChunkSize. Returning\n");

                /* TODO: Not closing the file */
                return EM_FAILURE;
            }

            /* Skip */
            while (0 != four_octets)
            {
                retval = EM_fops_file_read
                         (
                             (UINT8 *)&dummy_read,
                             (UINT16)sizeof(dummy_read),
                             wav_ctx->file_handle,
                             &nread
                         );

                if ((EM_SUCCESS != retval) || (sizeof(dummy_read) != nread))
                {
                    LC3_FOPS_ERR(
                    "[FOPS] <- Chunk Read Failed. Returning\n");
    
                    /* TODO: Not closing the file */
                    return EM_FAILURE;
                }

                four_octets--;
            }

            /* Read SubChunk ID - again and check */
            retval = EM_fops_file_read
                     (
                         (UINT8 *)marker,
                         (UINT16)sizeof(marker),
                         wav_ctx->file_handle,
                         &nread
                     );
    
            /* Check length and value */
            if ((EM_SUCCESS != retval) || (sizeof(marker) != nread))
            {
                LC3_FOPS_ERR(
                "[FOPS] <- Invalid or missing SubChunkID. Returning\n");
    
                /* TODO: Not closing the file */
                return EM_FAILURE;
            }
        }

        /* Read SubChunkSize for "data" */
        retval = EM_fops_file_read
                 (
                     (UINT8 *)&four_octets,
                     (UINT16)sizeof(four_octets),
                     wav_ctx->file_handle,
                     &nread
                 );

        if ((EM_SUCCESS != retval) || (sizeof(four_octets) != nread))
        {
            LC3_FOPS_ERR(
            "[FOPS] <- Invalid or missing SubChunkSize. Returning\n");

            /* TODO: Not closing the file */
            return EM_FAILURE;
        }

        /* Number of Samples */
        *num_samples = four_octets / (((*bps) + 7) / 8);

        /* Set other fields in the Wave File Context */
        wav_ctx->position = 0;
        wav_ctx->bps = *bps;
        wav_ctx->length = *num_samples;

        /* Samples per Channel */
        *num_samples /= (*num_channels);
    }

    return retval;
}

/* TODO: Add Header */
LC3_RESULT LC3_fops_wav_write_samples
           (
               /* IN */  LC3_FOPS_WAV_FILE_CTX  * wav_ctx,
               /* IN */  UINT32                 * samples,
               /* IN */  UINT32                   num_samples,
               /* OUT */ UINT32                 * samples_wrote
           )
{
    LC3_RESULT retval;
    UINT16 nwrote, two_octets;
    UINT32 samples_remaining, index;
    UINT32 four_octets;

    /* For bulk write allocate memory, to be thread safe */
#ifdef LC3_FOPS_BULK_WAV_WRITE
    UINT8 * write_buffer;
    UINT16  write_buffer_len, marker;
#endif /* LC3_FOPS_BULK_WAV_WRITE */

    /* Parameter Check */
    if ((NULL == wav_ctx) || (NULL == wav_ctx->file_handle) || (NULL == samples) ||
        (0 == num_samples) || (NULL == samples_wrote))
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Invalid Parameter in Wav Read. Returning\n");

        return EM_FAILURE;
    }

    /* allocate memory */
#ifdef LC3_FOPS_BULK_WAV_WRITE
    write_buffer_len = num_samples * (wav_ctx->bps >> 3);
    write_buffer = EM_alloc_mem(write_buffer_len);

    if (NULL == write_buffer)
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Failed to allocate memory for Bulk Write. Returning\n");

        return EM_FAILURE;
    }

    marker = 0;
#endif /* LC3_FOPS_BULK_WAV_WRITE */

    /* Check if requested number of samples are still remaining */
    samples_remaining = wav_ctx->length - wav_ctx->position;
    if (num_samples > samples_remaining)
    {
        num_samples = samples_remaining;
    }

    /* Write Samples */
    for (index = 0; index < num_samples; index++)
    {
        switch (wav_ctx->bps)
        {
        case 16:
        {
            if (32767 < (INT32)samples[index])
            {
                two_octets = (UINT16)32767;
            }
            else if (-32767 > (INT32)samples[index])
            {
                two_octets = (UINT16)(-32767);
            }
            else
            {
                two_octets = (UINT16)samples[index];
            }

#ifndef LC3_FOPS_BULK_WAV_WRITE
            retval = EM_fops_file_write
                     (
                         (UINT8 *)&two_octets,
                         (UINT16)sizeof(two_octets),
                         wav_ctx->file_handle,
                         &nwrote
                     );
#else
            /* Little Endian Packing */
            write_buffer[marker] = (UINT8)two_octets;
            marker++;
            write_buffer[marker] = (UINT8)(two_octets >> 8);
            marker++;
#endif /* LC3_FOPS_BULK_WAV_WRITE */
        }
        break;

        case 24:
        {
            if (8388607 < (INT32)samples[index])
            {
                four_octets = (8388607);
            }
            else if (-8388608 > (INT32)samples[index])
            {
                four_octets = (UINT32)(-8388608);
            }
            else
            {
                four_octets = (UINT32)samples[index];
            }

#ifndef LC3_FOPS_BULK_WAV_WRITE
            retval = EM_fops_file_write
                     (
                         (UINT8 *)&four_octets,
                         (UINT16)3,
                         wav_ctx->file_handle,
                         &nwrote
                     );
#else
            /* Little Endian Packing */
            write_buffer[marker] = (UINT8)four_octets;
            marker++;
            write_buffer[marker] = (UINT8)(four_octets >> 8);
            marker++;
            write_buffer[marker] = (UINT8)(four_octets >> 16);
            marker++;
#endif /* LC3_FOPS_BULK_WAV_WRITE */
        }
        break;

        case 32:
        {
#ifndef LC3_FOPS_BULK_WAV_WRITE
            retval = EM_fops_file_write
                     (
                         (UINT8 *)&samples[index],
                         (UINT16)4,
                         wav_ctx->file_handle,
                         &nwrote
                     );
#else
            /* Little Endian Packing */
            write_buffer[marker] = (UINT8)samples[index];
            marker++;
            write_buffer[marker] = (UINT8)(samples[index] >> 8);
            marker++;
            write_buffer[marker] = (UINT8)(samples[index] >> 16);
            marker++;
            write_buffer[marker] = (UINT8)(samples[index] >> 24);
            marker++;
#endif /* LC3_FOPS_BULK_WAV_WRITE */
        }
        break;

        default:
            /* Should not reach here! */
            return EM_FAILURE;
        }

#ifndef LC3_FOPS_BULK_WAV_WRITE
        if (EM_SUCCESS == retval)
        {
            wav_ctx->position += (wav_ctx->bps / 8);
        }
        else
        {
            return EM_FAILURE;
        }
#endif /* LC3_FOPS_BULK_WAV_WRITE */
    }

#ifdef LC3_FOPS_BULK_WAV_WRITE
    retval = EM_fops_file_write
             (
                 write_buffer,
                 write_buffer_len,
                 wav_ctx->file_handle,
                 &nwrote
             );

    wav_ctx->position += write_buffer_len;
    EM_free_mem(write_buffer);
#endif /* LC3_FOPS_BULK_WAV_WRITE */

    *samples_wrote = index;

#ifdef LC3_FOPS_BULK_WAV_WRITE
    return retval;
#else
    return EM_SUCCESS;
#endif /* LC3_FOPS_BULK_WAV_WRITE */
}


/* TODO: Add Header */
LC3_RESULT LC3_fops_wav_read_samples
           (
               /* IN */  LC3_FOPS_WAV_FILE_CTX  * wav_ctx,
               /* OUT */ UINT32                 * samples,
               /* IN */  UINT32                   num_samples,
               /* OUT */ UINT32                 * samples_read
           )
{
    LC3_RESULT retval;
    UINT16 nread, two_octets;
    UINT32 samples_remaining, index;

    /* Parameter Check */
    if ((NULL == wav_ctx) || (NULL == wav_ctx->file_handle) || (NULL == samples) ||
        (0 == num_samples) || (NULL == samples_read))
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Invalid Parameter in Wav Read. Returning\n");

        return EM_FAILURE;
    }

    /* Check if requested number of samples are still remaining */
    samples_remaining = wav_ctx->length - wav_ctx->position;
    if (num_samples > samples_remaining)
    {
        num_samples = samples_remaining;
    }

    /* Read Samples */
    for (index = 0; index < num_samples; index++)
    {
        switch (wav_ctx->bps)
        {
        case 16:
        {
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&two_octets,
                         (UINT16)sizeof(two_octets),
                         wav_ctx->file_handle,
                         &nread
                     );

            /* Assigning without retval check */
            samples[index] = two_octets;
        }
        break;

        case 24:
        {
            INT32  three_octets;
            three_octets = 0;
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&three_octets,
                         (UINT16)3,
                         wav_ctx->file_handle,
                         &nread
                     );

            if (three_octets >= 0x800000)
            {
                three_octets |= 0xFF000000;
            }

            samples[index] = three_octets;
        }
        break;

        case 32:
        {
            retval = EM_fops_file_read
                     (
                         (UINT8 *)&samples[index],
                         (UINT16)4,
                         wav_ctx->file_handle,
                         &nread
                     );
        }
        break;

        default:
            /* Should not reach here! */
            return EM_FAILURE;
        }

        if (EM_SUCCESS == retval)
        {
            wav_ctx->position++;
        }
        else
        {
            return EM_FAILURE;
        }
    }

    *samples_read = index;

    return EM_SUCCESS;
}


/**
 *  \brief To close Wave file
 *
 *  \param [in] wav_ctx
 *
 *  \return EM_RESULT: EM_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
LC3_RESULT LC3_fops_close_wav_writer
           (
               /* IN */ LC3_FOPS_WAV_FILE_CTX  * wav_ctx
           )
{
    UINT32 chunk_size;
    UINT16 nwrote;
    LC3_RESULT retval;

    /* Parameter Check */
    if ((NULL == wav_ctx) || (NULL == wav_ctx->file_handle))
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Invalid Parameter in Wav Close. Returning\n");

        return EM_FAILURE;
    }

    /* Update Chunk Size */
    EM_fops_file_seek(wav_ctx->file_handle, 4, EM_FOPS_SEEK_SET);

    chunk_size = wav_ctx->position + 36;

    retval = EM_fops_file_write
             (
                 (UINT8 *)&chunk_size,
                 (UINT16)sizeof(chunk_size),
                 wav_ctx->file_handle,
                 &nwrote
             );
    /* TODO: Not checking return value */

    /* Update Data SubChunk Size */
    EM_fops_file_seek(wav_ctx->file_handle, 4 + 36, EM_FOPS_SEEK_SET);

    chunk_size = wav_ctx->position;

    retval = EM_fops_file_write
             (
                 (UINT8 *)&chunk_size,
                 (UINT16)sizeof(chunk_size),
                 wav_ctx->file_handle,
                 &nwrote
             );
    /* TODO: Not checking return value */

    retval = EM_fops_file_close(wav_ctx->file_handle);

    return retval;
}

/**
 *  \brief To open LC3 file and write header
 *
 *  \param [in] file_name
 *              Name of LC3 file to be open in write mode
 *
 *  \param [in] sampling_rate
 *
 *  \param [in] bitrate
 *
 *  \param [in] num_channels
 *
 *  \param [in] frame_ms
 *
 *  \param [in] signal_length
 *
 *  \param [out] file_handle
 *
 *  \return EM_RESULT: EM_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
LC3_RESULT LC3_fops_open_lc3_writer
           (
                /* IN */  UINT8                * file_name,
                /* IN */  UINT32                 sampling_rate,
                /* IN */  UINT32                 bitrate,
                /* IN */  UINT16                 num_channels,
                /* IN */  UINT32                 frame_ms,
                /* IN */  UINT32                 signal_length,
                /* OUT */ LC3_fops_file_handle * file_handle
           )
{
    LC3_RESULT retval;

    retval = EM_fops_file_open(file_name, (UINT8 *)"wb", file_handle);

    /* Check */
    if (EM_SUCCESS != retval)
    {
        LC3_FOPS_TRC(
        "[FOPS] <- REQ to Open LC3 File. File Name = %s\n",
        file_name);
    }
    else
    {
        /* Write Header */
        UINT16 lc3_header[9];
        UINT16 bytes_written;

        /**
         *   ## LC3 File Header Format ##
         *
         *    Byte 0       1         2       3        4        5
         *    +--------+--------+--------+--------+--------+--------+
         *    |     0xCC1C      |   Header Size   |  Sampling Rate  |
         *    +--------+--------+--------+--------+--------+--------+
         *    |    Bitrate      |   Num Channels  |  Frames in dms  |
         *    +--------+--------+--------+--------+--------+--------+
         *    |    EP Mode      |           Signal Legnth           |
         *    +--------+--------+--------+--------+--------+--------+
         */
        lc3_header[0] = 0xCC1C; /* Magic Number */
        lc3_header[1] = (UINT16)sizeof(lc3_header);
        lc3_header[2] = (UINT16)(sampling_rate / 100);
        lc3_header[3] = (UINT16)(bitrate / 100);
        lc3_header[4] = (UINT16)num_channels;
        lc3_header[5] = (UINT16)(frame_ms * 10);
        lc3_header[6] = (UINT16)0; /* epmode */
        lc3_header[7] = (UINT16)signal_length;
        lc3_header[8] = (UINT16)(signal_length >> 16);

        /* TODO: endianness issue. UINT16[] used in place of UCHAR */
        retval = EM_fops_file_write
                 (
                     (UINT8 *)lc3_header,
                     sizeof(lc3_header),
                     *file_handle,
                     &bytes_written
                 );
    }

    return retval;
}

/**
 *  \brief To open LC3 file and read header
 *
 *  \param [in] file_name
 *              Name of LC3 file to be open in read mode
 *
 *  \param [out] sampling_rate
 *
 *  \param [out] bitrate
 *
 *  \param [out] num_channels
 *
 *  \param [out] frame_ms
 *
 *  \param [out] signal_length
 *
 *  \param [out] file_handle
 *
 *  \return EM_RESULT: EM_SUCCESS on success otherwise an error code
 *                      describing the cause of failure.
 */
LC3_RESULT LC3_fops_open_lc3_reader
           (
                /* IN */  UINT8                * file_name,
                /* OUT */ UINT32               * sampling_rate,
                /* OUT */ UINT32               * bitrate,
                /* OUT */ UINT16               * num_channels,
                /* OUT */ UINT32               * frame_ms,
                /* OUT */ UINT32               * signal_length,
                /* OUT */ LC3_fops_file_handle * file_handle
           )
{
    LC3_RESULT retval;

    /* Parameter Check */
    if ((NULL == file_name) || (NULL == sampling_rate) || (NULL == bitrate) ||
        (NULL == num_channels) || (NULL == frame_ms) || (NULL == signal_length) || (NULL == file_handle))
    {
        LC3_FOPS_ERR(
        "[FOPS] <- Invalid Parameter in LC3 File Open. Returning\n");

        return EM_FAILURE;
    }

    /* Open LC3 file */
    retval = EM_fops_file_open(file_name, (UINT8 *)"rb", file_handle);

    /* Check - if success */
    if (EM_SUCCESS != retval)
    {
        LC3_FOPS_ERR(
        "[FOPS] <- REQ to Open LC3 File Failed. File Name = %s. Retval: 0x%04X\n",
        file_name, retval);
    }
    else
    {
        /* Read Header */
        /* TODO: Why 10 and not 9? */
        UINT16 lc3_header[10] = { 0 };
        UINT16 nbytes;

        retval = EM_fops_file_read
                 (
                     (UINT8 *)lc3_header,
                     (UINT16)sizeof(lc3_header),
                     *file_handle,
                     &nbytes
                 );

        /**
         *   ## LC3 File Header Format ##
         *
         *    Byte 0       1         2       3        4        5
         *    +--------+--------+--------+--------+--------+--------+
         *    |     0xCC1C      |   Header Size   |  Sampling Rate  |
         *    +--------+--------+--------+--------+--------+--------+
         *    |    Bitrate      |   Num Channels  |  Frames in dms  |
         *    +--------+--------+--------+--------+--------+--------+
         *    |    EP Mode      |           Signal Legnth           |
         *    +--------+--------+--------+--------+--------+--------+
         */

        /* TODO: Take care of the endianness */
        if (EM_SUCCESS == retval)
        {
            /* Length Check */
            if (lc3_header[1] >= 18)
            {
                *sampling_rate = (UINT32)(lc3_header[2] * 100);
                *bitrate = (UINT32)(lc3_header[3] * 100);
                *num_channels = lc3_header[4];
                *frame_ms = (UINT32)(lc3_header[5] / 10);
                *signal_length = (UINT32)lc3_header[7] | ((UINT32)lc3_header[8] << 16);
                EM_fops_file_seek(*file_handle, lc3_header[1], EM_FOPS_SEEK_SET);
            }
            else
            {
                retval = EM_FAILURE;
            }
        }
    }

    return retval;
}

#if 0
LC3_RESULT LC3_fops_lc3_write_frame
           (
               /* IN */  LC3_fops_file_handle   file_handle,
               /* IN */  UINT8                * frame,
               /* IN */  UINT32                 frame_length,
               /* OUT */ UINT32               * bytes_written
           )
{
    /* Empty */
}
#endif /* 0 */

/* TODO: Add Header */
LC3_RESULT LC3_fops_lc3_write_multichannel_frames
           (
               /* IN */  LC3_fops_file_handle   file_handle,
               /* IN */  UINT8               ** frames,
               /* IN */  UINT32               * frame_lengths,
               /* IN */  UINT32                 total_size,
               /* IN */  UINT16                 num_channels
           )
{
    LC3_RESULT retval;
    UINT16 nbytes, nwritten;
    UINT16  i;
    UINT8  *bytes;

    nbytes = (UINT16)total_size;

    retval = EM_fops_file_write
             (
                 (UINT8 *)&nbytes,
                 (UINT16)sizeof(nbytes),
                 file_handle,
                 &nwritten
             );

    if (EM_SUCCESS == retval)
    {
        for (i = 0; i < num_channels; i++)
        {
            nbytes = (UINT16)frame_lengths[i];
            bytes = (UINT8*)frames[i];

            retval = EM_fops_file_write
                     (
                         bytes,
                         nbytes,
                         file_handle,
                         &nwritten
                     );

            /* TODO: Check retval */
        }
    }

    return retval;
}

/* TODO: Add Header */
LC3_RESULT LC3_fops_lc3_read_frame
           (
               /* IN */   LC3_fops_file_handle   file_handle,
               /* IN */   UINT8                * frame,
               /* IN */   UINT32                 frame_length,
               /* OUT */  UINT32               * bytes_read
           )
{
    LC3_RESULT retval;
    UINT16 nbytes, nread;

    retval = EM_fops_file_read
             (
                 (UINT8 *)&nbytes,
                 (UINT16)sizeof(nbytes),
                 file_handle,
                 &nread
             );

    if (EM_SUCCESS == retval)
    {
        /* Adjust read size */
        if (frame_length < nbytes)
        {
            nbytes = (UINT16)frame_length;
        }

        /* TODO: Check nbytes is not zero */
        retval = EM_fops_file_read
                 (
                     frame,
                     (UINT16)nbytes,
                     file_handle,
                     &nbytes
                 );

        if (EM_SUCCESS == retval)
        {
            *bytes_read = (UINT32)nbytes;
        }
    }

    return retval;
}

/* TODO: Add Header */
LC3_RESULT LC3_fops_file_close
           (
               /* IN */  LC3_fops_file_handle  file_handle
           )
{
    return EM_fops_file_close(file_handle);
}

#endif