/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "display_support.h"
#include "camera_support.h"
#include "fsl_pxp.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"

/*
 * In this example, the camera device output pixel format is RGB565, the MIPI_CSI
 * converts it to RGB888 internally and sends to CSI. In other words, the CSI input
 * data bus width is 24-bit. The CSI saves the frame as 32-bit format XRGB8888.
 *
 * The PXP is used in this example to rotate and convert the CSI output frame
 * to fit the display output.
 */

#include "fsl_soc_src.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PXP PXP

/* CSI output frame buffer is XYUV8888. */
#define DEMO_CAMERA_BUFFER_BPP 4

#define DEMO_CAMERA_BUFFER_COUNT 3

/* LCD input frame buffer is RGB565, converted by PXP. */
#define DEMO_LCD_BUFFER_BPP 2

#define DEMO_LCD_BUFFER_COUNT 2

#if (((DEMO_CAMERA_WIDTH < DEMO_CAMERA_HEIGHT) && (DEMO_BUFFER_WIDTH > DEMO_BUFFER_HEIGHT)) || \
     ((DEMO_CAMERA_WIDTH > DEMO_CAMERA_HEIGHT) && (DEMO_BUFFER_WIDTH < DEMO_BUFFER_HEIGHT)))
#define DEMO_ROTATE_FRAME 1
#else
#define DEMO_ROTATE_FRAME 0
#endif

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void DEMO_BufferSwitchOffCallback(void *param, void *switchOffBuffer);
static void DEMO_InitCamera(void);
static void DEMO_InitDisplay(void);
static void DEMO_InitPxp(void);
static void DEMO_CSI_MIPI_YUV(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
AT_NONCACHEABLE_SECTION_ALIGN(
    static uint8_t s_lcdBuffer[DEMO_LCD_BUFFER_COUNT][DEMO_BUFFER_HEIGHT][DEMO_BUFFER_WIDTH * DEMO_LCD_BUFFER_BPP],
    FRAME_BUFFER_ALIGN);

AT_NONCACHEABLE_SECTION_ALIGN(static uint8_t s_cameraBuffer[DEMO_CAMERA_BUFFER_COUNT][DEMO_CAMERA_HEIGHT]
                                                           [DEMO_CAMERA_WIDTH * DEMO_CAMERA_BUFFER_BPP],
                              DEMO_CAMERA_BUFFER_ALIGN);

/*
 * When new frame buffer sent to display, it might not be shown immediately.
 * Application could use callback to get new frame shown notification, at the
 * same time, when this flag is set, application could write to the older
 * frame buffer.
 */
static volatile bool s_newFrameShown = false;
static dc_fb_info_t fbInfo;
static volatile uint8_t s_lcdActiveFbIdx;

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



/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_BootClockRUN();
    BOARD_ResetDisplayMix();
    BOARD_EarlyInitCamera();
    BOARD_InitLpuartPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitMipiCameraPins();
    BOARD_InitDebugConsole();

    PRINTF("CSI MIPI YUV example start...\r\n");

    memset(s_cameraBuffer, 0, sizeof(s_cameraBuffer));
    memset(s_lcdBuffer, 0, sizeof(s_lcdBuffer));

    DEMO_InitPxp();

    DEMO_InitCamera();

    DEMO_InitDisplay();

    DEMO_CSI_MIPI_YUV();

    while (1)
    {
    }
}

static void DEMO_InitPxp(void)
{
    /*
     * Configure the PXP for rotate and scale.
     */
    PXP_Init(DEMO_PXP);

    PXP_SetProcessSurfaceBackGroundColor(DEMO_PXP, 0U);

#if DEMO_ROTATE_FRAME
    PXP_SetProcessSurfacePosition(DEMO_PXP, 0U, 0U, DEMO_BUFFER_HEIGHT - 1U, DEMO_BUFFER_WIDTH - 1U);
#else
    PXP_SetProcessSurfacePosition(DEMO_PXP, 0U, 0U, DEMO_BUFFER_WIDTH - 1U, DEMO_BUFFER_HEIGHT - 1U);
#endif

    /* Disable AS. */
    PXP_SetAlphaSurfacePosition(DEMO_PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);

    PXP_SetCsc1Mode(DEMO_PXP, kPXP_Csc1YCbCr2RGB);

    PXP_EnableCsc1(DEMO_PXP, true);
}

static void DEMO_InitCamera(void)
{
    camera_config_t cameraConfig;

    memset(&cameraConfig, 0, sizeof(cameraConfig));

    BOARD_InitCameraResource();

    /* CSI input data bus is 24-bit, and save as XYUV8888.. */
    cameraConfig.pixelFormat                = kVIDEO_PixelFormatXYUV;
    cameraConfig.bytesPerPixel              = DEMO_CAMERA_BUFFER_BPP;
    cameraConfig.resolution                 = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT);
    cameraConfig.frameBufferLinePitch_Bytes = DEMO_CAMERA_WIDTH * DEMO_CAMERA_BUFFER_BPP;
    cameraConfig.interface                  = kCAMERA_InterfaceGatedClock;
    cameraConfig.controlFlags               = DEMO_CAMERA_CONTROL_FLAGS;
    cameraConfig.framePerSec                = DEMO_CAMERA_FRAME_RATE;

    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, NULL, NULL);

    BOARD_InitMipiCsi();

    cameraConfig.pixelFormat   = kVIDEO_PixelFormatYUYV;
    cameraConfig.bytesPerPixel = 2;
    cameraConfig.resolution    = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT);
    cameraConfig.interface     = kCAMERA_InterfaceMIPI;
    cameraConfig.controlFlags  = DEMO_CAMERA_CONTROL_FLAGS;
    cameraConfig.framePerSec   = DEMO_CAMERA_FRAME_RATE;
    cameraConfig.csiLanes      = DEMO_CAMERA_MIPI_CSI_LANE;
    CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig);

    CAMERA_DEVICE_Start(&cameraDevice);

    /* Submit the empty frame buffers to buffer queue. */
    for (uint32_t i = 0; i < DEMO_CAMERA_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)(s_cameraBuffer[i]));
    }
}

static void DEMO_InitDisplay(void)
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
    fbInfo.pixelFormat = kVIDEO_PixelFormatRGB565;
    fbInfo.width       = DEMO_BUFFER_WIDTH;
    fbInfo.height      = DEMO_BUFFER_HEIGHT;
    fbInfo.startX      = DEMO_BUFFER_START_X;
    fbInfo.startY      = DEMO_BUFFER_START_Y;
    fbInfo.strideBytes = DEMO_BUFFER_WIDTH * DEMO_LCD_BUFFER_BPP;
    g_dc.ops->setLayerConfig(&g_dc, 0, &fbInfo);

    g_dc.ops->setCallback(&g_dc, 0, DEMO_BufferSwitchOffCallback, NULL);

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

static void DEMO_CSI_MIPI_YUV(void)
{
    uint32_t cameraReceivedFrameAddr;
    void *lcdFrameAddr;

    pxp_ps_buffer_config_t psBufferConfig = {
        .pixelFormat = kPXP_PsPixelFormatYUV1P444, /* Note: This is 32-bit per pixel */
        .swapByte    = false,
        .bufferAddrU = 0U,
        .bufferAddrV = 0U,
        .pitchBytes  = DEMO_CAMERA_WIDTH * DEMO_CAMERA_BUFFER_BPP,
    };

    /* Output config. */
    pxp_output_buffer_config_t outputBufferConfig = {
        .pixelFormat    = kPXP_OutputPixelFormatRGB565,
        .interlacedMode = kPXP_OutputProgressive,
        .buffer1Addr    = 0U,
        .pitchBytes     = DEMO_BUFFER_WIDTH * DEMO_LCD_BUFFER_BPP,
#if DEMO_ROTATE_FRAME
        .width  = DEMO_BUFFER_HEIGHT,
        .height = DEMO_BUFFER_WIDTH,
#else
        .width  = DEMO_BUFFER_WIDTH,
        .height = DEMO_BUFFER_HEIGHT,
#endif
    };

    PXP_SetProcessSurfaceBackGroundColor(DEMO_PXP, 0);
    /* Rotate and scale the camera input to fit display output. */
#if DEMO_ROTATE_FRAME
    /* The PS rotate and scale could not work at the same time, so rotate the output. */
    PXP_SetRotateConfig(DEMO_PXP, kPXP_RotateOutputBuffer, kPXP_Rotate90, kPXP_FlipDisable);
    PXP_SetProcessSurfaceScaler(DEMO_PXP, DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT, DEMO_BUFFER_HEIGHT, DEMO_BUFFER_WIDTH);
#else
    PXP_SetProcessSurfaceScaler(DEMO_PXP, DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT, DEMO_BUFFER_WIDTH, DEMO_BUFFER_HEIGHT);
#endif

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

        /* Convert the camera input picture to RGB format. */
        psBufferConfig.bufferAddr = cameraReceivedFrameAddr;
        PXP_SetProcessSurfaceBufferConfig(DEMO_PXP, &psBufferConfig);

        lcdFrameAddr                   = s_lcdBuffer[s_lcdActiveFbIdx ^ 1];
        outputBufferConfig.buffer0Addr = (uint32_t)lcdFrameAddr;
        PXP_SetOutputBufferConfig(DEMO_PXP, &outputBufferConfig);

        PXP_Start(DEMO_PXP);

        /* Wait for PXP process complete. */
        while (!(kPXP_CompleteFlag & PXP_GetStatusFlags(DEMO_PXP)))
        {
        }
        PXP_ClearStatusFlags(DEMO_PXP, kPXP_CompleteFlag);

        /* Return the camera buffer to camera receiver handle. */
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver, (uint32_t)cameraReceivedFrameAddr);

        /* Show the new frame. */
        s_newFrameShown = false;
        g_dc.ops->setFrameBuffer(&g_dc, 0, lcdFrameAddr);
    }
}

static void DEMO_BufferSwitchOffCallback(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
    s_lcdActiveFbIdx ^= 1;
}
