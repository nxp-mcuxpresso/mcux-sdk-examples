/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "fsl_common.h"
#include "fsl_debug_console.h"
#include "fsl_cache.h"
#include "fsl_pngdec.h"
#include "png.h"
#include "display_support.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_PNGDEC PNGDEC
#define APP_FB_HEIGHT  DEMO_BUFFER_HEIGHT
#define APP_FB_WIDTH   DEMO_BUFFER_WIDTH
#define APP_FB_START_X DEMO_BUFFER_START_X
#define APP_FB_START_Y DEMO_BUFFER_START_Y

/* The decoded pixel format is ABGR8888 for all kinds of PNG soure. */
#define APP_FB_BPP    4
#define APP_FB_FORMAT kVIDEO_PixelFormatXBGR8888

/* Cache line size. */
#ifndef FSL_FEATURE_L2CACHE_LINESIZE_BYTE
#define FSL_FEATURE_L2CACHE_LINESIZE_BYTE 0
#endif
#ifndef FSL_FEATURE_L1DCACHE_LINESIZE_BYTE
#define FSL_FEATURE_L1DCACHE_LINESIZE_BYTE 0
#endif

#if (FSL_FEATURE_L2CACHE_LINESIZE_BYTE > FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#define APP_CACHE_LINE_SIZE FSL_FEATURE_L2CACHE_LINESIZE_BYTE
#else
#define APP_CACHE_LINE_SIZE FSL_FEATURE_L1DCACHE_LINESIZE_BYTE
#endif

/*
 * For better performance, the frame buffers are located in cachable region and
 * the cache line maintanance is handled in this demo. The start address of
 * frame buffer, and the size of frame buffer, are aligned to the cache line
 * size.
 */
#ifndef APP_FB_ALIGN
#if (APP_CACHE_LINE_SIZE > 0)
#define APP_FB_ALIGN APP_CACHE_LINE_SIZE
#else
#define APP_FB_ALIGN 1
#endif
#endif /* APP_FB_ALIGN */

#ifndef APP_FB_STRIDE_BYTE
#define APP_FB_STRIDE_BYTE (APP_FB_WIDTH * APP_FB_BPP)
#endif

#define APP_FB_SIZE_BYTE (SDK_SIZEALIGN(APP_FB_STRIDE_BYTE * APP_FB_HEIGHT, APP_FB_ALIGN))

#ifndef APP_FB_USE_FIXED_ADDRESS
#define APP_FB_USE_FIXED_ADDRESS 1
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static dc_fb_info_t fbInfo;

static volatile bool s_newFrameShown = false;

#if APP_FB_USE_FIXED_ADDRESS
static void *g_frameBuffer = (void *)DEMO_BUFFER0_ADDR;
#else
SDK_ALIGN(static uint8_t g_frameBuffer[APP_FB_SIZE_BYTE], APP_FB_ALIGN);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
#if ((DEMO_PANEL_RM67162 == DEMO_PANEL) && (!RM67162_USE_LCDIF))
void BOARD_MIPI_TE_GPIO_IRQ_Handler(void)
{
    uint32_t intStat;

    intStat = GPIO_PinGetInterruptFlag(BOARD_MIPI_TE_GPIO, BOARD_MIPI_TE_PIN);

    GPIO_GpioClearInterruptFlags(BOARD_MIPI_TE_GPIO, 1U << BOARD_MIPI_TE_PIN);

    if (intStat & (1U << BOARD_MIPI_TE_PIN))
    {
        BOARD_DisplayTEPinHandler();
    }
}
#endif

/*!
 * @brief  PNG Decode
 * @param  buff: pointer to the decoded image buffer
 */
void png_decode(void)
{
    pngdec_image_t image;
    pngdec_config_t config;
    status_t result;

    /* Step 1: Init PNG decoder module. */
    PNGDEC_GetDefaultConfig(&config);
    PNGDEC_Init(APP_PNGDEC, &config);

    /* Step 2: Set source buffer, buffer size and pixel draw callback function for PNG decoder handler. */
    PNGDEC_SetPngBuffer(APP_PNGDEC, (uint8_t *)pngImg, pngImgLen);

    /* Step 3: Set buffer of generated image for PNG decoder handler, this is required when the pixel draw callback
     * function is null. */
    PNGDEC_SetOutputBuffer(APP_PNGDEC, g_frameBuffer, NULL);

    /* Step 4: Parse the PNG header to get the width, height, bit depth, etc. */
    result = PNGDEC_ParseHeader(&image, (uint8_t *)pngImg);

    if (result != kStatus_Success)
    {
        PRINTF("Error: PNG decode failed\r\n");
        assert(false);
    }

    /* Step 5: Start PNG decode. */
    result = PNGDEC_Decode(APP_PNGDEC, &image, NULL);

    if (result != kStatus_Success)
    {
        PRINTF("Error: PNG decode failed\r\n");
        assert(false);
    }

    /* Step 5: Configure dispaly layer configuration. */
    fbInfo.pixelFormat = APP_FB_FORMAT;
    fbInfo.width       = image.width;
    fbInfo.height      = image.height;
    fbInfo.startX      = (APP_FB_WIDTH - image.width) / 2U;
    fbInfo.startY      = (APP_FB_HEIGHT - image.height) / 2U;
    fbInfo.strideBytes = image.width * APP_FB_BPP;
    if (kStatus_Success != g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo))
    {
        PRINTF("Error: Could not configure the display controller\r\n");
        assert(false);
    }

    /* Step 6: Set the frame buffer and enable the layer. */
    g_dc.ops->setFrameBuffer(&g_dc, 0, g_frameBuffer);

    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        while (s_newFrameShown == false)
        {
        }
    }

    s_newFrameShown = true;

    g_dc.ops->enableLayer(&g_dc, 0);
}

void APP_InitDisplay(void)
{
    status_t status;

    status = BOARD_PrepareDisplayController();
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(false);
    }

    memset(g_frameBuffer, 0, APP_FB_SIZE_BYTE);

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(false);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    g_dc.ops->setCallback(&g_dc, 0, APP_BufferSwitchOffCallback, NULL);
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

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)
#if (SSD1963_DRIVEN_BY == SSD1963_DRIVEN_BY_FLEXIO)
    BOARD_InitFlexIOPanelPins();
#else /* SSD1963_DRIVEN_BY_LCDIF */
    BOARD_InitLcdDBIPanelPins();
#endif

    CLOCK_EnableClock(kCLOCK_Gpio2);
    RESET_PeripheralReset(kGPIO2_RST_SHIFT_RSTn);
#else
#if (DEMO_PANEL_RASPI_7INCH == DEMO_PANEL)
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
    BOARD_Init16bitsPsRam(XSPI2);

    POWER_DisablePD(kPDRUNCFG_APD_PNGDEC);
    POWER_DisablePD(kPDRUNCFG_PPD_PNGDEC);
    POWER_ApplyPD();

    PRINTF("PNG decoder demo start:\r\n");

    APP_InitDisplay();

    PRINTF("Decoding the image...\r\n");
    png_decode();
    PRINTF("done!\r\n");

    while (1)
    {
    }
}

static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
}
