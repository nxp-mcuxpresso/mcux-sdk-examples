/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "output_postproc.h"
#include "get_top_n.h"
#include "demo_config.h"
#include "labels.h"
#include "timer.h"

status_t MODEL_ProcessOutput(const uint8_t* data, const tensor_dims_t* dims,
                             tensor_type_t type, int inferenceTime)
{
    const float kThreshold = (float)DETECTION_TRESHOLD / 100;
    result_t topResults[NUM_RESULTS];
    static result_t lastTopResult = {.score = 0.0, .index = -1};
    static int lastPrintTime = 0;
    static int counter = 1; /* Inference counter for first static inputs */
    const int kUsInSecond = 1000000;

    /* Find the best label candidates. */
    MODEL_GetTopN(data, dims->data[dims->size - 1], type, NUM_RESULTS, kThreshold, topResults);

    auto result = topResults[0];
    if (result.index >= 0 && result.score > lastTopResult.score)
    {
        lastTopResult = result;
    }

    int time = TIMER_GetTimeInUS();
    if (counter <= 2 || (time - lastPrintTime) >= kUsInSecond)
    {
        int index = lastTopResult.index;
        const char* label = index >= 0 ? labels[index] : "No word detected";
        int score = (int)(lastTopResult.score * 100);

        PRINTF("----------------------------------------" EOL);
        PRINTF("     Inference time: %d ms" EOL, inferenceTime / 1000);
        PRINTF("     Detected: %s (%d%%)" EOL, label, score);
        PRINTF("----------------------------------------" EOL EOL);

        lastPrintTime = time;
        lastTopResult = {.score = 0.0, .index = -1};
        counter++;
    }

    return kStatus_Success;
}
