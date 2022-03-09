/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _DISPLAY_SUPPORT_H_
#define _DISPLAY_SUPPORT_H_

#include "fsl_dc_fb.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_PANEL_TFT_PROTO_5 0 /* MikroE TFT Proto 5" CAPACITIVE FlexIO Display */
#define DEMO_PANEL_RK055AHD091 1 /* NXP "RK055HDMIPI4M" MIPI Rectangular Display  */
#define DEMO_PANEL_RK055IQH091 2 /* NXP RESERVED                                  */
#define DEMO_PANEL_RM67162     3 /* NXP "G1120B0MIPI" MIPI Circular Display       */
#define DEMO_PANEL_RK055MHD091 4 /* NXP "RK055MHD091A0-CTG MIPI Rectangular Display  */

/* @TEST_ANCHOR */

#ifndef DEMO_PANEL
#define DEMO_PANEL DEMO_PANEL_TFT_PROTO_5
#endif

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)

#ifndef DEMO_SSD1963_USE_RG565
#define DEMO_SSD1963_USE_RG565 1
#endif

#ifndef DEMO_SSD1963_USE_FLEXIO_SMARTDMA
#define DEMO_SSD1963_USE_FLEXIO_SMARTDMA 1
#endif

/*
 * Use the FlexIO 8080 panel
 */

#define DEMO_PANEL_WIDTH  800U
#define DEMO_PANEL_HEIGHT 480U

/* The FlexIO LCD frame buffer does not need dedicate SRAM, but to share the
 * same link script, the frame buffers with fixed address are used here.
 */
#define DEMO_BUFFER_FIXED_ADDRESS 1
/* Put frame buffer in PSRAM */
#define DEMO_BUFFER0_ADDR 0x28000000U
#define DEMO_BUFFER1_ADDR 0x28200000U

/* Definitions for the frame buffer. */
#define DEMO_BUFFER_COUNT  1 /* 1 is enough for DBI interface display. */
#define FRAME_BUFFER_ALIGN 4 /* SMARTDMA buffer should be 4 byte aligned. */

#define DEMO_BUFFER_WIDTH  (DEMO_PANEL_WIDTH)
#define DEMO_BUFFER_HEIGHT (DEMO_PANEL_HEIGHT)

/* Where the frame buffer is shown in the screen. */
#define DEMO_BUFFER_START_X 0U
#define DEMO_BUFFER_START_Y 0U

/*
 * The chip only supports 8-bit 8080 data bus, so the pixels sent to LCD controller
 * should be RGB888 or BGR888. When SMARTDMA used, the SMARTDMA could read RGB565 from frame
 * buffer, convert to RGB888 and send out.
 */
#if DEMO_SSD1963_USE_RG565
#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB565
#define DEMO_BUFFER_BYTE_PER_PIXEL 2
#else
#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB888
#define DEMO_BUFFER_BYTE_PER_PIXEL 3
#endif

#if !DEMO_SSD1963_USE_FLEXIO_SMARTDMA
#error Currently only support FLEXIO MCULCD SMARTDMA
#endif

#elif ((DEMO_PANEL_RK055AHD091 == DEMO_PANEL) || (DEMO_PANEL_RK055IQH091 == DEMO_PANEL) || \
       (DEMO_PANEL_RK055MHD091 == DEMO_PANEL))

#define DEMO_BUFFER_FIXED_ADDRESS 1
/*
 * To get best performance, frame buffer should be in dedicate SRAM partition.
 * But due to the high resolution of MIPI panel and limitted SRAM size, here we put
 * frame buffer in on-board PSRAM.
 */

/*
 * Use the MIPI dumb panel
 */

/* Definitions for the frame buffer. */
#define DEMO_BUFFER_COUNT         2   /* 2 is enough for DPI interface display. */
#define FRAME_BUFFER_ALIGN        128 /* LCDIF buffer should be 128 byte aligned. */

#if ((DEMO_PANEL_RK055AHD091 == DEMO_PANEL) || (DEMO_PANEL_RK055MHD091 == DEMO_PANEL))

#ifndef DEMO_RK055AHD091_USE_XRGB8888
#define DEMO_RK055AHD091_USE_XRGB8888 0
#endif

#ifndef DEMO_RK055MHD091_USE_XRGB8888
#define DEMO_RK055MHD091_USE_XRGB8888 0
#endif

#if (DEMO_RK055AHD091_USE_XRGB8888 || DEMO_RK055MHD091_USE_XRGB8888)

/* Frame buffer #0 is 720 x 1280 x 4 = 0x384000 bytes long */
#define DEMO_BUFFER0_ADDR 0x28000000U
#define DEMO_BUFFER1_ADDR 0x28384000U

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatXRGB8888
#define DEMO_BUFFER_BYTE_PER_PIXEL 4

#else

/* Frame buffer #0 is 720 x 1280 x 2 = 0x1C2000 bytes long */
#define DEMO_BUFFER0_ADDR 0x28000000U
#define DEMO_BUFFER1_ADDR 0x28200000U

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB565
#define DEMO_BUFFER_BYTE_PER_PIXEL 2

#endif

#define DEMO_PANEL_WIDTH  (720)
#define DEMO_PANEL_HEIGHT (1280)

#elif (DEMO_PANEL_RK055IQH091 == DEMO_PANEL)

#define DEMO_BUFFER0_ADDR 0x28000000U
#define DEMO_BUFFER1_ADDR 0x28200000U

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB565
#define DEMO_BUFFER_BYTE_PER_PIXEL 2

#define DEMO_PANEL_WIDTH  (540)
#define DEMO_PANEL_HEIGHT (960)

#endif

#define DEMO_BUFFER_WIDTH   DEMO_PANEL_WIDTH
#define DEMO_BUFFER_HEIGHT  DEMO_PANEL_HEIGHT

/* Where the frame buffer is shown in the screen. */
#define DEMO_BUFFER_START_X 0U
#define DEMO_BUFFER_START_Y 0U

#elif (DEMO_PANEL_RM67162 == DEMO_PANEL)

/*
 * Use the MIPI smart panel
 *
 * MIPI_DSI + Smart DMA support three pixel formats:
 *
 * 1. RGB565: frame buffer format is RGB565, MIPI DSI send out data format is RGB565,
 *    to use this, set DEMO_RM67162_USE_RG565=1
 *
 * 2. RGB888: frame buffer format is RGB888, MIPI DSI send out data format is RGB888,
 *    to use this, set DEMO_RM67162_USE_RG565=0, DEMO_RM67162_USE_XRGB8888=0
 *
 * 3. XRGB8888: frame buffer format is XRGB888, SMARTDMA helps drop the useless byte
 *    and MIPI DSI send out data format is RGB888,
 *    to use this, set DEMO_RM67162_USE_RG565=0, DEMO_RM67162_USE_XRGB8888=1
 */

#ifndef DEMO_RM67162_USE_RG565
#define DEMO_RM67162_USE_RG565 1
#endif

#ifndef DEMO_RM67162_USE_XRGB8888
#define DEMO_RM67162_USE_XRGB8888 0
#endif

/* Pixel format macro mapping. */
#define DEMO_RM67162_BUFFER_RGB565   0
#define DEMO_RM67162_BUFFER_RGB888   1
#define DEMO_RM67162_BUFFER_XRGB8888 2

#if DEMO_RM67162_USE_RG565

#define DEMO_RM67162_BUFFER_FORMAT DEMO_RM67162_BUFFER_RGB565

#else

#if DEMO_RM67162_USE_XRGB8888
#define DEMO_RM67162_BUFFER_FORMAT DEMO_RM67162_BUFFER_XRGB8888
#else
#define DEMO_RM67162_BUFFER_FORMAT DEMO_RM67162_BUFFER_RGB888
#endif

#endif

/* Use SMARTDMA to send image to panel. */
#ifndef DEMO_RM67162_USE_DSI_SMARTDMA
#define DEMO_RM67162_USE_DSI_SMARTDMA 1
#endif

#if ((DEMO_RM67162_BUFFER_FORMAT == DEMO_RM67162_BUFFER_XRGB8888) && (!DEMO_RM67162_USE_DSI_SMARTDMA))
#error Must use SMARTDMA when use XRGB8888 pixel format
#endif

#define DEMO_BUFFER_FIXED_ADDRESS 1

/*
 * Place frame buffer in on-board PSRAM.
 */
#define DEMO_BUFFER0_ADDR         0x28000000U
#define DEMO_BUFFER1_ADDR         0x28200000U

/* Definitions for the frame buffer. */
/* 1 is enough, use 2 could render background buffer while display the foreground buffer. */
#define DEMO_BUFFER_COUNT         2
#define FRAME_BUFFER_ALIGN        4

#if (DEMO_RM67162_BUFFER_FORMAT == DEMO_RM67162_BUFFER_RGB565)

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB565
#define DEMO_BUFFER_BYTE_PER_PIXEL 2

#elif (DEMO_RM67162_BUFFER_FORMAT == DEMO_RM67162_BUFFER_RGB888)

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatRGB888
#define DEMO_BUFFER_BYTE_PER_PIXEL 3

#else

#define DEMO_BUFFER_PIXEL_FORMAT   kVIDEO_PixelFormatXRGB8888
#define DEMO_BUFFER_BYTE_PER_PIXEL 4

#endif /* DEMO_RM67162_BUFFER_FORMAT */

#define DEMO_PANEL_WIDTH  (400U)
#define DEMO_PANEL_HEIGHT (392U)

#define DEMO_BUFFER_WIDTH   (392U)
#define DEMO_BUFFER_HEIGHT  (392U)

/* Where the frame buffer is shown in the screen. */
#define DEMO_BUFFER_START_X 4U
#define DEMO_BUFFER_START_Y 0U

#endif

#define DEMO_BUFFER_STRIDE_BYTE (DEMO_BUFFER_WIDTH * DEMO_BUFFER_BYTE_PER_PIXEL)

extern const dc_fb_t g_dc;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

status_t BOARD_PrepareDisplayController(void);
void BOARD_DisplayTEPinHandler(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _DISPLAY_SUPPORT_H_ */
