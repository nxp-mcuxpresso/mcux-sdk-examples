/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LC3_CODEC_H_
#define __LC3_CODEC_H_

#include <stdint.h>

#include "LC3_api.h"

#define LC3_CODEC_ERR   -1

typedef struct _lc3_encoder_t {
    int sample_rate;  /* 8000Hz, 16000Hz, 24000Hz, 32000Hz, 44100Hz, 48000Hz. */
    int duration_us;  /* 10000us or 7500us */
    int enc_bytes;    /* output bytes of after encode. */
    int sample_bits;  /* 8bits, 16bits, 24bits, 32bits. */

    LC3_ENCODER_CNTX lc3_ctx;
    int32_t buf_in[LC3_INPUT_FRAME_SIZE_MAX];
    uint8_t buf_out[LC3_FRAME_SIZE_MAX];
    uint8_t core_buff[LC3_ENCODER_CORE_BUFFER_SIZE_MAX];
    uint8_t work_buff[LC3_ENCODER_WORK_BUFFER_SIZE_MAX];
} lc3_encoder_t;

typedef struct _lc3_decoder_t {
    int sample_rate;  /* 8000Hz, 16000Hz, 24000Hz, 32000Hz, 44100Hz, 48000Hz. */
    int duration_us;  /* 10000us or 7500us */
    int enc_bytes;    /* input bytes before decode. */
    int sample_bits;  /* 8bits, 16bits, 24bits, 32bits. */

    LC3_DECODER_CNTX lc3_ctx;
    uint8_t buf_in[LC3_FRAME_SIZE_MAX];
    int32_t buf_out[LC3_INPUT_FRAME_SIZE_MAX];
    uint8_t core_buff[LC3_DECODER_CORE_BUFFER_SIZE_MAX];
    uint8_t work_buff[LC3_DECODER_WORK_BUFFER_SIZE_MAX];
} lc3_decoder_t;

#define LC3_FRAME_FLAG_GOOD       0
#define LC3_FRAME_FLAG_BAD        1
#define LC3_FRAME_FLAG_REDUNDANCY 2

#define LC3_SAMPLES_PER_FRAME(encoder)              ((encoder->sample_rate != 44100) ? (encoder->sample_rate * (encoder->duration_us / 100) / 10000) : ((encoder->duration_us == 10000) ? (480) : (360)))
#define LC3_SAMPLES_BYTES_PER_FRAME(encoder)        (LC3_SAMPLES_PER_FRAME(encoder) * (encoder->sample_bits / 8))
#define LC3_ENC_BYTES_PER_FRAME(encoder_decoder)    (encoder_decoder->enc_bytes)

int lc3_encoder_init(lc3_encoder_t *encoder, int sample_rate, int duration_us, int target_bytes, int sample_bits);
int lc3_encoder(lc3_encoder_t *encoder, void *input, uint8_t *output);
int lc3_encoder_deinit(lc3_encoder_t *encoder);

int lc3_decoder_init(lc3_decoder_t *decoder, int sample_rate, int duration_us, int input_bytes, int sample_bits);
int lc3_decoder(lc3_decoder_t *decoder, uint8_t *input, int frame_flag, void *output);
int lc3_decoder_deinit(lc3_decoder_t *decoder);

#endif /* __LC3_CODEC_H_ */