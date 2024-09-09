/*
 * Copyright (c) 2019,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lcdif.h"
#include "lcdif_support.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_BYTE_PER_PIXEL 2U

#define DEMO_IMG_HEIGHT DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH  DEMO_PANEL_WIDTH
#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
#define DEMO_IMG_BYTES_PER_LINE LCDIF_ALIGN_ADDR((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL), LCDIF_FB_ALIGN)
#else
#define DEMO_IMG_BYTES_PER_LINE (DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL)
#endif
#define DEMO_MAKE_COLOR(red, green, blue) \
    ((((uint16_t)(red)&0xF8U) << 8U) | (((uint16_t)(green)&0xFCU) << 3U) | (((uint16_t)(blue)&0xF8U) >> 3U))

#define DEMO_COLOR_BLUE  DEMO_MAKE_COLOR(0, 0, 255)
#define DEMO_COLOR_RED   DEMO_MAKE_COLOR(255, 0, 0)
#define DEMO_COLOR_GREEN DEMO_MAKE_COLOR(0, 255, 0)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t s_frameBufferAddr = DEMO_FB0_ADDR;
AT_NONCACHEABLE_SECTION_ALIGN(static volatile uint32_t s_cursorBuffer[LCDIF_CURSOR_SIZE][LCDIF_CURSOR_SIZE],
                              DEMO_FB_ALIGN);
static volatile bool s_frameDone = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = LCDIF_GetAndClearInterruptPendingFlags(DEMO_LCDIF);

    if (0 != (intStatus & kLCDIF_Display0FrameDoneInterrupt))
    {
        s_frameDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_FillFrameBuffer(void)
{
    uint32_t i, j;

    uint8_t(*fb)[DEMO_IMG_BYTES_PER_LINE] = (void *)s_frameBufferAddr;

    for (i = 0; i < DEMO_IMG_HEIGHT / 2; i++)
    {
        for (j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            *((uint16_t *)&fb[i][j * 2]) = DEMO_COLOR_RED;
        }
    }

    for (; i < DEMO_IMG_HEIGHT; i++)
    {
        for (j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            *((uint16_t *)&fb[i][j * 2]) = DEMO_COLOR_BLUE;
        }
    }
}

void DEMO_LCDIF_Init(void)
{
    lcdif_dpi_config_t dpiConfig = {
        .panelWidth    = DEMO_IMG_WIDTH,
        .panelHeight   = DEMO_IMG_HEIGHT,
        .hsw           = DEMO_HSW,
        .hfp           = DEMO_HFP,
        .hbp           = DEMO_HBP,
        .vsw           = DEMO_VSW,
        .vfp           = DEMO_VFP,
        .vbp           = DEMO_VBP,
        .polarityFlags = DEMO_POL_FLAGS,
        .format        = kLCDIF_Output24Bit,
    };

    /* Clear the frame buffer. */
    DEMO_FillFrameBuffer();

    LCDIF_Init(DEMO_LCDIF);

    LCDIF_DpiModeSetConfig(DEMO_LCDIF, 0, &dpiConfig);

    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_BYTES_PER_LINE);

    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr);

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
    lcdif_panel_config_t config;
    LCDIF_PanelGetDefaultConfig(&config);
    LCDIF_SetPanelConfig(DEMO_LCDIF, 0, &config);
#endif

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    NVIC_EnableIRQ(DEMO_LCDIF_IRQn);

    LCDIF_EnableInterrupts(DEMO_LCDIF, kLCDIF_Display0FrameDoneInterrupt);
}

void DEMO_FillCursorBuffer(uint8_t alpha)
{
    uint16_t i;
    uint16_t j;

    for (i = 0; i < LCDIF_CURSOR_SIZE; i++)
    {
        /* Green with different alpha. */
        for (j = 0; j < LCDIF_CURSOR_SIZE; j++)
        {
            s_cursorBuffer[i][j] = 0x00FF00 | (alpha << 24);
        }
    }
}

void DEMO_LCDIF_Enable(void)
{
    lcdif_fb_config_t fbConfig;

    /*
     * fbConfig.enable = true;
     * fbConfig.enableGamma = false;
     * fbConfig.format = kLCDIF_PixelFormatRGB565;
     */
    LCDIF_FrameBufferGetDefaultConfig(&fbConfig);

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
    fbConfig.enable          = true;
    fbConfig.inOrder         = kLCDIF_PixelInputOrderARGB;
    fbConfig.rotateFlipMode  = kLCDIF_Rotate0;
    fbConfig.format          = kLCDIF_PixelFormatRGB565;
    fbConfig.alpha.enable    = false;
    fbConfig.colorkey.enable = false;
    fbConfig.topLeftX        = 0U;
    fbConfig.topLeftY        = 0U;
    fbConfig.width           = DEMO_IMG_WIDTH;
    fbConfig.height          = DEMO_IMG_HEIGHT;
#else
    fbConfig.enable      = true;
    fbConfig.enableGamma = false;
    fbConfig.format      = kLCDIF_PixelFormatRGB565;
#endif

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);
}

void DEMO_LCDIF_Cursor(void)
{
    uint16_t x     = 0;
    uint16_t y     = 0;
    uint16_t alpha = 0;

    lcdif_cursor_config_t cursorConfig;

    DEMO_FillCursorBuffer(alpha);

    /*
     * cursorConfig.enable = true;
     * cursorConfig.format = kLCDIF_CursorMasked;
     * cursorConfig.hotspotOffsetX = 0;
     * cursorConfig.hotspotOffsetY = 0;
     */
    LCDIF_CursorGetDefaultConfig(&cursorConfig);

    cursorConfig.format = kLCDIF_CursorARGB8888,

    LCDIF_SetCursorConfig(DEMO_LCDIF, &cursorConfig);

    LCDIF_SetCursorHotspotPosition(DEMO_LCDIF, x, y);

    LCDIF_SetCursorBufferAddress(DEMO_LCDIF, (uint32_t)s_cursorBuffer);

    DEMO_LCDIF_Enable();

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
    LCDIF_Start(DEMO_LCDIF);
#endif

    while (1)
    {
        /* Wait for previous frame complete. */
        while (!s_frameDone)
        {
        }

        alpha++;
        x++;
        y++;

        x     = (x >= DEMO_IMG_WIDTH) ? 0 : x;
        y     = (y >= DEMO_IMG_HEIGHT) ? 0 : y;
        alpha = (alpha >= 256) ? 0 : alpha;
        DEMO_FillCursorBuffer(alpha);
        s_frameDone = false;
        LCDIF_SetCursorHotspotPosition(DEMO_LCDIF, x, y);

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
        LCDIF_SetUpdateReady(DEMO_LCDIF);
#endif
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitAHBSC();
    BOARD_InitBootPins();
    BOARD_InitPsRamPins_Xspi2();

#if ((USE_DBI) && (USE_DBI_PANEL == PANEL_TFT_PROTO_5))
    CLOCK_EnableClock(kCLOCK_Gpio2);
    RESET_PeripheralReset(kGPIO2_RST_SHIFT_RSTn);
    
    BOARD_InitLcdDBIPanelPins();
#else
#if (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
    BOARD_InitI2cPins();
#endif
    BOARD_InitMipiPanelPinsEvk();

    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Gpio3);
    RESET_PeripheralReset(kGPIO1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kGPIO3_RST_SHIFT_RSTn);
#endif

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitLcdifClock();
    BOARD_Init16bitsPsRam(XSPI2);
    /* Change the PFD2 divide to 17. Then the DBI source frequency shall be 528 * 18 / 17 / 2 = 279.53MHz. */
    CLOCK_InitMainPfd(kCLOCK_Pfd2, 17);
    CLOCK_SetClkDiv(kCLOCK_DivMediaMainClk, 2U);
    CLOCK_AttachClk(kMAIN_PLL_PFD2_to_MEDIA_MAIN);

    PRINTF("LCDIF cursor ARGB mode example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_Cursor();

    while (1)
    {
    }
}
