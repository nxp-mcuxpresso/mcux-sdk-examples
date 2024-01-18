/*
 * Copyright 2018-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "image_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Get most significant byte */
#define GET_MSB(a, bits, ofs)  ((((a) >> (8 - (bits))) & ((1 << (bits)) - 1)) << (ofs))

/*! @brief RGB565 settings */
#define RGB565_RED 0xf800
#define RGB565_GREEN 0x07e0
#define RGB565_BLUE 0x001f

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Converts image from RGB888 to RGB565.
 *
 * @param r 0-255 red color value
 * @param g 0-255 green color value
 * @param b 0-255 blue color value
 * @return color in RGB565
 */
uint16_t IMAGE_ConvRgb888ToRgb565(uint32_t r, uint32_t g, uint32_t b)
{
    b = GET_MSB(b, 5, 0);
    g = GET_MSB(g, 6, 5);
    r = GET_MSB(r, 5, 11);

    return (uint16_t)(r | g | b);
}

/*!
 * @brief Extracts rectangle regions from source buffer.
 *
 * @param pDst image data address of source buffer
 * @param x0 drawing start line position on X axe
 * @param y0 drawing start line position on Y axe
 * @param w width of rectangle
 * @param h height of rectangle
 * @param pSrc image data address of destination buffer
 * @param srcW width of source image.
 */
void IMAGE_ExtractRect(uint8_t *pDst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h,
                       const uint16_t *pSrc, uint32_t srcW)
{
    const uint16_t *p;
    uint32_t x, y;
    
    for (x = 0; x < w; x++)
    {
        for (y = 0; y < h; y++)
        {
            p = pSrc + (y0 + h - y - 1) * srcW + x0 + x;
            /* Convert RGB565 to RGB888 */
            uint16_t rgb565 = *p;
            *pDst++ = (rgb565 & RGB565_RED) >> 8;
            *pDst++ = (rgb565 & RGB565_GREEN) >> 3;
            *pDst++ = (rgb565 & RGB565_BLUE) << 3;
        }
    }
}
