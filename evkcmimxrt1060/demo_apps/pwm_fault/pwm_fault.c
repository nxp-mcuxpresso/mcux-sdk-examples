/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "board.h"
#include "fsl_pwm.h"
#include "fsl_cmp.h"

#include "fsl_xbara.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* The PWM base address */
#define DEMO_PWM_BASEADDR                  PWM1
#define DEMO_PWM_SUBMODULE                 kPWM_Module_0
#define DEMO_PWM_FAULT_INPUT_PIN           kPWM_Fault_0
#define DEMO_PWM_CONTROL_SUBMODULE         kPWM_Control_Module_0
#define DEMO_PWM_CHANNEL                   kPWM_PwmA
#define DEMO_PWM_DELAY_VAL                 0x0FFFU
#define DEMO_PWM_CHANNEL_LOCATION_ON_BOARD "J24-6"

#define DEMO_CMP_BASE                        CMP1
#define DEMO_CMP_USER_CHANNEL                0U
#define DEMO_CMP_DAC_CHANNEL                 7U
#define DEMO_CMP_INPUT_PIN_LOCATION_ON_BOARD "J23-5"

#define DEMO_DEADTIME_VAL 650U
#define PWM_SRC_CLK_FREQ  CLOCK_GetFreq(kCLOCK_IpgClk)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/* Sets up the PWM signals for a PWM submodule */
static void PWM_InitPhasePwm(void);

/* Sets up the PWM fault protection */
static void PWM_SetupFaultPwm(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void PWM_InitPhasePwm(void)
{
    /* Structure of setup PWM */
    pwm_signal_param_t pwmSignal;
    uint16_t deadTimeVal;
    uint32_t pwmSourceClockInHz, pwmFrequencyInHz = 1U;

    pwmSourceClockInHz = PWM_SRC_CLK_FREQ;
    /* Set deadtime count */
    deadTimeVal                = ((uint64_t)pwmSourceClockInHz * DEMO_DEADTIME_VAL) / 1000000000U;
    pwmSignal.pwmChannel       = DEMO_PWM_CHANNEL;
    pwmSignal.level            = kPWM_HighTrue;
    pwmSignal.dutyCyclePercent = 50U; /* 50 percent dutycycle */
    pwmSignal.deadtimeValue    = deadTimeVal;
    pwmSignal.faultState       = kPWM_PwmFaultState0;
    pwmSignal.pwmchannelenable = true;

    PWM_SetupPwm(DEMO_PWM_BASEADDR, DEMO_PWM_SUBMODULE, &pwmSignal, 1U, kPWM_SignedCenterAligned, pwmFrequencyInHz,
                 pwmSourceClockInHz);

    /* Set the load okay bit for all submodules to load registers from their buffer */
    PWM_SetPwmLdok(DEMO_PWM_BASEADDR, DEMO_PWM_CONTROL_SUBMODULE, true);
}

static void PWM_SetupFaultPwm(void)
{
    /* Structure of the parameters to configure a PWM fault  */
    pwm_fault_param_t pwmFaultParam;

    /* Setup Fault config */
    /* No combination path is available */
    pwmFaultParam.enableCombinationalPath = false;

    /* Logic 1 on the fault input pin indicates fault */
    pwmFaultParam.faultLevel = true;
    /*
     * Automatic fault clearing
     * If use Manual fault clearing mode, then the user must clear fault flags
     */
    pwmFaultParam.faultClearingMode = kPWM_Automatic;
    pwmFaultParam.recoverMode       = kPWM_RecoverFullCycle;

    PWM_SetupFaults(DEMO_PWM_BASEADDR, DEMO_PWM_FAULT_INPUT_PIN, &pwmFaultParam);

    DEMO_PWM_BASEADDR->SM[DEMO_PWM_SUBMODULE].DISMAP[DEMO_PWM_SUBMODULE] = 0x00;
    DEMO_PWM_BASEADDR->SM[DEMO_PWM_SUBMODULE].DISMAP[DEMO_PWM_SUBMODULE] |= PWM_DISMAP_DIS0A(1);
}

int main(void)
{
    /* Structure of initialize PWM */
    pwm_config_t pwmConfig;
    pwm_fault_input_filter_param_t pwmFaultInputFilterParam;

    cmp_config_t mCmpConfigStruct;
    cmp_dac_config_t mCmpDacConfigStruct;

    uint8_t ret     = 0U;
    uint16_t i      = 0U;
    uint32_t pwmVal = 4U;

    BOARD_ConfigMPU();
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /* Set the PWM Fault inputs to CMP0 output pin */
    XBARA_Init(XBARA1);
    XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputAcmp1Out, kXBARA1_OutputFlexpwm1Fault0);

    /*
     * mCmpConfigStruct.enableCmp = true;
     * mCmpConfigStruct.hysteresisMode = kCMP_HysteresisLevel0;
     * mCmpConfigStruct.enableHighSpeed = false;
     * mCmpConfigStruct.enableInvertOutput = false;
     * mCmpConfigStruct.useUnfilteredOutput = false;
     * mCmpConfigStruct.enablePinOut = false;
     * mCmpConfigStruct.enableTriggerMode = false;
     */
    CMP_GetDefaultConfig(&mCmpConfigStruct);
    /* Init the CMP comparator. */
    CMP_Init(DEMO_CMP_BASE, &mCmpConfigStruct);

    /* Configure the DAC channel. */
    mCmpDacConfigStruct.referenceVoltageSource = kCMP_VrefSourceVin2; /* VCC. */
    mCmpDacConfigStruct.DACValue               = 32U;                 /* Half voltage of logic high level. */
    CMP_SetDACConfig(DEMO_CMP_BASE, &mCmpDacConfigStruct);
    CMP_SetInputChannels(DEMO_CMP_BASE, DEMO_CMP_USER_CHANNEL, DEMO_CMP_DAC_CHANNEL);

    /*
     * pwmConfig.enableDebugMode = false;
     * pwmConfig.enableWait = false;
     * pwmConfig.reloadSelect = kPWM_LocalReload;
     * pwmConfig.clockSource = kPWM_BusClock;
     * pwmConfig.prescale = kPWM_Prescale_Divide_1;
     * pwmConfig.initializationControl = kPWM_Initialize_LocalSync;
     * pwmConfig.forceTrigger = kPWM_Force_Local;
     * pwmConfig.reloadFrequency = kPWM_LoadEveryOportunity;
     * pwmConfig.reloadLogic = kPWM_ReloadImmediate;
     * pwmConfig.pairOperation = kPWM_Independent;
     */
    PWM_GetDefaultConfig(&pwmConfig);
    pwmConfig.prescale = kPWM_Prescale_Divide_1;
    /* Use full cycle reload */
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
    /* PWM A & PWM B operate as 2 independent channels */
    pwmConfig.pairOperation   = kPWM_Independent;
    pwmConfig.enableDebugMode = true;

    /* Initialize submodule 0 */
    ret = PWM_Init(DEMO_PWM_BASEADDR, DEMO_PWM_SUBMODULE, &pwmConfig);
    if (ret != kStatus_Success)
    {
        PRINTF("\r\nPWM INIT FAILED");
        return 1;
    }

    /* Fault filter count */
    pwmFaultInputFilterParam.faultFilterCount = 0x07U;
    /* Fault filter period; value of 0 will bypass the filter */
    pwmFaultInputFilterParam.faultFilterPeriod = 0x14U;
    /* Disable fault glitch stretch */
    pwmFaultInputFilterParam.faultGlitchStretch = false;
    PWM_SetupFaultInputFilter(DEMO_PWM_BASEADDR, &pwmFaultInputFilterParam);

    PWM_InitPhasePwm();

    PWM_SetupFaultPwm();

    PRINTF("\r\n\r\nWelcome to PWM Fault demo");
    PRINTF("\r\nUse oscilloscope to see PWM signal at probe pin: %s", DEMO_PWM_CHANNEL_LOCATION_ON_BOARD);
    PRINTF("\r\nConnect pin %s to high level and ground to see change.", DEMO_CMP_INPUT_PIN_LOCATION_ON_BOARD);

    PWM_StartTimer(DEMO_PWM_BASEADDR, DEMO_PWM_CONTROL_SUBMODULE);
    CMP_Enable(DEMO_CMP_BASE, true);

    while (1)
    {
        /* Time delay for update the duty cycle percentage */
        for (i = 0U; i < DEMO_PWM_DELAY_VAL; i++)
        {
            __NOP();
        }
        pwmVal = pwmVal + 4;

        /* Reset the duty cycle percentage */
        if (pwmVal > 100)
        {
            pwmVal = 4;
        }

        /* Update duty cycles for PWM signals */
        PWM_UpdatePwmDutycycle(DEMO_PWM_BASEADDR, DEMO_PWM_SUBMODULE, DEMO_PWM_CHANNEL, kPWM_SignedCenterAligned,
                               pwmVal);
        /* Set the load okay bit for all submodules to load registers from their buffer */
        PWM_SetPwmLdok(DEMO_PWM_BASEADDR, DEMO_PWM_CONTROL_SUBMODULE, true);
    }
}
