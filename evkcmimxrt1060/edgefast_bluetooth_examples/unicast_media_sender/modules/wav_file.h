/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __WAV_FILE_H__
#define __WAV_FILE_H__

#include "ff.h"

typedef struct _wav_file {
	int sample_rate;
	int channels;
	int bytes_per_sample;
	int bits;			/* 8bits, 16bits, 24bits, 32bits */
	uint32_t size;
	uint32_t samples;

	uint32_t remain_size; /* how many bytes remain */
	uint32_t remain_samples; /* how many samples remain */

	FIL     fp;
	FSIZE_t data_offset; /* where the data chunk PCM data start */
} wav_file_t;

#define WAV_FILE_ERR 	-1
#define WAV_FILE_END    -2 

int wav_file_open(wav_file_t *wav_file, char *file_path);
int wav_file_close(wav_file_t *wav_file);
int wav_file_read(wav_file_t *wav_file, uint8_t *output, int size);
int wav_file_read_samples(wav_file_t *wav_file, uint8_t *output, int samples);
int wav_file_rewind(wav_file_t *wav_file);

#endif /* __WAV_FILE_H__ */