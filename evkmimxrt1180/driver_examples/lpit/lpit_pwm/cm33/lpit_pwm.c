/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpit.h"

#include "fsl_xbar.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPIT_BASE    LPIT3
#define DEMO_LPIT_Channel kLPIT_Chnl_0

/* Get source clock for LPIT driver */
#define LPIT_SOURCECLOCK CLOCK_GetRootClockFreq(kCLOCK_Root_Lpit3)
#define LPIT_PWM_MIN_FREQUENCY (LPIT_SOURCECLOCK / 65535U)
#define LPIT_PWM_MAX_FREQUENCY (LPIT_SOURCECLOCK / 2U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_ConfigTriggerSource(void);
static void lpit_pwm_set(const uint32_t freq_Hz, uint32_t duty, const uint32_t polarity);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

void BOARD_ConfigTriggerSource()
{
    /* Init xbara module. */
    XBAR_Init(kXBAR_DSC1);

    /* Configure the XBARA signal connections. */
    XBAR_SetSignalsConnection(kXBAR1_InputPit3Trigger0, kXBAR1_OutputIomuxXbarInout20);
}
static void lpit_pwm_init(void)
{
    uint32_t pwm_freq = 0U;
    uint32_t duty     = 50U;

    /*channel timer configure structure*/
    lpit_chnl_params_t lpitChannelConfig = {
        .chainChannel          = false,
        .enableReloadOnTrigger = false,
        .enableStartOnTrigger  = false,
        .enableStopOnTimeout   = false,
        .timerMode             = kLPIT_DualPeriodicCounter,
        .triggerSelect         = kLPIT_Trigger_TimerChn0,
        .triggerSource         = kLPIT_TriggerSource_External,
    };

    /* Init lpit channel */
    LPIT_SetupChannel(DEMO_LPIT_BASE, DEMO_LPIT_Channel, &lpitChannelConfig);

    /*
     * Set the initial frequency and duty cycle.
     * Duty cycle set to 50%, PWM frequency is selected based on LPIT source clock.
     * Use frequency dividable by 1000.
     */
    pwm_freq = ((LPIT_PWM_MIN_FREQUENCY / 1000U) + 1U) * 1000U;

    PRINTF("Set default PWM output\r\n");
    PRINTF("Frequency: %dHz\r\n", pwm_freq);
    PRINTF("Duty Cycle: %d%%\r\n", duty);

    lpit_pwm_set(pwm_freq, duty, 1U);
}

static void lpit_pwm_set(const uint32_t freq_Hz, uint32_t duty, const uint32_t polarity)
{
    if (!((freq_Hz > LPIT_PWM_MIN_FREQUENCY) && (freq_Hz < LPIT_PWM_MAX_FREQUENCY)))
    {
        PRINTF("Invalid frequency\r\n");
        return;
    }

    /* Check duty */
    if ((duty >= 100U) || (duty == 0U))
    {
        /* LPIT hardware don't support 0% or 100% duty cycle*/
        duty = 50U;
        PRINTF("Invalid duty cycle, set to default value (50%) !\r\n");
    }

    /* Convert pwm_period_us to pwm_period_count */
    uint32_t pwm_period_count = LPIT_SOURCECLOCK / freq_Hz;

    /* Calculate lower_16bit_Value */
    uint16_t lower_16bit_value = pwm_period_count * duty / 100U;

    /* Calculate upper_16bit_Value */
    uint16_t upper_16bit_value = pwm_period_count - lower_16bit_value;

    /* Calculate TVAL register load value
       if polarity is set as 1,the pwm wave is high level active,default is low level active.
     */
    uint32_t load_value = 0U;

    load_value = (1U == polarity) ? (((uint32_t)lower_16bit_value << 16U) | upper_16bit_value) :
                                    (((uint32_t)upper_16bit_value << 16U) | lower_16bit_value);

    /* Set timer period for channel */
    LPIT_SetTimerValue(DEMO_LPIT_BASE, DEMO_LPIT_Channel, load_value);
}

/*!
 * @brief Main function
 */
int main(void)
{
    /* Set pwm frequency and duty */
    uint32_t duty      = 50U;
    uint32_t frequency = 1000U;

    /* configure structure */
    lpit_config_t lpitConfig;

    /* Board pin, clock, debug console init */
    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClockMux(kCLOCK_Root_Lpit3, kCLOCK_LPIT3_ClockRoot_MuxOscRc24M);

    PRINTF("\r\n lpit pwm demo start.\r\n");

    /* Config TriggerSource*/
    BOARD_ConfigTriggerSource();

    /* Init LPIT, use default configure
     * Disable in doze mode and Enable in debug mode
     */
    LPIT_GetDefaultConfig(&lpitConfig);
    LPIT_Init(DEMO_LPIT_BASE, &lpitConfig);

    /* Initialize LPIT PWM */
    lpit_pwm_init();

    /* Start lpit channel */
    LPIT_StartTimer(DEMO_LPIT_BASE, DEMO_LPIT_Channel);

    while (true)
    {
        PRINTF("\r\n Please input PWM frequency and duty like: %d %d\r\n", 2U * LPIT_PWM_MIN_FREQUENCY, 50);
        PRINTF("The valid PWM frequency is %dHz to %dHz\r\n", LPIT_PWM_MIN_FREQUENCY, LPIT_PWM_MAX_FREQUENCY);

        SCANF("%d %d", &frequency, &duty);

        PRINTF("Frequency: %dHz\r\n", frequency);
        PRINTF("Duty Cycle: %d%%\r\n", duty);

        lpit_pwm_set(frequency, duty, 1U);
    }
}
