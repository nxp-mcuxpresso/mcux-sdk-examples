/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _ELCDIF_SUPPORT_H_
#define _ELCDIF_SUPPORT_H_

#include "fsl_mipi_dsi.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_ELCDIF            LCDIF
#define APP_ELCDIF_IRQn       eLCDIF_IRQn
#define APP_ELCDIF_IRQHandler eLCDIF_IRQHandler

#define MIPI_PANEL_RK055AHD091 0 /* 720 * 1280 */
#define MIPI_PANEL_RK055IQH091 1 /* 540 * 960  */
#define MIPI_PANEL_RK055MHD091 2 /* 720 * 1280 */

#ifndef USE_MIPI_PANEL
#define USE_MIPI_PANEL MIPI_PANEL_RK055MHD091
#endif

#if (USE_MIPI_PANEL == MIPI_PANEL_RK055AHD091)
#define APP_PANEL_HEIGHT 1280
#define APP_PANEL_WIDTH  720
#define APP_HSW          8
#define APP_HFP          32
#define APP_HBP          32
#define APP_VSW          2
#define APP_VFP          16
#define APP_VBP          14
#elif (USE_MIPI_PANEL == MIPI_PANEL_RK055MHD091)
#define APP_PANEL_HEIGHT 1280
#define APP_PANEL_WIDTH  720
#define APP_HSW          6
#define APP_HFP          12
#define APP_HBP          24
#define APP_VSW          2
#define APP_VFP          16
#define APP_VBP          14
#else
#define APP_PANEL_HEIGHT 960
#define APP_PANEL_WIDTH  540
#define APP_HSW          2
#define APP_HFP          32
#define APP_HBP          30
#define APP_VSW          2
#define APP_VFP          16
#define APP_VBP          14
#endif
#define APP_POL_FLAGS \
    (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow | kELCDIF_DriveDataOnFallingClkEdge)

/* Frame buffer data alignment, for better performance, the LCDIF frame buffer should be 64B align. */
#define FRAME_BUFFER_ALIGN 64
#define APP_IMG_HEIGHT     APP_PANEL_HEIGHT
#define APP_IMG_WIDTH      APP_PANEL_WIDTH

extern const MIPI_DSI_Type g_mipiDsi;
#define APP_MIPI_DSI          (&g_mipiDsi)
#define APP_MIPI_DSI_LANE_NUM 2

/*
 * The DPHY bit clock must be fast enough to send out the pixels, it should be
 * larger than:
 *
 *         (Pixel clock * bit per output pixel) / number of MIPI data lane
 *
 * Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
 * it is fast enough.
 */
#define APP_MIPI_DPHY_BIT_CLK_ENLARGE(origin) (((origin) / 8) * 9)

/* Should call BOARD_InitDisplayInterface to initialize display interface. */
#define APP_ELCDIF_HAS_DISPLAY_INTERFACE 1
/* When working with MIPI DSI, the output pixel is 24-bit pixel */
#define APP_DATA_BUS       24
#define APP_LCDIF_DATA_BUS kELCDIF_DataBus24Bit

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_InitDisplayInterface(void);
void BOARD_InitLcdifClock(void);
void BOARD_EnableLcdInterrupt(void);

#endif /* _ELCDIF_SUPPORT_H_ */
