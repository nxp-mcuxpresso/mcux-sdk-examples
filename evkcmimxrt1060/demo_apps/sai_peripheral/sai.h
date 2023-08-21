/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_dmamux.h"
#include "fsl_sai_edma.h"
#include "arm_math.h"
#include "fsl_codec_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define OVER_SAMPLE_RATE (384U)
#define BUFFER_SIZE      (512)
#define BUFFER_NUM       (4)
#if defined BOARD_HAS_SDCARD && (BOARD_HAS_SDCARD != 0)
#define DEMO_SDCARD (1U)
#endif
/* demo audio sample rate */
#define DEMO_AUDIO_SAMPLE_RATE (kSAI_SampleRate16KHz)
/* demo audio master clock */
#if (defined FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER && FSL_FEATURE_SAI_HAS_MCLKDIV_REGISTER) || \
    (defined FSL_FEATURE_PCC_HAS_SAI_DIVIDER && FSL_FEATURE_PCC_HAS_SAI_DIVIDER)
#define DEMO_AUDIO_MASTER_CLOCK OVER_SAMPLE_RATE *DEMO_AUDIO_SAMPLE_RATE
#else
#define DEMO_AUDIO_MASTER_CLOCK DEMO_SAI_CLK_FREQ
#endif
/* demo audio data channel */
#define DEMO_AUDIO_DATA_CHANNEL (2U)
/* demo audio bit width */
#define DEMO_AUDIO_BIT_WIDTH (kSAI_WordWidth16bits)
/* file system aboslutely path */
#define DEMO_RECORD_PATH                                           \
    {                                                              \
        SDDISK + '0', ':', '/', 'r', 'e', 'c', 'o', 'r', 'd', '\0' \
    }
#define DEMO_RECORD_WAV_PATH                                                                                         \
    {                                                                                                                \
        SDDISK + '0', ':', '/', 'r', 'e', 'c', 'o', 'r', 'd', '/', 'm', 'u', 's', 'i', 'c', '1', '.', 'w', 'a', 'v', \
            '\0'                                                                                                     \
    }
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void PlaybackSine(I2S_Type *base, uint32_t SineWaveFreqHz, uint32_t time_s);
void RecordSDCard(I2S_Type *base, uint32_t time_s);
void RecordPlayback(I2S_Type *base, uint32_t time_s);
/*******************************************************************************
 * Variables
 ******************************************************************************/
