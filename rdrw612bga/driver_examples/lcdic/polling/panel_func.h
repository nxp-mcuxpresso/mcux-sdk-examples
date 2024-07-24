/*
 * Copyright 2022, 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PANEL_FUNC_H_
#define _PANEL_FUNC_H_

#include <stdint.h>
#include "lcdic_panel_config.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* Adafruit 2.8 captative panel, SPI interface */
#ifndef APP_PANEL_ADAFRUIT_2_8_CAPTATIVE
#define APP_PANEL_ADAFRUIT_2_8_CAPTATIVE 0
#endif

/* LCD_PAR_S035 panel, SPI interface */
#ifndef APP_PANEL_LCD_PAR_S035_SPI
#define APP_PANEL_LCD_PAR_S035_SPI 0
#endif

/* LCD_PAR_S035 panel, Intel 8080 interface */
#ifndef APP_PANEL_LCD_PAR_S035_I8080
#define APP_PANEL_LCD_PAR_S035_I8080 0
#endif

#define APP_LCD_BIG_ENDIAN    0
#define APP_LCD_LITTLE_ENDIAN 1

#if APP_PANEL_ADAFRUIT_2_8_CAPTATIVE
#define APP_LCD_HEIGHT 240u
#define APP_LCD_WIDTH  320u
#define APP_LCDIC_MODE kLCDIC_4WireSPI
#define APP_LCDIC_SPI_FLAG \
    (kLCDIC_SPI_MsbFirst | kLCDIC_SPI_ClkActiveLow | kLCDIC_SPI_ClkPhaseSecondEdge | kLCDIC_SPI_DcCmdLow)

#define APP_LCDIC_ENDIAN APP_LCD_BIG_ENDIAN

#elif APP_PANEL_LCD_PAR_S035_SPI
/* LCD_PAR_S035 panel using SPI interface. */
#define APP_LCD_HEIGHT 320u
#define APP_LCD_WIDTH  480u
#define APP_LCDIC_MODE kLCDIC_4WireSPI
#define APP_LCDIC_SPI_FLAG \
    (kLCDIC_SPI_MsbFirst | kLCDIC_SPI_ClkActiveHigh | kLCDIC_SPI_ClkPhaseFirstEdge | kLCDIC_SPI_DcCmdLow)

#define APP_LCDIC_ENDIAN APP_LCD_BIG_ENDIAN

#elif APP_PANEL_LCD_PAR_S035_I8080
/* LCD_PAR_S035 panel using I8080 interface. */
#define APP_LCD_HEIGHT 320u
#define APP_LCD_WIDTH  480u
#define APP_LCDIC_MODE kLCDIC_I8080
#define APP_LCDIC_I8080_FLAG \
    (kLCDIC_I8080_CsActiveLow | kLCDIC_I8080_DcCmdLow | kLCDIC_I8080_WrActiveLow | kLCDIC_I8080_CsEnableIdleOff)

#define APP_LCDIC_ENDIAN APP_LCD_BIG_ENDIAN

#endif

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus*/

void APP_InitPanel(void);

void APP_PanelSelectRegion(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY);

#if defined(__cplusplus)
}
#endif /* __cplusplus*/

#endif /* _PANEL_FUNC_H_ */
