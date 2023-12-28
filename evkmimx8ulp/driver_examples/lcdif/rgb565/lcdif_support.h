/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LCDIF_SUPPORT_H_
#define _LCDIF_SUPPORT_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_LCDIF            LCDIF
#define DEMO_LCDIF_IRQn       DCNano_IRQn
#define DEMO_LCDIF_IRQHandler DCNano_IRQHandler

#define MIPI_PANEL_RK055AHD091 0 /* 720 * 1280 */
#define MIPI_PANEL_RK055MHD091 1 /* 720 * 1280 */

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
#endif

#define DEMO_POL_FLAGS \
    (kLCDIF_DataEnableActiveHigh | kLCDIF_VsyncActiveLow | kLCDIF_HsyncActiveLow | kLCDIF_DriveDataOnRisingClkEdge)

/* Frame buffer must be 128 byte aligned. */
#define DEMO_FB_ALIGN 128

#define DEMO_MIPI_DSI          MIPI_DSI
#define DEMO_MIPI_DSI_LANE_NUM 2

#define DEMO_FB0_ADDR 0x80000000
#define DEMO_FB1_ADDR 0x80200000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_InitDisplayInterface(void);
void BOARD_InitLcdifClock(void);

#endif /* _LCDIF_SUPPORT_H_ */
