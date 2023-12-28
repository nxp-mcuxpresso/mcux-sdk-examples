/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "gprintf/chgui.h"
#include "gprintf/font.h"
#include "image_utils.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*! @brief Draws in X, Y configuration. */

#if APP_ROTATE_DISPLAY_NUM == 180
#define IMAGE_DRAW_PIXEL(LCDBUF, X, Y, R, G, B) \
    IMAGE_DrawPixel(LCDBUF, X, Y, R, G, B, DEMO_PANEL_WIDTH)

/*! @brief Plus or minus sign according monitor rotation. */

#define INCREMENT_X(X,Y) X + Y
#define DECREMENT_X(X,Y) X - Y
#define INCREMENT_Y(X,Y) X + Y
#define DECREMENT_Y(X,Y) X - Y
#else

/*! @brief Draws in Y, X configuration. */

#define IMAGE_DRAW_PIXEL(LCDBUF, X, Y, R, G, B) \
    IMAGE_DrawPixel(LCDBUF, Y, X, R, G, B, DEMO_PANEL_WIDTH)
#define INCREMENT_X(X, Y) ((X) + (Y))
#define DECREMENT_X(X, Y) ((X) - (Y))
#define INCREMENT_Y(X, Y) ((X) - (Y))
#define DECREMENT_Y(X, Y) ((X) + (Y))
#endif


/*!
 * @brief GUI data container.
 */
typedef struct
{
    int x;
    int y;
    int chars;
    uint16_t *lcdBuf;
    char printbuffer[GUI_PRINTF_BUF_SIZE];
} GUI_PrintStorage_t;

/*!
 * @brief GUI font configuration structure
 */
typedef struct
{
    char* mame;
    uint8_t  x_size;
    uint8_t  y_size;
    const char* data;
} chgui_font_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief Draws character to point in LCD buffer.
 *
 * @param c input character for drawing
 * @param x drawing position on X axe
 * @param y drawing position on Y axe
 */
static void GUI_DispChar(char c, int x, int y);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Allocated GUI data container. */
static GUI_PrintStorage_t storage =
{ 0, 0, 0, NULL };

/* Allocated GUI font configuration structure. */
static chgui_font_t _gFontTbl[] =
{{ "Font", FONT_XSize, FONT_YSize, consolas_12ptBitmaps }, };

/*!
 * @brief Draws text stored in local text buffer to LCD buffer.
 *
 * This function copy content of data from local text buffer
 * to the LCD. This function should be called after
 * GUI_PrintfToBuffer.
 *
 * @param lcd_buf LCD buffer address destination for drawing text

 */
void GUI_DrawBuffer(uint16_t *lcd_buf)
{
    storage.lcdBuf = lcd_buf;
    int i;
    for (i = 0; i < storage.chars; i++)
    {
        GUI_DispChar(storage.printbuffer[i],
                DECREMENT_X(storage.x, i * _gFontTbl[0].x_size), storage.y);
    }
}

/*!
 * @brief Writes the C string pointed by format to local text buffer.
 *
 * This function writes the C string pointed by format to local
 * text buffer at defined starting point. GUI_DrawBuffer function
 * should be called to draw content of this text buffer.
 *
 * @param x drawing position on X axe
 * @param y drawing position on Y axe
 * @format C string pointed by format
 * @return The return number of written chars to the buffer
 */
int GUI_PrintfToBuffer(int x, int y, const char *format, ...)
{
    storage.x = x;
    storage.y = y;

    va_list ap;
    va_start(ap, format);

    storage.chars = vsprintf(storage.printbuffer, format, ap);

    va_end(ap);
    return storage.chars;
}

/*!
 * @brief Draws pixel with RGB color to defined point.
 *
 * This function draw color pixel at required point.
 *
 * @param color RGB color.
 * @param x drawing position on X axe.
 * @param y drawing position on Y axe.
 */
static void GUI_DrawPixel(int color, int x, int y)
{
    uint32_t r = (uint32_t)((color & 0xFF0000) >> 16);
    uint32_t g = (uint32_t)((color & 0x00FF00) >> 8);
    uint32_t b = (uint32_t)(color & 0x0000FF);
    IMAGE_DRAW_PIXEL(storage.lcdBuf, x, y, r, g, b);
}

/*!
 * @brief Draw character to point in LCD buffer.
 *
 * @param c input character for drawing.
 * @param x drawing position on X axe.
 * @param y drawing position on Y axe.
 * @param pdata address to character bitmaps for console.
 * @param font_xsize size of font in X axe.
 * @param font_ysize size of font in Y axe.
 * @param fcolor foreground RGB color.
 * @param bcolor background RGB color.
 */
static void _GUI_DispChar(char c, int x, int y, const char *pdata,
        int font_xsize, int font_ysize, int fcolor, int bcolor)
{
    uint8_t j, pos, t;
    uint8_t temp;
    uint8_t XNum;
    uint32_t base;
    XNum = (font_xsize / 8) + 1;
    if (font_ysize % 8 == 0)
    {
        XNum--;
    }
    if (c < ' ')
    {
        return;
    }
    c = c - ' ';
    base = (c * XNum * font_ysize);

    for (j = 0; j < XNum; j++)
    {
        for (pos = 0; pos < font_ysize; pos++)
        {
            temp = (uint8_t) pdata[base + pos + j * font_ysize];
            if (j < XNum)
            {
                for (t = 0; t < 8; t++)
                {
                    if ((temp >> t) & 0x01)
                    {
                        GUI_DrawPixel(fcolor, INCREMENT_X(x, t), DECREMENT_Y(y, pos));
                    }
                    else
                    {
                        GUI_DrawPixel(bcolor, INCREMENT_X(x, t), DECREMENT_Y(y, pos));
                    }
                }
            }
            else
            {
                for (t = 0; t < font_xsize % 8; t++)
                {
                    if ((temp >> t) & 0x01U)
                    {
                        GUI_DrawPixel(fcolor, INCREMENT_X(x, t), DECREMENT_Y(y, pos));
                    }
                    else
                    {
                        GUI_DrawPixel(bcolor, INCREMENT_X(x, t), DECREMENT_Y(y, pos));
                    }
                }
            }
        }
        x = INCREMENT_X(x, 8);
    }
}

static void GUI_DispChar(char c, int x, int y)
{
    _GUI_DispChar(c, x, y, _gFontTbl[0].data, (int)_gFontTbl[0].x_size,
            (int)_gFontTbl[0].y_size, 0xFFFFFF, 0x0000);
}
