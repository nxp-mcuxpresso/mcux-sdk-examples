/*
 * Copyright 2020 NXP
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

/* Use ARGB4444 pixel format. */

#define DEMO_BYTE_PER_PIXEL 2

#define DEMO_IMG_HEIGHT         (DEMO_PANEL_HEIGHT / 2)
#define DEMO_IMG_WIDTH          (DEMO_PANEL_WIDTH / 2)
#define DEMO_IMG_BYTES_PER_LINE (DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL)

#define COLOR_ARGB4444_RED  0xFF00U
#define COLOR_ARGB4444_BLUE 0xF00FU

typedef struct
{
    lcdifv2_pd_blend_mode_t mode;
    char *name;
} porter_duff_mode_name_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static volatile bool s_frameDone = false;

static const porter_duff_mode_name_t s_porterDuffModeNameTable[] = {
    {kLCDIFV2_PD_Src, "kLCDIFV2_PD_Src"},         {kLCDIFV2_PD_Atop, "kLCDIFV2_PD_Atop"},
    {kLCDIFV2_PD_Over, "kLCDIFV2_PD_Over"},       {kLCDIFV2_PD_In, "kLCDIFV2_PD_In"},
    {kLCDIFV2_PD_Out, "kLCDIFV2_PD_Out"},         {kLCDIFV2_PD_Dst, "kLCDIFV2_PD_Dst"},
    {kLCDIFV2_PD_DstAtop, "kLCDIFV2_PD_DstAtop"}, {kLCDIFV2_PD_DstOver, "kLCDIFV2_PD_DstOver"},
    {kLCDIFV2_PD_DstIn, "kLCDIFV2_PD_DstIn"},     {kLCDIFV2_PD_DstOut, "kLCDIFV2_PD_DstOut"},
    {kLCDIFV2_PD_Xor, "kLCDIFV2_PD_Xor"},         {kLCDIFV2_PD_Clear, "kLCDIFV2_PD_Clear"},
};

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

void DEMO_LCDIFV2_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = LCDIFV2_GetInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID);
    LCDIFV2_ClearInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID, intStatus);

    if (0 != (intStatus & kLCDIFV2_VerticalBlankingInterrupt))
    {
        s_frameDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
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
        .pixelFormat = kLCDIFV2_PixelFormatARGB4444,
    };

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    PRINTF("Currently show both source layer and destination layer\r\n");

    DEMO_FillFrameBuffer(DEMO_FB0_ADDR, COLOR_ARGB4444_RED);
    DEMO_FillFrameBuffer(DEMO_FB1_ADDR, COLOR_ARGB4444_BLUE);

    LCDIFV2_Init(DEMO_LCDIFV2);

    LCDIFV2_SetDisplayConfig(DEMO_LCDIFV2, &lcdifv2Config);

    LCDIFV2_EnableDisplay(DEMO_LCDIFV2, true);

    EnableIRQ(DEMO_LCDIFV2_IRQn);

    LCDIFV2_EnableInterrupts(DEMO_LCDIFV2, DEMO_CORE_ID, kLCDIFV2_VerticalBlankingInterrupt);

    /* Layer 0: destination layer. */
    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 0, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 0, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 0, 0, 0);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 0, DEMO_FB0_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 0, true);

    /* Layer 1: source layer. */
    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 1, &fbConfig);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 1, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 1, DEMO_PANEL_WIDTH / 4, DEMO_PANEL_HEIGHT / 4);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 1, DEMO_FB1_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 1, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 0);
    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 1);
}

void DEMO_LCDIFV2_PorterDuff(void)
{
    lcdifv2_blend_config_t srcLayerBlendConfig;
    lcdifv2_blend_config_t destLayerBlendConfig;
    lcdifv2_pd_blend_mode_t blendMode;

    memset(&srcLayerBlendConfig, 0, sizeof(srcLayerBlendConfig));
    memset(&destLayerBlendConfig, 0, sizeof(destLayerBlendConfig));

    /*
     * Don't need to set srcLayerBlendConfig.globalAlpha and
     * destLayerBlendConfig.globalAlpha in this example,
     * because the srcLayerBlendConfig.pdGlobalAlphaMode and
     * destLayerBlendConfig.pdGlobalAlphaMode are set to
     * kLCDIFV2_PD_LocalAlpha by LCDIFV2_GetPorterDuffConfig,
     * with this mode, the globalAlpha is not used, and
     * the alpha within pixel is used.
     */

    uint8_t modeIndex = 0;

    for (;;)
    {
        /* Delay at least 2s to show next mode. */
        for (uint32_t i = 0; i < 2; i++)
        {
            SDK_DelayAtLeastUs(1000 * 1000, SystemCoreClock);
        }

        /*
         * Wait for previous frame complete.
         * New frame buffer configuration load at the next VSYNC.
         */
        s_frameDone = false;
        while (!s_frameDone)
        {
        }

        if (modeIndex >= ARRAY_SIZE(s_porterDuffModeNameTable))
        {
            modeIndex = 0;
        }

        blendMode = s_porterDuffModeNameTable[modeIndex].mode;

        PRINTF("Will show %s mode...\r\n", s_porterDuffModeNameTable[modeIndex].name);

        LCDIFV2_GetPorterDuffConfig(blendMode, kLCDIFV2_PD_SrcLayer, &srcLayerBlendConfig);

        LCDIFV2_GetPorterDuffConfig(blendMode, kLCDIFV2_PD_DestLayer, &destLayerBlendConfig);

        LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 0, &destLayerBlendConfig);

        LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 1, &srcLayerBlendConfig);

        LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 0);

        LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 1);

        modeIndex++;
    }
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

    PRINTF("LCDIF v2 porter duff example start...\r\n");

    DEMO_LCDIFV2_Init();

    DEMO_LCDIFV2_PorterDuff();

    while (1)
    {
    }
}
