/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "demo_info.h"
#include "fsl_debug_console.h"
#include "model.h"

void DEMO_PrintInfo(void)
{
    PRINTF("%s example using a %s model." EOL, EXAMPLE_NAME, FRAMEWORK_NAME);
    PRINTF("Detection threshold: %d%%" EOL, DETECTION_TRESHOLD);
    PRINTF("Model: %s" EOL, MODEL_GetModelName());
}
