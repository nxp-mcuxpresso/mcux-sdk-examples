/*
 * Copyright 2022 NXP.
 * This software is owned or controlled by NXP and may only be used strictly in accordance with the
 * license terms that accompany it. By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read, and that you
 * agree to comply with and are bound by, such license terms. If you do not agree to be bound by the
 * applicable license terms, then you may not retain, install, activate or otherwise use the software.
 */

#include <FreeRTOS.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "hal_graphics_dev.h"
#include "hal_draw.h"
#include "hal_utils.h"

#define BLACK_RGB565    (0x0000)
#define WHITE_RGB565    (0xFFFF)

int hal_label_rectangle(uint8_t *frame, int width, int height, mpp_pixel_format_t format,
                        mpp_labeled_rect_t *lr)
{
    uint32_t xsize, ysize, lw;

    xsize = lr->right - lr->left;
    ysize = lr->bottom - lr->top;

    for (lw = 0; lw < lr -> line_width; lw++) {
      hal_draw_rect((uint16_t *)frame, lr->left + lw, lr->top + lw,
                       xsize, ysize, lr->line_color.rgb.R,
                       lr->line_color.rgb.G, lr->line_color.rgb.B, width);
    }
    hal_draw_text((uint16_t *)frame, WHITE_RGB565, BLACK_RGB565, width,
                 lr->left, lr->top, (char *)lr->label);
    return 0;
}

int hal_pxp_setup(gfx_dev_t *dev);

int hal_img_convert_setup(gfx_dev_t *dev)
{
    return hal_pxp_setup(dev);
}

/* Display setup */
int HAL_DisplayDev_Lcdifv2Rk055a_setup(display_dev_t *dev);

hal_display_setup_t display_setup[] =
{
    {"Lcdifv2Rk055ah", HAL_DisplayDev_Lcdifv2Rk055a_setup},
};

int setup_display_dev(hal_display_setup_t display_setup[], int display_nb,
		      const char *name, display_dev_t *dev);
int hal_display_setup(const char *name, display_dev_t *dev)
{
    return setup_display_dev(display_setup, ARRAY_SIZE(display_setup), name, dev);
}

/* Camera setup */
int HAL_CameraDev_MipiOv5640_setup(const char *name, camera_dev_t *dev, _Bool defconfig);

hal_camera_setup_t camera_setup[] =
{
    {"MipiOv5640", HAL_CameraDev_MipiOv5640_setup},
};

int setup_camera_dev(hal_camera_setup_t camera_setup[], int camera_nb,
		      const char *name, camera_dev_t *dev, _Bool defconfig);
int hal_camera_setup(const char *name, camera_dev_t *dev, _Bool defconfig)
{
  return setup_camera_dev(camera_setup, ARRAY_SIZE(camera_setup), name, dev, defconfig);
}