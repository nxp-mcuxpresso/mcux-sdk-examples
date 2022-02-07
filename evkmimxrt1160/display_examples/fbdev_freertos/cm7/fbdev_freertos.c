/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "display_support.h"
#include "fsl_fbdev.h"

#include "fsl_soc_src.h"
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


int main(void)
{
    /* Init board hardware. */
    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    BOARD_ResetDisplayMix();
    BOARD_ConfigMPU();
    BOARD_InitLpuartPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitDebugConsole();

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
    else
    {
        fgColorTable[0] = 0x001FU; /* Blue. */
        fgColorTable[1] = 0x07E0U; /* Green. */
        fgColorTable[2] = 0xF800U; /* Red. */
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
    else
    {
        fgColorTable[0] = 0x0000FFU; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0xFF0000U; /* Red. */
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
    else
    {
        fgColorTable[0] = 0x0000FFU; /* Blue. */
        fgColorTable[1] = 0x00FF00U; /* Green. */
        fgColorTable[2] = 0xFF0000U; /* Red. */
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
    uint32_t count = 0;

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

    while (1)
    {
        PRINTF("Set frame buffer %d times.\r\n", ++count);

        buffer = FBDEV_GetFrameBuffer(&g_fbdev, 0);

        DEMO_FillFrameBuffer(buffer, g_fbInfo.bufInfo.height, g_fbInfo.bufInfo.width, g_fbInfo.bufInfo.strideBytes);

        FBDEV_SetFrameBuffer(&g_fbdev, buffer, 0);
    }
}
