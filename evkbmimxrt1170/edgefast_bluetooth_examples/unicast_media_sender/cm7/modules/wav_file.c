/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wav_file.h"

#include <string.h>

typedef struct _wav_header {
    char riff[4];
    uint32_t size;
    char wave[4];
} wav_header_t;

typedef struct _chunk_header {
    char id[4];
    uint32_t size;
} chunk_header_t;

typedef struct _wav_fmt_chunk {
    uint16_t type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t bytes_per_seconds;
    uint16_t bytes_per_sample;
    uint16_t bits_per_sample;

    uint16_t _ext_size;
    uint16_t _num_of_valid_bits;
    uint32_t _speaker_position_mask;
    uint8_t  _GUID[16];
} wav_fmt_chunk_t;

static int seek_chunk(FIL *file, chunk_header_t *chunk, const char *id)
{
    chunk_header_t temp_chunk;

    FRESULT f_res;
    UINT read_bytes;

    do {
        f_res = f_read(file, &temp_chunk, sizeof(temp_chunk), &read_bytes);
        if((f_res != FR_OK) || (read_bytes != sizeof(temp_chunk)))
        {
            return -1;
        }

        if(0 != memcmp(temp_chunk.id, id, 4))
        {
            /* skip this chunk */
            f_res = f_lseek(file, f_tell(file) + temp_chunk.size);
            if(f_res != FR_OK)
            {
                return -1;
            }
            continue;
        }

        /* found */
        break;

    } while(1);

    memcpy(chunk->id, temp_chunk.id, 4);
    chunk->size = temp_chunk.size;

    return 0;
}

static int read_wav_file_info(wav_file_t *wav_file_info, FIL *file)
{
    wav_header_t header;
    chunk_header_t chunk;
    wav_fmt_chunk_t fmt;

    FRESULT f_res;
    UINT read_bytes;

    /* read wav header */
    f_res = f_read(file, &header, sizeof(header), &read_bytes);
    if((f_res != FR_OK) || (read_bytes != sizeof(header)))
    {
        return -1;
    }

    if(0 != memcmp(header.riff, "RIFF", 4))
    {
        return -1;
    }

    if(0 != memcmp(header.wave, "WAVE", 4))
    {
        return -1;
    }

    /* find fmt chunk */
    if(seek_chunk(file, &chunk, "fmt "))
    {
        return -1;
    }

    /* read fmt chunk could be 16, 18 or 40 bytes */
    if((chunk.size != 16) && (chunk.size != 18) && (chunk.size != 40))
    {
        return -1;
    }

    f_res = f_read(file, &fmt, chunk.size, &read_bytes);
    if((f_res != FR_OK) || (read_bytes != chunk.size))
    {
        return -1;
    }

    /* find data chunk */
    if(seek_chunk(file, &chunk, "data"))
    {
        return -1;
    }

    /* save fmt and data chunk */
    wav_file_info->sample_rate      = fmt.sample_rate;
    wav_file_info->channels         = fmt.channels;
    wav_file_info->bytes_per_sample = fmt.bytes_per_sample;
    wav_file_info->bits             = fmt.bits_per_sample;
    wav_file_info->size             = chunk.size;
    wav_file_info->samples          = chunk.size / fmt.bytes_per_sample;

    wav_file_info->data_offset  = f_tell(file);

    wav_file_info->remain_size    = wav_file_info->size;
    wav_file_info->remain_samples = wav_file_info->samples;

    return 0;
}

int wav_file_open(wav_file_t *wav_file, char *file_path)
{
    FIL *file;
    FRESULT f_res;
    
    file = &wav_file->fp;

    f_res = f_open(file, file_path, FA_READ);
    if(f_res != FR_OK)
    {
        return WAV_FILE_ERR;
    }

    /* Read wav file header. */
    int res = read_wav_file_info(wav_file, file);
    if(res)
    {
        f_res = f_close(file);
        memset(wav_file, 0, sizeof(wav_file_t));
        if(f_res != FR_OK)
        {
            return WAV_FILE_ERR;
        }
        return WAV_FILE_ERR;
    }

    return 0;
}

int wav_file_close(wav_file_t *wav_file)
{
    FRESULT f_res;

    f_res = f_close(&wav_file->fp);
    if(f_res != FR_OK)
    {
        return WAV_FILE_ERR;
    }

    memset(wav_file, 0, sizeof(wav_file_t));

    return 0;
}

int wav_file_read(wav_file_t *wav_file, uint8_t *output, int size)
{
    FRESULT f_res;
    UINT read_bytes;

    if(size > wav_file->remain_size)
    {
        return WAV_FILE_END;
    }

    f_res = f_read(&wav_file->fp, output, size, &read_bytes);
    if((f_res != FR_OK) || (read_bytes != size))
    {
        return WAV_FILE_ERR;
    }

    wav_file->remain_size -= size;

    return 0;
}

int wav_file_read_samples(wav_file_t *wav_file, uint8_t *output, int samples)
{
    int res;
    int size;

    wav_file->remain_samples = wav_file->remain_size / wav_file->bytes_per_sample;

    if(samples > wav_file->remain_samples)
    {
        return WAV_FILE_END;
    }

    size = samples * wav_file->bytes_per_sample;

    res = wav_file_read(wav_file, output, size);
    if(res)
    {
        return res;
    }

    wav_file->remain_samples = wav_file->remain_size / wav_file->bytes_per_sample;

    return 0;
}

int wav_file_rewind(wav_file_t *wav_file)
{
    FRESULT f_res;

    f_res = f_lseek(&wav_file->fp, wav_file->data_offset);
    if(f_res != FR_OK)
    {
        return WAV_FILE_ERR;
    }

    wav_file->remain_size    = wav_file->size;
    wav_file->remain_samples = wav_file->samples;

    return 0;
}