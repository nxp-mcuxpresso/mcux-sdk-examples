/*
 * Copyright 2023,2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _LCDIF_SUPPORT_H_
#define _LCDIF_SUPPORT_H_

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_LCDIF            LCDIF
#define DEMO_LCDIF_IRQn       LCDIF_IRQn
#define DEMO_LCDIF_IRQHandler LCDIF_IRQHandler

#if USE_DBI

#define DEMO_FB0_ADDR 0x60000000U

#define PANEL_TFT_PROTO_5  0 /* MikroE TFT Proto 5" CAPACITIVE FlexIO/LCD DBI Display */
#define MIPI_PANEL_RM67162 1 /* NXP "G1120B0MIPI" MIPI Circular Display */

#ifndef USE_DBI_PANEL
#define USE_DBI_PANEL PANEL_TFT_PROTO_5
#endif

#if (USE_DBI_PANEL == PANEL_TFT_PROTO_5)
/* Definitions for PANEL_TFT_PROTO_5 */
/* SSD1963 XTAL_IN clock frequency, decided by the LCD board, don't change. */
#define DEMO_SSD1963_XTAL_FREQ 10000000U
/* Configures the SSD1963 output PCLK frequency, for 60Hz frame rate,
 * PCLK = (800 + 48 + 40 + 0) * (480 + 3 + 13 + 18) * 60 */
#define DEMO_SSD1963_PCLK_FREQ     30000000U
#define DEMO_SSD1963_HSW           48U
#define DEMO_SSD1963_HFP           40U
#define DEMO_SSD1963_HBP           0U
#define DEMO_SSD1963_VSW           3U
#define DEMO_SSD1963_VFP           13U
#define DEMO_SSD1963_VBP           18U
#define DEMO_SSD1963_POLARITY_FLAG 0U
#define DEMO_PANEL_WIDTH           (800U)
#define DEMO_PANEL_HEIGHT          (480U)

#define DEMO_BUFFER_START_X 0U
#define DEMO_BUFFER_START_Y 0U
#define DEMO_BUFFER_END_X   799U
#define DEMO_BUFFER_END_Y   479U
#else /* USE_DBI_PANEL == MIPI_PANEL_RM67162 */
/* Definitions for MIPI_PANEL_RM67162 */
#define DEMO_BUFFER_PIXEL_FORMAT kVIDEO_PixelFormatRGB565
#define DEMO_PANEL_WIDTH         (400U)
#define DEMO_PANEL_HEIGHT        (392U)
/* Where the frame buffer is shown in the screen. */
#define DEMO_BUFFER_START_X      4U
#define DEMO_BUFFER_START_Y      0U
#define DEMO_BUFFER_END_X        395U
#define DEMO_BUFFER_END_Y        391U
#define DEMO_MIPI_DSI_LANE_NUM   1
#define DEMO_HSW                 0U
#define DEMO_HFP                 0U
#define DEMO_HBP                 0U
#define DEMO_VSW                 0U
#define DEMO_VFP                 0U
#define DEMO_VBP                 0U
#endif

#else /* USE_DBI */

#define DEMO_FB0_ADDR 0x60000000U
#define DEMO_FB1_ADDR 0x60400000U

#define MIPI_PANEL_RK055AHD091 0 /* 720 * 1280 */
#define MIPI_PANEL_RK055IQH091 1 /* 540 * 960 */
#define MIPI_PANEL_RK055MHD091 2 /* 720 * 1280 */
#define MIPI_PANEL_RASPI_7INCH 3 /* 800 * 480 */

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
#define DEMO_POL_FLAGS \
    (kLCDIF_DataEnableActiveHigh | kLCDIF_VsyncActiveLow | kLCDIF_HsyncActiveLow | kLCDIF_DriveDataOnRisingClkEdge)

/* Frame buffer must be 128 byte aligned. */
#define DEMO_FB_ALIGN 128

#if ((!USE_DBI) && (USE_MIPI_PANEL == MIPI_PANEL_RASPI_7INCH))
#define DEMO_MIPI_DSI_LANE_NUM 1
#else
#define DEMO_MIPI_DSI_LANE_NUM 2
#endif
#define DEMO_MIPI_DSI_BIT_PER_PIXEL            24

/* Here the desired DPHY bit clock multiplied by ( 9 / 8 = 1.125) to ensure
 * it is fast enough.
 */
#define DEMO_MIPI_DPHY_BIT_CLK_ENLARGE(origin) (((origin) / 8) * 9)

/* For DC8000, the image buffer stride in memory must be 64 byte aligned. */
#define DEMO_BUFFER_STRIDE_BYTE                (((((DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL) - 1U) / 64U) + 1U) * 64U)
#endif /* USE_DBI */

#define DEMO_MIPI_DSI MIPI_DSI_HOST
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
status_t BOARD_InitDisplayInterface(void);
void BOARD_InitLcdifClock(void);

#endif /* _LCDIF_SUPPORT_H_ */
