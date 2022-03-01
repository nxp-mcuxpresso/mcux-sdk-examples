/*
 * Copyright 2019, 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "display_support.h"
#include "fsl_fbdev.h"

#include "fsl_gpio.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#if DEMO_BUFFER_COUNT > 3
#error This example does not support more than 3 frame buffers.
#endif

#ifndef DEMO_BUFFER_FIXED_ADDRESS
#define DEMO_BUFFER_FIXED_ADDRESS 0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void fbdev_task(void *pvParameters);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if !DEMO_BUFFER_FIXED_ADDRESS
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_frameBuffer[DEMO_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH][DEMO_BUFFER_BYTE_PER_PIXEL],
    FRAME_BUFFER_ALIGN);

#define DEMO_BUFFER0_ADDR (uint32_t) s_frameBuffer[0]

#if DEMO_BUFFER_COUNT > 1
#define DEMO_BUFFER1_ADDR (uint32_t) s_frameBuffer[1]
#endif

#if DEMO_BUFFER_COUNT > 2
#define DEMO_BUFFER2_ADDR (uint32_t) s_frameBuffer[2]
#endif

#endif

static const uint32_t s_frameBufferAddress[DEMO_BUFFER_COUNT] = {DEMO_BUFFER0_ADDR,
#if DEMO_BUFFER_COUNT > 1
                                                                 DEMO_BUFFER1_ADDR,
#endif
#if DEMO_BUFFER_COUNT > 2
                                                                 DEMO_BUFFER2_ADDR
#endif
};

fbdev_t g_fbdev;
fbdev_fb_info_t g_fbInfo;

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (DEMO_PANEL_RM67162 == DEMO_PANEL)
void GPIO_INTA_IRQHandler(void)
{
    uint32_t intStat;

    intStat = GPIO_PortGetInterruptStatus(GPIO, BOARD_MIPI_TE_PORT, 0);

    GPIO_PortClearInterruptFlags(GPIO, BOARD_MIPI_TE_PORT, 0, intStat);

    if (intStat & (1U << BOARD_MIPI_TE_PIN))
    {
        BOARD_DisplayTEPinHandler();
    }
}
#endif


int main(void)
{
    /* Init board hardware. */
    status_t status;

    BOARD_InitUARTPins();
    BOARD_InitPsRamPins();

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)
    BOARD_InitFlexIOPanelPins();

    GPIO_PortInit(GPIO, BOARD_SSD1963_RST_PORT);
    GPIO_PortInit(GPIO, BOARD_SSD1963_CS_PORT);
    GPIO_PortInit(GPIO, BOARD_SSD1963_RS_PORT);
#else
    BOARD_InitMipiPanelPins();

    GPIO_PortInit(GPIO, BOARD_MIPI_POWER_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_BL_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_RST_PORT);

#if (DEMO_PANEL_RM67162 == DEMO_PANEL)
    GPIO_PortInit(GPIO, BOARD_MIPI_TE_PORT);
#endif

#endif

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    status = BOARD_InitPsRam();
    if (status != kStatus_Success)
    {
        assert(false);
    }

    if (xTaskCreate(fbdev_task, "fbdev_task", configMINIMAL_STACK_SIZE + 200, NULL, configMAX_PRIORITIES - 1, NULL) !=
        pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();
    for (;;)
        ;
}

static uint32_t DEMO_GetTimeMs()
{
    return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

static void DEMO_ShowPixelFormat(video_pixel_format_t format)
{
    char *fmtString;

    switch (format)
    {
        case kVIDEO_PixelFormatRGB565:
            fmtString = "RGB565";
            break;

        case kVIDEO_PixelFormatBGR565:
            fmtString = "BGR565";
            break;

        case kVIDEO_PixelFormatXRGB4444:
            fmtString = "XRGB4444";
            break;

        case kVIDEO_PixelFormatXRGB1555:
            fmtString = "XRGB1555";
            break;

        case kVIDEO_PixelFormatBGR888:
            fmtString = "BGR888";
            break;

        case kVIDEO_PixelFormatRGB888:
            fmtString = "RGB888";
            break;

        case kVIDEO_PixelFormatXBGR8888:
            fmtString = "XBGR8888";
            break;

        case kVIDEO_PixelFormatXRGB8888:
            fmtString = "XRGB8888";
            break;

        default:
            fmtString = "UNSUPPORTED";
            break;
    }

    PRINTF("Pixel format: %s\r\n", fmtString);
}

static void DEMO_FillFrameBuffer(void *buffer, uint16_t height, uint16_t width, uint16_t strideBytes)
{
    /* Foreground color. */
    static uint8_t fgColorIndex = 0U;

#if (2 == DEMO_BUFFER_BYTE_PER_PIXEL)
    static uint16_t fgColorTable[3];
    uint16_t fgColor;

    if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatBGR565)
    {
        fgColorTable[0] = 0xF800U; /* Blue. */
        fgColorTable[1] = 0x07E0U; /* Green. */
        fgColorTable[2] = 0x001FU; /* Red. */
    }
    else if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatXRGB4444)
    {
        fgColorTable[0] = 0x000FU; /* Blue. */
        fgColorTable[1] = 0x00F0U; /* Green. */
        fgColorTable[2] = 0x0F00U; /* Red. */
    }
    else if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatXRGB1555)
    {
        fgColorTable[0] = 0x001FU; /* Blue. */
        fgColorTable[1] = 0x03E0U; /* Green. */
        fgColorTable[2] = 0x7C00U; /* Red. */
    }
    else if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatRGB565)
    {
        fgColorTable[0] = 0x001FU; /* Blue. */
        fgColorTable[1] = 0x07E0U; /* Green. */
        fgColorTable[2] = 0xF800U; /* Red. */
    }
    else
    {
        PRINTF("Pixel format not supported\r\n");
    }
#elif (3 == DEMO_BUFFER_BYTE_PER_PIXEL)
    static uint32_t fgColorTable[3];
    uint32_t fgColor;

    if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatBGR888)
    {
        fgColorTable[0] = 0xFF0000U; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0x0000FFU; /* Red. */
    }
    else if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatRGB888)
    {
        fgColorTable[0] = 0x0000FFU; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0xFF0000U; /* Red. */
    }
    else
    {
        PRINTF("Pixel format not supported\r\n");
    }
#elif (4 == DEMO_BUFFER_BYTE_PER_PIXEL)
    static uint32_t fgColorTable[3];
    uint32_t fgColor;

    if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatXBGR8888)
    {
        fgColorTable[0] = 0xFF0000U; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0x0000FFU; /* Red. */
    }
    else if (DEMO_BUFFER_PIXEL_FORMAT == kVIDEO_PixelFormatXRGB8888)
    {
        fgColorTable[0] = 0x0000FFU; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0xFF0000U; /* Red. */
    }
    else
    {
        PRINTF("Pixel format not supported\r\n");
    }

#endif

    fgColor = fgColorTable[fgColorIndex];

    /* Position of the foreground rectangle. */
    static uint16_t upperLeftX = 0U;
    static uint16_t upperLeftY = 0U;
    static int8_t incX         = 1;
    static int8_t incY         = 1;

    /* Change color in next forame or not. */
    static bool changeColor = false;

    uint16_t lowerRightX = upperLeftX + (width - 1U) / 2U;
    uint16_t lowerRightY = upperLeftY + (height - 1U) / 2U;
    uint32_t i, j;

    /* Fill background with black. */
    memset(buffer, 0, strideBytes * height);

    /* Set forground rectangle. */
    for (i = upperLeftY; i <= lowerRightY; i++)
    {
        for (j = upperLeftX; j <= lowerRightX; j++)
        {
#if (2 == DEMO_BUFFER_BYTE_PER_PIXEL)
            ((uint16_t *)(((uint8_t *)buffer) + strideBytes * i))[j] = fgColor;
#elif (3 == DEMO_BUFFER_BYTE_PER_PIXEL)
            *(((uint8_t *)buffer) + strideBytes * i + 3 * j + 0) = fgColor & 0xFF;
            *(((uint8_t *)buffer) + strideBytes * i + 3 * j + 1) = (fgColor >> 8) & 0xFF;
            *(((uint8_t *)buffer) + strideBytes * i + 3 * j + 2) = (fgColor >> 16) & 0xFF;
#elif (4 == DEMO_BUFFER_BYTE_PER_PIXEL)
            ((uint32_t *)(((uint8_t *)buffer) + strideBytes * i))[j] = fgColor;
#endif
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
    else if (width - 1 == lowerRightX)
    {
        incX        = -1;
        changeColor = true;
    }

    if (0U == upperLeftY)
    {
        incY        = 1;
        changeColor = true;
    }
    else if (height - 1 == lowerRightY)
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

static void fbdev_task(void *pvParameters)
{
    void *buffer;
    status_t status;

    PRINTF("FBDEV_FreeRTOS example started\r\n");

    DEMO_ShowPixelFormat(DEMO_BUFFER_PIXEL_FORMAT);

    BOARD_PrepareDisplayController();
    FBDEV_Open(&g_fbdev, &g_dc, 0);

    FBDEV_GetFrameBufferInfo(&g_fbdev, &g_fbInfo);

    g_fbInfo.bufInfo.pixelFormat = DEMO_BUFFER_PIXEL_FORMAT;
    g_fbInfo.bufInfo.startX      = DEMO_BUFFER_START_X;
    g_fbInfo.bufInfo.startY      = DEMO_BUFFER_START_Y;
    g_fbInfo.bufInfo.width       = DEMO_BUFFER_WIDTH;
    g_fbInfo.bufInfo.height      = DEMO_BUFFER_HEIGHT;
    g_fbInfo.bufInfo.strideBytes = DEMO_BUFFER_STRIDE_BYTE;

    g_fbInfo.bufferCount = DEMO_BUFFER_COUNT;
    for (uint8_t i = 0; i < DEMO_BUFFER_COUNT; i++)
    {
        g_fbInfo.buffers[i] = (void *)s_frameBufferAddress[i];
    }

    status = FBDEV_SetFrameBufferInfo(&g_fbdev, &g_fbInfo);

    if (status != kStatus_Success)
    {
        PRINTF("Set frame buffer info error\r\n");
        while (1)
            ;
    }

    /* Set the first frame, it is shown when enabled. */
    buffer = FBDEV_GetFrameBuffer(&g_fbdev, 0);

    assert(buffer != NULL);

    memset(buffer, 0, DEMO_BUFFER_STRIDE_BYTE * DEMO_BUFFER_HEIGHT);

    FBDEV_SetFrameBuffer(&g_fbdev, buffer, 0);

    FBDEV_Enable(&g_fbdev);

    uint32_t startTime, time, n = 0;
    startTime = DEMO_GetTimeMs();

    while (1)
    {
        buffer = FBDEV_GetFrameBuffer(&g_fbdev, 0);

        DEMO_FillFrameBuffer(buffer, g_fbInfo.bufInfo.height, g_fbInfo.bufInfo.width, g_fbInfo.bufInfo.strideBytes);

        FBDEV_SetFrameBuffer(&g_fbdev, buffer, 0);

        /*
         * Show the frame update rate.
         *
         * NOTE: The project spends time with filling frame buffer, so the calculated frame update rate
         * might be smaller the actual LCD frame refresh rate, especially when frame buffer is large.
         */
        n++;
        if (n > 60)
        {
            time = DEMO_GetTimeMs() - startTime;
            PRINTF("Frame Update Rate (include buffer fill time): %d fps\r\n", n * 1000 / time);
            n         = 0;
            startTime = DEMO_GetTimeMs();
        }
    }
}
