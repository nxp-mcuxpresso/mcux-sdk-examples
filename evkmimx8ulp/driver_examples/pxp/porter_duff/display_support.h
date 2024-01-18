/*
 * Copyright 2021 NXP
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
/* Whether use external convertor such as MIPI2HDMI convertor (IT6161) */
#ifndef APP_DISPLAY_EXTERNAL_CONVERTOR
#define APP_DISPLAY_EXTERNAL_CONVERTOR 0
// #define APP_DISPLAY_EXTERNAL_CONVERTOR 1
#endif

#define DEMO_PANEL_RK055AHD091 0 /* NXP "RK055AHD091(rm68200) MIPI Rectangular Display  */
#define DEMO_PANEL_RK055MHD091 1 /* NXP "RK055MHD091(hx8394)  MIPI Rectangular Display  */

#if APP_DISPLAY_EXTERNAL_CONVERTOR
#define DEMO_PANEL_HEIGHT 480
#define DEMO_PANEL_WIDTH  720
#define DEMO_POL_FLAGS    (kLCDIF_DataEnableActiveHigh | kLCDIF_VsyncActiveLow | kLCDIF_HsyncActiveLow)
#define USE_XRGB8888      1
#else
#define DEMO_PANEL_HEIGHT 1280
#define DEMO_PANEL_WIDTH  720
#define DEMO_POL_FLAGS \
    (kLCDIF_DataEnableActiveHigh | kLCDIF_VsyncActiveLow | kLCDIF_HsyncActiveLow | kLCDIF_DriveDataOnRisingClkEdge)
#define USE_RGB565 1
#ifndef DEMO_PANEL
#define DEMO_PANEL DEMO_PANEL_RK055MHD091
#endif
#endif

#define DEMO_DPI_VIDEO_MODE    (kDSI_DpiNonBurstWithSyncPulse)
#define DEMO_DCNANO_CLK_SRC    (kCLOCK_Pcc5PlatIpSrcPll4Pfd0Div1)
#define DEMO_DCNANO_CLK_DIV    (0)
#define DEMO_PLL4_PFD0DIV1     (11U)
#define DEMO_PLL4_PFD0         (32U)
#define DEMO_MIPI_DSI_LANE_NUM 2

#define DEMO_LCDIF            LCDIF
#define DEMO_LCDIF_IRQn       DCNano_IRQn
#define DEMO_LCDIF_IRQHandler DCNano_IRQHandler

#define DEMO_BUFFER_WIDTH  DEMO_PANEL_WIDTH
#define DEMO_BUFFER_HEIGHT DEMO_PANEL_HEIGHT

#define DEMO_BUFFER_START_X (0U)
#define DEMO_BUFFER_START_Y (0U)

#define DEMO_MIPI_DSI MIPI_DSI

#define FRAME_BUFFER_ALIGN 32

#if APP_DISPLAY_EXTERNAL_CONVERTOR
/* GPIO */
#define APP_GPIO_IDX(ioId)          ((uint8_t)(((uint16_t)ioId) >> 8U))
#define APP_PIN_IDX(ioId)           ((uint8_t)ioId)
#define APP_IT6161_INT_PIN          (0x0013U) /* PTA19, 0x00: PTA, 0x13: pin 19 */
#define APP_IT6161_INT_PIN_INT_SEL  (kRGPIO_InterruptOutput2)
#define APP_IT6161_INT_PIN_INT_CFG  (kRGPIO_InterruptLogicOne)
#define APP_IT6161_INT_PIN_IRQ_NUM  (GPIOA_INT0_IRQn)
#define APP_IT6161_INT_PIN_IRQ_PRIO (5)
#endif

extern const dc_fb_t g_dc;
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_InitDisplayInterface(void);
void BOARD_InitLcdifClock(void);
status_t BOARD_PrepareDisplayController(void);
#endif /* _DISPLAY_SUPPORT_H_ */
