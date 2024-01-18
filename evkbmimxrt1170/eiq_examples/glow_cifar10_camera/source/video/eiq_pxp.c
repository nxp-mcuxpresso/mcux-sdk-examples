/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "eiq_pxp.h"
#include "eiq_display.h"
#include "eiq_camera.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* PXP instance. */
#define APP_PXP PXP

/* PXP surface position. */
#define APP_PS_ULC_X 0U
#define APP_PS_ULC_Y 0U
#define APP_PS_LRC_X (APP_IMG_WIDTH -1U)
#define APP_PS_LRC_Y (APP_IMG_HEIGHT- 1U)

/* PXP color. */
#define APP_RED 0xF800U
#define APP_GREEN 0x07E0U
#define APP_BLUE 0x001FU
#define APP_WHITE 0xFFFFU

/* Pixel format definition. */
#define APP_DC_FORMAT kVIDEO_PixelFormatRGB565

#define APP_IMG_WIDTH DEMO_PANEL_WIDTH
#define APP_IMG_HEIGHT DEMO_PANEL_HEIGHT

/* PS input buffer is square. */
#if APP_IMG_WIDTH > APP_IMG_HEIGHT
#define APP_PS_SIZE APP_IMG_WIDTH
#else
#define APP_PS_SIZE APP_IMG_HEIGHT
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief PXP initialization.
 *
 * Initializes PXP driver for conversion image data from camera buffer to LCD buffer.
 *
 */
void EIQ_PXP_Init(void)
{
    PXP_Init(APP_PXP);

    /* PS configure. */
    const pxp_ps_buffer_config_t psBufferConfig = {
      .pixelFormat = APP_PXP_PS_FORMAT,
      .swapByte = false,
      .bufferAddr = 0U,
      .bufferAddrU = 0U,
      .bufferAddrV = 0U,
      .pitchBytes = APP_PS_SIZE * DEMO_CAMERA_BUFFER_BPP,
    };

    PXP_SetProcessSurfaceBackGroundColor(APP_PXP, 0U);

    PXP_SetProcessSurfaceBufferConfig(APP_PXP, &psBufferConfig);
#if DEMO_ROTATE_FRAME
  PXP_SetProcessSurfacePosition(APP_PXP, APP_PS_ULC_X, APP_PS_ULC_Y, APP_PS_LRC_Y, APP_PS_LRC_X);
#else
    PXP_SetProcessSurfacePosition(APP_PXP, APP_PS_ULC_X, APP_PS_ULC_Y,
            APP_PS_LRC_X, APP_PS_LRC_Y);
#endif
    /* Disable AS. */
    PXP_SetAlphaSurfacePosition(APP_PXP, 0xFFFFU, 0xFFFFU, 0U, 0U);

    pxp_output_buffer_config_t outputBufferConfig;
    /* Output config. */
    outputBufferConfig.pixelFormat = APP_PXP_OUT_FORMAT;
    outputBufferConfig.interlacedMode = kPXP_OutputProgressive;
    outputBufferConfig.buffer0Addr = 0U;
    outputBufferConfig.buffer1Addr = 0U;
    outputBufferConfig.pitchBytes = APP_IMG_WIDTH * DEMO_LCD_BUFFER_BPP;
#if DEMO_ROTATE_FRAME
  outputBufferConfig.width          = APP_IMG_HEIGHT;
  outputBufferConfig.height         = APP_IMG_WIDTH;
#else
    outputBufferConfig.width = APP_IMG_WIDTH;
    outputBufferConfig.height = APP_IMG_HEIGHT;
#endif

    PXP_SetOutputBufferConfig(APP_PXP, &outputBufferConfig);

    PXP_SetCsc1Mode(APP_PXP, APP_CSC1_MODE);
    /* Disable CSC1, it is enabled by default. */
    PXP_EnableCsc1(APP_PXP, APP_CSC1_MODE_ENABLE);
}

/*!
 * @brief PXP Rotate.
 *
 * This function copies and transforms input buffer data to the output data buffer.
 *
 */
void EIQ_PXP_Rotate(uint32_t input_buffer, uint32_t output_buffer)
{
    APP_PXP->PS_BUF = input_buffer;
    APP_PXP->OUT_BUF = output_buffer;
    /* Prepare next buffer for LCD. */
    PXP_SetRotateConfig(APP_PXP, kPXP_RotateOutputBuffer, APP_ROTATE_DISPLAY,
            APP_FLIP_DISPLAY);

    PXP_Start(APP_PXP);

    /* Wait for process complete. */
    while (!(kPXP_CompleteFlag & PXP_GetStatusFlags(APP_PXP)))
    {
    }

    PXP_ClearStatusFlags(APP_PXP, kPXP_CompleteFlag);
}
