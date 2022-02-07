/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "image_utils.h"

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Decodes image.
 *
 * @param srcData image data address of source buffer
 * @param dstData image data address of destination buffer
 * @param dstWidth destination image width
 * @param dstHeight destination image height
 * @param dstChannels destination number of channels
 * @return status code
 */
status_t IMAGE_Decode(const uint8_t* srcData, uint8_t* dstData,
                      int32_t dstWidth, int32_t dstHeight, int32_t dstChannels)
{
    /* No decoding is necessary. */
    memcpy(dstData, srcData, dstWidth * dstHeight * dstChannels);

    return kStatus_Success;
}
