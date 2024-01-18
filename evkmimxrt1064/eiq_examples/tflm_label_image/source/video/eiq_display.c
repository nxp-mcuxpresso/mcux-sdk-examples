/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "eiq_display.h"
#include "pin_mux.h"
#include "fsl_debug_console.h"
#include "eiq_pxp.h"

/*******************************************************************************
 * Variables
 ******************************************************************************/

/* Display instance. */
static EIQ_Display_t s_display;
/* Currently used buffer index. */
static volatile uint8_t s_bufferIdx = 1;
/* Frame had been shown flag. */
static volatile bool s_newFrameShown = false;
/* Keeps frame buffer information. */
static dc_fb_info_t s_fbInfo;
/* Ready callback. */
static EIQ_IUpdater_t s_readyCallback;

/* Display buffer located in noncachable memory block. */
#if !defined(__ARMCC_VERSION)
AT_NONCACHEABLE_SECTION_ALIGN(
        static uint8_t s_buffer[DEMO_LCD_BUFFER_COUNT][DEMO_PANEL_HEIGHT] [DEMO_PANEL_WIDTH * DEMO_LCD_BUFFER_BPP],
        DEMO_FRAME_BUFFER_ALIGN);
#else
AT_NONCACHEABLE_SECTION_ALIGN_INIT(
    static uint8_t s_buffer[DEMO_LCD_BUFFER_COUNT][DEMO_PANEL_HEIGHT]
                                  [DEMO_PANEL_WIDTH * DEMO_LCD_BUFFER_BPP],
                                  DEMO_FRAME_BUFFER_ALIGN);
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Starts transfer to the display.
 *
 * This function sends first frame buffer to device. BufferReleased is called
 * after frame buffer had been sent.
 */
static void start(void)
{
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void*) s_buffer[s_bufferIdx]);
}

/*!
 * @brief Gets display screen dimensions.
 *
 * This function gets dimensions of the display screen.
 *
 * @return Display dimensions.
 */
static Dims_t getResolution(void)
{
    Dims_t dims;
    dims.width = DEMO_PANEL_WIDTH;
    dims.height = DEMO_PANEL_HEIGHT;
    return dims;
}

/*!
 * @brief Notifies display.
 *
 * This function notifies display driver that data in buffer are ready
 * and buffer could be used as new frame.
 */
static void notify(void)
{
    g_dc.ops->setFrameBuffer(&g_dc, 0, (void*) s_buffer[s_bufferIdx]);
    s_bufferIdx ^= 1U;
}

/*!
 * @brief Gets empty buffer
 *
 * @return empty LCD buffer which could be overwritten by new data and then
 * could be shown in display.
 */
static uint32_t getEmptyBuffer(void)
{
    return (uint32_t) s_buffer[s_bufferIdx];
}

/*!
 * @brief Sets ready callback.
 *
 * This function sets external callback which is called when
 * LCD buffer is empty and can be overwrite by new data.
 *
 * @param updater callback.
 */
static void setReadyCallback(EIQ_IUpdater_t updater)
{
    s_readyCallback = updater;
}

/*!
 * @brief Callback from LCD driver.
 *
 * This function is called from LCD, when buffer is empty.
 * It changes s_newFrameShown flag and it calls ready callback.
 * It could be used to trigger camera.
 *
 * @param param is not used in this implementation.
 * @param switchOffBuffer is not used in this implementation.
 */
static void bufferReleased(void *param, void *switchOffBuffer)
{
    s_newFrameShown = true;
    if (s_readyCallback != NULL)
    {
        s_readyCallback();
    }
}

/*!
 * @brief Initializes display.
 */
static void init(void)
{
    status_t status;

    memset(s_buffer, 0, sizeof(s_buffer));

    status = g_dc.ops->init(&g_dc);
    if (kStatus_Success != status)
    {
        PRINTF("Display initialization failed\r\n");
        assert(0);
    }

    g_dc.ops->getLayerDefaultConfig(&g_dc, 0, &s_fbInfo);
    s_fbInfo.pixelFormat = kVIDEO_PixelFormatRGB565;
    s_fbInfo.width = DEMO_PANEL_WIDTH;
    s_fbInfo.height = DEMO_PANEL_HEIGHT;
    s_fbInfo.strideBytes = DEMO_PANEL_WIDTH * DEMO_LCD_BUFFER_BPP;
    g_dc.ops->setLayerConfig(&g_dc, 0, &s_fbInfo);

    g_dc.ops->setFrameBuffer(&g_dc, 0, (void*) s_buffer[s_bufferIdx]);

    /* For the DBI interface display, application must wait for the first
     frame buffer sent to the panel. */
    if ((g_dc.ops->getProperty(&g_dc) & kDC_FB_ReserveFrameBuffer) == 0)
    {
        while (s_newFrameShown == false)
        {
        }
    }

    g_dc.ops->enableLayer(&g_dc, 0);
    g_dc.ops->setCallback(&g_dc, 0, bufferReleased, NULL);

}

/*!
 * @brief Initializes display.
 *
 * @return Pointer to initialized display instance.
 */
EIQ_Display_t* EIQ_InitDisplay(void)
{
    s_display.base.getResolution = getResolution;
    s_display.base.notify = notify;
    s_display.base.start = start;
    s_display.getEmptyBuffer = getEmptyBuffer;
    s_display.setReadyCallback = setReadyCallback;

    init();

    return &s_display;
}
