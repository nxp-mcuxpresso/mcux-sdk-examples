/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _VOICE_SEEKER_H_
#define _VOICE_SEEKER_H_

#define INT_TO_FLOAT 3.0517578125e-05f
#define FLOAT_TO_INT 32768

#define FRAME_SIZE                 480 // 30ms of 16kHz
#define RDSP_NUM_SPK               0
#define RDSP_ENABLE_AEC            0
#define RDSP_ENABLE_DOA            0
#define RDSP_ENABLE_VOICESPOT      0
#define RDSP_ENABLE_PAYLOAD_BUFFER 0
#define RDSP_ENABLE_AGC            0
#define RDSP_BUFFER_LENGTH_SEC     1.5f;
#define RDSP_AEC_FILTER_LENGTH_MS  150

typedef int (*VoiceSeeker_Initialize_T)(void *arg);
typedef int (*VoiceSeeker_Execute_T)(void *arg, void *inputBuffer, int size);
typedef int (*VoiceSeeker_Deinit_T)(void);

void DeInterleave(const int16_t *pDataInput, int16_t *pDataOutput, uint16_t FrameSize, uint16_t ChannelNumber);

extern VoiceSeeker_Initialize_T VoiceSeeker_Initialize_func;
extern VoiceSeeker_Execute_T VoiceSeeker_Execute_func;
extern VoiceSeeker_Deinit_T VoiceSeeker_Deinit_func;

#endif
