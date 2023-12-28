/*
 * Copyright 2020-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _EIQ_CAMERA_CONF_H_
#define _EIQ_CAMERA_CONF_H_

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

#include "fsl_camera.h"
#include "fsl_camera_receiver.h"
#include "fsl_camera_device.h"
#include "eiq_display_conf.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_CAMERA_MT9M114 0
#define DEMO_CAMERA_OV7725 1
#define DEMO_CAMERA_RM68191 2
#define DEMO_CAMERA_RM68200 3

#ifndef DEMO_CAMERA
#if defined(CPU_MIMXRT1052DVL6B) || defined(CPU_MIMXRT1062DVL6A) || defined(CPU_MIMXRT1062DVL6B) || defined(CPU_MIMXRT1064DVL6A)

#ifndef DEMO_CAMERA
#define DEMO_CAMERA DEMO_CAMERA_MT9M114
#endif
//#define DEMO_CAMERA DEMO_CAMERA_OV7725
#define DEMO_CAMERA_BUFFER_BPP 2
#define DEMO_CAMERA_BUFFER_COUNT 4
#define DEMO_CAMERA_PIXEL_FORMAT kVIDEO_PixelFormatRGB565
#define DEMO_DATA_BUS kCSI_DataBus8Bit
#define APP_PXP_PS_FORMAT kPXP_PsPixelFormatRGB565
#define DEMO_CAMERA_HEIGHT        272/*DEMO_PANEL_HEIGHT*/
#define DEMO_CAMERA_WIDTH         480/*DEMO_PANEL_WIDTH*/
#else // CPU_MIMXRT1176DVMAA

#ifndef DEMO_CAMERA
#define DEMO_CAMERA DEMO_CAMERA_RM68191
#endif
//#define DEMO_CAMERA DEMO_CAMERA_RM68200
#define DEMO_CAMERA_BUFFER_BPP 4
#define DEMO_CAMERA_BUFFER_COUNT 2
#define DEMO_CAMERA_PIXEL_FORMAT kVIDEO_PixelFormatXYUV
#define DEMO_DATA_BUS kCSI_DataBus24Bit
#define APP_PXP_PS_FORMAT kPXP_PsPixelFormatYUV1P444
#define DEMO_CAMERA_HEIGHT        720/*DEMO_PANEL_HEIGHT*/
#define DEMO_CAMERA_WIDTH         1280/*DEMO_PANEL_WIDTH*/
#endif
#endif // CPU_MIMXRT1176DVMAA

#define DEMO_CAMERA_FRAME_RATE    30
#define DEMO_CAMERA_CONTROL_FLAGS (kCAMERA_HrefActiveHigh | kCAMERA_DataLatchOnRisingEdge)
#define DEMO_CAMERA_BUFFER_ALIGN  64 /* Buffer should be 64 byte aligned. */
#define DEMO_CAMERA_MIPI_CSI_LANE 2

#if (((DEMO_CAMERA_WIDTH < DEMO_CAMERA_HEIGHT) && (DEMO_PANEL_WIDTH > DEMO_PANEL_HEIGHT)) || \
     ((DEMO_CAMERA_WIDTH > DEMO_CAMERA_HEIGHT) && (DEMO_PANEL_WIDTH < DEMO_PANEL_HEIGHT)))
#define DEMO_ROTATE_FRAME 1
#else
#define DEMO_ROTATE_FRAME 0
#endif

/*******************************************************************************
 * Variables
 ******************************************************************************/

extern camera_device_handle_t cameraDevice;
extern camera_receiver_handle_t cameraReceiver;

/*******************************************************************************
 * API
 ******************************************************************************/

/*!
 * @brief Prepares camera for initialization.
 */
void BOARD_EarlyPrepareCamera(void);

/*!
 * @brief Initializes camera controler.
 */
void BOARD_InitCameraResource(void);

/*!
 * @brief Initializes Mipi Csi Clock.
 */
void BOARD_InitMipiCsi(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _CAMERA_SUPPORT_H_ */
