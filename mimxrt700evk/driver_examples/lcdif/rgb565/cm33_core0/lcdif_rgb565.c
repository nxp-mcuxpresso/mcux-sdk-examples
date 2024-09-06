/*
 * Copyright (c) 2019, NXP
 * All rights reserved.
 *
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
#define DEMO_IMG_HEIGHT     DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH      DEMO_PANEL_WIDTH
#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
#define DEMO_IMG_BYTES_PER_LINE LCDIF_ALIGN_ADDR((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL), LCDIF_FB_ALIGN)
#else
#define DEMO_IMG_BYTES_PER_LINE (DEMO_PANEL_WIDTH * DEMO_BYTE_PER_PIXEL)
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_frameBufferAddr[2] = {DEMO_FB0_ADDR, DEMO_FB1_ADDR};
static volatile bool s_frameDone     = false;

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

void DEMO_FillFrameBuffer(uint32_t frameBufferAddr)
{
    uint8_t(*frameBuffer)[DEMO_IMG_BYTES_PER_LINE] = (void *)frameBufferAddr;

    /* Foreground color. */
    static uint8_t fgColorIndex          = 0U;
    static const uint16_t fgColorTable[] = {0x001FU, 0x07E0U, 0x07FFU, 0xF800U, 0xF81FU, 0xFFE0U, 0xFFFFU};
    uint16_t fgColor                     = fgColorTable[fgColorIndex];

    /* Position of the foreground rectangle. */
    static uint16_t upperLeftX  = 0;
    static uint16_t upperLeftY  = 0;
    static uint16_t lowerRightX = (DEMO_IMG_WIDTH - 1U) / 2U;
    static uint16_t lowerRightY = (DEMO_IMG_HEIGHT - 1U) / 2U;

    static int8_t incX = 1;
    static int8_t incY = 1;

    /* Change color in next frame or not. */
    static bool changeColor = false;

    uint32_t i, j;

    /* Set background color to black. */
    memset(frameBuffer, 0, DEMO_IMG_WIDTH * DEMO_IMG_HEIGHT * 2);

    /* Foreground color. */
    for (i = upperLeftY; i < lowerRightY; i++)
    {
        for (j = upperLeftX; j < lowerRightX; j++)
        {
            *((uint16_t *)&frameBuffer[i][j * 2]) = fgColor;
        }
    }

    /* Update the format: color and rectangle position. */
    upperLeftX += incX;
    upperLeftY += incY;
    lowerRightX += incX;
    lowerRightY += incY;

    changeColor = false;

    if (0U == upperLeftX)
    {
        incX        = 1;
        changeColor = true;
    }
    else if (DEMO_IMG_WIDTH - 1 == lowerRightX)
    {
        incX        = -1;
        changeColor = true;
    }

    if (0U == upperLeftY)
    {
        incY        = 1;
        changeColor = true;
    }
    else if (DEMO_IMG_HEIGHT - 1 == lowerRightY)
    {
        incY        = -1;
        changeColor = true;
    }

    if (changeColor)
    {
        fgColorIndex++;

        if (ARRAY_SIZE(fgColorTable) == fgColorIndex)
        {
            fgColorIndex = 0U;
        }
    }
}

void DEMO_LCDIF_Init(void)
{
    const lcdif_dpi_config_t dpiConfig = {
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

    LCDIF_Init(DEMO_LCDIF);

    LCDIF_DpiModeSetConfig(DEMO_LCDIF, 0, &dpiConfig);

    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_BYTES_PER_LINE);

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

void DEMO_LCDIF_RGB(void)
{
    uint32_t frameBufferIndex = 0;

    lcdif_fb_config_t fbConfig;

    LCDIF_FrameBufferGetDefaultConfig(&fbConfig);

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
    lcdif_panel_config_t config;
    LCDIF_PanelGetDefaultConfig(&config);
    LCDIF_SetPanelConfig(DEMO_LCDIF, 0, &config);

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

    DEMO_FillFrameBuffer(s_frameBufferAddr[frameBufferIndex]);

    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr[frameBufferIndex]);

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
    LCDIF_Start(DEMO_LCDIF);
#endif

    while (1)
    {
        frameBufferIndex ^= 1U;
        /*
         * Wait for previous frame complete.
         * Interrupt happens when the last pixel sent out. New frame buffer configuration
         * load at the next VSYNC.
         */
        while (!s_frameDone)
        {
        }

#if defined(FSL_FEATURE_LCDIF_VERSION_DC8000) & FSL_FEATURE_LCDIF_VERSION_DC8000
        LCDIF_SetUpdateReady(DEMO_LCDIF);
#endif
        s_frameDone = false;
        DEMO_FillFrameBuffer(s_frameBufferAddr[frameBufferIndex]);
        LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)s_frameBufferAddr[frameBufferIndex]);
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

    PRINTF("LCDIF RGB565 example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_RGB();

    while (1)
    {
    }
}
