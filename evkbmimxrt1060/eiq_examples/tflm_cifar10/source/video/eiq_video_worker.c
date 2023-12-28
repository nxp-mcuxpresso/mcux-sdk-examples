/*
 * Copyright 2020-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>
#include <string.h>

#include "image_utils.h"
#include "eiq_video_worker.h"
#include "eiq_display.h"
#include "eiq_camera.h"
#include "fsl_debug_console.h"
#include "gprintf/chgui.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* Number of loop before data are copied from camera. */
#ifndef EIQ_COUNTER_TOP
#define EIQ_COUNTER_TOP 4
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/
/*! @brief VideoWorker instance. */
static EIQ_VideoWorker_t s_worker;

/*! @brief VideoWorker handler. It can be called when data are ready.*/
static EIQ_IWorkerUpdater_t s_workerHandler;
/*! @brief Window keeps size of the capture rectangle. */
static Rect_t s_window;
/*! @brief Counting loops without data extraction. */
static volatile uint8_t s_counter = EIQ_COUNTER_TOP;

/*! @brief Pointer to keep address of allocated memory for captured data. */
static uint8_t *s_pExtract = NULL;
/*! @brief Variable for storing initial state of the display. */
static bool s_startDisplay = true;

/*!
 * @brief Start data transfer.
 *
 * This function runs camera and display transfer. It is not blocking function.
 * Display is notified when camera buffer is ready.
 * Check if data are ready using isReady function.
 */
static void start(void)
{
    /* Check if capture windows is created. */
    if (s_window.height == 0 || s_window.width == 0)
    {
        PRINTF("Default capture rate width = %d%%,%d%% is used.\r\n",
        EIQ_DEFAULT_CAPTURE_RATE, EIQ_DEFAULT_CAPTURE_RATE);
        s_worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
    }

    s_worker.sender->setReadyCallback(s_worker.receiver->base.notify);
    s_worker.receiver->setReadyCallback(s_worker.base.refresh);
    s_startDisplay = true;
    s_worker.receiver->base.start();
}

/*!
 * @brief Check if data is ready.
 *
 * This function check if data is ready.
 *
 * @return True in case data is ready
 */
static bool isReady(void)
{
    return !s_counter;
}

/*!
 * @brief Notify data transfer.
 *
 * This function notify data transfer.
 *
 */
static void notify(void)
{
    s_worker.receiver->base.notify();
}

/*!
 * @brief Get rectangle dimensions.
 *
 * This function returns dimensions of the captured window.
 *
 * @return Dimensions of the captured window
 */
static Dims_t getResolution(void)
{
    if (s_pExtract == NULL)
    {
        s_worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
    }

    Dims_t dims;
    dims.width = s_window.width;
    dims.height = s_window.height;
    return dims;
}

/*!
 * @brief Gets extracted data from camera.
 *
 * This function returns extracted data from camera.
 *
 * @return extracted data from camera
 */
static uint8_t* getData(void)
{
    if (s_pExtract == NULL)
    {
        /* Allocate capture windows for default size. */
        s_worker.setCaptureWindowHeightRate(EIQ_DEFAULT_CAPTURE_RATE);
    }

    return (uint8_t*) s_pExtract;
}

/*!
 * @brief Refreshes data on the screen.
 *
 * This function extracts camera data and extract them to LCD buffer.
 * This data is extracted to extract buffer for user. LCD buffer data
 * is modified by drawing to pixels and LCD buffer address is sent to
 * LCD device.
 *
 * @param camera buffer address with stored data
 *
 */
static void refresh(uint32_t bufferAddr)
{
    if (s_startDisplay)
    {
        s_worker.sender->base.start();
        s_startDisplay = false;
        s_worker.receiver->base.notify();
    }

    uint32_t lcdAddr = s_worker.sender->getEmptyBuffer();
    EIQ_PXP_Rotate(bufferAddr, lcdAddr);
    uint16_t *lcdPtr = (uint16_t*) lcdAddr;
    /* Check if camera buffer is extracted for new inference. */
    if (++s_counter > EIQ_COUNTER_TOP)
    {
        /* Extract image from camera. */
        IMAGE_ExtractRect(s_pExtract, s_window.x, s_window.y, s_window.width,
                s_window.height, lcdPtr, DEMO_PANEL_WIDTH);
        /* Draw red rectangle. */
        IMAGE_DrawRect(lcdPtr, s_window.x, s_window.y, s_window.width, s_window.height,
                255, 0, 0, DEMO_PANEL_WIDTH);

        s_counter = 0;
        if (s_workerHandler != NULL)
        {
            s_workerHandler((EIQ_IWorker_t*) &s_worker);
        }
    }
    else
    {
        /* Draw white rectangle. */
        IMAGE_DrawRect(lcdPtr, s_window.x, s_window.y, s_window.width, s_window.height,
                255, 255, 255, DEMO_PANEL_WIDTH);

    }
    GUI_DrawBuffer(lcdPtr);

    s_worker.sender->base.notify();

}

/*!
 * @brief Sets ready Callback
 *
 * This function stores address to external function which
 * could be called when data are ready.
 *
 * @param iworker address of external handler
 *
 */
static void setReadyCallback(EIQ_IWorkerUpdater_t iworker)
{
    if (iworker != NULL)
    {
        s_workerHandler = iworker;
    }
}

/*!
 * @brief Set capture window height rate
 *
 * This function sets capture window rectangle size
 * relative to display height.
 *
 * @param capture window rectangle size
 * relative to display height (defined in %).
 *
 */
static status_t setCaptureWindowHeightRate(int heightRate)
{
    if (heightRate < 10)
    {
        PRINTF("Incorrect input rate. Set value between 10-100.\r\n");
        return kStatus_Fail;
    }

    Dims_t lcdDims = s_worker.sender->base.getResolution();
    int dy = (lcdDims.height * (100 - heightRate)) / 200;

    s_window.y = dy;
    s_window.height = lcdDims.height - 2 * dy;
    s_window.width = s_window.height;
    s_window.x = (lcdDims.width - s_window.width) / 2;

    if (s_pExtract != NULL)
    {
        free(s_pExtract);
    }

    s_pExtract = (uint8_t*) malloc(s_window.width * s_window.height * 3);
    if (s_pExtract == NULL)
    {
        PRINTF("Unable to allocate internal buffer\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

/*!
 * @brief Initialize the VideoWorker.
 *
 * This function initialize camera and display.
 *
 * @return Pointer to initialized VideoWorker.
 */
EIQ_VideoWorker_t* EIQ_InitVideoWorker(void)
{
    s_worker.base.start = start;
    s_worker.base.isReady = isReady;
    s_worker.base.getResolution = getResolution;
    s_worker.base.notify = notify;
    s_worker.base.getData = getData;
    s_worker.base.refresh = refresh;
    s_worker.base.setReadyCallback = setReadyCallback;
    s_worker.setCaptureWindowHeightRate = setCaptureWindowHeightRate;
    EIQ_PXP_Init();
    s_worker.receiver = EIQ_InitCamera();
    s_worker.sender = EIQ_InitDisplay();

    return &s_worker;
}

