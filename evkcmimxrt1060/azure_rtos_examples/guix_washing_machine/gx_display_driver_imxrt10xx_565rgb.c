/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include "fsl_gpio.h"
#include "clock_config.h"
#include "pin_mux.h"
#include "fsl_common.h"
#include "fsl_elcdif.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "display_support.h"

#ifdef GUIX_PXP_ENABLE
#include "fsl_pxp.h"

#define GX_PXP_OUT_COLOR_FORMAT kPXP_OutputPixelFormatRGB565
#define GX_PXP_AS_COLOR_FORMAT  kPXP_AsPixelFormatRGB565
#define GX_PXP_PS_COLOR_FORMAT  kPXP_PsPixelFormatRGB565
#endif

#include "gx_api.h"
#include "gx_display.h"

#define APP_ELCDIF         LCDIF
#define APP_LCDIF_DATA_BUS kELCDIF_DataBus16Bit

/* Display. */
#define LCD_DISP_GPIO     GPIO1
#define LCD_DISP_GPIO_PIN 2

/* Back light. */
#define LCD_BL_GPIO     GPIO2
#define LCD_BL_GPIO_PIN 31

#define DISPLAY_HEIGHT 272
#define DISPLAY_WIDTH  480

#if defined(DEMO_PANEL) && (DEMO_PANEL == DEMO_PANEL_RK043FN66HS)
#define APP_HSW          4
#define APP_HFP          8
#define APP_HBP         43
#define APP_VSW          4
#define APP_VFP          8
#define APP_VBP         12

#else
#define APP_HSW         41
#define APP_HFP          4
#define APP_HBP          8
#define APP_VSW         10
#define APP_VFP          4
#define APP_VBP          2
#endif

#define APP_POL_FLAGS \
    (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow | kELCDIF_DriveDataOnRisingClkEdge)
/* Frame buffer data alignment, for better performance, the LCDIF frame buffer should be 64B align. */

#ifndef APP_LCDIF_DATA_BUS
#define APP_LCDIF_DATA_BUS kELCDIF_DataBus16Bit
#endif

#define FRAME_BUFFER_ALIGN  64
#define FRAME_BUFFER_PIXELS (DISPLAY_HEIGHT * DISPLAY_WIDTH)

static volatile GX_BOOL gx_frame_done = GX_FALSE;
static int visible_frame              = 0;

AT_NONCACHEABLE_SECTION_ALIGN(static USHORT frame_buffer[2][FRAME_BUFFER_PIXELS], FRAME_BUFFER_ALIGN);

/* Enable interrupt. */
static void BOARD_EnableLcdInterrupt(void)
{
    EnableIRQ(LCDIF_IRQn);
}

void APP_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = ELCDIF_GetInterruptStatus(APP_ELCDIF);

    ELCDIF_ClearInterruptStatus(APP_ELCDIF, intStatus);

    if (intStatus & kELCDIF_CurFrameDone)
    {
        gx_frame_done = GX_TRUE;
    }
}

/* Configure display controller for width, height, buffer format */
static void _gx_config_lcd_mode(void)
{
    const elcdif_rgb_mode_config_t config = {
        .panelWidth    = DISPLAY_WIDTH,
        .panelHeight   = DISPLAY_HEIGHT,
        .hsw           = APP_HSW,
        .hfp           = APP_HFP,
        .hbp           = APP_HBP,
        .vsw           = APP_VSW,
        .vfp           = APP_VFP,
        .vbp           = APP_VBP,
        .polarityFlags = APP_POL_FLAGS,
        .bufferAddr    = (uint32_t)frame_buffer[visible_frame],
        .pixelFormat   = kELCDIF_PixelFormatRGB565,
        .dataBus       = APP_LCDIF_DATA_BUS,
    };

    ELCDIF_RgbModeInit(APP_ELCDIF, &config);

    BOARD_EnableLcdInterrupt();
    ELCDIF_EnableInterrupts(APP_ELCDIF, kELCDIF_CurFrameDoneInterruptEnable);
    NVIC_EnableIRQ(LCDIF_IRQn);
}

static void _gx_init_lcd_pixel_clock(void)
{
    uint32_t videoPllFreq;

    /*
     * Initialize the Video PLL.
     * Video PLL output clock is OSC24M * (loopDivider + (denominator / numerator)) / postDivider = 93MHz.
     */
    clock_video_pll_config_t config = {
        .loopDivider = 31,
        .postDivider = 8,
        .numerator   = 0,
        .denominator = 0,
    };

    CLOCK_InitVideoPll(&config);

    videoPllFreq = CLOCK_GetPllFreq(kCLOCK_PllVideo);

    if (videoPllFreq != 93000000)
    {
        while (1)
            ;
    }

    /*
     * 000 derive clock from PLL2
     * 001 derive clock from PLL3 PFD3
     * 010 derive clock from PLL5
     * 011 derive clock from PLL2 PFD0
     * 100 derive clock from PLL2 PFD1
     * 101 derive clock from PLL3 PFD1
     */
    CLOCK_SetMux(kCLOCK_LcdifPreMux, 2);
    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 4);
    CLOCK_SetDiv(kCLOCK_LcdifDiv, 1);
}

/* Initialize the backlight gpio pin */
static void _gx_init_lcd_pins(void)
{
    volatile uint32_t i = 0x100U;

    gpio_pin_config_t config = {
        kGPIO_DigitalOutput,
        0,
    };

    /* Reset the LCD. */
    GPIO_PinInit(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, &config);

    GPIO_PinWrite(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, 0);

    while (i--)
    {
    }

    GPIO_PinWrite(LCD_DISP_GPIO, LCD_DISP_GPIO_PIN, 1);

    /* Backlight. */
    config.outputLogic = 1;
    GPIO_PinInit(LCD_BL_GPIO, LCD_BL_GPIO_PIN, &config);
}

void gx_lcd_board_setup(void)
{
    /* perform LCD initialization */
    _gx_init_lcd_pixel_clock();
    _gx_init_lcd_pins();
    _gx_config_lcd_mode();
#ifdef GUIX_PXP_ENABLE
    PXP_Init(PXP);
#endif

}

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
    pxpCopyConfig.srcOffsetX      = (uint16_t)copy->gx_rectangle_left;
    pxpCopyConfig.srcOffsetY      = (uint16_t)copy->gx_rectangle_top;
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
                    dest.gx_rectangle_left, dest.gx_rectangle_top, &src);

}

#endif

/* toggle the visible and working frame buffer pointers */
static void _gx_display_driver_imxrt_buffer_toggle(GX_CANVAS *canvas, GX_RECTANGLE *dirty)
{
    GX_RECTANGLE Limit;
    GX_RECTANGLE Copy;

    gx_utility_rectangle_define(&Limit, 0, 0,
                                canvas->gx_canvas_x_resolution - 1,
                                canvas->gx_canvas_y_resolution - 1);

    // toggle the visible frame:
    visible_frame ^= 1;
    int working_frame = visible_frame ^ 0x01;

    ELCDIF_SetNextBufferAddr(LCDIF, (uint32_t)frame_buffer[visible_frame]);
    canvas->gx_canvas_memory = (ULONG *)frame_buffer[working_frame];

    // wait for hardware to finish the current frame:

    gx_frame_done = GX_FALSE;
    while (!gx_frame_done)
    {
    }

    /* copy from the newly visible frame to the new working frame */

    if (gx_utility_rectangle_overlap_detect(&Limit, &canvas->gx_canvas_dirty_area, &Copy))
    {
#ifdef GUIX_PXP_ENABLE

        _gx_pxp_bitblit(frame_buffer[visible_frame],
                        canvas->gx_canvas_x_resolution * 2,
                        frame_buffer[working_frame],
                        canvas->gx_canvas_x_resolution * 2,
                        Copy.gx_rectangle_left,
                        Copy.gx_rectangle_top,
                        &Copy);
#else
        ULONG offset;
        INT copy_width;
        INT copy_height;
        INT row;
        INT frame_pitch_offset;
        USHORT *get;
        USHORT *put;

        /* copy modified portion from visible frame to working frame */
        copy_width  = Copy.gx_rectangle_right - Copy.gx_rectangle_left + 1;
        copy_height = Copy.gx_rectangle_bottom - Copy.gx_rectangle_top + 1;

        offset = Copy.gx_rectangle_top * canvas->gx_canvas_x_resolution;
        offset += Copy.gx_rectangle_left;

        get = frame_buffer[visible_frame];
        get += offset;

        // offset by dirty rect pos
        put = (USHORT *)canvas->gx_canvas_memory;

        // offset by canvas offset:
        offset = (canvas->gx_canvas_display_offset_y + Copy.gx_rectangle_top) * canvas->gx_canvas_x_resolution;
        offset += canvas->gx_canvas_display_offset_x + Copy.gx_rectangle_left;
        put += offset;

        frame_pitch_offset = canvas->gx_canvas_x_resolution;

        for (row = 0; row < copy_height; row++)
        {
            memcpy(put, get, copy_width * 2);
            put += frame_pitch_offset;
            get += frame_pitch_offset;
        }
#endif /* GUIX_PXP_ENABLE */
    }
}

static UINT _gx_rt10xx_graphics_layer_initialize(INT layer, GX_CANVAS *canvas)
{
    int working_frame             = visible_frame ^ 0x1;
    canvas->gx_canvas_memory      = (ULONG *)frame_buffer[working_frame];
    canvas->gx_canvas_memory_size = sizeof(frame_buffer[0]);
    return GX_SUCCESS;
}

static GX_DISPLAY_LAYER_SERVICES gx_rt10xx_graphics_layer_services = {
    _gx_rt10xx_graphics_layer_initialize,
    GX_NULL, // GraphicsLayerShow,
    GX_NULL, // GraphicsLayerHide,
    GX_NULL, // GraphicsLayerAlphaSet,
    GX_NULL, // GraphicsLayerPosSet
};

UINT gx_display_driver_imxrt10xx_565rgb_setup(GX_DISPLAY *display)
{

#if defined(DEMO_PANEL) && (DEMO_PANEL == DEMO_PANEL_RK043FN66HS)
    PRINTF("LCD: RK043FN66HS\r\n");
#endif

    _gx_display_driver_565rgb_setup(display, GX_NULL, _gx_display_driver_imxrt_buffer_toggle);

#ifdef GUIX_PXP_ENABLE
    /* override functions that can leverage PXP feature */
    display->gx_display_driver_pixelmap_draw = _gx_pxp_pixelmap_draw;
    display->gx_display_driver_horizontal_line_draw = _gx_pxp_horizontal_line_draw;

    PRINTF("PXP enabled\r\n");
#endif

    // configure canvas binding functions:
    display->gx_display_layer_services = &gx_rt10xx_graphics_layer_services;

    memset(frame_buffer, 0, sizeof(frame_buffer));
    ELCDIF_RgbModeStart(LCDIF);

    return GX_SUCCESS;
}
