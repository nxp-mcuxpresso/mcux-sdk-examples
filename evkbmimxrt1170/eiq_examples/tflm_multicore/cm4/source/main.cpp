/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <audio/audio.h>

#include "board_init.h"
#include "demo_config.h"
#include "demo_info.h"
#include "model.h"
#include "output_postproc.h"
#include "timer.h"
#include "fsl_debug_console.h"
#include "fsl_mu.h"
#include "board.h"

/* Flag indicates Core Boot Up*/
#define BOOT_FLAG 0x01U


int main(void)
{
    BOARD_Init();
    TIMER_Init();
    MU_Init(MU_BASE);

    /* Send flag to Core 0 to indicate Core 1 has startup */
    MU_SetFlags(MU_BASE, BOOT_FLAG);
    PRINTF("This is the secondary core: core1" EOL);

    DEMO_PrintInfo();

    if (MODEL_Init() != kStatus_Success)
    {
        PRINTF("Failed initializing model" EOL);
        for (;;) {}
    }

    tensor_dims_t inputDims;
    tensor_type_t inputType;
    uint8_t* inputData = MODEL_GetInputTensorData(&inputDims, &inputType);

    tensor_dims_t outputDims;
    tensor_type_t outputType;
    uint8_t* outputData = MODEL_GetOutputTensorData(&outputDims, &outputType);

    while (1)
    {
        /* Expected tensor dimensions: [batches, frames, mfcc, channels] */
        if (AUDIO_GetSpectralSample(inputData, inputDims.data[1] * inputDims.data[2]) != kStatus_Success)
            continue;

        auto startTime = TIMER_GetTimeInUS();
        MODEL_RunInference();
        auto endTime = TIMER_GetTimeInUS();

        if (MODEL_ProcessOutput(outputData, &outputDims, outputType, \
                endTime - startTime) == kStatus_Success)
        {
            PRINTF("Core1: send message to wake up core0\r\n");
            MU_SendMsg(MU_BASE, 0, 0);
        }
    }

}


