/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_hscmp.h"

#include "fsl_vref.h"
#include "fsl_power.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_HSCMP_BASE             HSCMP0
#define DEMO_HSCMP_USER_CHANNEL     3U
#define DEMO_HSCMP_DAC_CHANNEL      5U
#define DEMO_HSCMP_IRQ_ID           HSCMP0_IRQn
#define LED_INIT()                  LED_RED_INIT(LOGIC_LED_OFF)
#define LED_ON()                    LED_RED_ON()
#define LED_OFF()                   LED_RED_OFF()
#define DEMO_HSCMP_IRQ_HANDLER_FUNC HSCMP0_IRQHandler
#define DEMO_VREF_BASE              VREF

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
    hscmp_config_t mHscmpConfigStruct;
    hscmp_dac_config_t mHscmpDacConfigStruct;

    /* Initialize hardware. */
    vref_config_t vrefConfig;

    BOARD_InitPins();
    BOARD_BootClockPLL150M();
    BOARD_InitDebugConsole();

    /* Disable VREF power down */
    POWER_DisablePD(kPDRUNCFG_PD_VREF);
    POWER_DisablePD(kPDRUNCFG_PD_CMPBIAS);
    POWER_DisablePD(kPDRUNCFG_PD_HSCMP0);
    POWER_DisablePD(kPDRUNCFG_PD_HSCMP0_DAC);

    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(DEMO_VREF_BASE, &vrefConfig);
    /* Get a 1.8V reference voltage. */
    VREF_SetTrim21Val(DEMO_VREF_BASE, 8U);

    PRINTF("HSCMP Interrupt Example.\r\n");

    /*
     *   k_HscmpConfigStruct->enableStopMode      = false;
     *   k_HscmpConfigStruct->enableOutputPin     = false;
     *   k_HscmpConfigStruct->useUnfilteredOutput = false;
     *   k_HscmpConfigStruct->enableInvertOutput  = false;
     *   k_HscmpConfigStruct->hysteresisMode      = kHSCMP_HysteresisLevel0;
     *   k_HscmpConfigStruct->powerMode           = kHSCMP_LowSpeedPowerMode;
     */
    HSCMP_GetDefaultConfig(&mHscmpConfigStruct);
    /* Init the HSCMP module. */
    HSCMP_Init(DEMO_HSCMP_BASE, &mHscmpConfigStruct);

    /* Configure the internal DAC to output half of reference voltage. */
    mHscmpDacConfigStruct.enableLowPowerMode     = false;
    mHscmpDacConfigStruct.referenceVoltageSource = kHSCMP_VrefSourceVin2;
    mHscmpDacConfigStruct.DACValue =
        ((HSCMP_DCR_DAC_DATA_MASK >> HSCMP_DCR_DAC_DATA_SHIFT) >> 1U); /* Half of reference voltage. */
    HSCMP_SetDACConfig(DEMO_HSCMP_BASE, &mHscmpDacConfigStruct);

    /* Configure HSCMP input channels. */
    HSCMP_SetInputChannels(DEMO_HSCMP_BASE, DEMO_HSCMP_USER_CHANNEL, DEMO_HSCMP_DAC_CHANNEL);

    /* Init the LED. */
    LED_INIT();

    /* Enable the interrupt. */
    EnableIRQ(DEMO_HSCMP_IRQ_ID);
    HSCMP_EnableInterrupts(DEMO_HSCMP_BASE, kHSCMP_OutputRisingEventFlag | kHSCMP_OutputFallingEventFlag);

    while (1)
    {
    }
}

/*!
 * @brief ISR for HSCMP interrupt function.
 */
void DEMO_HSCMP_IRQ_HANDLER_FUNC(void)
{
    HSCMP_ClearStatusFlags(DEMO_HSCMP_BASE, kHSCMP_OutputRisingEventFlag | kHSCMP_OutputFallingEventFlag);
    if (kHSCMP_OutputAssertEventFlag == (kHSCMP_OutputAssertEventFlag & HSCMP_GetStatusFlags(DEMO_HSCMP_BASE)))
    {
        LED_ON(); /* Turn on the led. */
    }
    else
    {
        LED_OFF(); /* Turn off the led. */
    }
    SDK_ISR_EXIT_BARRIER;
}
