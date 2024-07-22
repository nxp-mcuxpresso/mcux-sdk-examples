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
#include "fsl_tpm.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* define instance */
#define BOARD_TPM_BASEADDR TPM2
/* TPM channel pair 0 works with TPM channels 0 and 1*/
#define BOARD_TPM_CHANNEL_PAIR kTPM_Chnl_0

/* Interrupt to enable and flag to read; depends on the TPM channel pair used */
#define TPM_CHANNEL_INTERRUPT_ENABLE kTPM_Chnl0InterruptEnable
#define TPM_CHANNEL_FLAG             kTPM_Chnl0Flag

/* Interrupt number and interrupt handler for the TPM instance used */
#define BOARD_TPM_IRQ_NUM TPM2_IRQn
#define BOARD_TPM_HANDLER TPM2_IRQHandler
/* Get source clock for TPM driver */
#define LPTPM_CLOCK_ROOT kCLOCK_Root_Tpm2
#define LPTPM_CLOCK_GATE kCLOCK_Tpm2
#define TPM_SOURCE_CLOCK CLOCK_GetIpFreq(LPTPM_CLOCK_ROOT)
#ifndef TPM_LED_ON_LEVEL
#define TPM_LED_ON_LEVEL kTPM_HighTrue
#endif
#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (24000U)
#endif
#ifndef DEMO_PWM_DEADTIME
/*The unit of dead time is 4 times the TPM clock cycle */
#define DEMO_PWM_DEADTIME (1U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile bool brightnessUp        = true; /* Indicate LED is brighter or dimmer */
volatile uint8_t updatedDutycycle = 10U;
volatile uint8_t getCharValue     = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    tpm_config_t tpmInfo;
    tpm_chnl_pwm_signal_param_t tpmParam;
    uint8_t control[2];

    /* Board pin, clock, debug console init */
    /* clang-format off */

    const clock_root_config_t lptpmClkCfg = {
        .clockOff = false,
	.mux = 0,
	.div = 1
    };
    /* clang-format on */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetRootClock(LPTPM_CLOCK_ROOT, &lptpmClkCfg);
    CLOCK_EnableClock(LPTPM_CLOCK_GATE);

    /* Print a note to terminal */
    PRINTF("\r\nTPM example to output combined complementary PWM signals on two channels\r\n");
    PRINTF(
        "\r\nIf an LED is connected to the TPM pin, you will see a change in LED brightness if you enter different "
        "values");
    PRINTF("\r\nIf no LED is connected to the TPM pin, then probe the signal using an oscilloscope");

    /* Fill in the TPM config struct with the default settings */
    TPM_GetDefaultConfig(&tpmInfo);

    /* Calculate the clock division based on the PWM frequency to be obtained */
    tpmInfo.prescale = TPM_CalculateCounterClkDiv(BOARD_TPM_BASEADDR, DEMO_PWM_FREQUENCY, TPM_SOURCE_CLOCK);
    /* Initialize TPM module */
    TPM_Init(BOARD_TPM_BASEADDR, &tpmInfo);

    /* Configure tpm params with frequency 24kHZ */
    tpmParam.chnlNumber = BOARD_TPM_CHANNEL_PAIR;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam.pauseLevel    = kTPM_ClearOnPause;
    tpmParam.secPauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam.level               = TPM_LED_ON_LEVEL;
    tpmParam.dutyCyclePercent    = updatedDutycycle;
    tpmParam.enableComplementary = true;
    /* Note: If setting deadtime insertion, the value of firstEdgeDelayPercent must be non-zero */
    tpmParam.firstEdgeDelayPercent = 10U;
    /* Insert deadtime into each pair of channels */
    tpmParam.deadTimeValue[0] = DEMO_PWM_DEADTIME;
    tpmParam.deadTimeValue[1] = DEMO_PWM_DEADTIME;

    /* Configure PWM with frequency 24kHZ */
    if (kStatus_Success !=
        TPM_SetupPwm(BOARD_TPM_BASEADDR, &tpmParam, 1U, kTPM_CombinedPwm, DEMO_PWM_FREQUENCY, TPM_SOURCE_CLOCK))
    {
        PRINTF("\r\nSetup combined PWM fail, please check the configuration parameters!\r\n");
        return -1;
    }

    TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);

    /* Record channel PWM mode configure */
    control[0] = TPM_GetChannelContorlBits(BOARD_TPM_BASEADDR, (tpm_chnl_t)(BOARD_TPM_CHANNEL_PAIR * 2));
    control[1] = TPM_GetChannelContorlBits(BOARD_TPM_BASEADDR, (tpm_chnl_t)((BOARD_TPM_CHANNEL_PAIR * 2) + 1));
    while (1)
    {
        do
        {
            PRINTF("\r\nPlease enter a value to update the Duty cycle:\r\n");
            PRINTF("Note: The range of value is 1 to 9.\r\n");
            PRINTF("For example: If enter '5', the duty cycle will be set to 50 percent.\r\n");
            PRINTF("Value:");
            getCharValue = GETCHAR() - 0x30U;
            PRINTF("%d", getCharValue);
            PRINTF("\r\n");
        } while ((getCharValue > 9U) || (getCharValue == 0U));

        updatedDutycycle = getCharValue * 10U;

        /* Disable output on each channel of the pair before updating the dutycycle */
        TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)(BOARD_TPM_CHANNEL_PAIR * 2));
        TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)((BOARD_TPM_CHANNEL_PAIR * 2) + 1));

        /* Update PWM duty cycle on the channel pair */
        /* Note: If setting deadtime insertion, the value of Dutycycle must be non-zero */
        if (kStatus_Success !=
            TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, BOARD_TPM_CHANNEL_PAIR, kTPM_CombinedPwm, updatedDutycycle))
        {
            PRINTF("Update duty cycle fail, the target duty cycle may out of range!\r\n");
        }
        else
        {
            PRINTF("The duty cycle was successfully updated!\r\n");
        }

        /* Start output on each channel of the pair with updated dutycycle */
        TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)(BOARD_TPM_CHANNEL_PAIR * 2), control[0]);
        TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)((BOARD_TPM_CHANNEL_PAIR * 2) + 1), control[1]);
    }
}
