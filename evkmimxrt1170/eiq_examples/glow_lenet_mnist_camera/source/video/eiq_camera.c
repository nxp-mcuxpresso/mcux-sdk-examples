/*
 * Copyright 2018-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_camera.h"
#include "fsl_debug_console.h"
#include "board.h"
#include "pin_mux.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Camera instance. */
static EIQ_Camera_t s_camera;
/* Address of received buffer from the camera. */
static uint32_t s_cameraReceivedFrameAddr;
/* Ready callback. */
static EIQ_IBufferAddrUpdater_t s_readyCallback = NULL;

#include "eiq_video_worker.h"

/* Camera buffer located in noncachable memory block. */
#if !defined(__ARMCC_VERSION)
AT_NONCACHEABLE_SECTION_ALIGN(
        static uint8_t s_buffer[DEMO_CAMERA_BUFFER_COUNT][DEMO_CAMERA_HEIGHT][DEMO_CAMERA_WIDTH*DEMO_CAMERA_BUFFER_BPP],
        DEMO_CAMERA_BUFFER_ALIGN);
#else
AT_NONCACHEABLE_SECTION_ALIGN_INIT(
    static uint8_t s_buffer[DEMO_CAMERA_BUFFER_COUNT][DEMO_CAMERA_HEIGHT][DEMO_CAMERA_WIDTH*DEMO_CAMERA_BUFFER_BPP],
    DEMO_CAMERA_BUFFER_ALIGN);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Starts transfer from camera.
 *
 * This function starts transfer from camera to buffer. Ready handler is called
 * when data are prepared.
 */
static void start(void)
{
    CAMERA_RECEIVER_Start(&cameraReceiver);
}

/*!
 * @brief Gets camera capture dimensions.
 *
 * This function gets camera capture dimensions.
 *
 * @return Display dimensions.
 */
static Dims_t getResolution(void)
{
    Dims_t dims;
    dims.width = DEMO_CAMERA_WIDTH;
    dims.height = DEMO_CAMERA_HEIGHT;
    return dims;
}

/*!
 * @brief Notifies camera.
 *
 * This function notifies camera driver that data in buffer could be overwritten.
 */
static void notify(void)
{
    CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver,
            s_cameraReceivedFrameAddr);
    CAMERA_RECEIVER_Start(&cameraReceiver);
}

/*!
 * @brief Sets ready callback.
 *
 * This function sets external callback which is called when
 * camera buffer is ready.
 *
 * @param updater callback.
 */
static void setReadyCallback(EIQ_IBufferAddrUpdater_t updater)
{
    s_readyCallback = updater;
}

/*!
 * @brief Callback from LCD driver.
 *
 * This function is called from LCD, when buffer is empty.
 *
 * @param handle is not used in this implementation.
 * @param status is not used in this implementation.
 */
static void bufferReady(camera_receiver_handle_t *handle, status_t status,
        void *userData)
{
    if (s_readyCallback != NULL
            && CAMERA_RECEIVER_GetFullBuffer(&cameraReceiver,
                    &s_cameraReceivedFrameAddr) == kStatus_Success)
    {
        s_readyCallback(s_cameraReceivedFrameAddr);
    }
}

/*!
 * @brief Initializes camera.
 */
static void init(void)
{
    camera_config_t cameraConfig;
    memset(&cameraConfig, 0, sizeof(cameraConfig));
    memset(s_buffer, 0, sizeof(s_buffer));

    BOARD_InitCameraResource();

    cameraConfig.pixelFormat = DEMO_CAMERA_PIXEL_FORMAT;
    cameraConfig.bytesPerPixel = DEMO_CAMERA_BUFFER_BPP;
    cameraConfig.resolution = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH,
            DEMO_CAMERA_HEIGHT);
    /* Set the camera buffer stride according to panel, so that if
     * camera resoution is smaller than display, it can still be shown
     * correct in the screen.
     */
    cameraConfig.frameBufferLinePitch_Bytes = DEMO_CAMERA_WIDTH
            * DEMO_CAMERA_BUFFER_BPP, 
    cameraConfig.interface    = kCAMERA_InterfaceGatedClock,
    cameraConfig.controlFlags = DEMO_CAMERA_CONTROL_FLAGS,
    cameraConfig.framePerSec  = DEMO_CAMERA_FRAME_RATE,

    CAMERA_RECEIVER_Init(&cameraReceiver, &cameraConfig, bufferReady, NULL);

#if ( DEMO_CAMERA == DEMO_CAMERA_RM68191 || DEMO_CAMERA == DEMO_CAMERA_RM68200 )

  BOARD_InitMipiCsi();

  cameraConfig.pixelFormat   = kVIDEO_PixelFormatYUYV;
  cameraConfig.bytesPerPixel = 2;
  cameraConfig.resolution    = FSL_VIDEO_RESOLUTION(DEMO_CAMERA_WIDTH, DEMO_CAMERA_HEIGHT);
  cameraConfig.interface     = kCAMERA_InterfaceMIPI;
  cameraConfig.controlFlags  = DEMO_CAMERA_CONTROL_FLAGS;
  cameraConfig.framePerSec   = DEMO_CAMERA_FRAME_RATE;
  cameraConfig.csiLanes      = DEMO_CAMERA_MIPI_CSI_LANE;
#endif

    if (kStatus_Success != CAMERA_DEVICE_Init(&cameraDevice, &cameraConfig))
    {
        PRINTF("Camera device initialization failed\r\n");
    }

    CAMERA_DEVICE_Start(&cameraDevice);

    /* Submit the empty frame buffers to buffer queue. */
    for (uint32_t i = 0; i < DEMO_CAMERA_BUFFER_COUNT; i++)
    {
        CAMERA_RECEIVER_SubmitEmptyBuffer(&cameraReceiver,
                (uint32_t) (s_buffer[i]));
    }

}

/*!
 * @brief Initializes camera.
 *
 * This function initializes camera.
 *
 * @return Pointer to initialized camera instance.
 */
EIQ_Camera_t* EIQ_InitCamera(void)
{
    s_camera.base.getResolution = getResolution;
    s_camera.base.notify = notify;
    s_camera.base.start = start;
    s_camera.setReadyCallback = setReadyCallback;

    init();

    return &s_camera;
}
