/*
 * Copyright 2022, 2024 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "panel_func.h"
#include "fsl_lcdic.h"
#include "fsl_dbi.h"

#if APP_PANEL_ADAFRUIT_2_8_CAPTATIVE
#include "fsl_ili9341.h"
#elif APP_PANEL_LCD_PAR_S035_SPI
#include "fsl_st7796s.h"
#elif APP_PANEL_LCD_PAR_S035_I8080
#include "fsl_st7796s.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const dbi_iface_xfer_ops_t s_dbiIfaceOps;
static dbi_iface_t s_dbiIface = {.xferOps = &s_dbiIfaceOps};

/*******************************************************************************
 * Code
 ******************************************************************************/

static status_t APP_LcdWriteCmdData(dbi_iface_t *dbiIface, uint8_t cmd, const void *data, uint32_t dataLen)
{
    lcdic_tx_xfer_t xfer;

    if (0U == dataLen)
    {
        LCDIC_SendCommandBlocking(LCDIC, cmd);
    }
    else
    {
        xfer.cmd            = cmd;
        xfer.teSyncMode     = kLCDIC_TeNoSync;
        xfer.trxTimeoutMode = kLCDIC_ShortTimeout;
        xfer.dataFormat     = kLCDIC_DataFormatByte;
        xfer.dataLen        = dataLen;
        xfer.txData         = data;

        LCDIC_SendDataArrayBlocking(LCDIC, &xfer);
    }

    return kStatus_Success;
}

static const dbi_iface_xfer_ops_t s_dbiIfaceOps =
{
    /*
     * Here only panel configuration function is used,
     * so only need to implement the writeCommandData.
     */
    .writeCommandData = APP_LcdWriteCmdData,
};

void APP_PanelSelectRegion(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    DBI_IFACE_SelectArea(&s_dbiIface, startX, startY, endX, endY);
}

#if APP_PANEL_ADAFRUIT_2_8_CAPTATIVE
static ili9341_handle_t s_ili9341Handle;
void APP_InitPanel(void)
{
    ILI9341_InitDBI(&s_ili9341Handle, NULL, &s_dbiIface);
    DBI_IFACE_SetDiplayOn(&s_dbiIface, true);
}

#elif APP_PANEL_LCD_PAR_S035_SPI
static st7796s_handle_t s_st7796sHandle;

static const st7796s_config_t s_st7796sConfig =
{
    .driverPreset = kST7796S_DriverPresetLCDPARS035,
    .pixelFormat = kST7796S_PixelFormatRGB565,
    .orientationMode = kST7796S_Orientation270,
    .teConfig = kST7796s_TEVSync,
    .invertDisplay = true,
    .flipDisplay = true,
    .bgrFilter = true,
};

void APP_InitPanel(void)
{
    ST7796S_Init(&s_st7796sHandle, &s_st7796sConfig, &s_dbiIface);

    DBI_IFACE_SetDiplayOn(&s_dbiIface, true);
}

#elif APP_PANEL_LCD_PAR_S035_I8080
static st7796s_handle_t s_st7796sHandle;

static const st7796s_config_t s_st7796sConfig =
{
    .driverPreset = kST7796S_DriverPresetLCDPARS035,
    .pixelFormat = kST7796S_PixelFormatRGB565,
    .orientationMode = kST7796S_Orientation270,
    .teConfig = kST7796S_TEDisabled,
    .invertDisplay = true,
    .flipDisplay = true,
    .bgrFilter = true,
};

void APP_InitPanel(void)
{
    ST7796S_Init(&s_st7796sHandle, &s_st7796sConfig, &s_dbiIface);

    DBI_IFACE_SetDiplayOn(&s_dbiIface, true);
}
#endif
