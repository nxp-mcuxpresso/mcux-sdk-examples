/*
 * Copyright 2019-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "fsl_vref.h"
#include "fsl_lpadc.h"

#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_LPADC_BASE ADC0
/* Use adc to measure VREF_OUT. */
#define DEMO_LPADC_USER_CHANNEL 2U
#define DEMO_LPADC_USER_CMDID   1U
/* Use VDD_ANA. */
#define DEMO_LPADC_VREF_SOURCE  kLPADC_ReferenceVoltageAlt3
#define DEMO_LPADC_VREF_VOLTAGE 3.300f

#define DEMO_VREF_BASE VREF0

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitDebugConsole(void);
static void LPADC_Configuration(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/

#if defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE
const uint32_t g_Lpadc_FullRange   = 65536UL;
const uint32_t g_Lpadc_ResultShift = 0UL;
#else
const uint32_t g_Lpadc_FullRange   = 4096UL;
const uint32_t g_Lpadc_ResultShift = 3UL;
#endif /* FSL_FEATURE_LPADC_HAS_CMDL_MODE */

/*******************************************************************************
 * Code
 ******************************************************************************/


static void LPADC_Configuration(void)
{
    lpadc_config_t mLpadcConfigStruct;
    lpadc_conv_trigger_config_t mLpadcTriggerConfigStruct;
    lpadc_conv_command_config_t mLpadcCommandConfigStruct;

    LPADC_GetDefaultConfig(&mLpadcConfigStruct);
    mLpadcConfigStruct.enableAnalogPreliminary = true;
#if defined(DEMO_LPADC_VREF_SOURCE)
    mLpadcConfigStruct.referenceVoltageSource = DEMO_LPADC_VREF_SOURCE;
#endif /* DEMO_LPADC_VREF_SOURCE */
#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS) && FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS
    mLpadcConfigStruct.conversionAverageMode = kLPADC_ConversionAverage128;
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CAL_AVGS */
    LPADC_Init(DEMO_LPADC_BASE, &mLpadcConfigStruct);

#if defined(FSL_FEATURE_LPADC_HAS_CTRL_CALOFS) && FSL_FEATURE_LPADC_HAS_CTRL_CALOFS
#if defined(FSL_FEATURE_LPADC_HAS_OFSTRIM) && FSL_FEATURE_LPADC_HAS_OFSTRIM
    LPADC_DoOffsetCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_OFSTRIM */
    /* Request gain calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CTRL_CALOFS */
#if (defined(FSL_FEATURE_LPADC_HAS_CFG_CALOFS) && FSL_FEATURE_LPADC_HAS_CFG_CALOFS)
    /* Do auto calibration. */
    LPADC_DoAutoCalibration(DEMO_LPADC_BASE);
#endif /* FSL_FEATURE_LPADC_HAS_CFG_CALOFS */

    /* Set conversion CMD configuration. */
    LPADC_GetDefaultConvCommandConfig(&mLpadcCommandConfigStruct);
    mLpadcCommandConfigStruct.channelNumber = DEMO_LPADC_USER_CHANNEL;
#if defined(FSL_FEATURE_LPADC_HAS_CMDL_MODE) && FSL_FEATURE_LPADC_HAS_CMDL_MODE
    mLpadcCommandConfigStruct.conversionResolutionMode = kLPADC_ConversionResolutionHigh;
#endif /* FSL_FEATURE_LPADC_HAS_CMDL_MODE */
    LPADC_SetConvCommandConfig(DEMO_LPADC_BASE, DEMO_LPADC_USER_CMDID, &mLpadcCommandConfigStruct);

    /* Set trigger configuration. */
    LPADC_GetDefaultConvTriggerConfig(&mLpadcTriggerConfigStruct);
    mLpadcTriggerConfigStruct.targetCommandId = DEMO_LPADC_USER_CMDID;           /* CMD15 is executed. */
    LPADC_SetConvTriggerConfig(DEMO_LPADC_BASE, 0U, &mLpadcTriggerConfigStruct); /* Configurate the trigger0. */
}

/*!
 * @brief Main function
 */
int main(void)
{
    vref_config_t vrefConfig;
    lpadc_conv_result_t mLpadcResultConfigStruct;
    uint8_t trimVal   = 0U;
    uint16_t adcValue = 0U;
    float voltage     = 0.0f;

    /* attach FRO 12M to FLEXCOMM4 (debug console) */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1u);
    CLOCK_AttachClk(BOARD_DEBUG_UART_CLK_ATTACH);

    /* attach FRO HF to ADC0 */
    CLOCK_SetClkDiv(kCLOCK_DivAdc0Clk, 1u);
    CLOCK_AttachClk(kFRO_HF_to_ADC0);

    /* enable VREF */
    SPC0->ACTIVE_CFG1 |= 0x1;

    BOARD_InitPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    PRINTF("\r\nVREF example\r\n");

    VREF_GetDefaultConfig(&vrefConfig);
    /* Initialize the VREF mode. */
    VREF_Init(DEMO_VREF_BASE, &vrefConfig);

    LPADC_Configuration();

    PRINTF("ADC Full Range: %d\r\n", g_Lpadc_FullRange);
    PRINTF("Default (Factory) trim value is :%d\r\n", VREF_GetTrim21Val(DEMO_VREF_BASE));

    /* Change the voltage by 0.1V each loop. */
    for (trimVal = 0U; trimVal < 0xCU; trimVal++)
    {
        VREF_SetTrim21Val(DEMO_VREF_BASE, trimVal);
        LPADC_DoSoftwareTrigger(DEMO_LPADC_BASE, 1U);
#if (defined(FSL_FEATURE_LPADC_FIFO_COUNT) && (FSL_FEATURE_LPADC_FIFO_COUNT == 2U))
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct, 0U))
#else
        while (!LPADC_GetConvResult(DEMO_LPADC_BASE, &mLpadcResultConfigStruct))
#endif /* FSL_FEATURE_LPADC_FIFO_COUNT */
        {
        }
        adcValue = mLpadcResultConfigStruct.convValue >> g_Lpadc_ResultShift;
        voltage  = DEMO_LPADC_VREF_VOLTAGE * ((float)adcValue / (float)g_Lpadc_FullRange);
        PRINTF("\r\nUse trim value: %d\r\n", trimVal);
        PRINTF("ADC conversion result: %d\r\n", adcValue);
        PRINTF("Expected voltage on VREF_OUT: %.3fV\r\n", 1.000f + (0.100f * (double)trimVal));
        PRINTF("Actual voltage on VREF_OUT: %.3fV\r\n", (double)voltage);
    }

    while (1)
    {
    }
}
