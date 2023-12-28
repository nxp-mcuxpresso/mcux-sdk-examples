/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio.h"
#include "audio_data.h"
#include "audio_stream.h"
#include "demo_config.h"
#include "fsl_debug_console.h"
#include "kws_mfcc.hpp"

/* Recording window is one frame */
const int kRecordingWin = 1;
static KWS_MFCC s_kws(kRecordingWin);
static int s_sampleCount = 0;

status_t AUDIO_GetSpectralSample(uint8_t* dstData, size_t size)
{
    /* Audio buffer size must be a multiple of audio block size.
       Otherwise the remaining non-complete part of the buffer will not be processed. */
    assert(SAMP_FREQ % s_kws.audio_block_size == 0);

    s_sampleCount++;
    /* Switch to microphone audio capture after two static samples. */
    if (s_sampleCount <= 2)
    {
        if (s_sampleCount == 1)
        {
            PRINTF(EOL "Static data processing:" EOL);
        }
        /* Select a static sample */
        const int16_t *staticData = (s_sampleCount == 1) ? off_sample_data : right_sample_data;
        PRINTF("Expected category: %s" EOL, AUDIO_GetSampleName());

        AUDIO_PreprocessSample(staticData, NUM_FRAMES);
        s_kws.store_features(dstData);

        if (s_sampleCount == 2)
        {
            AUDIO_Init();
        }
    }
    else
    {
        if (s_sampleCount == 3)
        {
            PRINTF(EOL "Microphone data processing:" EOL);
        }
        int16_t* sampleData;
        while (AUDIO_GetNextFrame(&sampleData))
        {
            AUDIO_PreprocessSample(sampleData - FRAME_SHIFT, FRAME_LEN / FRAME_SHIFT);
        }
        s_kws.store_features(dstData);
    }
    return kStatus_Success;
}

void AUDIO_PreprocessSample(const int16_t* srcData, size_t audioBlocksPerBuffer)
{
    for (int i = 0; i < audioBlocksPerBuffer; i++)
    {
        /* Redirect the source for MFCC extraction to the shifted chunks of frame length */
        s_kws.audio_buffer = srcData + i * s_kws.audio_block_size;
        s_kws.extract_features();
    }
}

const char* AUDIO_GetSampleName(void)
{
    switch (s_sampleCount)
    {
        case 1:
            return OFF_SAMPLE_NAME;
        case 2:
            return RIGHT_SAMPLE_NAME;
    }
    return "microphone";
}
