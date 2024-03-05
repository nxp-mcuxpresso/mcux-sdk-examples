/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "panel_func.h"
#include "fsl_lcdic.h"

#if (defined(APP_PANEL_ADAFRUIT_2_8_CAPTATIVE) && APP_PANEL_ADAFRUIT_2_8_CAPTATIVE)
#include "fsl_ili9341.h"
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

/*******************************************************************************
 * Code
 ******************************************************************************/

static void APP_LcdWriteCmdData(uint8_t cmd, const uint8_t *data, uint32_t dataLen)
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
}

#if (defined(APP_PANEL_ADAFRUIT_2_8_CAPTATIVE) && APP_PANEL_ADAFRUIT_2_8_CAPTATIVE)
void APP_InitPanel(void)
{
    FT9341_Init1(APP_LcdWriteCmdData);

    APP_LcdWriteCmdData(ILI9341_CMD_MAC, (const uint8_t[]){0x28U}, 1u);
}

void APP_PanelSelectRegion(uint16_t startX, uint16_t startY, uint16_t endX, uint16_t endY)
{
    uint8_t data[4];
    /*Column addresses*/
    data[0] = (startX >> 8) & 0xFF;
    data[1] = startX & 0xFF;
    data[2] = (endX >> 8) & 0xFF;
    data[3] = endX & 0xFF;
    APP_LcdWriteCmdData(ILI9341_CMD_COLADDR, data, 4u);

    /*Page addresses*/
    data[0] = (startY >> 8) & 0xFF;
    data[1] = startY & 0xFF;
    data[2] = (endY >> 8) & 0xFF;
    data[3] = endY & 0xFF;
    APP_LcdWriteCmdData(ILI9341_CMD_PAGEADDR, data, 4u);
}
#endif
