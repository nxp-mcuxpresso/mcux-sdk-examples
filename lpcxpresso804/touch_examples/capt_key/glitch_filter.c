/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "glitch_filter.h"

void FILTER_Init(glitch_filter_handle_t *handle, int32_t initValue, uint32_t filter_level)
{
    /* For Attr. */
    handle->filterLevelMax = filter_level - 1U;

    /* For Static. */
    handle->initValue        = initValue;
    handle->stableOutput     = initValue;
    handle->newOutput        = initValue;
    handle->filterLevelCount = 1U;
}

int32_t FILTER_Output(glitch_filter_handle_t *handle, int32_t inputRaw)
{
    if (inputRaw != handle->stableOutput)
    {
        if (inputRaw != handle->newOutput)
        {
            handle->newOutput        = inputRaw;
            handle->filterLevelCount = 1u; /* Start to count the new input value. */
        }
        else                               /* inputRaw == handle->newOutput */
        {
            if (handle->filterLevelCount < handle->filterLevelMax)
            {
                handle->filterLevelCount++;
            }
            else /* (handle->filterLevelCount >= handle->filterLevelMax) */
            {
                /* New value has last for a period long enough, treat it as stable value. */
                handle->stableOutput = handle->newOutput;
            }
        }
    }

    return handle->stableOutput;
}
