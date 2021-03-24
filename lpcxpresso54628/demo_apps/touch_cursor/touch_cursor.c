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
#include "fsl_debug_console.h"
#include "fsl_lcdc.h"
#include "fsl_i2c.h"
#include "fsl_ft5406.h"

#include "fsl_sctimer.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_LCD                    LCD
#define LCD_PANEL_CLK              9000000
#define LCD_PPL                    480
#define LCD_HSW                    2
#define LCD_HFP                    8
#define LCD_HBP                    43
#define LCD_LPP                    272
#define LCD_VSW                    10
#define LCD_VFP                    4
#define LCD_VBP                    12
#define LCD_POL_FLAGS              kLCDC_InvertVsyncPolarity | kLCDC_InvertHsyncPolarity
#define IMG_HEIGHT                 272
#define IMG_WIDTH                  480
#define LCD_INPUT_CLK_FREQ         CLOCK_GetLcdClkFreq()
#define APP_LCD_IRQHandler         LCD_IRQHandler
#define APP_LCD_IRQn               LCD_IRQn
#define EXAMPLE_I2C_MASTER_BASE    (I2C2_BASE)
#define I2C_MASTER_CLOCK_FREQUENCY (12000000)
#define APP_PIXEL_PER_BYTE 8

#define EXAMPLE_I2C_MASTER         ((I2C_Type *)EXAMPLE_I2C_MASTER_BASE)
#define I2C_MASTER_SLAVE_ADDR_7BIT 0x7EU
#define I2C_BAUDRATE               100000U

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
static uint8_t s_frameBufs[IMG_HEIGHT][IMG_WIDTH / APP_PIXEL_PER_BYTE];
void *vram_ptr = s_frameBufs;

/* Frame end flag. */
static volatile bool s_frameEndFlag;

/* Color palette. */
static const uint32_t palette[] = {0x0000001F};

/* 32x32 pixel cursor image. */
#if (defined(__CC_ARM) || defined(__ARMCC_VERSION) || defined(__GNUC__))
__attribute__((aligned(4)))
#elif defined(__ICCARM__)
#pragma data_alignment = 4
#else
#error Toolchain not support.
#endif
static const uint8_t cursor32Img0[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 1.  */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 2.  */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 3.  */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 4.  */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 5.  */
    0xAA, 0xAA, 0xAA, 0xFA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 6.  */
    0xAA, 0xAA, 0xAB, 0xFE, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 7.  */
    0xAA, 0xAA, 0xAB, 0xFE, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 8.  */
    0xAA, 0xAA, 0xAB, 0xFE, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 9.  */
    0xAA, 0xAA, 0xAB, 0xFE, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 10  */
    0xAA, 0xAA, 0xAB, 0xFF, 0xEA, 0xAA, 0xAA, 0xAA, /* Line 11. */
    0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xAA, 0xAA, 0xAA, /* Line 12. */
    0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xFA, 0xAA, 0xAA, /* Line 13. */
    0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xFE, 0xAA, 0xAA, /* Line 14. */
    0xAA, 0xAB, 0xFB, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 15. */
    0xAA, 0xAB, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 16. */
    0xAA, 0xAB, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 17. */
    0xAA, 0xAA, 0xFF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 18. */
    0xAA, 0xAA, 0xBF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 19. */
    0xAA, 0xAA, 0xBF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 20. */
    0xAA, 0xAA, 0xAF, 0xFF, 0xFF, 0xFF, 0xAA, 0xAA, /* Line 21. */
    0xAA, 0xAA, 0xAF, 0xFF, 0xFF, 0xFE, 0xAA, 0xAA, /* Line 22. */
    0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xFE, 0xAA, 0xAA, /* Line 23. */
    0xAA, 0xAA, 0xAB, 0xFF, 0xFF, 0xFE, 0xAA, 0xAA, /* Line 24. */
    0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFA, 0xAA, 0xAA, /* Line 25. */
    0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFA, 0xAA, 0xAA, /* Line 26. */
    0xAA, 0xAA, 0xAA, 0xFF, 0xFF, 0xFA, 0xAA, 0xAA, /* Line 27. */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 28. */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 29. */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 30. */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, /* Line 31. */
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA  /* Line 32. */
};

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

    if (intStatus & kLCDC_VerticalCompareInterrupt)
    {
        s_frameEndFlag = true;
    }
    __DSB();
    SDK_ISR_EXIT_BARRIER;
}

static void APP_FillBuffer(void *buffer)
{
    /*
     * Fill the frame buffer, show rectangle in the center.
     */
    uint8_t(*buf)[IMG_WIDTH / APP_PIXEL_PER_BYTE] = buffer;

    uint32_t i, j;

    for (i = 0; i < IMG_HEIGHT; i++)
    {
        for (j = 0; j < IMG_WIDTH / APP_PIXEL_PER_BYTE; j++)
        {
            buf[i][j] = ((i % 32) == 31) ? 0xffU : ((j % 4) == 3) ? 0x01U : 0;
        }
    }

    for (i = IMG_HEIGHT / 4; i < IMG_HEIGHT * 3 / 4; i++)
    {
        for (j = IMG_WIDTH / APP_PIXEL_PER_BYTE / 4; j < IMG_WIDTH * 3 / APP_PIXEL_PER_BYTE / 4; j++)
        {
            buf[i][j] = 0xFFU;
        }
    }
}

status_t APP_LCDC_Init(void)
{
    /* Initialize the display. */
    lcdc_config_t lcdConfig;
    lcdc_cursor_config_t cursorConfig;

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
    lcdConfig.upperPanelAddr = (uint32_t)s_frameBufs;
    lcdConfig.bpp            = kLCDC_1BPP;
    lcdConfig.display        = kLCDC_DisplayTFT;
    lcdConfig.swapRedBlue    = false;
    lcdConfig.dataFormat     = kLCDC_WinCeMode;

    LCDC_Init(APP_LCD, &lcdConfig, LCD_INPUT_CLK_FREQ);

    LCDC_SetPalette(APP_LCD, palette, ARRAY_SIZE(palette));

    /* Setup the Cursor. */
    LCDC_CursorGetDefaultConfig(&cursorConfig);

    cursorConfig.size     = kLCDC_CursorSize32;
    cursorConfig.syncMode = kLCDC_CursorSync;
    cursorConfig.image[0] = (uint32_t *)cursor32Img0;

    LCDC_SetCursorConfig(APP_LCD, &cursorConfig);
    LCDC_ChooseCursor(APP_LCD, 0);

    /* Trigger interrupt at start of every vertical back porch. */
    LCDC_SetVerticalInterruptMode(APP_LCD, kLCDC_StartOfBackPorch);
    LCDC_EnableInterrupts(APP_LCD, kLCDC_VerticalCompareInterrupt);
    NVIC_EnableIRQ(APP_LCD_IRQn);

    LCDC_EnableCursor(APP_LCD, true);

    LCDC_Start(APP_LCD);
    LCDC_PowerUp(APP_LCD);

    return kStatus_Success;
}

status_t APP_I2C_Init(void)
{
    i2c_master_config_t masterConfig;

    I2C_MasterGetDefaultConfig(&masterConfig);

    /* Change the default baudrate configuration */
    masterConfig.baudRate_Bps = I2C_BAUDRATE;

    /* Initialize the I2C master peripheral */
    I2C_MasterInit(EXAMPLE_I2C_MASTER, &masterConfig, I2C_MASTER_CLOCK_FREQUENCY);

    return kStatus_Success;
}

void APP_SetCursorPosition(int posX, int posY)
{
    posX -= 12;
    posY -= 5;

    LCDC_SetCursorPosition(APP_LCD, posX, posY);
}

int main(void)
{
    int cursorPosX = 0U;
    int cursorPosY = 0U;

    ft5406_handle_t touch_handle;
    touch_event_t touch_event;

    status_t status;

    gpio_pin_config_t pin_config = {
        kGPIO_DigitalOutput,
        0,
    };

    /* Board pin, clock, debug console init */
    /* attach 12 MHz clock to FLEXCOMM0 (debug console) */
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* Route Main clock to LCD. */
    CLOCK_AttachClk(kMAIN_CLK_to_LCD_CLK);

    /* attach 12 MHz clock to FLEXCOMM2 (I2C master for touch controller) */
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM2);

    CLOCK_EnableClock(kCLOCK_Gpio2);

    CLOCK_SetClkDiv(kCLOCK_DivLcdClk, 1, true);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set the back light PWM. */
    BOARD_InitPWM();

    APP_FillBuffer((void *)(s_frameBufs));

    status = APP_LCDC_Init();
    if (status != kStatus_Success)
    {
        PRINTF("LCD init failed\n");
    }
    assert(status == kStatus_Success);

    status = APP_I2C_Init();
    if (status != kStatus_Success)
    {
        PRINTF("I2C init failed\n");
    }
    assert(status == kStatus_Success);

    /* Trigger LCD reset */
    GPIO_PinInit(GPIO, 2, 27, &pin_config);
    GPIO_PinWrite(GPIO, 2, 27, 1);

    status = FT5406_Init(&touch_handle, EXAMPLE_I2C_MASTER);
    if (status != kStatus_Success)
    {
        PRINTF("Touch panel init failed\n");
    }
    assert(status == kStatus_Success);

    for (;;)
    {
        if (kStatus_Success == FT5406_GetSingleTouch(&touch_handle, &touch_event, &cursorPosX, &cursorPosY))
        {
            if ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact))
            {
                /* Update cursor position */
                APP_SetCursorPosition(cursorPosY, cursorPosX);
                PRINTF("0x%2x 0x%2x", cursorPosX, cursorPosY);
                PRINTF("\r\n");
            }
        }
        else
        {
            PRINTF("error reading touch controller\r\n");
        }
    }
}
