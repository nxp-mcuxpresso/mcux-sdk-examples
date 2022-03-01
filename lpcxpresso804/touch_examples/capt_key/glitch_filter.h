/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef __GLITCH_FILTER_H__
#define __GLITCH_FILTER_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Keep all the static variable and information for each key. */
typedef struct
{
    /* Attr. */
    uint32_t filterLevelMax; /* The output would change if the new value keeps enough time. */

    /* Static. */
    int32_t initValue;         /* Initialize value. */
    int32_t stableOutput;      /* Current output value. */
    int32_t newOutput;         /* Potential output value, would be change to output value if it keeps long enough. */
    uint32_t filterLevelCount; /* Tick counter for next value. */
} glitch_filter_handle_t;

void FILTER_Init(glitch_filter_handle_t *handle, int32_t initValue, uint32_t filter_level);
int32_t FILTER_Output(glitch_filter_handle_t *handle, int32_t inputRaw);

#endif /* __GLITCH_FILTER_H__ */
