/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include "slcd_engine.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_slcd.h"
#include "fsl_rnga.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define APP_SLCD_DUTY_CYCLE          kSLCD_1Div8DutyCycle
#define APP_SLCD_LOW_PIN_ENABLED     0xF7D86000U /* LCD_P31/P30/P29/P28/P26/P25/P24/P23/P22/P20/P19/P14/P13. */
#define APP_SLCD_HIGH_PIN_ENABLED    0x0F04387FU /* LCD_P59/P58/P57/P56/P50/P45/P44/P43/P38/P37/P36/P35/P34/P33/P32. */
#define APP_SLCD_BACK_PANEL_LOW_PIN  0x00586000U /* LCD_P22/20/19/14/13 --> b22/b20/b19/b14/b13 = 1. */
#define APP_SLCD_BACK_PANEL_HIGH_PIN 0x07000000U /* LCD_P58/57/56 --> b26/b25/b24 = 1. */
#define APP_SLCD_ALL_PHASE_ON                                                                    \
    (kSLCD_PhaseAActivate | kSLCD_PhaseBActivate | kSLCD_PhaseCActivate | kSLCD_PhaseDActivate | \
     kSLCD_PhaseEActivate | kSLCD_PhaseFActivate | kSLCD_PhaseGActivate | kSLCD_PhaseHActivate)

#define offLow32Pin(n)    ((uint32_t)1 << (n))      /* Pin offset for the low 32 pins. */
#define offHigh32Pin(n)   ((uint32_t)1 << (n - 32)) /* Pin offset for the high 32 pins. */
#define SLCD_INVALIDINPUT (-1)                      /* Invalid input. */
#define SLCD_OK           (0)                       /* Execute success. */

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_SetSlcdBackPlanePhase(void);

/*!
 * @brief SLCD time delay.
 * @param count The timedelay counter number.
 */
static void SLCD_TimeDelay(uint32_t count);

/*!
 * @brief SLCD set lcd pins.
 *
 * @param type lcd setting type @ref "lcd_set_type_t".
 * @param lcd_pin lcd pin.
 * @param pin_val pin setting value.
 * @param on The display on/off flag.
 */
static void SLCD_SetLCDPin(lcd_set_type_t type, uint32_t lcd_pin, uint8_t pin_val, int32_t on);

/*!
 * @brief SLCD Application Initialization.
 */
static void SLCD_APP_Init(void);

/*!
 * @brief SLCD Clear Screen.
 */
static void SLCD_Clear_Screen(void);

/*!
 * @brief SLCD basic test.
 * @param slcd_engine The SLCD engine structure pointer.
 */
static void SLCD_Basic_Test(tSLCD_Engine *slcd_engine);

/*!
 * @brief SLCD show victory.
 * @param slcd_engine The SLCD engine structure pointer.
 */
static void SLCD_Show_Victory(tSLCD_Engine *slcd_engine);

/*!
 * @brief SLCD show game start.
 * @param slcd_engine The SLCD engine structure pointer.
 */
static void SLCD_Show_Game_Start(tSLCD_Engine *slcd_engine);

/*!
 * @brief SLCD show number.
 * @param slcd_engine The SLCD engine structure pointer.
 * @param number The number to be shown.
 * @return The execution result.
 */
static int32_t SLCD_Show_Number(tSLCD_Engine *slcd_engine, int32_t number);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern slcd_clock_config_t slcdClkConfig;
extern const uint8_t slcd_lcd_gpio_seg_pin[];
slcd_clock_config_t slcdClkConfig = {kSLCD_DefaultClk, kSLCD_AltClkDivFactor1, kSLCD_ClkPrescaler01
#if FSL_FEATURE_SLCD_HAS_FAST_FRAME_RATE
                                     ,
                                     false
#endif
};

const uint8_t slcd_lcd_gpio_seg_pin[] = {38, 36, 34, 32, 31, 29, 25, 23, 43, 37,
                                         35, 33, 50, 30, 45, 24, 26, 28, 44, 59};

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_SetSlcdBackPlanePhase(void)
{
    SLCD_SetBackPlanePhase(LCD, 57, kSLCD_PhaseAActivate); /* SLCD_COM1 --- LCD_P57. */
    SLCD_SetBackPlanePhase(LCD, 19, kSLCD_PhaseBActivate); /* SLCD_COM2 --- LCD_P19. */
    SLCD_SetBackPlanePhase(LCD, 13, kSLCD_PhaseCActivate); /* SLCD_COM3 --- LCD_P13. */
    SLCD_SetBackPlanePhase(LCD, 58, kSLCD_PhaseDActivate); /* SLCD_COM4 --- LCD_P58. */
    SLCD_SetBackPlanePhase(LCD, 56, kSLCD_PhaseEActivate); /* SLCD_COM5 --- LCD_P56. */
    SLCD_SetBackPlanePhase(LCD, 22, kSLCD_PhaseFActivate); /* SLCD_COM6 --- LCD_P22. */
    SLCD_SetBackPlanePhase(LCD, 20, kSLCD_PhaseGActivate); /* SLCD_COM7 --- LCD_P20. */
    SLCD_SetBackPlanePhase(LCD, 14, kSLCD_PhaseHActivate); /* SLCD_COM8 --- LCD_P14. */
}


static void SLCD_TimeDelay(uint32_t count)
{
    while (count--)
    {
        __NOP();
    }
}

static void SLCD_SetLCDPin(lcd_set_type_t type, uint32_t lcd_pin, uint8_t pin_val, int32_t on)
{
    assert(lcd_pin > 0);

    uint8_t gpio_pin = 0;
    uint8_t bit_val  = 0;
    uint8_t i        = 0;

    /* lcd _pin starts from 1. */
    gpio_pin = slcd_lcd_gpio_seg_pin[lcd_pin - 1];

    if (type == SLCD_Set_Num)
    {
        SLCD_SetFrontPlaneSegments(LCD, gpio_pin, (on ? pin_val : 0));
    }
    else
    {
        for (i = 0; i < 8; ++i)
        {
            bit_val = (uint8_t)(pin_val >> i) & 0x1U;
            if (bit_val)
            {
                SLCD_SetFrontPlaneOnePhase(LCD, gpio_pin, (slcd_phase_index_t)i, on);
            }
        }
    }
}

static void SLCD_APP_Init(void)
{
    slcd_config_t config;

    /* Get Default configuration. */
    /*
     * config.displayMode = kSLCD_NormalMode;
     * config.powerSupply = kSLCD_InternalVll3UseChargePump;
     * config.voltageTrim = kSLCD_RegulatedVolatgeTrim00;
     * config.lowPowerBehavior = kSLCD_EnabledInWaitStop;
     * config.frameFreqIntEnable = false;
     * config.faultConfig = NULL;
     */
    SLCD_GetDefaultConfig(&config);

    /* Verify and Complete the configuration structure. */
    config.clkConfig          = &slcdClkConfig;
    config.loadAdjust         = kSLCD_HighLoadOrSlowestClkSrc;
    config.dutyCycle          = APP_SLCD_DUTY_CYCLE;
    config.slcdLowPinEnabled  = APP_SLCD_LOW_PIN_ENABLED;
    config.slcdHighPinEnabled = APP_SLCD_HIGH_PIN_ENABLED;
    config.backPlaneLowPin    = APP_SLCD_BACK_PANEL_LOW_PIN;
    config.backPlaneHighPin   = APP_SLCD_BACK_PANEL_HIGH_PIN;

    SLCD_Init(LCD, &config);
}

static void SLCD_Clear_Screen(void)
{
    uint8_t i;

    for (i = 0; i < SLCD_PIN_NUM; i++)
    {
        SLCD_SetFrontPlaneSegments(LCD, slcd_lcd_gpio_seg_pin[i], kSLCD_NoPhaseActivate);
    }
}

static void SLCD_Basic_Test(tSLCD_Engine *slcd_engine)
{
    uint32_t pinNum     = 0;
    uint32_t i          = 0;
    uint32_t j          = 0;
    uint32_t allPhaseOn = APP_SLCD_ALL_PHASE_ON;

    for (pinNum = 0; pinNum < FSL_FEATURE_SLCD_HAS_PIN_NUM; pinNum++)
    {
        SLCD_SetFrontPlaneSegments(LCD, pinNum, allPhaseOn);
    }

    BOARD_SetSlcdBackPlanePhase();

    PRINTF("---------- Start basic SLCD test -------------\r\n");

    SLCD_StartDisplay(LCD);

    SLCD_StartBlinkMode(LCD, kSLCD_AltDisplayBlink, kSLCD_BlinkRate03);

    SLCD_TimeDelay(0x7FFFFFU);

    SLCD_StopBlinkMode(LCD);

    SLCD_Clear_Screen();

    for (i = 0; i < NUM_POSEND; ++i)
    {
        for (j = 0; j < 10; ++j)
        {
            SLCD_Engine_Show_Num(slcd_engine, j, i, 1);
            SLCD_TimeDelay(0x1FFFFFU);
            SLCD_Engine_Show_Num(slcd_engine, j, i, 0);
        }
    }

    for (i = 0; i < ICON_END; ++i)
    {
        SLCD_Engine_Show_Icon(slcd_engine, i, 1);
        SLCD_TimeDelay(0x1FFFFFU);
        SLCD_Engine_Show_Icon(slcd_engine, i, 0);
    }

    SLCD_TimeDelay(0xFFFFFU);
}

static void SLCD_Show_Victory(tSLCD_Engine *slcd_engine)
{
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S3, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S6, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S7, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S9, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S11, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S12, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_S13, 1);
}

static void SLCD_Show_Game_Start(tSLCD_Engine *slcd_engine)
{
    SLCD_Engine_Show_Num(slcd_engine, 1, NUM_POS6, 0);
    SLCD_Engine_Show_Num(slcd_engine, 1, NUM_POS6, 1);
    SLCD_TimeDelay(0x3FFFFFU);
    SLCD_Engine_Show_Num(slcd_engine, 2, NUM_POS6, 0);
    SLCD_Engine_Show_Num(slcd_engine, 2, NUM_POS6, 1);
    SLCD_TimeDelay(0x3FFFFFU);
    SLCD_Engine_Show_Num(slcd_engine, 3, NUM_POS6, 0);
    SLCD_Engine_Show_Num(slcd_engine, 3, NUM_POS6, 1);
    SLCD_TimeDelay(0x3FFFFFU);
    SLCD_Engine_Show_Num(slcd_engine, 3, NUM_POS6, 0);
    /* Show Let */
    /* Clear number */
    /* Show G at num pos 8 */
    SLCD_Engine_Show_Num(slcd_engine, 0, NUM_POS5, 0);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_5A, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_5C, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_5D, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_5E, 1);
    SLCD_Engine_Show_Icon(slcd_engine, ICON_5F, 1);
    /* Show O at num pos 9. */
    SLCD_Engine_Show_Num(slcd_engine, 0, NUM_POS6, 0);
    SLCD_Engine_Show_Num(slcd_engine, 0, NUM_POS6, 1);
    SLCD_TimeDelay(0xFFFFFU);
}

static int32_t SLCD_Show_Number(tSLCD_Engine *slcd_engine, int32_t number)
{
    if ((number < 0) || (number > 9999))
    {
        return SLCD_INVALIDINPUT;
    }

    uint32_t thousand_val = 0;
    uint32_t hundred_val  = 0;
    uint32_t decimal_val  = 0;
    uint32_t single_val   = 0;

    SLCD_Engine_Show_Num(slcd_engine, 8, NUM_POS3, 0);
    SLCD_Engine_Show_Num(slcd_engine, 8, NUM_POS4, 0);
    SLCD_Engine_Show_Num(slcd_engine, 8, NUM_POS5, 0);
    SLCD_Engine_Show_Num(slcd_engine, 8, NUM_POS6, 0);

    thousand_val = number / 1000;
    hundred_val  = (number % 1000) / 100;
    decimal_val  = (number % 100) / 10;
    single_val   = (number % 10);

    if (number >= 1000)
    {
        SLCD_Engine_Show_Num(slcd_engine, thousand_val, NUM_POS3, 0);
        SLCD_Engine_Show_Num(slcd_engine, thousand_val, NUM_POS3, 1);
    }

    if (number >= 100)
    {
        SLCD_Engine_Show_Num(slcd_engine, hundred_val, NUM_POS4, 0);
        SLCD_Engine_Show_Num(slcd_engine, hundred_val, NUM_POS4, 1);
    }

    if (number >= 10)
    {
        SLCD_Engine_Show_Num(slcd_engine, decimal_val, NUM_POS5, 0);
        SLCD_Engine_Show_Num(slcd_engine, decimal_val, NUM_POS5, 1);
    }

    SLCD_Engine_Show_Num(slcd_engine, single_val, NUM_POS6, 0);
    SLCD_Engine_Show_Num(slcd_engine, single_val, NUM_POS6, 1);

    return SLCD_OK;
}

static void SLCD_Guess_Num(tSLCD_Engine *slcd_engine)
{
    int32_t input_num  = 0;
    int32_t target_num = 0;

    while (1)
    {
        int32_t play_again = 0;
        uint32_t randout   = 0;

        PRINTF("-------------- SLCD Guess Num Game --------------\r\n");
        PRINTF("The number input and final number will be shown on SLCD.\r\n");
        PRINTF("Please check SLCD for these numbers.\r\n");
        PRINTF("Let's play:\r\n");

        RNGA_GetRandomData(RNG, &randout, sizeof(uint32_t));

        target_num = randout % 10000;

        SLCD_Clear_Screen();

        SLCD_Show_Game_Start(slcd_engine);

        while (1)
        {
            uint8_t input_num_size = 4;
            char ch_in             = 0;
            PRINTF("Please guess the number I want(0 - 9999), Press 'enter' to end: ");
            input_num = 0;
            /* Check for enter key */
            while (1)
            {
                if (('\r' == ch_in) && (input_num_size < 4))
                {
                    break;
                }

                ch_in = GETCHAR();
                if ((ch_in >= '0') && (ch_in <= '9') && (input_num_size > 0))
                {
                    PUTCHAR(ch_in);
                    input_num = (input_num * 10) + (ch_in - '0');
                    input_num_size--;
                }
            }
            PRINTF("\r\n");

            if (SLCD_INVALIDINPUT == SLCD_Show_Number(slcd_engine, input_num))
            {
                PRINTF("Input number out of range! Should be in the range of 0-9999.\r\n");
            }
            else
            {
                if (input_num == target_num)
                {
                    ch_in     = 0;
                    bool loop = true;

                    SLCD_Show_Victory(slcd_engine);
                    PRINTF("Great, %d, you have GOT it!\r\n", input_num);
                    while (loop)
                    {
                        PRINTF("Play again? Input 'Y' or 'N':\r\n");
                        ch_in = GETCHAR();
                        PUTCHAR(ch_in);
                        PRINTF("\r\n");

                        if ((ch_in == 'Y') || (ch_in == 'y'))
                        {
                            play_again = 1;
                            loop       = false;
                        }
                        else if ((ch_in == 'N') || (ch_in == 'n'))
                        {
                            play_again = 0;
                            loop       = false;
                        }
                        else
                        {
                            PRINTF("Wrong input!Please input again.\r\n");
                            continue;
                        }
                    }
                    break;
                }
                else
                {
                    PRINTF("The input number %d is %s than what I want. Guess again!\r\n", input_num,
                           (input_num > target_num) ? "bigger" : "smaller");
                }
            }
        }

        if (play_again == 0)
        {
            SLCD_Clear_Screen();
            PRINTF("Bye bye!\r\n");
            break;
        }
    }
}

/*!
 * @brief main function
 */
int main(void)
{
    tSLCD_Engine slcdEngine;

    /* Init hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* RNGA clock bug fix. */
    CLOCK_EnableClock(kCLOCK_Rnga0);
    CLOCK_DisableClock(kCLOCK_Rnga0);
    CLOCK_EnableClock(kCLOCK_Rnga0);

    /* RNGA Initialization. */
    RNGA_Init(RNG);

    /* SLCD Initialization. */
    SLCD_APP_Init();

    memset(&slcdEngine, 0, sizeof(tSLCD_Engine));
    SLCD_Engine_Init(&slcdEngine, SLCD_SetLCDPin);

    SLCD_Basic_Test(&slcdEngine);

    SLCD_Guess_Num(&slcdEngine);

    while (1)
    {
    }
}
