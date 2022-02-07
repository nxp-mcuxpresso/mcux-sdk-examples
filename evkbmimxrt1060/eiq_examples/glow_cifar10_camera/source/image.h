/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _IMAGE_H_
#define _IMAGE_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "stdint.h"
#include "stdbool.h"

#ifndef PI

/*! @brief PI value */
#define PI (3.1415926)
#endif

#ifndef APP_LCD_WIDTH

/*! @brief LCD Width settings */
#define APP_LCD_WIDTH 480
#endif

#ifndef APP_LCD_HEIGHT
/*! @brief LCD Height settings */
#define APP_LCD_HEIGHT 272
#endif

/*! @brief Get Most significant byte */
#define GET_MSB(a, bits, ofs)  (((a) >> (8 - (bits))) & ((1 << (bits)) - 1)) << ofs

/*! @brief RGB565 settings */
#define RGB565_RED 0xf800
#define RGB565_GREEN 0x07e0
#define RGB565_BLUE 0x001f

#define RGB_COLOR_ORDER  0
#define BGR_COLOR_ORDER  1
#define NHWC_LAYOUT 0
#define NCHW_LAYOUT 1
#define SCALE_NEG1TO1     0
#define SCALE_0TO1        1
#define SCALE_0TO255      2
#define SCALE_NEG128TO127 3
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Image structure */
typedef struct 
{
  int width;
  int height;
  int channels;
  uint8_t *imageData;
} Image;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Allocates new image space with scaled size from origin image
 *
 * @param pointer to origin image
 * @param scaled factor of x dimension
 * @param scaled factor of y dimension
 */
Image* ImCreate(Image* imgOrig, double dx, double dy);

/*!
 * @brief Scales origin image according scaled factors.
 *
 * @param pointer to origin image
 * @param pointer to new allocated image
 * @param scaled factor of x dimension
 * @param scaled factor of y dimension
 * @retval scaled image
 */
Image* ImScale(Image* imgOrig, Image* imgSca, double dx, double dy);

/*!
 * @brief Converts RGB 888 to RGB 565
 *
 * @param red color
 * @param green color
 * @param blue color
 * @retval 565 RGB color
 */
uint16_t Rgb888ToRgb565(uint32_t r, uint32_t g, uint32_t b);

/*!
 * @brief Converts RGB 565 to RGB 888
 *
 * @param RGB 888
 * @retval 888 RGB color
 */
uint32_t Rgb565ToRgb888(uint16_t col);

/*!
 * @brief Extracts image from camera buffer
 *
 * @param pointer to destination buffer
 * @param source image start x position
 * @param source image start y position
 * @param source image width
 * @param source image height
 * @param pointer to source buffer
 */
void ExtractImage(uint16_t *pExtract, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint16_t *pSrc);

/*!
 * @brief Converts CSI to Image
 *
 * @param pointer to destination buffer
 * @param source image width
 * @param source image height
 * @param pointer to source buffer
 * @param switch for RGB color. Set true for RGB otherwise BGR is used
 */
void CSI2Image(uint8_t *pDst, uint32_t w, uint32_t h, uint16_t *pSrc, uint8_t color_order, uint8_t layout_format);

/*!
 * @brief Draws pixel
 *
 * @param image buffer
 * @param pixel x position
 * @param pixel y position
 * @param red color
 * @param green color
 * @param blue color
 */
void DrawPixel(uint16_t *pDst, uint32_t x, uint32_t y, uint32_t r,uint32_t g, uint32_t b);

/*!
 * @brief Draws line
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
void DrawLine(uint16_t *pDst,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t r, uint32_t g, uint32_t b);

/*!
 * @brief Draws rectangle
 *
 * @param image buffer
 * @param rectangle start x position
 * @param rectangle start y position
 * @param red color
 * @param green color
 * @param blue color
 */
void DrawRect(uint16_t *pDst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t r, uint32_t g, uint32_t b);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _IMAGE_H_ */
