/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AUDIO_I2S_H_
#define __AUDIO_I2S_H_

#include <stdint.h>
#include <stdbool.h>

#ifndef AUDIO_I2S_BUFF_COUNT
#define AUDIO_I2S_BUFF_COUNT 10
#endif

#ifndef AUDIO_I2S_BUFF_SIZE
#define AUDIO_I2S_BUFF_SIZE  4096 /* 2ch * 512samples * 4bytes */
#endif

#define AUDIO_I2S_MODE_TX    0x01

typedef void (*audio_i2s_tx_callback_t)(int err);

int audio_i2s_init(int sample_rate, int channels, int bits, int mode);
int audio_i2s_install_callback(audio_i2s_tx_callback_t tx_callback);
int audio_i2s_deinit(void);
int audio_i2s_write(uint8_t *data, int len);
int audio_i2s_start(void);
int audio_i2s_stop(void);

bool audio_i2s_is_working(void);

#endif /* __AUDIO_I2S_H_ */