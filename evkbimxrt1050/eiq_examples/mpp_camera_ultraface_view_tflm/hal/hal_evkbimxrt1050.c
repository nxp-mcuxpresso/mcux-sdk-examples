/*
 * Copyright 2022-2023 NXP.
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

#include "fsl_cache.h"

#include "hal_graphics_dev.h"
#include "hal_utils.h"

/* Graphics setup */
int HAL_GfxDev_PXP_Register(gfx_dev_t *dev);

hal_graphics_setup_t gfx_setup[] =
{
    {"gfx_PXP", HAL_GfxDev_PXP_Register},
};

int setup_graphic_dev(hal_graphics_setup_t gfx_setup[], int graphic_nb,
                      const char *name, gfx_dev_t *dev);
int hal_gfx_setup(const char *name, gfx_dev_t *dev)
{
    return setup_graphic_dev(gfx_setup, ARRAY_SIZE(gfx_setup), name, dev);
}

/* Display setup */
int HAL_DisplayDev_LcdifRk043fn_setup(display_dev_t *dev);

hal_display_setup_t display_setup[] =
{
    {"LcdifRk043fn", HAL_DisplayDev_LcdifRk043fn_setup},
};

int setup_display_dev(hal_display_setup_t display_setup[], int display_nb,
		      const char *name, display_dev_t *dev);
int hal_display_setup(const char *name, display_dev_t *dev)
{
    return setup_display_dev(display_setup, ARRAY_SIZE(display_setup), name, dev);
}

/* Camera setup */
int HAL_CameraDev_CsiMt9m114_setup(const char *name, camera_dev_t *dev);

hal_camera_setup_t camera_setup[] =
{
    {"CsiMt9m114", HAL_CameraDev_CsiMt9m114_setup},
};

int setup_camera_dev(hal_camera_setup_t camera_setup[], int camera_nb,
		      const char *name, camera_dev_t *dev);
int hal_camera_setup(const char *name, camera_dev_t *dev)
{
  return setup_camera_dev(camera_setup, ARRAY_SIZE(camera_setup), name, dev);
}

void HAL_DCACHE_CleanInvalidateByRange(uint32_t addr, uint32_t size)
{
    DCACHE_CleanInvalidateByRange(addr, size);
    return;
}
