/*
 * Copyright 2023-2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LE_AUDIO_SYNC_H
#define __LE_AUDIO_SYNC_H

#include <stdint.h>

#include "le_audio_common.h"

struct sync_info {
        uint32_t iso_interval;
        uint32_t sync_delay;
        uint32_t presentation_delay;

        uint32_t time_stamp;
};

void le_audio_sync_init(void);
void le_audio_sync_start(int sample_rate, int samples_per_frame);
void le_audio_sync_set(uint32_t iso_interval_us, uint32_t sync_delay_us, uint32_t presentation_delay_us);
void le_audio_sync_process(frame_packet_t *frame);
void le_audio_sync_stop(void);
void le_audio_sync_info_get(struct sync_info *info);

void le_audio_sync_test_init(int sample_rate);
/* 
 * mode 0: disable.
 * mode 1: will output 500hz sine.
 * mode 2: will outout 10ms 500hz sine followed with 20ms mute.
 */
int le_audio_sync_test_mode(int mode);

#endif /* __LE_AUDIO_SYNC_H */