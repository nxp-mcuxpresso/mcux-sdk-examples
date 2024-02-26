/*
 * Copyright (C) 2018 Arm Limited or its affiliates. All rights reserved.
 * Copyright 2021 NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __MFCC_H__
#define __MFCC_H__

extern "C" {
#ifdef __ARM_ARCH
  #include "arm_math.h"

  typedef float fft_type;
#else
  #include "fft.h"

  #include <math.h>

  typedef double fft_type;
#endif
}

#include <string.h>
#include <stdint.h>

#define SAMP_FREQ 16000
#define NUM_FBANK_BINS 40
#define MEL_LOW_FREQ 20
#define MEL_HIGH_FREQ 4000

#define M_2PI 6.283185307179586476925286766559005

class MFCC
{
  private:
    int num_mfcc_features;
    int frame_len;
    int frame_len_padded;
    fft_type * frame;
    float * buffer;
    float * mel_energies;
    float * window_func;
    int32_t * fbank_filter_first;
    int32_t * fbank_filter_last;
    float ** mel_fbank;
    float * dct_matrix;
#ifdef __ARM_ARCH
    arm_rfft_fast_instance_f32 * rfft;
#endif
    float * create_dct_matrix(int32_t input_length, int32_t coefficient_count); 
    float ** create_mel_fbank();
 
    static inline float InverseMelScale(float mel_freq) {
      return 700.0f * (expf (mel_freq / 1127.0f) - 1.0f);
    }

    static inline float MelScale(float freq) {
      return 1127.0f * logf (1.0f + freq / 700.0f);
    }

  public:
    MFCC(int num_mfcc_features, int frame_len);
    ~MFCC();
    void mfcc_compute(const int16_t* data, float* mfcc_out);
};

#endif
