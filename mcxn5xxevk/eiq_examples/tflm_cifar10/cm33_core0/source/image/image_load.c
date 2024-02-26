/*
 * Copyright 2021-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "demo_config.h"
#include "fsl_debug_console.h"
#include "image.h"
#include "image_utils.h"
#include "image_data.h"

int s_staticCount = 0;
const char* s_imageName = STATIC_IMAGE_NAME;

status_t IMAGE_GetImage(uint8_t* dstData, int32_t dstWidth, int32_t dstHeight, int32_t dstChannels)
{
    s_staticCount++;
    /* Single static sample only */
    if (s_staticCount == 1)
    {
        PRINTF(EOL "Static data processing:" EOL);
        return IMAGE_Decode(image_data, dstData, dstWidth, dstHeight, dstChannels);
    }
    else
    {
        PRINTF(EOL "Camera data processing:" EOL);
        PRINTF("Camera input is currently not supported on this device" EOL);
        for (;;)
            ;
    }
    return kStatus_Success;
}

const char* IMAGE_GetImageName()
{
    return s_imageName;
}
