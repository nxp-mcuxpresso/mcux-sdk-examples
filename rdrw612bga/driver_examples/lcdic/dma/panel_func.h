/*
 * Copyright 2022 NXP
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
#define APP_LCD_BIG_ENDIAN    0
#define APP_LCD_LITTLE_ENDIAN 1

#if (defined(APP_PANEL_ADAFRUIT_2_8_CAPTATIVE) && APP_PANEL_ADAFRUIT_2_8_CAPTATIVE)
#define APP_LCD_HEIGHT 240u
#define APP_LCD_WIDTH  320u
#define APP_LCDIC_MODE kLCDIC_4WireSPI
#define APP_LCDIC_SPI_FLAG \
    (kLCDIC_SPI_MsbFirst | kLCDIC_SPI_ClkActiveLow | kLCDIC_SPI_ClkPhaseSecondEdge | kLCDIC_SPI_DcCmdLow)

#define APP_LCDI_ENDIAN APP_LCD_BIG_ENDIAN

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
