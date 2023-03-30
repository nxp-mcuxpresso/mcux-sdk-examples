/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_common.h"
#include "display_support.h"
#include "fsl_debug_console.h"
#include "fsl_cache.h"

#ifdef GUIX_PXP_ENABLE
#include "fsl_pxp.h"

#define GX_PXP_OUT_COLOR_FORMAT kPXP_OutputPixelFormatRGB565
#define GX_PXP_AS_COLOR_FORMAT  kPXP_AsPixelFormatRGB565
#define GX_PXP_PS_COLOR_FORMAT  kPXP_PsPixelFormatRGB565
#endif

#include "gx_api.h"
#include "gx_display.h"
#include "gx_canvas.h"

#define DEMO_FB_SIZE                                                                                    \
    (((DEMO_BUFFER_WIDTH * DEMO_BUFFER_HEIGHT * DEMO_BUFFER_BYTE_PER_PIXEL) + FRAME_BUFFER_ALIGN - 1) & \
     ~(FRAME_BUFFER_ALIGN - 1))

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_frameBuffer[DEMO_FB_SIZE], FRAME_BUFFER_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t main_display_canvas_memory[DEMO_FB_SIZE], FRAME_BUFFER_ALIGN);

static volatile bool gx_frame_done = true;

static GX_CANVAS canvas_frame;

#ifdef GUIX_PXP_ENABLE

static void _gx_pxp_wait_complete()
{
    /* Wait for process complete. */
    while (!(kPXP_CompleteFlag & PXP_GetStatusFlags(PXP)))
    {
    }

    PXP_ClearStatusFlags(PXP, kPXP_CompleteFlag);
}

static void _gx_pxp_run(void)
{
    PXP_Start(PXP);
    _gx_pxp_wait_complete();
}

static ULONG _gx_convert_565_to_32bpp(GX_COLOR color)
{
    ULONG out = 0xff000000;
    out |= (color & 0xf800) << 8;
    out |= (color & 0x07e0) << 5;
    out += (color & 0x001f) << 3;
    return out;
}

static void _gx_pxp_bitblit(USHORT *src_base,
                            USHORT src_stride,
                            USHORT *dest_base,
                            USHORT dest_stride,
                            GX_VALUE x_dest,
                            GX_VALUE y_dest,
                            GX_RECTANGLE *copy)
{
    pxp_pic_copy_config_t pxpCopyConfig;

    pxpCopyConfig.srcPicBaseAddr  = (uint32_t)src_base;
    pxpCopyConfig.srcPitchBytes   = (uint16_t)src_stride;
    pxpCopyConfig.srcOffsetX      = copy->gx_rectangle_left;
    pxpCopyConfig.srcOffsetY      = copy->gx_rectangle_top;
    pxpCopyConfig.destPicBaseAddr = (uint32_t)dest_base;
    pxpCopyConfig.destPitchBytes  = (uint16_t)dest_stride;
    pxpCopyConfig.destOffsetX     = (uint16_t)x_dest;
    pxpCopyConfig.destOffsetY     = (uint16_t)y_dest;
    pxpCopyConfig.width           = (uint16_t)(copy->gx_rectangle_right
                                               - copy->gx_rectangle_left + 1);
    pxpCopyConfig.height          = (uint16_t)(copy->gx_rectangle_bottom
                                               - copy->gx_rectangle_top + 1);
    pxpCopyConfig.pixelFormat     = GX_PXP_AS_COLOR_FORMAT;

    PXP_StartPictureCopy(PXP, &pxpCopyConfig);
    _gx_pxp_wait_complete();
}

/**************************************************************************/
/*  _gx_pxp_rectangle_fill                                                */
/*    GUIX draws rectangles (at the driver leve) by invoking a wide       */
/*    horizontal line. For very large fills, utilize PXP to improve       */
/*    performance.                                                        */
/**************************************************************************/
void _gx_pxp_rectangle_fill(GX_DRAW_CONTEXT *context, INT xstart, INT xend,
                            INT ypos, INT width, GX_COLOR color)
{
    USHORT *rowstart;

    PXP_Init(PXP);
    PXP_EnableCsc1(PXP, GX_FALSE);                      /* Disable CSC1     */
    PXP_SetProcessBlockSize(PXP, kPXP_BlockSize16);     /* Block size 16x16 */

    /* pick up start address of canvas memory */
    rowstart = (USHORT *)context->gx_draw_context_memory;

    /* calculate start of row address */
    rowstart += context->gx_draw_context_pitch * ypos;

    /* calculate pixel address */
    rowstart += xstart;

    /* output buffer configure */
    pxp_output_buffer_config_t outputConfig = {
        .pixelFormat    = GX_PXP_OUT_COLOR_FORMAT,
        .interlacedMode = kPXP_OutputProgressive,
        .buffer0Addr    = (uint32_t)rowstart,
        .buffer1Addr    = (uint32_t)GX_NULL,
        .pitchBytes     = context->gx_draw_context_pitch * 2,
        .width          = xend - xstart + 1,
        .height         = width,
    };

    PXP_SetOutputBufferConfig(PXP, &outputConfig);

    if(context->gx_draw_context_brush.gx_brush_alpha == GX_ALPHA_VALUE_OPAQUE)
    {
        /* No alpha, disable Alpha Surface, us Process Surface as color generator */
        PXP_SetAlphaSurfacePosition(PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);      /* Disable AS. */
        PXP_SetProcessSurfacePosition(PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);    /* Disable PS. */
        PXP_SetProcessSurfaceBackGroundColor(PXP, _gx_convert_565_to_32bpp(color));
    }
    else
    {
        /* Fill with alpha - Alpha Surface used as source, PS used as color generator */
        pxp_as_buffer_config_t asBufferConfig;
        pxp_porter_duff_config_t pdConfig;

        /* Set AS to OUT */
        asBufferConfig.pixelFormat = GX_PXP_AS_COLOR_FORMAT;
        asBufferConfig.bufferAddr  = (uint32_t)outputConfig.buffer0Addr;
        asBufferConfig.pitchBytes  = outputConfig.pitchBytes;

        PXP_SetAlphaSurfaceBufferConfig(PXP, &asBufferConfig);
        PXP_SetAlphaSurfacePosition(PXP, 0U, 0U, xend - xstart + 1,  width);

        /* Disable Process Surface, use as color source */
        PXP_SetProcessSurfacePosition(PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);
        PXP_SetProcessSurfaceBackGroundColor(PXP, _gx_convert_565_to_32bpp(color));

        /* Configure Porter-Duff blending - For RGB 565 format */
        pdConfig.enable = 1;
        pdConfig.dstColorMode = kPXP_PorterDuffColorStraight;
        pdConfig.srcColorMode = kPXP_PorterDuffColorStraight;
        pdConfig.dstGlobalAlphaMode = kPXP_PorterDuffGlobalAlpha;
        pdConfig.srcGlobalAlphaMode = kPXP_PorterDuffGlobalAlpha;
        pdConfig.srcFactorMode = kPXP_PorterDuffFactorStraight;
        pdConfig.dstFactorMode = kPXP_PorterDuffFactorStraight;
        pdConfig.srcGlobalAlpha = context->gx_draw_context_brush.gx_brush_alpha;
        pdConfig.dstGlobalAlpha = 255 - context->gx_draw_context_brush.gx_brush_alpha;
        pdConfig.srcAlphaMode = kPXP_PorterDuffAlphaStraight;
        pdConfig.dstAlphaMode = kPXP_PorterDuffAlphaStraight;
        PXP_SetPorterDuffConfig(PXP, &pdConfig);
    }

    _gx_pxp_run(); /* Go */
}

/**************************************************************************/
/*  _gx_pxp_horizontal_line_draw                                          */
/*    Override generic function to utilize PXP                            */
/**************************************************************************/
static VOID _gx_pxp_horizontal_line_draw(GX_DRAW_CONTEXT *context,
                                         INT xstart, INT xend,
                                         INT ypos, INT width, GX_COLOR color)
{
    if (width <= 4)
    {
        /* invoke generic software implementation until can be optimized to use PXP */
        _gx_display_driver_16bpp_horizontal_line_draw(context, xstart, xend,
                                                      ypos, width, color);
    }
    else
    {
        _gx_pxp_rectangle_fill(context, xstart, xend, ypos, width, color);
    }
}
/**************************************************************************/
/*  _gx_pxp_pixelmap_draw                                                 */
/*    Override generic function to utilize PXP                            */
/**************************************************************************/
static void _gx_pxp_pixelmap_draw(GX_DRAW_CONTEXT *context,
                                  INT xpos,
                                  INT ypos,
                                  GX_PIXELMAP *pixelmap)
{
    GX_RECTANGLE *clip;
    GX_CANVAS *canvas;
    GX_RECTANGLE src;
    GX_RECTANGLE dest;

    /* if the pixelmap is compressed, we must draw in software */
    if (pixelmap->gx_pixelmap_flags & GX_PIXELMAP_COMPRESSED)
    {
        _gx_display_driver_565rgb_pixelmap_draw(context, xpos, ypos, pixelmap);
        return;
    }

    /* if the pixelmap is in 565 format with alpha channel, we must draw in software */
    if (pixelmap->gx_pixelmap_flags & GX_PIXELMAP_ALPHA ||
        pixelmap->gx_pixelmap_format != GX_COLOR_FORMAT_565RGB)
    {
        _gx_display_driver_565rgb_pixelmap_draw(context, xpos, ypos, pixelmap);
        return;
    }


    dest.gx_rectangle_left = xpos;
    dest.gx_rectangle_top = ypos;
    dest.gx_rectangle_right = xpos + pixelmap->gx_pixelmap_width - 1;
    dest.gx_rectangle_bottom = ypos + pixelmap->gx_pixelmap_height - 1;

    clip = context->gx_draw_context_clip;
    gx_utility_rectangle_overlap_detect(clip, &dest, &dest);
    src = dest;
    gx_utility_rectangle_shift(&src, -xpos, -ypos);

    canvas = context->gx_draw_context_canvas;

    _gx_pxp_bitblit((USHORT *) pixelmap->gx_pixelmap_data,
                    pixelmap->gx_pixelmap_width * 2,
                    (USHORT *) canvas->gx_canvas_memory,
                    canvas->gx_canvas_x_resolution * 2,
                    //src.gx_rectangle_left, src.gx_rectangle_top,
                    dest.gx_rectangle_left, dest.gx_rectangle_top,
                    &src);

}

#endif

static void imxrt_display_callback(void *param, void *inactiveBuffer)
{
    gx_frame_done = true;
}

static UINT imxrt_display_layer_initialize(INT layer, GX_CANVAS *canvas)
{
    dc_fb_info_t fbInfo;

    canvas->gx_canvas_memory      = (ULONG *)main_display_canvas_memory;
    canvas->gx_canvas_memory_size = sizeof(main_display_canvas_memory);

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
    if (canvas == NULL || dirty == NULL)
        return;

    gx_frame_done = false;
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_frameBuffer);

    while (!gx_frame_done)
    {
    }

    _gx_canvas_dirty_mark(&canvas_frame, dirty);

#ifdef GUIX_PXP_ENABLE
    GX_RECTANGLE Limit;
    GX_RECTANGLE overlap;

    gx_utility_rectangle_define(&Limit, 0, 0,
                                canvas->gx_canvas_x_resolution - 1,
                                canvas->gx_canvas_y_resolution - 1);
    gx_utility_rectangle_shift(&Limit, canvas->gx_canvas_display_offset_x,
                               canvas->gx_canvas_display_offset_y);

    if (gx_utility_rectangle_overlap_detect(&Limit, &canvas_frame.gx_canvas_dirty_area, &overlap))
    {
        _gx_pxp_bitblit((USHORT *)canvas->gx_canvas_memory,
                canvas->gx_canvas_x_resolution * 2,
                (USHORT *)canvas_frame.gx_canvas_memory,
                canvas_frame.gx_canvas_x_resolution * 2,
                overlap.gx_rectangle_left,
                overlap.gx_rectangle_top,
                &overlap);
    }
#else
    GX_DISPLAY *display = canvas->gx_canvas_display;

    display->gx_display_driver_canvas_copy(canvas, &canvas_frame);
#endif
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

#ifdef GUIX_PXP_ENABLE
    /* override functions that can leverage PXP feature */
    display->gx_display_driver_pixelmap_draw        = _gx_pxp_pixelmap_draw;
    display->gx_display_driver_horizontal_line_draw = _gx_pxp_horizontal_line_draw;

    PRINTF("PXP enabled\r\n");
#endif

    display->gx_display_layer_services = &gx_imxrt_graphics_layer_services;

    gx_canvas_create(&canvas_frame, "main_buffer", display, GX_CANVAS_SIMPLE,
            DEMO_BUFFER_WIDTH, DEMO_BUFFER_HEIGHT, (GX_COLOR *)s_frameBuffer,
            DEMO_FB_SIZE);

    return GX_SUCCESS;
}
