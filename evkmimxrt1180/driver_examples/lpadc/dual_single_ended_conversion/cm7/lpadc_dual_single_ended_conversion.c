/*
 * Copyright 2022 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_lpadc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE                    ADC1
#define DEMO_LPADC_USER_CHANNELA           7U
#define DEMO_LPADC_USER_CHANNELB           6U
#define DEMO_LPADC_USER_CMDID              1U /* CMD1 */
/* ERRATA051385: ADC INL/DNL degrade under high ADC clock frequency when VREFH selected as reference. */
#define DEMO_LPADC_VREF_SOURCE             kLPADC_ReferenceVoltageAlt2
#define DEMO_LPADC_USE_HIGH_RESOLUTION     true
#define DEMO_LPADC_OFFSET_CALIBRATION_MODE kLPADC_OffsetCalibration16bitMode

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitADCClock(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
#if (defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION)
const uint32_t g_LpadcFullRange   = 65536U;
const uint32_t g_LpadcResultShift = 0U;
#else
const uint32_t g_LpadcFullRange   = 4096U;
const uint32_t g_LpadcResultShift = 3U;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */

/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_InitADCClock(void)
{
    clock_root_config_t adc1ClkRoot;
    adc1ClkRoot.mux      = 1U;
    adc1ClkRoot.div      = 7U;
    adc1ClkRoot.clockOff = false;
    CLOCK_SetRootClock(kCLOCK_Root_Adc1, &adc1ClkRoot);
}


/*!
 * @brief Main function
 */
int main(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;
    lpadc_conv_result_t mLpadcResultConfigStruct;

    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("LPADC Triger Dual Channel Example\r\n");

    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
#if defined(DEMO_LPADC_VREF_SOURCE)
    mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
    mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

    /* Set offset calibration mode*/
    LPADC_SetOffsetCalibrationMode(DEMO_LPADC_BASE, DEMO_LPADC_OFFSET_CALIBRATION_MODE);
    /* Request offset calibration. */
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
    /* Request gain calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.sampleChannelMode = kLPADC_SampleChannelDualSingleEndBothSide;
    mLpadcCommandConfigStruct.channelNumber     = DEMO_LPADC_USER_CHANNELA;
#if defined(DEMO_LPADC_USE_HIGH_RESOLUTION) && DEMO_LPADC_USE_HIGH_RESOLUTION
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* DEMO_LPADC_USE_HIGH_RESOLUTION */
    mLpadcCommandConfigStruct.channelBNumber = DEMO_LPADC_USER_CHANNELB;
    mLpadcCommandConfigStruct.enableChannelB = true;
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId       = DEMO_LPADC_USER_CMDID;
    mLpadcTriggerConfigStruct.enableHardwareTrigger = false;
    mLpadcTriggerConfigStruct.channelBFIFOSelect    = 1U;
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */

    PRINTF("ADC Full Range: %d\r\n", g_LpadcFullRange);
    PRINTF("Please press any key to get user channel's ADC value.\r\n");

    while (1)
    {
        GETCHAR();
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U); /* 1U is trigger0 mask. */
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 0U))
        {
        }
        PRINTF("ADC channal A value: %d\r\n", ((mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift));

        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 1U))
        {
        }
        PRINTF("ADC channal B value: %d\r\n", ((mLpadcResultConfigStruct.convValue) >> g_LpadcResultShift));
    }
}
