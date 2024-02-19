/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_opamp.h"
#include "fsl_sctimer.h"

#include "fsl_inputmux.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_INPUTMUX_BASEADDR INPUTMUX

#define SCTIMER_CLK_FREQ       CLOCK_GetFreq(kCLOCK_BusClk)
#define DEMO_FIRST_SCTIMER_OUT kSCTIMER_Out_4

#define DEMO_OPAMP_BASEADDR OPAMP0

void OPAMP_Configuration(void);
void SCTIMER_Configuration(void);
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void Inputmux_Configuration(void);
/*******************************************************************************
 * Variables
 ******************************************************************************/
/*******************************************************************************
 * Code
 ******************************************************************************/

void Inputmux_Configuration(void)
{
    /*  Init input_mux */
    INPUTMUX_Init(DEMO_INPUTMUX_BASEADDR);
    INPUTMUX_AttachSignal(DEMO_INPUTMUX_BASEADDR, 0, kINPUTMUX_SctOut4ToOpamp0Trigger);
}

int main(void)
{
    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to SCT */
    CLOCK_SetClkDiv(kCLOCK_DivSctClk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_SCT);

    /* enable analog module */
    SPC0->ACTIVE_CFG1 |= 0x100;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    OPAMP_Configuration();
    Inputmux_Configuration();
    SCTIMER_Configuration();

    PRINTF("OPAMP TRIGGER MODE EXAMPLE!\r\n");

    while (1)
    {
    }
}

void OPAMP_Configuration(void)
{
    opamp_config_t config;
    /*
     *  config->enable        = false;
     *  config->enablePosADCSw1 = false;
     *  config->mode          = kOPAMP_LowNoiseMode;
     *  config->trimOption    = kOPAMP_TrimOptionDefault;
     *  config->intRefVoltage = kOPAMP_IntRefVoltVddaDiv2;
     *  config->enablePosADCSw1 = false;
     *  config->enablePosADCSw2 = false;
     *  config->posRefVoltage = kOPAMP_PosRefVoltVrefh3;
     *  config->posGain       = kOPAMP_PosGainReserved;
     *  config->negGain       = kOPAMP_NegGainBufferMode;
     *  config->enableRefBuffer = false;
     *  config->PosInputChannelSelection  = kOPAMP_PosInputChannel0
     *  config->enableTriggerMode = false;
     *  config->enableOutputSwitch = true;
     */
    OPAMP_GetDefaultConfig(&config);
    config.posGain = kOPAMP_PosGainNonInvertDisableBuffer2X;
    config.negGain = kOPAMP_NegGainInvert1X;
    config.enable  = true;
    OPAMP_Init(DEMO_OPAMP_BASEADDR, &config);
    OPAMP_EnableTriggerMode(DEMO_OPAMP_BASEADDR, true);
}

void SCTIMER_Configuration(void)
{
    sctimer_config_t sctimerInfo;
    sctimer_pwm_signal_param_t pwmParam;
    uint32_t event;
    uint32_t sctimerClock;

    sctimerClock = SCTIMER_CLK_FREQ;
    SCTIMER_GetDefaultConfig(&sctimerInfo);
    /* Initialize SCTimer module */
    SCTIMER_Init(SCT0, &sctimerInfo);

    /* Configure first PWM with frequency 24kHZ from first output */
    pwmParam.output           = DEMO_FIRST_SCTIMER_OUT;
    pwmParam.level            = kSCTIMER_LowTrue;
    pwmParam.dutyCyclePercent = 10;
    if (SCTIMER_SetupPwm(SCT0, &pwmParam, kSCTIMER_CenterAlignedPwm, 24000U, sctimerClock, &event) == kStatus_Fail)
    {
        return;
    }
    SCTIMER_StartTimer(SCT0, kSCTIMER_Counter_U);
}
