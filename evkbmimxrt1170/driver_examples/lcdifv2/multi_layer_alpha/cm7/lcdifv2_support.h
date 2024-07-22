/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LCDIFV2_SUPPORT_H_
#define _LCDIFV2_SUPPORT_H_

#include "fsl_mipi_dsi.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_LCDIFV2            LCDIFV2
#define DEMO_LCDIFV2_IRQn       LCDIFv2_IRQn
#define DEMO_LCDIFV2_IRQHandler LCDIFv2_IRQHandler

#define MIPI_PANEL_RK055AHD091 0 /* 720 * 1280 */
#define MIPI_PANEL_RK055IQH091 1 /* 540 * 960 */
#define MIPI_PANEL_RK055MHD091 2 /* 720 * 1280 */
#define MIPI_PANEL_RASPI_7INCH 3 /* 800 * 480, Raspberry Pi 7" */

#ifndef USE_MIPI_PANEL
#define USE_MIPI_PANEL MIPI_PANEL_RK055MHD091
#endif

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
#define DEMO_PANEL_HEIGHT 1280
#define DEMO_PANEL_WIDTH  720
#define DEMO_HSW          8
#define DEMO_HFP          32
#define DEMO_HBP          32
#define DEMO_VSW          2
#define DEMO_VFP          16
#define DEMO_VBP          14
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
#define DEMO_PANEL_HEIGHT 1280
#define DEMO_PANEL_WIDTH  720
#define DEMO_HSW          6
#define DEMO_HFP          12
#define DEMO_HBP          24
#define DEMO_VSW          2
#define DEMO_VFP          16
#define DEMO_VBP          14
#elif (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
#define DEMO_PANEL_HEIGHT 480
#define DEMO_PANEL_WIDTH  800
#define DEMO_HSW          20
#define DEMO_HFP          70
#define DEMO_HBP          23
#define DEMO_VSW          2
#define DEMO_VFP          7
#define DEMO_VBP          21
#else
#define DEMO_PANEL_HEIGHT 960
#define DEMO_PANEL_WIDTH  540
#define DEMO_HSW          2
#define DEMO_HFP          32
#define DEMO_HBP          30
#define DEMO_VSW          2
#define DEMO_VFP          16
#define DEMO_VBP          14
#endif
#define DEMO_POL_FLAGS                                                                   \
    (kLCDIFV2_DataEnableActiveHigh | kLCDIFV2_VsyncActiveLow | kLCDIFV2_HsyncActiveLow | \
     kLCDIFV2_DriveDataOnFallingClkEdge)

/* Frame buffer must be 8 byte aligned. */
#define DEMO_FB_ALIGN              8
#define DEMO_BUFFER_COUNT          3
#define DEMO_BUFFER_BYTE_PER_PIXEL 2

extern const MIPI_DSI_Type g_mipiDsi;
#define DEMO_MIPI_DSI (&g_mipiDsi)
#if (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH)
#define DEMO_MIPI_DSI_LANE_NUM 1
#else
#define DEMO_MIPI_DSI_LANE_NUM 2
#endif

#define DEMO_FB0_ADDR ((uint32_t)s_frameBuffer[0])
#define DEMO_FB1_ADDR ((uint32_t)s_frameBuffer[1])
#define DEMO_FB2_ADDR ((uint32_t)s_frameBuffer[2])

/*
 * The DPHY bit clock must be fast enough to send out the pixels, it should be
 * larger than:
 *
 *         (Pixel clock * bit per output pixel) / number of MIPI data lane
 *
 * Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
 * it is fast enough.
 */
#define DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(origin) (((origin) / 8) * 9)

extern uint8_t s_frameBuffer[DEMO_BUFFER_COUNT][DEMO_PANEL_HEIGHT][DEMO_PANEL_WIDTH][DEMO_BUFFER_BYTE_PER_PIXEL];

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_InitDisplayInterface(void);
void BOARD_InitLcdifClock(void);

#endif /* _LCDIFV2_SUPPORT_H_ */
