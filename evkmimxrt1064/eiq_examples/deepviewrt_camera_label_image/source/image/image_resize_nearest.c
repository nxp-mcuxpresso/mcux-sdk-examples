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
 * @brief Simple resizes using nearest neighbor.
 *
 * @param srcData to original image data
 * @param srcWidth original image width
 * @param srcHeight original image height
 * @param dstData to destination image data
 * @param dstWidth destination image width
 * @param dstHeight destination image height
 * @param channels count
 */
void IMAGE_Resize(uint8_t* srcData, int srcWidth, int srcHeight,
                  uint8_t* dstData, int dstWidth, int dstHeight, int channels)
{
    double dx = 1.0 * dstWidth / srcWidth;
    double dy = 1.0 * dstHeight / srcHeight;

    int step = channels * srcWidth;
    int scale_step = channels * dstWidth;

    int i, j, k, pre_i, pre_j, after_i, after_j;
    if (channels == 1)
    {
        for (i = 0; i < dstHeight; i++)
        {
            for (j = 0; j < dstWidth; j++)
            {
                dstData[(dstHeight - 1 - i) * scale_step + j] = 0;
            }
        }

        for (i = 0; i < dstHeight; i++)
        {
            for (j = 0; j < dstWidth; j++)
            {
              after_i = i;
              after_j = j;
              pre_i = (int)(after_i / dy + 0);
              pre_j = (int)(after_j / dx + 0);
              if (pre_i >= 0 && pre_i < srcHeight && pre_j >= 0 && pre_j < srcWidth)
              {
                  dstData[i * scale_step + j] = srcData[pre_i * step + pre_j];
              }
            }
        }
    }
    else if (channels == 3)
    {
        for (i = 0; i < dstHeight; i++)
        {
            for (j = 0; j < dstWidth; j++)
            {
                for (k = 0; k < 3; k++)
                {
                    dstData[(dstHeight - 1 - i) * scale_step + j * 3 + k] = 0;
                }
            }
        }
        for (i = 0; i < dstHeight; i++)
        {
            for (j = 0; j < dstWidth; j++)
            {
                after_i = i;
                after_j = j;
                pre_i = (int)(after_i / dy + 0.5);
                pre_j = (int)(after_j / dx + 0.5);
                if (pre_i >= 0 && pre_i < srcHeight && pre_j >= 0 && pre_j < srcWidth)
                {
                    for (k = 0; k < 3; k++)
                    {
                        dstData[i * scale_step + j * 3 + k] = srcData[pre_i * step + pre_j * 3 + k];
                    }
                }
            }
        }
    }
}
