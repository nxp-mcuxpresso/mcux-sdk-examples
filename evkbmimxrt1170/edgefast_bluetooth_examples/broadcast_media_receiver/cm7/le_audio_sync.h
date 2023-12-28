/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LE_AUDIO_SYNC_H
#define __LE_AUDIO_SYNC_H

#include <stdint.h>

#include "le_audio_common.h"

void le_audio_sync_init(void);
void le_audio_sync_start(uint32_t iso_interval_us, uint32_t sync_delay_us, int sample_rate, int samples_per_frame, uint32_t presentation_delay_us, uint32_t sync_index_init);
void le_audio_sync_process(frame_packet_t *frame);
void le_audio_sync_stop(void);

#if defined(LE_AUDIO_SYNC_TEST) && (LE_AUDIO_SYNC_TEST > 0)
void le_audio_sync_test_init(int sample_rate);
#endif

#endif /* __LE_AUDIO_SYNC_H */