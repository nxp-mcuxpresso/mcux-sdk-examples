/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __HW_CODEC_H_
#define __HW_CODEC_H_

#include <stdint.h>

#define HW_CODEC_ERROR -1

int hw_codec_init(int sample_rate, int channels, int bits);
int hw_codec_deinit(void);

int hw_codec_vol_get(void);
int hw_codec_vol_set_step(uint8_t step);
int hw_codec_vol_set(uint8_t vol);
int hw_codec_vol_up(void);
int hw_codec_vol_down(void);
int hw_codec_mute(void);
int hw_codec_unmute(void);

#endif /* __HW_CODEC_H_ */