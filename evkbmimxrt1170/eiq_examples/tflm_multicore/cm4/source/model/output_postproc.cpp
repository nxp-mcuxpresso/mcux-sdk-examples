/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include "fsl_debug_console.h"
#include "output_postproc.h"
#include "get_top_n.h"
#include "demo_config.h"
#include "labels.h"
#include "timer.h"


/* Enable to use any keyword to wakeup core0 */
#define WAKEUP_WITH_ANY_KEYWORD 1
/* Use specific keyword to wakeup, ignored when WAKEUP_WITH_ANY_KEYWORD is set */
#define WAKEUP_KEYWORD  "go"

status_t MODEL_ProcessOutput(const uint8_t* data, const tensor_dims_t* dims,
                             tensor_type_t type, int inferenceTime)
{
    const float kThreshold = (float)DETECTION_TRESHOLD / 100;
    result_t topResults[NUM_RESULTS];
    static result_t lastTopResult = {.score = 0.0, .index = -1};
    bool print = false;
    static int lastPrintTime = 0;
    const int kUsInSecond = 1000000;
    const char* label = NULL;
    int index = 0;

    /* Find the best label candidates. */
    MODEL_GetTopN(data, dims->data[dims->size - 1], type, NUM_RESULTS, kThreshold, topResults);

    auto result = topResults[0];
    if (result.index >= 0 && result.score > kThreshold)
    {
        print = lastTopResult.index != result.index;
        lastTopResult = result;
    }

    int time = TIMER_GetTimeInUS();
    if (print)
    {
        index = result.index;
        label = index >= 0 ? labels[index] : "No word detected";
        int score = (int)(result.score * 100);

        PRINTF("----------------CM4 CORE----------------" EOL);
        PRINTF("     Inference time: %d ms" EOL, inferenceTime / 1000);
        PRINTF("     Detected: %s (%d%%)" EOL, label, score);
        PRINTF("----------------------------------------" EOL EOL);

        lastPrintTime = time;
    }

    if ((time - lastPrintTime) > kUsInSecond)
    {
        lastTopResult = {.score = 0.0, .index = -1};
    }

#if WAKEUP_WITH_ANY_KEYWORD
    if ((index > 0) && strcmp(label, "Unknown") != 0)
        return kStatus_Success;
    else
        return kStatus_ReadOnly;
#else
    if (strcmp(label, WAKEUP_KEYWORD) == 0)
        return kStatus_Success;
    else
        return kStatus_ReadOnly;
#endif

}

