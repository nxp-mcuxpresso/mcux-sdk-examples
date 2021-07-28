/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "display_support.h"
#include "camera_support.h"

#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* RAW8 data, 1 byte per pixel. */
#define APP_CAMERA_BPP 1

#define APP_CAMERA_FRAME_BUFFER_COUNT 3

/* LCD frame buffer format is RGB565. */
#define APP_LCD_BPP 2

#define APP_LCD_FRAME_BUFFER_COUNT 2

#define MAKE_RGB565(r, g, b) (((r >> 3)) << 11) | (((g >> 2)) << 5) | (((b >> 3)) << 0)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer);
static void APP_InitCamera(void);
static void APP_InitDisplay(void);
static void APP_CSI_RAW8(void);
static void RAW8_2_RGB565(
    const uint8_t *raw, uint8_t *rgb, int width, int height, uint32_t inputPitch, uint32_t outputPitch);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_cameraBuffer[APP_CAMERA_FRAME_BUFFER_COUNT][DEMO_CAMERA_HEIGHT][DEMO_CAMERA_WIDTH],
    DEMO_CAMERA_BUFFER_ALIGN);
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint16_t s_lcdBuffer[APP_LCD_FRAME_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH], FRAME_BUFFER_ALIGN);

static volatile uint8_t s_lcdActiveFbIdx = 0;

/*
 * When new frame buffer sent to display, it might not be shown immediately.
 * Application could use callback to get new frame shown notification, at the
 * same time, when this flag is set, application could write to the older
 * frame buffer.
 */
static volatile bool s_newFrameShown = false;
static dc_fb_info_t fbInfo;

/*******************************************************************************
 * Code
 ******************************************************************************/


static void RAW8_2_RGB565(
    const uint8_t *raw, uint8_t *rgb, int width, int height, uint32_t inputPitch, uint32_t outputPitch)
{
    uint16_t *local_rgb;
    const uint8_t *local_raw;

    uint32_t i, j;
    uint8_t R, G, B;

    /* Set right column to black. */
    for (i = 0; i < height; i++)
    {
        local_rgb = (uint16_t *)((uint32_t)rgb + i * outputPitch);

        local_rgb[width - 1] = 0;
    }

    /* Set bottom line to black. */
    local_rgb = (uint16_t *)((uint32_t)rgb + (height - 1) * outputPitch);

    for (i = 0; i < width; i++)
    {
        local_rgb[i] = 0;
    }

    /* Do color conversion. */
    for (i = 0; i < height - 1; i++)
    {
        local_rgb = (uint16_t *)rgb;
        local_raw = raw;

        /* Line 0, 2, 4, ... */
        if ((i & 1) == 0)
        {
            /*
             * G1 R1
             * B1 G2
             */
            R = local_raw[1];                                        /* R(G1) = R1. */
            G = (local_raw[0] + local_raw[inputPitch + 1] + 1) >> 1; /* G(G1) = (G1 + G2) / 2 */
            B = local_raw[inputPitch];                               /* B(G1) = B1. */

            *local_rgb = MAKE_RGB565(R, G, B);
            local_raw++;
            local_rgb++;

            /*
             * G1 R1 G2 R2
             * B1 G3 B2 G4
             */
            for (j = 0; j < width - 2; j += 2)
            {
                R = local_raw[0];                                    /* R(R1) = R1. */
                G = (local_raw[1] + local_raw[inputPitch] + 1) >> 1; /* G(R1) = (G2 + G3) / 2 */
                B = local_raw[inputPitch + 1];                       /* B(R1) = B2 */

                *local_rgb = MAKE_RGB565(R, G, B);
                local_rgb++;

                R = local_raw[2];                                        /* R(G2) = R2 */
                G = (local_raw[1] + local_raw[inputPitch + 2] + 1) >> 1; /* G(G2) = (G2 + G4) / 2 */
                B = local_raw[inputPitch + 1];                           /* B(G2) = B2 */

                *local_rgb = MAKE_RGB565(R, G, B);
                local_rgb++;

                local_raw += 2;
            }
        }
        /* Line 1, 3, 5, ... */
        else
        {
            for (j = 0; j < width - 2; j += 2)
            {
                /*
                 * B1 G1 B2 G2
                 * G3 R1 G4 R2
                 */
                R = local_raw[inputPitch + 1];                       /* R(B1) = R1 */
                G = (local_raw[1] + local_raw[inputPitch] + 1) >> 1; /* G(B1) = (G1 + G3) / 2 */
                B = local_raw[0];                                    /* B(B1) = B1 */

                *local_rgb = MAKE_RGB565(R, G, B);
                local_rgb++;

                R = local_raw[inputPitch + 1];                           /* R(G1) = R1 */
                G = (local_raw[1] + local_raw[inputPitch + 2] + 1) >> 1; /* G(G1) = (G1 + G4) / 2 */
                B = local_raw[2];                                        /* B(G1) = B2 */

                *local_rgb = MAKE_RGB565(R, G, B);
                local_rgb++;

                local_raw += 2;
            }

            /*
             * B1 G2 B2 G2
             * G3 R1 G4 R2
             */
            R = local_raw[inputPitch + 1];                       /* R(B2) = R2. */
            G = (local_raw[1] + local_raw[inputPitch] + 1) >> 1; /* G(B2) = (G2 + G4) / 2 */
            B = local_raw[0];                                    /* B(B2) = B2. */

            *local_rgb = MAKE_RGB565(R, G, B);
        }

        raw += inputPitch;
        rgb += outputPitch;
    }
}

static void APP_InitCamera(void)
{
    const camera_config_t cameraConfig = {
        .pixelFormat                = kVIDEO_PixelFormatRAW8,
        .bytesPerPixel              = APP_CAMERA_BPP,
        .resolution                 = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT),
        .frameBufferLinePitch_Bytes = DEMO_CAMERA_WIDTH * APP_CAMERA_BPP,
        .interface                  = kCAMERA_InterfaceGatedClock,
        .controlFlags               = DEMO_CAMERA_CONTROL_FLAGS,
        .framePerSec                = 30,
    };

    memset(s_cameraBuffer, 0, sizeof(s_cameraBuffer));

    BOARD_InitCameraResource();

    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, NULL, NULL);

    if (kStatus_Success != CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig))
    {
        PRINTF("Camera device initialization failed\r\n");
        while (1)
        {
            ;
        }
    }

    CAMERA_DEVICE_Start(&cameraDevice);

    /* Submit the empty frame buffers to buffer queue. */
    for (uint32_t i = 0; i < APP_CAMERA_FRAME_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(s_cameraBuffer[i]));
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitDEBUG_UARTPins();
    BOARD_InitSDRAMPins();
    BOARD_EarlyPrepareCamera();
    BOARD_InitCSIPins();
    BOARD_InitLCDPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("CSI RAW8 example start...\r\n");

    APP_InitCamera();

    APP_InitDisplay();

    APP_CSI_RAW8();

    while (1)
    {
    }
}

static void APP_InitDisplay(void)
{
    status_t status;

    memset(s_lcdBuffer, 0, sizeof(s_lcdBuffer));

    BOARD_PrepareDisplayController();

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(0);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &fbInfo);
    fbInfo.pixelFormat = kVIDEO_PixelFormatRGB565;
    fbInfo.width       = DEMO_BUFFER_WIDTH;
    fbInfo.height      = DEMO_BUFFER_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = DEMO_BUFFER_WIDTH * APP_LCD_BPP;
    g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo);

    g_dc.ops->setCallback(&g_dc, 0, APP_BufferSwitchOffCallback, NULL);

    s_lcdActiveFbIdx = 0;
    s_newFrameShown  = false;
    g_dc.ops->setFrameBuffer(&g_dc, 0, s_lcdBuffer[s_lcdActiveFbIdx]);

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

static void APP_CSI_RAW8(void)
{
    uint32_t cameraReceivedFrameAddr;
    void *lcdFrameAddr;

    CAMERA_RECEIVER_Start(&cameraReceiver);

    while (1)
    {
        /* Wait to get the full frame buffer to show. */
        while (kStatus_Success != CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver, &cameraReceivedFrameAddr))
        {
        }

        /* Wait for the previous frame buffer is shown, and there is available
           inactive buffer to fill. */
        while (s_newFrameShown == false)
        {
        }

        lcdFrameAddr = s_lcdBuffer[s_lcdActiveFbIdx ^ 1];

        RAW8_2_RGB565((const uint8_t *)cameraReceivedFrameAddr, lcdFrameAddr, DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT,
                      DEMO_CAMERA_WIDTH * APP_CAMERA_BPP, DEMO_CAMERA_WIDTH * APP_LCD_BPP);

        /* Return the camera buffer to camera receiver handle. */
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)cameraReceivedFrameAddr);

        /* Show the new frame. */
        s_newFrameShown = false;
        g_dc.ops->setFrameBuffer(&g_dc, 0, lcdFrameAddr);
    }
}

static void APP_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
    s_lcdActiveFbIdx ^= 1;
}
