/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "wav.h"

typedef struct _fact_chunk{
	    CHAR		ChunkID[4];
       INT32		ChunkSize;
       INT32     SampleLength;
}WAV_fact_chunk;

int readWavInfo(WAV *wav, FIL *file)
{
	INT16 ExtraParamSize;
    WAV_fact_chunk fact_chunk;
    UINT8 extension_chunk_22[22];
    UINT readbytes;

    // read first 36 bytes
    f_read(file, (void *)wav, 36, &readbytes);

    switch(wav->Subchunk1Size)
    {
        case 16: // PCM
            break;
        case 18: // Non-PCM
            // Size of the extension: 0
            f_read(file, (void *)&ExtraParamSize, 2, &readbytes);
            if (ExtraParamSize == 0)
            {
                // read fact chunk
                f_read(file, (void *)&fact_chunk, sizeof(fact_chunk), &readbytes);
            }
            else
            {
                return -1;
            }
            break;
        case 40: // Extensible Format
            // Size of the extension: 22
            f_read(file, (void *)&ExtraParamSize, 2, &readbytes);
            if (ExtraParamSize == 22)
            {
                // read extension chunk 22
                f_read(file, (void *)&extension_chunk_22, sizeof(extension_chunk_22), &readbytes);
                // read fact chunk
                f_read(file, (void *)&fact_chunk, sizeof(fact_chunk), &readbytes);
            }
            else
            {
                return -2;
            }
            break;
        default:
            return -3;
    }

    // read data chunk
    f_read(file, (void *)&wav->Subchunk2ID, 8, &readbytes);

    return 0;
}