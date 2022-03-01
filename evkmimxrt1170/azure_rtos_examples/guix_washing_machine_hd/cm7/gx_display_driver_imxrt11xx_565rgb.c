/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "display_support.h"
#include "fsl_debug_console.h"

#include "gx_api.h"
#include "gx_display.h"
#include "gx_canvas.h"

#define DEMO_FB_SIZE                                                                                    \
    (((DEMO_BUFFER_WIDTH * DEMO_BUFFER_HEIGHT * DEMO_BUFFER_BYTE_PER_PIXEL) + FRAME_BUFFER_ALIGN - 1) & \
     ~(FRAME_BUFFER_ALIGN - 1))

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_frameBuffer[DEMO_FB_SIZE], FRAME_BUFFER_ALIGN);

static volatile bool gx_frame_done = true;

static GX_CANVAS canvas_frame;

static void imxrt_display_callback(void *param, void *inactiveBuffer)
{
    gx_frame_done = true;
}

static UINT imxrt_display_layer_initialize(INT layer, GX_CANVAS *canvas)
{
    dc_fb_info_t fbInfo;

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    fbInfo.pixelFormat = DEMO_BUFFER_PIXEL_FORMAT;
    fbInfo.width       = DEMO_BUFFER_WIDTH;
    fbInfo.height      = DEMO_BUFFER_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = DEMO_BUFFER_STRIDE_BYTE;
    g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo);

    g_dc.ops->setCallback(&g_dc, 0, imxrt_display_callback, NULL);

    memset(s_frameBuffer, 0, DEMO_FB_SIZE);
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_frameBuffer);

    g_dc.ops->enableLayer(&g_dc, 0);

    return GX_SUCCESS;
}

static VOID imxrt_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
    GX_DISPLAY *display = canvas->gx_canvas_display;

    if (canvas == NULL || dirty == NULL)
        return;

    gx_frame_done = false;
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_frameBuffer);

    while (!gx_frame_done)
    {
    }

    _gx_canvas_dirty_mark(&canvas_frame, dirty);
    display->gx_display_driver_canvas_copy(canvas, &canvas_frame);
}

static GX_DISPLAY_LAYER_SERVICES gx_imxrt_graphics_layer_services = {
        .gx_display_layer_initialize = imxrt_display_layer_initialize,
};

UINT gx_display_driver_imxrt11xx_565rgb_setup(GX_DISPLAY *display)
{
    status_t status;

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed!\r\n");
        assert(false);
    }

    _gx_display_driver_565rgb_setup(display, GX_NULL, imxrt_buffer_toggle);

    display->gx_display_layer_services = &gx_imxrt_graphics_layer_services;

    gx_canvas_create(&canvas_frame, "main_buffer", display, GX_CANVAS_SIMPLE,
            DEMO_BUFFER_WIDTH, DEMO_BUFFER_HEIGHT, (GX_COLOR *)s_frameBuffer,
            DEMO_FB_SIZE);

    return GX_SUCCESS;
}
