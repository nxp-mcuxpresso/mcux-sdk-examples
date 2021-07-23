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
#define DEMO_CORE_ID 1

#define DEMO_BYTE_PER_PIXEL 2U

#define DEMO_IMG_HEIGHT         DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH          DEMO_PANEL_WIDTH
#define DEMO_IMG_BYTES_PER_LINE (DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL)

/* Use layer 0 in this example. */
#define DEMO_LCDIFV2_LAYER 0

#define DEMO_MAKE_YU(y, u) ((y << 0) | (u << 8))
#define DEMO_MAKE_YV(y, v) ((y << 0) | (v << 8))

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t s_frameBufferAddr = DEMO_FB0_ADDR;

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


void DEMO_FillFrameBuffer(uint32_t frameBufferAddr)
{
    uint32_t row, col;
    uint16_t(*frameBuffer)[DEMO_IMG_WIDTH] = (void *)frameBufferAddr;

    /* RED */
    for (row = 0; row < DEMO_IMG_HEIGHT / 4; row++)
    {
        for (col = 0; col < DEMO_IMG_WIDTH;)
        {
            frameBuffer[row][col++] = DEMO_MAKE_YU(76, 84);
            frameBuffer[row][col++] = DEMO_MAKE_YV(76, 255);
        }
    }

    /* GREEN */
    for (; row < DEMO_IMG_HEIGHT / 2; row++)
    {
        for (col = 0; col < DEMO_IMG_WIDTH;)
        {
            frameBuffer[row][col++] = DEMO_MAKE_YU(149, 43);
            frameBuffer[row][col++] = DEMO_MAKE_YV(149, 21);
        }
    }

    /* BLUE */
    for (; row < DEMO_IMG_HEIGHT * 3 / 4; row++)
    {
        for (col = 0; col < DEMO_IMG_WIDTH;)
        {
            frameBuffer[row][col++] = DEMO_MAKE_YU(29, 255);
            frameBuffer[row][col++] = DEMO_MAKE_YV(29, 107);
        }
    }

    /* WHITE */
    for (; row < DEMO_IMG_HEIGHT; row++)
    {
        for (col = 0; col < DEMO_IMG_WIDTH;)
        {
            frameBuffer[row][col++] = DEMO_MAKE_YU(255, 128);
            frameBuffer[row][col++] = DEMO_MAKE_YV(255, 128);
        }
    }
}

void DEMO_LCDIFV2_Init(void)
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
        .pixelFormat = kLCDIFV2_PixelFormatYUYV,
    };

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    LCDIFV2_Init(DEMO_LCDIFV2);

    LCDIFV2_SetDisplayConfig(DEMO_LCDIFV2, &lcdifv2Config);

    LCDIFV2_EnableDisplay(DEMO_LCDIFV2, true);

    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, 0, 0);

    LCDIFV2_SetCscMode(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, kLCDIFV2_CscYCbCr2RGB);

    DEMO_FillFrameBuffer(s_frameBufferAddr);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, true);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER, s_frameBufferAddr);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, DEMO_LCDIFV2_LAYER);
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

    PRINTF("LCDIF v2 CSC example start...\r\n");

    DEMO_LCDIFV2_Init();

    PRINTF("LCDIF v2 CSC example finished...\r\n");

    while (1)
    {
    }
}
