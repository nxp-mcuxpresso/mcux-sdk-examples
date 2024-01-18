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
#include "fsl_reset.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* define instance */
#define BOARD_TPM_BASEADDR       TPM0
#define BOARD_FIRST_TPM_CHANNEL  kTPM_Chnl_0
#define BOARD_SECOND_TPM_CHANNEL kTPM_Chnl_3

/* Get source clock for TPM driver */
#define TPM_SOURCE_CLOCK (CLOCK_GetTpmClkFreq(0))
#ifndef TPM_LED_ON_LEVEL
#define TPM_LED_ON_LEVEL kTPM_HighTrue
#endif
#ifndef DEMO_PWM_FREQUENCY
#define DEMO_PWM_FREQUENCY (24000U)
#endif
/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
volatile uint8_t getCharValue     = 0U;
volatile uint8_t updatedDutycycle = 10U;

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    tpm_config_t tpmInfo;
    tpm_chnl_pwm_signal_param_t tpmParam[2];
    uint8_t control;

    /* Board pin, clock, debug console init */
    BOARD_InitBootPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    CLOCK_SetIpSrc(kCLOCK_Tpm0, kCLOCK_Pcc1BusIpSrcCm33Bus);
    RESET_PeripheralReset(kRESET_Tpm0);

    if (BOARD_IsLowPowerBootType() != true) /* not low power boot type */
    {
        BOARD_HandshakeWithUboot(); /* Must handshake with uboot, unless will get issues(such as: SoC reset all the
                                       time) */
    }
    else                            /* low power boot type */
    {
        BOARD_SetTrdcGlobalConfig();
    }

    /* Print a note to terminal */
    PRINTF("\r\nTPM example to output PWM on 2 channels\r\n");
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
    tpmParam[0].chnlNumber = (tpm_chnl_t)BOARD_FIRST_TPM_CHANNEL;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam[0].pauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam[0].level            = TPM_LED_ON_LEVEL;
    tpmParam[0].dutyCyclePercent = updatedDutycycle;

    tpmParam[1].chnlNumber = (tpm_chnl_t)BOARD_SECOND_TPM_CHANNEL;
#if (defined(FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT) && FSL_FEATURE_TPM_HAS_PAUSE_LEVEL_SELECT)
    tpmParam[1].pauseLevel = kTPM_ClearOnPause;
#endif
    tpmParam[1].level            = TPM_LED_ON_LEVEL;
    tpmParam[1].dutyCyclePercent = updatedDutycycle;
    if (kStatus_Success !=
        TPM_SetupPwm(BOARD_TPM_BASEADDR, tpmParam, 2U, kTPM_EdgeAlignedPwm, DEMO_PWM_FREQUENCY, TPM_SOURCE_CLOCK))
    {
        PRINTF("\r\nSetup PWM fail!\r\n");
        return -1;
    }

    TPM_StartTimer(BOARD_TPM_BASEADDR, kTPM_SystemClock);
    /* Record channel PWM mode configure */
    control = TPM_GetChannelContorlBits(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_FIRST_TPM_CHANNEL);
    while (1)
    {
        do
        {
            PRINTF("\r\nPlease enter a value to update the Duty cycle:\r\n");
            PRINTF("Note: The range of value is 0 to 9.\r\n");
            PRINTF("For example: If enter '5', the duty cycle will be set to 50 percent.\r\n");
            PRINTF("Value:");
            getCharValue = GETCHAR() - 0x30U;
            PRINTF("%d\r\n", getCharValue);
        } while (getCharValue > 9U);

        updatedDutycycle = getCharValue * 10U;

        /* Disable output on each channel of the pair before updating the dutycycle */
        TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_FIRST_TPM_CHANNEL);
        TPM_DisableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_SECOND_TPM_CHANNEL);

        /* Update PWM duty cycle */
        if ((kStatus_Success == TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_FIRST_TPM_CHANNEL,
                                                       kTPM_EdgeAlignedPwm, updatedDutycycle)) &&
            (kStatus_Success == TPM_UpdatePwmDutycycle(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_SECOND_TPM_CHANNEL,
                                                       kTPM_EdgeAlignedPwm, updatedDutycycle)))
        {
            PRINTF("The duty cycle was successfully updated!\r\n");
        }

        /* Start output on each channel of the pair with updated dutycycle */
        TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_FIRST_TPM_CHANNEL, control);
        TPM_EnableChannel(BOARD_TPM_BASEADDR, (tpm_chnl_t)BOARD_SECOND_TPM_CHANNEL, control);
    }
}
