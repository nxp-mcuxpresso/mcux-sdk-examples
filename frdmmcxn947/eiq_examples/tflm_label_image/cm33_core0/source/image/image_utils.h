/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IMAGE_UTILS_H_
#define _IMAGE_UTILS_H_

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
 * @brief Resizes image to desired dimensions using a predefined algorithm.
 *
 * @param pointer to original image data
 * @param original image width
 * @param original image height
 * @param pointer to destination image data
 * @param desired image width
 * @param desired image height
 * @param original and target image channel count
 */
void IMAGE_Resize(uint8_t* srcData, int srcWidth, int srcHeight,
                  uint8_t* dstData, int dstWidth, int dstHeight, int channels);

/*!
 * @brief Decodes and resizes image data of predefined format.
 *
 * @param pointer to encoded image data
 * @param pointer to destination image data
 * @param desired image width
 * @param desired image height
 * @param target image channel count
 * @return status code
 */
status_t IMAGE_Decode(const uint8_t* srcData, uint8_t* dstData,
                      int32_t dstWidth, int32_t dstHeight, int32_t dstChannels);

/*!
 * @brief Converts RGB 888 to RGB 565.
 *
 * @param red color
 * @param green color
 * @param blue color
 * @return 565 RGB color
 */
uint16_t IMAGE_ConvRgb888ToRgb565(uint32_t r, uint32_t g, uint32_t b);

/*!
 * @brief Extracts image from camera buffer.
 *
 * @param pointer to destination buffer.
 * @param source image start x position
 * @param source image start y position
 * @param source image width
 * @param source image height
 * @param pointer to source buffer
 */
void IMAGE_ExtractRect(uint8_t *pDst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, const uint16_t *pSrc, uint32_t srcW);

/*!
 * @brief Draws pixel.
 *
 * @param image buffer
 * @param pixel x position
 * @param pixel y position
 * @param red color
 * @param green color
 * @param blue color
 */
void IMAGE_DrawPixel(uint16_t *pDst, uint32_t x, uint32_t y, uint32_t r,uint32_t g, uint32_t b, uint32_t lcd_w);

/*!
 * @brief Draws line.
 *
 * @param image buffer
 * @param line start x position
 * @param line start y position
 * @param line end x position
 * @param line end y position
 * @param red color
 * @param green color
 * @param blue color
 */
void IMAGE_DrawLine(uint16_t *pDst,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t r, uint32_t g, uint32_t b, uint32_t lcd_w);

/*!
 * @brief Draws rectangle.
 *
 * @param image buffer
 * @param rectangle start x position
 * @param rectangle start y position
 * @param red color
 * @param green color
 * @param blue color
 */
void IMAGE_DrawRect(uint16_t *pDst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t r, uint32_t g, uint32_t b, uint32_t lcd_w);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _IMAGE_UTILS_H_ */
