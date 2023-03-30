/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lcdifv2.h"
#include "lcdifv2_support.h"
#include "fsl_debug_console.h"

#include "fsl_soc_src.h"
#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_CORE_ID 0

#define DEMO_BYTE_PER_PIXEL 2

#define DEMO_ALPHA_LAYER_HEIGHT (DEMO_PANEL_HEIGHT / 4)
#define DEMO_ALPHA_LAYER_WIDTH  (DEMO_PANEL_WIDTH / 2)
#define DEMO_ALPHA_LAYER_STRIDE (DEMO_ALPHA_LAYER_WIDTH * DEMO_BYTE_PER_PIXEL)

#define COLOR_RGB565_RED   0xF800U
#define COLOR_RGB565_BLUE  0x001FU
#define COLOR_RGB565_GREEN 0x07E0U

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_ResetDisplayMix(void)
{
    /*
     * Reset the displaymix, otherwise during debugging, the
     * debugger may not reset the display, then the behavior
     * is not right.
     */
    SRC_AssertSliceSoftwareReset(SRC, kSRC_DisplaySlice);
    while (kSRC_SliceResetInProcess == SRC_GetSliceResetState(SRC, kSRC_DisplaySlice))
    {
    }
}


void DEMO_FillFrameBuffer(
    uint32_t frameBufferAddr, uint16_t width, uint16_t height, uint32_t strideBytes, uint16_t color)
{
    uint32_t row, col;

    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            ((uint16_t *)(frameBufferAddr + strideBytes * row))[col] = color;
        }
    }
}

void DEMO_LCDIFV2_InitLayer(uint8_t layerIndex,
                            uint16_t width,
                            uint16_t height,
                            uint32_t strideBytes,
                            uint16_t offsetX,
                            uint16_t offsetY,
                            uint8_t globalAlpha,
                            uint32_t fbAddr)
{
    const lcdifv2_buffer_config_t fbConfig = {
        .strideBytes = strideBytes,
        .pixelFormat = kLCDIFV2_PixelFormatRGB565,
    };

    const lcdifv2_blend_config_t blendConfig = {
        .globalAlpha = globalAlpha,
        .alphaMode   = kLCDIFV2_AlphaOverride,
    };

    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, layerIndex, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, layerIndex, width, height);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, layerIndex, offsetX, offsetY);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, layerIndex, &blendConfig);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, layerIndex, fbAddr);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, layerIndex, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, layerIndex);
}

void DEMO_LCDIFV2_Alpha(void)
{
    uint8_t alpha[] = {31, 31, 31, 31, 31, 31, 31, 31};
    uint8_t globalAlpha[8];

    const lcdifv2_display_config_t lcdifv2Config = {
        .panelWidth    = DEMO_PANEL_WIDTH,
        .panelHeight   = DEMO_PANEL_HEIGHT,
        .hsw           = DEMO_HSW,
        .hfp           = DEMO_HFP,
        .hbp           = DEMO_HBP,
        .vsw           = DEMO_VSW,
        .vfp           = DEMO_VFP,
        .vbp           = DEMO_VBP,
        .polarityFlags = DEMO_POL_FLAGS,
        .lineOrder     = kLCDIFV2_LineOrderRGB,
    };

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    if (kStatus_Success != LCDIFV2_GetMultiLayerGlobalAlpha(alpha, globalAlpha, ARRAY_SIZE(alpha)))
    {
        PRINTF("Can't get valid global alpha value\r\n");

        while (1)
        {
        }
    }

    DEMO_FillFrameBuffer(DEMO_FB0_ADDR, DEMO_PANEL_WIDTH, DEMO_PANEL_HEIGHT, DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL,
                         COLOR_RGB565_BLUE);
    DEMO_FillFrameBuffer(DEMO_FB1_ADDR, DEMO_ALPHA_LAYER_WIDTH, DEMO_ALPHA_LAYER_HEIGHT,
                         DEMO_ALPHA_LAYER_WIDTH * DEMO_BYTE_PER_PIXEL, COLOR_RGB565_GREEN);

    LCDIFV2_Init(DEMO_LCDIFV2);

    LCDIFV2_SetDisplayConfig(DEMO_LCDIFV2, &lcdifv2Config);

    LCDIFV2_EnableDisplay(DEMO_LCDIFV2, true);

    /* Layer 0 */
    DEMO_LCDIFV2_InitLayer(0,                                      /* layerIndex  */
                           DEMO_PANEL_WIDTH,                       /* width       */
                           DEMO_PANEL_HEIGHT,                      /* height      */
                           DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL, /* strideBytes */
                           0,                                      /* offsetX     */
                           0,                                      /* offsetY     */
                           globalAlpha[0],                         /* globalAlpha */
                           DEMO_FB0_ADDR);                         /* fbAddr      */

    /* Layer 1 */
    DEMO_LCDIFV2_InitLayer(1,                       /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,  /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT, /* height      */
                           DEMO_ALPHA_LAYER_STRIDE, /* strideBytes */
                           0,                       /* offsetX     */
                           0,                       /* offsetY     */
                           globalAlpha[1],          /* globalAlpha */
                           DEMO_FB1_ADDR);          /* fbAddr      */

    /* Layer 2 */
    DEMO_LCDIFV2_InitLayer(2,                       /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,  /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT, /* height      */
                           DEMO_ALPHA_LAYER_STRIDE, /* strideBytes */
                           DEMO_PANEL_WIDTH / 2,    /* offsetX     */
                           0,                       /* offsetY     */
                           globalAlpha[2],          /* globalAlpha */
                           DEMO_FB1_ADDR);          /* fbAddr      */

    /* Layer 3 */
    DEMO_LCDIFV2_InitLayer(3,                       /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,  /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT, /* height      */
                           DEMO_ALPHA_LAYER_STRIDE, /* strideBytes */
                           0,                       /* offsetX     */
                           DEMO_PANEL_HEIGHT / 4,   /* offsetY     */
                           globalAlpha[3],          /* globalAlpha */
                           DEMO_FB1_ADDR);          /* fbAddr      */

    /* Layer 4 */
    DEMO_LCDIFV2_InitLayer(4,                       /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,  /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT, /* height      */
                           DEMO_ALPHA_LAYER_STRIDE, /* strideBytes */
                           DEMO_PANEL_WIDTH / 2,    /* offsetX     */
                           DEMO_PANEL_HEIGHT / 4,   /* offsetY     */
                           globalAlpha[4],          /* globalAlpha */
                           DEMO_FB1_ADDR);          /* fbAddr      */

    /* Layer 5 */
    DEMO_LCDIFV2_InitLayer(5,                         /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,    /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT,   /* height      */
                           DEMO_ALPHA_LAYER_STRIDE,   /* strideBytes */
                           0,                         /* offsetX     */
                           2 * DEMO_PANEL_HEIGHT / 4, /* offsetY     */
                           globalAlpha[5],            /* globalAlpha */
                           DEMO_FB1_ADDR);            /* fbAddr      */

    /* Layer 6 */
    DEMO_LCDIFV2_InitLayer(6,                         /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,    /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT,   /* height      */
                           DEMO_ALPHA_LAYER_STRIDE,   /* strideBytes */
                           DEMO_PANEL_WIDTH / 2,      /* offsetX     */
                           2 * DEMO_PANEL_HEIGHT / 4, /* offsetY     */
                           globalAlpha[6],            /* globalAlpha */
                           DEMO_FB1_ADDR);            /* fbAddr      */

    /* Layer 7 */
    DEMO_LCDIFV2_InitLayer(7,                         /* layerIndex  */
                           DEMO_ALPHA_LAYER_WIDTH,    /* width       */
                           DEMO_ALPHA_LAYER_HEIGHT,   /* height      */
                           DEMO_ALPHA_LAYER_STRIDE,   /* strideBytes */
                           0,                         /* offsetX     */
                           3 * DEMO_PANEL_HEIGHT / 4, /* offsetY     */
                           globalAlpha[7],            /* globalAlpha */
                           DEMO_FB1_ADDR);            /* fbAddr      */
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    BOARD_ResetDisplayMix();
    BOARD_InitLpuartPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitDebugConsole();
    BOARD_InitLcdifClock();

    PRINTF("LCDIF v2 alpha example start...\r\n");

    DEMO_LCDIFV2_Alpha();

    PRINTF("LCDIF v2 alpha example finished...\r\n");

    while (1)
    {
    }
}
