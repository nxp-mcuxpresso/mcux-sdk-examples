/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*  Standard C Included Files */
#include <stdio.h>
#include <string.h>
#include "pin_mux.h"
#include "board.h"
#include "fsl_lcdc.h"

#include "fsl_sctimer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_LCD            LCD
#define LCD_PANEL_CLK      9000000
#define LCD_PPL            480
#define LCD_HSW            2
#define LCD_HFP            8
#define LCD_HBP            43
#define LCD_LPP            272
#define LCD_VSW            10
#define LCD_VFP            4
#define LCD_VBP            12
#define LCD_POL_FLAGS      kLCDC_InvertVsyncPolarity | kLCDC_InvertHsyncPolarity
#define IMG_HEIGHT         272
#define IMG_WIDTH          480
#define LCD_INPUT_CLK_FREQ CLOCK_GetLcdClkFreq()
#define APP_LCD_IRQHandler LCD_IRQHandler
#define APP_LCD_IRQn       LCD_IRQn
#define APP_BIT_PER_PIXEL   2
#define APP_PIXEL_PER_BYTE  4
#define APP_PIXEL_MAX_VALUE 3
#define APP_PIXEL_MIN_VALUE 3

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if (defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__GNUC__))
__attribute__((aligned(8)))
#elif defined(__ICCARM__)
#pragma data_alignment = 8
#else
#error Toolchain not support.
#endif
static uint8_t s_frameBuf0[IMG_HEIGHT][IMG_WIDTH / APP_PIXEL_PER_BYTE];

#if (defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__GNUC__))
__attribute__((aligned(8)))
#elif defined(__ICCARM__)
#pragma data_alignment = 8
#else
#error Toolchain not support.
#endif
static uint8_t s_frameBuf1[IMG_HEIGHT][IMG_WIDTH / APP_PIXEL_PER_BYTE];

static const uint32_t s_frameBufAddr[] = {(uint32_t)s_frameBuf0, (uint32_t)s_frameBuf1};

/*
 * In this example, LCD controller works in 2 bpp mode, supports 4 colors,
 * the palette data format is:
 *
 * RGB format: (lcdConfig.swapRedBlue = false)
 * Bit(s)   Name     Description
 * 4:0      R[4:0]   Red palette data B[4:0] Blue palette data
 * 9:5      G[4:0]   Green palette data G[4:0] Green palette data
 * 14:10    B[4:0]   Blue palette data R[4:0] Red palette data
 * 15       I        Intensity / unused I Intensity / unused
 * 20:16    R[4:0]   Red palette data B[4:0] Blue palette data
 * 25:21    G[4:0]   Green palette data G[4:0] Green palette data
 * 30:26    B[4:0]   Blue palette data R[4:0] Red palette data
 * 31       I        Intensity / unused I Intensity / unused
 *
 * BGR format: (lcdConfig.swapRedBlue = true)
 * Bit(s)   Name     Description
 * 4:0      B[4:0]   Blue palette data
 * 9:5      G[4:0]   Green palette data
 * 14:10    R[4:0]   Red palette data
 * 15       I        Intensity / unused
 * 20:16    B[4:0]   Blue palette data
 * 25:21    G[4:0]   Green palette data
 * 30:26    R[4:0]   Red palette data
 * 31       I        Intensity / unused
 *
 * This example uses RGB format, the supported colors set in palette
 * are: black, red, green, and blue.
 */
static const uint32_t palette[] = {0x001F0000U, 0x7C0003E0U};

/* The index of the inactive buffer. */
static volatile uint8_t s_inactiveBufsIdx;

/* The new frame address already loaded to the LCD controller. */
static volatile bool s_frameAddrUpdated = false;

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_InitPWM(void)
{
    sctimer_config_t config;
    sctimer_pwm_signal_param_t pwmParam;
    uint32_t event;

    CLOCK_AttachClk(kMAIN_CLK_to_SCT_CLK);

    CLOCK_SetClkDiv(kCLOCK_DivSctClk, 2, true);

    SCTIMER_GetDefaultConfig(&config);

    SCTIMER_Init(SCT0, &config);

    pwmParam.output           = kSCTIMER_Out_5;
    pwmParam.level            = kSCTIMER_HighTrue;
    pwmParam.dutyCyclePercent = 5;

    SCTIMER_SetupPwm(SCT0, &pwmParam, kSCTIMER_CenterAlignedPwm, 1000U, CLOCK_GetSctClkFreq(), &event);
}

void APP_LCD_IRQHandler(void)
{
    uint32_t intStatus = LCDC_GetEnabledInterruptsPendingStatus(APP_LCD);

    LCDC_ClearInterruptsStatus(APP_LCD, intStatus);

    if (intStatus & kLCDC_BaseAddrUpdateInterrupt)
    {
        s_frameAddrUpdated = true;
    }
    __DSB();
    SDK_ISR_EXIT_BARRIER;
}

static void APP_Draw2BPPLine(uint8_t *line, uint16_t start, uint16_t end, uint8_t color)
{
    uint8_t i;
    uint16_t startByte;
    uint16_t endByte;

    startByte = start / APP_PIXEL_PER_BYTE;
    endByte   = end / APP_PIXEL_PER_BYTE;

    if (startByte == endByte)
    {
        for (i = (start & 0x03U); i < (end & 0x03U); i++)
        {
            line[startByte] = (line[startByte] & ~(0x03U << (i * 2U))) | (color << (i * 2U));
        }
    }
    else
    {
        for (i = (start & 0x03U); i < APP_PIXEL_PER_BYTE; i++)
        {
            line[startByte] = (line[startByte] & ~(0x03U << (i * 2U))) | (color << (i * 2U));
        }

        for (i = (startByte + 1U); i < endByte; i++)
        {
            line[i] = color * 0x55U;
        }

        for (i = 0U; i < (end & 0x03U); i++)
        {
            line[endByte] = (line[endByte] & ~(0x03U << (i * 2U))) | (color << (i * 2));
        }
    }
}

static void APP_FillBuffer(void *buffer)
{
    /* Background color. */
    static uint8_t bgColor = 0U;
    /* Foreground color. */
    static uint8_t fgColor = 1U;
    uint8_t colorToSet     = 0U;
    /* Position of the foreground rectangle. */
    static uint16_t upperLeftX  = 0U;
    static uint16_t upperLeftY  = 0U;
    static uint16_t lowerRightX = (IMG_WIDTH - 1U) / 2U;
    static uint16_t lowerRightY = (IMG_HEIGHT - 1U) / 2U;
    static int8_t incX          = 1;
    static int8_t incY          = 1;
    /* Change color in next forame or not. */
    static bool changeColor = false;

    uint32_t i, j;
    uint8_t(*buf)[IMG_WIDTH / APP_PIXEL_PER_BYTE] = buffer;

    /*
     +------------------------------------------------------------------------+
     |                                                                        |
     |                                                                        |
     |                                                                        |
     |                          Area 1                                        |
     |                                                                        |
     |                                                                        |
     |                  +---------------------------+                         |
     |                  |XXXXXXXXXXXXXXXXXXXXXXXXXXX|                         |
     |                  |XXXXXXXXXXXXXXXXXXXXXXXXXXX|                         |
     |    Area 2        |XXXXXXXXXXXXXXXXXXXXXXXXXXX|       Area 3            |
     |                  |XXXXXXXXXXXXXXXXXXXXXXXXXXX|                         |
     |                  |XXXXXXXXXXXXXXXXXXXXXXXXXXX|                         |
     |                  |XXXXXXXXXXXXXXXXXXXXXXXXXXX|                         |
     |                  +---------------------------+                         |
     |                                                                        |
     |                                                                        |
     |                                                                        |
     |                                                                        |
     |                         Area 4                                         |
     |                                                                        |
     |                                                                        |
     +------------------------------------------------------------------------+
     */

    /* Fill the frame buffer. */
    /* Fill area 1. */
    colorToSet = bgColor * 0x55U;
    for (i = 0; i < upperLeftY; i++)
    {
        for (j = 0; j < IMG_WIDTH / APP_PIXEL_PER_BYTE; j++)
        {
            /* Background color. */
            buf[i][j] = colorToSet;
        }
    }

    APP_Draw2BPPLine((uint8_t *)buf[i], 0, upperLeftX, bgColor);
    APP_Draw2BPPLine((uint8_t *)buf[i], upperLeftX, lowerRightX + 1, fgColor);
    APP_Draw2BPPLine((uint8_t *)buf[i], lowerRightX + 1, IMG_WIDTH, bgColor);

    for (i++; i <= lowerRightY; i++)
    {
        for (j = 0; j < (IMG_WIDTH / APP_PIXEL_PER_BYTE); j++)
        {
            buf[i][j] = buf[upperLeftY][j];
        }
    }

    /* Fill area 4. */
    colorToSet = bgColor * 0x55U;
    for (; i < IMG_HEIGHT; i++)
    {
        for (j = 0; j < IMG_WIDTH / APP_PIXEL_PER_BYTE; j++)
        {
            /* Background color. */
            buf[i][j] = colorToSet;
        }
    }

    /* Update the format: color and rectangle position. */
    upperLeftX += incX;
    upperLeftY += incY;
    lowerRightX += incX;
    lowerRightY += incY;

    changeColor = false;

    if (0U == upperLeftX)
    {
        incX        = 1;
        changeColor = true;
    }
    else if (IMG_WIDTH - 1 == lowerRightX)
    {
        incX        = -1;
        changeColor = true;
    }

    if (0U == upperLeftY)
    {
        incY        = 1;
        changeColor = true;
    }
    else if (IMG_HEIGHT - 1 == lowerRightY)
    {
        incY        = -1;
        changeColor = true;
    }

    if (changeColor)
    {
        if (APP_PIXEL_MAX_VALUE == fgColor)
        {
            fgColor = 1U;
        }
        else
        {
            fgColor++;
        }
    }
}

int main(void)
{
    lcdc_config_t lcdConfig;

    /* Route Main clock to LCD. */
    CLOCK_AttachClk(kMAIN_CLK_to_LCD_CLK);

    CLOCK_SetClkDiv(kCLOCK_DivLcdClk, 1, true);

    BOARD_InitPins();
    BOARD_BootClockPLL180M();

    /* Set the back light PWM. */
    BOARD_InitPWM();

    s_frameAddrUpdated = false;
    s_inactiveBufsIdx  = 1;

    APP_FillBuffer((void *)(s_frameBufAddr[0]));

    LCDC_GetDefaultConfig(&lcdConfig);

    lcdConfig.panelClock_Hz  = LCD_PANEL_CLK;
    lcdConfig.ppl            = LCD_PPL;
    lcdConfig.hsw            = LCD_HSW;
    lcdConfig.hfp            = LCD_HFP;
    lcdConfig.hbp            = LCD_HBP;
    lcdConfig.lpp            = LCD_LPP;
    lcdConfig.vsw            = LCD_VSW;
    lcdConfig.vfp            = LCD_VFP;
    lcdConfig.vbp            = LCD_VBP;
    lcdConfig.polarityFlags  = LCD_POL_FLAGS;
    lcdConfig.upperPanelAddr = (uint32_t)s_frameBufAddr[0];
    lcdConfig.bpp            = kLCDC_2BPP;
    lcdConfig.display        = kLCDC_DisplayTFT;
    lcdConfig.swapRedBlue    = false;

    LCDC_Init(APP_LCD, &lcdConfig, LCD_INPUT_CLK_FREQ);

    LCDC_SetPalette(APP_LCD, palette, ARRAY_SIZE(palette));

    LCDC_EnableInterrupts(APP_LCD, kLCDC_BaseAddrUpdateInterrupt);
    NVIC_EnableIRQ(APP_LCD_IRQn);

    LCDC_Start(APP_LCD);
    LCDC_PowerUp(APP_LCD);

    while (1)
    {
        /* Fill the inactive buffer. */
        APP_FillBuffer((void *)s_frameBufAddr[s_inactiveBufsIdx]);

        while (!s_frameAddrUpdated)
        {
        }

        /*
         * The buffer address has been loaded to the LCD controller, now
         * set the inactive buffer to active buffer.
         */
        LCDC_SetPanelAddr(APP_LCD, kLCDC_UpperPanel, (uint32_t)(s_frameBufAddr[s_inactiveBufsIdx]));

        s_frameAddrUpdated = false;
        s_inactiveBufsIdx ^= 1U;
    }
}
