/*
 * Copyright 2023 NXP.
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpcmp.h"

#include "fsl_vref.h"
#include "fsl_spc.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_SPC_BASE                                  SPC0
#define DEMO_VREF_BASE                                 VREF0
#define DEMO_LPCMP_BASE                                CMP0
#define DEMO_LPCMP_IRQ_ID                              HSCMP0_IRQn
#define DEMO_LPCMP_IRQ_HANDLER_FUNC                    HSCMP0_IRQHandler
#define DEMO_LPCMP_ROUND_ROBIN_FIXED_CHANNEL           7U
#define DEMO_LPCMP_ROUND_ROBIN_CHANNELS_CHECKER_MASK   0x31U
#define DEMO_LPCMP_ROUND_ROBIN_CHANNELS_PRE_STATE_MASK 0x10U
#define DEMO_LPCMP_ROUND_ROBIN_FIXED_MUX_PORT          kLPCMP_FixedPlusMuxPort

/* (LPCMPx_RRCR0[RR_NSAM] / roundrobin clock period) should bigger than CMP propagation delay, roundrobin clock period
 * can be obtained by the CLOCK_GetCmpRRClkFreq function, the propagation delay specified in reference manual is 5us.
 * Note that the range of LPCMPx_RRCR0[RR_NSAM] is limited, so the user needs to set the appropriate roundrobin clock
 * period.
 */
#define DEMO_LPCMP_ROUND_ROBIN_SAMPLE_CLOCK_NUMBERS (uint8_t)(5U * CLOCK_GetCmpRRClkFreq(0U) / 1000000UL)

/* (LPCMPx_RRCR0[RR_INITMOD] / roundrobin clock period) should bigger than Initialization time, roundrobin clock period
 * can be obtained by the CLOCK_GetCmpRRClkFreq function, the initialization time specified in Reference Manual is 40us.
 * Note that the range of LPCMPx_RRCR0[RR_INITMOD] is limited, so the user needs to set the appropriate roundrobin clock
 * period.
 */
#define DEMO_LPCMP_ROUND_ROBIN_INIT_DELAY_MODULES (uint8_t)(40U * CLOCK_GetCmpRRClkFreq(0U) / 1000000UL)

/*
 * The roundrobin internal trigger signal generates rate is(LPCMPx_RRCR2[RR_TIMER_RELOAD] + 1) * roundrobin clock
 * period, roundrobin clock period can be obtained by the CLOCK_GetCmpRRClkFreq function, set the internal trigger
 * signal generates rate to 1s, LPCMPx_RRCR2[RR_TIMER_RELOAD] is (roundrobin clock period - 1).
 */
#define DEMO_LPCMP_ROUND_ROBIN_INTERAL_TIMER_RATE (CLOCK_GetCmpRRClkFreq(0U) - 1U)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    lpcmp_config_t mLpcmpConfigStruct;
    lpcmp_dac_config_t mLpcmpDacConfigStruct;
    lpcmp_roundrobin_config_t mLpcmpRoundRobinConfigStruct;

    /* Initialize hardware. */
    vref_config_t vrefConfig;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO 12M to CMP0 */
    CLOCK_SetClkDiv(kCLOCK_DivCmp0rrClk, 0x0FU);
    CLOCK_AttachClk(kFRO12M_to_CMP0RR);

    /* enable CMP0, CMP0_DAC  and VREF. */
    SPC_EnableActiveModeAnalogModules(DEMO_SPC_BASE, (kSPC_controlCmp0 | kSPC_controlCmp0Dac | kSPC_controlVref));

    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(DEMO_VREF_BASE, &vrefConfig);
    /* Get a 1.8V reference voltage. */
    VREF_SetTrim21Val(DEMO_VREF_BASE, 8U);

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    /*
     *   config->enableStopMode        = false;
     *   config->enableOutputPin       = false;
     *   config->useUnfilteredOutput   = false;
     *   config->enableInvertOutput    = false;
     *   config->enableNanoPowerMode   = false;
     *   config->enableHighSpeedMode   = false;
     *   config->hysteresisMode        = kLPCMP_HysteresisLevel0;
     *   config->functionalSourceClock = kLPCMP_FunctionalClockSource0;
     */
    LPCMP_GetDefaultConfig(&mLpcmpConfigStruct);

    /* Init the LPCMP module. */
    LPCMP_Init(DEMO_LPCMP_BASE, &mLpcmpConfigStruct);

    /* Configure the internal DAC to output half of reference voltage. */
    mLpcmpDacConfigStruct.enableLowPowerMode     = false;
    mLpcmpDacConfigStruct.referenceVoltageSource = kLPCMP_VrefSourceVin2;
    mLpcmpDacConfigStruct.DACValue =
        ((LPCMP_DCR_DAC_DATA_MASK >> LPCMP_DCR_DAC_DATA_SHIFT) >> 1U); /* Half of reference voltage. */
    LPCMP_SetDACConfig(DEMO_LPCMP_BASE, &mLpcmpDacConfigStruct);

    /* Configure the roundrobin mode. */
    mLpcmpRoundRobinConfigStruct.roundrobinTriggerSource = kLPCMP_TriggerSourceInternally;
    mLpcmpRoundRobinConfigStruct.sampleClockNumbers      = DEMO_LPCMP_ROUND_ROBIN_SAMPLE_CLOCK_NUMBERS;
    mLpcmpRoundRobinConfigStruct.initDelayModules        = DEMO_LPCMP_ROUND_ROBIN_INIT_DELAY_MODULES;
    mLpcmpRoundRobinConfigStruct.channelSampleNumbers    = 2U;

    /* The sampleTimeThreshhold can't bigger than channelSampleNumbers. */
    mLpcmpRoundRobinConfigStruct.sampleTimeThreshhold  = 1U;
    mLpcmpRoundRobinConfigStruct.fixedMuxPort          = DEMO_LPCMP_ROUND_ROBIN_FIXED_MUX_PORT;
    mLpcmpRoundRobinConfigStruct.roundrobinClockSource = kLPCMP_RoundRobinClockSource3;
    mLpcmpRoundRobinConfigStruct.fixedChannel          = DEMO_LPCMP_ROUND_ROBIN_FIXED_CHANNEL;
    mLpcmpRoundRobinConfigStruct.checkerChannelMask    = DEMO_LPCMP_ROUND_ROBIN_CHANNELS_CHECKER_MASK;

    /* Disable roundrobin mode before configure related registers. */
    LPCMP_EnableRoundRobinMode(DEMO_LPCMP_BASE, false);
    LPCMP_EnableRoundRobinInternalTimer(DEMO_LPCMP_BASE, false);

    LPCMP_SetRoundRobinConfig(DEMO_LPCMP_BASE, &mLpcmpRoundRobinConfigStruct);
    LPCMP_SetRoundRobinInternalTimer(DEMO_LPCMP_BASE, DEMO_LPCMP_ROUND_ROBIN_INTERAL_TIMER_RATE);
    LPCMP_SetPreSetValue(DEMO_LPCMP_BASE, DEMO_LPCMP_ROUND_ROBIN_CHANNELS_PRE_STATE_MASK);

    LPCMP_EnableRoundRobinInternalTimer(DEMO_LPCMP_BASE, true);
    LPCMP_EnableRoundRobinMode(DEMO_LPCMP_BASE, true);

    /* Enable the interrupt. */
    LPCMP_EnableInterrupts(DEMO_LPCMP_BASE, kLPCMP_RoundRobinInterruptEnable);
    EnableIRQ(DEMO_LPCMP_IRQ_ID);

    PRINTF("LPCMP RoundRobin Example.\r\n");

    while (1)
    {
    }
}

/*!
 * @brief ISR for LPCMP interrupt function.
 */
void DEMO_LPCMP_IRQ_HANDLER_FUNC(void)
{
    /* Get which channel is changed from pre-set value. */
    for (uint8_t index = 0U; index < 8U; index++)
    {
        if (0x01U == ((LPCMP_GetInputChangedFlags(DEMO_LPCMP_BASE) >> index) & 0x01U))
        {
            PRINTF("channel %d comparison result is different from preset value!\r\n", index);
        }
    }

    LPCMP_ClearStatusFlags(DEMO_LPCMP_BASE, LPCMP_GetStatusFlags(DEMO_LPCMP_BASE));
    LPCMP_ClearInputChangedFlags(DEMO_LPCMP_BASE, LPCMP_GetInputChangedFlags(DEMO_LPCMP_BASE));

    /* reset channel pre-set value, otherwise, the next interrupt can't take place. */
    LPCMP_SetPreSetValue(DEMO_LPCMP_BASE, DEMO_LPCMP_ROUND_ROBIN_CHANNELS_PRE_STATE_MASK);

    SDK_ISR_EXIT_BARRIER;
}
