/*
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_ftm.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The Flextimer instance/channel used for board */
#define BOARD_FTM_BASEADDR       FTM0
#define BOARD_FIRST_FTM_CHANNEL  6U
#define BOARD_SECOND_FTM_CHANNEL 7U

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)
#ifndef FTM_PWM_ON_LEVEL
#define FTM_PWM_ON_LEVEL kFTM_HighTrue
#endif
#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (24000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief delay a while.
 */
void delay(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
void delay(void)
{
    volatile uint32_t i = 0U;
    for (i = 0U; i < 800000U; ++i)
    {
        __asm("NOP"); /* delay */
    }
}

/*!
 * @brief Main function
 */
int main(void)
{
    bool brightnessUp = true; /* Indicate LEDs are brighter or dimmer */
    ftm_config_t ftmInfo;
    uint8_t updatedDutycycle = 0U;
    ftm_chnl_pwm_signal_param_t ftmParam[2];

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Print a note to terminal */
    PRINTF("\r\nFTM example to output PWM on 2 channels\r\n");
    PRINTF("\r\nYou will see a change in LED brightness if an LED is connected to the FTM pin");
    PRINTF("\r\nIf no LED is connected to the FTM pin, then probe the signal using an oscilloscope");

    /* Fill in the FTM config struct with the default settings */
    FTM_GetDefaultConfig(&ftmInfo);
    /* Calculate the clock division based on the PWM frequency to be obtained */
    ftmInfo.prescale = FTM_CalculateCounterClkDiv(BOARD_FTM_BASEADDR, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK);
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Configure ftm params with frequency 24kHZ */
    ftmParam[0].chnlNumber            = (ftm_chnl_t)BOARD_FIRST_FTM_CHANNEL;
    ftmParam[0].level                 = FTM_PWM_ON_LEVEL;
    ftmParam[0].dutyCyclePercent      = 0U;
    ftmParam[0].firstEdgeDelayPercent = 0U;
    ftmParam[0].enableComplementary   = false;
    ftmParam[0].enableDeadtime        = false;

    ftmParam[1].chnlNumber            = (ftm_chnl_t)BOARD_SECOND_FTM_CHANNEL;
    ftmParam[1].level                 = FTM_PWM_ON_LEVEL;
    ftmParam[1].dutyCyclePercent      = 0U;
    ftmParam[1].firstEdgeDelayPercent = 0U;
    ftmParam[1].enableComplementary   = false;
    ftmParam[1].enableDeadtime        = false;
    if (kStatus_Success !=
        FTM_SetupPwm(BOARD_FTM_BASEADDR, ftmParam, 2U, kFTM_EdgeAlignedPwm, DEMO_PWM_FREQUENCY, FTM_SOURCE_CLOCK))
    {
        PRINTF("\r\nSetup PWM fail, please check the configuration parameters!\r\n");
        return -1;
    }
    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);
    while (1)
    {
        /* Delay to see the change of LEDs brightness */
        delay();

        if (brightnessUp)
        {
            /* Increase duty cycle until it reach limited value */
            if (++updatedDutycycle == 100U)
            {
                brightnessUp = false;
            }
        }
        else
        {
            /* Decrease duty cycle until it reach limited value */
            if (--updatedDutycycle == 0U)
            {
                brightnessUp = true;
            }
        }
        /* Start PWM mode with updated duty cycle */
        if ((kStatus_Success != FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, (ftm_chnl_t)BOARD_FIRST_FTM_CHANNEL,
                                                       kFTM_EdgeAlignedPwm, updatedDutycycle)) ||
            (kStatus_Success != FTM_UpdatePwmDutycycle(BOARD_FTM_BASEADDR, (ftm_chnl_t)BOARD_SECOND_FTM_CHANNEL,
                                                       kFTM_EdgeAlignedPwm, updatedDutycycle)))
        {
            PRINTF("Update duty cycle fail, the target duty cycle may out of range!\r\n");
        }
        /* Software trigger to update registers */
        FTM_SetSoftwareTrigger(BOARD_FTM_BASEADDR, true);
    }
}
