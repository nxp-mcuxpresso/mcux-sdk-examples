/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "fsl_flexio.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_TIME_DELAY_FOR_DUTY_CYCLE_UPDATE (2000000U)
#define DEMO_FLEXIO_BASEADDR                  FLEXIO2
#define DEMO_FLEXIO_OUTPUTPIN                 (5U) /* Select FXIO2_D5 as PWM output */
#define DEMO_FLEXIO_TIMER_CH                  (0U) /* Flexio timer0 used */

/* Select USB1 PLL (480 MHz) as flexio clock source */
#define FLEXIO_CLOCK_SELECT (3U)
/* Clock pre divider for flexio clock source */
#define FLEXIO_CLOCK_PRE_DIVIDER (4U)
/* Clock divider for flexio clock source */
#define FLEXIO_CLOCK_DIVIDER (7U)
#define DEMO_FLEXIO_CLOCK_FREQUENCY \
    (CLOCK_GetFreq(kCLOCK_Usb1PllClk) / (FLEXIO_CLOCK_PRE_DIVIDER + 1U) / (FLEXIO_CLOCK_DIVIDER + 1U))
/* FLEXIO output PWM frequency */
#define DEMO_FLEXIO_FREQUENCY (48000U)
#define FLEXIO_MAX_FREQUENCY (DEMO_FLEXIO_CLOCK_FREQUENCY / 2U)
#define FLEXIO_MIN_FREQUENCY (DEMO_FLEXIO_CLOCK_FREQUENCY / 512U)

/* flexio timer number */
#define FLEXIO_TIMER_CHANNELS (8)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief Configures the timer as a 8-bits PWM mode to generate the PWM waveform
 *
 * @param freq_Hz PWM frequency in hertz, range is [FLEXIO_MIN_FREQUENCY, FLEXIO_MAX_FREQUENCY]
 * @param duty Specified duty in unit of %, with a range of [0, 100]
 */
static status_t flexio_pwm_init(uint32_t freq_Hz, uint32_t duty);

/*!
 * @brief Set PWM output in idle status (high or low).
 *
 * @param base               FlexIO peripheral base address
 * @param timerChannel       FlexIO timer channel
 * @param idleStatus         True: PWM output is high in idle status; false: PWM output is low in idle status
 */
static void FLEXIO_SetPwmOutputToIdle(FLEXIO_Type *base, uint8_t timerChannel, bool idleStatus);

#if defined(FSL_FEATURE_FLEXIO_HAS_PIN_STATUS) && FSL_FEATURE_FLEXIO_HAS_PIN_STATUS
/*!
 * @brief Get the pwm dutycycle value.
 *
 * @param base        FlexIO peripheral base address
 * @param timerChannel  FlexIO timer channel
 * @param channel     FlexIO as pwm output channel number
 *
 * @return Current channel dutycycle value.
 */
static uint8_t PWM_GetPwmOutputState(FLEXIO_Type *base, uint8_t timerChannel, uint8_t channel);
#endif

/*!
 * @brief Get pwm duty cycle value.
 */
static uint8_t s_flexioGetPwmDutyCycle[FLEXIO_TIMER_CHANNELS] = {0};

/*******************************************************************************
 * Variables
 *******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static status_t flexio_pwm_init(uint32_t freq_Hz, uint32_t duty)
{
    assert((freq_Hz < FLEXIO_MAX_FREQUENCY) && (freq_Hz > FLEXIO_MIN_FREQUENCY));

    uint32_t lowerValue = 0; /* Number of clock cycles in high logic state in one period */
    uint32_t upperValue = 0; /* Number of clock cycles in low logic state in one period */
    uint32_t sum        = 0; /* Number of clock cycles in one period */
    flexio_timer_config_t fxioTimerConfig;

    /* Configure the timer DEMO_FLEXIO_TIMER_CH for generating PWM */
    fxioTimerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_SHIFTnSTAT(0U);
    fxioTimerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    fxioTimerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveLow;
    fxioTimerConfig.pinConfig       = kFLEXIO_PinConfigOutput;
    fxioTimerConfig.pinPolarity     = kFLEXIO_PinActiveHigh;
    fxioTimerConfig.pinSelect       = DEMO_FLEXIO_OUTPUTPIN; /* Set pwm output */
    fxioTimerConfig.timerMode       = kFLEXIO_TimerModeDisabled;
    fxioTimerConfig.timerOutput     = kFLEXIO_TimerOutputOneNotAffectedByReset;
    fxioTimerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnFlexIOClockShiftTimerOutput;
    fxioTimerConfig.timerDisable    = kFLEXIO_TimerDisableNever;
    fxioTimerConfig.timerEnable     = kFLEXIO_TimerEnabledAlways;
    fxioTimerConfig.timerReset      = kFLEXIO_TimerResetNever;
    fxioTimerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;
    fxioTimerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;

    /* Calculate timer lower and upper values of TIMCMP */
    /* Calculate the nearest integer value for sum, using formula round(x) = (2 * floor(x) + 1) / 2 */
    /* sum = DEMO_FLEXIO_CLOCK_FREQUENCY / freq_H */
    sum = (DEMO_FLEXIO_CLOCK_FREQUENCY * 2 / freq_Hz + 1) / 2;

    /* Calculate the nearest integer value for lowerValue, the high period of the pwm output */
    lowerValue = (sum * duty) / 100;
    /* Calculate upper value, the low period of the pwm output */
    upperValue = sum - lowerValue - 2;

    fxioTimerConfig.timerCompare = ((upperValue << 8U) | (lowerValue));

    if ((duty > 0) && (duty < 100))
    {
        /* Set Timer mode to kFLEXIO_TimerModeDual8BitPWM to start timer */
        fxioTimerConfig.timerMode = kFLEXIO_TimerModeDual8BitPWM;
    }
    else if (duty == 100)
    {
        fxioTimerConfig.pinPolarity = kFLEXIO_PinActiveLow;
    }
    else if (duty == 0)
    {
        /* Set high level as active level */
        fxioTimerConfig.pinPolarity = kFLEXIO_PinActiveHigh;
    }
    else
    {
        return kStatus_Fail;
    }

    FLEXIO_SetTimerConfig(DEMO_FLEXIO_BASEADDR, DEMO_FLEXIO_TIMER_CH, &fxioTimerConfig);

    s_flexioGetPwmDutyCycle[DEMO_FLEXIO_TIMER_CH] = duty;

    return kStatus_Success;
}

/*!
 * brief Set PWM output in idle status (high or low).
 *
 * param base               FlexIO peripheral base address
 * param timerChannel       FlexIO timer channel
 * param idleStatus         True: PWM output is high in idle status; false: PWM output is low in idle status
 */
static void FLEXIO_SetPwmOutputToIdle(FLEXIO_Type *base, uint8_t timerChannel, bool idleStatus)
{
    flexio_timer_config_t fxioTimerConfig;

    /* Configure the timer DEMO_FLEXIO_TIMER_CH for generating PWM */
    fxioTimerConfig.triggerSelect   = FLEXIO_TIMER_TRIGGER_SEL_SHIFTnSTAT(0U);
    fxioTimerConfig.triggerSource   = kFLEXIO_TimerTriggerSourceInternal;
    fxioTimerConfig.triggerPolarity = kFLEXIO_TimerTriggerPolarityActiveLow;
    fxioTimerConfig.pinConfig       = kFLEXIO_PinConfigOutput;
    fxioTimerConfig.pinSelect       = DEMO_FLEXIO_OUTPUTPIN; /* Set pwm output */
    fxioTimerConfig.timerMode       = kFLEXIO_TimerModeDisabled;
    fxioTimerConfig.timerOutput     = kFLEXIO_TimerOutputOneNotAffectedByReset;
    fxioTimerConfig.timerDecrement  = kFLEXIO_TimerDecSrcOnFlexIOClockShiftTimerOutput;
    fxioTimerConfig.timerDisable    = kFLEXIO_TimerDisableNever;
    fxioTimerConfig.timerEnable     = kFLEXIO_TimerEnabledAlways;
    fxioTimerConfig.timerReset      = kFLEXIO_TimerResetNever;
    fxioTimerConfig.timerStart      = kFLEXIO_TimerStartBitDisabled;
    fxioTimerConfig.timerStop       = kFLEXIO_TimerStopBitDisabled;
    fxioTimerConfig.timerCompare    = 0U;

    /* Clear TIMCMP register */
    base->TIMCMP[timerChannel] = 0;

    if (idleStatus)
    {
        /* Set low level as active level */
        fxioTimerConfig.pinPolarity = kFLEXIO_PinActiveLow;
    }
    else
    {
        /* Set high level as active level */
        fxioTimerConfig.pinPolarity = kFLEXIO_PinActiveHigh;
    }

    FLEXIO_SetTimerConfig(DEMO_FLEXIO_BASEADDR, timerChannel, &fxioTimerConfig);

    s_flexioGetPwmDutyCycle[timerChannel] = 0;
}

#if defined(FSL_FEATURE_FLEXIO_HAS_PIN_STATUS) && FSL_FEATURE_FLEXIO_HAS_PIN_STATUS
/*!
 * brief Get the pwm dutycycle value.
 *
 * param base          FlexIO peripheral base address
 * param timerChannel  FlexIO timer channel
 * param channel       FlexIO as pwm output channel number
 *
 * return Current channel dutycycle value.
 */
static uint8_t PWM_GetPwmOutputState(FLEXIO_Type *base, uint8_t timerChannel, uint8_t channel)
{
    if ((base->PIN & (1U << channel)) ^ (base->TIMCTL[timerChannel] & FLEXIO_TIMCTL_PINPOL_MASK))
    {
        return kFLEXIO_PwmHigh;
    }
    else
    {
        return kFLEXIO_PwmLow;
    }
}
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    uint8_t duty            = 0;
    uint8_t idleState       = 0;
    uint32_t dutyCycleValue = 0;
    uint32_t idleStateValue = 0;
    flexio_config_t fxioUserConfig;

    /* Init board hardware */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Clock setting for Flexio */
    CLOCK_SetMux(kCLOCK_Flexio2Mux, FLEXIO_CLOCK_SELECT);
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, FLEXIO_CLOCK_PRE_DIVIDER);
    CLOCK_SetDiv(kCLOCK_Flexio2Div, FLEXIO_CLOCK_DIVIDER);

    /* Init flexio, use default configure
     * Disable doze and fast access mode
     * Enable in debug mode
     */
    FLEXIO_GetDefaultConfig(&fxioUserConfig);
    FLEXIO_Init(DEMO_FLEXIO_BASEADDR, &fxioUserConfig);

    PRINTF("\r\nFLEXIO_PWM demo start.\r\n");

    while (1)
    {
        duty           = 0;
        dutyCycleValue = 0;
        idleState      = 0;
        idleStateValue = 0;

        PRINTF("\r\nPlease input a value (0 - 100) to set duty cycle: ");
        while (duty != 0x0D)
        {
            duty = GETCHAR();
            if ((duty >= '0') && (duty <= '9'))
            {
                PUTCHAR(duty);
                dutyCycleValue = dutyCycleValue * 10 + (duty - 0x30U);
            }
        }
        PRINTF("\r\nInput value is %d\r\n", dutyCycleValue);

        if (dutyCycleValue > 0x64U)
        {
            PRINTF("Your value is output of range.\r\n");
            PRINTF("Set pwm output to IDLE.\r\n");

            PRINTF("\r\nPlease input pwm idle status (0 or 1): ");
            while (idleState != 0x0D)
            {
                idleState = GETCHAR();
                if ((idleState >= '0') && (idleState <= '9'))
                {
                    PUTCHAR(idleState);
                    idleStateValue = idleStateValue * 10 + (idleState - 0x30U);
                }
            }

            PRINTF("\r\nInput IDLE state value is %d\r\n", idleStateValue);

            if (idleStateValue > 0x1U)
            {
                PRINTF("\r\nYour value is output of range.\r\n");

                continue;
            }

            FLEXIO_SetPwmOutputToIdle(DEMO_FLEXIO_BASEADDR, DEMO_FLEXIO_TIMER_CH, idleStateValue);
#if defined(FSL_FEATURE_FLEXIO_HAS_PIN_STATUS) && FSL_FEATURE_FLEXIO_HAS_PIN_STATUS
            PRINTF("\r\nPWM leave is: %d \r\n",
                   PWM_GetPwmOutputState(DEMO_FLEXIO_BASEADDR, DEMO_FLEXIO_TIMER_CH, DEMO_FLEXIO_OUTPUTPIN));
#endif
        }
        else
        {
            if (flexio_pwm_init(DEMO_FLEXIO_FREQUENCY, dutyCycleValue) == kStatus_Fail)
            {
                PRINTF("FLEXIO PWM initialization failed\n");
                return -1;
            }

            PRINTF("\r\nPWM duty cycle is: %d\r\n", s_flexioGetPwmDutyCycle[DEMO_FLEXIO_TIMER_CH]);
#if defined(FSL_FEATURE_FLEXIO_HAS_PIN_STATUS) && FSL_FEATURE_FLEXIO_HAS_PIN_STATUS
            PRINTF("\r\nPWM leave is: %d \r\n",
                   PWM_GetPwmOutputState(DEMO_FLEXIO_BASEADDR, DEMO_FLEXIO_TIMER_CH, DEMO_FLEXIO_OUTPUTPIN));
#endif
        }
    }
}
