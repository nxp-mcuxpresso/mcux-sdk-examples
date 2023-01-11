/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "demo_config.h"
#include "fsl_debug_console.h"
#include "image.h"
#include "image_utils.h"
#include "image_data.h"
#include "eiq_video_worker.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Image name. */
static const char* s_imageName = STATIC_IMAGE_NAME;
/* Pointer to video worker. */
static EIQ_VideoWorker_t* s_worker = NULL;
/* Pointer to capture buffer. */
static uint8_t* s_captureBuffer = NULL;
/* Dimensions of capture buffer. */
static Dims_t s_captureBufferDims;
static bool s_static = true;

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Stores image data to destination buffer.
 *
 * This function gets image from static image or from camera device.
 *
 * @param dstData address of destination buffer for storing image data
 * @param dstWidth width of destination image
 * @param dstHeight height of destination image
 * @param dstChannels number of color channels of destination image
 * @return status code
 */
status_t IMAGE_GetImage(uint8_t* dstData, int32_t dstWidth, int32_t dstHeight, int32_t dstChannels)
{
    /* Switch to camera image capture after one static image. */
    if (s_static)
    {
        s_static = false;
        PRINTF(EOL "Static data processing:" EOL);
        return IMAGE_Decode(image_data, dstData, dstWidth, dstHeight, dstChannels);
    }
    else
    {
        if (s_worker == NULL)
        {
            PRINTF(EOL "Camera data processing:" EOL);
            s_worker = EIQ_InitVideoWorker();
            s_captureBuffer = s_worker->base.getData();
            s_captureBufferDims = s_worker->base.getResolution();
            s_worker->base.start();
        }

        while (!s_worker->base.isReady()) {}
        PRINTF(EOL "Data for inference are ready" EOL);

        IMAGE_Resize(s_captureBuffer, s_captureBufferDims.width, s_captureBufferDims.height,
                     dstData, dstWidth, dstHeight, dstChannels);
    }
    return kStatus_Success;
}

/*!
 * @brief Gets Image name.
 *
 * This function returns name of static image.
 *
 * @return name of the image
 */
const char* IMAGE_GetImageName(void)
{
    if (s_static)
    {
        return s_imageName;
    }
    return "camera";
}
