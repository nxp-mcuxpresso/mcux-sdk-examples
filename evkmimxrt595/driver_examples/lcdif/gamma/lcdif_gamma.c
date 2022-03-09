/*
 * Copyright (c) 2019, NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_lcdif.h"
#include "lcdif_support.h"
#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "board.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define DEMO_IMG_HEIGHT     DEMO_PANEL_HEIGHT
#define DEMO_IMG_WIDTH      DEMO_PANEL_WIDTH
#define DEMO_BYTE_PER_PIXEL 4

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static uint32_t gammaTable[LCDIF_GAMMA_INDEX_MAX];
static volatile bool s_frameDone = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
void DEMO_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = LCDIF_GetAndClearInterruptPendingFlags(DEMO_LCDIF);

    if (0 != (intStatus & kLCDIF_Display0FrameDoneInterrupt))
    {
        s_frameDone = true;
    }
    SDK_ISR_EXIT_BARRIER;
}

void DEMO_LCDIF_Init(void)
{
    uint32_t(*frameBuffer)[DEMO_IMG_WIDTH] = (void *)DEMO_FB0_ADDR;

    lcdif_dpi_config_t dpiConfig = {
        .panelWidth    = DEMO_IMG_WIDTH,
        .panelHeight   = DEMO_IMG_HEIGHT,
        .hsw           = DEMO_HSW,
        .hfp           = DEMO_HFP,
        .hbp           = DEMO_HBP,
        .vsw           = DEMO_VSW,
        .vfp           = DEMO_VFP,
        .vbp           = DEMO_VBP,
        .polarityFlags = DEMO_POL_FLAGS,
        .format        = kLCDIF_Output24Bit,
    };

    /* Fill the buffer with gradual changed gray bars. */
    for (uint32_t i = 0; i < DEMO_IMG_HEIGHT; i++)
    {
        for (uint32_t j = 0; j < DEMO_IMG_WIDTH; j++)
        {
            frameBuffer[i][j] = ((j & 0xFFU) << 16U) | ((j & 0xFFU) << 8U) | ((j & 0xFFU));
        }
    }

    LCDIF_Init(DEMO_LCDIF);

    LCDIF_DpiModeSetConfig(DEMO_LCDIF, 0, &dpiConfig);

    LCDIF_SetFrameBufferStride(DEMO_LCDIF, 0, DEMO_IMG_WIDTH * DEMO_BYTE_PER_PIXEL);

    if (kStatus_Success != BOARD_InitDisplayInterface())
    {
        PRINTF("Display interface initialize failed\r\n");

        while (1)
        {
        }
    }

    NVIC_EnableIRQ(DEMO_LCDIF_IRQn);

    LCDIF_EnableInterrupts(DEMO_LCDIF, kLCDIF_Display0FrameDoneInterrupt);
}

void DEMO_LCDIF_Gamma(void)
{
    uint32_t i;
    uint8_t element;

    lcdif_fb_config_t fbConfig;

    /*
     * In this example, the gamma correction inverts the original picture.
     */
    for (i = 0; i < LCDIF_GAMMA_INDEX_MAX; i++)
    {
        element       = 255 - i;
        gammaTable[i] = (element << 16) | (element << 8) | (element << 0);
    }

    LCDIF_SetGammaData(DEMO_LCDIF, 0, 0, gammaTable, LCDIF_GAMMA_INDEX_MAX);

    /* Enable the LCDIF to show. */
    LCDIF_FrameBufferGetDefaultConfig(&fbConfig);

    fbConfig.enable      = true;
    fbConfig.enableGamma = false;
    fbConfig.format      = kLCDIF_PixelFormatXRGB8888;

    LCDIF_SetFrameBufferAddr(DEMO_LCDIF, 0, DEMO_FB0_ADDR);

    LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);

    while (1)
    {
        /* Show some frame. */
        for (uint32_t frame = 0x100; frame > 0; frame--)
        {
            s_frameDone = false;
            while (!s_frameDone)
            {
            }
        }

        fbConfig.enableGamma = !fbConfig.enableGamma;

        LCDIF_SetFrameBufferConfig(DEMO_LCDIF, 0, &fbConfig);
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    BOARD_InitPins();
    BOARD_InitMipiPanelPins();
    BOARD_InitPsRamPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitLcdifClock();

    GPIO_PortInit(GPIO, BOARD_MIPI_POWER_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_BL_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_RST_PORT);

    status_t status = BOARD_InitPsRam();
    if (status != kStatus_Success)
    {
        assert(false);
    }

    PRINTF("LCDIF gamma example start...\r\n");

    DEMO_LCDIF_Init();

    DEMO_LCDIF_Gamma();

    while (1)
    {
    }
}
