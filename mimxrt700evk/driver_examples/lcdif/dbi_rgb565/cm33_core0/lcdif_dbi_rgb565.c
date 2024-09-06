/*
 * Copyright (c) 2023,2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lcdif.h"
#include "lcdif_support.h"
#if (USE_DBI_PANEL != PANEL_TFT_PROTO_5)
#include "fsl_mipi_dsi.h"
#endif
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_BYTE_PER_PIXEL     2U /* RGB565 */
#define DEMO_IMG_HEIGHT         (DEMO_BUFFER_END_Y - DEMO_BUFFER_START_Y + 1U)
#define DEMO_IMG_WIDTH          (DEMO_BUFFER_END_X - DEMO_BUFFER_START_X + 1U)
#define DEMO_IMG_BYTES_PER_LINE LCDIF_ALIGN_ADDR((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL), LCDIF_FB_ALIGN)

#if (USE_DBI_PANEL == PANEL_TFT_PROTO_5)
#define DEMO_PIXEL_HEIGHT_EACH_TIME DEMO_PANEL_HEIGHT /* For DBI interface panel, no need to seperate the data. */
#else
#define DEMO_PIXEL_HEIGHT_EACH_TIME 64U /* For MIPI interface panel, send 64 lines of the area each time. */
#endif

#define DBI_CMD_WRITE_MEMORY_START    0x2CU
#define DBI_CMD_WRITE_MEMORY_CONTINUE 0x3CU
#define DBI_CMD_SET_COLUMN_ADDRESS    0x2AU
#define DBI_CMD_SET_PAGE_ADDRESS      0x2BU

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static uint32_t s_frameBufferAddr = DEMO_FB0_ADDR;
static volatile bool s_frameDone  = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = LCDIF_GetAndClearInterruptPendingFlags(DEMO_LCDIF);

    if (0UL != (intStatus & kLCDIF_Display0FrameDoneInterrupt))
    {
        s_frameDone = true;
    }

    /* Check the LCDIF underflow status. */
    if (0UL != (intStatus & kLCDIF_PanelUnderflowInterrupt))
    {
        PRINTF("Panel underflow\r\n");
    }

    SDK_ISR_EXIT_BARRIER;
}

void DEMO_FillFrameBuffer(uint32_t frameBufferAddr)
{
    uint16_t(*frameBuffer)[DEMO_IMG_WIDTH] = (void *)frameBufferAddr;

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

    /* Change color in next forame or not. */
    static bool changeColor = false;

    uint32_t i, j;

    /* Set background color to black. */
    memset(frameBuffer, 0U, DEMO_IMG_BYTES_PER_LINE * DEMO_IMG_HEIGHT);

    /* Foreground color. */
    for (i = upperLeftY; i < lowerRightY; i++)
    {
        for (j = upperLeftX; j < lowerRightX; j++)
        {
            ((uint16_t *)(((uint8_t *)frameBuffer) + DEMO_IMG_BYTES_PER_LINE * i))[j] = fgColor;
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
    lcdif_dbi_config_t dbiConfig;

    LCDIF_Init(DEMO_LCDIF);
    /*
     * config->swizzle    = kLCDIF_DbiOutSwizzleRGB;
     * config->format     = kLCDIF_DbiOutD8RGB332;
     * config->acTimeUnit = 0;
     * config->type       = kLCDIF_DbiTypeA_ClockedE;
     * config->reversePolarity = false;
     * config->writeWRPeriod = 3U;
     * config->writeWRAssert = 0U;
     * config->writeCSAssert = 0U;
     * config->writeWRDeassert = 0U;
     * config->writeCSDeassert = 0U;
     * config->typeCTas     = 1U;
     * config->typeCSCLTwrl = 1U;
     * config->typeCSCLTwrh = 1U;
     */
    LCDIF_DbiModeGetDefaultConfig(&dbiConfig);
    dbiConfig.acTimeUnit = 0;
#if (defined(FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY) && FSL_FEATURE_LCDIF_HAS_DBIX_POLARITY)
    dbiConfig.reversePolarity = true;
#endif
    dbiConfig.writeWRPeriod = 14;
#if (USE_DBI_PANEL == PANEL_TFT_PROTO_5)
    dbiConfig.format = kLCDIF_DbiOutD8RGB888;
#else
    /* With 279.53MHz source and 16bpp format, a 14 cycle period requires a 279.53MHz / 14 * 16 = 319.46Mhz DPHY clk
     * source. */
    dbiConfig.format = kLCDIF_DbiOutD16RGB565;
#endif
    dbiConfig.type            = kLCDIF_DbiTypeB;
    dbiConfig.writeCSAssert   = 1;
    dbiConfig.writeCSDeassert = 4;
    dbiConfig.writeWRAssert   = (dbiConfig.writeWRPeriod - 1U) / 2U; /* Asset at the middle. */
    dbiConfig.writeWRDeassert = (dbiConfig.writeWRPeriod - 1U);      /* Deassert at the end */

    LCDIF_DbiModeSetConfig(DEMO_LCDIF, 0, &dbiConfig);

    lcdif_panel_config_t config;
    LCDIF_PanelGetDefaultConfig(&config);
    LCDIF_SetPanelConfig(DEMO_LCDIF, 0, &config);

    /* Enable clock and panel. */
    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    NVIC_ClearPendingIRQ(DEMO_LCDIF_IRQn);
    NVIC_EnableIRQ(DEMO_LCDIF_IRQn);

    LCDIF_EnableInterrupts(DEMO_LCDIF, kLCDIF_Display0FrameDoneInterrupt | kLCDIF_PanelUnderflowInterrupt);
}

void DEMO_SendFrameBuffer(
    uint16_t width, uint16_t height, uint16_t strideBytes, const uint8_t *data, uint8_t bytePerPixel)
{
    uint16_t leftHeight = height;
    uint16_t cmd        = DBI_CMD_WRITE_MEMORY_START;
    uint16_t line       = 0;

    LCDIF_SetFrameBufferStride(LCDIF, 0, strideBytes);

#if (USE_DBI_PANEL != PANEL_TFT_PROTO_5)
    /* Every time buffer 64 pixels first then begin the send. */
    DSI_SetDbiPixelFifoSendLevel(MIPI_DSI_HOST, 64);

    /* Set payload size. Send DEMO_PIXEL_HEIGHT_EACH_TIME lines of pixels each time. */
    DSI_SetDbiPixelPayloadSize(MIPI_DSI_HOST, DEMO_PIXEL_HEIGHT_EACH_TIME * DEMO_IMG_WIDTH);
#endif

    /* Only one layer is used, so for each memory write the select area's size shall be the same as the buffer. */
    LCDIF_SetFrameBufferPosition(DEMO_LCDIF, 0U, 0U, 0U, DEMO_IMG_WIDTH, DEMO_PIXEL_HEIGHT_EACH_TIME);

    while (leftHeight >= DEMO_PIXEL_HEIGHT_EACH_TIME)
    {
        s_frameDone = false;

        LCDIF_DbiSelectArea(DEMO_LCDIF, 0, 0, line, width - 1U, line + DEMO_PIXEL_HEIGHT_EACH_TIME - 1U, false);

        LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)data);

        /* Send command: Memory write start. */
        LCDIF_DbiSendCommand(DEMO_LCDIF, 0, cmd);

        /* Enable DMA and send out data. */
        LCDIF_DbiWriteMem(DEMO_LCDIF, 0);

        while (!s_frameDone)
        {
        }

        /* Update next chunk's info. */
        line += DEMO_PIXEL_HEIGHT_EACH_TIME;
        data += (DEMO_PIXEL_HEIGHT_EACH_TIME * strideBytes);
        leftHeight -= DEMO_PIXEL_HEIGHT_EACH_TIME;
        cmd = DBI_CMD_WRITE_MEMORY_CONTINUE;
    }

    /* Send the remaining lines. */
    if (leftHeight > 0u)
    {
        s_frameDone = false;
#if (USE_DBI_PANEL != PANEL_TFT_PROTO_5)
        /* Set payload size for the remaining pixels. */
        DSI_SetDbiPixelPayloadSize(MIPI_DSI_HOST, leftHeight * DEMO_IMG_WIDTH);
#endif

        /* Only one layer is used, so for each memory write the select area's size shall be the same as the buffer. */
        LCDIF_SetFrameBufferPosition(DEMO_LCDIF, 0U, 0U, 0U, DEMO_IMG_WIDTH, leftHeight);

        LCDIF_DbiSelectArea(DEMO_LCDIF, 0, 0, line, width - 1U, line + leftHeight - 1, false);

        LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, (uint32_t)data);

        /* Send command: Memory write continue. */
        LCDIF_DbiSendCommand(DEMO_LCDIF, 0, cmd);

        /* Enable DMA and send out data. */
        LCDIF_DbiWriteMem(DEMO_LCDIF, 0);

        while (!s_frameDone)
        {
        }
    }
}

void DEMO_LCDIF_RGB(void)
{
    lcdif_fb_config_t fbConfig;
    uint8_t cmdParam[4];

    LCDIF_FrameBufferGetDefaultConfig(&fbConfig);

    fbConfig.enable          = true;
    fbConfig.inOrder         = kLCDIF_PixelInputOrderARGB;
    fbConfig.rotateFlipMode  = kLCDIF_Rotate0;
    fbConfig.format          = kLCDIF_PixelFormatRGB565;
    fbConfig.alpha.enable    = false;
    fbConfig.colorkey.enable = false;
    fbConfig.topLeftX        = 0U;
    fbConfig.topLeftY        = 0U;
    fbConfig.width           = DEMO_IMG_WIDTH;
    /* Only one layer is used, so for each memory write the select area's size shall be the same as the buffer. */
    fbConfig.height = DEMO_PIXEL_HEIGHT_EACH_TIME;

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

    while (1)
    {
        /* Select the panel region used. */
        cmdParam[0] = (uint8_t)((DEMO_BUFFER_START_X >> 8U) & 0xFFU);
        cmdParam[1] = (uint8_t)(DEMO_BUFFER_START_X & 0xFFU);
        cmdParam[2] = (uint8_t)((DEMO_BUFFER_END_X >> 8U) & 0xFFU);
        cmdParam[3] = (uint8_t)(DEMO_BUFFER_END_X & 0xFFU);
        LCDIF_DbiSendCommandAndData(DEMO_LCDIF, 0, DBI_CMD_SET_COLUMN_ADDRESS, cmdParam, 4U);
        cmdParam[0] = (uint8_t)((DEMO_BUFFER_START_Y >> 8U) & 0xFFU);
        cmdParam[1] = (uint8_t)(DEMO_BUFFER_START_Y & 0xFFU);
        cmdParam[2] = (uint8_t)((DEMO_BUFFER_END_Y >> 8U) & 0xFFU);
        cmdParam[3] = (uint8_t)(DEMO_BUFFER_END_Y & 0xFFU);
        LCDIF_DbiSendCommandAndData(DEMO_LCDIF, 0, DBI_CMD_SET_PAGE_ADDRESS, cmdParam, 4U);

        DEMO_FillFrameBuffer(s_frameBufferAddr);

        DEMO_SendFrameBuffer(DEMO_IMG_WIDTH,          /* Width  */
                             DEMO_IMG_HEIGHT,         /* Height */
                             DEMO_IMG_BYTES_PER_LINE, /* Stride */
                             (const uint8_t *)s_frameBufferAddr, DEMO_BYTE_PER_PIXEL);
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

    PRINTF("LCDIF DBI mode RGB565 example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_RGB();

    while (1)
    {
    }
}
