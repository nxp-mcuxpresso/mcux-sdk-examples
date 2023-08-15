/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
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

#define DEMO_IMG_HEIGHT         (DEMO_PANEL_HEIGHT / 2)
#define DEMO_IMG_WIDTH          (DEMO_PANEL_WIDTH / 2)
#define DEMO_IMG_BYTES_PER_LINE (DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL)

#define COLOR_RGB565_RED  0xF800U
#define COLOR_RGB565_BLUE 0x001FU

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


void DEMO_FillFrameBuffer(uint32_t frameBufferAddr, uint16_t color)
{
    uint32_t row, col;
    uint16_t(*frameBuffer)[DEMO_IMG_WIDTH] = (void *)frameBufferAddr;

    for (row = 0; row < DEMO_IMG_HEIGHT; row++)
    {
        for (col = 0; col < DEMO_IMG_WIDTH; col++)
        {
            frameBuffer[row][col] = color;
        }
    }
}

void DEMO_LCDIFV2_Alpha(void)
{
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

    const lcdifv2_buffer_config_t fbConfig = {
        .strideBytes = DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL,
        .pixelFormat = kLCDIFV2_PixelFormatRGB565,
    };

    const lcdifv2_blend_config_t blendConfig = {
        .globalAlpha = 128,
        .alphaMode   = kLCDIFV2_AlphaOverride,
    };

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    DEMO_FillFrameBuffer(DEMO_FB0_ADDR, COLOR_RGB565_RED);
    DEMO_FillFrameBuffer(DEMO_FB1_ADDR, COLOR_RGB565_BLUE);

    LCDIFV2_Init(DEMO_LCDIFV2);

    LCDIFV2_SetDisplayConfig(DEMO_LCDIFV2, &lcdifv2Config);

    LCDIFV2_EnableDisplay(DEMO_LCDIFV2, true);

    /* Layer 0 */
    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 0, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 0, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 0, 0, 0);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 0, &blendConfig);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 0, DEMO_FB0_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 0, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 0);

    /* Layer 1 */
    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 1, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 1, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 1, DEMO_PANEL_WIDTH / 4, DEMO_PANEL_HEIGHT / 4);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 1, &blendConfig);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 1, DEMO_FB1_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 1, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 1);
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
