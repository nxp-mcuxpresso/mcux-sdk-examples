/*
 * Copyright 2020 NXP
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
#define DEMO_CORE_ID 1

#define DEMO_RGB565_BPP   2
#define DEMO_ARGB4444_BPP 2
#define DEMO_ARGB8888_BPP 4

/* The image is a square block. */
#if (DEMO_PANEL_HEIGHT > DEMO_PANEL_WIDTH)
#define DEMO_IMG_WIDTH (DEMO_PANEL_WIDTH / 2)
#else
#define DEMO_IMG_WIDTH (DEMO_PANEL_HEIGHT / 2)
#endif
#define DEMO_IMG_HEIGHT DEMO_IMG_WIDTH

#define DEMO_IMG_STRIDE_RGB565   (DEMO_PANEL_WIDTH * DEMO_RGB565_BPP)
#define DEMO_IMG_STRIDE_ARGB4444 (DEMO_PANEL_WIDTH * DEMO_ARGB4444_BPP)
#define DEMO_IMG_STRIDE_ARGB8888 (DEMO_PANEL_WIDTH * DEMO_ARGB8888_BPP)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint16_t s_layer0OffsetX = 0;
static uint16_t s_layer0OffsetY = 0;
static uint16_t s_layer1OffsetX = (DEMO_IMG_WIDTH / 2);
static uint16_t s_layer1OffsetY = (DEMO_IMG_HEIGHT / 2);
static uint16_t s_layer2OffsetX = (DEMO_IMG_WIDTH);
static uint16_t s_layer2OffsetY = (DEMO_IMG_HEIGHT);

static int16_t s_layer0OffsetIncX = 1;
static int16_t s_layer0OffsetIncY = 1;
static int16_t s_layer1OffsetIncX = 1;
static int16_t s_layer1OffsetIncY = 1;
static int16_t s_layer2OffsetIncX = 1;
static int16_t s_layer2OffsetIncY = 1;

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
}


void DEMO_FillFrameBuffer(void)
{
    uint32_t x, y;
    uint32_t fb;

    /* Layer 0: RGB565, BLUE. */
    fb = DEMO_FB0_ADDR;
    for (y = 0; y < DEMO_IMG_HEIGHT; y++)
    {
        for (x = 0; x < DEMO_IMG_WIDTH; x++)
        {
            *((uint16_t *)(fb + (x * DEMO_RGB565_BPP))) = 0x001FU;
        }

        fb += DEMO_IMG_STRIDE_RGB565;
    }

    /* Layer 1: ARGB4444, RED with alpha 50% */
    fb = DEMO_FB1_ADDR;
    for (y = 0; y < DEMO_IMG_HEIGHT; y++)
    {
        for (x = 0; x < DEMO_IMG_WIDTH; x++)
        {
            *((uint16_t *)(fb + (x * DEMO_ARGB4444_BPP))) = 0x8F00U;
        }

        fb += DEMO_IMG_STRIDE_ARGB4444;
    }

    /* Layer 2: ARGB8888, GREEN with alpha 25% */
    fb = DEMO_FB2_ADDR;
    for (y = 0; y < DEMO_IMG_HEIGHT; y++)
    {
        for (x = 0; x < DEMO_IMG_WIDTH; x++)
        {
            *((uint32_t *)(fb + (x * DEMO_ARGB8888_BPP))) = 0x4F00FF00U;
        }

        fb += DEMO_IMG_STRIDE_ARGB8888;
    }
}

void DEMO_UpdateLayerOffset(uint16_t *offsetX, uint16_t *offsetY, int16_t *offsetIncX, int16_t *offsetIncY)
{
    if (*offsetX == 0U)
    {
        *offsetIncX = 1;
    }
    else if (*offsetX + DEMO_IMG_WIDTH >= DEMO_PANEL_WIDTH - 1)
    {
        *offsetX    = DEMO_PANEL_WIDTH - DEMO_IMG_WIDTH - 1;
        *offsetIncX = -1;
    }

    if (*offsetY == 0U)
    {
        *offsetIncY = 1;
    }
    else if (*offsetY + DEMO_IMG_HEIGHT >= DEMO_PANEL_HEIGHT - 1)
    {
        *offsetY    = DEMO_PANEL_HEIGHT - DEMO_IMG_HEIGHT - 1;
        *offsetIncY = -1;
    }

    *offsetX += *offsetIncX;
    *offsetY += *offsetIncY;
}

void DEMO_LCDIFV2_AlphaInit(void)
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

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    DEMO_FillFrameBuffer();

    LCDIFV2_Init(DEMO_LCDIFV2);

    LCDIFV2_SetDisplayConfig(DEMO_LCDIFV2, &lcdifv2Config);

    LCDIFV2_EnableDisplay(DEMO_LCDIFV2, true);

    /* Layer 0: RGB565, global alpha 50% */
    const lcdifv2_buffer_config_t fb0Config = {
        .strideBytes = DEMO_IMG_STRIDE_RGB565,
        .pixelFormat = kLCDIFV2_PixelFormatRGB565,
    };

    const lcdifv2_blend_config_t blend0Config = {
        .globalAlpha = 128,
        .alphaMode   = kLCDIFV2_AlphaOverride,
    };

    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 0, &fb0Config);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 0, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 0, s_layer0OffsetX, s_layer0OffsetY);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 0, &blend0Config);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 0, DEMO_FB0_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 0, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 0);

    /* Layer 1: ARGB4444 */
    const lcdifv2_buffer_config_t fb1Config = {
        .strideBytes = DEMO_IMG_STRIDE_ARGB4444,
        .pixelFormat = kLCDIFV2_PixelFormatARGB4444,
    };

    const lcdifv2_blend_config_t blend1Config = {
        .alphaMode = kLCDIFV2_AlphaEmbedded,
    };

    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 1, &fb1Config);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 1, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 1, s_layer1OffsetX, s_layer1OffsetY);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 1, &blend1Config);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 1, DEMO_FB1_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 1, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 1);

    /* Layer 2: ARGB8888 */
    const lcdifv2_buffer_config_t fb2Config = {
        .strideBytes = DEMO_IMG_STRIDE_ARGB8888,
        .pixelFormat = kLCDIFV2_PixelFormatARGB8888,
    };

    const lcdifv2_blend_config_t blend2Config = {
        .alphaMode = kLCDIFV2_AlphaEmbedded,
    };

    LCDIFV2_SetLayerBufferConfig(DEMO_LCDIFV2, 2, &fb2Config);

    LCDIFV2_SetLayerSize(DEMO_LCDIFV2, 2, DEMO_IMG_WIDTH, DEMO_IMG_HEIGHT);

    LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 2, s_layer2OffsetX, s_layer2OffsetY);

    LCDIFV2_SetLayerBlendConfig(DEMO_LCDIFV2, 2, &blend2Config);

    LCDIFV2_SetLayerBufferAddr(DEMO_LCDIFV2, 2, DEMO_FB2_ADDR);

    LCDIFV2_EnableLayer(DEMO_LCDIFV2, 2, true);

    LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 2);
}

void DEMO_LCDIFV2_AlphaRun(void)
{
    uint32_t intStatus;

    intStatus = LCDIFV2_GetInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID);
    LCDIFV2_ClearInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID, intStatus);

    while (1)
    {
        DEMO_UpdateLayerOffset(&s_layer0OffsetX, &s_layer0OffsetY, &s_layer0OffsetIncX, &s_layer0OffsetIncY);
        DEMO_UpdateLayerOffset(&s_layer1OffsetX, &s_layer1OffsetY, &s_layer1OffsetIncX, &s_layer1OffsetIncY);
        DEMO_UpdateLayerOffset(&s_layer2OffsetX, &s_layer2OffsetY, &s_layer2OffsetIncX, &s_layer2OffsetIncY);

        /* Wait to make sure previous configuration takes effect, the configuration
         * is loaded at vertical blanking.
         */
        while (1)
        {
            intStatus = LCDIFV2_GetInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID);
            if (0 != (intStatus & kLCDIFV2_VerticalBlankingInterrupt))
            {
                LCDIFV2_ClearInterruptStatus(DEMO_LCDIFV2, DEMO_CORE_ID, kLCDIFV2_VerticalBlankingInterrupt);
                break;
            }
        }

        LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 0, s_layer0OffsetX, s_layer0OffsetY);
        LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 0);

        LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 1, s_layer1OffsetX, s_layer1OffsetY);
        LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 1);

        LCDIFV2_SetLayerOffset(DEMO_LCDIFV2, 2, s_layer2OffsetX, s_layer2OffsetY);
        LCDIFV2_TriggerLayerShadowLoad(DEMO_LCDIFV2, 2);
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

    PRINTF("LCDIF v2 embedded_alpha example start...\r\n");

    /* With this function, the screen shows static color blocks. */
    DEMO_LCDIFV2_AlphaInit();

    /* With this function, the color blocks moves and shows different
     * blend results.
     */
    DEMO_LCDIFV2_AlphaRun();

    while (1)
    {
    }
}
