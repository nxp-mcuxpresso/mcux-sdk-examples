/*
 * Copyright 2018-2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "image.h"
#include "stdlib.h"
#include "arm_math.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : ImCreate
 * Description   : Allocates new image space with scaled size from origin image.
 *
 * END ****************************************************************************************************************/
Image* ImCreate(Image* imgOrig, double dx, double dy)
{
  Image* ImgSca;
  ImgSca = (Image*)malloc(sizeof(Image));
  ImgSca->channels = imgOrig->channels;
  ImgSca->width = (int)(imgOrig->width * dx + 0.5);
  ImgSca->height = (int)(imgOrig->height * dy + 0.5);
  ImgSca->imageData = (uint8_t*)malloc(sizeof(uint8_t) * ImgSca->width * ImgSca->height * imgOrig->channels);
  return ImgSca;
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : ImScale
 * Description   : Scales origin image according scaled factors.
 *
 * END ****************************************************************************************************************/
Image* ImScale(Image* imgOrig, Image* imgSca, double dx, double dy)
{
  int width = 0, height = 0, channels = 1, step = 0, scale_step = 0;
  width = imgOrig->width;
  height = imgOrig->height;
  channels = imgOrig->channels;

  step = channels * width;
  scale_step = channels * imgSca->width;

  int i, j, k, pre_i, pre_j, after_i, after_j;
  if (channels == 1)
  {
    for (i = 0; i < imgSca->height; i++)
    {
      for (j = 0; j < imgSca->width; j++)
      {
        imgSca->imageData[(imgSca->height - 1 - i) * scale_step + j] = 0;
      }
    }

    for (i = 0; i < imgSca->height; i++)
    {
      for (j = 0; j < imgSca->width; j++)
      {
        after_i = i;
        after_j = j;
        pre_i = (int)(after_i / dy + 0);
        pre_j = (int)(after_j / dx + 0);
        if (pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)
        {
          imgSca->imageData[i * scale_step + j] = imgOrig->imageData[pre_i * step + pre_j];
        }
      }
    }
  }
  else if(channels == 3)
  {
    for (i = 0; i < imgSca->height; i++)
    {
      for (j = 0; j < imgSca->width; j++)
      {
        for (k = 0; k < 3; k++)
        {
          imgSca->imageData[(imgSca->height - 1 - i) * scale_step + j * 3 + k] = 0;
        }
      }
    }
    for (i = 0; i < imgSca->height; i++)
    {
      for (j = 0; j < imgSca->width; j++)
      {
        after_i = i;
        after_j = j;
        pre_i = (int)(after_i / dy + 0.5);
        pre_j = (int)(after_j / dx + 0.5);
        if (pre_i >= 0 && pre_i < height && pre_j >= 0 && pre_j < width)
        {
          for (k = 0; k<3; k++)
          {
            imgSca->imageData[i * scale_step + j * 3 + k] = imgOrig->imageData[pre_i * step + pre_j * 3 + k];
          }
        }
      }
    }
  }
  return imgSca;
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : Rgb888ToRgb565
 * Description   : Converts RGB 888 to RGB 565
 *
 * END ****************************************************************************************************************/
uint16_t Rgb888ToRgb565(uint32_t r, uint32_t g, uint32_t b) 
{
  b = GET_MSB(b, 5, 0);
  g = GET_MSB(g, 6, 5);
  r = GET_MSB(r, 5, 11);
  return (uint16_t)(r | g | b);
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : Rgb565ToRgb888
 * Description   : Converts RGB 565 to RGB 888
 *
 * END ****************************************************************************************************************/
uint32_t Rgb565ToRgb888(uint16_t col)
{
  uint32_t n888Color = 0;

  uint32_t r, g, b;
  r = (col >> 11) & 0x1F;  //in order to use the BFI instruction
  g = (col >> 5)  & 0x3F;
  b = (col >> 0)  & 0x1F;
  r = (r << 3) | (r >> 5); //extract the highest 3 bits, and add it to the lowest
  g = (g << 2) | (g >> 6);
  b = (b << 3) | (b >> 5);
  n888Color = (r << 16) | (g << 8) | (b << 0);
  
  return n888Color;
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : ExtractImage
 * Description   : Extracts image from camera buffer
 *
 * END ****************************************************************************************************************/
void ExtractImage(uint16_t *pDst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint16_t *pSrc)
{
  uint16_t *p;
  uint32_t x, y;
  for (y = 0; y < h; y++) 
  {
    p = pSrc + (y0 + y) * 480 + x0;
    for (x = 0; x < w; x++) 
    {
      *pDst++ = *p++;
    }
  }
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : CSI2Image
 * Description   : Converts CSI to Image
 *
 * END ****************************************************************************************************************/
void CSI2Image(uint8_t *pDst, uint32_t w, uint32_t h, uint16_t *pSrc, uint8_t color_order, uint8_t layout_format)
{
  uint32_t t_Color;
  uint32_t x,y;
  uint8_t b, g, r;
  uint8_t *temp;

  for (y = 0; y < h; y++)
  {
    for (x = 0; x < w; x++)
    {
      t_Color = Rgb565ToRgb888(*pSrc++);
      b = t_Color & 0xff; //b
      g = (t_Color & (0xff00)) >> 8; //g
      r = (t_Color & (0xff0000)) >> 16; //r
      if (color_order==RGB_COLOR_ORDER)
      {
    	  //if NHWC order (RGBRGBRGB...)
          if(layout_format==NHWC_LAYOUT)
          {
              *pDst++ = r;
              *pDst++ = g;
              *pDst++ = b;
          }
          //if NCHW order (RRRR.. GGGGG... BBBB...)
          else
          {
              *pDst = r;
              temp= pDst+h*w;    //Math to find offset (width*height) to store green pixel value
              *temp = g;
              temp=pDst+h*w*2;   //Math to find offset (2*width*height) to store blue pixel value
              *temp = b;
              pDst++;
          }
      }
      else
      {
    	  //if NHWC order (BGRBGRBGR...)
          if(layout_format==NHWC_LAYOUT)
          {
              *pDst++ = b;
              *pDst++ = g;
              *pDst++ = r;
          }
          //if NCHW order (BBBBB.. GGGGG... RRRRR...)
          else
          {
              *pDst = b;
              temp=pDst+h*w;  //Math to find offset (width*height) to store green pixel value
              *temp = g;
              temp=pDst+h*w*2; //Math to find offset (2*width*height) to store red pixel value
              *temp = r;
              pDst++;
          }
      }
    }
  }
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : DrawPixel
 * Description   : Draws pixel
 *
 * END ****************************************************************************************************************/
void DrawPixel(uint16_t *pDst, uint32_t x, uint32_t y, uint32_t r,uint32_t g, uint32_t b)
{
  uint32_t lcd_w = APP_LCD_WIDTH;
  uint16_t col = Rgb888ToRgb565(r, g, b); 
  pDst[y * (lcd_w) + x] = col;
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : DrawLine
 * Description   : Draws line
 *
 * END ****************************************************************************************************************/
void DrawLine(uint16_t* pDst,uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t r, uint32_t g, uint32_t b)
{
  uint32_t x;
  float dx, dy, y, k;
  dx = x1 - x0;
  dy = y1 - y0;
  if (dx != 0)
  {
    k = dy / dx;
    y = y0;
    for (x = x0; x < x1; x++)
    {
      DrawPixel(pDst, x, (uint32_t)(y + 0.5f), r, g, b);
      y = y + k;
    }
  }
  else
  {
    x = x0;
    for (y = y0; y < y1; y++)
    {
      DrawPixel(pDst, x, (uint32_t)y, r, g, b);
    }
  }
}

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : DrawRect
 * Description   : Draws rectangle
 *
 * END ****************************************************************************************************************/
void DrawRect(uint16_t *pdst, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h, uint32_t r, uint32_t g, uint32_t b)
{
  DrawLine(pdst, x0, y0, x0 + w, y0, r, g, b);
  DrawLine(pdst, x0, y0, x0, y0 + h, r, g, b);
  DrawLine(pdst, x0, y0 + h, x0 + w, y0 + h, r, g, b);
  DrawLine(pdst, x0 + w, y0, x0 + w, y0 + h, r, g, b);
}
