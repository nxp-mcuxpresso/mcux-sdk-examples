#ifndef _CHGUI_H_
#define _CHGUI_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "eiq_display_conf.h"

/*!
 * @addtogroup gprintf
 * @{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#if APP_ROTATE_DISPLAY_NUM == 180
#define IMAGE_DRAW_PIXEL(LCDBUF, X, Y, R, G, B) IMAGE_DrawPixel(LCDBUF, X, Y, R, G, B, DEMO_PANEL_WIDTH)
/*! @brief Starting position of text on X axe. */
#define GUI_X_POS DEMO_PANEL_WIDTH *66 / 100
/*! @brief Starting position of text on Y axe. */
#define GUI_Y_POS DEMO_PANEL_HEIGHT *14 / 100
#else
/*! @brief Starting position of text on X axe. */
#define GUI_X_POS DEMO_PANEL_WIDTH
/*! @brief Starting position of text on Y axe. */
#define GUI_Y_POS DEMO_PANEL_HEIGHT
#endif


/*! @brief Local text buffer size. */
#define GUI_PRINTF_BUF_SIZE 64

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Draws text stored in local text buffer to LCD buffer.
 *
 * This function copy content of data from local text buffer
 * to the LCD. This function should be called after
 * GUI_PrintfToBuffer.
 *
 * @param lcd_buf LCD buffer address destination for drawing text

 */
void GUI_DrawBuffer(uint16_t *lcd_buf);

/*!
 * @brief Writes the C string pointed by format to local text buffer.
 *
 * This function writes the C string pointed by format to local
 * text buffer at defined starting point. GUI_DrawBuffer function
 * should be called to draw content of this text buffer.
 *
 * @param x drawing position on X axe
 * @paramy drawing position on Y axe
 * @param format C string pointed by format
 * @return The return number of written chars to the buffer
 */
int GUI_PrintfToBuffer(int x, int y, const char *format, ...);


#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif
