/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __WAV_H__
#define __WAV_H__

#include "BT_common.h"
#include "ff.h"

typedef struct _wav {
		char ChunkID[4];
	uint32_t ChunkSize;
		char Format[4];
		char Subchunk1ID[4];
	uint32_t Subchunk1Size;
	uint16_t AudioFormat;
	uint16_t NumChannels;
	uint32_t SampleRate;
	uint32_t ByteRate;
	uint16_t BlockAlign;
	uint16_t BitsPerSample;
		char Subchunk2ID[4];
	uint32_t Subchunk2Size;
} WAV;

int readWavInfo(WAV *wav, FIL *file);
int createWavFile(FIL *file);
void closeWavFile(FIL *file, UINT32 length);


#endif /* __WAV_H__ */
