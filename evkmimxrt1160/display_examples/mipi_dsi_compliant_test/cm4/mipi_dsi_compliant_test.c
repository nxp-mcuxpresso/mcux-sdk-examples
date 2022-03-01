/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "display_support.h"

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#ifndef DEMO_BUFFER_FIXED_ADDRESS
#define DEMO_BUFFER_FIXED_ADDRESS 0
#endif

/* XRGB8888 pixel definition. */
typedef struct pixel
{
    uint8_t B;
    uint8_t G;
    uint8_t R;
    uint8_t X;
} pixel_t;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dc_fb_info_t fbInfo;

#if !DEMO_BUFFER_FIXED_ADDRESS
AT_NONCACHEABLE_SECTION_ALIGN(static pixel_t s_frameBuffer[DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH], FRAME_BUFFER_ALIGN);

#else

static pixel_t (*s_frameBuffer)[DEMO_BUFFER_WIDTH] = (void *)DEMO_BUFFER0_ADDR;

#endif

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


static void DEMO_InitFrameBuffer(void)
{
    uint32_t i;
    uint32_t j;

    /*
     * For compliant test, there should be 3 types of data on data lane:
     *
     * 1. ... 11110000 11110000 ...
     * 2. ... 11111000 11111000 ...
     * 3. ... 00000111 00000111 ...
     *
     * The whole frame buffer is divided into three parts, one for each
     * data pattern.
     */

    for (i = 0; i < DEMO_BUFFER_HEIGHT / 3; i++)
    {
        for (j = 0; j < DEMO_BUFFER_WIDTH; j++)
        {
            s_frameBuffer[i][j].X = 0x00U;
            s_frameBuffer[i][j].R = 0xF0U;
            s_frameBuffer[i][j].G = 0xF0U;
            s_frameBuffer[i][j].B = 0xF0U;
        }
    }

    for (; i < DEMO_BUFFER_HEIGHT * 2 / 3; i++)
    {
        for (j = 0; j < DEMO_BUFFER_WIDTH; j++)
        {
            s_frameBuffer[i][j].X = 0x00U;
            s_frameBuffer[i][j].R = 0xF8U;
            s_frameBuffer[i][j].G = 0xF8U;
            s_frameBuffer[i][j].B = 0xF8U;
        }
    }

    for (; i < DEMO_BUFFER_HEIGHT * 3 / 3; i++)
    {
        for (j = 0; j < DEMO_BUFFER_WIDTH; j++)
        {
            s_frameBuffer[i][j].X = 0x00U;
            s_frameBuffer[i][j].R = 0x07U;
            s_frameBuffer[i][j].G = 0x07U;
            s_frameBuffer[i][j].B = 0x07U;
        }
    }
}

int main(void)
{
    status_t status;

    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    BOARD_ResetDisplayMix();
    BOARD_InitLpuartPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitDebugConsole();

    BOARD_PrepareDisplayController();

    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        PRINTF("Only do compliant test with video mode\r\n");
        assert(false);
    }

    /*
     * Initialize the display controller.
     */
    status = g_dc.ops->init(&g_dc);

    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(false);
    }

    /*
     * Set the layer configurations, pixel format set to XRGB8888.
     */
    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);

    fbInfo.pixelFormat = kVIDEO_PixelFormatXRGB8888;
    fbInfo.width       = DEMO_BUFFER_WIDTH;
    fbInfo.height      = DEMO_BUFFER_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = DEMO_BUFFER_WIDTH * sizeof(pixel_t);

    if (kStatus_Success != g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo))
    {
        PRINTF("Error: Could not configure the display controller\r\n");
        assert(false);
    }

    /*
     * Fill the frame buffer, and send to display controller.
     */
    DEMO_InitFrameBuffer();

    g_dc.ops->setFrameBuffer(&g_dc, 0, s_frameBuffer);

    g_dc.ops->enableLayer(&g_dc, 0);

    while (1)
        ;
}
