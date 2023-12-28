/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_pxp.h"
#include "display_support.h"

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_PXP PXP

/* Use RGB565 or XRGB8888 */
#define USE_RGB565 0

#define APP_IMG_WIDTH  DEMO_BUFFER_WIDTH
#define APP_IMG_HEIGHT DEMO_BUFFER_HEIGHT

#if USE_RGB565
typedef uint16_t pixel_t;
#define APP_BPP            2U /* Use 16-bit RGB565 format. */
#define APP_PXP_OUT_FORMAT kPXP_OutputPixelFormatRGB565
#define APP_DC_FORMAT      kVIDEO_PixelFormatRGB565
#else
typedef uint32_t pixel_t;
#define APP_BPP            4U /* Use 32-bit XRGB888 format. */
#define APP_PXP_OUT_FORMAT kPXP_OutputPixelFormatRGB888
#define APP_DC_FORMAT      kVIDEO_PixelFormatXRGB8888
#endif

#define APP_RED   0x00FF0000U
#define APP_GREEN 0x0000FF00U
#define APP_BLUE  0x000000FFU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_InitLcdif(void);
static void APP_FillPanel(void);
static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(static pixel_t s_BufferLcd[2][APP_IMG_HEIGHT][APP_IMG_WIDTH], FRAME_BUFFER_ALIGN);

/*
 * When new frame buffer sent to display, it might not be shown immediately.
 * Application could use callback to get new frame shown notification, at the
 * same time, when this flag is set, application could write to the older
 * frame buffer.
 */
static volatile bool s_newFrameShown = false;
static uint8_t curLcdBufferIdx       = 0;
static dc_fb_info_t fbInfo;

/*******************************************************************************
 * Code
 ******************************************************************************/

int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\nPXP fill rectangel example start...\r\n");

    APP_InitLcdif();

    APP_FillPanel();

    while (1)
    {
    }
}

static void APP_InitLcdif(void)
{
    status_t status;

    BOARD_PrepareDisplayController();

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(0);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    fbInfo.pixelFormat = APP_DC_FORMAT;
    fbInfo.width       = APP_IMG_WIDTH;
    fbInfo.height      = APP_IMG_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = APP_IMG_WIDTH * APP_BPP;
    g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo);

    g_dc.ops->setCallback(&g_dc, 0, APP_BufferSwitchOffCallback, NULL);

    s_newFrameShown = false;
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_BufferLcd[curLcdBufferIdx]);

    /* For the DBI interface display, application must wait for the first
     * frame buffer sent to the panel.
     */
    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        while (s_newFrameShown == false)
        {
        }
    }

    s_newFrameShown = true;

    g_dc.ops->enableLayer(&g_dc, 0);
}

static void APP_FillPanel(void)
{
    uint8_t i;
    PXP_Init(APP_PXP);

    static const uint32_t color[] = {APP_RED, APP_GREEN, APP_BLUE};

    for (;;)
    {
        for (i = 0; i < ARRAY_SIZE(color); i++)
        {
            /*
             * Wait for the new set frame buffer active, so that the older frame
             * buffer is inactive, then PXP could output to the older frame buffer.
             */
            while (s_newFrameShown == false)
            {
            }

            /* Switch to the other LCD buffer. */
            curLcdBufferIdx ^= 1U;

            /* Fill the whole screen with the configured color */
            PXP_BuildRect(APP_PXP, APP_PXP_OUT_FORMAT, color[i], APP_IMG_WIDTH, APP_IMG_HEIGHT, APP_IMG_WIDTH * APP_BPP,
                          (uint32_t)s_BufferLcd[curLcdBufferIdx]);

            /* Now new frame is ready, pass it to LCDIF. */
            s_newFrameShown = false;
            g_dc.ops->setFrameBuffer(&g_dc, 0, (void *)s_BufferLcd[curLcdBufferIdx]);

            /* Show the active frame buffer for a while. */
            SDK_DelayAtLeastUs(1 * 1000 * 1000, SDK_DEVICE_MAXIMUM_CPU_CLOCK_FREQUENCY);
        }
    }
}

static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
}
