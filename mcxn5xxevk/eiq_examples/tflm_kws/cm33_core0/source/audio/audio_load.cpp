/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "audio.h"
#include "audio_data.h"
#include "demo_config.h"
#include "fsl_debug_console.h"
#include "kws_mfcc.hpp"

int s_staticCount = 0;
KWS_MFCC s_kws(NUM_FRAMES);

status_t AUDIO_GetSpectralSample(uint8_t* dstData, size_t size)
{
    s_staticCount++;
    /* Two static samples only */
    if (s_staticCount == 1)
    {
        PRINTF(EOL "Static data processing:" EOL);
        AUDIO_PreprocessSample(off_sample_data, NUM_FRAMES);
        s_kws.store_features(dstData);
    }
    else if (s_staticCount == 2)
    {
        AUDIO_PreprocessSample(right_sample_data, NUM_FRAMES);
        s_kws.store_features(dstData);
    }
    else
    {
        PRINTF(EOL "Microphone data processing:" EOL);
        PRINTF("Microphone input is currently not supported on this device" EOL);
        for (;;)
            ;
    }
    return kStatus_Success;
}

void AUDIO_PreprocessSample(const int16_t* srcData, size_t audioBlocksPerBuffer)
{
    s_kws.audio_buffer = srcData;
    s_kws.extract_features();
}

const char* AUDIO_GetSampleName()
{
    switch (s_staticCount)
    {
        case 1:
            return OFF_SAMPLE_NAME;
        case 2:
            return RIGHT_SAMPLE_NAME;
    }
    return "";
}
