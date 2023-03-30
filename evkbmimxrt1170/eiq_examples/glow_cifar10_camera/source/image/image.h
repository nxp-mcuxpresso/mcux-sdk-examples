/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <stdint.h>

#include "fsl_common.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/*!
 * @addtogroup image_utils
 * @{
 */

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Stores image to destination buffer.
 *
 * This function gets image from static image or from camera device.
 *
 * @param dstData address of destination buffer for storing image data
 * @param dstWidth width of destination image
 * @param dstHeight height of destination image
 * @param dstChannels number of color channels of destination image
 * @return status code
 */
status_t IMAGE_GetImage(uint8_t* dstData, int32_t dstWidth, int32_t dstHeight, int32_t dstChannels);

/*!
 * @brief Gets Image name.
 *
 * This function returns name of static image.
 *
 * @return name of the image
 */
const char* IMAGE_GetImageName(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _IMAGE_H_ */
